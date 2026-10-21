/* Native credits data flow from engine/movie/credits.asm and the final
   HallOfFame script. Text, silhouettes, copyright and THE END are ROM data.
   No script copies or generated artwork are bundled with the executable. */
enum {
    PCRED_MON_LIST = 0x74131, PCRED_PALETTES = 0x74160,
    PCRED_END_STRING = 0x74229, PCRED_ORDER = 0x74243, PCRED_TEXTS = 0x742c3
};
static void PplayCreditsStart(PbrbPlay* p) {
    p->endingPending = 0; p->scene = PBRB_PLAY_CREDITS; p->overlay = PBRB_OVERLAY_NONE;
    p->text.visible = 0; p->creditsAt = PCRED_ORDER; p->creditsMon = 0; p->creditsLines = 0;
    p->creditsPhase = 0; p->creditsTimer = 100; p->creditsElapsed = 0; p->creditsCopyright = 0;
    p->battleAnim.active = 0; PbrbInputStopRumble();
}
static void PplayCreditsNext(PbrbPlay* p) {
    p->creditsLines = 0; p->creditsCopyright = 0; p->creditsElapsed = 0;
    for (unsigned guard = 0; guard < 12 && p->creditsAt < PCRED_TEXTS; ++guard) {
        unsigned command = p->gb->rom[p->creditsAt++];
        if (command == 0xfb) { p->creditsCopyright = 1; continue; }
        if (command >= 0xfa) {
            p->creditsCommand = (int)command;
            if (command == 0xfa) { p->creditsPhase = 5; p->creditsTimer = 16; return; }
            p->creditsPhase = 2; p->creditsTimer = command >= 0xfe ? 110 : 140; return;
        }
        if (command >= 64 || p->creditsLines >= 5)break;
        unsigned at = PCRED_TEXTS + command * 2u;
        unsigned address = p->gb->rom[at] | ((unsigned)p->gb->rom[at + 1] << 8);
        if (address < 0x4000 || address >= 0x8000)break;
        p->creditsTexts[p->creditsLines++] = 0x70000u + address;
    }
    p->creditsPhase = 10; PplayNotice(p, "Credits data error.", "Please reload.", PBRB_OVERLAY_NONE);
}
static void PplayCreditsTick(PbrbPlay* p, unsigned keys) {
    if (p->creditsPhase == 10) { if (keys & (PBRB_PAD_A | PBRB_PAD_B))p->exitRequested = 3; return; }
    if (p->creditsPhase == 8) {
        if (!(keys & PBRB_PAD_A))return;
        if (!PplaySaveCheckpoint(p)) { PplayNotice(p, "Save failed.", "Press A to retry.", PBRB_OVERLAY_NONE); return; }
        p->creditsPhase = 9; p->creditsTimer = 600; return;
    }
    if (p->creditsTimer > 0) { --p->creditsTimer; ++p->creditsElapsed; return; }
    switch (p->creditsPhase) {
    case 0:PbrbAudioPlayMusic(PBRB_MUSIC_CREDITS); p->creditsPhase = 1; p->creditsTimer = 128; p->creditsElapsed = 0; break;
    case 1:PplayCreditsNext(p); break;
    case 2:
        if (p->creditsCommand >= 0xfe) {
            if (p->creditsMon >= 15) { p->creditsPhase = 10; PplayNotice(p, "Credits data error.", "Please reload.", PBRB_OVERLAY_NONE); return; }
            unsigned species = p->gb->rom[PCRED_MON_LIST + p->creditsMon++];
            PbrbGbDecodePokemon(p->gb, species, 0, p->picture, sizeof(p->picture));
            p->creditsPhase = 3; p->creditsTimer = 27; p->creditsElapsed = 0;
        }
        else PplayCreditsNext(p); break;
    case 3:PplayCreditsNext(p); break;
    case 5:p->creditsPhase = 6; p->creditsTimer = 20; p->creditsElapsed = 0; break;
    case 6:
        p->creditsPhase = 8;
        if (!PplaySaveCheckpoint(p)) { PplayNotice(p, "Save failed.", "Press A to retry.", PBRB_OVERLAY_NONE); return; }
        p->creditsPhase = 9; p->creditsTimer = 600; break;
    case 9:if (keys & (PBRB_PAD_A | PBRB_PAD_B))p->exitRequested = 3; break;
    default:break;
    }
}
static void PplayCreditsText(PbrbPlay* p, int scroll) {
    PbrbGb* g = p->gb;
    for (unsigned line = 0; line < p->creditsLines; ++line) {
        unsigned at = p->creditsTexts[line];
        int x = 72 + (int)(signed char)g->rom[at] * 8 - scroll;
        PplayUIString(g, at + 1, x, 48 + (int)line * 16, 0, 64);
    }
    /* Credits shift the font's black plane to color index1. The black bars
       keep index3, so the four original BGP values fade only the text. */
    for (int y = 32; y < 112; ++y)for (int x = 0; x < 160; ++x) {
        unsigned color = g->background[y * 160 + x]; if (color)PbrbGbPixel(g, x, y, 1, 0);
    }
    if (p->creditsCopyright) {
        unsigned at = 0x4556u; int x = 16 - scroll, y = 56;
        for (unsigned n = 0; n < 80; ++n) {
            unsigned c = g->rom[at++]; if (c == 0x50)break;
            if (c == 0x4e) { x = 16 - scroll; y += 16; continue; }
            if (c >= 0x60 && c <= 0x7b)PbrbGbTile(g, 0x120c8u + (c - 0x60u) * 16u, 2, x, y); x += 8;
        }
    }
}
static void PplayCreditsRender(PbrbPlay* p) {
    PbrbGb* g = p->gb; unsigned palette = 0xc0;
    PbrbGbSetPalettes(g, 0xe4, 0xe4, 0xe4); PbrbGbClear(g, 0);
    if (!p->creditsPhase)return;
    if (p->creditsPhase == 2 || p->creditsPhase == 5) {
        PplayCreditsText(p, 0); palette = 0xcc;
        if (p->creditsPhase == 2 && (p->creditsCommand == 0xff || p->creditsCommand == 0xfd)) {
            unsigned frame = (unsigned)p->creditsElapsed / 5u; if (frame > 3)frame = 3; palette = g->rom[PCRED_PALETTES + frame];
        }
    }
    else if (p->creditsPhase == 3) {
        int scroll = p->creditsElapsed * 8; if (scroll > 208)scroll = 208;
        PplayCreditsText(p, scroll); PbrbGbBitmap(g, p->picture, 56, 56, 160 - scroll, 48, 0, 0); palette = 0xfc;
    }
    else if (p->creditsPhase >= 6) {
        unsigned graphics = p->game == 2 ? 0x7473fu : 0x7473eu, at = PCRED_END_STRING;
        for (int row = 0; row < 2; ++row) {
            int x = 32;
            for (unsigned n = 0; n < 20; ++n) {
                unsigned c = g->rom[at++]; if (c == 0x50)break;
                if (c >= 0x60 && c <= 0x69)PbrbGbTile(g, graphics + (c - 0x60) * 16u, 2, x, 64 + row * 8); x += 8;
            }
        }
        palette = 0xcc; if (p->creditsPhase == 6) { unsigned frame = (unsigned)p->creditsElapsed / 5u; if (frame > 3)frame = 3; palette = g->rom[PCRED_PALETTES + frame]; }
    }
    for (int y = 0; y < 32; ++y)for (int x = 0; x < 160; ++x)PbrbGbPixel(g, x, y, 3, 0);
    for (int y = 112; y < 144; ++y)for (int x = 0; x < 160; ++x)PbrbGbPixel(g, x, y, 3, 0);
    PbrbGbApplyBGPalette(g, palette);
}
