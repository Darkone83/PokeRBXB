#ifndef PBRB_BATTLE_ANIM_INL_INCLUDED
#define PBRB_BATTLE_ANIM_INL_INCLUDED
#include "pbrb_battle_anim.h"
#include "pbrb_audio.h"
#include <string.h>

/* Bank $1e tables in the verified USA/Europe Red and Blue ROMs. The compiler
   below expands only the published animation commands into bounded native
   operations. It never executes instructions from the ROM. */
enum PbanOp {
    PBAN_WAIT, PBAN_LOAD, PBAN_SOUND, PBAN_RAW_SOUND, PBAN_FRAME,
    PBAN_CLEAR_OAM, PBAN_DEST, PBAN_COPY_GROWL, PBAN_PALETTE,
    PBAN_SAVE_PALETTE, PBAN_RESTORE_PALETTE, PBAN_OBJECT_XOR, PBAN_OBJECT_SET,
    PBAN_SHOW, PBAN_HIDE, PBAN_SHIFT_UP, PBAN_SLIDE_OFF, PBAN_SQUISH,
    PBAN_PICTURE, PBAN_SCROLL, PBAN_WATER, PBAN_SPIRAL, PBAN_PILLAR_INIT,
    PBAN_PILLAR_STEP, PBAN_FALL_INIT, PBAN_FALL_STEP, PBAN_WAVE,
    PBAN_HUD_COPY, PBAN_HUD_MODE, PBAN_BLANK, PBAN_RECT_CLEAR,
    PBAN_GHOST_LEFT, PBAN_TRADE_MOVE, PBAN_WAIT_SOUND,
    PBAN_FAINT_ROW, PBAN_GROW, PBAN_CRY, PBAN_SEMANTIC_SOUND,
    PBAN_MUSIC, PBAN_EVOLUTION_PICTURE, PBAN_BG_TRANSFER, PBAN_CANCEL_WINDOW
};
/* Original ChangeMonPic elapsed frames, measured with the native species
   headers in both orientations. Front decompression cost varies with the
   ROM's dimensions/RLE data. Zero entries are unused internal species IDs.
   QA provenance: qa/playtest/battle_anim/picture_times.py, original DMG ROM.
   No pixel data is present in this table. */
static const unsigned char PbanPictureFrames[2][191] = {
    {
        0,29,29,26,26,26,26,30,30,27,28,29,27,28,26,26,28,27,30,29,
        27,26,27,26,26,27,28,28,29,27,27,0,0,28,29,27,27,28,29,28,
        26,29,27,28,29,27,29,28,29,28,0,28,0,29,29,30,0,27,26,29,
        27,0,0,0,29,26,28,0,0,0,28,26,26,28,26,27,28,26,27,0,
        0,0,28,27,27,29,0,0,26,27,28,29,26,28,0,0,27,27,26,28,
        25,29,28,27,30,28,28,26,26,27,26,26,28,28,26,0,28,27,29,28,
        26,0,0,26,27,27,29,0,29,27,28,27,26,29,0,0,29,0,30,30,
        0,26,27,28,27,29,0,27,29,29,28,28,28,27,30,28,0,27,28,0,
        0,0,0,27,28,25,29,28,27,28,28,29,0,28,0,0,28,27,28,29,
        28,0,0,0,0,27,30,29,27,29,29
    },
    {
        0,51,51,26,27,26,27,53,53,38,51,48,51,31,35,25,50,27,51,49,
        47,27,47,30,27,51,46,33,51,44,36,0,0,30,43,43,28,31,36,41,
        35,50,35,48,40,53,50,31,39,40,0,39,0,40,36,37,0,27,35,29,
        50,0,0,0,41,27,50,0,0,0,26,26,37,47,47,45,28,27,28,0,
        0,0,39,45,27,44,0,0,26,38,28,41,28,35,0,0,28,36,29,41,
        26,39,29,39,35,35,30,30,27,26,35,42,25,25,45,0,43,44,40,48,
        34,0,0,25,24,44,50,0,49,50,46,45,49,40,0,0,51,0,50,46,
        0,29,34,52,45,35,0,37,30,51,39,49,40,29,52,39,0,34,51,0,
        0,0,0,35,46,26,36,35,34,29,38,49,0,24,0,0,30,29,38,34,
        51,0,0,0,0,25,34,47,25,38,49
    }
};
/* Evolution_LoadPic uses the flipped front loader with music stopped.
   Its ROM-dependent decompression/copy times differ from ChangeMonPic. */
static const unsigned char PbanEvolutionPictureFrames[191] = {
    0,49,50,26,27,26,27,51,52,37,50,47,50,31,34,25,49,26,50,48,
    46,27,46,30,26,50,45,33,50,43,36,0,0,29,42,43,28,30,35,40,
    35,49,34,47,39,51,49,30,38,39,0,38,0,39,36,37,0,27,35,29,
    48,0,0,0,41,27,48,0,0,0,26,26,36,46,46,44,28,27,28,0,
    0,0,39,44,27,43,0,0,26,38,28,40,28,34,0,0,27,36,29,40,
    26,39,29,39,35,34,29,29,27,26,35,41,25,25,44,0,42,43,40,47,
    34,0,0,25,24,44,49,0,48,49,45,44,48,39,0,0,50,0,49,45,
    0,28,33,50,44,35,0,36,29,50,38,48,39,29,51,38,0,33,49,0,
    0,0,0,34,45,26,35,35,34,29,38,48,0,24,0,0,29,29,37,34,
    49,0,0,0,0,25,34,46,25,37,48
};
static int PbanFits(const PbrbBattleAnim* a, unsigned at, unsigned bytes) {
    return a && a->gb && a->gb->rom && (size_t)at <= a->gb->bytes &&
        (size_t)bytes <= a->gb->bytes - (size_t)at;
}
static unsigned PbanRead(PbrbBattleAnim* a, unsigned at) {
    if (!PbanFits(a, at, 1)) { ++a->errors; return 0; }
    return a->gb->rom[at];
}
static unsigned PbanPointer(PbrbBattleAnim* a, unsigned at) {
    if (!PbanFits(a, at, 2)) { ++a->errors; return 0; }
    const unsigned p = a->gb->rom[at] | ((unsigned)a->gb->rom[at + 1] << 8);
    if (p < 0x4000u || p >= 0x8000u) { ++a->errors; return 0; }
    return 0x74000u + p;
}
static void PbanAdd(PbrbBattleAnim* a, unsigned kind, unsigned x = 0,
    unsigned y = 0, unsigned z = 0, unsigned frames = 0) {
    if (a->count >= PBRB_BATTLE_ANIM_OPS) { ++a->errors; return; }
    PbrbBattleAnimOp* o = a->ops + a->count++;
    o->kind = (unsigned short)kind; o->a = (unsigned short)x;
    o->b = (unsigned short)y; o->c = (unsigned short)z; o->frames = (unsigned short)frames;
}
static void PbanWait(PbrbBattleAnim* a, unsigned frames) {
    PbanAdd(a, PBAN_WAIT, 0, 0, 0, frames);
}
static void PbanLoad(PbrbBattleAnim* a, unsigned tileset) {
    PbanAdd(a, PBAN_LOAD, tileset, 0, 0, tileset == 2u ? 9u : 10u);
}
static void PbanFlash(PbrbBattleAnim* a) {
    PbanAdd(a, PBAN_SAVE_PALETTE);
    PbanAdd(a, PBAN_PALETTE, 0x1b, 0, 0, 2);
    PbanAdd(a, PBAN_PALETTE, 0, 0, 0, 2);
    PbanAdd(a, PBAN_RESTORE_PALETTE);
}
static void PbanShow(PbrbBattleAnim* a, unsigned side, unsigned frames = 3) {
    PbanAdd(a, PBAN_SHOW, side, 0, 7, frames);
}
static void PbanSlideDown(PbrbBattleAnim* a, unsigned side) {
    for (unsigned row = 7; row; --row) {
        PbanAdd(a, PBAN_SHOW, side, 0, row, 3);
        PbanAdd(a, PBAN_HIDE, side);
    }
}
static void PbanBlink(PbrbBattleAnim* a, unsigned side) {
    for (unsigned i = 0; i < 6; ++i) {
        PbanAdd(a, PBAN_HIDE, side, 0, 0, 5);
        PbanShow(a, side, 8);
    }
}
static void PbanShake(PbrbBattleAnim* a, unsigned amount, int vertical) {
    for (unsigned i = amount; i; --i) {
        const unsigned first = i == amount ? i : 0u, second = i == amount ? 0u : i;
        PbanAdd(a, PBAN_SCROLL, vertical ? 0u : first, vertical ? first : 0u, 0, vertical ? 3u : 5u);
        PbanAdd(a, PBAN_SCROLL, vertical ? 0u : second, vertical ? second : 0u, 0, vertical ? 3u : 4u);
    }
    PbanAdd(a, PBAN_SCROLL);
}
static void PbanSpiral(PbrbBattleAnim* a, unsigned side) {
    PbanLoad(a, 0);
    for (unsigned i = 0; i < 20; ++i)PbanAdd(a, PBAN_SPIRAL, i, side, 0, i == 19u ? 1u : 5u);
    PbanAdd(a, PBAN_CLEAR_OAM);
    PbanFlash(a);
}
static void PbanPillar(PbrbBattleAnim* a, unsigned baseY, unsigned baseX, unsigned count) {
    PbanLoad(a, 0);
    PbanAdd(a, PBAN_PILLAR_INIT, baseY, baseX, count, 1);
    for (unsigned i = 0; i < count * 2u - 1u; ++i)
        PbanAdd(a, PBAN_PILLAR_STEP, baseY, count, 0, 1);
}
static void PbanReplacePicture(PbrbBattleAnim* a, unsigned side, unsigned mode) {
    const unsigned species = mode ? a->targetSpecies : a->species[side];
    const unsigned copyFrames = side ? 7u : 14u;
    unsigned frames = species <= 190u ? PbanPictureFrames[side][species] : 0u;
    if (frames < copyFrames)frames = copyFrames;
    if (!side)PbanAdd(a, PBAN_RECT_CLEAR, 1, 5, 0x708);
    PbanWait(a, frames - copyFrames);
    PbanAdd(a, PBAN_PICTURE, side, mode, side ? 0u : 1u, copyFrames);
    PbanShow(a, side, 0);
}
static void PbanSpecial(PbrbBattleAnim* a, unsigned op, unsigned side) {
    unsigned i, j;
    switch (op) {
    case 0xfe: PbanFlash(a); break;
    case 0xfd: PbanAdd(a, PBAN_PALETTE, 0x6f); break;
    case 0xfc: PbanAdd(a, PBAN_PALETTE, 0xe4); break;
    case 0xfb: PbanShake(a, 8, 0); break;
    case 0xfa:
        PbanLoad(a, 0);
        for (i = 0; i < 64; ++i) {
            PbanAdd(a, PBAN_WATER, i ? 0u : 1u, (i & 1u) ? 24u : 16u, 0, 1);
            PbanAdd(a, PBAN_CLEAR_OAM, 0, 0, 0, 1);
        }
        break;
    case 0xf9: PbanAdd(a, PBAN_PALETTE, 0xf9); break;
    case 0xf8:
        for (i = 0; i < 3; ++i)for (j = 0; j < 12; ++j)
            PbanAdd(a, PBAN_PALETTE, PbanRead(a, 0x7918eu + j), 0, 0, i ? 1u : 2u);
        break;
    case 0xf7:
        for (i = 0; i < 7; ++i)PbanAdd(a, PBAN_SHIFT_UP, side, i, 0, 2);
        break;
    case 0xf6: PbanSlideDown(a, side); break;
    case 0xf5: case 0xe0:
        if (op == 0xe0)side ^= 1u;
        PbanReplacePicture(a, side, 0);
        break;
    case 0xf4: case 0xdb: case 0xe5:
        if (op == 0xdb)side ^= 1u;
        for (i = 0; i < (op == 0xe5 ? 4u : 8u); ++i)
            PbanAdd(a, PBAN_SLIDE_OFF, side, 0, 0, op == 0xe5 ? 4u : 3u);
        if (op == 0xe5)PbanWait(a, 3);
        break;
    case 0xf3: case 0xde: PbanBlink(a, op == 0xde ? side ^ 1u : side); break;
    case 0xf2:
        PbanAdd(a, PBAN_HIDE, side);
        PbanAdd(a, PBAN_SHOW, side, side ? 65535u : 1u, 7, 3);
        break;
    case 0xf1:
        PbanAdd(a, PBAN_RECT_CLEAR, side ? 11u : 2u, side ? 0u : 5u, 0x707);
        PbanShow(a, side);
        break;
    case 0xf0: PbanAdd(a, PBAN_PALETTE, 0x90); break;
    case 0xef: PbanAdd(a, PBAN_HIDE, side); break;
    case 0xee:
        for (i = 0; i < 4; ++i) {
            PbanAdd(a, PBAN_SQUISH, side, 0, 0, 3);
            PbanAdd(a, PBAN_SQUISH, side, 1, 0, 3);
        }
        PbanAdd(a, PBAN_HIDE, side, 0, 0, 1);
        break;
    case 0xed:
        PbanPillar(a, side ? 0u : 48u, side ? 128u : 40u, 5);
        PbanWait(a, 1); PbanAdd(a, PBAN_CLEAR_OAM);
        break;
    case 0xec:
        for (i = 0; i < 6; ++i)PbanPillar(a, side ? 40u : 80u,
            PbanRead(a, (side ? 0x79598u : 0x79591u) + i), 4);
        PbanWait(a, 1); PbanAdd(a, PBAN_CLEAR_OAM);
        break;
    case 0xeb:
        for (i = 0; i < 5; ++i)PbanSlideDown(a, side);
        PbanShow(a, side);
        break;
    case 0xea:
        PbanAdd(a, PBAN_PICTURE, side, 2, 0, 10); PbanShow(a, side); break;
    case 0xe9:
        for (i = 1; i <= 2; ++i) {
            PbanAdd(a, PBAN_HIDE, side);
            PbanAdd(a, PBAN_SHOW, side, i, 0x100, 8);
        }
        PbanAdd(a, PBAN_HIDE, side);
        PbanAdd(a, PBAN_PICTURE, side, 4, 0, 7);
        break;
    case 0xe8:
        PbanReplacePicture(a, side, 1);
        break;
    case 0xe7: case 0xe6:
        PbanLoad(a, 1);
        PbanAdd(a, PBAN_FALL_INIT, op == 0xe7 ? 3u : 20u, op == 0xe7 ? 0x37u : 0x71u);
        for (i = 0; i < 52; ++i)PbanAdd(a, PBAN_FALL_STEP, 0, 0, 0, 3);
        if (op == 0xe6)PbanAdd(a, PBAN_CLEAR_OAM);
        break;
    case 0xe4: case 0xe3:
        PbanAdd(a, PBAN_HUD_COPY, 0, 0, 0, 7);
        PbanWait(a, 6);
        PbanAdd(a, PBAN_HUD_MODE, 1, 0, 0, 3);
        PbanAdd(a, PBAN_HIDE, side, 0, 0, 3);
        for (i = 0; i < 8; ++i) {
            PbanAdd(a, PBAN_HUD_MODE, 1, 2, 0, 2);
            PbanAdd(a, PBAN_HUD_MODE, 1, 65534u, 0, 2);
        }
        PbanAdd(a, PBAN_HUD_MODE, 1, 0);
        PbanShow(a, side);
        PbanAdd(a, PBAN_CLEAR_OAM);
        PbanAdd(a, PBAN_HUD_MODE, 0, 0, 0, 15);
        break;
    case 0xe2: PbanSpiral(a, side); break;
    case 0xe1: PbanWait(a, 10); break;
    case 0xdf: PbanAdd(a, PBAN_HIDE, side ^ 1u, 0, 0, 3); break;
    case 0xdd: case 0xdc: PbanShow(a, op == 0xdc ? side ^ 1u : side); break;
    case 0xda:
        for (i = 0; i < 16; ++i) {
            PbanAdd(a, PBAN_SHOW, side, 65535u, 7, 3);
            PbanAdd(a, PBAN_RECT_CLEAR, side ? 11u : 0u, side ? 0u : 5u, 0x709);
            PbanAdd(a, PBAN_SHOW, side, 1, 7, 3);
            PbanAdd(a, PBAN_RECT_CLEAR, side ? 11u : 0u, side ? 0u : 5u, 0x709);
        }
        break;
    case 0xd9:
        PbanAdd(a, PBAN_PICTURE, side, 3, 0, 7); PbanShow(a, side); break;
    case 0xd8:
        PbanWait(a, 6);
        /* The original tests LY=143 after each H-blank write, so two
           outer iterations usually finish on that same scanline. Its
           255-count loop occupies 128 displayed frames, not 255. */
        for (i = 0; i < 128; ++i)PbanAdd(a, PBAN_WAVE, i, 0, 0, 1);
        PbanAdd(a, PBAN_WAVE, 65535u, 0, 0, 9);
        break;
    default: ++a->errors; break;
    }
}
static void PbanCallback(PbrbBattleAnim* a, unsigned move, unsigned left) {
    switch (move) {
    case 5: case 12: case 25: case 29: case 50: case 61: case 115: case 147:
        PbanFlash(a); break;
    case 39: PbanWait(a, 20); break;
    case 45:
        PbanAdd(a, PBAN_COPY_GROWL, 0, 0, 0, left == 1u ? 1u : 0u);
        if (left == 1u)PbanAdd(a, PBAN_CLEAR_OAM);
        break;
    case 59: if (left == 13u || left == 9u || left == 5u || left == 1u)PbanFlash(a); break;
    case 63: if (!(left & 3u))PbanFlash(a); break;
    case 85: if (!(left & 7u))PbanFlash(a); break;
    case 120: case 153:
        if (left == 1u)PbanAdd(a, PBAN_HIDE, a->enemyTurn);
        else if (!(left & 3u))PbanFlash(a);
        break;
    case 157:
        if (left >= 8u && left < 12u) { PbanShake(a, 1, 0); PbanShake(a, 1, 1); }
        else if (left == 1u)PbanFlash(a);
        break;
    case 170:
        if (left == 6u)PbanAdd(a, PBAN_RECT_CLEAR, 7, 2, 0x707);
        break;
    case 171:
        if (left == 1u) {
            for (unsigned i = 0; i < 3; ++i)PbanAdd(a, PBAN_TRADE_MOVE, PbanRead(a, 0x79078u + i), 0, 0, 3);
            PbanWait(a, 1); PbanAdd(a, PBAN_CLEAR_OAM);
            PbanAdd(a, PBAN_RAW_SOUND, 0x98, 2);
        }
        break;
    case 172:
        for (unsigned i = 0; i < 8; ++i) {
            PbanAdd(a, PBAN_TRADE_MOVE, PbanRead(a, 0x790b3u + i));
            if (i == 0u || i == 4u || i == 7u)PbanAdd(a, PBAN_RAW_SOUND, 0xae, 2);
            PbanWait(a, 5);
        }
        PbanAdd(a, PBAN_RECT_CLEAR, 0, 0, 0x1214);
        break;
    case 193: case 197: case 198:
        if (a->item && a->item < 3u)PbanAdd(a, PBAN_OBJECT_XOR, 0x3c);
        if (left == 11u)PbanAdd(a, PBAN_RAW_SOUND, 0x91);
        if (a->endWait == 4u && left <= 3u)PbanAdd(a, PBAN_GHOST_LEFT);
        break;
    case 194:
        if (left == 4u) { PbanAdd(a, PBAN_RAW_SOUND, 0x8c); PbanWait(a, 40); }
        break;
    case 195: if (left == 5u)PbanAdd(a, PBAN_RAW_SOUND, 0x93); break;
    default: break;
    }
}
static int PbanCompile(PbrbBattleAnim* a, unsigned move) {
    if (!move || move > 203u) { ++a->errors; return 0; }
    if (a->enemyTurn && move == 133u)move = 191;
    else if (a->enemyTurn && move == 156u)move = 189;
    unsigned pc = PbanPointer(a, 0x7a07du + (move - 1u) * 2u);
    for (unsigned commands = 0; commands < 512u && !a->errors; ++commands) {
        const unsigned op = PbanRead(a, pc++);
        if (op == 255u)return 1;
        const unsigned sound = PbanRead(a, pc++);
        if (op >= 0xc0u) {
            if (sound != 255u)PbanAdd(a, PBAN_SOUND, sound, move);
            PbanSpecial(a, op, a->enemyTurn);
            continue;
        }
        const unsigned set = op >> 6, delay = (op & 63u) ? (op & 63u) : 256u;
        const unsigned sub = PbanRead(a, pc++);
        if (sub >= 86u || set > 2u) { ++a->errors; break; }
        const unsigned start = PbanPointer(a, 0x7a76du + sub * 2u), header = PbanRead(a, start);
        const unsigned total = header & 31u, type = header >> 5;
        const unsigned transform = type == 5u ? (a->enemyTurn ? 0u : 2u) : (a->enemyTurn ? type : 0u);
        if (!total || type > 5u) { ++a->errors; break; }
        PbanAdd(a, PBAN_OBJECT_SET, 0xe4);
        PbanLoad(a, set);
        if (sound != 255u)PbanAdd(a, PBAN_SOUND, sound, move);
        PbanAdd(a, PBAN_DEST);
        const unsigned repeat = move == 194u && a->shakes ? a->shakes : 1u;
        for (unsigned rep = 0; rep < repeat; ++rep)for (unsigned block = 0; block < total; ++block) {
            unsigned left = total - block;
            if (a->endWait == 3u && (move == 193u || move == 197u || move == 198u) && left == 2u)continue;
            const unsigned at = start + 1u + (transform == 4u ? total - 1u - block : block) * 3u;
            const unsigned id = PbanRead(a, at), coord = PbanRead(a, at + 1u), mode = PbanRead(a, at + 2u);
            if (mode > 4u || id > 121u || coord > 176u) { ++a->errors; return 0; }
            unsigned duration = mode == 2u ? 0u : delay;
            if (mode != 2u && mode != 3u && mode != 4u && move != 45u)++duration;
            PbanAdd(a, PBAN_FRAME, id, coord, transform | (mode << 8), duration);
            if (mode != 2u && mode != 3u && mode != 4u) {
                if (move != 45u)PbanAdd(a, PBAN_CLEAR_OAM);
                PbanAdd(a, PBAN_DEST);
            }
            PbanCallback(a, move, left);
        }
        PbanAdd(a, PBAN_OBJECT_SET, 0xe4);
    }
    ++a->errors; return 0;
}
static unsigned PbanPicTile(unsigned side, unsigned tile) { return 360u + side * 49u + tile; }
static void PbanTile(PbrbBattleAnim* a, int x, int y, unsigned tile) {
    if ((unsigned)x < 20u && (unsigned)y < 18u)a->tilemap[y * 20 + x] = (unsigned short)tile;
}
static unsigned PbanGetTile(const PbrbBattleAnim* a, int x, int y) {
    return (unsigned)x < 20u && (unsigned)y < 18u ? a->tilemap[y * 20 + x] : 65535u;
}
static void PbanClearRect(PbrbBattleAnim* a, int x, int y, unsigned w, unsigned h) {
    for (unsigned yy = 0; yy < h; ++yy)for (unsigned xx = 0; xx < w; ++xx)
        PbanTile(a, x + (int)xx, y + (int)yy, 65535u);
}
static void PbanShowNow(PbrbBattleAnim* a, unsigned side, int xOffset, unsigned rows, unsigned list) {
    const int x = (side ? 12 : 1) + xOffset, y = side ? 0 : 5;
    unsigned pointer = PbanPointer(a, 0x79aeau + list * 3u);
    if (list)rows = PbanRead(a, 0x79aecu + list * 3u) >> 4;
    if (rows > 7u) { ++a->errors; return; }
    for (unsigned row = 0; row < rows; ++row)for (unsigned col = 0; col < 7u; ++col) {
        unsigned tile = PbanRead(a, pointer + row * 7u + col);
        PbanTile(a, x + (int)col, y + 7 - (int)rows + (int)row, tile < 49u ? PbanPicTile(side, tile) : 65535u);
    }
}
static void PbanLoadPart(PbrbBattleAnim* a) {
    const unsigned count = a->loadSet == 2u ? 64u : 79u;
    const unsigned offset = a->loadSet == 1u ? 0x786eeu : 0x781feu;
    unsigned first = a->loadPart++ * 8u;
    for (unsigned tile = first; tile < first + 8u && tile < count; ++tile) {
        if (!PbanFits(a, offset + tile * 16u, 16)) { ++a->errors; return; }
        const unsigned char* p = a->gb->rom + offset + tile * 16u;
        for (unsigned y = 0; y < 8; ++y)for (unsigned x = 0; x < 8; ++x)
            a->tiles[0x31u + tile][y * 8u + x] = (unsigned char)
            (((p[y * 2u] >> (7u - x)) & 1u) | (((p[y * 2u + 1u] >> (7u - x)) & 1u) << 1));
    }
}
static void PbanFrame(PbrbBattleAnim* a, const PbrbBattleAnimOp* o) {
    const unsigned p = PbanPointer(a, 0x7af74u + o->a * 2u), n = PbanRead(a, p);
    const unsigned baseY = PbanRead(a, 0x7bc85u + o->b * 2u), baseX = PbanRead(a, 0x7bc86u + o->b * 2u);
    const unsigned transform = o->c & 255u, mode = o->c >> 8;
    if (n > 40u || a->oamDest + n > 40u || !PbanFits(a, p + 1u, n * 4u)) { ++a->errors; return; }
    for (unsigned i = 0; i < n; ++i) {
        const unsigned char* in = a->gb->rom + p + 1u + i * 4u;
        unsigned char* out = a->oam[a->oamDest + i];
        unsigned y = baseY + in[0], x = baseX + in[1], attr = in[3];
        if (transform == 1u) {
            y = 136u - y; x = 168u - x;
            attr = attr == 0u ? 0x60u : attr == 0x20u ? 0x40u : attr == 0x40u ? 0x20u : 0u;
        }
        else if (transform == 2u) { y += 40u; x = 168u - x; attr ^= 0x20u; }
        else if (transform == 3u) { y = 136u - baseY + in[0]; x = 168u - baseX + in[1]; }
        out[0] = (unsigned char)y; out[1] = (unsigned char)x;
        out[2] = (unsigned char)(in[2] + 0x31u); out[3] = (unsigned char)attr;
    }
    if (mode == 2u || mode == 3u)a->oamDest += n;
}
static void PbanMoveSound(PbrbBattleAnim* a, unsigned sound, unsigned move) {
    const unsigned at = 0x798bcu + sound * 3u;
    if (!PbanFits(a, at, 3u)) { ++a->errors; return; }
    unsigned id = a->gb->rom[at], frequency = a->gb->rom[at + 1], tempo = a->gb->rom[at + 2];
    if (move == 45u || move == 46u) {
        const unsigned species = a->actorSpecies;
        if (!species || species > 190u) { ++a->errors; return; }
        const unsigned cry = 0x39446u + (species - 1u) * 3u;
        id = 20u + PbanRead(a, cry) * 3u;
        frequency = (frequency + PbanRead(a, cry + 1u)) & 255u;
        tempo = (tempo + PbanRead(a, cry + 2u)) & 255u;
    }
    PbrbAudioPlaySfxModified(PBRB_AUDIO_RAW(8, id), frequency, tempo);
}
static void PbanPicture(PbrbBattleAnim* a, unsigned side, unsigned mode) {
    unsigned char* pic = a->scratch;
    unsigned i, j;
    if (mode <= 1u) {
        const unsigned species = mode ? a->targetSpecies : a->species[side];
        if (!PbrbGbDecodePokemon(a->gb, species, side ? 0 : 1, a->original[side], 56u * 56u)) {
            ++a->errors; return;
        }
        if (side)memcpy(pic, a->original[side], 56u * 56u);
        else for (i = 0; i < 56; ++i)for (j = 0; j < 56; ++j)
            pic[i * 56u + j] = a->original[side][(24u + i / 2u) * 56u + 16u + j / 2u];
    }
    else {
        memset(pic, 0, 56u * 56u);
        if (mode == 2u)for (i = 0; i < 5; ++i) {
            const unsigned bits = PbanRead(a, 0x795c4u + i);
            for (j = 0; j < 8; ++j)pic[(34u + i) * 56u + 24u + j] = (unsigned char)(((bits >> (7u - j)) & 1u) * 3u);
        }
        else if (mode == 3u)for (unsigned tile = 0; tile < 4; ++tile) {
            const unsigned at = 0x14780u + (side ? tile : tile + 4u) * 16u;
            for (i = 0; i < 8; ++i)for (j = 0; j < 8; ++j) {
                const unsigned lo = PbanRead(a, at + i * 2u), hi = PbanRead(a, at + i * 2u + 1u);
                pic[(32u + (tile / 2u) * 8u + i) * 56u + (side ? 16u : 24u) + (tile & 1u) * 8u + j] =
                    (unsigned char)(((lo >> (7u - j)) & 1u) | (((hi >> (7u - j)) & 1u) << 1));
            }
        }
    }
}
static void PbanPicturePart(PbrbBattleAnim* a) {
    const unsigned sprites = a->loadPart >= 7u;
    const unsigned part = a->loadPart++ - (sprites ? 7u : 0u), side = a->loadSet;
    for (unsigned tile = part * 8u; tile < part * 8u + 8u && tile < 49u; ++tile)
        for (unsigned y = 0; y < 8; ++y)for (unsigned x = 0; x < 8; ++x) {
            const unsigned at = ((tile % 7u) * 8u + y) * 56u + (tile / 7u) * 8u + x;
            if (sprites)a->tiles[tile][y * 8u + x] = a->scratch[at];
            else a->picture[side][at] = a->scratch[at];
        }
    if (a->loadPart >= (a->pictureCopySprites ? 14u : 7u))a->loadActive = 0;
}
static void PbanApply(PbrbBattleAnim* a, const PbrbBattleAnimOp* o) {
    unsigned i, j, k;
    const unsigned side = o->a & 1u;
    const int x = side ? 12 : 1, y = side ? 0 : 5;
    switch (o->kind) {
    case PBAN_WAIT: break;
    case PBAN_LOAD: a->loadSet = o->a; a->loadPart = 0; a->loadActive = 1; break;
    case PBAN_SOUND: PbanMoveSound(a, o->a, o->b); break;
    case PBAN_RAW_SOUND: PbrbAudioPlaySfx(PBRB_AUDIO_RAW(o->b ? o->b : 8u, o->a)); break;
    case PBAN_FRAME: PbanFrame(a, o); break;
    case PBAN_CLEAR_OAM: memset(a->oam, 0, sizeof(a->oam)); break;
    case PBAN_DEST: a->oamDest = 0; break;
    case PBAN_COPY_GROWL: memcpy(a->oam[4], a->oam[0], 16); break;
    case PBAN_PALETTE: a->palette = o->a; break;
    case PBAN_SAVE_PALETTE: a->savedPalette = a->palette; break;
    case PBAN_RESTORE_PALETTE: a->palette = a->savedPalette; break;
    case PBAN_OBJECT_XOR: a->objectPalette ^= o->a; break;
    case PBAN_OBJECT_SET: a->objectPalette = o->a; break;
    case PBAN_SHOW:
        PbanShowNow(a, side, o->c == 0x100u ? 0 : (short)o->b, o->c == 0x100u ? 0 : o->c, o->c == 0x100u ? o->b : 0);
        break;
    case PBAN_HIDE: PbanClearRect(a, x, y, 7, 7); break;
    case PBAN_SHIFT_UP:
        for (i = 0; i < 6; ++i)for (j = 0; j < 7; ++j)
            PbanTile(a, x + (int)j, y + (int)i, PbanGetTile(a, x + (int)j, y + 1 + (int)i));
        for (j = 0; j < 7; ++j)PbanTile(a, x + (int)j, y + 6, PbanPicTile(side, j * 7u + o->b));
        break;
    case PBAN_SLIDE_OFF:
        for (i = 0; i < 7; ++i)for (j = 0; j < 8; ++j) {
            const int xx = side ? 12 + (int)j : (int)j;
            const unsigned tile = PbanGetTile(a, xx, y + (int)i), base = PbanPicTile(side, 0);
            unsigned next = 65535u;
            if (tile >= base && tile < base + 49u) {
                const unsigned index = tile - base;
                if (side && index >= 7u && index - 7u < 48u)next = base + index - 7u;
                else if (!side && index + 7u < 48u)next = base + index + 7u;
            }
            PbanTile(a, xx, y + (int)i, next);
        }
        break;
    case PBAN_SQUISH:
        for (i = 0; i < 7; ++i) {
            if (!o->b) {
                for (j = 0; j < 3; ++j)PbanTile(a, x + 3 + (int)j, y + (int)i, PbanGetTile(a, x + 4 + (int)j, y + (int)i));
                PbanTile(a, x + 6, y + (int)i, 65535u);
            }
            else {
                for (j = 0; j < 3; ++j)PbanTile(a, x + 3 - (int)j, y + (int)i, PbanGetTile(a, x + 2 - (int)j, y + (int)i));
                PbanTile(a, x, y + (int)i, 65535u);
            }
        }
        break;
    case PBAN_PICTURE:
        PbanPicture(a, side, o->b); a->loadSet = side; a->loadPart = 0;
        a->loadActive = 2; a->pictureCopySprites = o->c;
        break;
    case PBAN_SCROLL: a->scrollX = (short)o->a; a->scrollY = (short)o->b; break;
    case PBAN_WATER: {
        if (o->a)a->waterX = 240;
        unsigned xx = a->waterX, yy = o->b;
        memset(a->oam, 0, sizeof(a->oam)); i = 0;
        while (yy < 112u && i < 40u) {
            xx = (xx + 27u) & 255u;
            a->oam[i][0] = (unsigned char)yy; a->oam[i][1] = (unsigned char)xx;
            a->oam[i][2] = 0x71; a->oam[i][3] = 0; ++i;
            if (xx >= 144u) { xx = (xx - 168u) & 255u; yy += 16u; }
        }
        a->waterX = xx;
        break;
    }
    case PBAN_SPIRAL:
        for (i = 0; i < 3; ++i) {
            const unsigned at = 0x79476u + (o->a + i) * 2u, yy = PbanRead(a, at);
            if (yy == 255u)break;
            a->oam[i][0] = (unsigned char)(yy + (o->b ? 216u : 0u));
            a->oam[i][1] = (unsigned char)(PbanRead(a, at + 1u) + (o->b ? 80u : 0u));
            a->oam[i][2] = 0x7a; a->oam[i][3] = 0;
        }
        break;
    case PBAN_PILLAR_INIT:
        for (i = 0; i < o->c && i < 40u; ++i) {
            a->oam[i][0] = (unsigned char)(o->a + (i + 1u) * 8u);
            a->oam[i][1] = (unsigned char)o->b; a->oam[i][2] = 0x7a; a->oam[i][3] = 0;
        }
        break;
    case PBAN_PILLAR_STEP:
        for (i = 0; i < o->b && i < 40u; ++i)a->oam[i][0] =
            a->oam[i][0] == o->a + 8u ? 0 : (unsigned char)(a->oam[i][0] - 4u);
        break;
    case PBAN_FALL_INIT:
        a->fallingCount = o->a;
        for (i = 0; i < o->a && i < 40u; ++i) {
            a->oam[i][0] = (unsigned char)(i ? (i + 1u) * 8u : 0);
            a->oam[i][1] = (unsigned char)PbanRead(a, 0x79d3eu + i);
            a->oam[i][2] = (unsigned char)o->b; a->oam[i][3] = 0;
            a->motion[i] = (unsigned char)PbanRead(a, 0x79d63u + i);
        }
        break;
    case PBAN_FALL_STEP:
        for (i = 0; i < a->fallingCount && i < 40u; ++i) {
            unsigned m = (a->motion[i] + 1u) & 255u;
            if ((m & 127u) == 9u)m = (m & 128u) ^ 128u;
            a->motion[i] = (unsigned char)m;
            unsigned yy = (a->oam[i][0] + 2u) & 255u;
            a->oam[i][0] = (unsigned char)(yy >= 112u ? 160u : yy);
            const unsigned delta = PbanRead(a, 0x79d0du + (m & 127u));
            a->oam[i][1] = (unsigned char)(a->oam[i][1] + ((m & 128u) ? 0u - delta : delta));
            a->oam[i][3] = (unsigned char)((m & 128u) ? 0x20u : 0u);
        }
        break;
    case PBAN_WAVE: a->wavePhase = o->a == 65535u ? -1 : (int)o->a; break;
    case PBAN_HUD_COPY:
        for (i = 0; i < 49; ++i)for (j = 0; j < 8; ++j)for (k = 0; k < 8; ++k)
            a->tiles[i][j * 8u + k] = a->picture[0][((i % 7u) * 8u + j) * 56u + (i / 7u) * 8u + k];
        break;
    case PBAN_HUD_MODE:
        if (o->a && !a->hudShake) {
            /* AnimationShakeEnemyHUD copies this scene into the window
               before it hides the back pic in BG map 0. Below row 7 the
               window retains the picture; only its upper part needs OAM. */
            memcpy(a->hudWindowMap, a->tilemap, sizeof(a->hudWindowMap));
            for (i = 0; i < 7; ++i)for (j = 0; j < 5; ++j) {
                const unsigned index = i * 5u + j;
                a->oam[index][0] = (unsigned char)(56u + j * 8u);
                a->oam[index][1] = (unsigned char)(16u + i * 8u);
                a->oam[index][2] = (unsigned char)(i * 7u + j); a->oam[index][3] = 0;
            }
        }
        a->hudShake = o->a != 0; a->scrollX = (short)o->b;
        break;
    case PBAN_BLANK: a->blank = o->a != 0; break;
    case PBAN_RECT_CLEAR: PbanClearRect(a, o->a, o->b, o->c & 255u, o->c >> 8); break;
    case PBAN_GHOST_LEFT:
        for (i = 0; i < 7; ++i)for (j = 0; j < 7; ++j)
            PbanTile(a, 18 - (int)j, (int)i, PbanGetTile(a, 17 - (int)j, (int)i));
        break;
    case PBAN_TRADE_MOVE:
        for (i = 0; i < 4; ++i)a->oam[i][0] = (unsigned char)(a->oam[i][0] + (signed char)o->a);
        if (a->move == 172u)a->scrollX += 8;
        break;
    case PBAN_FAINT_ROW:
        for (i = 6; i; --i)for (j = 0; j < 7; ++j)
            PbanTile(a, x + (int)j, y + (int)i, PbanGetTile(a, x + (int)j, y + (int)i - 1));
        for (j = 0; j < 7; ++j)PbanTile(a, x + (int)j, y, 65535u);
        break;
    case PBAN_GROW:
        if (o->b == 0u)PbanTile(a, x + 3, y + 6, PbanPicTile(0, o->c ? 29u : 28u));
        else if (o->b == 3u)PbanShowNow(a, side, 0, 7, 0);
        else {
            const unsigned width = o->b == 1u ? 3u : 5u;
            const unsigned table = width == 3u ? 0x79b1bu : 0x79b02u;
            const int xx = x + (width == 3u ? 2 : 1), yy = y + (width == 3u ? 4 : 2);
            for (i = 0; i < width; ++i)for (j = 0; j < width; ++j) {
                const unsigned tile = PbanRead(a, table + i * width + j);
                PbanTile(a, xx + (int)j, yy + (int)i, tile >= 0x31u && tile < 0x62u ? PbanPicTile(side, tile - 0x31u) : 65535u);
            }
        }
        break;
    case PBAN_CRY: PbrbAudioPlayCry(o->a); break;
    case PBAN_SEMANTIC_SOUND: PbrbAudioPlaySfx(o->a); break;
    case PBAN_MUSIC: PbrbAudioPlayMusic(o->a); break;
    case PBAN_EVOLUTION_PICTURE:
        for (i = 0; i < 7; ++i)for (j = 0; j < 7; ++j)
            PbanTile(a, 7 + (int)i, 2 + (int)j, PbanPicTile(side, i * 7u + j));
        break;
    case PBAN_BG_TRANSFER: a->transferEnabled = o->a != 0; break;
    case PBAN_CANCEL_WINDOW: a->canCancel = o->a != 0; break;
    default: ++a->errors; break;
    }
}
static int PbanCaptureFrame(PbrbBattleAnim* a, PbrbGb* g, unsigned move,
    int enemy, unsigned userSpecies, unsigned targetSpecies) {
    if (!a || !g || !g->rom || !g->pixels || g->bytes != 1048576u)return 0;
    memset(a, 0, sizeof(*a)); a->gb = g; a->move = move; a->enemyTurn = enemy ? 1u : 0u;
    a->actorSpecies = userSpecies; a->targetSpecies = targetSpecies;
    a->species[a->enemyTurn] = userSpecies; a->species[a->enemyTurn ^ 1u] = targetSpecies;
    a->palette = a->compilePalette = a->displayedPalette = 0xe4;
    a->objectPalette = a->displayedObjectPalette = 0xe4;
    a->wavePhase = a->displayedWave = -1;
    a->transferPhase = 1; a->transferEnabled = 1;
    memcpy(a->base, g->background, sizeof(a->base));
    for (unsigned i = 0; i < 360; ++i)a->tilemap[i] = (unsigned short)i;
    for (unsigned side = 0; side < 2; ++side) {
        const unsigned x = side ? 96u : 8u, y = side ? 0u : 40u;
        for (unsigned yy = 0; yy < 56; ++yy)for (unsigned xx = 0; xx < 56; ++xx)
            a->picture[side][yy * 56u + xx] = a->base[(y + yy) * 160u + x + xx];
        memcpy(a->original[side], a->picture[side], 56u * 56u);
        for (unsigned col = 0; col < 7; ++col)for (unsigned row = 0; row < 7; ++row)
            PbanTile(a, (int)x / 8 + (int)col, (int)y / 8 + (int)row, PbanPicTile(side, col * 7u + row));
    }
    memcpy(a->displayedMap, a->tilemap, sizeof(a->tilemap));
    a->captured = 1; a->active = 1; PbanAdd(a, PBAN_WAIT_SOUND);
    return 1;
}
static void PbanAdvance(PbrbBattleAnim* a) {
    unsigned guard = 0;
    while (a->active && !a->remaining && guard++ < PBRB_BATTLE_ANIM_OPS) {
        if (a->next >= a->count || a->errors) { a->active = 2; break; }
        const PbrbBattleAnimOp* op = a->ops + a->next;
        if (op->kind == PBAN_WAIT_SOUND) {
            if (PbrbAudioEffectsBusy())return;
            ++a->next; continue;
        }
        ++a->next; PbanApply(a, op); a->remaining = op->frames;
    }
}
int PbrbBattleAnimInit(PbrbBattleAnim* a, PbrbGb* g, unsigned move,
    int enemyTurn, unsigned userSpecies, unsigned targetSpecies, int substitute, int minimized) {
    if (!PbanCaptureFrame(a, g, move, enemyTurn, userSpecies, targetSpecies))return 0;
    if (substitute) {
        PbanSpecial(a, 0xf4, a->enemyTurn);
        PbanSpecial(a, minimized ? 0xeau : 0xf5u, a->enemyTurn);
        if (!minimized)PbanShow(a, a->enemyTurn);
    }
    if (!PbanCompile(a, move)) { a->active = 0; return 0; }
    if (substitute) {
        PbanSpecial(a, 0xf4, a->enemyTurn);
        PbanSpecial(a, 0xd9, a->enemyTurn);
        PbanShow(a, a->enemyTurn);
    }
    PbanAdd(a, PBAN_WAIT_SOUND); PbanAdvance(a);
    return !a->errors;
}
int PbrbBattleAnimCapture(PbrbBattleAnim* a, PbrbGb* g, unsigned item,
    unsigned shakes, unsigned result, unsigned playerSpecies, unsigned enemySpecies) {
    if (shakes > 3u || result > 4u || !PbanCaptureFrame(a, g, 193, 0, playerSpecies, enemySpecies))return 0;
    a->item = item; a->shakes = shakes; a->endWait = result;
    PbanCompile(a, result == 3u ? 193u : item == 4u ? 193u : item == 3u ? 197u : 198u);
    if (result == 3u) { PbanAdd(a, PBAN_RAW_SOUND, 0x95); PbanCompile(a, 196); }
    else if (result != 2u && result != 4u) {
        PbanCompile(a, 195); PbanCompile(a, 200);
        if (shakes)PbanCompile(a, 194);
        if (result == 1u) { PbanCompile(a, 195); PbanCompile(a, 166); }
    }
    PbanAdd(a, PBAN_WAIT_SOUND); PbanAdvance(a);
    if (a->errors)a->active = 0;
    return !a->errors;
}
int PbrbBattleAnimFaint(PbrbBattleAnim* a, PbrbGb* g,
    unsigned side, unsigned species, int trainerBattle) {
    if (side > 1u || !PbanCaptureFrame(a, g, 0, (int)side, species, 0))return 0;
    if (!side)PbanAdd(a, PBAN_RECT_CLEAR, 9, 7, 0x50b);
    for (unsigned i = 0; i < 7; ++i)PbanAdd(a, PBAN_FAINT_ROW, side, 0, 0, 2);
    if (side) {
        PbanAdd(a, PBAN_RECT_CLEAR, 0, 0, 0x40b);
        if (trainerBattle) {
            PbanAdd(a, PBAN_WAIT_SOUND);
            PbanAdd(a, PBAN_SEMANTIC_SOUND, PBRB_SFX_FAINT_FALL);
            PbanAdd(a, PBAN_WAIT_SOUND);
            PbanAdd(a, PBAN_SEMANTIC_SOUND, PBRB_SFX_FAINT_THUD);
            PbanAdd(a, PBAN_WAIT_SOUND);
        }
    }
    else {
        PbanAdd(a, PBAN_CRY, species); PbanAdd(a, PBAN_WAIT_SOUND);
    }
    PbanAdvance(a); return !a->errors;
}
int PbrbBattleAnimSendOut(PbrbBattleAnim* a, PbrbGb* g,
    unsigned side, unsigned species, int trainerBattle) {
    if (side > 1u || !species || species > 190u ||
        !PbanCaptureFrame(a, g, 0, (int)side, species, 0))return 0;
    const unsigned copyFrames = side ? 7u : 14u;
    unsigned frames = PbanPictureFrames[side][species];
    if (frames < copyFrames)frames = copyFrames;
    PbanAdd(a, PBAN_HIDE, side);
    if (side)PbanAdd(a, PBAN_RECT_CLEAR, 0, 0, 0x40b);
    PbanWait(a, frames - copyFrames);
    PbanAdd(a, PBAN_PICTURE, side, 0, side ? 0u : 1u, copyFrames);
    if (!side) {
        PbanWait(a, 3); /* PlayMoveAnimation's original leading Delay3. */
        a->enemyTurn = 1; PbanCompile(a, 195); a->enemyTurn = side;
        PbanAdd(a, PBAN_WAIT_SOUND);
    }
    PbanAdd(a, PBAN_GROW, side, 0, trainerBattle ? 1u : 0u, 3);
    PbanAdd(a, PBAN_GROW, side, 1, 0, 4);
    PbanAdd(a, PBAN_GROW, side, 2, 0, 5);
    PbanAdd(a, PBAN_GROW, side, 3);
    PbanAdd(a, PBAN_CRY, species); PbanAdd(a, PBAN_WAIT_SOUND);
    PbanAdvance(a); return !a->errors;
}
int PbrbBattleAnimRetreat(PbrbBattleAnim* a, PbrbGb* g) {
    if (!PbanCaptureFrame(a, g, 0, 0, 0, 0))return 0;
    PbanAdd(a, PBAN_HIDE, 0);
    PbanAdd(a, PBAN_GROW, 0, 2, 0, 4);
    PbanAdd(a, PBAN_HIDE, 0);
    PbanAdd(a, PBAN_GROW, 0, 1, 0, 3);
    PbanAdd(a, PBAN_HIDE, 0);
    PbanAdvance(a); return !a->errors;
}
int PbrbBattleAnimEnemyWithdraw(PbrbBattleAnim* a, PbrbGb* g) {
    if (!PbanCaptureFrame(a, g, 0, 1, 0, 0))return 0;
    for (unsigned column = 1; column <= 8; ++column) {
        PbanAdd(a, PBAN_RECT_CLEAR, 11, 0, 0x709);
        PbanAdd(a, PBAN_SHOW, 1, column, 7, 2);
    }
    PbanAdvance(a); return !a->errors;
}
int PbrbBattleAnimSubstituteBreak(PbrbBattleAnim* a, PbrbGb* g,
    unsigned side, unsigned species, int minimized) {
    if (side > 1u || !PbanCaptureFrame(a, g, 0, (int)side, species, 0))return 0;
    PbanSlideDown(a, side);
    PbanSpecial(a, minimized ? 0xeau : 0xf5u, side);
    if (!minimized)PbanShow(a, side);
    PbanAdvance(a); return !a->errors;
}
int PbrbBattleAnimHit(PbrbBattleAnim* a, PbrbGb* g,
    unsigned side, unsigned factor, unsigned move) {
    if (!a || side > 1u)return 0;
    if (!move)move = a->move;
    if (!PbanCaptureFrame(a, g, move, (int)(side ^ 1u), 0, 0))return 0;
    unsigned effect = 0;
    if (move && move <= 165u)effect = PbanRead(a, 0x38001u + (move - 1u) * 6u);
    factor &= 0x7fu;
    if (factor)PbanAdd(a, PBAN_SEMANTIC_SOUND, factor == 10u ? PBRB_SFX_DAMAGE :
        factor < 10u ? PBRB_SFX_NOT_VERY_EFFECTIVE : PBRB_SFX_SUPER_EFFECTIVE);
    if (side) {
        if (effect)PbanShake(a, 2, 0);
        else PbanBlink(a, 1);
    }
    else PbanShake(a, 8, effect ? 0 : 1);
    PbanAdd(a, PBAN_WAIT_SOUND);
    PbanAdvance(a); return !a->errors;
}
int PbrbBattleAnimEvolution(PbrbBattleAnim* a, PbrbGb* g,
    unsigned oldSpecies, unsigned newSpecies, int forced) {
    if (!oldSpecies || oldSpecies > 190u || !newSpecies || newSpecies > 190u ||
        !PbanEvolutionPictureFrames[oldSpecies] || !PbanEvolutionPictureFrames[newSpecies] ||
        !PbanCaptureFrame(a, g, 0, 0, oldSpecies, newSpecies))return 0;
    a->count = 0; a->evolution = 1; a->forced = forced ? 1u : 0u;
    for (unsigned side = 0; side < 2; ++side) {
        if (!PbrbGbDecodePokemon(g, side ? newSpecies : oldSpecies, 0, a->scratch, sizeof(a->scratch))) {
            a->active = 0; ++a->errors; return 0;
        }
        for (unsigned y = 0; y < 56; ++y)for (unsigned x = 0; x < 56; ++x)
            a->picture[side][y * 56u + x] = a->scratch[y * 56u + 55u - x];
    }
    /* Until the original loader shows a picture, all captured scene tiles
       must keep referring to the captured frame rather than battle pics. */
    for (unsigned i = 0; i < 360; ++i)a->tilemap[i] = (unsigned short)i;
    memcpy(a->displayedMap, a->tilemap, sizeof(a->tilemap));
    PbanAdd(a, PBAN_RECT_CLEAR, 0, 0, 0x0c14);
    PbanAdd(a, PBAN_SEMANTIC_SOUND, PBRB_SFX_STOP_ALL);
    PbanAdd(a, PBAN_SEMANTIC_SOUND, PBRB_SFX_TINK);
    PbanWait(a, 3);
    PbanAdd(a, PBAN_BG_TRANSFER, 0);
    /* The enclosing two-load/copy sequence crosses one more VBlank than
       the sum of the isolated loader probes (verified in EvolveMon). */
    PbanWait(a, PbanEvolutionPictureFrames[newSpecies] + 8u + PbanEvolutionPictureFrames[oldSpecies]);
    PbanAdd(a, PBAN_EVOLUTION_PICTURE, 0);
    PbanAdd(a, PBAN_BG_TRANSFER, 1);
    PbanAdd(a, PBAN_CRY, oldSpecies); PbanAdd(a, PBAN_WAIT_SOUND);
    PbanAdd(a, PBAN_MUSIC, PBRB_MUSIC_EVOLUTION); PbanWait(a, 80);
    for (unsigned group = 1; group <= 8; ++group) {
        PbanAdd(a, PBAN_CANCEL_WINDOW, 1, 0, 0, 18u - group * 2u);
        PbanAdd(a, PBAN_CANCEL_WINDOW, 0);
        for (unsigned n = 0; n < group; ++n) {
            PbanAdd(a, PBAN_EVOLUTION_PICTURE, 1, 0, 0, 3);
            PbanAdd(a, PBAN_EVOLUTION_PICTURE, 0, 0, 0, 3);
        }
    }
    PbanAdd(a, PBAN_EVOLUTION_PICTURE, 1, 0, 0, 3);
    PbanAdd(a, PBAN_SEMANTIC_SOUND, PBRB_SFX_STOP_ALL);
    PbanAdd(a, PBAN_CRY, newSpecies); PbanAdd(a, PBAN_WAIT_SOUND);
    PbanAdvance(a); return !a->errors;
}
int PbrbBattleAnimCancelEvolution(PbrbBattleAnim* a) {
    if (!a || !a->active || !a->evolution || a->forced || !a->canCancel)return 0;
    a->cancelled = 1; a->canCancel = 0; a->count = a->next = a->remaining = 0; a->loadActive = 0;
    PbanAdd(a, PBAN_SEMANTIC_SOUND, PBRB_SFX_STOP_ALL);
    PbanAdd(a, PBAN_CRY, a->actorSpecies); PbanAdd(a, PBAN_WAIT_SOUND);
    PbanAdvance(a); return 1;
}
void PbrbBattleAnimTick(PbrbBattleAnim* a) {
    if (!a || !a->active)return;
    ++a->elapsed;
    /* OAM DMA becomes visible at the following VBlank. The original BG
       copier transfers six rows per VBlank, rotating through thirds. */
    memcpy(a->displayedOam, a->oam, sizeof(a->oam));
    if (a->transferEnabled) {
        memcpy(a->displayedMap + a->transferPhase * 120u,
            a->tilemap + a->transferPhase * 120u, 120u * sizeof(unsigned short));
        a->transferPhase = (a->transferPhase + 1u) % 3u;
    }
    a->displayedPalette = a->palette; a->displayedObjectPalette = a->objectPalette;
    a->displayedX = a->scrollX; a->displayedY = a->scrollY;
    a->displayedWave = a->wavePhase; a->displayedHud = a->hudShake; a->displayedBlank = a->blank;
    if (a->active == 2u) { a->active = 0; return; }
    if (a->remaining) {
        if (a->loadActive == 1u)PbanLoadPart(a);
        else if (a->loadActive == 2u)PbanPicturePart(a);
        --a->remaining;
        if (!a->remaining)a->loadActive = 0;
    }
    PbanAdvance(a);
}
int PbrbBattleAnimDone(const PbrbBattleAnim* a) { return !a || !a->active; }
static unsigned PbanBackground(const PbrbBattleAnim* a, int x, int y,
    const unsigned short* map) {
    if ((unsigned)x >= 160u || (unsigned)y >= 144u || a->displayedBlank)return 0;
    const unsigned tile = map[(unsigned)y / 8u * 20u + (unsigned)x / 8u];
    if (tile < 360u) {
        const unsigned xx = (tile % 20u) * 8u + ((unsigned)x & 7u), yy = (tile / 20u) * 8u + ((unsigned)y & 7u);
        return a->base[yy * 160u + xx];
    }
    if (tile < 458u) {
        const unsigned t = tile - 360u, side = t / 49u, n = t % 49u;
        return a->picture[side][((n % 7u) * 8u + ((unsigned)y & 7u)) * 56u + (n / 7u) * 8u + ((unsigned)x & 7u)];
    }
    return 0;
}
void PbrbBattleAnimRender(const PbrbBattleAnim* a, PbrbGb* g) {
    if (!a || !a->captured || !g || !g->pixels)return;
    const unsigned oldBg = g->bgPalette, old0 = g->objPalette0, old1 = g->objPalette1;
    /* Palette and window-position registers take effect directly; unlike
       shadow OAM they must not be delayed by the next DMA latch. */
    PbrbGbSetPalettes(g, a->palette, a->objectPalette, 0x6c);
    for (int y = 0; y < 144; ++y)for (int x = 0; x < 160; ++x) {
        int xx = x, yy = y;
        if (a->displayedWave >= 0) {
            const unsigned at = 0x796bfu + ((unsigned)(a->displayedWave * 2 + y * 2) & 31u);
            if (PbanFits(a, at, 1))xx += (signed char)a->gb->rom[at];
        }
        else if (a->displayedHud) { if (y < 56)xx += a->displayedX; }
        else { xx -= a->scrollX; yy -= a->scrollY; }
        /* HUD shake changes the shadow hSCX/hWY registers, which VBlank
           latches. Ordinary screen shakes write window registers directly. */
        const unsigned short* map = a->displayedHud && y >= 56 ? a->hudWindowMap : a->displayedMap;
        PbrbGbPixel(g, x, y, PbanBackground(a, xx, yy, map), 0);
    }
    PbrbGbSetPalettes(g, oldBg, old0, old1);
    PbrbBattleAnimRenderSprites(a, g);
}
void PbrbBattleAnimRenderSprites(const PbrbBattleAnim* a, PbrbGb* g) {
    if (!a || !a->captured || !g || !g->pixels)return;
    const unsigned oldBg = g->bgPalette, old0 = g->objPalette0, old1 = g->objPalette1;
    PbrbGbSetPalettes(g, oldBg, a->objectPalette, 0x6c);
    for (int y = 0; y < 144; ++y) {
        unsigned selected[10], n = 0;
        for (unsigned i = 0; i < 40u && n < 10u; ++i)
            if (y + 16 >= a->displayedOam[i][0] && y + 16 < a->displayedOam[i][0] + 8)selected[n++] = i;
        for (int x = 0; x < 160; ++x) {
            int best = -1; unsigned color = 0, bestX = 256;
            for (unsigned i = 0; i < n; ++i) {
                const unsigned char* o = a->displayedOam[selected[i]];
                if (o[1] >= bestX || x + 8 < o[1] || x + 8 >= o[1] + 8 || o[2] >= 128u)continue;
                unsigned row = (unsigned)(y + 16 - o[0]), col = (unsigned)(x + 8 - o[1]);
                if (o[3] & 0x40u)row = 7u - row;
                if (o[3] & 0x20u)col = 7u - col;
                const unsigned c = a->tiles[o[2]][row * 8u + col];
                if (c) { best = (int)selected[i]; bestX = o[1]; color = c; }
            }
            if (best < 0)continue;
            const unsigned attr = a->displayedOam[best][3];
            if ((attr & 0x80u) && g->background[y * 160 + x])continue;
            PbrbGbPixel(g, x, y, color, (attr & 0x10u) ? 2u : 1u);
        }
    }
    PbrbGbSetPalettes(g, oldBg, old0, old1);
}
#endif
