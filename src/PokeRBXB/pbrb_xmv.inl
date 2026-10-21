/* Startup XMV playback, included only by pbrb_video.cpp.
   The SDK's native RGB texture path permits a real fade to black. The packet
   API keeps file reads asynchronous and bounds data to two packet buffers.
   Native/older RXDK xmv.lib is required in Linker / Additional Dependencies;
   the project's /NODEFAULTLIB setting intentionally ignores pragma libraries.
   No launcher assets or gameplay audio are active during this call. */

#include <dsound.h>
#include <xmv.h>
#include "pbrb_input.h"
#include <string.h>

#define PBRB_XMV_CONTEXT 0x50584D56u
#define PBRB_XMV_HEADER_BYTES 4096u
#define PBRB_XMV_MAX_PACKET (8u * 1024u * 1024u)
#define PBRB_XMV_FADE_MS 300u

struct PbrbXmvContext {
    HANDLE file, readEvent, stopEvent, worker;
    OVERLAPPED read;
    BYTE* buffers;
    BYTE* loading;
    BYTE* decoding;
    BYTE* pending;
    DWORD fileSize, packetSize, stride;
    DWORD loadingSize, loadingOffset, nextOffset;
    DWORD readSize;
    int readActive, readReady, readFailed, busy;
    XMVDecoder* decoder;
    LPDIRECTSOUND sound;
    IDirect3DTexture8* textures[2];
    IDirect3DSurface8* surfaces[2];
    IDirectSoundStream* audioStreams[3]; /* Borrowed from the decoder. */
    DWORD audioStreamCount;
    LONG masterAttenuation;
    unsigned audioGain;
};
static PbrbXmvContext g_pbrb_xmv;

/* Only the main thread touches file, decoder, graphics and input state. The
   service thread only pumps DirectSound while the native decoder is active. */
static int PbrbXmvQueueRead(void* destination, DWORD offset, DWORD bytes) {
    PbrbXmvContext& c = g_pbrb_xmv;
    if (c.readActive || !destination || offset > c.fileSize ||
        bytes > c.fileSize - offset)return 0;
    c.readReady = c.readFailed = 0;
    c.readSize = bytes;
    ZeroMemory(&c.read, sizeof(c.read));
    c.read.Offset = offset;
    c.read.hEvent = c.readEvent;
    ResetEvent(c.readEvent);
    if (!bytes) { c.readReady = 1; return 1; }
    c.readActive = 1;
    if (!ReadFile(c.file, destination, bytes, NULL, &c.read) &&
        GetLastError() != ERROR_IO_PENDING) {
        c.readActive = 0; c.readFailed = 1;
        return 0;
    }
    return 1;
}
static int PbrbXmvReadReady(BOOL wait) {
    PbrbXmvContext& c = g_pbrb_xmv;
    if (c.readFailed)return -1;
    if (c.readReady)return 1;
    if (!c.readActive)return -1;
    DWORD bytes = 0;
    if (!GetOverlappedResult(c.file, &c.read, &bytes, wait)) {
        if (GetLastError() == ERROR_IO_INCOMPLETE)return 0;
        c.readActive = 0; c.readFailed = 1;
        return -1;
    }
    c.readActive = 0;
    if (bytes != c.readSize) { c.readFailed = 1; return -1; }
    c.readReady = 1;
    return 1;
}
static HRESULT CALLBACK PbrbXmvGetNextPacket(DWORD context, void** packet,
    DWORD* advance) {
    PbrbXmvContext& c = g_pbrb_xmv;
    if (!packet || !advance)return E_INVALIDARG;
    *packet = NULL; *advance = 0;
    if (context != PBRB_XMV_CONTEXT || !c.loading)return E_FAIL;
    const int ready = PbrbXmvReadReady(FALSE);
    if (ready < 0)return E_FAIL;
    if (!ready)return S_OK;

    /* The previous packet may still feed the audio hardware. Reuse it only
       after the decoder explicitly invokes ReleasePreviousPacket. */
    c.pending = c.decoding;
    c.decoding = c.loading;
    c.loading = NULL;
    c.readReady = 0;
    c.nextOffset = c.loadingOffset + c.loadingSize;
    *packet = c.decoding;
    *advance = c.loadingSize;
    return S_OK;
}
static HRESULT CALLBACK PbrbXmvReleasePreviousPacket(DWORD context,
    LONGLONG offset, DWORD bytes) {
    PbrbXmvContext& c = g_pbrb_xmv;
    if (context != PBRB_XMV_CONTEXT || offset < 0 ||
        offset != (LONGLONG)c.nextOffset)return E_FAIL;
    if (!bytes) { c.pending = NULL; return S_OK; }
    if (bytes < sizeof(DWORD) || bytes > c.packetSize || !c.pending ||
        c.loading || c.readActive || offset > (LONGLONG)c.fileSize ||
        bytes > c.fileSize - (DWORD)offset)return E_FAIL;
    c.loading = c.pending;
    c.pending = NULL;
    c.loadingSize = bytes;
    c.loadingOffset = (DWORD)offset;
    return PbrbXmvQueueRead(c.loading, c.loadingOffset, bytes) ? S_OK : E_FAIL;
}

static DWORD WINAPI PbrbXmvService(LPVOID) {
    PbrbXmvContext& c = g_pbrb_xmv;
    for (;;) {
        if (WaitForSingleObject(c.stopEvent, 0) != WAIT_TIMEOUT)break;
        if (c.sound)DirectSoundDoWork();
        if (WaitForSingleObject(c.stopEvent, 4) != WAIT_TIMEOUT)break;
    }
    return 0;
}

static void PbrbXmvDisableAudio() {
    PbrbXmvContext& c = g_pbrb_xmv;
    for (DWORD i = 0; i < c.audioStreamCount; ++i) {
        XMVDecoder_DisableAudioStream(c.decoder, i);
        c.audioStreams[i] = NULL;
    }
    c.audioStreamCount = 0;
}

static void PbrbXmvSetAudioGain(unsigned gain) {
    PbrbXmvContext& c = g_pbrb_xmv;
    if (gain > 256u)gain = 256u;
    if (gain == c.audioGain)return;
    c.audioGain = gain;
    /* 2000*log10(gain/256), in DirectSound's hundredths of a dB. Keeping the
       curve here avoids new math/formatting CRT dependencies on older RXDK.
       Adding dB attenuation multiplies the saved master and transient gain. */
    static const LONG fadeAttenuation[257] = {
        -10000, -4816, -4214, -3862, -3612, -3419, -3260, -3126, -3010, -2908, -2816, -2734,
        -2658, -2589, -2524, -2464, -2408, -2356, -2306, -2259, -2214, -2172, -2132, -2093,
        -2056, -2021, -1987, -1954, -1922, -1892, -1862, -1834, -1806, -1779, -1754, -1728,
        -1704, -1680, -1657, -1634, -1612, -1591, -1570, -1550, -1530, -1510, -1491, -1472,
        -1454, -1436, -1419, -1401, -1384, -1368, -1352, -1336, -1320, -1305, -1290, -1275,
        -1260, -1246, -1232, -1218, -1204, -1191, -1177, -1164, -1151, -1139, -1126, -1114,
        -1102, -1090, -1078, -1066, -1055, -1043, -1032, -1021, -1010, -1000, -989, -978,
        -968, -958, -947, -937, -928, -918, -908, -898, -889, -880, -870, -861,
        -852, -843, -834, -825, -816, -808, -799, -791, -782, -774, -766, -758,
        -750, -742, -734, -726, -718, -710, -703, -695, -688, -680, -673, -665,
        -658, -651, -644, -637, -630, -623, -616, -609, -602, -595, -589, -582,
        -575, -569, -562, -556, -549, -543, -537, -530, -524, -518, -512, -506,
        -500, -494, -488, -482, -476, -470, -464, -459, -453, -447, -441, -436,
        -430, -425, -419, -414, -408, -403, -397, -392, -387, -382, -376, -371,
        -366, -361, -356, -350, -345, -340, -335, -330, -325, -321, -316, -311,
        -306, -301, -296, -292, -287, -282, -277, -273, -268, -264, -259, -254,
        -250, -245, -241, -236, -232, -228, -223, -219, -214, -210, -206, -201,
        -197, -193, -189, -185, -180, -176, -172, -168, -164, -160, -156, -152,
        -148, -144, -140, -136, -132, -128, -124, -120, -116, -112, -108, -104,
        -101, -97, -93, -89, -86, -82, -78, -74, -71, -67, -63, -60,
        -56, -52, -49, -45, -42, -38, -35, -31, -28, -24, -21, -17,
        -14, -10, -7, -3, 0
    };
    LONG attenuation = c.masterAttenuation + fadeAttenuation[gain];
    if (attenuation < -10000)attenuation = -10000;
    int failed = 0;
    for (DWORD i = 0; i < c.audioStreamCount; ++i)
        if (c.audioStreams[i] && FAILED(c.audioStreams[i]->SetVolume(attenuation)))failed = 1;
    /* Do not let an incomplete surround group continue at full volume. */
    if (failed)PbrbXmvDisableAudio();
}

static unsigned PbrbXmvFadeGain(DWORD elapsed) {
    if (elapsed >= PBRB_XMV_FADE_MS)return 0;
    const unsigned t = elapsed * 256u / PBRB_XMV_FADE_MS;
    const unsigned eased = t * t * (768u - 2u * t) / 65536u;
    return 256u - eased;
}

static void PbrbXmvClose() {
    PbrbXmvContext& c = g_pbrb_xmv;
    if (c.worker) {
        SetEvent(c.stopEvent);
        /* Join before closing audio owned by the decoder. Our stop event
           wakes the DirectSound service thread immediately. */
        WaitForSingleObject(c.worker, INFINITE);
        CloseHandle(c.worker); c.worker = NULL;
    }
    if (c.decoder) { XMVDecoder_CloseDecoder(c.decoder); c.decoder = NULL; }
    for (unsigned i = 0; i < 2u; ++i) {
        /* Present may still be using the last frame. Retire its GPU work
           before releasing the cached surface and its owning texture. */
        if (c.textures[i])c.textures[i]->BlockUntilNotBusy();
        if (c.surfaces[i]) { c.surfaces[i]->Release(); c.surfaces[i] = NULL; }
        if (c.textures[i]) { c.textures[i]->Release(); c.textures[i] = NULL; }
    }
    ZeroMemory(c.audioStreams, sizeof(c.audioStreams));
    c.audioStreamCount = 0;
    if (c.sound) { c.sound->Release(); c.sound = NULL; }
    if (c.file != INVALID_HANDLE_VALUE) {
        if (c.readActive) {
            /* Reads and cancellation originate on this same main thread.
               Even if cancellation fails, wait for IO completion before the
               OVERLAPPED/event or physical packet memory can be reclaimed. */
            CancelIo(c.file);
            DWORD bytes = 0;
            GetOverlappedResult(c.file, &c.read, &bytes, TRUE);
            c.readActive = 0;
        }
        CloseHandle(c.file); c.file = INVALID_HANDLE_VALUE;
    }
    if (c.buffers) { XPhysicalFree(c.buffers); c.buffers = NULL; }
    if (c.readEvent) { CloseHandle(c.readEvent); c.readEvent = NULL; }
    if (c.stopEvent) { CloseHandle(c.stopEvent); c.stopEvent = NULL; }
    c.busy = 0;
}

int PbrbVideoPlayIntro(const char* path, int volume) {
    PbrbXmvContext& c = g_pbrb_xmv;
    if (!g_dev || !path || !*path || c.busy)return -1;
    ZeroMemory(&c, sizeof(c));
    c.busy = 1; c.file = INVALID_HANDLE_VALUE;
    c.file = CreateFile(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_OVERLAPPED | FILE_FLAG_SEQUENTIAL_SCAN, NULL);
    if (c.file == INVALID_HANDLE_VALUE) {
        const DWORD error = GetLastError();
        c.busy = 0;
        return error == ERROR_FILE_NOT_FOUND || error == ERROR_PATH_NOT_FOUND ? 0 : -1;
    }
    DWORD high = 0;
    c.fileSize = GetFileSize(c.file, &high);
    if (high || c.fileSize == 0xFFFFFFFFu || c.fileSize < PBRB_XMV_HEADER_BYTES) {
        PbrbXmvClose(); return -1;
    }
    c.readEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!c.readEvent) { PbrbXmvClose(); return -1; }

    DWORD header[PBRB_XMV_HEADER_BYTES / sizeof(DWORD)];
    if (!PbrbXmvQueueRead(header, 0, sizeof(header)) || PbrbXmvReadReady(TRUE) != 1) {
        PbrbXmvClose(); return -1;
    }
    const DWORD firstSize = header[1];
    c.packetSize = header[2];
    /* The stock 64 MiB target must not allocate memory proportional to the
       movie length. Cap encoded packet size, then align each buffer page. */
    if (firstSize < sizeof(header) || firstSize > c.packetSize ||
        c.packetSize > PBRB_XMV_MAX_PACKET || firstSize > c.fileSize ||
        header[0] > c.packetSize || header[0] > c.fileSize - firstSize) {
        PbrbXmvClose(); return -1;
    }
    c.stride = (c.packetSize + 4095u) & ~4095u;
    c.buffers = (BYTE*)XPhysicalAlloc(c.stride * 2u, MAXULONG_PTR, 0, PAGE_READWRITE);
    if (!c.buffers) { PbrbXmvClose(); return -1; }
    memcpy(c.buffers, header, sizeof(header));
    c.loading = c.buffers;
    c.decoding = c.buffers + c.stride;
    c.loadingSize = firstSize;
    c.loadingOffset = 0;
    /* Reuse the header already read. Prefetch the first packet remainder
       while the decoder, sound stream and service thread are prepared. */
    if (!PbrbXmvQueueRead(c.buffers + sizeof(header), sizeof(header),
        firstSize - sizeof(header))) {
        PbrbXmvClose(); return -1;
    }
    HRESULT hr = XMVDecoder_CreateDecoderForPackets(XMVFLAG_NONE, header,
        PBRB_XMV_CONTEXT, PbrbXmvGetNextPacket, PbrbXmvReleasePreviousPacket, &c.decoder);
    if (FAILED(hr) || !c.decoder) { PbrbXmvClose(); return -1; }
    XMVVIDEO_DESC info;
    ZeroMemory(&info, sizeof(info));
    XMVDecoder_GetVideoDescriptor(c.decoder, &info);
    if (!info.Width || !info.Height || info.Width > 2048u || info.Height > 2048u) {
        PbrbXmvClose(); return -1;
    }
    /* The native decoder requires an exact-size linear RGB surface. Do not
       pad to powers of two or use a smaller intermediate image. Two textures
       keep the last valid frame separate from the next decode destination. */
    for (unsigned i = 0; i < 2u; ++i) {
        hr = g_dev->CreateTexture(info.Width, info.Height, 1, 0,
            D3DFMT_LIN_A8R8G8B8, 0, &c.textures[i]);
        if (FAILED(hr) || !c.textures[i]) { PbrbXmvClose(); return -1; }
        hr = c.textures[i]->GetSurfaceLevel(0, &c.surfaces[i]);
        if (FAILED(hr) || !c.surfaces[i]) { PbrbXmvClose(); return -1; }
    }
    if (volume < 0)volume = 0;
    if (volume > 10)volume = 10;
    /* Match the launcher's linear gain steps using DirectSound attenuation
       in hundredths of a dB; no new math/formatting CRT imports are needed. */
    static const LONG attenuation[11] = {
        -10000, -2000, -1398, -1046, -796, -602, -444, -310, -194, -92, 0
    };
    c.masterAttenuation = attenuation[volume];
    c.audioGain = 257u;
    if (info.AudioStreamCount && SUCCEEDED(DirectSoundCreate(NULL, &c.sound, NULL)) && c.sound) {
        /* NULL stream output preserves native automatic ADPCM 5.1-triplet
           enabling. Query borrowed stream handles only to adjust volume. */
        const HRESULT audioResult = XMVDecoder_EnableAudioStream(c.decoder, 0, 0, NULL, NULL);
        c.audioStreamCount = info.AudioStreamCount < 3u ? info.AudioStreamCount : 3u;
        for (DWORD i = 0; i < c.audioStreamCount; ++i)
            XMVDecoder_GetAudioStream(c.decoder, i, &c.audioStreams[i]);
        /* A partially enabled surround group must not escape volume/mute if
           its later stream creation or any attenuation call failed. */
        if (FAILED(audioResult) || !c.audioStreams[0])PbrbXmvDisableAudio();
        else PbrbXmvSetAudioGain(256u);
    }
    c.stopEvent = CreateEvent(NULL, TRUE, FALSE, NULL);
    if (!c.stopEvent) { PbrbXmvClose(); return -1; }
    DWORD state = 0;
    if (FAILED(g_dev->CreateStateBlock(D3DSBT_ALL, &state))) { PbrbXmvClose(); return -1; }
    DWORD threadId = 0;
    c.worker = CreateThread(NULL, 0, PbrbXmvService, NULL, 0, &threadId);
    if (!c.worker) {
        g_dev->DeleteStateBlock(state);
        PbrbXmvClose(); return -1;
    }
    SetThreadPriority(c.worker, THREAD_PRIORITY_ABOVE_NORMAL);
    const RECT destination = PbrbVideoIntroRect(info.Width, info.Height);
    const unsigned skipKeys = PBRB_PAD_A | PBRB_PAD_B | PBRB_PAD_START;
    int showing = -1, nextTexture = 0, armed = 0, fading = 0, decodeDone = 0;
    int result = 1;
    DWORD fadeStart = 0;
    PbrbVideoPresentIntroFrame(NULL, destination, info.Width, info.Height, 0);
    for (;;) {
        /* The main thread owns input. A held boot/reconnect button cannot
           skip until released and pressed again. Once fading, stay latched. */
        const PbrbEvents e = PbrbInputPoll();
        if (!e.connected || e.controllerChanged)armed = 0;
        if (e.connected) {
            if (!(e.held & skipKeys))armed = 1;
            else if (armed && (e.pressed & skipKeys) && !fading) {
                fading = 1; fadeStart = GetTickCount();
            }
        }
        unsigned gain = fading ? PbrbXmvFadeGain(GetTickCount() - fadeStart) : 256u;
        if (!decodeDone && gain) {
            /* Only the other texture is written. The fence also covers the
               older draw which may still be queued on the GPU. Native timing
               and audio synchronization remain enabled through NULL time. */
            c.textures[nextTexture]->BlockUntilNotBusy();
            XMVRESULT frame = XMV_NOFRAME;
            hr = XMVDecoder_GetNextFrame(c.decoder, c.surfaces[nextTexture], &frame, NULL);
            if (FAILED(hr) || frame == XMV_FAIL) {
                result = -1; decodeDone = 1;
            }
            else if (frame == XMV_NEWFRAME) {
                showing = nextTexture;
                nextTexture = 1 - nextTexture;
            }
            else if (frame == XMV_ENDOFFILE)decodeDone = 1;
            /* NOFRAME does not change the displayed texture or advance the
               write target. EOF/error retains the last image for the fade. */
            if (decodeDone && !fading) {
                fading = 1; fadeStart = GetTickCount();
            }
        }
        gain = fading ? PbrbXmvFadeGain(GetTickCount() - fadeStart) : 256u;
        PbrbXmvSetAudioGain(gain);
        PbrbVideoPresentIntroFrame(showing >= 0 ? c.textures[showing] : NULL,
            destination, info.Width, info.Height, gain);
        if (fading && !gain)break;
    }
    /* Skip keeps frames and sound advancing during the fade. Stop only once
       black is presented, on the same thread that owns GetNextFrame. */
    XMVDecoder_TerminateImmediately(c.decoder);
    PbrbXmvClose();
    g_dev->ApplyStateBlock(state);
    g_dev->DeleteStateBlock(state);
    g_dev->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF000000, 1.0f, 0);
    g_dev->Present(NULL, NULL, NULL, NULL);
    return result;
}
