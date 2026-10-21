#include <xtl.h>
#include "pbrb_packages.h"
#include "xpd_reader.h"
#include <string.h>

// Foundation-only XPD validation. XPD1's structural checks happen in xpd_open.
// Read the first and final bank with CRC verification. The cartridge title must
// match the selected version as well as the outer XPD game identifier.
// Full 1MB verification is intentionally deferred to the engine/ROM audit pass.
PbrbPackStatus PbrbInspectPack(const char* path, unsigned game) {
    XpdArchive x; XpdResource full, bank0, bankLast;
    memset(&x, 0, sizeof(x));
    unsigned char bank[16384]; uint32_t got = 0;
    if (!path)return PBRB_INVALID;
    DWORD attr = GetFileAttributesA(path);
    if (attr == (DWORD)-1 || (attr & FILE_ATTRIBUTE_DIRECTORY))return PBRB_MISSING;
    if (!xpd_open(&x, path))return PBRB_INVALID;
    const char* title = (game == XPD_GAME_RED) ? "POKEMON RED" : "POKEMON BLUE";
    const unsigned titleLen = game == XPD_GAME_RED ? 11u : 12u;
    int ok = x.game == game &&
        xpd_find(&x, "rom/full.gb", &full) && full.size == 1024u * 1024u &&
        xpd_find(&x, "rom/bank/00.bin", &bank0) && bank0.size == sizeof(bank) &&
        xpd_find(&x, "rom/bank/63.bin", &bankLast) && bankLast.size == sizeof(bank) &&
        xpd_read(&x, "rom/bank/00.bin", bank, sizeof(bank), &got) && got == sizeof(bank) &&
        memcmp(bank + 0x104, "\xCE\xED\x66\x66", 4) == 0 &&
        memcmp(bank + 0x134, title, titleLen) == 0 &&
        xpd_read(&x, "rom/bank/63.bin", bank, sizeof(bank), &got) && got == sizeof(bank);
    xpd_close(&x);
    return ok ? PBRB_READY : PBRB_INVALID;
}

/* Native Xbox storage. U: is the current XBE title's persistent user-data
   root; Z: is utility/cache storage. XCreateSaveGame returns the container
   path, including its native metadata. Do not mount/format a utility drive,
   hardcode another title's ID, or nest a second UDATA directory under U:. */
static const PbrbPreferences kDefaultPreferences = { 0, 0, 0, 1, 1, 1, 10, 0, 100, 0, 0 };
static PbrbPreferences s_preferences = kDefaultPreferences;
static char s_saveDir[MAX_PATH] = "";
static const char* s_saveError = "";
static const unsigned char kSaveMagic[8] = { 'P','B','R','B','S','A','V','E' };
enum { SAVE_HEADER_BYTES = 32, SAVE_FORMAT = 1 };
struct SaveRecord { unsigned generation, bytes, crc; };

void PbrbDefaultPreferences(PbrbPreferences* p) { if (p)*p = kDefaultPreferences; }
const PbrbPreferences* PbrbGetPreferences() { return &s_preferences; }
const char* PbrbSaveError() { return s_saveError; }

static unsigned ReadLE32(const unsigned char* p) {
    return (unsigned)p[0] | ((unsigned)p[1] << 8) |
        ((unsigned)p[2] << 16) | ((unsigned)p[3] << 24);
}
static void WriteLE32(unsigned char* p, unsigned v) {
    p[0] = (unsigned char)v; p[1] = (unsigned char)(v >> 8);
    p[2] = (unsigned char)(v >> 16); p[3] = (unsigned char)(v >> 24);
}
static unsigned SaveCRC(unsigned c, const unsigned char* p, unsigned n) {
    while (n--) {
        c ^= *p++;
        for (int bit = 0; bit < 8; ++bit)
            c = (c >> 1) ^ (0xedb88320u & (0u - (c & 1u)));
    }
    return c;
}
enum { PREFERENCES_BYTES = 16, PREFERENCES_FORMAT = 1 };
static int ValidPreferences(const PbrbPreferences* p) {
    return p && p->smooth >= 0 && p->smooth <= 1 && p->rumble >= 0 && p->rumble <= 2 &&
        p->color >= 0 && p->color <= 1 && p->launcherMusic >= 0 && p->launcherMusic <= 1 &&
        p->music >= 0 && p->music <= 1 && p->effects >= 0 && p->effects <= 1 &&
        p->volume >= 0 && p->volume <= 10 && p->dpadOnly >= 0 && p->dpadOnly <= 1 &&
        p->screenScale >= 80 && p->screenScale <= 100 &&
        p->screenX >= -5 && p->screenX <= 5 && p->screenY >= -5 && p->screenY <= 5;
}
/* -2 protects an unfamiliar format from being overwritten by this build.
   The three-byte format is the original filtering/rumble/color preference. */
static int DecodePreferences(const unsigned char* data, unsigned bytes, PbrbPreferences* p) {
    PbrbPreferences next = kDefaultPreferences;
    if (bytes == 3u) {
        if (data[0] > 1u || data[1] > 1u || data[2] > 1u)return -1;
        next.smooth = data[0]; next.rumble = data[1] ? 2 : 0; next.color = data[2];
    }
    else {
        if (bytes < 5u || memcmp(data, "PBOP", 4) || data[4] != PREFERENCES_FORMAT)return -2;
        if (bytes != PREFERENCES_BYTES)return -1;
        next.smooth = data[5]; next.rumble = data[6]; next.color = data[7];
        next.launcherMusic = data[8]; next.music = data[9]; next.effects = data[10];
        next.volume = data[11]; next.dpadOnly = data[12]; next.screenScale = data[13];
        /* Biased unsigned bytes avoid compiler-dependent signed-char decoding. */
        next.screenX = (int)data[14] - 5; next.screenY = (int)data[15] - 5;
        if (!ValidPreferences(&next))return -1;
    }
    if (p)*p = next;
    return 1;
}
static int SaveDirectory(int create) {
    if (s_saveDir[0]) {
        const DWORD attr = GetFileAttributesA(s_saveDir);
        if (attr != (DWORD)-1 && (attr & FILE_ATTRIBUTE_DIRECTORY))return 1;
        s_saveDir[0] = 0;
    }
    const DWORD result = XCreateSaveGame("U:\\", L"PokeRBXB",
        create ? OPEN_ALWAYS : OPEN_EXISTING, 0, s_saveDir, sizeof(s_saveDir));
    if (result != ERROR_SUCCESS && result != ERROR_ALREADY_EXISTS) {
        s_saveDir[0] = 0;
        if (create || (result != ERROR_FILE_NOT_FOUND && result != ERROR_PATH_NOT_FOUND))
            s_saveError = "Could not open the Xbox save container.";
        return 0;
    }
    unsigned n = 0;
    while (n < sizeof(s_saveDir) && s_saveDir[n])++n;
    if (!n || n >= sizeof(s_saveDir) - 1u) {
        s_saveDir[0] = 0;
        s_saveError = "The Xbox save path is invalid.";
        return 0;
    }
    if (s_saveDir[n - 1] != '\\' && s_saveDir[n - 1] != '/') {
        s_saveDir[n] = '\\'; s_saveDir[n + 1] = 0;
    }
    const DWORD attr = GetFileAttributesA(s_saveDir);
    if (attr == (DWORD)-1 || !(attr & FILE_ATTRIBUTE_DIRECTORY)) {
        s_saveDir[0] = 0;
        s_saveError = "The Xbox save container is unavailable.";
        return 0;
    }
    return 1;
}
static int SavePath(char* path, unsigned kind, unsigned copy) {
    const char* file = kind == 1 ? (copy ? "red1.sav" : "red0.sav") :
        kind == 2 ? (copy ? "blue1.sav" : "blue0.sav") :
        (copy ? "options1.dat" : "options0.dat");
    const unsigned a = (unsigned)strlen(s_saveDir), b = (unsigned)strlen(file);
    if (!a || a + b >= MAX_PATH)return 0;
    memcpy(path, s_saveDir, a); memcpy(path + a, file, b + 1);
    return 1;
}
/* 0 absent, -1 invalid/unreadable, -2 newer format, 1 valid. A null output verifies the
   complete payload. No compiler struct layout is written to the disk. */
static int ReadSaveRecord(const char* path, unsigned kind, SaveRecord* record,
    void* output, unsigned capacity) {
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        const DWORD e = GetLastError();
        return (e == ERROR_FILE_NOT_FOUND || e == ERROR_PATH_NOT_FOUND) ? 0 : -1;
    }
    unsigned char header[SAVE_HEADER_BYTES], block[1024], prefix[PREFERENCES_BYTES];
    memset(prefix, 0, sizeof(prefix));
    DWORD got = 0;
    const DWORD size = GetFileSize(file, NULL);
    int ok = ReadFile(file, header, sizeof(header), &got, NULL) && got == sizeof(header);
    if (ok) {
        if (!memcmp(header, kSaveMagic, 8) && ReadLE32(header + 8) != SAVE_FORMAT) {
            CloseHandle(file);
            return -2; /* Preserve a format this executable cannot interpret. */
        }
        record->bytes = ReadLE32(header + 16);
        record->generation = ReadLE32(header + 20);
        record->crc = ReadLE32(header + 24);
        ok = !memcmp(header, kSaveMagic, 8) && ReadLE32(header + 8) == SAVE_FORMAT &&
            ReadLE32(header + 12) == kind && record->bytes > 0u &&
            record->bytes <= PBRB_SAVE_MAX_BYTES &&
            size == SAVE_HEADER_BYTES + record->bytes &&
            ReadLE32(header + 28) == ~SaveCRC(0xffffffffu, header, 28) &&
            (!output || record->bytes <= capacity);
    }
    unsigned crc = 0xffffffffu, done = 0;
    while (ok && done < record->bytes) {
        const unsigned remain = record->bytes - done;
        const DWORD count = remain < sizeof(block) ? remain : (DWORD)sizeof(block);
        got = 0;
        ok = ReadFile(file, block, count, &got, NULL) && got == count;
        if (ok) {
            crc = SaveCRC(crc, block, count);
            if (!done)memcpy(prefix, block, count < sizeof(prefix) ? count : sizeof(prefix));
            if (output)memcpy((unsigned char*)output + done, block, count);
            done += count;
        }
    }
    CloseHandle(file);
    if (!ok || ~crc != record->crc)return -1;
    if (!kind)return DecodePreferences(prefix, record->bytes, NULL);
    return 1;
}
static int NewerGeneration(unsigned a, unsigned b) {
    /* Serial-number arithmetic also handles the unsigned counter wrapping. */
    const unsigned d = a - b;
    return d != 0u && d < 0x80000000u;
}
static int LoadSaveRecord(unsigned kind, void* output, unsigned capacity, unsigned* bytes) {
    s_saveError = "";
    if (bytes)*bytes = 0;
    if (!SaveDirectory(0))return 0;
    char paths[2][MAX_PATH];
    if (!SavePath(paths[0], kind, 0) || !SavePath(paths[1], kind, 1)) {
        s_saveError = "The Xbox save path is too long."; return 0;
    }
    SaveRecord records[2]; memset(records, 0, sizeof(records));
    const int a = ReadSaveRecord(paths[0], kind, &records[0], NULL, 0);
    const int b = ReadSaveRecord(paths[1], kind, &records[1], NULL, 0);
    if (a != 1 && b != 1) {
        if (a < 0 || b < 0)s_saveError = "The saved data could not be read.";
        return 0;
    }
    const unsigned best = a != 1 || (b == 1 && NewerGeneration(records[1].generation,
        records[0].generation)) ? 1u : 0u;
    if (output) {
        if (records[best].bytes > capacity) {
            s_saveError = "The save uses an unsupported data size."; return 0;
        }
        if (ReadSaveRecord(paths[best], kind, &records[best], output, capacity) != 1) {
            /* A read can fail after the initial check. Keep the preceding
               snapshot available instead of returning a partial payload. */
            const unsigned older = best ^ 1u;
            if ((older ? b : a) != 1 || records[older].bytes > capacity ||
                ReadSaveRecord(paths[older], kind, &records[older], output, capacity) != 1) {
                s_saveError = "The saved data could not be read."; return 0;
            }
            if (bytes)*bytes = records[older].bytes;
            return 1;
        }
    }
    if (bytes)*bytes = records[best].bytes;
    return 1;
}
static int WriteSaveRecord(unsigned kind, const void* data, unsigned bytes) {
    s_saveError = "";
    if (!data || !bytes || bytes > PBRB_SAVE_MAX_BYTES) {
        s_saveError = "The game supplied invalid save data."; return 0;
    }
    if (!SaveDirectory(1))return 0;
    char paths[2][MAX_PATH];
    if (!SavePath(paths[0], kind, 0) || !SavePath(paths[1], kind, 1)) {
        s_saveError = "The Xbox save path is too long."; return 0;
    }
    SaveRecord records[2]; memset(records, 0, sizeof(records));
    const int a = ReadSaveRecord(paths[0], kind, &records[0], NULL, 0);
    const int b = ReadSaveRecord(paths[1], kind, &records[1], NULL, 0);
    if (a == -2 || b == -2) {
        s_saveError = "A newer save format is present and has been preserved.";
        return 0;
    }
    unsigned target = 0, generation = 1;
    if (a == 1 || b == 1) {
        const unsigned newest = a != 1 || (b == 1 && NewerGeneration(records[1].generation,
            records[0].generation)) ? 1u : 0u;
        target = newest ^ 1u;
        generation = records[newest].generation + 1u;
    }
    unsigned char header[SAVE_HEADER_BYTES];
    memcpy(header, kSaveMagic, 8);
    WriteLE32(header + 8, SAVE_FORMAT); WriteLE32(header + 12, kind);
    WriteLE32(header + 16, bytes); WriteLE32(header + 20, generation);
    const unsigned crc = ~SaveCRC(0xffffffffu, (const unsigned char*)data, bytes);
    WriteLE32(header + 24, crc);
    WriteLE32(header + 28, ~SaveCRC(0xffffffffu, header, 28));

    /* Alternating files avoid relying on FATX rename atomicity. The last
       valid generation is never opened for writing during this operation. */
    HANDLE file = CreateFileA(paths[target], GENERIC_WRITE, 0, NULL,
        CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE) {
        s_saveError = "Could not write the Xbox save file."; return 0;
    }
    DWORD wrote = 0;
    int ok = WriteFile(file, header, sizeof(header), &wrote, NULL) && wrote == sizeof(header);
    if (ok) {
        wrote = 0;
        ok = WriteFile(file, data, bytes, &wrote, NULL) && wrote == bytes;
    }
    if (ok)ok = FlushFileBuffers(file) != 0;
    if (!CloseHandle(file))ok = 0;
    SaveRecord verify; memset(&verify, 0, sizeof(verify));
    if (ok)ok = ReadSaveRecord(paths[target], kind, &verify, NULL, 0) == 1 &&
        verify.generation == generation && verify.bytes == bytes && verify.crc == crc;
    if (!ok) {
        /* A failed flush may leave an apparently valid new record in cache.
           Remove that attempted generation; the prior valid file is intact. */
        DeleteFileA(paths[target]);
        s_saveError = (a == 1 || b == 1) ?
            "Save failed. The previous save is retained." :
            "Save failed. No earlier save was available.";
        return 0;
    }
    return 1;
}
int PbrbLoadPreferences() {
    unsigned char data[PREFERENCES_BYTES]; unsigned bytes = 0;
    PbrbDefaultPreferences(&s_preferences);
    if (!LoadSaveRecord(0, data, sizeof(data), &bytes))return 0;
    if (DecodePreferences(data, bytes, &s_preferences) != 1) {
        s_saveError = "The saved options are invalid."; return 0;
    }
    return 1;
}
int PbrbSavePreferences(const PbrbPreferences* p) {
    if (!ValidPreferences(p)) { s_saveError = "The selected options are invalid."; return 0; }
    unsigned char data[PREFERENCES_BYTES];
    memcpy(data, "PBOP", 4); data[4] = PREFERENCES_FORMAT;
    data[5] = (unsigned char)p->smooth; data[6] = (unsigned char)p->rumble;
    data[7] = (unsigned char)p->color; data[8] = (unsigned char)p->launcherMusic;
    data[9] = (unsigned char)p->music; data[10] = (unsigned char)p->effects;
    data[11] = (unsigned char)p->volume; data[12] = (unsigned char)p->dpadOnly;
    data[13] = (unsigned char)p->screenScale;
    data[14] = (unsigned char)(p->screenX + 5); data[15] = (unsigned char)(p->screenY + 5);
    s_preferences = *p;
    return WriteSaveRecord(0, data, sizeof(data));
}
int PbrbSaveGame(unsigned game, const void* data, unsigned bytes) {
    if (game != 1u && game != 2u) { s_saveError = "The save version is invalid."; return 0; }
    return WriteSaveRecord(game, data, bytes);
}
int PbrbLoadGame(unsigned game, void* data, unsigned capacity, unsigned* bytes) {
    if (bytes)*bytes = 0;
    if ((game != 1u && game != 2u) || !data || !capacity) {
        s_saveError = "The save request is invalid."; return 0;
    }
    return LoadSaveRecord(game, data, capacity, bytes);
}
int PbrbHasSave(unsigned game) {
    if (game != 1u && game != 2u)return 0;
    return LoadSaveRecord(game, NULL, 0, NULL);
}

/* Read the documented native payload layout, never a live gameplay object.
   This bounded preview checks framing and displayed fields without a ROM;
   CONTINUE remains responsible for ROM-specific Pokemon/map validation. */
struct SummaryReader { const unsigned char* data; unsigned bytes, at; int ok; };
static unsigned SummaryGet(SummaryReader* r, unsigned bytes) {
    if (!r->ok || bytes > r->bytes - r->at) { r->ok = 0; return 0; }
    unsigned v = 0;
    for (unsigned i = 0; i < bytes; ++i)v |= (unsigned)r->data[r->at++] << (i * 8u);
    return v;
}
static void SummarySkip(SummaryReader* r, unsigned bytes) {
    if (!r->ok || bytes > r->bytes - r->at)r->ok = 0;
    else r->at += bytes;
}
static unsigned SummaryName(SummaryReader* r, unsigned limit, char* output) {
    unsigned used = 0, letters = 0; int ended = 0;
    for (unsigned i = 0; i < 11u; ++i) {
        unsigned c = SummaryGet(r, 1);
        if (c == 0x50u) { ended = 1; continue; }
        if (ended || i >= limit || c < 0x60u) { r->ok = 0; continue; }
        ++letters;
        if (!output)continue;
        char a = '?', b = 0;
        if (c >= 0x80u && c <= 0x99u)a = (char)('A' + c - 0x80u);
        else if (c >= 0xa0u && c <= 0xb9u)a = (char)('a' + c - 0xa0u);
        else if (c >= 0xf6u)a = (char)('0' + c - 0xf6u);
        else if (c >= 0x9au && c <= 0x9fu)a = "():;[]"[c - 0x9au];
        else switch (c) {
        case 0x7f: a = ' '; break;
        case 0xba: a = 'e'; break;
        case 0x70: case 0x71: case 0xe0: a = '\''; break;
        case 0x72: case 0x73: a = '"'; break;
        case 0xbb: case 0xbc: case 0xbd: case 0xbe: case 0xbf:
            a = '\''; b = "dlstv"[c - 0xbbu]; break;
        case 0xe1: a = 'P'; b = 'K'; break;
        case 0xe2: a = 'M'; b = 'N'; break;
        case 0xe3: a = '-'; break;
        case 0xe4: a = '\''; b = 'r'; break;
        case 0xe5: a = '\''; b = 'm'; break;
        case 0xe6: a = '?'; break;
        case 0xe7: a = '!'; break;
        case 0xe8: case 0xf2: a = '.'; break;
            /* The launcher font is ASCII; use readable forms for name symbols. */
        case 0xef: a = 'M'; break;
        case 0xf0: a = '$'; break;
        case 0xf1: a = 'x'; break;
        case 0xf3: a = '/'; break;
        case 0xf4: a = ','; break;
        case 0xf5: a = 'F'; break;
        }
        if (used < 15u)output[used++] = a;
        if (b && used < 15u)output[used++] = b;
    }
    if (!ended)r->ok = 0;
    if (output)output[used] = 0;
    return letters;
}
static void SummaryMons(SummaryReader* r, unsigned count) {
    /* PGM1/PGM2 write 62 bytes per monster; no compiler struct layout. */
    for (unsigned i = 0; i < count; ++i) {
        unsigned species = SummaryGet(r, 1), level = SummaryGet(r, 1), status = SummaryGet(r, 1);
        if (!species || species > 190u || !level || level > 100u || status > 127u)r->ok = 0;
        SummarySkip(r, 59);
    }
}
static void SummaryItems(SummaryReader* r, unsigned maximum) {
    unsigned count = SummaryGet(r, 1);
    if (count > maximum) { r->ok = 0; return; }
    for (unsigned i = 0; i < count; ++i) {
        unsigned item = SummaryGet(r, 1), quantity = SummaryGet(r, 1);
        if (!item || !quantity || quantity > 99u)r->ok = 0;
    }
}
static int SummaryGame(const unsigned char* data, unsigned bytes, unsigned money, unsigned* badges) {
    SummaryReader r = { data, bytes, 0, 1 };
    unsigned format = SummaryGet(&r, 4);
    if (format != 0x314d4750u && format != 0x324d4750u)return 0;
    unsigned party = SummaryGet(&r, 1), box = SummaryGet(&r, 1), active = SummaryGet(&r, 1);
    if (party > 6u || box >= 12u || active > 5u || (party && active >= party))return 0;
    SummaryMons(&r, party);
    for (unsigned b = 0; b < 12u; ++b) {
        unsigned count = SummaryGet(&r, 1);
        if (count > 20u)return 0;
        SummaryMons(&r, count);
    }
    SummaryItems(&r, 20); SummaryItems(&r, 50);
    for (unsigned d = 0; d < 19u; ++d) {
        unsigned seen = SummaryGet(&r, 1), owned = SummaryGet(&r, 1);
        if ((owned & ~seen) || (d == 18u && ((seen | owned) & 128u)))r.ok = 0;
    }
    unsigned bits = SummaryGet(&r, 1), dex = SummaryGet(&r, 1), savedMoney = SummaryGet(&r, 3);
    unsigned coins = SummaryGet(&r, 2), repel = SummaryGet(&r, 2);
    SummarySkip(&r, 4);
    unsigned balls = SummaryGet(&r, 1), safariSteps = SummaryGet(&r, 2), hall = SummaryGet(&r, 2);
    unsigned bicycle = SummaryGet(&r, 1), surfing = SummaryGet(&r, 1);
    SummarySkip(&r, 2);
    if (dex > 1u || savedMoney != money || coins > 9999u || repel > 250u ||
        balls > 30u || safariSteps > 500u || hall > 50u || bicycle > 1u || surfing > 1u)return 0;
    if (format == 0x324d4750u) {
        unsigned total = SummaryGet(&r, 1), daycare = SummaryGet(&r, 1);
        if (total < hall || daycare > 1u)return 0;
        if (daycare) {
            unsigned level = SummaryGet(&r, 1);
            if (!level || level > 100u)return 0;
            SummarySkip(&r, 4); SummaryMons(&r, 1);
        }
        for (unsigned h = 0; h < hall; ++h) {
            unsigned count = SummaryGet(&r, 1);
            if (!count || count > 6u)return 0;
            for (unsigned m = 0; m < count; ++m) {
                unsigned species = SummaryGet(&r, 1), level = SummaryGet(&r, 1);
                if (!species || species > 190u || !level || level > 100u)return 0;
                SummaryName(&r, 10, NULL);
            }
        }
    }
    if (!r.ok || r.at != bytes)return 0;
    *badges = 0;
    for (unsigned i = 0; i < 8u; ++i)if (bits & (1u << i))++*badges;
    return 1;
}
static int SummaryWorld(const unsigned char* data, unsigned bytes, unsigned game) {
    if (bytes < 32u || bytes > 4096u ||
        ReadLE32(data + bytes - 4u) != ~SaveCRC(0xffffffffu, data, bytes - 4u))return 0;
    SummaryReader r = { data, bytes - 4u, 0, 1 };
    if (SummaryGet(&r, 4) != 0x31574e50u)return 0;
    unsigned version = SummaryGet(&r, 2), savedGame = SummaryGet(&r, 1), map = SummaryGet(&r, 1);
    if (version < 1u || version > 3u || game != savedGame || (version == 1u && map > 4u))return 0;
    if (SummaryGet(&r, 4) > 63u)return 0;
    SummarySkip(&r, 10);
    unsigned facing = SummaryGet(&r, 1), lock = SummaryGet(&r, 1);
    SummarySkip(&r, 2);
    unsigned actors = SummaryGet(&r, 1), reserved = SummaryGet(&r, 1);
    if (facing > 3u || lock > 1u || actors > 16u || reserved)return 0;
    /* PNW1 v2/v3: 320 event bytes, two 256-word object/trainer arrays,
       23 field bytes, 32 story bytes, and v3's eight hidden-item bytes. */
    if (version >= 2u)SummarySkip(&r, 320u + 1024u + 23u + 32u + (version == 3u ? 8u : 0u));
    SummarySkip(&r, actors * 16u);
    if (version >= 2u) {
        unsigned changes = SummaryGet(&r, 1);
        if (changes > 64u)return 0;
        SummarySkip(&r, changes * 3u);
    }
    return r.ok && r.at == r.bytes;
}
static int DecodeSaveSummary(const unsigned char* data, unsigned bytes, unsigned game,
    PbrbSaveSummary* out) {
    if (bytes < 100u)return 0;
    SummaryReader r = { data, bytes, 0, 1 };
    if (SummaryGet(&r, 4) != 0x31544250u)return 0;
    unsigned version = SummaryGet(&r, 2), savedGame = SummaryGet(&r, 1), reserved = SummaryGet(&r, 1);
    if ((version != 1u && version != 2u) || savedGame != game || reserved)return 0;
    unsigned frames = SummaryGet(&r, 4);
    SummarySkip(&r, 6);
    unsigned money = SummaryGet(&r, 4), speed = SummaryGet(&r, 1);
    unsigned animation = SummaryGet(&r, 1), style = SummaryGet(&r, 1);
    if (money > 999999u || speed > 2u || animation > 1u || style > 1u)return 0;
    if (version == 1u) {
        unsigned pc = SummaryGet(&r, 1), bag = SummaryGet(&r, 1);
        if (pc > 1u || bag > 1u || pc + bag > 1u || money != 3000u)return 0;
    }
    PbrbSaveSummary next; memset(&next, 0, sizeof(next));
    if (!SummaryName(&r, 7, next.trainer) || !SummaryName(&r, 7, NULL))return 0;
    unsigned starter = 0;
    if (version == 1u) {
        starter = SummaryGet(&r, 1);
        unsigned level = SummaryGet(&r, 1), status = SummaryGet(&r, 1);
        SummarySkip(&r, 10); SummaryName(&r, 10, NULL);
        unsigned hp = SummaryGet(&r, 2), maxHP = SummaryGet(&r, 2);
        SummarySkip(&r, 8);
        unsigned experience = SummaryGet(&r, 4);
        SummarySkip(&r, 2);
        if (starter) {
            if ((starter != 0x99u && starter != 0xb0u && starter != 0xb1u) ||
                level != 5u || status || hp > maxHP || maxHP < 15u || maxHP > 25u || experience != 135u)return 0;
        }
        else if (level || hp || maxHP)return 0;
    }
    else {
        unsigned length = SummaryGet(&r, 2);
        if (!r.ok || length > r.bytes - r.at || !SummaryGame(data + r.at, length, money, &next.badges))return 0;
        SummarySkip(&r, length);
    }
    unsigned length = SummaryGet(&r, 2);
    if (!r.ok || length != r.bytes - r.at || !SummaryWorld(data + r.at, length, game) ||
        (version == 1u && data[r.at + 24u] != starter))return 0;
    const unsigned cap = 255u * 60u * 60u * 60u;
    if (frames > cap)frames = cap;
    next.hours = frames / (60u * 60u * 60u); next.minutes = (frames / (60u * 60u)) % 60u;
    *out = next; return 1;
}
int PbrbReadSaveSummary(unsigned game, PbrbSaveSummary* summary) {
    if (summary)memset(summary, 0, sizeof(*summary));
    if (!summary || (game != 1u && game != 2u)) {
        s_saveError = "The save summary request is invalid."; return -1;
    }
    /* Static staging avoids a 64 KB allocation on the Xbox thread stack. */
    static unsigned char data[PBRB_SAVE_MAX_BYTES]; unsigned bytes = 0;
    if (!LoadSaveRecord(game, data, sizeof(data), &bytes))return s_saveError[0] ? -1 : 0;
    if (!DecodeSaveSummary(data, bytes, game, summary)) {
        s_saveError = "The saved adventure summary is unavailable."; return -1;
    }
    return 1;
}
