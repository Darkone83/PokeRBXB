#include "pbrb_texture.h"
#include "pbrb_stbi.h"
#include <xgraphics.h>
#include <stdlib.h>
#include <string.h>

static int Np2(int n) { int p = 1; while (p < n && p < 4096)p <<= 1; return p; }
void PbrbReleaseTexture(PbrbTexture* t) {
    if (!t)return;
    if (t->gpu)t->gpu->Release();
    memset(t, 0, sizeof(*t));
}
int PbrbUploadBGRA(const unsigned char* bgra, int w, int h, PbrbTexture* t) {
    IDirect3DTexture8* tex = NULL; D3DLOCKED_RECT lock;
    unsigned char* temp = NULL; int pw, ph; HRESULT hr;
    if (!t || !bgra || w <= 0 || h <= 0 || w > 2048 || h > 2048)return 0;
    pw = Np2(w); ph = Np2(h);
    temp = (unsigned char*)malloc((size_t)pw * ph * 4);
    if (!temp)return 0;
    memset(temp, 0, (size_t)pw * ph * 4);
    for (int y = 0; y < h; ++y)memcpy(temp + (size_t)y * pw * 4, bgra + (size_t)y * w * 4, (size_t)w * 4);
    hr = PbrbDevice()->CreateTexture(pw, ph, 1, 0, D3DFMT_A8R8G8B8, D3DPOOL_MANAGED, &tex);
    if (FAILED(hr)) { free(temp); return 0; }
    hr = tex->LockRect(0, &lock, NULL, 0);
    if (FAILED(hr)) { tex->Release(); free(temp); return 0; }
    XGSwizzleRect(temp, pw * 4, NULL, lock.pBits, pw, ph, NULL, 4);
    tex->UnlockRect(0); free(temp);
    PbrbReleaseTexture(t);
    t->gpu = tex; t->width = w; t->height = h; t->padded_width = pw; t->padded_height = ph;
    return 1;
}
int PbrbLoadPNG(const char* path, PbrbTexture* t) {
    HANDLE f; DWORD len, got = 0; unsigned char* file = NULL, * rgba = NULL, * bgra = NULL;
    int w = 0, h = 0, ok = 0;
    if (!path || !t)return 0;
    f = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (f == INVALID_HANDLE_VALUE)return 0;
    len = GetFileSize(f, NULL);
    if (len == INVALID_FILE_SIZE || len == 0 || len > 16u * 1024u * 1024u)goto end;
    file = (unsigned char*)malloc(len);
    if (!file)goto end;
    if (!ReadFile(f, file, len, &got, NULL) || got != len)goto end;
    rgba = PbrbDecodeImage(file, (int)len, &w, &h);
    if (!rgba || w <= 0 || h <= 0 || w > 2048 || h > 2048)goto end;
    bgra = (unsigned char*)malloc((size_t)w * h * 4);
    if (!bgra)goto end;
    for (int i = 0; i < w * h; ++i) {
        bgra[i * 4 + 0] = rgba[i * 4 + 2]; bgra[i * 4 + 1] = rgba[i * 4 + 1];
        bgra[i * 4 + 2] = rgba[i * 4 + 0]; bgra[i * 4 + 3] = rgba[i * 4 + 3];
    }
    ok = PbrbUploadBGRA(bgra, w, h, t);
end:
    if (bgra)free(bgra);
    if (rgba)PbrbFreeDecoded(rgba);
    if (file)free(file);
    CloseHandle(f);
    return ok;
}
