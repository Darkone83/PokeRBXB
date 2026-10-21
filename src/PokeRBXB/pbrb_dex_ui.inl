/* Included once at the end of pbrb_game_menu.inl. Native translations of
   engine/menus/pokedex.asm; all labels, glyphs and names remain in the ROM.
   menuCursor is the visible row, menuCount the original list scroll offset. */
enum {
    PPLAY_DEX_DASHES = 0x401ed, PPLAY_DEX_SEEN = 0x4029d, PPLAY_DEX_OWN = 0x402a2,
    PPLAY_DEX_CONTENTS = 0x402a6, PPLAY_DEX_COMMANDS = 0x402af, PPLAY_DEX_BALL = 0x3a97e
};
static int PplayDexHas(const unsigned char* flags, unsigned dex) {
    return dex >= 1u && dex <= 151u && (flags[(dex - 1u) / 8u] & (1u << ((dex - 1u) % 8u)));
}
static unsigned PplayDexHighest(const PbrbPlay* p) {
    for (unsigned dex = 151; dex; --dex)if (PplayDexHas(p->gameState.seen, dex))return dex;
    return 0;
}
static unsigned PplayDexPosition(PbrbPlay* p) {
    unsigned highest = PplayDexHighest(p), rows = highest < 7u ? highest : 7u;
    int last = highest > 7u ? (int)highest - 7 : 0;
    if (p->menuCount < 0)p->menuCount = 0; if (p->menuCount > last)p->menuCount = last;
    if (p->menuCursor < 0)p->menuCursor = 0; if ((unsigned)p->menuCursor >= rows)p->menuCursor = rows ? (int)rows - 1 : 0;
    return highest;
}
static void PplayDexReturnList(PbrbPlay* p) {
    p->overlay = p->menuMode == 3 ? PBRB_OVERLAY_NONE : PBRB_OVERLAY_POKEDEX;
    p->selection = p->menuCursor; p->text.visible = 0;
}
static int PplayDexMenuTick(PbrbPlay* p, unsigned keys) {
    int overlay = p->overlay; PbrbGameRom rom = PplayRom(p);
    if (overlay == PBRB_OVERLAY_POKEDEX) {
        unsigned highest = PplayDexPosition(p), rows = highest < 7u ? highest : 7u;
        if (keys & PBRB_PAD_B) { PbrbAudioPlaySfx(PBRB_SFX_PRESS); PplayMenuBack(p); return 1; }
        if (keys & PBRB_PAD_UP) { if (p->menuCursor > 0)--p->menuCursor; else if (p->menuCount > 0)--p->menuCount; }
        else if (keys & PBRB_PAD_DOWN) {
            if ((unsigned)(p->menuCursor + 1) < rows)++p->menuCursor;
            else if (highest > 7u && (unsigned)p->menuCount < highest - 7u)++p->menuCount;
        }
        else if (keys & PBRB_PAD_RIGHT)p->menuCount += 7;
        else if (keys & PBRB_PAD_LEFT)p->menuCount -= 7;
        PplayDexPosition(p); p->selection = p->menuCursor;
        if (keys & PBRB_PAD_A) {
            PbrbAudioPlaySfx(PBRB_SFX_PRESS); unsigned dex = (unsigned)(p->menuCount + p->menuCursor + 1);
            if (highest && PplayDexHas(p->gameState.seen, dex)) {
                p->questionValue = (int)PbrbGameSpecies(&rom, dex);
                p->overlay = PBRB_OVERLAY_POKEDEX_SIDE; p->selection = 0; p->menuMode = 0;
            }
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_POKEDEX_SIDE) {
        if (keys & PBRB_PAD_B) { PbrbAudioPlaySfx(PBRB_SFX_PRESS); PplayDexReturnList(p); return 1; }
        if ((keys & PBRB_PAD_UP) && p->selection > 0)--p->selection;
        else if ((keys & PBRB_PAD_DOWN) && p->selection < 3)++p->selection;
        if (keys & PBRB_PAD_A) {
            PbrbAudioPlaySfx(PBRB_SFX_PRESS);
            if (p->selection == 0) { p->overlay = PBRB_OVERLAY_DEX_ENTRY; p->selection = 0; p->text.visible = 0; PbrbAudioPlayCry((unsigned)p->questionValue); }
            else if (p->selection == 1)PbrbAudioPlayCry((unsigned)p->questionValue);
            else if (p->selection == 2) { p->overlay = PBRB_OVERLAY_DEX_AREA; p->timer = 0; p->text.visible = 0; }
            else PplayMenuBack(p);
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_DEX_ENTRY) {
        unsigned dex = PbrbGameDex(&rom, (unsigned)p->questionValue);
        int owned = p->menuMode == 3 || PplayDexHas(p->gameState.owned, dex);
        if (p->selection > 1) { --p->selection; return 1; }
        if (keys & (PBRB_PAD_A | PBRB_PAD_B)) {
            if (!p->selection && owned) { PbrbAudioPlaySfx(PBRB_SFX_PRESS); p->selection = 21; }
            else PplayDexReturnList(p);
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_DEX_AREA) {
        p->timer = (p->timer + 1) % 50;
        if (keys & (PBRB_PAD_A | PBRB_PAD_B)) { PbrbAudioPlaySfx(PBRB_SFX_PRESS); PplayDexReturnList(p); }
        return 1;
    }
    return 0;
}
static int PplayDexMenuRender(PbrbPlay* p) {
    int overlay = p->overlay; PbrbGb* g = p->gb; PbrbGameRom rom = PplayRom(p);
    if (overlay == PBRB_OVERLAY_DEX_ENTRY) {
        unsigned dex = PbrbGameDex(&rom, (unsigned)p->questionValue);
        PbrbPlayDrawDexEntry(g, (unsigned)p->questionValue, p->selection ? 1 : 0, p->menuMode == 3 || PplayDexHas(p->gameState.owned, dex));
        if (p->selection > 1)for (int y = 11; y < 17; ++y)for (int x = 1; x < 19; ++x)PbrbGbGlyph(g, 0x7f, x * 8, y * 8);
        return 1;
    }
    if (overlay == PBRB_OVERLAY_DEX_AREA) { PbrbPlayDrawDexArea(g, (unsigned)p->questionValue, (unsigned)p->world->romMapId, (unsigned)p->timer); return 1; }
    if (overlay != PBRB_OVERLAY_POKEDEX && overlay != PBRB_OVERLAY_POKEDEX_SIDE)return 0;
    unsigned highest = PplayDexPosition(p); PbrbGbSetPalettes(g, 0xe4u, 0xd0u, 0xe4u); PbrbGbClear(g, 0);
    for (int y = 0; y < 18; ++y)PplayUIGlyph(g, y == 0 || y % 2 ? 0x71u : 0x70u, 112, y * 8, 1);
    for (int x = 15; x < 20; ++x)PbrbGbGlyph(g, 0x7a, x * 8, 64);
    PplayUIString(g, PPLAY_DEX_SEEN, 128, 16, 1, 5); PplayUIString(g, PPLAY_DEX_OWN, 128, 40, 1, 4);
    PplayUIString(g, PPLAY_DEX_CONTENTS, 8, 8, 1, 9); PplayUIString(g, PPLAY_DEX_COMMANDS, 128, 80, 1, 20);
    PplayUINumber(g, PbrbGameDexCount(p->gameState.seen), 3, 128, 24, 0);
    PplayUINumber(g, PbrbGameDexCount(p->gameState.owned), 3, 128, 48, 0);
    for (unsigned row = 0; row < 7u && (unsigned)p->menuCount + row < highest; ++row) {
        unsigned dex = (unsigned)p->menuCount + row + 1u; int y = 16 + (int)row * 16;
        PplayUINumber(g, dex, 3, 8, y, 1);
        if (PplayDexHas(p->gameState.owned, dex))PbrbGbTile(g, PPLAY_DEX_BALL, 2, 24, y + 8);
        if (PplayDexHas(p->gameState.seen, dex)) { unsigned char name[11]; PbrbGameMonName(&rom, PbrbGameSpecies(&rom, dex), name); PbrbGbCodes(g, name, 32, y + 8, 10); }
        else PplayUIString(g, PPLAY_DEX_DASHES, 32, y + 8, 1, 11);
    }
    if (highest)PbrbGbGlyph(g, overlay == PBRB_OVERLAY_POKEDEX ? 0xed : 0xec, 0, 24 + p->menuCursor * 16);
    if (overlay == PBRB_OVERLAY_POKEDEX_SIDE)PbrbGbGlyph(g, 0xed, 120, 80 + p->selection * 16);
    return 1;
}
