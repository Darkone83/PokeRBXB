#include "pbrb_intro.h"
#include <string.h>
/* Original ROM tiles/OAM and source-defined scripts, including the
   six-tile-row VBlank transfers and separate palette/OAM timing.
   The three fade edges retain the preceding palette above their original
   measured scanline boundary; no CPU/PPU emulation is introduced. */
#define GF_SPLASH 0x41959u
#define GENGAR_GFX 0x41A99u
#define GENGAR_MAP 0x79B8Du
#define FRONT_1 0x42099u
#define FALLING_STAR 0x70190u
#define GF_START PBRB_INTRO_STAR_START
#define BATTLE_START PBRB_INTRO_BATTLE_START
#define INTRO_END PBRB_INTRO_END
static unsigned Crc(const unsigned char* b, unsigned n) {
    unsigned c = 0xffffffffu;
    for (unsigned i = 0; i < n; i++) {
        c ^= b[i];
        for (int j = 0; j < 8; j++) c = (c >> 1) ^ ((c & 1u) ? 0xedb88320u : 0);
    }
    return ~c;
}
int PbrbIntroAssetsValid(const unsigned char* r, unsigned bytes, unsigned game) {
    if (!r || bytes != 1048576u || (game != 1u && game != 2u))return 0;
    if (Crc(r + GF_SPLASH, 320u) != 0xff7cdac2u)return 0;
    if (Crc(r + GENGAR_GFX, 0x600u) != 0x7562dfbau)return 0;
    if (Crc(r + FRONT_1, 1728u) != (game == 1u ? 0x7a6c8bc3u : 0x8367dfa5u))return 0;
    if (Crc(r + FALLING_STAR, 16u) != 0xb6b1f4fdu)return 0;
    if (Crc(r + GENGAR_MAP, 3u * 49u) != 0x637fab93u)return 0;
    if (Crc(r + 0x70140u, 80u) != 0xd074c412u ||
        Crc(r + 0x700feu, 32u) != 0x0d156277u ||
        Crc(r + 0x7871eu, 16u) != 0x8716ca13u ||
        Crc(r + 0x7881eu, 16u) != 0x802c2477u)return 0;
    return 1;
}
void PbrbIntroReset(PbrbIntro* t, unsigned game) {
    if (!t)return;
    memset(t, 0, sizeof(*t)); t->game = game;
}
void PbrbIntroTick(PbrbIntro* t) { if (t && t->frame < INTRO_END)++t->frame; }
int PbrbIntroComplete(const PbrbIntro* t) { return t && t->frame >= INTRO_END; }
int PbrbIntroCanSkip(const PbrbIntro* t) { return t && t->frame >= GF_START; }
/* CopyTileIDs consumes 7 columns per row: the maps are row-major.
   The graphics were deduplicated from a column-major atlas; this does
   not transpose the tilemap itself. 0 is a
   real indexed tile, not transparent framebuffer pixels. */
static void Gengar(PbrbGb* gb, int variant, int offsetX, int transferFrame = -1) {
    if (variant < 0 || variant>2)return;
    for (int x = 0; x < 7; x++)for (int y = 0; y < 7; y++) {
        int visible = variant;
        if (transferFrame >= 0) {
            /* VBlank's AutoBgMapTransfer copies six tile rows per frame.
               Gengar spans rows7..13, so its upper and lower portions
               reach VRAM in different phases of the three-frame cycle.
               intro.asm changes the shadow map at 0,220,258,337. */
            const int phase = y < 5 ? 0 : 1;
            const int lastCopy = transferFrame - (transferFrame + 3 - phase) % 3;
            if (lastCopy < 0)continue;
            visible = lastCopy >= 337 ? 0 : (lastCopy >= 258 ? 2 : (lastCopy >= 220 ? 1 : 0));
        }
        const unsigned char* map = gb->rom + GENGAR_MAP + (unsigned)visible * 49u;
        unsigned t = map[y * 7 + x];
        if (t > 0x5Eu)continue;
        PbrbGbIntroTile(gb, GENGAR_GFX + 16u * t, 104 + x * 8 - offsetX, 56 + y * 8, 0);
    }
}
/* Original nidorino/jigglypuff frames are 6x6 column-major OAM tiles. */
static void Front(PbrbGb* gb, int frame, int x, int y, unsigned palette = 0xe4u) {
    if (frame < 0)frame = 0;
    if (frame > 2)frame = 2;
    unsigned base = FRONT_1 + (unsigned)frame * 576u;
    for (int col = 0; col < 6; col++)for (int row = 0; row < 6; row++) {
        PbrbGbSpriteTile(gb, base + 16u * (unsigned)(col * 6 + row),
            x + 8 * col, y + 8 * row, palette, 0x80u);
    }
}
static void Bars(PbrbGb* gb) {
    /* ldpal packs its arguments from bits 7..6 down to 1..0. The source
       BLACK,DARK,LIGHT,WHITE is $e4, not a reversed palette. Tile 0 is
       white; IntroDrawBlackBars uses tile 1 for the four black rows. */
    PbrbGbClear(gb, 0);
    for (int row = 0; row < 4; ++row)for (int col = 0; col < 20; ++col) {
        PbrbGbIntroTile(gb, GENGAR_GFX + 16u, col * 8, row * 8, 0);
        PbrbGbIntroTile(gb, GENGAR_GFX + 16u, col * 8, 112 + row * 8, 0);
    }
}
static unsigned StarTile(unsigned tile) {
    if (tile >= 0x80u && tile <= 0x93u)return GF_SPLASH + (tile - 0x80u) * 16u;
    if (tile == 0xa0u)return 0x7871eu; /* MoveAnimationTiles1 tile 3 */
    if (tile == 0xa1u)return 0x7881eu; /* MoveAnimationTiles1 tile 19 */
    return FALLING_STAR;
}
static void DrawGameFreak(PbrbGb* gb, unsigned frame) {
    Bars(gb);
    PbrbGbOam oam[40];
    memset(oam, 0, sizeof(oam));
    for (unsigned i = 0; i < 40u; ++i)oam[i].y = 160;
    /* dbsprite writes raw OAM coordinates. PbrbGbDrawOam applies the
       hardware's -8 X / -16 Y offsets, ten-sprite scanline limit, flips,
       color-zero transparency, BG priority and both OBP palettes. */
    for (unsigned i = 0; i < 16u; ++i) {
        const unsigned char* src = gb->rom + 0x70140u + 4u * i;
        oam[24u + i].y = src[0]; oam[24u + i].x = src[1];
        oam[24u + i].offset = StarTile(src[2]);
        oam[24u + i].attributes = src[3];
    }
    unsigned pal0 = 0xf9u, pal1 = 0xa4u;
    if (frame < 40u) {
        for (unsigned i = 0; i < 4u; ++i) {
            const unsigned char* src = gb->rom + 0x70180u + 4u * i;
            oam[i].y = (unsigned char)(src[0] + 4u * (frame + 1u));
            oam[i].x = (unsigned char)(src[1] - 4u * (frame + 1u));
            oam[i].offset = StarTile(src[2]); oam[i].attributes = src[3];
        }
    }
    /* Palette writes take effect in the current display frame; OAM
       movement reaches the display through the following DMA. At the
       flash/wave boundaries the palette therefore leads OAM by one. */
    if (frame + 1u >= 40u) {
        unsigned flashes = (frame + 1u - 40u) / 10u + 1u;
        if (flashes > 3u)flashes = 3u;
        while (flashes--)pal0 = ((pal0 >> 2) | (pal0 << 6)) & 0xffu;
    }
    if (frame >= 70u) {
        for (unsigned i = 0; i < 24u; ++i) {
            oam[i].x = oam[i].y = 0;
            oam[i].offset = FALLING_STAR; oam[i].attributes = 0x90u;
        }
        unsigned active = 0;
        for (unsigned wave = 0; wave < 6u; ++wave) {
            const unsigned at = 70u + wave * 24u;
            if (frame < at)break;
            if (wave < 4u) {
                const unsigned char* coords = gb->rom + 0x700feu + wave * 8u;
                for (unsigned i = 0; i < 4u; ++i) {
                    oam[20u + i].y = coords[i * 2u]; oam[20u + i].x = coords[i * 2u + 1u];
                }
                /* Match the original +=6 quirk; the extra OAM entries
                   remain off the left edge of the screen. */
                if (active != 24u)active += 6u;
            }
            unsigned steps = (frame - at) / 3u + 1u;
            if (steps > 8u)steps = 8u;
            for (unsigned step = 0; step < steps; ++step) {
                for (unsigned i = 24u - active; i < 24u; ++i)++oam[i].y;
            }
            if (frame < at + 24u)break;
            for (unsigned i = 0; i < 20u; ++i)oam[i] = oam[i + 4u];
        }
    }
    if (frame + 1u >= 70u) {
        unsigned steps = (frame + 1u - 70u) / 3u + 1u;
        if (steps > 48u)steps = 48u;
        if (steps & 1u)pal1 ^= 0xa0u;
    }
    PbrbGbDrawOam(gb, oam, 40u, pal0, pal1);
}
/* Every AnimateIntroNidorino script entry is a RELATIVE offset added to
   shadow OAM, not an absolute position. Reconstruct the accumulated relative
   OAM movement at the requested animation frame, including completed steps. */
static const signed char hipY[5] = { 0,-2,-1,1,2 }, hipX[5] = { 0,2,2,2,2 };
static const signed char hopY[5] = { 0,-2,-1,1,2 }, hopX[5] = { 0,-2,-2,-2,-2 };
static const signed char jumpY[5] = { 0,-12,-8,8,12 }, jumpX[5] = { 0,6,6,6,6 };
static const signed char downY[5] = { 0,-8,-4,4,8 }, downX[5] = { 0,-4,-4,-4,-4 };
static const signed char upY[5] = { 0,-8,-4,4,8 }, upX[5] = { 0,4,4,4,4 };
static const signed char shortY[4] = { 0,2,2,0 }, shortX[4] = { 0,0,0,0 };
static const signed char lungeY[4] = { -8,-7,-6,-4 }, lungeX[4] = { -16,-14,-12,-10 };
static void IntroAccumulate(unsigned f, unsigned at, const signed char* xs,
    const signed char* ys, unsigned n, int* x, int* y) {
    if (f < at)return;
    unsigned count = (f - at) / 5u + 1u;
    if (count > n)count = n;
    for (unsigned i = 0; i < count; i++) { *x += xs[i]; *y += ys[i]; }
}
static void Battle(PbrbGb* gb, unsigned f, unsigned bgPalette = 0xe4u, unsigned objPalette = 0xe4u) {
    /* The original opening is scripted in engine/movie/intro.asm and lasts
       537 frames after the shooting star. OAM is sprite-priority composited. */
    int scroll = f < 80u ? 2 * (int)(f / 2u + 1u) : 80;
    int baseX = -8 + scroll, baseY = 72, variant = 0, dx = 0, dy = 0;
    if (f >= 220u && f < 228u)scroll += 2 * (int)((f - 220u) / 2u + 1u);
    else if (f >= 228u && f < 258u)scroll += 8;
    else if (f >= 258u && f < 274u)scroll += 8 - 2 * (int)((f - 258u) / 2u + 1u);
    else if (f >= 274u && f < 329u)scroll -= 8;
    else if (f >= 329u && f < 337u)scroll += -8 + 2 * (int)((f - 329u) / 2u + 1u);
    int variantG = 0;
    if (f >= 220u && f < 337u)variantG = (f >= 258u) ? 2 : 1;
    if ((f >= 274u && f < 397u) || f >= 467u)variant = 1;
    if (f >= 517u)variant = 2;
    IntroAccumulate(f, 80u, hipX, hipY, 5, &dx, &dy);
    IntroAccumulate(f, 105u, hopX, hopY, 5, &dx, &dy);
    IntroAccumulate(f, 140u, hipX, hipY, 5, &dx, &dy);
    IntroAccumulate(f, 165u, hopX, hopY, 5, &dx, &dy);
    IntroAccumulate(f, 274u, jumpX, jumpY, 5, &dx, &dy);
    IntroAccumulate(f, 397u, downX, downY, 5, &dx, &dy);
    IntroAccumulate(f, 422u, upX, upY, 5, &dx, &dy);
    IntroAccumulate(f, 467u, shortX, shortY, 4, &dx, &dy);
    IntroAccumulate(f, 517u, lungeX, lungeY, 4, &dx, &dy);
    Bars(gb);
    Gengar(gb, variantG, scroll, (int)f);
    if (bgPalette != 0xe4u)PbrbGbApplyBGPalette(gb, bgPalette);
    Front(gb, variant, baseX + dx, baseY + dy, objPalette);
}
void PbrbIntroRender(PbrbGb* gb, const PbrbIntro* t) {
    if (!gb || !t)return;
    if (t->frame < 180u) {
        /* Exact tile data and text layout from LoadCopyrightTiles and
           CopyrightTextString in engine/movie/title.asm.  This screen is
           held for the original 180 DMG frames. */
        static const unsigned char lines[3][17] = {
            {0x60,0x61,0x62,0x61,0x63,0x61,0x64,0x7f,0x65,0x66,0x67,0x68,0x69,0x6a,0,0,0},
            {0x60,0x61,0x62,0x61,0x63,0x61,0x64,0x7f,0x6b,0x6c,0x6d,0x6e,0x6f,0x70,0x71,0x72,0},
            {0x60,0x61,0x62,0x61,0x63,0x61,0x64,0x7f,0x73,0x74,0x75,0x76,0x77,0x78,0x79,0x7a,0x7b}
        };
        PbrbGbClear(gb, 0);
        for (int row = 0; row < 3; row++)
            for (int col = 0; col < 17; col++) {
                unsigned ch = lines[row][col];
                if (ch < 0x60u || ch>0x7bu)continue;
                PbrbGbTile(gb, 0x120c8u + 16u * (ch - 0x60u), 2,
                    16 + col * 8, 56 + row * 16);
            }
        return;
    }
    if (t->frame < 186u) { PbrbGbClear(gb, 0); return; }
    if (t->frame < GF_START) { Bars(gb); return; }
    if (t->frame < PBRB_INTRO_MUSIC_START) {
        DrawGameFreak(gb, t->frame - GF_START);
        return;
    }
    if (t->frame < BATTLE_START) { Bars(gb); return; }
    if (t->frame + 1u >= PBRB_INTRO_FADE_START) {
        static const unsigned bg[3] = { 0x90u,0x40u,0x00u };
        static const unsigned obj[3] = { 0x80u,0x40u,0x00u };
        /* As in the star flashes, OBP/BGP writes take effect before
           the next DMA-delivered OAM pose. The original performs the
           actual writes partway through the first visible scanlines. */
        const unsigned fadeFrame = t->frame + 1u - PBRB_INTRO_FADE_START;
        unsigned stage = fadeFrame / 8u;
        if (stage > 2u)stage = 2u;
        Battle(gb, PBRB_INTRO_BATTLE_FRAMES - 1u, bg[stage], obj[stage]);
        if (fadeFrame < 24u && !(fadeFrame & 7u)) {
            /* GBFadeIncCommon writes BGP after these rows have been drawn.
               Both original Red/Blue intro traces retain 8,8,14 old rows.
               This calibration is for the deterministic intro path, not a
               general model of CPU/audio-interrupt-dependent raster phase.
               Only the upper black bar occupies this part of the screen. */
            const unsigned rows = stage == 2u ? 14u : 8u;
            const unsigned previous = stage ? bg[stage - 1u] : 0xe4u;
            const unsigned savedBg = gb->bgPalette;
            PbrbGbSetPalettes(gb, previous, gb->objPalette0, gb->objPalette1);
            for (unsigned y = 0; y < rows; ++y)for (unsigned x = 0; x < PBRB_GB_W; ++x)
                PbrbGbPixel(gb, (int)x, (int)y, gb->background[y * PBRB_GB_W + x], 0);
            PbrbGbSetPalettes(gb, savedBg, gb->objPalette0, gb->objPalette1);
        }
    }
    else Battle(gb, t->frame - BATTLE_START);
}
