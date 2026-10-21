#include "pbrb_ddf.h"
#include "pbrb_texture.h"
#include <stdlib.h>
#include <string.h>

struct DdfGlyph { int x, y, w, h, advance, bear_y; };
static DdfGlyph g_metrics[3][95];
static PbrbTexture g_atlas = { 0 };
static float g_units[3] = { 1,1,1 };
static int g_ascent[3] = { 0,0,0 };
static int g_ready = 0;
static const int k_virtual[3] = { 14,18,24 };
static unsigned int RD32(const unsigned char* p) {
    return (unsigned int)p[0] | ((unsigned int)p[1] << 8) | ((unsigned int)p[2] << 16) | ((unsigned int)p[3] << 24);
}
int PbrbFontReady() { return g_ready; }
void PbrbFontShutdown() { PbrbReleaseTexture(&g_atlas); g_ready = 0; }
int PbrbFontLoad(const char* path) {
    HANDLE f; DWORD got = 0; unsigned char hdr[28], rec[24];
    unsigned char* bgra = NULL; DdfGlyph temp[3][95];
    PbrbTexture atlas = { 0 }; int width = 0, height = 0, ok = 0;
    float ratio[3]; int ascend[3] = { 0,0,0 };
    if (!path)return 0;
    f = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE)return 0;
    if (!ReadFile(f, hdr, sizeof(hdr), &got, NULL) || got != sizeof(hdr))goto end;
    if (memcmp(hdr, "DDF1", 4) || RD32(hdr + 4) != 1)goto end;
    width = (int)RD32(hdr + 8); height = (int)RD32(hdr + 12);
    if (width < 16 || height < 16 || width>2048 || height>2048)goto end;
    for (int t = 0; t < 3; ++t) {
        const unsigned raster = RD32(hdr + 16 + t * 4);
        if (raster < 4 || raster>1024)goto end;
        ratio[t] = (float)k_virtual[t] / (float)raster;
        for (int i = 0; i < 95; ++i) {
            if (!ReadFile(f, rec, sizeof(rec), &got, NULL) || got != sizeof(rec))goto end;
            DdfGlyph* g = &temp[t][i];
            g->x = (int)RD32(rec + 0) + 1; // bundled DarkDash encoder padding
            g->y = (int)RD32(rec + 4) + 1;
            g->w = (int)RD32(rec + 8); g->h = (int)RD32(rec + 12);
            g->advance = (int)RD32(rec + 16); g->bear_y = (int)RD32(rec + 20);
            if (g->w<0 || g->h<0 || g->w>width || g->h>height ||
                g->x<0 || g->y<0 || g->x + g->w>width || g->y + g->h>height ||
                g->advance < 0 || g->advance>4096 || g->bear_y < -4096 || g->bear_y>4096)goto end;
            if (-g->bear_y > ascend[t])ascend[t] = -g->bear_y;
        }
    }
    bgra = (unsigned char*)malloc((size_t)width * height * 4);
    if (!bgra)goto end;
    if (!ReadFile(f, bgra, (DWORD)((size_t)width * height * 4), &got, NULL) ||
        got != (DWORD)((size_t)width * height * 4))goto end;
    // DDF1 uses white glyphs on transparent black; avoid black linear-filter fringes.
    for (int i = 0; i < width * height; ++i)
        if (!bgra[i * 4 + 3])bgra[i * 4] = bgra[i * 4 + 1] = bgra[i * 4 + 2] = 255;
    if (!PbrbUploadBGRA(bgra, width, height, &atlas))goto end;
    memcpy(g_metrics, temp, sizeof(temp));
    for (int t = 0; t < 3; ++t) { g_ascent[t] = ascend[t]; g_units[t] = ratio[t]; }
    PbrbReleaseTexture(&g_atlas); g_atlas = atlas;
    g_ready = 1; ok = 1;
end:
    if (bgra)free(bgra);
    if (!ok)PbrbReleaseTexture(&atlas);
    CloseHandle(f); return ok;
}
int PbrbTextWidth(const char* text, int tier, float scale) {
    float sum = 0;
    if (!g_ready || !text || tier < 0 || tier>2)return 0;
    for (; *text; ++text) {
        unsigned char c = (unsigned char)*text;
        if (c >= 32 && c <= 126)sum += (float)g_metrics[tier][c - 32].advance * g_units[tier] * scale;
    }
    return (int)(sum + 0.5f);
}
void PbrbText(float x, float y, const char* text, int tier, float multiplier, DWORD color) {
    if (!g_ready || !text || tier < 0 || tier>2)return;
    const float s = g_units[tier] * multiplier * PbrbScale();
    PbrbPoint origin = PbrbPt(x, y);
    float cx = origin.x;
    for (; *text; ++text) {
        const unsigned char c = (unsigned char)*text;
        if (c < 32 || c>126)continue;
        const DdfGlyph* g = &g_metrics[tier][c - 32];
        if (g->w > 0 && g->h > 0) {
            float u0 = (float)g->x / g_atlas.padded_width, v0 = (float)g->y / g_atlas.padded_height;
            float u1 = (float)(g->x + g->w) / g_atlas.padded_width, v1 = (float)(g->y + g->h) / g_atlas.padded_height;
            PbrbSpriteUV(&g_atlas, cx, origin.y + (float)(g_ascent[tier] + g->bear_y) * s,
                (float)g->w * s, (float)g->h * s, u0, v0, u1, v1, color);
        }
        cx += (float)g->advance * s;
    }
}
void PbrbTextCenter(float center_x, float y, const char* text, int tier, float multiplier, DWORD color) {
    // width in artwork units, transformed by PbrbText once.
    const float w = (float)PbrbTextWidth(text, tier, multiplier);
    PbrbText(center_x - w * 0.5f, y, text, tier, multiplier, color);
}
