#pragma once
#pragma once
#include "pbrb_video.h"
// DDF1 v1, byte-compatible with the existing DarkDash font encoder.
// Three font tiers: 0 small, 1 medium, 2 large.
int PbrbFontLoad(const char* path);
void PbrbFontShutdown();
int PbrbTextWidth(const char* text, int tier, float scale);
void PbrbText(float x, float y, const char* text, int tier, float scale, DWORD color);
void PbrbTextCenter(float center_x, float y, const char* text, int tier, float scale, DWORD color);
int PbrbFontReady();
