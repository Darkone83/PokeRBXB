#ifndef PBRB_GB_RENDERER_H_INCLUDED
#define PBRB_GB_RENDERER_H_INCLUDED
/* Game Boy pixel renderer: all game glyphs and art come from the user's ROM.
   No DDF/launcher fonts, no generated PNGs, no Windows/Xbox dependencies. */
#include <stddef.h>
#define PBRB_GB_W 160
#define PBRB_GB_H 144
#define PBRB_GB_RGBA_BYTES (PBRB_GB_W*PBRB_GB_H*4)

struct PbrbGb {
    const unsigned char* rom;
    size_t bytes;
    unsigned char* pixels; /* caller-owned BGRA, 160x144 */
    int game; /* 1 Red, 2 Blue */
    /* Preserve original BG color IDs for OAM_PRIO; the displayed palette
       shade alone is not enough to determine sprite priority. */
    unsigned char background[PBRB_GB_W * PBRB_GB_H];
    unsigned char titleMon[56 * 56]; /* decoded/padded current title mon */
    unsigned char colorMode; /* 0 original grayscale, 1 actual GBC compatibility palette */
    unsigned char bgPalette, objPalette0, objPalette1;
};
struct PbrbGbOam {
    unsigned offset; /* ROM address of the 8x8 2bpp tile */
    unsigned char y, x, attributes;
};

/* Fingerprints direct ROM assets used for verified US/EU Red/Blue revision. */
int PbrbGbInit(PbrbGb* gb, const unsigned char* rom, size_t bytes, int game,
    unsigned char* pixels, size_t pixelBytes);
void PbrbGbClear(PbrbGb* gb, int paletteIndex);
void PbrbGbTile(PbrbGb* gb, unsigned offset, int depth, int x, int y);
/* Intro BGP is $e4: white, light, dark, black. Sprite index 0 is transparent. */
void PbrbGbIntroTile(PbrbGb* gb, unsigned offset, int x, int y, int sprite);
/* Palette is the actual OBP register byte; attributes are Game Boy OAM
   bits ($20 X flip, $40 Y flip, $80 behind nonzero BG colors). */
void PbrbGbSpriteTile(PbrbGb* gb, unsigned offset, int x, int y,
    unsigned palette, unsigned attributes);
void PbrbGbDrawOam(PbrbGb* gb, const PbrbGbOam* entries, unsigned count,
    unsigned palette0, unsigned palette1);
void PbrbGbApplyBGPalette(PbrbGb* gb, unsigned palette);
void PbrbGbTiles(PbrbGb* gb, unsigned offset, int depth, int x, int y, int columns, int rows);
void PbrbGbGlyph(PbrbGb* gb, unsigned char code, int x, int y);
void PbrbGbString(PbrbGb* gb, const char* text, int x, int y);
void PbrbGbTextBox(PbrbGb* gb, int x, int y, int columns, int rows);
/* Shared indexed renderer for the native overworld and scenes. layer 0 is
   background/window, 1 is OBP0, and 2 is OBP1. OBJ index zero is transparent. */
void PbrbGbSetColorMode(PbrbGb* gb, int mode);
void PbrbGbSetPalettes(PbrbGb* gb, unsigned bg, unsigned obj0, unsigned obj1);
void PbrbGbPixel(PbrbGb* gb, int x, int y, unsigned index, unsigned layer);
void PbrbGbBitmap(PbrbGb* gb, const unsigned char* indices, int width, int height,
    int x, int y, unsigned layer, int flipX);
/* Bounded original sprite decompression, returning a bottom-aligned 56x56
   indexed image; no ROM graphics are duplicated into the executable. */
int PbrbGbDecodePicture(const PbrbGb* gb, unsigned offset, unsigned char* out,
    size_t outBytes);
/* Decode the original internal-species front/back pointer and dimensions. */
int PbrbGbDecodePokemon(const PbrbGb* gb, unsigned species, int back,
    unsigned char* out, size_t outBytes);
void PbrbGbCodes(PbrbGb* gb, const unsigned char* text, int x, int y, int maxLength);
/* scrollY is hSCY: positive values move BG pixels upward; the title window
   keeps the mon/trainer/copyright stationary below scanline 64. */
void PbrbGbTitle(PbrbGb* gb, int logoScrollY, int versionX, int showVersion, int showPlayer,
    int monScrollX = 0, int ballY = 0x74, int showMon = 1);
void PbrbGbGameMenu(PbrbGb* gb, int selection, int hasSave = 0);
void PbrbGbOptions(PbrbGb* gb, int selection, int textSpeed, int battleAnimation, int battleStyle);
#endif /* PBRB_GB_RENDERER_H_INCLUDED */
