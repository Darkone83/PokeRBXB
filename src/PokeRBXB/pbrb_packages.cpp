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
static PbrbPreferences s_preferences = { 0, 0, 0 };
static char s_saveDir[MAX_PATH] = "";
static const char* s_saveError = "";
static const unsigned char kSaveMagic[8] = { 'P','B','R','B','S','A','V','E' };
enum { SAVE_HEADER_BYTES = 32, SAVE_FORMAT = 1 };
struct SaveRecord { unsigned generation, bytes, crc; };

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
    unsigned char header[SAVE_HEADER_BYTES], block[1024];
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
            if (output)memcpy((unsigned char*)output + done, block, count);
            done += count;
        }
    }
    CloseHandle(file);
    if (!ok || ~crc != record->crc)return -1;
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
    unsigned char data[3]; unsigned bytes = 0;
    s_preferences.smooth = s_preferences.rumble = s_preferences.color = 0;
    if (!LoadSaveRecord(0, data, sizeof(data), &bytes))return 0;
    if (bytes != sizeof(data) || data[0] > 1u || data[1] > 1u || data[2] > 1u) {
        s_saveError = "The saved options are invalid."; return 0;
    }
    s_preferences.smooth = data[0]; s_preferences.rumble = data[1]; s_preferences.color = data[2];
    return 1;
}
int PbrbSavePreferences(const PbrbPreferences* p) {
    if (!p) { s_saveError = "No options were supplied."; return 0; }
    unsigned char data[3];
    data[0] = (unsigned char)(p->smooth != 0); data[1] = (unsigned char)(p->rumble != 0);
    data[2] = (unsigned char)(p->color != 0);
    s_preferences.smooth = data[0]; s_preferences.rumble = data[1]; s_preferences.color = data[2];
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
