#pragma once
#include <xtl.h>
#include <d3d8.h>

struct PbrbTexture {
    IDirect3DTexture8* gpu;
    int width, height, padded_width, padded_height;
};

struct PbrbPoint { float x, y; };
struct PbrbRect { float x, y, w, h; };

int PbrbVideoInit();
void PbrbVideoShutdown();
IDirect3DDevice8* PbrbDevice();
int PbrbScreenWidth();
int PbrbScreenHeight();
const char* PbrbModeName();

// Game placement uses output pixels, independently of the launcher canvas.
// Fit the 160x144 image within the art's measured opening. SD wide/PAL
// modes compensate for non-square output pixels to preserve physical 10:9.
void PbrbSetGameArea(float left, float top, float width, float height);
PbrbRect PbrbGameRect();
void PbrbBegin();
void PbrbEnd();
// Full output black cover for transitions, unaffected by screen calibration.
// Draw after the scene, before PbrbEnd; opacity is 0..255.
void PbrbVideoBlackFade(unsigned opacity);

// One full-frame transform, shared by the art, typography and focus geometry.
// Center/contain preserves the 16:9 art on 4:3 monitors; black letterbox bars.
void PbrbSetArtworkCanvas(float width, float height);
PbrbPoint PbrbPt(float artwork_x, float artwork_y);
float PbrbScale();
void PbrbRectangle(float x, float y, float w, float h, DWORD argb);
void PbrbLine(float ax, float ay, float bx, float by, float thickness, DWORD argb);
void PbrbCircle(float x, float y, float radius, DWORD argb);
void PbrbSprite(PbrbTexture* tex, float x, float y, float w, float h, DWORD tint);
void PbrbSpriteUV(PbrbTexture* tex, float x, float y, float w, float h,
    float u0, float v0, float u1, float v1, DWORD tint);

// Point-sampled for original GB tile art, independent of launcher/font filtering.
void PbrbSpritePoint(PbrbTexture* tex, float x, float y, float w, float h, DWORD tint);

// The gameplay surface alone follows the player's Sharp/Smooth preference.
// SpritePoint remains the fixed reference path for diagnostics/comparisons.
/* Final output transform shared by borders, game pixels, text and glyphs.
   Logical game placement/aspect remains unchanged. Percent is 80..100;
   horizontal/vertical offsets are -5..5 percent of the output surface. */
void PbrbSetScreenCalibration(int percent, int horizontal, int vertical);
void PbrbSetSmoothFiltering(int smooth);
void PbrbSpriteGame(PbrbTexture* tex, float x, float y, float w, float h, DWORD tint);

// Optional startup XMV, using the native decoder and bounded packet reads.
// Call on the D3D/main thread after input/preferences init, before AudioInit.
// Master volume is 0..10. Returns 1 on completion/skip, 0 if absent, -1 on
// failure; callers continue to the launcher and consume held skip input.
// Requires the RXDK xmv.lib in the linker's explicit dependencies.
int PbrbVideoPlayIntro(const char* path, int volume);
