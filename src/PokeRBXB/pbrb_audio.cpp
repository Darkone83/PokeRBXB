#include <xtl.h>
#include <dsound.h>
#include "pbrb_audio.h"
#include "pbrb_audio_core.h"
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
static DWORD s_nextChunk = 0;
static volatile DWORD s_lastRenderTick = 0;
static PbrbSynth s_synth;
/* Correct scratch size is 1024*2 samples, not 1024. */
static short s_stereo[PBRB_FRAMES_PER_CHUNK * 2u];

static void FillChunk(void* ptr, DWORD bytes) {
    if (!ptr || !bytes)return;
    /* Device Lock can wrap, and each region can be shorter than a chunk. */
    short* dst = (short*)ptr;
    unsigned frames = (unsigned)bytes / 4u;
    while (frames) {
        unsigned n = frames > PBRB_FRAMES_PER_CHUNK ? PBRB_FRAMES_PER_CHUNK : frames;
        PbrbSynthRender(&s_synth, s_stereo, (int)n);
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
int PbrbAudioStartTitle(const unsigned char* rom, size_t bytes) {
    int ok;
    if (s_lockValid)EnterCriticalSection(&s_lock);
    ok = PbrbSynthStartTitle(&s_synth, rom, bytes);
    if (s_lockValid)LeaveCriticalSection(&s_lock);
    return ok;
}
int PbrbAudioAttachIntro(const unsigned char* rom, size_t bytes) {
    int ok;
    if (s_lockValid)EnterCriticalSection(&s_lock);
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
    if (s_lockValid)LeaveCriticalSection(&s_lock);
}
void PbrbAudioPauseSequencer(unsigned frames) {
    if (s_lockValid)EnterCriticalSection(&s_lock);
    PbrbSynthPauseSequencer(&s_synth, frames);
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
