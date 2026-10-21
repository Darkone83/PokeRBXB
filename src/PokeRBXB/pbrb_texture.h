#pragma once
#pragma once
#include "pbrb_video.h"
int PbrbUploadBGRA(const unsigned char* bgra, int width, int height, PbrbTexture* tex);
int PbrbLoadPNG(const char* path, PbrbTexture* tex);
void PbrbReleaseTexture(PbrbTexture* tex);
