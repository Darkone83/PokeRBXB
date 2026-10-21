#include "pbrb_title.h"
/* engine/movie/title.asm TitleScreenPokemonLogoYScrolls:
   (-4,16), (+3,4), (-3,4), (+2,2), (-2,2), (+1,2), (-1,2).
   Then DelayFrames(36). Version enters at d=144 and +=4 each frame,
   wrapping at 256 (28 frames). SCX is written immediately at scanline64,
   whereas the bounce waits for VBlank before changing SCY. Align the
   version's visible frames to that scanline phase, not another VBlank
   delay. The resulting 110-frame title sequence matches both ROMs. */
void PbrbTitleReset(PbrbTitle* t) {
    if (!t)return;
    t->frame = 0; t->logoScrollY = 64; t->bounceDone = 0;
    t->versionX = 56; t->versionVisible = 0;
    t->monSpecies = 0; t->random = 1; t->monScrollX = 0;
    t->monVisible = 1; t->ballY = 0x74;
    t->idlePhase = PBRB_TITLE_IDLE_NONE; t->idleFrame = t->loadFrames = 0;
}
void PbrbTitleTick(PbrbTitle* t) {
    if (!t)return;
    static const signed char amount[] = { -4,3,-3,2,-2,1,-1 };
    static const unsigned char length[] = { 16,4,4,2,2,2,2 };
    if (t->frame < 32) {
        int cursor = 0;
        for (int k = 0; k < 7; k++) {
            int next = cursor + length[k];
            if (t->frame < next) { t->logoScrollY += amount[k]; break; }
            cursor = next;
        }
    }
    if (t->frame >= 66 && t->frame < 94) {
        unsigned d = (144u + 4u * (unsigned)(t->frame - 66)) & 255u;
        t->versionX = (56 - (int)d + 256) & 255;
        t->versionVisible = 1;
    }
    else if (t->frame >= 94) {
        t->versionX = 56; t->versionVisible = 1;
    }
    t->frame++;
    if (t->frame >= 32) { t->logoScrollY = 0; t->bounceDone = 1; }
}
void PbrbTitleBeginIdle(PbrbTitle* t, const unsigned char* rom, size_t bytes, unsigned seed) {
    if (!t || !rom || bytes != 1048576u)return;
    t->monSpecies = rom[0x4588u]; t->random = seed ? seed : 1u;
    t->monScrollX = 0; t->monVisible = 1; t->ballY = 0x74;
    t->idlePhase = PBRB_TITLE_IDLE_HOLD; t->idleFrame = 0;
}
int PbrbTitleCanAccept(const PbrbTitle* t) {
    return t && (t->idlePhase == PBRB_TITLE_IDLE_HOLD || t->idlePhase == PBRB_TITLE_IDLE_POLL);
}
static int TitleScrollAt(const unsigned char* rom, unsigned table, unsigned frame, unsigned start) {
    /* Original packed high-nibble speed / low-nibble duration lists. */
    for (unsigned i = 0; i < 9u && rom[table + i]; ++i) {
        unsigned step = rom[table + i] >> 4, count = rom[table + i] & 15u;
        if (frame < count)return (int)((start + step * frame) & 255u);
        start += step * count; frame -= count;
    }
    return (int)(start & 255u);
}
static int TitleStarter(unsigned species) {
    return species == 0xb0u || species == 0xb1u || species == 0x99u;
}
int PbrbTitleIdleTick(PbrbTitle* t, const unsigned char* rom, size_t bytes, int game) {
    if (!t || !rom || bytes != 1048576u || (game != 1 && game != 2))return 0;
    switch (t->idlePhase) {
    case PBRB_TITLE_IDLE_HOLD:
        t->monScrollX = 0;
        if (++t->idleFrame == 200u) {
            t->idlePhase = PBRB_TITLE_IDLE_OUT; t->idleFrame = 0;
        }
        break;
    case PBRB_TITLE_IDLE_OUT:
        if (++t->idleFrame == 18u) {
            t->idlePhase = PBRB_TITLE_IDLE_POLL; t->idleFrame = 0; t->monVisible = 0;
        }
        else t->monScrollX = TitleScrollAt(rom, 0x3724fu, t->idleFrame, 0);
        break;
    case PBRB_TITLE_IDLE_POLL:
        if (TitleStarter(t->monSpecies)) {
            t->idlePhase = PBRB_TITLE_IDLE_BALL; t->idleFrame = 1;
            t->ballY = rom[0x372a1u];
        }
        else { t->idlePhase = PBRB_TITLE_IDLE_COPY; t->idleFrame = 1; }
        break;
    case PBRB_TITLE_IDLE_BALL:
        t->ballY = rom[0x372a0u + ++t->idleFrame];
        /* The last OAM update overlaps the three-frame tile-map copy. */
        if (t->idleFrame == 9u) { t->idlePhase = PBRB_TITLE_IDLE_COPY; t->idleFrame = 0; }
        break;
    case PBRB_TITLE_IDLE_COPY:
        t->ballY = rom[0x372aau];
        if (++t->idleFrame == 3u) {
            unsigned pick;
            do {
                t->random ^= t->random << 13; t->random ^= t->random >> 17;
                t->random ^= t->random << 5; pick = t->random & 15u;
            } while (rom[0x4588u + pick] == t->monSpecies);
            t->monSpecies = rom[0x4588u + pick];
            /* Original LoadTitleMonSprite includes decompression and timed
               VRAM copies. These complete GB-frame durations were measured
               in each verified ROM, in its TitleMons order. */
            static const unsigned char load[2][16] = {
                {30,29,29,25,26,46,27,27,51,30,51,28,39,43,35,40},
                {29,30,29,27,48,39,35,49,36,50,34,26,26,38,35,44}
            };
            t->loadFrames = load[game - 1][pick]; t->idleFrame = 0;
            t->idlePhase = PBRB_TITLE_IDLE_LOAD; return 1;
        }
        break;
    case PBRB_TITLE_IDLE_LOAD:
        if (++t->idleFrame == t->loadFrames) {
            t->idlePhase = PBRB_TITLE_IDLE_IN; t->idleFrame = 0;
            t->monVisible = 1; t->monScrollX = 0x88;
        }
        break;
    case PBRB_TITLE_IDLE_IN:
        t->monScrollX = TitleScrollAt(rom, 0x37247u, ++t->idleFrame, 0x88u);
        if (t->idleFrame == 16u) { t->idlePhase = PBRB_TITLE_IDLE_HOLD; t->idleFrame = 0; }
        break;
    default: break;
    }
    return 0;
}
