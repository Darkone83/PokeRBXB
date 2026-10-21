#include "pbrb_video.h"
#include <math.h>

static IDirect3D8* g_d3d = NULL;
static IDirect3DDevice8* g_dev = NULL;
static int g_sw = 640, g_sh = 480;
static const char* g_mode = "480i";
static float g_pixel_aspect = 1.0f;
static PbrbRect g_game_rect = { 0, 0, 640, 480 };
static float g_canvas_w = 1672.0f, g_canvas_h = 941.0f;
static float g_scale = 1.0f, g_off_x = 0.0f, g_off_y = 0.0f;
static int g_smooth_game = 0;
static int g_screen_percent = 100, g_screen_horizontal = 0, g_screen_vertical = 0;
struct V { float x, y, z, rhw; DWORD c; float u, v; };
#define PBRB_FVF (D3DFVF_XYZRHW | D3DFVF_DIFFUSE | D3DFVF_TEX1)

IDirect3DDevice8* PbrbDevice() { return g_dev; }
int PbrbScreenWidth() { return g_sw; }
int PbrbScreenHeight() { return g_sh; }
const char* PbrbModeName() { return g_mode; }
void PbrbSetGameArea(float x, float y, float w, float h) {
    if (w <= 0 || h <= 0)return;
    const float aspect = (160.0f / 144.0f) / g_pixel_aspect;
    float gw = h * aspect, gh = h;
    if (gw > w) { gw = w; gh = w / aspect; }
    g_game_rect.x = x + (w - gw) * 0.5f;
    g_game_rect.y = y + (h - gh) * 0.5f;
    g_game_rect.w = gw; g_game_rect.h = gh;
}
PbrbRect PbrbGameRect() { return g_game_rect; }
void PbrbSetArtworkCanvas(float w, float h) {
    float sx, sy;
    if (w <= 0 || h <= 0)return;
    g_canvas_w = w; g_canvas_h = h;
    sx = (float)g_sw / w; sy = (float)g_sh / h;
    g_scale = sx < sy ? sx : sy;  // contain, not anamorphic distortion
    g_off_x = ((float)g_sw - w * g_scale) * 0.5f;
    g_off_y = ((float)g_sh - h * g_scale) * 0.5f;
}
PbrbPoint PbrbPt(float x, float y) { PbrbPoint v = { g_off_x + x * g_scale,g_off_y + y * g_scale }; return v; }
float PbrbScale() { return g_scale; }

static HRESULT TryDevice(D3DPRESENT_PARAMETERS* pp) {
    HRESULT hr = g_d3d->CreateDevice(0, D3DDEVTYPE_HAL, NULL, D3DCREATE_HARDWARE_VERTEXPROCESSING, pp, &g_dev);
    if (FAILED(hr) && g_dev) { g_dev->Release(); g_dev = NULL; }
    return hr;
}
int PbrbVideoInit() {
    D3DPRESENT_PARAMETERS pp; HRESULT hr; DWORD f, s;
    if (g_dev)return 1;
    f = XGetVideoFlags(); s = XGetVideoStandard();
    ZeroMemory(&pp, sizeof(pp));
    pp.BackBufferFormat = D3DFMT_X8R8G8B8;
    pp.BackBufferCount = 1;
    pp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    pp.EnableAutoDepthStencil = FALSE;
    pp.MultiSampleType = D3DMULTISAMPLE_NONE;
    pp.FullScreen_PresentationInterval = D3DPRESENT_INTERVAL_ONE;
    pp.FullScreen_RefreshRateInHz = 60;
    if (f & XC_VIDEO_FLAGS_HDTV_720p) {
        pp.BackBufferWidth = 1280; pp.BackBufferHeight = 720;
        pp.Flags = D3DPRESENTFLAG_PROGRESSIVE | D3DPRESENTFLAG_WIDESCREEN;
        g_mode = "720p";
    }
    else if (f & XC_VIDEO_FLAGS_HDTV_480p) {
        pp.BackBufferWidth = 640; pp.BackBufferHeight = 480;
        pp.Flags = D3DPRESENTFLAG_PROGRESSIVE;
        if (f & XC_VIDEO_FLAGS_WIDESCREEN)pp.Flags |= D3DPRESENTFLAG_WIDESCREEN;
        g_mode = "480p";
    }
    else if (s == XC_VIDEO_STANDARD_PAL_I && !(f & XC_VIDEO_FLAGS_PAL_60Hz)) {
        pp.BackBufferWidth = 640; pp.BackBufferHeight = 576;
        pp.Flags = D3DPRESENTFLAG_INTERLACED;
        pp.FullScreen_RefreshRateInHz = 50;
        g_mode = "576i";
    }
    else {
        pp.BackBufferWidth = 640; pp.BackBufferHeight = 480;
        pp.Flags = D3DPRESENTFLAG_INTERLACED;
        g_mode = "480i";
    }
    if (f & XC_VIDEO_FLAGS_WIDESCREEN)pp.Flags |= D3DPRESENTFLAG_WIDESCREEN;
    g_d3d = Direct3DCreate8(D3D_SDK_VERSION);
    if (!g_d3d)return 0;
    hr = TryDevice(&pp);
    if (FAILED(hr)) {
        // SD-safe fallback, do not force progressive on 1.6/SDTV systems.
        pp.BackBufferWidth = 640; pp.BackBufferHeight = 480;
        pp.Flags = D3DPRESENTFLAG_INTERLACED;
        if (f & XC_VIDEO_FLAGS_WIDESCREEN)pp.Flags |= D3DPRESENTFLAG_WIDESCREEN;
        pp.FullScreen_RefreshRateInHz = 60; g_mode = "480i";
        if (s == XC_VIDEO_STANDARD_PAL_I && !(f & XC_VIDEO_FLAGS_PAL_60Hz)) {
            pp.BackBufferHeight = 576; pp.FullScreen_RefreshRateInHz = 50; g_mode = "576i";
        }
        hr = TryDevice(&pp);
    }
    if (FAILED(hr)) { g_d3d->Release(); g_d3d = NULL; return 0; }
    g_sw = (int)pp.BackBufferWidth; g_sh = (int)pp.BackBufferHeight;
    // The PAL 640x576 surface is displayed at 4:3 (or 16:9), not 10:9.
    // Likewise a widescreen 640x480 surface is anamorphic. 720p is square.
    const float displayAspect = (pp.Flags & D3DPRESENTFLAG_WIDESCREEN) ?
        (16.0f / 9.0f) : (4.0f / 3.0f);
    g_pixel_aspect = displayAspect * (float)g_sh / (float)g_sw;
    PbrbSetGameArea(0, 0, (float)g_sw, (float)g_sh);
    g_dev->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
    g_dev->SetRenderState(D3DRS_LIGHTING, FALSE);
    PbrbSetArtworkCanvas(g_canvas_w, g_canvas_h);
    return 1;
}
void PbrbVideoShutdown() {
    if (g_dev) { g_dev->Release(); g_dev = NULL; }
    if (g_d3d) { g_d3d->Release(); g_d3d = NULL; }
}
void PbrbBegin() { g_dev->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF070911, 1.0f, 0); g_dev->BeginScene(); }
void PbrbEnd() { g_dev->EndScene(); g_dev->Present(NULL, NULL, NULL, NULL); }

static void DrawVerts(V* v, int primitives, IDirect3DTexture8* tex, int pointFilter = 0,
    int applyCalibration = 1, int opaque = 0) {
    if (!g_dev)return;
    if (applyCalibration && (g_screen_percent != 100 || g_screen_horizontal || g_screen_vertical)) {
        const float scale = (float)g_screen_percent / 100.0f;
        const float x = g_sw * ((1.0f - scale) * 0.5f + g_screen_horizontal / 100.0f);
        const float y = g_sh * ((1.0f - scale) * 0.5f + g_screen_vertical / 100.0f);
        for (int i = 0; i < primitives * 3; ++i) {
            v[i].x = v[i].x * scale + x;
            v[i].y = v[i].y * scale + y;
        }
    }
    g_dev->SetRenderState(D3DRS_ZENABLE, FALSE);
    g_dev->SetRenderState(D3DRS_ALPHABLENDENABLE, opaque ? FALSE : TRUE);
    g_dev->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
    g_dev->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    g_dev->SetTexture(0, tex);
    g_dev->SetTextureStageState(0, D3DTSS_COLOROP, tex ? D3DTOP_MODULATE : D3DTOP_SELECTARG2);
    g_dev->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    g_dev->SetTextureStageState(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    g_dev->SetTextureStageState(0, D3DTSS_ALPHAOP, tex && !opaque ? D3DTOP_MODULATE : D3DTOP_SELECTARG2);
    g_dev->SetTextureStageState(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
    g_dev->SetTextureStageState(0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE);
    g_dev->SetTextureStageState(0, D3DTSS_ADDRESSU, D3DTADDRESS_CLAMP);
    g_dev->SetTextureStageState(0, D3DTSS_ADDRESSV, D3DTADDRESS_CLAMP);
    g_dev->SetTextureStageState(0, D3DTSS_MINFILTER, pointFilter ? D3DTEXF_POINT : D3DTEXF_LINEAR);
    g_dev->SetTextureStageState(0, D3DTSS_MAGFILTER, pointFilter ? D3DTEXF_POINT : D3DTEXF_LINEAR);
    g_dev->SetTextureStageState(0, D3DTSS_MIPFILTER, D3DTEXF_NONE);
    g_dev->SetVertexShader(PBRB_FVF);
    g_dev->DrawPrimitiveUP(D3DPT_TRIANGLELIST, primitives, v, sizeof(V));
    g_dev->SetTexture(0, NULL);
}
static void MakeQuad(V* v, float x, float y, float w, float h, DWORD tint, float u0, float v0, float u1, float v1) {
    V a = { x - .5f,y - .5f,0,1,tint,u0,v0 };
    V b = { x + w - .5f,y - .5f,0,1,tint,u1,v0 };
    V c = { x - .5f,y + h - .5f,0,1,tint,u0,v1 };
    V d = { x + w - .5f,y + h - .5f,0,1,tint,u1,v1 };
    v[0] = a; v[1] = b; v[2] = c; v[3] = c; v[4] = b; v[5] = d;
}
void PbrbRectangle(float x, float y, float w, float h, DWORD c) { V v[6]; MakeQuad(v, x, y, w, h, c, 0, 0, 0, 0); DrawVerts(v, 2, NULL); }
void PbrbSpriteUV(PbrbTexture* t, float x, float y, float w, float h, float u0, float v0, float u1, float v1, DWORD tint) {
    V v[6]; if (!t || !t->gpu)return;
    MakeQuad(v, x, y, w, h, tint, u0, v0, u1, v1); DrawVerts(v, 2, t->gpu);
}
void PbrbSprite(PbrbTexture* t, float x, float y, float w, float h, DWORD tint) {
    if (!t || !t->gpu)return;
    PbrbSpriteUV(t, x, y, w, h, 0, 0, (float)t->width / t->padded_width, (float)t->height / t->padded_height, tint);
}
void PbrbLine(float x1, float y1, float x2, float y2, float thick, DWORD tint) {
    float dx = x2 - x1, dy = y2 - y1, len = (float)sqrt((double)(dx * dx + dy * dy));
    if (len < 0.01f)return;
    float nx = -dy * thick / (2 * len), ny = dx * thick / (2 * len);
    V v[6];
    V a = { x1 + nx,y1 + ny,0,1,tint,0,0 }, b = { x2 + nx,y2 + ny,0,1,tint,0,0 };
    V c = { x1 - nx,y1 - ny,0,1,tint,0,0 }, d = { x2 - nx,y2 - ny,0,1,tint,0,0 };
    v[0] = a; v[1] = b; v[2] = c; v[3] = c; v[4] = b; v[5] = d; DrawVerts(v, 2, NULL);
}
void PbrbCircle(float x, float y, float radius, DWORD color) {
    int i; for (i = 0; i < 16; ++i) {
        const float a = (float)(6.28318530718 * i / 16.0), b = (float)(6.28318530718 * (i + 1) / 16.0);
        V v[3] = { {x,y,0,1,color,0,0},
            {x + radius * (float)cos(a),y + radius * (float)sin(a),0,1,color,0,0},
            {x + radius * (float)cos(b),y + radius * (float)sin(b),0,1,color,0,0} };
        DrawVerts(v, 1, NULL);
    }
}

void PbrbSpritePoint(PbrbTexture* t, float x, float y, float w, float h, DWORD tint) {
    V v[6]; if (!t || !t->gpu)return;
    MakeQuad(v, x, y, w, h, tint, 0.0f, 0.0f, (float)t->width / t->padded_width, (float)t->height / t->padded_height);
    DrawVerts(v, 2, t->gpu, 1);
}

void PbrbSetScreenCalibration(int percent, int horizontal, int vertical) {
    g_screen_percent = percent < 80 ? 80 : percent > 100 ? 100 : percent;
    g_screen_horizontal = horizontal < -5 ? -5 : horizontal > 5 ? 5 : horizontal;
    g_screen_vertical = vertical < -5 ? -5 : vertical > 5 ? 5 : vertical;
}
void PbrbSetSmoothFiltering(int smooth) { g_smooth_game = smooth != 0; }
void PbrbSpriteGame(PbrbTexture* t, float x, float y, float w, float h, DWORD tint) {
    V v[6]; if (!t || !t->gpu)return;
    MakeQuad(v, x, y, w, h, tint, 0.0f, 0.0f, (float)t->width / t->padded_width, (float)t->height / t->padded_height);
    DrawVerts(v, 2, t->gpu, !g_smooth_game);
}

/* Fit decoded video using physical pixel aspect and saved calibration.
   Keep the destination on-screen; even edges preserve the established movie
   placement across the SD and HD output modes. */
static RECT PbrbVideoIntroRect(unsigned width, unsigned height) {
    RECT rect = { 0, 0, g_sw, g_sh };
    if (!width || !height || g_pixel_aspect <= 0.0f)return rect;
    const float aspect = ((float)width / (float)height) / g_pixel_aspect;
    float movieWidth = (float)g_sw, movieHeight = movieWidth / aspect;
    if (movieHeight > g_sh) {
        movieHeight = (float)g_sh; movieWidth = movieHeight * aspect;
    }
    const float scale = (float)g_screen_percent / 100.0f;
    const float offsetX = g_sw * ((1.0f - scale) * 0.5f + g_screen_horizontal / 100.0f);
    const float offsetY = g_sh * ((1.0f - scale) * 0.5f + g_screen_vertical / 100.0f);
    int w = (int)(movieWidth * scale + 0.5f) & ~1;
    int h = (int)(movieHeight * scale + 0.5f) & ~1;
    if (w < 2)w = 2; if (w > g_sw)w = g_sw & ~1;
    if (h < 2)h = 2; if (h > g_sh)h = g_sh & ~1;
    int left = (int)(((g_sw - movieWidth) * 0.5f) * scale + offsetX + 0.5f);
    int top = (int)(((g_sh - movieHeight) * 0.5f) * scale + offsetY + 0.5f);
    if (left < 0)left = 0; if (left > g_sw - w)left = g_sw - w;
    if (top < 0)top = 0; if (top > g_sh - h)top = g_sh - h;
    rect.left = left & ~1; rect.top = top & ~1;
    rect.right = rect.left + w; rect.bottom = rect.top + h;
    return rect;
}

/* Fade covers the physical output, including areas outside calibration.
   Call inside an active scene after the launcher has drawn all its layers. */
void PbrbVideoBlackFade(unsigned opacity) {
    if (!opacity || !g_dev)return;
    if (opacity > 255u)opacity = 255u;
    V v[6];
    MakeQuad(v, 0, 0, (float)g_sw, (float)g_sh, opacity << 24,
        0.0f, 0.0f, 0.0f, 0.0f);
    DrawVerts(v, 2, NULL, 0, 0);
}

/* Native LIN_A8R8G8B8 frames use pixel-unit UVs. Decode at source size and
   scale once with linear filtering, independently of the game's pixel filter.
   RGB modulation provides a real fade and ignores unspecified movie alpha. */
static void PbrbVideoPresentIntroFrame(IDirect3DTexture8* texture,
    const RECT& destination, unsigned width, unsigned height, unsigned gain) {
    if (!g_dev)return;
    if (gain > 256u)gain = 256u;
    g_dev->Clear(0, NULL, D3DCLEAR_TARGET, 0xFF000000, 1.0f, 0);
    g_dev->BeginScene();
    if (texture && width && height) {
        const DWORD level = (gain * 255u + 128u) / 256u;
        const DWORD tint = 0xFF000000u | (level << 16) | (level << 8) | level;
        V v[6];
        MakeQuad(v, (float)destination.left, (float)destination.top,
            (float)(destination.right - destination.left),
            (float)(destination.bottom - destination.top), tint,
            0.0f, 0.0f, (float)width, (float)height);
        g_dev->SetPixelShader(0);
        g_dev->SetRenderState(D3DRS_YUVENABLE, FALSE);
        g_dev->SetRenderState(D3DRS_ALPHATESTENABLE, FALSE);
        g_dev->SetRenderState(D3DRS_FOGENABLE, FALSE);
        for (unsigned stage = 1; stage < 4; ++stage) {
            g_dev->SetTexture(stage, NULL);
            g_dev->SetTextureStageState(stage, D3DTSS_COLOROP, D3DTOP_DISABLE);
            g_dev->SetTextureStageState(stage, D3DTSS_ALPHAOP, D3DTOP_DISABLE);
        }
        DrawVerts(v, 2, texture, 0, 0, 1);
    }
    g_dev->EndScene();
    g_dev->Present(NULL, NULL, NULL, NULL);
}

// Included here so source-only updates do not require another compile item.
#include "pbrb_xmv.inl"
