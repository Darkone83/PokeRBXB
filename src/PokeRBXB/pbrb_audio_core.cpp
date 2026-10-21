#include "pbrb_audio_core.h"
#include <string.h>

/* USA/Europe Red and Blue audio-bank locations, checked against both supplied
   ROMs. Streams and wave samples are read from the player's ROM, not invented.
   Reference: pokered/audio/engine_{1,2,3}.asm, home/pokemon.asm and
   data/pokemon/cries.asm. No ROM bank is substituted with synthesized songs. */
static const unsigned WORLD_BANK = 0x8000u;
static const unsigned BATTLE_BANK = 0x20000u;
static const unsigned TITLE_BANK = 0x7c000u;
static const unsigned TITLE_HEADER = 0x7c249u;
static const unsigned INTRO_HEADER = 0x7c294u;
static const unsigned CRY_DATA = 0x39446u;
struct MusicLocation { unsigned bank, header; };
static const MusicLocation kMusic[PBRB_MUSIC_COUNT] = {
    {0,0}, {TITLE_BANK,TITLE_HEADER}, {TITLE_BANK,INTRO_HEADER},
    {WORLD_BANK,0x82cdu}, {WORLD_BANK,0x822eu}, {WORLD_BANK,0x8291u},
    {TITLE_BANK,0x7c267u}, {WORLD_BANK,0x829au}, {BATTLE_BANK,0x202c7u},
    {WORLD_BANK,0x8237u}, {WORLD_BANK,0x82b8u}, {BATTLE_BANK,0x202e2u},
    {BATTLE_BANK,0x202d0u}, {BATTLE_BANK,0x202beu}, {BATTLE_BANK,0x202d9u},
    {BATTLE_BANK,0x202ebu}, {BATTLE_BANK,0x202f4u}, {WORLD_BANK,0x82afu},
    {TITLE_BANK,0x7c25eu}, {TITLE_BANK,0x7c255u}, {TITLE_BANK,0x7c276u},
    {TITLE_BANK,0x7c282u}, {TITLE_BANK,0x7c270u}, {WORLD_BANK,0x8240u},
    {WORLD_BANK,0x8249u}, {WORLD_BANK,0x8255u}, {WORLD_BANK,0x825eu},
    {WORLD_BANK,0x8267u}, {WORLD_BANK,0x8270u}, {WORLD_BANK,0x827cu},
    {WORLD_BANK,0x8288u}, {WORLD_BANK,0x82a3u}, {WORLD_BANK,0x82afu},
    {WORLD_BANK,0x82c1u}, {WORLD_BANK,0x82d9u}, {WORLD_BANK,0x82e5u},
    {WORLD_BANK,0x82f1u}, {TITLE_BANK,0x7c28bu},
    {TITLE_BANK,0x7c2a0u}, {TITLE_BANK,0x7c2acu}, {TITLE_BANK,0x7c2b8u},
    {TITLE_BANK,0x7c2c4u}, {TITLE_BANK,0x7c2d0u}, {TITLE_BANK,0x7c2d9u},
    {TITLE_BANK,0x7c2e2u}, {TITLE_BANK,0x7c2ebu}, {TITLE_BANK,0x7c2f4u}
};
static const unsigned kSfxHeader[PBRB_SFX_COUNT] = {
    0x1b0u,0x1adu,0x21cu,0x207u,0x21fu,0x1d4u,
    0x222u,0x192u,0x1a7u,0x1cbu,0x1ceu,0x2bbu,0x1b9u,0x1bcu,
    0x1f2u,0x1f5u,0x210u,0x1dau,0x1bfu,0x1b3u,0x1ceu,0x192u,0x1c8u,
    0x1c5u,0x1aau,0x19bu,0x1a4u,0x1c5u,0x1e6u,0x216u,0x1efu,
    0x204u,0x1ecu,0x1ddu,0x1e0u,0x1dau,0x228u,0x22bu,
    0x201u,0x20au,0x1d7u,0x1d1u,0x1f8u,0x1f5u,0x1fbu,
    0x23au,0x23du,0x240u,0u
};
static const unsigned UI_PRESS = 0x0462u;
static const unsigned UI_START = 0x03c3u;
static const unsigned char kPress[] = {
    0xec,2,0x20,0x91,0xc0,7,0x20,0x81,0xd0,7,
    0x20,0x91,0xc0,7,0x2c,0xa1,0xd0,7,0xff
};
static const unsigned char kStart[] = { 0x21,0xe2,0x33,0x28,0xe1,0x22,0xff };
struct IntroSfxLocation { unsigned offset, length; };
static const IntroSfxLocation kIntroSfx[7] = {
    {0x7c6beu,11u}, {0x7c6c9u,11u}, {0x7c6d4u,10u},
    {0x7c6deu,7u}, {0x7c6a5u,25u}, {0x7c6e5u,16u}, {0x7c72du,43u}
};
static const unsigned char kIntroSignatures[7][5] = {
    {0xec,2,0x10,0x26,0x2c}, {0xec,2,0x10,0x26,0x2c},
    {0x22,0x6f,33,0x22,0xaf}, {0x22,0xd2,50,0x2f,0xf2},
    {0x26,0x20,16,0x26,0x2f}, {0x24,0x2c,32,0x23,0xa0},
    {0xfc,0xe4,0x10,0x2f,0x24}
};
static unsigned Rd16(const unsigned char* p) {
    return (unsigned)p[0] | ((unsigned)p[1] << 8);
}
static unsigned MapPointer(unsigned bank, unsigned address) {
    return address >= 0x4000u && address < 0x8000u ? bank + address - 0x4000u : 0;
}
static int Within(const PbrbSynth* s, unsigned address, unsigned count) {
    return s->rom && (size_t)address <= s->romSize &&
        (size_t)count <= s->romSize - (size_t)address;
}
static void ResetVoice(PbrbVoice* v, unsigned channel) {
    memset(v, 0, sizeof(*v));
    v->channel = (unsigned char)channel;
    v->enabled = 1; v->remaining = 1; v->noteType = 1;
    v->loopRemaining = 1;
}
void PbrbSynthInit(PbrbSynth* s, int rate) {
    if (!s)return;
    memset(s, 0, sizeof(*s));
    s->sampleRate = rate > 0 && rate <= 192000 ? (unsigned)rate : PBRB_AUDIO_RATE;
    s->tempo = s->sfxTempo = 256; s->musicOn = 1; s->effectsOn = 1;
    s->panning = 0xff; s->masterVolume = 0x77;
    /* 70224 Game Boy CPU cycles per VBlank = 4389 / 262144 seconds.
       Start at a boundary; all four voices then share one sequencer clock. */
    s->frameClock = s->sampleRate * 4389u;
}
int PbrbSynthLoadMenuBank(PbrbSynth* s, const unsigned char* bank, int bytes) {
    if (!s || !bank || bytes < 16384)return 0;
    if (memcmp(bank + UI_PRESS, kPress, sizeof(kPress)) ||
        memcmp(bank + UI_START, kStart, sizeof(kStart)))return 0;
    memcpy(s->menuData[0], bank + UI_PRESS, sizeof(kPress));
    memcpy(s->menuData[1], bank + UI_START, sizeof(kStart));
    s->menuLength[0] = (unsigned char)sizeof(kPress);
    s->menuLength[1] = (unsigned char)sizeof(kStart);
    s->menuLoaded = 1;
    return 1;
}
void PbrbSynthStopTitle(PbrbSynth* s) {
    if (!s)return;
    s->songValid = 0; s->rom = 0; s->romSize = 0;
    s->musicId = PBRB_MUSIC_NONE; s->musicBank = s->musicHeader = 0; s->savedVolume = 0;
    s->fadeActive = 0; s->sequencerPause = 0; s->lowHealthAlarm = 0;
    memset(s->song, 0, sizeof(s->song));
    memset(s->effect, 0, sizeof(s->effect));
    memset(s->apu, 0, sizeof(s->apu));
}
void PbrbSynthPauseSequencer(PbrbSynth* s, unsigned frames) {
    if (s)s->sequencerPause = frames;
}
static int ReadHeader(const unsigned char* rom, size_t size, unsigned bank,
    unsigned header, int effect, unsigned pc[4]) {
    unsigned i, count, used = 0;
    if (!rom || bank > size || size - bank < 16384u ||
        header < bank || header >= bank + 16384u)return 0;
    count = (rom[header] >> 6) + 1u;
    if (header + count * 3u > bank + 16384u)return 0;
    for (i = 0; i < 4; ++i)pc[i] = 0;
    for (i = 0; i < count; ++i) {
        unsigned at = header + i * 3u, ch = rom[at] & 15u, pointer;
        const int flute = effect && bank == WORLD_BANK && header == 0x8228u;
        if ((rom[at] & 0x30u) || (i && (rom[at] & 0xc0u)) ||
            (flute ? ch != 2u : (ch < (effect ? 4u : 0u) || ch >= (effect ? 8u : 4u))))return 0;
        ch &= 3u;
        pointer = MapPointer(bank, Rd16(rom + at + 1));
        if (!pointer || (used & (1u << ch)))return 0;
        used |= 1u << ch; pc[ch] = pointer;
    }
    return 1;
}
static unsigned MusicTableStart(unsigned bank) {
    return bank == WORLD_BANK ? 0x22eu : bank == BATTLE_BANK ? 0x2beu :
        bank == TITLE_BANK ? 0x249u : 0u;
}
static int HeaderInTable(const PbrbSynth* s, unsigned bank, unsigned header, int effect) {
    unsigned begin = MusicTableStart(bank), pc[4];
    if (!begin || !Within(s, bank, 16384u))return 0;
    unsigned end = effect ? begin : 0x2fdu;
    unsigned at = effect ? 3u : begin;
    if (header < bank + at || header >= bank + end)return 0;
    while (at < end) {
        unsigned next = at + (((unsigned)s->rom[bank + at] >> 6) + 1u) * 3u;
        if (next > end)return 0;
        if (bank + at == header)return ReadHeader(s->rom, s->romSize, bank, header, effect, pc);
        at = next;
    }
    return 0;
}
static int RawLocation(const PbrbSynth* s, int id, int effect, MusicLocation* location) {
    if (((unsigned)id & 0xffff0000u) != 0x10000u)return 0;
    unsigned bank = ((unsigned)id >> 8) & 255u;
    if (bank != 2u && bank != 8u && bank != 31u)return 0;
    location->bank = bank * 16384u;
    location->header = location->bank + ((unsigned)id & 255u) * 3u;
    return HeaderInTable(s, location->bank, location->header, effect);
}
static int MusicForId(const PbrbSynth* s, int id, MusicLocation* location) {
    if (id >= 0 && id < PBRB_MUSIC_COUNT) {
        *location = kMusic[id];
        return id == PBRB_MUSIC_NONE || HeaderInTable(s, location->bank, location->header, 0);
    }
    return RawLocation(s, id, 0, location);
}
static int ReadWaves(const unsigned char* rom, size_t size, unsigned bank,
    unsigned char wave[9][32]) {
    if (!rom || bank > size || size - bank < 16384u)return 0;
    for (unsigned i = 0; i < 9; ++i) {
        unsigned at = MapPointer(bank, Rd16(rom + bank + 0x361u + i * 2u));
        if (!at || at + 16u > bank + 16384u)return 0;
        for (unsigned j = 0; j < 32; ++j) {
            unsigned b = rom[at + j / 2u];
            wave[i][j] = (unsigned char)((j & 1u) ? b & 15u : b >> 4);
        }
    }
    return 1;
}
static int BeginMusic(PbrbSynth* s, const unsigned char* rom, size_t size,
    unsigned bank, unsigned header, int music) {
    unsigned i, pc[4];
    unsigned char sfxFraction[4], songOctave[4], sfxOctave[4];
    unsigned char wave[9][32];
    if (!s || size != 1048576u || !ReadHeader(rom, size, bank, header, 0, pc) ||
        !ReadWaves(rom, size, bank, wave))return 0;
    s->rom = rom; s->romSize = size;
    for (i = 0; i < 4; ++i) {
        sfxFraction[i] = s->effect[i].fraction;
        songOctave[i] = s->song[i].octave; sfxOctave[i] = s->effect[i].octave;
    }
    memset(s->song, 0, sizeof(s->song));
    memset(s->effect, 0, sizeof(s->effect));
    memset(s->apu, 0, sizeof(s->apu));
    for (i = 0; i < 4; ++i) {
        s->effect[i].fraction = sfxFraction[i]; s->effect[i].octave = sfxOctave[i];
        s->song[i].octave = songOctave[i];
    }
    for (i = 0; i < 4; ++i) {
        if (!pc[i])continue;
        ResetVoice(&s->song[i], i);
        /* The ROM deliberately retains octave between songs. PkmnHealed's
           first channel relies on the preceding Pokecenter octave. */
        s->song[i].octave = songOctave[i];
        s->song[i].pc = pc[i]; s->song[i].bank = bank;
        s->song[i].soundId = (unsigned char)((header - bank) / 3u);
    }
    memcpy(s->wave, wave, sizeof(wave));
    PbrbSynthLoadMenuBank(s, rom + 0x8000u, 16384);
    s->tempo = s->sfxTempo = 256; s->panning = 0xff; s->masterVolume = 0x77;
    s->musicBank = bank; s->musicHeader = header; s->musicId = music; s->savedVolume = 0;
    s->fadeActive = 0; s->sequencerPause = 0;
    s->songValid = 1; s->decodeErrors = 0;
    return 1;
}
int PbrbSynthAttachIntro(PbrbSynth* s, const unsigned char* rom, size_t size) {
    unsigned i;
    if (!s || !rom || size != 1048576u)return 0;
    for (i = 0; i < 7; ++i) {
        unsigned o = kIntroSfx[i].offset, n = kIntroSfx[i].length;
        if (memcmp(rom + o, kIntroSignatures[i], 5) || rom[o + n - 1] != 0xff)return 0;
    }
    s->rom = rom; s->romSize = size; s->sequencerPause = 0;
    if (!s->musicBank)s->musicBank = TITLE_BANK;
    PbrbSynthLoadMenuBank(s, rom + 0x8000u, 16384);
    for (i = 0; i < 7; ++i) {
        memcpy(s->menuData[i + 2], rom + kIntroSfx[i].offset, kIntroSfx[i].length);
        s->menuLength[i + 2] = (unsigned char)kIntroSfx[i].length;
    }
    return 1;
}
int PbrbSynthStartIntroMusic(PbrbSynth* s) {
    if (!s || !Within(s, 0x7f844u, 10))return 0;
    if (memcmp(s->rom + 0x7f844u, "\xed\x00\x62\xf0\x77\xec\x03\xea\x06\x34", 10))return 0;
    return BeginMusic(s, s->rom, s->romSize, TITLE_BANK, INTRO_HEADER, PBRB_MUSIC_INTRO);
}
int PbrbSynthStartTitle(PbrbSynth* s, const unsigned char* rom, size_t size) {
    if (!s || !rom || size != 1048576u)return 0;
    if (memcmp(rom + 0x7e4c0u, "\xed\x00\x90\xf0\x77\xea\x09\x34\xec\x03", 10))return 0;
    return BeginMusic(s, rom, size, TITLE_BANK, TITLE_HEADER, PBRB_MUSIC_TITLE);
}
int PbrbSynthPlayMusic(PbrbSynth* s, int music) {
    MusicLocation location;
    if (!s || !MusicForId(s, music, &location))return 0;
    if (s->fadeActive) { s->fadeActive = 0; s->masterVolume = 0x77; }
    if (music == PBRB_MUSIC_NONE) {
        s->musicId = music; s->musicHeader = 0; s->songValid = 0;
        s->sequencerPause = 0;
        for (unsigned i = 0; i < 4; ++i) {
            unsigned char octave = s->song[i].octave;
            memset(&s->song[i], 0, sizeof(s->song[i])); s->song[i].octave = octave;
            if (s->effect[i].drum)memset(&s->effect[i], 0, sizeof(s->effect[i]));
            if (s->apu[i].owner != 2)s->apu[i].active = 0;
        }
        return 1;
    }
    if (s->songValid && s->musicBank == location.bank && s->musicHeader == location.header)
        for (unsigned i = 0; i < 4; ++i)if (s->song[i].enabled) { s->musicId = music; return 1; }
    return BeginMusic(s, s->rom, s->romSize, location.bank, location.header, music);
}
int PbrbSynthFadeMusic(PbrbSynth* s, int music, unsigned framesPerStep) {
    MusicLocation location;
    if (!s || framesPerStep > 255u || !MusicForId(s, music, &location))return 0;
    if (!framesPerStep || !s->songValid)return PbrbSynthPlayMusic(s, music);
    s->fadeCounter = s->fadeReload = framesPerStep;
    s->fadeTarget = music; s->fadeActive = 1;
    return 1;
}
static int CryActive(const PbrbSynth* s) {
    return s->effect[0].enabled && s->effect[0].soundId >= 20u &&
        s->effect[0].soundId < 134u;
}
static int FrequencyModified(const PbrbSynth* s, const PbrbVoice* v) {
    if (CryActive(s))return 1;
    if (v->bank != BATTLE_BANK)return 0;
    /* Audio2 tests the OR of square-1/noise sound IDs, including its quirks. */
    unsigned id = (s->effect[0].enabled ? s->effect[0].soundId : 0u) |
        (s->effect[3].enabled ? s->effect[3].soundId : 0u);
    return id >= 0x9du && id < 0xeau;
}
static int BeginSfx(PbrbSynth* s, unsigned bank, unsigned header) {
    unsigned pc[4], soundId;
    if (!ReadHeader(s->rom, s->romSize, bank, header, 1, pc))return 0;
    soundId = (header - bank) / 3u;
    if (bank == WORLD_BANK && header == 0x8228u) {
        /* The outdoor Pokeflute deliberately hijacks the music wave voice,
           rather than software SFX channel 7. It therefore keeps music
           tempo and uses the music note/vibrato path. */
        PbrbVoice* v = &s->song[2];
        if (v->enabled && soundId > v->soundId)return 0;
        unsigned char fraction = v->fraction, octave = v->octave;
        ResetVoice(v, 2); v->fraction = fraction; v->octave = octave;
        v->pc = pc[2]; v->bank = bank; v->soundId = (unsigned char)soundId;
        s->songValid = 1;
        return 1;
    }
    /* Original SFX priority: lower sound IDs win; drums never interrupt a
       foreground noise effect. Reset channels in the original reverse order. */
    for (int i = 3; i >= 0; --i)if (pc[i]) {
        PbrbVoice* v = &s->effect[i];
        if (v->enabled && !(i == 3 && v->drum) && soundId > v->soundId)return 0;
        unsigned char fraction = v->fraction, octave = v->octave;
        ResetVoice(v, (unsigned)i); v->enabled = 0;
        v->fraction = fraction; v->octave = octave;
        if (i == 0)s->apu[0].sweep = 8;
    }
    for (unsigned i = 0; i < 4; ++i)if (pc[i]) {
        PbrbVoice* v = &s->effect[i];
        v->enabled = 1; v->pc = pc[i]; v->bank = bank;
        v->soundId = (unsigned char)soundId;
    }
    if (soundId >= 20u && soundId < 134u) {
        unsigned ret = bank == WORLD_BANK ? 0x5b16u : (bank == BATTLE_BANK ? 0x62d5u : 0x5b8au);
        s->effect[2].channel = 2; s->effect[2].enabled = 1; s->effect[2].source = 0;
        if (!s->effect[2].remaining)s->effect[2].remaining = 1;
        s->effect[2].pc = MapPointer(bank, ret); s->effect[2].bank = bank;
        for (unsigned i = 0; i < 4; ++i) {
            s->effect[i].cry = 1; s->effect[i].soundId = (unsigned char)soundId;
        }
        if (!s->savedVolume)s->savedVolume = s->masterVolume;
        s->masterVolume = 0x77;
    }
    return 1;
}
int PbrbSynthPlaySfx(PbrbSynth* s, int effect) {
    /* Semantic hit/faint cues use the original call-site modifiers. Raw
       headers retain neutral modifiers for callers supplying their own. */
    if (effect == PBRB_SFX_DAMAGE)return PbrbSynthPlaySfxModified(s, effect, 0x20u, 0x30u);
    if (effect == PBRB_SFX_NOT_VERY_EFFECTIVE)return PbrbSynthPlaySfxModified(s, effect, 0x50u, 1u);
    if (effect == PBRB_SFX_SUPER_EFFECTIVE)return PbrbSynthPlaySfxModified(s, effect, 0xe0u, 255u);
    if (effect == PBRB_SFX_FAINT_FALL)return PbrbSynthPlaySfxModified(s, effect, 0u, 0u);
    return PbrbSynthPlaySfxModified(s, effect, 0u, 128u);
}
int PbrbSynthPlaySfxModified(PbrbSynth* s, int effect, unsigned frequency, unsigned tempo) {
    MusicLocation location;
    if (!s || !s->rom || frequency > 255u || tempo > 255u)return 0;
    if (effect == PBRB_SFX_STOP_ALL) {
        for (unsigned i = 0; i < 4; ++i) {
            const unsigned char songOctave = s->song[i].octave, songFraction = s->song[i].fraction;
            const unsigned char effectOctave = s->effect[i].octave, effectFraction = s->effect[i].fraction;
            memset(s->song + i, 0, sizeof(s->song[i]));
            memset(s->effect + i, 0, sizeof(s->effect[i]));
            s->song[i].octave = songOctave; s->song[i].fraction = songFraction;
            s->effect[i].octave = effectOctave; s->effect[i].fraction = effectFraction;
        }
        memset(s->apu, 0, sizeof(s->apu));
        s->songValid = 0; s->musicId = PBRB_MUSIC_NONE; s->musicHeader = 0;
        s->fadeActive = 0; s->sequencerPause = 0; s->savedVolume = 0; s->masterVolume = 0x77; s->panning = 0xff;
        s->tempo = s->sfxTempo = 256;
        return 1;
    }
    if (effect == PBRB_SFX_POKEFLUTE && s->musicBank == BATTLE_BANK) {
        /* Music_PokeFluteInBattle starts CAUGHT_MON and immediately replaces
           its three pointers. These are the original replacement streams. */
        if (!s->effectsOn)return 1;
        if (!BeginSfx(s, BATTLE_BANK, 0x201ceu))return 0;
        s->effect[0].pc = 0x22322u; s->effect[1].pc = 0x22325u;
        s->effect[2].pc = 0x2049bu;
        return 1;
    }
    if (effect >= 0 && effect < PBRB_SFX_COUNT) {
        unsigned bank = s->musicBank ? s->musicBank : WORLD_BANK;
        if (effect == PBRB_SFX_TRAINER_APPEARED || effect == PBRB_SFX_BALL_POOF ||
            (effect >= PBRB_SFX_DAMAGE && effect <= PBRB_SFX_RUN))bank = BATTLE_BANK;
        else if (effect == PBRB_SFX_POKEFLUTE || effect == PBRB_SFX_SAFARI_PA)bank = WORLD_BANK;
        else if (effect >= PBRB_SFX_SLOTS_STOP)bank = TITLE_BANK;
        else if (bank == BATTLE_BANK && effect != PBRB_SFX_PRESS && effect != PBRB_SFX_START_MENU &&
            effect != PBRB_SFX_HEAL && effect != PBRB_SFX_HEAL_AILMENT &&
            effect != PBRB_SFX_GET_ITEM2 && effect != PBRB_SFX_TINK)bank = WORLD_BANK;
        location.bank = bank; location.header = bank + kSfxHeader[effect];
        if (!HeaderInTable(s, bank, location.header, 1))return 0;
    }
    else if (!RawLocation(s, effect, 1, &location))return 0;
    if (!s->effectsOn)return 1;
    if (!BeginSfx(s, location.bank, location.header))return 0;
    s->cryFrequency = (unsigned char)frequency; s->cryTempo = (unsigned char)tempo;
    return 1;
}
int PbrbSynthPlayCry(PbrbSynth* s, unsigned species) {
    unsigned at, bank, id;
    if (!s || species < 1 || species > 190)return 0;
    at = CRY_DATA + (species - 1u) * 3u;
    if (!Within(s, at, 3) || s->rom[at] >= 38u)return 0;
    if (!s->effectsOn)return 1;
    bank = s->musicBank ? s->musicBank : WORLD_BANK;
    id = 20u + (unsigned)s->rom[at] * 3u;
    if (!BeginSfx(s, bank, bank + id * 3u))return 0;
    s->cryFrequency = s->rom[at + 1]; s->cryTempo = s->rom[at + 2];
    return 1;
}
void PbrbSynthSetMusicEnabled(PbrbSynth* s, int enabled) { if (s)s->musicOn = enabled != 0; }
void PbrbSynthSetEffectsEnabled(PbrbSynth* s, int enabled) { if (s)s->effectsOn = enabled != 0; }
void PbrbSynthSetLowHealth(PbrbSynth* s, int enabled) {
    if (!s)return;
    if (enabled) {
        if (!(s->lowHealthAlarm & 0x80u) || s->lowHealthAlarm == 0xffu)
            s->lowHealthAlarm = 0x80u;
    }
    else if (s->lowHealthAlarm & 0x80u)s->lowHealthAlarm = 0xffu;
}
int PbrbSynthEffectsBusy(const PbrbSynth* s) {
    if (!s || !s->effectsOn)return 0;
    /* home/delay.asm: WaitForSoundToFinish returns immediately while the
       repeating alarm owns CHAN5. It must never block gameplay forever. */
    if (s->lowHealthAlarm & 0x80u)return 0;
    if (s->song[2].enabled && s->song[2].bank == WORLD_BANK &&
        s->song[2].soundId == 0xb8u)return 1;
    for (unsigned i = 0; i < 4; ++i)
        if (s->effect[i].enabled && !s->effect[i].drum)return 1;
    return 0;
}
int PbrbSynthMusicBusy(const PbrbSynth* s) {
    if (!s || !s->songValid || !s->musicOn)return 0;
    for (unsigned i = 0; i < 4; ++i)if (s->song[i].enabled)return 1;
    return 0;
}
void PbrbSynthUi(PbrbSynth* s, int which) {
    static const unsigned char ids[9] = { 144,143,185,186,187,188,184,189,194 };
    unsigned channel;
    if (!s || !s->effectsOn || which < 0 || which > 8 || !s->menuLength[which])return;
    channel = (which == 0 || which == 2 || which == 3 || which == 8) ? 0u : 3u;
    unsigned char fraction = s->effect[channel].fraction, octave = s->effect[channel].octave;
    ResetVoice(&s->effect[channel], channel);
    s->effect[channel].fraction = fraction; s->effect[channel].octave = octave;
    s->effect[channel].source = (unsigned char)(which + 1);
    s->effect[channel].soundId = ids[which];
    if (channel == 0)s->apu[0].sweep = 8; /* original PlaySfx disables NR10 first */
}
/* Exact signed pitch-table arithmetic from Audio3_CalculateFrequency, also
   used by CppRed/common/calculate_frequency.h. GB octave numbers are not
   conventional concert-pitch octave numbers for the square channels. */
static unsigned NoteRegister(unsigned note, unsigned octave) {
    static const unsigned pitches[12] = {
        0xf82c,0xf89d,0xf907,0xf96b,0xf9ca,0xfa23,
        0xfa77,0xfac7,0xfb12,0xfb58,0xfb9b,0xfbda
    };
    unsigned p;
    if (note >= 12 || octave > 7)return 0;
    p = pitches[note];
    while (octave++ < 7)p = (p >> 1) | 0x8000u;
    return (p + 0x800u) & 0x7ffu;
}
static void SetFrequency(PbrbSynth* s, PbrbApuChannel* h, unsigned reg) {
    h->frequency = reg & 2047u;
    if (h->channel == 3) {
        static const unsigned divisor[8] = { 8,16,32,48,64,80,96,112 };
        unsigned period = divisor[reg & 7u] << ((reg >> 4) & 15u);
        h->step = (4194304ULL << 32) / ((unsigned long long)period * s->sampleRate);
        h->noiseMode = (unsigned char)((reg & 8u) != 0);
    }
    else {
        unsigned clock = h->channel == 2 ? 65536u : 131072u;
        h->step = ((unsigned long long)clock << 32) /
            ((unsigned long long)(2048u - h->frequency) * s->sampleRate);
    }
}
static void Trigger(PbrbSynth* s, PbrbVoice* v, unsigned reg, unsigned env, int effect) {
    PbrbApuChannel* h = &s->apu[v->channel];
    h->channel = v->channel; h->owner = (unsigned char)(effect ? (v->drum ? 3 : 2) : 1);
    if (!effect && v->bank == WORLD_BANK && v->soundId == 0xb8u)h->owner = 2;
    h->duty = v->duty & 3u; h->waveInstrument = v->waveInstrument;
    h->panning = (unsigned char)((s->panning >> v->channel) & 0x11u);
    if (effect && v->channel != 3)h->panning = 0x11;
    h->envelope = (unsigned char)env;
    h->volume = (unsigned char)(env >> 4);
    h->envelopeTimer = (unsigned char)((env & 7u) ? env & 7u : 8u);
    h->envelopeEnabled = (unsigned char)((env & 7u) != 0);
    h->active = (unsigned char)((env & 0xf8u) != 0);
    if (v->channel == 2) {
        h->volume = v->volume; h->active = (unsigned char)(v->volume != 0);
        h->phase = 0; h->envelopeEnabled = 0;
        unsigned at = 0;
        if (Within(s, v->bank + 0x361u + (unsigned)v->waveInstrument * 2u, 2))
            at = MapPointer(v->bank, Rd16(s->rom + v->bank + 0x361u + (unsigned)v->waveInstrument * 2u));
        if (at && Within(s, at, 16))for (unsigned i = 0; i < 32; ++i) {
            unsigned b = s->rom[at + i / 2u];
            h->waveData[i] = (unsigned char)((i & 1u) ? b & 15u : b >> 4);
        }
        else memcpy(h->waveData, s->wave[v->waveInstrument], 32);
    }
    if (v->channel == 3)h->noise = 0x7fffu;
    SetFrequency(s, h, reg);
    if (v->channel == 0) {
        unsigned shift = h->sweep & 7u, period = (h->sweep >> 4) & 7u;
        h->sweepShadow = h->frequency;
        h->sweepTimer = (unsigned char)(period ? period : 8);
        h->sweepEnabled = (unsigned char)(period || shift);
        if (shift && !(h->sweep & 8u) && h->sweepShadow + (h->sweepShadow >> shift) > 2047u)
            h->active = 0;
    }
}
static void Duration(PbrbSynth* s, PbrbVoice* v, unsigned count, int effect) {
    unsigned n = (count * v->noteType) & 255u;
    unsigned tempo = s->tempo;
    if (effect) {
        /* Noise uses fixed 256. The square/wave channels use the original
           cry modifier; Audio2 also applies it to the battle-SFX range. */
        tempo = 256u;
        if (v->channel != 3) {
            s->sfxTempo = FrequencyModified(s, v) ? (unsigned)s->cryTempo + 128u : 256u;
            tempo = s->sfxTempo;
        }
    }
    unsigned t = n * tempo + v->fraction;
    v->fraction = (unsigned char)t;
    v->remaining = (t >> 8) & 255u;
    /* A zero delay wraps to 255 on the next update in the original engine. */
    if (!v->remaining)v->remaining = 256;
}
static int Byte(PbrbSynth* s, PbrbVoice* v, unsigned* value) {
    if (v->source) {
        unsigned i = v->source - 1u;
        if (i >= 9 || v->pc >= s->menuLength[i])return 0;
        *value = s->menuData[i][v->pc++];
    }
    else {
        if (!Within(s, v->pc, 1) || v->pc < v->bank || v->pc >= v->bank + 16384u)return 0;
        *value = s->rom[v->pc++];
    }
    return 1;
}
static int Word(PbrbSynth* s, PbrbVoice* v, unsigned* value) {
    unsigned lo, hi;
    if (!Byte(s, v, &lo) || !Byte(s, v, &hi))return 0;
    *value = lo | (hi << 8); return 1;
}
static void StartDrum(PbrbSynth* s, unsigned bank, unsigned instrument) {
    unsigned at, pc;
    PbrbVoice* v = &s->effect[3];
    /* Music drums are SFX channel 8 in the ROM. An in-flight effect has
       priority; its noise-note sequence must not be replaced by a fake tone. */
    if (v->enabled || instrument < 1 || instrument > 19)return;
    at = bank + instrument * 3u;
    if (!Within(s, at, 3) || s->rom[at] != 7)return;
    pc = MapPointer(bank, Rd16(s->rom + at + 1));
    if (!pc)return;
    ResetVoice(v, 3); v->pc = pc; v->bank = bank; v->drum = 1;
    v->soundId = (unsigned char)instrument;
}
static unsigned InitSlide(PbrbVoice* v, unsigned frequency) {
    int delta = (int)frequency - (int)v->slideTarget;
    unsigned length = v->remaining >= v->slideLength ? v->remaining - v->slideLength : 1u;
    unsigned remainder, quotient;
    if (!length)length = 1;
    v->slideCurrent = frequency;
    v->slideDecreasing = (unsigned char)(delta >= 0);
    if (delta < 0) {
        delta = -delta;
        /* Preserve Audio3's documented wrong-high-byte borrow. */
        if ((frequency & 255u) > (v->slideTarget & 255u))delta += 512;
    }
    quotient = (unsigned)delta / length + 1u;
    remainder = (unsigned)delta % length;
    v->slideStep = (unsigned char)quotient;
    v->slideFraction = v->slideAccumulator = (unsigned char)remainder;
    /* The ROM also leaves DE holding its division workspace for the first
       trigger. Subsequent VBlanks use slideCurrent. CppRed models this too. */
    return (((quotient & 255u) << 8) | ((remainder - length) & 255u)) & 2047u;
}
static void Fetch(PbrbSynth* s, PbrbVoice* v, int effect) {
    unsigned iter;
    v->vibratoWait = v->vibratoDelay; v->slideActive = 0;
    /* Audio2_PlayNextNote leaves the first SFX channel suspended while
       the register-driven alarm is active. Other channels keep running. */
    if (effect && !v->channel && (s->lowHealthAlarm & 0x80u))return;
    for (iter = 0; iter < 256; ++iter) {
        unsigned op, a, b, pointer;
        if (!Byte(s, v, &op))break;
        if (op == 0xffu) {
            if (v->callDepth) { v->pc = v->callReturn[--v->callDepth]; continue; }
            if (effect && v->channel == 2)s->apu[2].active = 0;
            if (effect && CryActive(s)) {
                if (v->channel != 0) { --v->pc; return; }
                s->masterVolume = s->savedVolume; s->savedVolume = 0;
            }
            v->enabled = 0;
            if (!effect && v->channel < 3 && !s->effect[v->channel].enabled)
                s->apu[v->channel].active = 0;
            /* Sound effects leave their hardware envelope ringing out. */
            return;
        }
        if (op == 0xfdu) {
            if (!Word(s, v, &a) || v->callDepth >= 8 || !(pointer = MapPointer(v->bank, a)))break;
            v->callReturn[v->callDepth++] = v->pc; v->pc = pointer; continue;
        }
        if (op == 0xfeu) {
            if (!Byte(s, v, &a) || !Word(s, v, &b) || !(pointer = MapPointer(v->bank, b)))break;
            if (!a) { v->pc = pointer; continue; }
            /* There is one ascending loop counter per software channel.
               SFX_Battle_36 jumps from channel 6 into channel 5's loop;
               resetting the counter when the loop address changes is wrong. */
            if (v->loopRemaining != a) { ++v->loopRemaining; v->pc = pointer; }
            else v->loopRemaining = 1;
            continue;
        }
        if (op >= 0xd0u && op <= 0xdfu) {
            v->noteType = (unsigned char)(op & 15u);
            if (v->channel != 3) {
                if (!Byte(s, v, &a))break;
                v->volume = (unsigned char)(a >> 4); v->envelope = (unsigned char)(a & 15u);
                if (v->channel == 2) {
                    v->volume &= 3u; v->waveInstrument = (unsigned char)(a & 15u);
                    if (v->waveInstrument >= 9)break;
                }
            }
            continue;
        }
        if (op >= 0xe0u && op <= 0xe7u) { v->octave = (unsigned char)(op & 7u); continue; }
        if (op == 0xe8u) { v->perfectPitch ^= 1; continue; }
        if (op == 0xeau) {
            if (!Byte(s, v, &a) || !Byte(s, v, &b))break;
            v->vibratoDelay = v->vibratoWait = (unsigned char)a;
            v->vibratoDepth = (unsigned char)(b >> 4);
            v->vibratoRate = v->vibratoCounter = (unsigned char)(b & 15u); continue;
        }
        if (op == 0xebu) {
            if (!Byte(s, v, &a) || !Byte(s, v, &b))break;
            v->slideLength = (unsigned char)a;
            v->slideTarget = NoteRegister(b & 15u, b >> 4);
            v->slideActive = 1; continue;
        }
        if (op == 0xecu || op == 0xfcu) {
            if (!Byte(s, v, &a))break;
            if (op == 0xfcu) {
                v->dutyPattern = (unsigned char)a; v->rotateDuty = 1; v->duty = (unsigned char)(a >> 6);
            }
            else v->duty = (unsigned char)(a & 3u);
            continue;
        }
        if (op == 0xedu) {
            if (!Byte(s, v, &a) || !Byte(s, v, &b))break;
            if (effect) {
                s->sfxTempo = (a << 8) | b;
                for (unsigned k = 0; k < 4; ++k)s->effect[k].fraction = 0;
            }
            else {
                s->tempo = (a << 8) | b;
                for (unsigned k = 0; k < 4; ++k)s->song[k].fraction = 0;
            }
            continue;
        }
        if (op == 0xeeu || op == 0xf0u) {
            if (!Byte(s, v, &a))break;
            if (op == 0xeeu)s->panning = (unsigned char)a;
            else s->masterVolume = (unsigned char)a;
            continue;
        }
        if (op == 0xf8u) { v->executeMusic = 1; continue; }
        if (effect && !v->executeMusic && op == 0x10u) {
            if (!Byte(s, v, &a))break;
            s->apu[0].sweep = (unsigned char)a; continue;
        }
        if (effect && !v->executeMusic && (op & 0xf0u) == 0x20u) {
            if (!Byte(s, v, &a))break;
            if (v->channel == 3) { if (!Byte(s, v, &b))break; }
            else if (!Word(s, v, &b))break;
            Duration(s, v, (op & 15u) + 1u, 1);
            if (FrequencyModified(s, v))b = (b + s->cryFrequency) & (v->channel == 3 ? 255u : 2047u);
            v->frequency = b; v->lastNote = (unsigned char)op; ++v->noteSerial;
            Trigger(s, v, b, a, 1); return;
        }
        if (!effect && v->channel == 3 && op >= 0xb0u && op <= 0xbfu) {
            if (!Byte(s, v, &a))break;
            Duration(s, v, (op & 15u) + 1u, 0);
            v->lastNote = (unsigned char)a; ++v->noteSerial;
            if (s->musicOn)StartDrum(s, v->bank, a);
            return;
        }
        if ((!effect || v->executeMusic) && op <= 0xcfu) {
            Duration(s, v, (op & 15u) + 1u, effect);
            v->lastNote = (unsigned char)op; ++v->noteSerial;
            if (op >= 0xc0u) {
                if (v->channel != 3 && (effect || !s->effect[v->channel].enabled))s->apu[v->channel].active = 0;
                return;
            }
            if (v->channel == 3)break;
            a = NoteRegister(op >> 4, v->octave);
            if (v->slideActive)a = InitSlide(v, a);
            /* INC E does not set carry on the original CPU. Preserve its
               low-byte-only increment, including the healing-jingle wrap. */
            if (v->perfectPitch)a = (a & 0x700u) | ((a + 1u) & 255u);
            v->frequency = a;
            if (effect && v->bank == BATTLE_BANK && FrequencyModified(s, v))
                a = (a + s->cryFrequency) & 2047u;
            if (effect || !s->effect[v->channel].enabled)
                Trigger(s, v, a, (v->volume << 4) | v->envelope, effect);
            return;
        }
        break; /* Unsupported data is a diagnosable failure, never fake audio. */
    }
    ++s->decodeErrors; v->enabled = 0;
    if (!effect && !s->effect[v->channel].enabled)s->apu[v->channel].active = 0;
}
static void EffectsTick(PbrbSynth* s, PbrbVoice* v, int effect) {
    PbrbApuChannel* h = &s->apu[v->channel];
    if (!effect && s->effect[v->channel].enabled)return;
    if (v->rotateDuty) {
        v->dutyPattern = (unsigned char)((v->dutyPattern << 2) | (v->dutyPattern >> 6));
        h->duty = (unsigned char)(v->dutyPattern >> 6);
    }
    if ((effect && !v->executeMusic) || v->channel == 3)return;
    if (v->slideActive) {
        unsigned next;
        if (v->slideDecreasing) {
            /* Audio3 doubles its fractional step and borrows its carry. */
            unsigned doubled = (unsigned)v->slideFraction * 2u;
            v->slideFraction = (unsigned char)doubled;
            next = (v->slideCurrent - v->slideStep - (doubled >> 8)) & 0xffffu;
            if (next < v->slideTarget || next > 2047u) { v->slideActive = 0; return; }
        }
        else {
            unsigned sum = (unsigned)v->slideAccumulator + v->slideFraction;
            v->slideAccumulator = (unsigned char)sum;
            next = v->slideCurrent + v->slideStep + (sum >> 8);
            if (next > v->slideTarget) { v->slideActive = 0; return; }
        }
        v->slideCurrent = next; SetFrequency(s, h, next); return;
    }
    if (v->vibratoWait) { --v->vibratoWait; return; }
    if (!v->vibratoDepth)return;
    if (v->vibratoCounter) { --v->vibratoCounter; return; }
    v->vibratoCounter = v->vibratoRate;
    int low = (int)(v->frequency & 255u);
    if (v->vibratoDirection)low -= v->vibratoDepth / 2u;
    else low += (v->vibratoDepth + 1u) / 2u;
    v->vibratoDirection ^= 1;
    if (low < 0)low = 0;
    if (low > 255)low = 255;
    SetFrequency(s, h, (h->frequency & 0x700u) | (unsigned)low);
}
static void LowHealthTick(PbrbSynth* s) {
    if (!s->lowHealthAlarm)return;
    PbrbApuChannel* h = &s->apu[0];
    if (s->lowHealthAlarm == 0xffu) {
        s->lowHealthAlarm = 0; s->effect[0].enabled = 0; s->effect[0].soundId = 0;
        h->channel = 0; h->owner = 4; h->active = 0; h->volume = 0;
        h->envelope = h->envelopeEnabled = h->sweep = h->sweepEnabled = 0;
        h->duty = 0; SetFrequency(s, h, 0); return;
    }
    if (s->musicBank != BATTLE_BANK)return;
    unsigned timer = s->lowHealthAlarm & 0x7fu;
    if (!timer || timer == 20u) {
        /* Exact NR10..NR14 writes from audio/low_health_alarm.asm:
           $00,$a0,$e2,$50,$87 and $00,$b0,$e2,$ee,$86.
           The duty is 50%; pulse phase and NR51 panning are retained. */
        h->channel = 0; h->owner = 4; h->duty = 2; h->active = 1;
        h->sweep = h->sweepEnabled = 0; h->volume = 14;
        h->envelope = 0xe2; h->envelopeTimer = 2; h->envelopeEnabled = 1;
        SetFrequency(s, h, timer ? 0x6eeu : 0x750u);
    }
    if (!timer)timer = 30;
    else {
        /* The original reserves CHAN5 with CRY_SFX_END on the intervening
           frames, then decrements the timer. Preserve that ordering. */
        s->effect[0].enabled = 1; s->effect[0].channel = 0;
        s->effect[0].soundId = 134; s->effect[0].drum = 0;
        if (!s->effect[0].remaining)s->effect[0].remaining = 1;
        --timer;
    }
    s->lowHealthAlarm = (unsigned char)(0x80u | timer);
}
static void SequencerTick(PbrbSynth* s) {
    unsigned i;
    /* home/fade_audio.asm runs before the ROM's eight sequencer channels. */
    if (s->fadeActive) {
        if (s->fadeCounter)--s->fadeCounter;
        else {
            s->fadeCounter = s->fadeReload;
            if (!s->masterVolume) {
                int next = s->fadeTarget;
                s->fadeActive = 0;
                for (i = 0; i < 4; ++i) {
                    unsigned char octave = s->effect[i].octave, fraction = s->effect[i].fraction;
                    memset(&s->effect[i], 0, sizeof(s->effect[i]));
                    s->effect[i].octave = octave; s->effect[i].fraction = fraction;
                }
                memset(s->apu, 0, sizeof(s->apu));
                s->masterVolume = 0x77; s->savedVolume = 0;
                PbrbSynthPlayMusic(s, next);
            }
            else {
                unsigned left = (s->masterVolume >> 4) & 7u, right = s->masterVolume & 7u;
                if (left)--left;
                if (right)--right;
                s->masterVolume = (unsigned char)((left << 4) | right);
            }
        }
    }
    LowHealthTick(s);
    ++s->tick;
    if (s->songValid)for (i = 0; i < 4; ++i) {
        PbrbVoice* v = &s->song[i];
        if (!v->enabled)continue;
        if (v->remaining <= 1)Fetch(s, v, 0);
        else { --v->remaining; EffectsTick(s, v, 0); }
    }
    for (i = 0; i < 4; ++i) {
        PbrbVoice* v = &s->effect[i];
        if (!v->enabled)continue;
        if (v->remaining <= 1)Fetch(s, v, 1);
        else { --v->remaining; EffectsTick(s, v, 1); }
    }
}
static void ApuTick(PbrbSynth* s) {
    unsigned i;
    /* Hardware envelope clock is 64 Hz, independent of 59.7275 Hz VBlank. */
    if (s->apuFrame == 7)for (i = 0; i < 4; ++i) {
        PbrbApuChannel* h = &s->apu[i];
        if (!h->active || !h->envelopeEnabled)continue;
        if (h->envelopeTimer)--h->envelopeTimer;
        if (h->envelopeTimer)continue;
        h->envelopeTimer = h->envelope & 7u;
        if (h->envelope & 8u) {
            if (h->volume < 15)++h->volume; else h->envelopeEnabled = 0;
        }
        else {
            if (h->volume)--h->volume; else h->envelopeEnabled = 0;
        }
    }
    if (s->apuFrame == 2 || s->apuFrame == 6) {
        PbrbApuChannel* h = &s->apu[0];
        unsigned period = (h->sweep >> 4) & 7u, shift = h->sweep & 7u;
        if (h->sweepTimer)--h->sweepTimer;
        if (!h->sweepTimer) {
            h->sweepTimer = (unsigned char)(period ? period : 8);
            if (h->sweepEnabled && period && shift) {
                unsigned delta = h->sweepShadow >> shift;
                unsigned next = (h->sweep & 8u) ? h->sweepShadow - delta : h->sweepShadow + delta;
                if (next > 2047)h->active = 0;
                else {
                    h->sweepShadow = next; SetFrequency(s, h, next);
                    if (!(h->sweep & 8u) && next + (next >> shift) > 2047)h->active = 0;
                }
            }
        }
    }
    s->apuFrame = (s->apuFrame + 1u) & 7u;
}
static int SampleChannel(PbrbSynth*, PbrbApuChannel* h) {
    unsigned long long total;
    if (!h->active)return 0;
    total = (unsigned long long)h->phase + h->step;
    h->phase = (unsigned)total;
    if (h->channel == 3) {
        unsigned steps = (unsigned)(total >> 32);
        while (steps--) {
            unsigned bit = (h->noise ^ (h->noise >> 1)) & 1u;
            h->noise = (h->noise >> 1) | (bit << 14);
            if (h->noiseMode)h->noise = (h->noise & ~64u) | (bit << 6);
        }
        return (h->noise & 1u) ? -(int)h->volume : (int)h->volume;
    }
    if (h->channel == 2) {
        unsigned shift = h->volume ? h->volume - 1u : 4u;
        unsigned sample = h->waveData[h->phase >> 27];
        return (int)((sample >> shift) * 2u) - (15 >> shift);
    }
    /* Hardware duty sequences (rather than restarting a sine or triangle).
       Pulse phase is retained on retrigger; wave phase resets in Trigger. */
    static const unsigned char duty[4] = { 0x01,0x81,0x87,0x7e };
    return (duty[h->duty] & (0x80u >> (h->phase >> 29))) ? (int)h->volume : -(int)h->volume;
}
void PbrbSynthRender(PbrbSynth* s, short* out, int frames) {
    int i;
    if (!out || frames <= 0)return;
    if (!s) { memset(out, 0, (size_t)frames * 4u); return; }
    for (i = 0; i < frames; ++i) {
        int mix[2] = { 0,0 }; unsigned k;
        if (s->frameClock >= s->sampleRate * 4389u) {
            s->frameClock -= s->sampleRate * 4389u;
            if (s->sequencerPause)--s->sequencerPause;
            else SequencerTick(s);
        }
        s->frameClock += 262144u;
        s->apuClock += 512u;
        if (s->apuClock >= s->sampleRate) { s->apuClock -= s->sampleRate; ApuTick(s); }
        for (k = 0; k < 4; ++k) {
            PbrbApuChannel* h = &s->apu[k];
            int a = SampleChannel(s, h);
            const int effect = h->owner == 2 || h->owner == 4;
            if ((effect && !s->effectsOn) || (!effect && !s->musicOn))a = 0;
            if (h->panning & 0x10u)mix[0] += a;
            if (h->panning & 1u)mix[1] += a;
        }
        for (k = 0; k < 2; ++k) {
            unsigned level = ((s->masterVolume >> (k ? 0 : 4)) & 7u) + 1u;
            int in = mix[k] * (int)level * 56;
            /* DMG-like DC removal. Keep integer arithmetic for older RXDK.
               Feedback factor 65275/65536 ~= CppRed's 0.996013 at 44.1 kHz. */
            int value = in - s->filterIn[k] + (int)(((long long)s->filterOut[k] * 65275) / 65536);
            s->filterIn[k] = in; s->filterOut[k] = value;
            if (value > 32767)value = 32767;
            if (value < -32768)value = -32768;
            out[i * 2 + k] = (short)value;
        }
    }
}
