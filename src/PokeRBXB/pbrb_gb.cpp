#include "pbrb_gb.h"
#include <string.h>
/* Verified asset offsets from the user-supplied 1 MiB US/EU pokered ROMs.
   The data matches gfx/font/font.png, gfx/title/pokemon_logo.png,
   gfx/title/player.png, and gfx/title/{red,blue}_version.png after rgbgfx. */
#define GB_FONT       0x11A80u
#define GB_LOGO       0x11380u
#define GB_TEXT_EXTRA 0x12288u
#define GB_COPYRIGHT  0x120C8u
#define GB_GAMEFREAK   0x121F8u
#define GB_PLAYER     0x126A8u
#define GB_VERSION    0x6802Fu
#define GB_CHARMANDER 0x35C5Cu
#define GB_SQUIRTLE   0x35DB8u

static const unsigned char kPalette[4][4] = { {255,255,255,255},{170,170,170,255},
                                            {85,85,85,255},{0,0,0,255} };
static int Fits(const PbrbGb* g, unsigned off, unsigned n) {
    return g && g->rom && off <= g->bytes && n <= g->bytes - off;
}
/* GBC boot palette combinations: Red=13 (OBJ0=3, OBJ1/BG=4),
   Blue=11 (OBJ0=4, OBJ1/BG=28). Values are original RGB555 colors. */
static const unsigned short kGbcPalettes[2][3][4] = {
    {{0x7fff,0x421f,0x1cf2,0},{0x7fff,0x1bef,0x0200,0},{0x7fff,0x421f,0x1cf2,0}},
    {{0x7fff,0x7e8c,0x7c00,0},{0x7fff,0x421f,0x1cf2,0},{0x7fff,0x7e8c,0x7c00,0}}
};
static void PutLayerShade(PbrbGb* g, int x, int y, unsigned c, unsigned layer) {
    if (!g || !g->pixels || (unsigned)x >= PBRB_GB_W || (unsigned)y >= PBRB_GB_H)return;
    unsigned char* p = g->pixels + ((unsigned)y * PBRB_GB_W + (unsigned)x) * 4;
    if (g->colorMode && g->game >= 1 && g->game <= 2) {
        const unsigned rgb = kGbcPalettes[g->game - 1][layer < 3 ? layer : 0][c & 3u];
        const unsigned red = rgb & 31u, green = (rgb >> 5) & 31u, blue = (rgb >> 10) & 31u;
        p[0] = (unsigned char)((blue << 3) | (blue >> 2));
        p[1] = (unsigned char)((green << 3) | (green >> 2));
        p[2] = (unsigned char)((red << 3) | (red >> 2));
    }
    else {
        p[0] = kPalette[c & 3u][0]; p[1] = kPalette[c & 3u][1];
        p[2] = kPalette[c & 3u][2];
    }
    p[3] = 255;
}
static void PutShade(PbrbGb* g, int x, int y, int c) {
    PutLayerShade(g, x, y, (unsigned)c, 0);
}
static void Put(PbrbGb* g, int x, int y, int c) {
    if (!g || (unsigned)x >= PBRB_GB_W || (unsigned)y >= PBRB_GB_H)return;
    g->background[y * PBRB_GB_W + x] = (unsigned char)(c & 3);
    PutShade(g, x, y, (g->bgPalette >> (2u * (unsigned)(c & 3))) & 3u);
}
void PbrbGbSetColorMode(PbrbGb* g, int mode) { if (g)g->colorMode = (unsigned char)(mode ? 1 : 0); }
void PbrbGbSetPalettes(PbrbGb* g, unsigned bg, unsigned obj0, unsigned obj1) {
    if (!g)return;
    g->bgPalette = (unsigned char)bg; g->objPalette0 = (unsigned char)obj0; g->objPalette1 = (unsigned char)obj1;
}
void PbrbGbPixel(PbrbGb* g, int x, int y, unsigned index, unsigned layer) {
    if (!g)return;
    index &= 3u;
    if (!layer) { Put(g, x, y, (int)index); return; }
    if (!index)return;
    const unsigned p = layer == 2u ? g->objPalette1 : g->objPalette0;
    PutLayerShade(g, x, y, (p >> (index * 2u)) & 3u, layer);
}
void PbrbGbBitmap(PbrbGb* g, const unsigned char* indices, int width, int height,
    int x, int y, unsigned layer, int flipX) {
    if (!g || !indices || width <= 0 || height <= 0 || width > 160 || height > 144)return;
    for (int yy = 0; yy < height; ++yy)for (int xx = 0; xx < width; ++xx)
        PbrbGbPixel(g, x + xx, y + yy, indices[yy * width + (flipX ? width - 1 - xx : xx)], layer);
}
void PbrbGbCodes(PbrbGb* g, const unsigned char* t, int x, int y, int maxLength) {
    if (!g || !t || maxLength < 0)return;
    for (int i = 0; i < maxLength && t[i] != 0x50u; ++i)PbrbGbGlyph(g, t[i], x + i * 8, y);
}
static unsigned int Crc32(const unsigned char* p, unsigned n) {
    unsigned int c = 0xffffffffu;
    for (unsigned i = 0; i < n; i++) {
        c ^= p[i];
        for (int j = 0; j < 8; j++)c = (c >> 1) ^ ((c & 1) ? 0xedb88320u : 0);
    }
    return ~c;
}
/* Native form of home/uncompress.asm. The ROM stores two RLE/differential
   bit planes, not raw tiles. Bound both the bitstream and the 7x7 buffers. */
struct SpriteBits {
    const unsigned char* data;
    unsigned bytes, bit;
    int ok;
};
static unsigned SpriteRead(SpriteBits* b, unsigned count) {
    unsigned v = 0;
    if (!b->ok || count > 16u || b->bit > b->bytes * 8u ||
        count > b->bytes * 8u - b->bit) {
        b->ok = 0; return 0;
    }
    while (count--) {
        v = (v << 1) | ((b->data[b->bit >> 3] >> (7u - (b->bit & 7u))) & 1u);
        ++b->bit;
    }
    return v;
}
static int SpritePlane(SpriteBits* bits, unsigned char* plane, unsigned w, unsigned h) {
    const unsigned pairs = w * h * 4u;
    unsigned done = 0, literal = SpriteRead(bits, 1);
    memset(plane, 0, 7u * 56u);
    while (bits->ok && done < pairs) {
        if (literal) {
            unsigned p = SpriteRead(bits, 2);
            if (!p) { literal = 0; continue; }
            const unsigned column = done / (h * 4u);
            const unsigned y = done % h;
            const unsigned shift = 6u - 2u * ((done / h) & 3u);
            plane[column * h + y] |= (unsigned char)(p << shift);
            ++done;
        }
        else {
            unsigned ones = 0;
            while (bits->ok && SpriteRead(bits, 1)) {
                if (++ones >= 16u) { bits->ok = 0; break; }
            }
            if (!bits->ok)return 0;
            const unsigned run = ((1u << (ones + 1u)) - 1u) + SpriteRead(bits, ones + 1u);
            if (!bits->ok || run > pairs - done)return 0;
            done += run;
            literal = 1;
        }
    }
    return bits->ok && done == pairs;
}
static void SpriteDifferential(unsigned char* plane, unsigned w, unsigned h) {
    /* Differential state carries horizontally through a scanline, and
       resets at its left edge, even though storage is column-major. */
    for (unsigned y = 0; y < h; ++y) {
        unsigned last = 0;
        for (unsigned x = 0; x < w; ++x) {
            unsigned src = plane[x * h + y], dst = 0;
            for (int bit = 7; bit >= 0; --bit) {
                last ^= (src >> bit) & 1u;
                dst |= last << bit;
            }
            plane[x * h + y] = (unsigned char)dst;
        }
    }
}
int PbrbGbDecodePicture(const PbrbGb* g, unsigned offset, unsigned char* out, size_t outBytes) {
    if (!g || !out || outBytes < 56u * 56u || !Fits(g, offset, 1u))return 0;
    SpriteBits bits; bits.data = g->rom + offset; bits.bytes = (unsigned)(g->bytes - offset);
    if (bits.bytes > 2048u)bits.bytes = 2048u;
    bits.bit = 0; bits.ok = 1;
    const unsigned w = SpriteRead(&bits, 4), h = SpriteRead(&bits, 4) * 8u;
    if (!w || w > 7u || !h || h > 56u)return 0;
    unsigned char planes[2][7 * 56];
    const unsigned order = SpriteRead(&bits, 1);
    if (!SpritePlane(&bits, planes[order], w, h))return 0;
    unsigned mode = SpriteRead(&bits, 1); if (mode)mode += SpriteRead(&bits, 1);
    if (!SpritePlane(&bits, planes[order ^ 1u], w, h))return 0;
    SpriteDifferential(planes[order], w, h);
    if (mode != 1u)SpriteDifferential(planes[order ^ 1u], w, h);
    if (mode)for (unsigned i = 0; i < w * h; ++i)planes[order ^ 1u][i] ^= planes[order][i];
    memset(out, 0, 56u * 56u);
    const unsigned left = ((8u - w) / 2u) * 8u, top = 56u - h;
    for (unsigned y = 0; y < h; ++y)for (unsigned x = 0; x < w * 8u; ++x) {
        const unsigned at = (x / 8u) * h + y, bit = 7u - (x & 7u);
        out[(top + y) * 56u + left + x] = (unsigned char)(((planes[0][at] >> bit) & 1u) | (((planes[1][at] >> bit) & 1u) << 1));
    }
    return 1;
}
int PbrbGbDecodePokemon(const PbrbGb* g, unsigned species, int back, unsigned char* out, size_t bytes) {
    if (!g || !species || species > 190u || !Fits(g, 0x41024u + species - 1u, 1u))return 0;
    const unsigned dex = g->rom[0x41024u + species - 1u];
    if (!dex || dex > 151u)return 0;
    const unsigned header = species == 0x15u ? 0x425bu : 0x383deu + (dex - 1u) * 28u;
    if (!Fits(g, header, 28u))return 0;
    const unsigned p = header + (back ? 13u : 11u);
    const unsigned address = g->rom[p] | ((unsigned)g->rom[p + 1u] << 8);
    const unsigned bank = species == 0x15u ? 1u : (species < 0x1fu ? 9u : (species < 0x4au ? 10u : (species < 0x74u ? 11u : (species < 0x99u ? 12u : 13u))));
    if (address < 0x4000u || address >= 0x8000u)return 0;
    return PbrbGbDecodePicture(g, bank * 0x4000u + address - 0x4000u, out, bytes);
}
static int LoadTitleMon(PbrbGb* g) {
    const unsigned off = g->game == 1 ? GB_CHARMANDER : GB_SQUIRTLE;
    const unsigned bytes = g->game == 1 ? 256u : 215u;
    if (!Fits(g, off, bytes))return 0;
    if (Crc32(g->rom + off, bytes) !=
        (g->game == 1 ? 0xab3c1bc2u : 0xa52313b9u))return 0;
    SpriteBits bits;
    bits.data = g->rom + off; bits.bytes = bytes; bits.bit = 0; bits.ok = 1;
    const unsigned w = SpriteRead(&bits, 4), h = SpriteRead(&bits, 4) * 8u;
    if (!w || w > 7u || !h || h > 56u)return 0;
    unsigned char planes[2][7 * 56];
    const unsigned order = SpriteRead(&bits, 1);
    if (!SpritePlane(&bits, planes[order], w, h))return 0;
    unsigned mode = SpriteRead(&bits, 1);
    if (mode)mode += SpriteRead(&bits, 1);
    if (!SpritePlane(&bits, planes[order ^ 1u], w, h))return 0;
    SpriteDifferential(planes[order], w, h);
    if (mode != 1u)SpriteDifferential(planes[order ^ 1u], w, h);
    if (mode)for (unsigned i = 0; i < w * h; ++i)
        planes[order ^ 1u][i] ^= planes[order][i];
    memset(g->titleMon, 0, sizeof(g->titleMon));
    /* home/pics.asm: round up the horizontal tile offset and bottom-align
       within the 7x7 background picture. */
    const unsigned left = ((8u - w) / 2u) * 8u, top = 56u - h;
    for (unsigned y = 0; y < h; ++y)for (unsigned x = 0; x < w * 8u; ++x) {
        const unsigned p = (x / 8u) * h + y, bit = 7u - (x & 7u);
        g->titleMon[(top + y) * 56u + left + x] = (unsigned char)
            (((planes[0][p] >> bit) & 1u) | (((planes[1][p] >> bit) & 1u) << 1));
    }
    return 1;
}
int PbrbGbInit(PbrbGb* g, const unsigned char* r, size_t n, int game,
    unsigned char* pixels, size_t pixelBytes) {
    if (!g || !r || n != 1048576u || (game != 1 && game != 2) || !pixels || pixelBytes < PBRB_GB_RGBA_BYTES)
        return 0;
    memset(g, 0, sizeof(*g)); g->rom = r; g->bytes = n; g->game = game; g->pixels = pixels;
    PbrbGbSetPalettes(g, 0xe4u, 0xd0u, 0xe4u);
    if (!Fits(g, GB_LOGO, 1792) || !Fits(g, GB_FONT, 1024) || !Fits(g, GB_PLAYER, 560) ||
        !Fits(g, GB_VERSION, 80) || !Fits(g, GB_TEXT_EXTRA, 512) ||
        !Fits(g, GB_COPYRIGHT, 304) || !Fits(g, GB_GAMEFREAK, 144))return 0;
    /* CRCs independently established from the original rgbgfx binary payloads
       inside verified pokered ROM images. Fail closed for another revision. */
    if (Crc32(r + GB_LOGO, 1792) != 0x9066d870UL ||
        Crc32(r + GB_FONT, 1024) != 0x6a5d81c9UL ||
        Crc32(r + GB_TEXT_EXTRA, 512) != 0x04a17cb9UL ||
        Crc32(r + GB_COPYRIGHT, 304) != 0x78b25a88UL ||
        Crc32(r + GB_GAMEFREAK, 144) != 0x2bb6608eUL ||
        Crc32(r + GB_PLAYER, 560) != 0x901fcfdfUL ||
        Crc32(r + GB_VERSION, game == 1 ? 80u : 64u) !=
        (game == 1 ? 0xc69e09fbUL : 0xcacf91dbUL))return 0;
    if (!LoadTitleMon(g))return 0;
    PbrbGbClear(g, 0); return 1;
}
void PbrbGbClear(PbrbGb* g, int shade) {
    if (!g || !g->pixels)return;
    for (int y = 0; y < PBRB_GB_H; ++y)for (int x = 0; x < PBRB_GB_W; ++x)Put(g, x, y, shade);
}
void PbrbGbTile(PbrbGb* g, unsigned off, int depth, int x, int y) {
    if (!Fits(g, off, (depth == 1) ? 8u : 16u) || !g->pixels)return;
    const unsigned char* t = g->rom + off;
    for (int row = 0; row < 8; row++) {
        unsigned lo = t[row * (depth == 1 ? 1 : 2)];
        unsigned hi = depth == 1 ? 0 : t[row * 2 + 1];
        for (int col = 0; col < 8; col++) {
            unsigned b = (unsigned)(7 - col);
            int c = (int)(((lo >> b) & 1u) | (((hi >> b) & 1u) << 1));
            /* GB font 1bpp white=0, ink=1 (foreground black). */
            if (depth == 1)c = c ? 3 : 0;
            Put(g, x + col, y + row, c);
        }
    }
}
void PbrbGbIntroTile(PbrbGb* g, unsigned off, int x, int y, int sprite) {
    if (sprite) { PbrbGbSpriteTile(g, off, x, y, 0xe4u, 0x80u); return; }
    PbrbGbTile(g, off, 2, x, y);
}
void PbrbGbSpriteTile(PbrbGb* g, unsigned off, int x, int y,
    unsigned palette, unsigned attributes) {
    if (!Fits(g, off, 16u) || !g->pixels)return;
    const unsigned char* tile = g->rom + off;
    for (int yy = 0; yy < 8; yy++) {
        const unsigned row = (attributes & 0x40u) ? 7u - (unsigned)yy : (unsigned)yy;
        const unsigned lo = tile[row * 2u], hi = tile[row * 2u + 1u];
        for (int xx = 0; xx < 8; xx++) {
            const unsigned shift = (attributes & 0x20u) ? (unsigned)xx : 7u - (unsigned)xx;
            const unsigned idx = ((lo >> shift) & 1u) | (((hi >> shift) & 1u) << 1u);
            const int px = x + xx, py = y + yy;
            if (idx == 0u)continue;
            if ((unsigned)px >= PBRB_GB_W || (unsigned)py >= PBRB_GB_H)continue;
            if ((attributes & 0x80u) && g->background[py * PBRB_GB_W + px] != 0u)continue;
            PutLayerShade(g, px, py, (palette >> (idx * 2u)) & 3u, (attributes & 0x10u) ? 2u : 1u);
        }
    }
}
void PbrbGbApplyBGPalette(PbrbGb* g, unsigned palette) {
    if (!g || !g->pixels)return;
    for (int y = 0; y < PBRB_GB_H; ++y)for (int x = 0; x < PBRB_GB_W; ++x)
        PutShade(g, x, y, (int)((palette >> (2u * g->background[y * PBRB_GB_W + x])) & 3u));
}
void PbrbGbDrawOam(PbrbGb* g, const PbrbGbOam* entries, unsigned count,
    unsigned palette0, unsigned palette1) {
    if (!g || !g->pixels || !entries)return;
    if (count > 40u)count = 40u;
    /* DMG selects the first ten Y-overlapping OAM entries per scanline.
       Within that set, lower X wins, then lower OAM index. A hidden
       priority sprite still prevents a lower-priority sprite showing. */
    for (int y = 0; y < PBRB_GB_H; ++y) {
        unsigned selected[10], n = 0;
        for (unsigned i = 0; i < count && n < 10u; ++i)
            if (y + 16 >= entries[i].y && y + 16 < entries[i].y + 8)
                selected[n++] = i;
        for (int x = 0; x < PBRB_GB_W; ++x) {
            int best = -1;
            unsigned color = 0, bestX = 256u;
            for (unsigned s = 0; s < n; ++s) {
                const PbrbGbOam* o = entries + selected[s];
                if (o->x >= bestX || x + 8 < o->x || x + 8 >= o->x + 8 ||
                    !Fits(g, o->offset, 16u))continue;
                unsigned row = (unsigned)(y + 16 - o->y);
                unsigned bit = 7u - (unsigned)(x + 8 - o->x);
                if (o->attributes & 0x40u)row = 7u - row;
                if (o->attributes & 0x20u)bit = 7u - bit;
                const unsigned char* p = g->rom + o->offset + row * 2u;
                const unsigned c = ((p[0] >> bit) & 1u) | (((p[1] >> bit) & 1u) << 1);
                if (c) { best = (int)selected[s]; bestX = o->x; color = c; }
            }
            if (best < 0)continue;
            const unsigned attr = entries[best].attributes;
            if ((attr & 0x80u) && g->background[y * PBRB_GB_W + x])continue;
            const unsigned pal = (attr & 0x10u) ? palette1 : palette0;
            PutLayerShade(g, x, y, (pal >> (color * 2u)) & 3u, (attr & 0x10u) ? 2u : 1u);
        }
    }
}
void PbrbGbTiles(PbrbGb* g, unsigned off, int depth, int x, int y, int cols, int rows) {
    if (!g || cols < 0 || rows < 0)return;
    for (int j = 0; j < rows; j++)for (int i = 0; i < cols; i++)
        PbrbGbTile(g, off + (unsigned)(j * cols + i) * (depth == 1 ? 8u : 16u), depth, x + 8 * i, y + 8 * j);
}
static unsigned char GbCode(unsigned char c) {
    if (c >= 'A' && c <= 'Z')return (unsigned char)(0x80 + c - 'A');
    if (c >= 'a' && c <= 'z')return (unsigned char)(0xA0 + c - 'a');
    if (c >= '0' && c <= '9')return (unsigned char)(0xF6 + c - '0');
    if (c == ' ')return 0x7F;
    if (c == '!')return 0xE7;
    if (c == '?')return 0xE6;
    if (c == '.')return 0xE8;
    if (c == ',')return 0xF4;
    if (c == '-')return 0xE3;
    if (c == '\'')return 0xE0;
    if (c == ':')return 0x9C;
    if (c == '/')return 0xF3;
    return 0x7F;
}
void PbrbGbGlyph(PbrbGb* g, unsigned char code, int x, int y) {
    if (!g || !g->pixels)return;
    if (code == 0x7F) { for (int j = 0; j < 8; j++)for (int i = 0; i < 8; i++)Put(g, x + i, y + j, 0); return; }
    /* GB vFont is tile $80..$FF, but only 128 characters are stored here;
       font.1bpp source is 128x64 = 128 tiles. */
    if (code >= 0x80u) { PbrbGbTile(g, GB_FONT + 8u * (unsigned)(code - 0x80u), 1, x, y); return; }
    /* Characters below $80 use 2bpp supplementary font atlas loaded into
       vChars2 at the appropriate time; $79..$7e are box glyphs. */
    if (code >= 0x60u && code < 0x80u)PbrbGbTile(g, GB_TEXT_EXTRA + 16u * (code - 0x60u), 2, x, y);
}
void PbrbGbString(PbrbGb* g, const char* s, int x, int y) {
    if (!g || !s)return;
    for (int col = 0; s[col] && col < 48; col++) {
        if (x + col * 8 >= 160)break;
        if (s[col] == '\n') { y += 16; x -= 8 * (col + 1); continue; }
        PbrbGbGlyph(g, GbCode((unsigned char)s[col]), x + col * 8, y);
    }
}
void PbrbGbTextBox(PbrbGb* g, int x, int y, int cols, int rows) {
    if (!g || cols < 2 || rows < 2)return;
    /* Border tiles match the original charmap ($79..$7e), rather than
       Direct3D lines or launcher UI artwork. */
    for (int yy = 0; yy < rows; yy++)for (int xx = 0; xx < cols; xx++) {
        unsigned char c = 0x7F;
        if (yy == 0)c = (xx == 0) ? 0x79 : (xx == cols - 1 ? 0x7B : 0x7A);
        else if (yy == rows - 1)c = (xx == 0) ? 0x7D : (xx == cols - 1 ? 0x7E : 0x7A);
        else if (xx == 0 || xx == cols - 1)c = 0x7C;
        PbrbGbGlyph(g, c, x + 8 * xx, y + 8 * yy);
    }
}
void PbrbGbTitle(PbrbGb* g, int logoScrollY, int versionX, int showVersion, int showPlayer,
    int monScrollX, int ballY, int showMon) {
    if (!g || !g->pixels)return;
    PbrbGbClear(g, 0);
    /* Original Pokémon logo: 16x7 tiles at hlcoord 2,1, and 640/480
       Game Boy viewport pixels are scaled externally, never in this module. */
    PbrbGbTiles(g, GB_LOGO, 2, 16, 8 - logoScrollY, 16, 7);
    /* During the bounce the window begins at WY=64. It covers any logo
       pixels below the top band and holds the lower title picture still. */
    for (int yy = 64; yy < PBRB_GB_H; ++yy)
        for (int xx = 0; xx < PBRB_GB_W; ++xx)Put(g, xx, yy, 0);
    /* The original scrolls only the picture band at LY72..135, using the
       256-pixel BG map wrap. OAM, logo, version and copyright stay fixed. */
    if (showMon)for (int my = 0; my < 56; ++my)for (int mx = 0; mx < 56; ++mx)
        Put(g, (40 + mx - monScrollX + 256) & 255, 80 + my, g->titleMon[my * 56 + mx]);
    if (showVersion) {
        /* Version_GFX is a character set, not a linear text bitmap.
           engine/movie/title.asm places the Red $60,$61,space,$65..$69
           or Blue $61..$68 glyph IDs at hlcoord 7,8. */
        for (int k = 0; k < 8; k++) {
            int idx = g->game == 1 ? (k < 2 ? k : (k == 2 ? -1 : k + 2)) : k;
            if (idx >= 0)PbrbGbTile(g, GB_VERSION + 8u * (unsigned)idx, 1,
                versionX + 8 * k, 64);
        }
    }
    if (showPlayer) {
        /* DrawPlayerCharacter uses OAM, with transparent index zero.
           The title relocates sprite 10 (the ball) to raw OAM Y=$74. */
        for (int tile = 34; tile >= 0; --tile) {
            const int px = 82 + (tile % 5) * 8;
            const int py = tile == 10 ? ballY - 16 : 80 + (tile / 5) * 8;
            PbrbGbSpriteTile(g, GB_PLAYER + 16u * (unsigned)tile, px, py, 0xe4u, 0);
        }
    }
    /* engine/movie/title.asm .tileScreenCopyrightTiles, at hlcoord 2,17.
       These 14 glyphs share the copyright/logo ROM asset bank. */
    static const unsigned char rights[16] = {
        0x41,0x42,0x43,0x42,0x44,0x42,0x45,0x46,
        0x47,0x48,0x49,0x4a,0x4b,0x4c,0x4d,0x4e
    };
    for (int n = 0; n < 16; n++) {
        unsigned ch = rights[n];
        if (ch <= 0x45)PbrbGbTile(g, GB_COPYRIGHT + 16u * (ch - 0x41), 2, 16 + n * 8, 136);
        else if (ch <= 0x4e)PbrbGbTile(g, GB_GAMEFREAK + 16u * (ch - 0x46), 2, 16 + n * 8, 136);
    }
}
void PbrbGbGameMenu(PbrbGb* g, int selected, int hasSave) {
    if (!g)return;
    PbrbGbClear(g, 0);
    PbrbGbTextBox(g, 0, 0, 15, hasSave ? 8 : 6);
    if (hasSave)PbrbGbString(g, "CONTINUE", 16, 16);
    PbrbGbString(g, "NEW GAME", 16, hasSave ? 32 : 16);
    PbrbGbString(g, "OPTION", 16, hasSave ? 48 : 32);
    PbrbGbGlyph(g, 0xED, 8, 16 + selected * 16);
}
void PbrbGbOptions(PbrbGb* g, int selected, int speed, int anim, int style) {
    if (!g)return;
    PbrbGbClear(g, 0);
    /* engine/menus/main_menu.asm DisplayOptionMenu — original layout:
       three 20x5 GB text boxes at y=0,5,10 tiles, CANCEL at tile (2,16).
       Cursor rows y=3,8,13,16 tiles and speed columns x=1,7,14. */
    PbrbGbTextBox(g, 0, 0, 20, 5);
    PbrbGbTextBox(g, 0, 40, 20, 5);
    PbrbGbTextBox(g, 0, 80, 20, 5);
    PbrbGbString(g, "TEXT SPEED", 8, 8);
    PbrbGbString(g, " FAST  MEDIUM SLOW", 8, 24);
    PbrbGbString(g, "BATTLE ANIMATION", 8, 48);
    PbrbGbString(g, " ON       OFF", 8, 64);
    PbrbGbString(g, "BATTLE STYLE", 8, 88);
    PbrbGbString(g, " SHIFT    SET", 8, 104);
    PbrbGbString(g, "CANCEL", 16, 128);
    /* Original strings reserve a blank tile before every value. Keep the
       cursor there, with hollow markers on the inactive option rows. */
    const int xs[4] = { speed == 0 ? 8 : (speed == 1 ? 56 : 112),
        anim ? 8 : 80, style ? 80 : 8, 8 };
    const int ys[4] = { 24, 64, 104, 128 };
    const int active = selected >= 0 && selected < 3 ? selected : 3;
    for (int row = 0; row < 4; ++row)
        PbrbGbGlyph(g, row == active ? 0xed : 0xec, xs[row], ys[row]);
}
