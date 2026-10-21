/* Reuses the tested DarkDash stb_image configuration for legacy RXDK.
   Copy stb_image.h from DarkDash into /third_party prior to building. */
#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_SIMD
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_NO_STDIO
#define STBI_NO_HDR
#define STBI_NO_LINEAR
#define STBI_NO_THREAD_LOCALS
#define STBI_NO_FAILURE_STRINGS
#define STBI_ASSERT(x) ((void)0)
#include "stb_image.h"
#include "pbrb_stbi.h"
unsigned char* PbrbDecodeImage(const unsigned char* bytes, int length, int* w, int* h) {
    int channels = 0;
    if (!bytes || length <= 0 || !w || !h)return 0;
    return stbi_load_from_memory(bytes, length, w, h, &channels, 4);
}
void PbrbFreeDecoded(void* p) { if (p)stbi_image_free(p); }
