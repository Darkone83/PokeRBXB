#include <xtl.h>
#include <dsound.h>
#include "pbrb_audio.h"
#include "pbrb_audio_core.h"
#include "pbrb_rng.h"
#include "xpd_reader.h"
#include <string.h>
#include <stdlib.h>

/* Four-chunk DirectSound ring based on the proven XBSCPinball pattern.
   Keep all XTL and DirectSound dependencies here; the ROM synthesizer is
   platform independent and testable on a PC. */
#define PBRB_FRAMES_PER_CHUNK 1024u
#define PBRB_RING_CHUNKS 4u
#define PBRB_BYTES_PER_CHUNK (PBRB_FRAMES_PER_CHUNK*4u)
#define PBRB_RING_BYTES (PBRB_BYTES_PER_CHUNK*PBRB_RING_CHUNKS)
static LPDIRECTSOUND s_ds = NULL;
static LPDIRECTSOUNDBUFFER s_buffer = NULL;
static HANDLE s_thread = NULL, s_event = NULL;
static CRITICAL_SECTION s_lock;
static int s_lockValid = 0, s_ready = 0;
static int s_outputVolume = 10, s_outputPaused = 0;
static DWORD s_nextChunk = 0;
static volatile DWORD s_lastRenderTick = 0;
static PbrbSynth s_synth;
/* The launcher retains its own ROM only while it owns the synthesizer.
   Gameplay attaches an independently owned ROM after this one is released. */
static unsigned char* s_launcherRom = 0;
static int s_launcherTrack = -1, s_lastLauncherTrack = -1;
struct LauncherTrack { int music; const char* name; };
static const LauncherTrack kLauncherTracks[] = {
    {PBRB_MUSIC_TITLE, "Title Screen"},
    {PBRB_MUSIC_PALLET, "Pallet Town"},
    {PBRB_MUSIC_LAB, "Professor Oak's Laboratory"},
    {PBRB_MUSIC_OAK_SPEECH, "Routes 24 & 25"},
    {PBRB_MUSIC_POKECENTER, "Pokemon Center"},
    {PBRB_MUSIC_TRAINER_BATTLE, "Trainer Battle"},
    {PBRB_MUSIC_WILD_BATTLE, "Wild Pokemon Battle"},
    {PBRB_MUSIC_GYM_BATTLE, "Gym Leader Battle"},
    {PBRB_MUSIC_CHAMPION_BATTLE, "Champion Battle"},
    {PBRB_MUSIC_HALL_OF_FAME, "Hall of Fame"},
    {PBRB_MUSIC_BIKE, "Bicycle"},
    {PBRB_MUSIC_SURFING, "Surfing"},
    {PBRB_MUSIC_GYM, "Pokemon Gym"},
    {PBRB_MUSIC_CITIES1, "Viridian, Pewter & Saffron City"},
    {PBRB_MUSIC_CITIES2, "Cerulean & Fuchsia City"},
    {PBRB_MUSIC_CELADON, "Celadon City"},
    {PBRB_MUSIC_CINNABAR, "Cinnabar Island"},
    {PBRB_MUSIC_VERMILION, "Vermilion City"},
    {PBRB_MUSIC_LAVENDER, "Lavender Town"},
    {PBRB_MUSIC_SS_ANNE, "S.S. Anne"},
    {PBRB_MUSIC_SAFARI, "Safari Zone"},
    {PBRB_MUSIC_ROUTES1, "Routes 1 & 2"},
    {PBRB_MUSIC_ROUTES3, "Routes 3-10 & 16-22"},
    {PBRB_MUSIC_ROUTES4, "Routes 11-15"},
    {PBRB_MUSIC_INDIGO_PLATEAU, "Indigo Plateau"},
    {PBRB_MUSIC_GAME_CORNER, "Game Corner"},
    {PBRB_MUSIC_DUNGEON1, "Rocket Hideout"},
    {PBRB_MUSIC_DUNGEON2, "Viridian Forest"},
    {PBRB_MUSIC_DUNGEON3, "Mt. Moon"},
    {PBRB_MUSIC_MANSION, "Pokemon Mansion"},
    {PBRB_MUSIC_POKEMON_TOWER, "Pokemon Tower"},
    {PBRB_MUSIC_SILPH_CO, "Silph Co."}
};
/* Call with the audio lock held. Detach before freeing any byte the worker
   can still read; do not disturb a separately attached gameplay ROM. */
static void ReleaseLauncherMusic() {
    if (s_launcherRom) {
        if (s_synth.rom == s_launcherRom)PbrbSynthStopTitle(&s_synth);
        free(s_launcherRom); s_launcherRom = 0;
    }
    s_launcherTrack = -1;
}
/* Correct scratch size is 1024*2 samples, not 1024. */
static short s_stereo[PBRB_FRAMES_PER_CHUNK * 2u];

static void FillChunk(void* ptr, DWORD bytes) {
    if (!ptr || !bytes)return;
    /* Device Lock can wrap, and each region can be shorter than a chunk. */
    short* dst = (short*)ptr;
    unsigned frames = (unsigned)bytes / 4u;
    while (frames) {
        unsigned n = frames > PBRB_FRAMES_PER_CHUNK ? PBRB_FRAMES_PER_CHUNK : frames;
        /* A platform pause freezes both sequencer and oscillator state.
           Muting/volume, in contrast, never changes game sound-wait timing. */
        if (s_outputPaused)memset(s_stereo, 0, n * 4u);
        else {
            PbrbSynthRender(&s_synth, s_stereo, (int)n);
            if (s_outputVolume < 10)for (unsigned i = 0; i < n * 2u; ++i)
                s_stereo[i] = (short)((int)s_stereo[i] * s_outputVolume / 10);
        }
        s_lastRenderTick = GetTickCount();
        memcpy(dst, s_stereo, n * 4u);
        dst += n * 2u; frames -= n;
    }
}
static DWORD WINAPI AudioWorker(LPVOID) {
    while (WaitForSingleObject(s_event, 2) == WAIT_TIMEOUT) {
        DWORD play = 0, write = 0;
        if (!s_buffer || FAILED(s_buffer->GetCurrentPosition(&play, &write)))continue;
        DWORD playing = play / PBRB_BYTES_PER_CHUNK;
        unsigned guard = 0;
        while (s_nextChunk != playing && guard++ < PBRB_RING_CHUNKS) {
            const DWORD off = s_nextChunk * PBRB_BYTES_PER_CHUNK;
            void* p1 = 0, * p2 = 0; DWORD b1 = 0, b2 = 0;
            if (FAILED(s_buffer->Lock(off, PBRB_BYTES_PER_CHUNK, &p1, &b1, &p2, &b2, 0)))break;
            EnterCriticalSection(&s_lock);
            FillChunk(p1, b1); FillChunk(p2, b2);
            LeaveCriticalSection(&s_lock);
            s_buffer->Unlock(p1, b1, p2, b2);
            s_nextChunk = (s_nextChunk + 1) % PBRB_RING_CHUNKS;
        }
    }
    return 0;
}
void PbrbAudioShutdown() {
    s_ready = 0;
    if (s_event)SetEvent(s_event);
    if (s_thread) {
        WaitForSingleObject(s_thread, 2000);
        CloseHandle(s_thread); s_thread = NULL;
    }
    PbrbAudioStopTitle();
    if (s_event) { CloseHandle(s_event); s_event = NULL; }
    if (s_buffer) { s_buffer->Stop(); s_buffer->Release(); s_buffer = NULL; }
    if (s_ds) { s_ds->Release(); s_ds = NULL; }
    if (s_lockValid) { DeleteCriticalSection(&s_lock); s_lockValid = 0; }
    s_ready = 0;
}
int PbrbAudioInit() {
    HRESULT hr;
    if (s_ready)return 1;
    PbrbSynthInit(&s_synth, PBRB_AUDIO_RATE);
    s_outputVolume = 10; s_outputPaused = 0;
    InitializeCriticalSection(&s_lock); s_lockValid = 1;
    hr = DirectSoundCreate(NULL, &s_ds, NULL);
    if (FAILED(hr) || !s_ds) { PbrbAudioShutdown(); return 0; }
    WAVEFORMATEX w;
    ZeroMemory(&w, sizeof(w));
    w.wFormatTag = WAVE_FORMAT_PCM; w.nChannels = 2; w.nSamplesPerSec = PBRB_AUDIO_RATE;
    w.wBitsPerSample = 16; w.nBlockAlign = 4; w.nAvgBytesPerSec = PBRB_AUDIO_RATE * 4;
    DSBUFFERDESC desc;
    ZeroMemory(&desc, sizeof(desc));
    desc.dwSize = sizeof(desc); desc.dwBufferBytes = PBRB_RING_BYTES; desc.lpwfxFormat = &w;
    hr = s_ds->CreateSoundBuffer(&desc, &s_buffer, NULL);
    if (FAILED(hr) || !s_buffer) { PbrbAudioShutdown(); return 0; }
    void* p1 = NULL, * p2 = NULL; DWORD b1 = 0, b2 = 0;
    hr = s_buffer->Lock(0, PBRB_RING_BYTES, &p1, &b1, &p2, &b2, 0);
    if (FAILED(hr)) { PbrbAudioShutdown(); return 0; }
    memset(p1, 0, b1); if (p2)memset(p2, 0, b2);
    s_buffer->Unlock(p1, b1, p2, b2);
    s_nextChunk = 0;
    s_event = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!s_event) { PbrbAudioShutdown(); return 0; }
    hr = s_buffer->Play(0, 0, DSBPLAY_LOOPING);
    if (FAILED(hr)) { PbrbAudioShutdown(); return 0; }
    s_lastRenderTick = GetTickCount();
    s_thread = CreateThread(NULL, 0, AudioWorker, NULL, 0, NULL);
    if (!s_thread) { PbrbAudioShutdown(); return 0; }
    SetThreadPriority(s_thread, THREAD_PRIORITY_ABOVE_NORMAL);
    s_ready = 1; return 1;
}
int PbrbAudioReady() { return s_ready; }
int PbrbAudioEffectsBusy(void) {
    int busy;
    /* Game scripts may wait for a cry. A missing device, exited worker,
       or stalled DirectSound cursor must allow silent play to continue. */
    if (!s_ready || !s_thread || WaitForSingleObject(s_thread, 0) != WAIT_TIMEOUT ||
        (DWORD)(GetTickCount() - s_lastRenderTick) > 1000u)return 0;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    busy = PbrbSynthEffectsBusy(&s_synth);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return busy;
}
int PbrbAudioMusicBusy(void) {
    int busy;
    if (!s_ready || !s_thread || WaitForSingleObject(s_thread, 0) != WAIT_TIMEOUT ||
        (DWORD)(GetTickCount() - s_lastRenderTick) > 1000u)return 0;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    busy = PbrbSynthMusicBusy(&s_synth);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return busy;
}
int PbrbAudioLoadUiPack(const char* path) {
    XpdArchive a; uint32_t n = 0; int ok = 0;
    unsigned char* bank = (unsigned char*)malloc(16384);
    if (!bank || !path) { if (bank)free(bank); return 0; }
    memset(&a, 0, sizeof(a));
    if (xpd_open(&a, path)) {
        if (xpd_read(&a, "rom/bank/02.bin", bank, 16384, &n) && n == 16384u) {
            if (s_lockValid)EnterCriticalSection(&s_lock);
            ok = PbrbSynthLoadMenuBank(&s_synth, bank, (int)n);
            if (s_lockValid)LeaveCriticalSection(&s_lock);
        }
        xpd_close(&a);
    }
    free(bank);
    return ok;
}
void PbrbAudioStopLauncherMusic() {
    if (s_lockValid)EnterCriticalSection(&s_lock);
    ReleaseLauncherMusic();
    if (s_lockValid)LeaveCriticalSection(&s_lock);
}
/* The caller holds the audio lock. Reuse the one owned ROM on skips. */
static int StartLauncherTrack(unsigned seed) {
    if (!s_launcherRom)return 0;
    const unsigned count = sizeof(kLauncherTracks) / sizeof(kLauncherTracks[0]);
    const unsigned choices = count - (s_lastLauncherTrack >= 0 ? 1u : 0u);
    const unsigned limit = ~0u - (~0u % choices);
    unsigned value;
    do { value = PbrbRandomNext(&seed); } while (value > limit);
    unsigned track = (value - 1u) % choices;
    if (s_lastLauncherTrack >= 0 && track >= (unsigned)s_lastLauncherTrack)++track;
    /* A different theme starts from clean voice state; output preferences
       are retained, including a muted but logically active soundtrack. */
    PbrbSynthStopTitle(&s_synth);
    const int ok = PbrbSynthAttachIntro(&s_synth, s_launcherRom, 1048576u) &&
        PbrbSynthPlayMusic(&s_synth, kLauncherTracks[track].music);
    if (ok) { s_launcherTrack = (int)track; s_lastLauncherTrack = (int)track; }
    else ReleaseLauncherMusic();
    return ok;
}
int PbrbAudioStartLauncherMusic(const char* path, unsigned seed) {
    PbrbAudioStopLauncherMusic();
    if (!s_ready || !path || !s_thread || WaitForSingleObject(s_thread, 0) != WAIT_TIMEOUT ||
        (DWORD)(GetTickCount() - s_lastRenderTick) > 1000u)return 0;
    XpdArchive archive; XpdResource resource; void* content = 0;
    uint32_t bytes = 0; int valid = 0;
    memset(&archive, 0, sizeof(archive));
    if (xpd_open(&archive, path)) {
        const unsigned expectedCRC = archive.game == XPD_GAME_RED ? 0x9f7fdd53u :
            archive.game == XPD_GAME_BLUE ? 0xd6da8a1au : 0u;
        /* InspectPack only checks the edge banks. Require the known complete
           ROM CRC here; xpd_read_alloc verifies all bytes against that CRC. */
        valid = expectedCRC && xpd_find(&archive, "rom/full.gb", &resource) &&
            resource.size == 1048576u && resource.crc32 == expectedCRC &&
            xpd_read_alloc(&archive, "rom/full.gb", &content, &bytes) && bytes == 1048576u;
        xpd_close(&archive);
    }
    if (!valid) { if (content)free(content); return 0; }
    if (s_lockValid)EnterCriticalSection(&s_lock);
    s_launcherRom = (unsigned char*)content;
    const int ok = StartLauncherTrack(seed);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
int PbrbAudioNextLauncherMusic() {
    if (!s_ready || !s_thread || WaitForSingleObject(s_thread, 0) != WAIT_TIMEOUT ||
        (DWORD)(GetTickCount() - s_lastRenderTick) > 1000u)return 0;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    const int ok = s_launcherTrack >= 0 && s_launcherRom && s_synth.rom == s_launcherRom ?
        StartLauncherTrack((unsigned)GetTickCount()) : 0;
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
const char* PbrbAudioLauncherTrackName() {
    const char* name = "";
    if (!s_ready || !s_thread || WaitForSingleObject(s_thread, 0) != WAIT_TIMEOUT ||
        (DWORD)(GetTickCount() - s_lastRenderTick) > 1000u)return name;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    if (!s_outputPaused && s_outputVolume > 0 && s_synth.musicOn &&
        s_launcherTrack >= 0 && s_launcherRom && s_synth.rom == s_launcherRom &&
        s_synth.musicId == kLauncherTracks[s_launcherTrack].music &&
        !s_synth.decodeErrors && PbrbSynthMusicBusy(&s_synth))
        name = kLauncherTracks[s_launcherTrack].name;
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return name;
}
int PbrbAudioStartTitle(const unsigned char* rom, size_t bytes) {
    int ok;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    if (rom != s_launcherRom)ReleaseLauncherMusic();
    s_launcherTrack = -1;
    ok = PbrbSynthStartTitle(&s_synth, rom, bytes);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
int PbrbAudioAttachIntro(const unsigned char* rom, size_t bytes) {
    int ok;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    if (rom != s_launcherRom)ReleaseLauncherMusic();
    s_launcherTrack = -1;
    ok = PbrbSynthAttachIntro(&s_synth, rom, bytes);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
int PbrbAudioStartIntroMusic(void) {
    int ok;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    ok = PbrbSynthStartIntroMusic(&s_synth);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
int PbrbAudioPlayMusic(int music) {
    int ok;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    ok = PbrbSynthPlayMusic(&s_synth, music);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
int PbrbAudioPlayCry(unsigned species) {
    int ok;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    ok = PbrbSynthPlayCry(&s_synth, species);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
int PbrbAudioFadeMusic(int music, unsigned framesPerStep) {
    int ok;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    ok = PbrbSynthFadeMusic(&s_synth, music, framesPerStep);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
int PbrbAudioPlaySfx(int effect) {
    int ok;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    ok = PbrbSynthPlaySfx(&s_synth, effect);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
int PbrbAudioPlaySfxModified(int effect, unsigned frequency, unsigned tempo) {
    int ok;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    ok = PbrbSynthPlaySfxModified(&s_synth, effect, frequency, tempo);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
void PbrbAudioStopTitle() {
    if (s_lockValid)EnterCriticalSection(&s_lock);
    PbrbSynthStopTitle(&s_synth);
    ReleaseLauncherMusic();
    if (s_lockValid)LeaveCriticalSection(&s_lock);
}
void PbrbAudioPauseSequencer(unsigned frames) {
    if (s_lockValid)EnterCriticalSection(&s_lock);
    PbrbSynthPauseSequencer(&s_synth, frames);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
}
void PbrbAudioSetVolume(int volume) {
    if (volume < 0)volume = 0;
    if (volume > 10)volume = 10;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    s_outputVolume = volume;
    if (s_lockValid)LeaveCriticalSection(&s_lock);
}
void PbrbAudioSetPaused(int paused) {
    if (s_lockValid)EnterCriticalSection(&s_lock);
    s_outputPaused = paused != 0;
    if (s_lockValid)LeaveCriticalSection(&s_lock);
}
void PbrbAudioSetLowHealth(int enabled) {
    if (s_lockValid)EnterCriticalSection(&s_lock);
    PbrbSynthSetLowHealth(&s_synth, enabled);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
}
void PbrbAudioSetMusic(int enabled) {
    if (s_lockValid)EnterCriticalSection(&s_lock);
    PbrbSynthSetMusicEnabled(&s_synth, enabled);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
}
void PbrbAudioSetEffects(int enabled) {
    if (s_lockValid)EnterCriticalSection(&s_lock);
    PbrbSynthSetEffectsEnabled(&s_synth, enabled);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
}
void PbrbAudioUi(int kind) {
    if (s_lockValid)EnterCriticalSection(&s_lock);
    PbrbSynthUi(&s_synth, kind);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
}
