#ifndef PBRB_SERVICES_INL_INCLUDED
#define PBRB_SERVICES_INL_INCLUDED
/* Original Red/Blue NPC services. This file is included once, after the game
   menus, so the existing RXDK project needs no extra translation unit. The
   service owns a pending world event until its actual state change succeeds.
   Texts, prices, trade records, reels and artwork are read from the user's ROM.
   References: scripts/Daycare.asm, engine/events/{in_game_trades,prize_menu,
   pokedex_rating,give_pokemon}.asm, engine/slots/slot_machine.asm and
   engine/movie/{trade,hall_of_fame}.asm in the original disassembly. */
#include "pbrb_service_data.h"
enum PbrbServiceStep {
    PSS_DONE, PSS_GIFT_TEXT, PSS_GIFT_ASK, PSS_GIFT_NAMING, PSS_GIFT_BOX,
    PSS_DAYCARE_ASK = 20, PSS_DAYCARE_PARTY, PSS_DAYCARE_DEPOSIT,
    PSS_DAYCARE_GROWTH, PSS_DAYCARE_PAY, PSS_DAYCARE_RETURN,
    PSS_TRADE_ASK = 40, PSS_TRADE_PARTY, PSS_TRADE_START,
    PSS_TRADE_ANIMATE, PSS_TRADE_COMMIT, PSS_TRADE_THANKS,
    PSS_DEX_RATING = 60, PSS_DEX_SOUND, PSS_DEX_WAIT,
    PSS_HOF_START = 70, PSS_HOF_BACK, PSS_HOF_FRONT, PSS_HOF_INFO,
    PSS_HOF_TITLE, PSS_HOF_FADE, PSS_HOF_PLAYER_STATS,
    PSS_HOF_PLAYER_SEEN, PSS_HOF_PLAYER_RATING, PSS_HOF_PLAYER_END,
    PSS_HOF_VIEW_START = 85, PSS_HOF_VIEW,
    PSS_PRIZE_INTRO = 90, PSS_PRIZE_LIST, PSS_PRIZE_ASK,
    PSS_VENDOR_ASK = 100, PSS_ELEVATOR = 104,
    PSS_VENDING_MENU, PSS_VENDING_DELIVERY, PSS_VENDING_PAID, PSS_ITEM_CHOICE,
    PSS_SLOTS_ASK = 110, PSS_SLOTS_BET_TEXT, PSS_SLOTS_BET,
    PSS_SLOTS_START, PSS_SLOTS_SPIN, PSS_SLOTS_MATCH,
    PSS_SLOTS_WIN, PSS_SLOTS_FLASH, PSS_SLOTS_PAY,
    PSS_SLOTS_AGAIN_TEXT, PSS_SLOTS_AGAIN, PSS_SLOTS_EMPTY
};
static unsigned PsRom(const PbrbPlay* p, unsigned label) {
    return label < PSR_COUNT ? kPbrbServiceRom[p->game == 2 ? 1 : 0][label] : 0;
}
static unsigned PsRead(const PbrbPlay* p, unsigned at) {
    return at < p->gb->bytes ? p->gb->rom[at] : 0;
}
static unsigned PsPointer(const PbrbPlay* p, unsigned at, unsigned bankBase) {
    if (at + 1u >= p->gb->bytes)return 0;
    unsigned a = PsRead(p, at) | (PsRead(p, at + 1u) << 8);
    if (a < 0x4000u || a >= 0x8000u)return 0;
    a = (bankBase & ~0x3fffu) + a - 0x4000u;
    return a < p->gb->bytes ? a : 0;
}
static void PsTextAt(PbrbPlay* p, unsigned at, int next, int wait = 0) {
    p->serviceStep = next; PplayBeginText(p, at, wait, PPLAY_AFTER_NONE, 0);
}
static void PsText(PbrbPlay* p, unsigned label, int next, int wait = 0) {
    PsTextAt(p, PsRom(p, label), next, wait);
}
static void PsGameText(PbrbPlay* p, int id, int next, int wait = 0) {
    p->serviceStep = next; PplayGameText(p, id, wait, PPLAY_AFTER_NONE);
}
static void PsDone(PbrbPlay* p, int result) {
    int kind = p->serviceEvent.kind;
    p->overlay = p->serviceReturn; p->text.visible = 0; p->selection = 0;
    PplaySyncAliases(p); PplayWorldPlayer(p);
    if (!(p->serviceEvent.flags & 0x80000000u)) {
        PbrbWorldAcknowledge(p->world, result); p->worldEvent = 0;
    }
    if (kind == PBRB_WORLD_HALL_OF_FAME && result && !(p->serviceEvent.flags & 0x80000000u))p->endingPending = 1;
    p->serviceEvent.kind = PBRB_WORLD_NONE; p->serviceStep = PSS_DONE;
}
static const unsigned char* PbrbPlayServiceRam(PbrbPlay* p, unsigned addr) {
    if (p->serviceEvent.kind == PBRB_WORLD_TRADE) {
        if (addr == 0xcd13)return p->serviceData;
        if (addr == 0xcd1e)return p->serviceData + 11;
        if (addr == 0xd887)return p->serviceData + 33;
    }
    if (addr == 0xda49 && p->serviceEvent.kind == PBRB_WORLD_DAYCARE)
        return p->gameState.daycare.nickname;
    if (addr == 0xde06 && (p->serviceEvent.kind == PBRB_WORLD_GIVE_MON ||
        p->serviceEvent.kind == PBRB_WORLD_PRIZE_MENU))return p->serviceData;
    return 0;
}
static void PsDigits(PbrbGb* g, unsigned n, int x, int y, unsigned digits) {
    unsigned place = 1; for (unsigned i = 1; i < digits; ++i)place *= 10u;
    for (unsigned i = 0; i < digits; ++i) { PbrbGbGlyph(g, (unsigned char)(0xf6u + (n / place) % 10u), x + (int)i * 8, y); place /= 10u; }
}
static int PsChoice(PbrbPlay* p, unsigned keys) {
    if (keys & (PBRB_PAD_UP | PBRB_PAD_DOWN))p->serviceSelection ^= 1;
    if (keys & PBRB_PAD_B)return -1;
    if (keys & PBRB_PAD_A)return p->serviceSelection ? -1 : 1;
    return 0;
}
static void PsYesNo(PbrbPlay* p, int x = 112, int y = 56) {
    PbrbGbTextBox(p->gb, x, y, 6, 5); PbrbGbString(p->gb, "YES", x + 16, y + 8);
    PbrbGbString(p->gb, "NO", x + 16, y + 24);
    PbrbGbGlyph(p->gb, 0xed, x + 8, y + 8 + p->serviceSelection * 16);
}
static void PsMoney(PbrbPlay* p) {
    PbrbGbTextBox(p->gb, 80, 0, 10, 3); PbrbGbString(p->gb, "MONEY", 88, 0);
    PbrbGbGlyph(p->gb, 0xf0, 96, 8); PsDigits(p->gb, p->gameState.money, 104, 8, 6);
}
static void PsCoins(PbrbPlay* p) {
    PbrbGbTextBox(p->gb, 88, 0, 9, 3); PbrbGbString(p->gb, "COIN", 96, 0);
    PsDigits(p->gb, p->gameState.coins, 104, 8, 4);
}
static void PsParty(PbrbPlay* p) {
    int selection = p->selection, swap = p->menuSwap;
    p->selection = p->serviceSelection; p->menuSwap = -1; PplayMenuPartyRender(p);
    p->selection = selection; p->menuSwap = swap;
}
static int PsPartyChoice(PbrbPlay* p, unsigned keys) {
    unsigned count = p->gameState.partyCount; if (!count)return -1;
    if (keys & PBRB_PAD_UP)p->serviceSelection = (p->serviceSelection + (int)count - 1) % (int)count;
    if (keys & PBRB_PAD_DOWN)p->serviceSelection = (p->serviceSelection + 1) % (int)count;
    if (keys & PBRB_PAD_B)return -1;
    return (keys & PBRB_PAD_A) ? p->serviceSelection + 1 : 0;
}
static void PsRemoveParty(PbrbGameState* s, unsigned index) {
    if (index >= s->partyCount)return;
    memmove(s->party + index, s->party + index + 1, (s->partyCount - index - 1u) * sizeof(PbrbPlayMon));
    --s->partyCount; memset(s->party + s->partyCount, 0, sizeof(PbrbPlayMon));
    if (s->active > index)--s->active; else if (s->active == index)s->active = 0;
}
static int PsGiveMon(PbrbPlay* p, unsigned species, unsigned level, int showText) {
    PbrbGameRom rom = PplayRom(p); PbrbGameState* s = &p->gameState;
    if (s->partyCount >= 6 && (s->currentBox >= 12 || s->boxCount[s->currentBox] >= 20)) {
        p->serviceValue = 0; PsGameText(p, PBRB_GT_BoxIsFullText, PSS_DONE, 1); return 0;
    }
    PbrbPlayMon mon; if (!PbrbGameMakeMon(&rom, &mon, species, level, &p->rng, p->trainerID, p->player, 0)) {
        PsDone(p, 0); return 0;
    }
    p->serviceData[40] = (unsigned char)(s->partyCount < 6 ? 1 : 2);
    p->serviceData[41] = (unsigned char)(s->partyCount < 6 ? s->partyCount : 0);
    p->serviceData[42] = s->currentBox;
    if (!PbrbGameAddMon(&rom, s, &mon)) { PsDone(p, 0); return 0; }
    p->serviceEvent.species = species; PplayCopyName(p->serviceData, mon.nickname, 10);
    PplayCopyName(p->nameBuffer, mon.nickname, 10); p->serviceValue = 1;
    PplaySyncAliases(p);
    if (showText) { PsGameText(p, PBRB_GT_GotMonText, PSS_GIFT_TEXT); PbrbAudioPlaySfx(PBRB_SFX_GET_ITEM); }
    else p->serviceStep = PSS_GIFT_TEXT;
    return 1;
}
static void PsGiftAfterText(PbrbPlay* p) {
    if (p->serviceData[40] == 1) {
        p->serviceSelection = 0; p->serviceStep = PSS_GIFT_ASK;
        PplayText(p, PBRB_DATA_DoYouWantToNicknameText, 0, PPLAY_AFTER_NONE);
    }
    else {
        unsigned box = p->serviceData[42] + 1u; memset(p->stringBuffer, 0x50, sizeof(p->stringBuffer));
        if (box >= 10) { p->stringBuffer[0] = 0xf7; p->stringBuffer[1] = (unsigned char)(0xf6 + box % 10u); }
        else p->stringBuffer[0] = (unsigned char)(0xf6 + box);
        PsGameText(p, PBRB_GT_SentToBoxText, PSS_DONE, 1);
    }
}
static void PbrbPlayServiceNamed(PbrbPlay* p) {
    PbrbGameState* s = &p->gameState; PbrbPlayMon* m = 0;
    if (p->serviceData[40] == 1 && p->serviceData[41] < s->partyCount)m = s->party + p->serviceData[41];
    else if (p->serviceData[40] == 2 && p->serviceData[42] < 12 &&
        p->serviceData[41] < s->boxCount[p->serviceData[42]])m = &s->boxes[p->serviceData[42]][p->serviceData[41]];
    if (m && p->namingLength)PplayCopyName(m->nickname, p->naming, 10);
    p->overlay = PBRB_OVERLAY_SERVICE; p->text.visible = 0; p->serviceStep = PSS_DONE;
    PplaySyncAliases(p);
}
static void PsDaycareBegin(PbrbPlay* p) {
    PbrbGameState* s = &p->gameState; p->serviceSelection = 0; p->serviceValue = 0;
    if (!s->daycareOccupied) { PsText(p, PSR_DaycareGentlemanText_IntroText, PSS_DAYCARE_ASK); return; }
    PbrbGameRom rom = PplayRom(p); unsigned level = s->daycareLevel;
    while (level < 100 && s->daycare.experience >= PbrbGameExperience(&rom, s->daycare.species, level + 1u))++level;
    if (level >= 100)s->daycare.experience = PbrbGameExperience(&rom, s->daycare.species, 100);
    p->serviceValue = (int)level; p->textNumber = level - s->daycareLevel;
    p->serviceCost = (int)(100u * (p->textNumber + 1u)); p->textMoney = (unsigned)p->serviceCost;
    PplayCopyName(p->nameBuffer, s->daycare.nickname, 10);
    PsText(p, p->textNumber ? PSR_DaycareGentlemanText_MonHasGrownText :
        PSR_DaycareGentlemanText_MonNeedsMoreTimeText, PSS_DAYCARE_GROWTH);
}
static void PsDaycareWithdraw(PbrbPlay* p) {
    PbrbGameState* s = &p->gameState; PbrbGameRom rom = PplayRom(p); PbrbPlayMon m = s->daycare;
    m.level = (unsigned char)p->serviceValue;
    for (unsigned lv = s->daycareLevel + 1; lv <= m.level; ++lv) {
        unsigned char learned[8]; unsigned count = PbrbGameMovesAtLevel(&rom, m.species, lv, learned);
        for (unsigned n = 0; n < count; ++n) {
            unsigned i = 0;
            for (; i < 4; ++i)if (m.moves[i] == learned[n])break; if (i < 4)continue;
            for (i = 0; i < 4 && m.moves[i]; ++i) {}
            if (i == 4) { memmove(m.moves, m.moves + 1, 3); memmove(m.pp, m.pp + 1, 3); i = 3; }
            m.moves[i] = learned[n]; m.pp[i] = (unsigned char)PbrbGameMaxPP(&rom, learned[n], 0);
        }
    }
    PbrbGameRecalculate(&rom, &m, 0); m.hp = m.maxHP;
    s->party[s->partyCount++] = m; s->money -= (unsigned)p->serviceCost;
    s->daycareOccupied = 0; s->daycareSteps = 0; s->daycareLevel = 0;
    p->serviceValue = 1; PplaySyncAliases(p); PbrbAudioPlaySfx(PBRB_SFX_PURCHASE);
    PsText(p, PSR_DaycareGentlemanText_HeresYourMonText, PSS_DAYCARE_RETURN);
}
static unsigned PsTradeText(PbrbPlay* p, unsigned type) {
    unsigned table = PsRom(p, PSR_InGameTradeTextPointers);
    unsigned at = PsPointer(p, table + p->serviceData[60] * 2u, table);
    return PsPointer(p, at + type * 2u, at);
}
static int PsTradeBegin(PbrbPlay* p) {
    if (p->serviceEvent.value < 0 || p->serviceEvent.value >= 10)return 0;
    PbrbGameRom rom = PplayRom(p); unsigned at = PsRom(p, PSR_TradeMons) + (unsigned)p->serviceEvent.value * 14u;
    p->serviceEvent.itemId = PsRead(p, at); p->serviceEvent.species = PsRead(p, at + 1);
    p->serviceData[60] = (unsigned char)PsRead(p, at + 2);
    if (p->serviceData[60] > 2)return 0;
    PbrbGameMonName(&rom, p->serviceEvent.itemId, p->serviceData);
    PbrbGameMonName(&rom, p->serviceEvent.species, p->serviceData + 11);
    memcpy(p->serviceData + 22, p->gb->rom + at + 3, 11);
    memcpy(p->serviceData + 33, p->gb->rom + PsRom(p, PSR_InGameTrade_TrainerString), 11);
    p->serviceValue = 0; p->serviceSelection = 0;
    if (p->serviceEvent.flags & 0x02000000u)PsTextAt(p, PsTradeText(p, 4), PSS_DONE);
    else PsTextAt(p, PsTradeText(p, 0), PSS_TRADE_ASK);
    return 1;
}
static int PsTradeCommit(PbrbPlay* p) {
    PbrbGameRom rom = PplayRom(p); PbrbGameState* s = &p->gameState;
    unsigned index = p->serviceData[61]; if (index >= s->partyCount)return 0;
    PbrbPlayMon m; if (!PbrbGameMakeMon(&rom, &m, p->serviceEvent.species,
        s->party[index].level, &p->rng, (unsigned)p->serviceCost, p->serviceData + 33, 0))return 0;
    PplayCopyName(m.nickname, p->serviceData + 22, 10);
    PsRemoveParty(s, index); s->party[s->partyCount++] = m; PbrbGameMarkDex(&rom, s, m.species, 1);
    PplaySyncAliases(p); p->serviceValue = 1; return 1;
}
enum PbrbTradePhase {
    PST_PLAYER_LOAD, PST_PLAYER_SLIDE, PST_PLAYER_HOLD, PST_PLAYER_POOF,
    PST_PLAYER_DROP, PST_CABLE_OUT, PST_BALL_SHAKE, PST_BALL_WAIT, PST_BULGE_OUT,
    PST_CABLE_CLEAR, PST_OUT_INIT, PST_OUT_H1, PST_OUT_H2, PST_OUT_H3,
    PST_OUT_V1, PST_OUT_V2, PST_OUT_DELAY, PST_WENT, PST_WENT_WAIT,
    PST_WENT_SLIDE, PST_WENT_TAIL, PST_FOR, PST_SENDS, PST_FAREWELL,
    PST_TRANSFERRED, PST_TRANSFERRED_WAIT, PST_TRANSFERRED_SLIDE, PST_TRANSFERRED_TAIL,
    PST_RETURN_INIT, PST_RETURN_V1, PST_RETURN_V2, PST_RETURN_H1, PST_RETURN_H2,
    PST_RETURN_H3, PST_RETURN_CLEAR, PST_CABLE_IN, PST_BALL_TILT,
    PST_ENEMY_INFO, PST_ENEMY_LOAD, PST_ENEMY_POOF, PST_ENEMY_CRY, PST_CARE, PST_FINAL_DELAY
};
static void PsMapBox(unsigned char* map, int x, int y, int cols, int rows) {
    for (int row = 0; row < rows; ++row)for (int col = 0; col < cols; ++col) {
        if (x + col < 0 || x + col >= 32 || y + row < 0 || y + row >= 18)continue; unsigned char c = 0x7f;
        if (!row)c = (unsigned char)(!col ? 0x79 : col == cols - 1 ? 0x7b : 0x7a);
        else if (row == rows - 1)c = (unsigned char)(!col ? 0x7d : col == cols - 1 ? 0x7e : 0x7a);
        else if (!col || col == cols - 1)c = 0x7c;
        map[(y + row) * 32 + x + col] = c;
    }
}
static void PsMapCodes(unsigned char* map, const unsigned char* codes, unsigned count, int x, int y) {
    int left = x; for (unsigned i = 0; i < count; ++i) {
        unsigned c = codes[i]; if (c == 0x50)break;
        if (c == 0x4e) { x = left; y += 2; continue; }
        if (c == 0x5d) {
            static const char name[] = "TRAINER";
            for (unsigned j = 0; j < 7; ++j) { if (x >= 0 && x < 32 && y >= 0 && y < 18)map[y * 32 + x] = PplayChar(name[j]); ++x; }continue;
        }
        if (x >= 0 && x < 32 && y >= 0 && y < 18)map[y * 32 + x] = (unsigned char)c; ++x;
    }
}
static void PsMapDigits(unsigned char* map, unsigned n, int x, int y, unsigned digits) {
    unsigned place = 1; for (unsigned i = 1; i < digits; ++i)place *= 10u;
    for (unsigned i = 0; i < digits; ++i) { if (x + (int)i < 32 && y < 18)map[y * 32 + x + i] = (unsigned char)(0xf6 + (n / place) % 10u); place /= 10u; }
}
static void PsTradeTile(PbrbPlay* p, unsigned c, int x, int y) {
    if (c >= 0x31u && c <= 0x61u)PbrbGbTile(p->gb, PsRom(p, PSR_TradingAnimationGraphics) + (c - 0x31u) * 16u, 2, x, y);
    else PbrbGbGlyph(p->gb, (unsigned char)c, x, y);
}
static void PsTradeMap(PbrbPlay* p, const unsigned char* map, unsigned scroll, int windowX = 0, int windowY = 0, int isWindow = 0) {
    for (int y = 0; y < 18; ++y)for (int x = 0; x < 32; ++x) {
        int xx = x * 8 - (int)scroll, yy = y * 8;
        if (isWindow) {
            xx = x * 8 + windowX; yy += windowY; if (xx >= 160 || yy >= 144)continue;
            PsTradeTile(p, map[y * 32 + x], xx, yy);
        }
        else { if (xx < -7)xx += 256; PsTradeTile(p, map[y * 32 + x], xx, yy); }
    }
}
static void PsTradeMapTiles(PbrbPlay* p, unsigned char* map, unsigned label, int x, int y, unsigned width, unsigned height) {
    unsigned at = PsRom(p, label); for (unsigned row = 0; row < height; ++row)for (unsigned col = 0; col < width; ++col)
        map[(y + (int)row) * 32 + x + col] = (unsigned char)PsRead(p, at + row * width + col);
}
static void PsTradeInfo(PbrbPlay* p, unsigned char* map, int enemy, int y) {
    PsMapBox(map, 4, y, 12, 8); PsMapCodes(map, p->gb->rom + PsRom(p, PSR_Trade_MonInfoText), 32, 5, y);
    PbrbGameRom rom = PplayRom(p); unsigned species = enemy ? p->serviceEvent.species : p->serviceEvent.itemId;
    PsMapDigits(map, PbrbGameDex(&rom, species), 9, y, 3);
    PsMapCodes(map, p->serviceData + (enemy ? 11 : 0), 11, 5, y + 2);
    const PbrbPlayMon* m = p->gameState.party + p->serviceData[61];
    PsMapCodes(map, enemy ? p->serviceData + 33 : m->originalTrainer, 11, 8, y + 4);
    PsMapDigits(map, enemy ? (unsigned)p->serviceCost : m->ot, 8, y + 6, 5);
}
static void PsTradeCable(PbrbPlay* p, unsigned scroll) {
    unsigned char map[32 * 18]; memset(map, 0x7f, sizeof(map));
    PsTradeMapTiles(p, map, PSR_LinkCableTiles, 6, 2, 12, 3); PsTradeMap(p, map, scroll);
}
static void PsTradeGameboy(PbrbPlay* p, int side, unsigned scroll) {
    unsigned char map[32 * 18]; memset(map, 0x7f, sizeof(map));
    /* The original copies the cable into the offscreen 32-column BG map. */
    for (unsigned x = 0; x < 32; ++x)map[4 * 32 + x] = 0x5e;
    if (side == 0) {
        for (unsigned y = 0; y < 18; ++y)for (unsigned x = 0; x < 20; ++x)map[y * 32 + x] = 0x7f;
        map[4 * 32 + 11] = 0x5d; for (unsigned x = 12; x < 20; ++x)map[4 * 32 + x] = 0x5e;
        PsTradeMapTiles(p, map, PSR_GameBoyTiles, 5, 3, 6, 8); PsMapBox(map, 4, 12, 9, 4); PsMapCodes(map, p->player, 11, 5, 14);
    }
    else if (side == 1) {
        for (unsigned y = 0; y < 18; ++y)for (unsigned x = 0; x < 20; ++x)map[y * 32 + x] = 0x7f;
        for (unsigned x = 0; x < 14; ++x)map[4 * 32 + x] = 0x5e; map[4 * 32 + 14] = 0x5f;
        for (unsigned y = 5; y < 9; ++y)map[y * 32 + 14] = 0x61; map[9 * 32 + 14] = 0x60; map[9 * 32 + 13] = 0x5d;
        PsTradeMapTiles(p, map, PSR_GameBoyTiles, 7, 8, 6, 8); PsMapBox(map, 6, 0, 9, 4); PsMapCodes(map, p->serviceData + 33, 11, 7, 2);
    }
    PsTradeMap(p, map, scroll);
}
static unsigned PsIconTile(PbrbPlay* p, unsigned tile) {
    unsigned table = PsRom(p, PSR_MonPartySpritePointers);
    for (unsigned i = 0; i < 28; ++i) {
        unsigned at = table + i * 6u;
        unsigned dest = (PsRead(p, at + 4) | (PsRead(p, at + 5) << 8)) - 0x8000u; dest /= 16u;
        if (tile >= dest && tile < dest + PsRead(p, at + 2)) {
            unsigned a = PsRead(p, at) | (PsRead(p, at + 1) << 8);
            return PsRead(p, at + 3) * 0x4000u + a - 0x4000u + (tile - dest) * 16u;
        }
    }return 0;
}
static void PsTradeCircle(PbrbPlay* p, unsigned species, int baseX, int baseY, unsigned frame) {
    PbrbGameRom rom = PplayRom(p); unsigned dex = PbrbGameDex(&rom, species); if (!dex)return;
    unsigned packed = PsRead(p, PsRom(p, PSR_MonPartyData) + (dex - 1u) / 2u), icon = (dex & 1u) ? packed >> 4 : packed & 15u;
    unsigned shift = (frame & 1u) ? 64u : 0u;
    for (unsigned y = 0; y < 2; ++y)for (unsigned x = 0; x < 2; ++x) {
        unsigned tile = (icon * 4u + y * 2u + (icon == 2 ? x : 0u)) ^ shift;
        PbrbGbSpriteTile(p->gb, PsIconTile(p, tile), baseX + 8 + (int)x * 8, baseY + (int)y * 8, 0xe4, icon != 2 && x ? 0x20 : 0);
    }
    unsigned table = PsRom(p, PSR_Trade_CircleOAMBlocks);
    for (unsigned block = 0; block < 4; ++block) {
        unsigned at = table + block * 4u, def = PsPointer(p, at, table);
        int x = (int)PsRead(p, at + 2) - 8 + baseX, y = (int)PsRead(p, at + 3) - 16 + baseY;
        for (unsigned tile = 0; tile < 4; ++tile) {
            unsigned id = PsRead(p, def + tile * 2u) ^ shift, attr = PsRead(p, def + tile * 2u + 1u);
            PbrbGbSpriteTile(p->gb, PsIconTile(p, id), x + (int)(tile & 1u) * 8, y + (int)(tile / 2u) * 8, 0xd0, attr);
        }
    }
}
static void PsTradeWindowText(PbrbPlay* p, int x) {
    if (!p->text.visible)return; PbrbGbTextBox(p->gb, x, 96, 20, 6);
    for (int row = 0; row < 4; ++row)for (int col = 0; col < 18; ++col)
        PbrbGbGlyph(p->gb, p->text.cells[row][col], x + 8 + col * 8, 104 + row * 8);
}
static int PsTradeSliding(PbrbPlay* p) {
    return p->serviceSelection == PST_WENT_SLIDE || p->serviceSelection == PST_TRANSFERRED_SLIDE;
}
static void PsTradeRender(PbrbPlay* p) {
    PbrbGb* g = p->gb; int phase = p->serviceSelection; unsigned t = p->serviceTimer;
    PbrbGbSetPalettes(g, 0xe4, 0xe4, 0xd0); PbrbGbClear(g, 0);
    if (phase <= PST_PLAYER_DROP) {
        unsigned scroll = phase == PST_PLAYER_LOAD ? 134u : phase == PST_PLAYER_SLIDE ? 126u - t * 2u : 2u;
        int x = (int)((56u - scroll) & 255u); PbrbGbBitmap(g, p->picture, 56, 56, x, 16, 0, 1);
        PbrbGbBitmap(g, p->picture, 56, 56, x - 256, 16, 0, 1);
        unsigned char map[32 * 18]; memset(map, 0x7f, sizeof(map)); PsTradeInfo(p, map, 0, 0);
        PsTradeMap(p, map, 0, (int)scroll - 7, 80, 1); return;
    }
    if ((phase >= PST_CABLE_OUT && phase <= PST_BULGE_OUT) || phase == PST_CABLE_IN || phase == PST_BALL_TILT) {
        unsigned scroll = phase == PST_BALL_TILT ? (240u - (unsigned)p->battleAnim.scrollX) & 255u : 240u;
        PsTradeCable(p, scroll);
        if (phase == PST_BULGE_OUT) {
            unsigned n = t / 3u, id = 2u + ((n + 1u) & 1u);
            for (unsigned i = 0; i < 4; ++i)PbrbGbSpriteTile(g, PsRom(p, PSR_TradingAnimationGraphics2) + id * 16u,
                88 + (int)n * 4 + (int)(i & 1u) * 8, 16 + (int)(i / 2u) * 8, 0xe4, (i & 1u ? 0x20u : 0u) | (i & 2u ? 0x40u : 0u));
        }
        return;
    }
    if ((phase >= PST_OUT_INIT && phase <= PST_OUT_DELAY) || (phase >= PST_RETURN_INIT && phase <= PST_RETURN_H3)) {
        int incoming = phase >= PST_RETURN_INIT, side = incoming ? 1 : 0, x = incoming ? 100 : 84, y = incoming ? 68 : 28;
        unsigned scroll = 0, frame = 0; int icon = phase != PST_OUT_DELAY;
        if (phase == PST_OUT_H1) { scroll = t * 2u; frame = t / 8u; }
        else if (phase == PST_OUT_H2) { side = 2; scroll = 96u + t * 2u; frame = 6u + t / 8u; }
        else if (phase == PST_OUT_H3) { side = 1; scroll = 160u + t * 2u; frame = 10u + t / 8u; }
        else if (phase == PST_OUT_V1) { side = 1; x += 4 * (int)(t / 8u + 1u); frame = 17u + t / 8u; }
        else if (phase == PST_OUT_V2) { side = 1; x += 16; y += 10 * (int)(t / 8u + 1u); frame = 21u + t / 8u; }
        else if (phase == PST_OUT_DELAY)side = 1;
        else if (phase == PST_RETURN_V1) { y -= 10 * (int)(t / 8u + 1u); frame = 1u + t / 8u; }
        else if (phase == PST_RETURN_V2) { y = 28; x -= 4 * (int)(t / 8u + 1u); frame = 5u + t / 8u; }
        else if (phase == PST_RETURN_H1) { y = 28; x = 84; scroll = (256u - t * 2u) & 255u; frame = 8u + t / 8u; }
        else if (phase == PST_RETURN_H2) { side = 2; y = 28; x = 84; scroll = 160u - t * 2u; frame = 14u + t / 8u; }
        else if (phase == PST_RETURN_H3) { side = 0; y = 28; x = 84; scroll = 96u - t * 2u; frame = 18u + t / 8u; }
        unsigned palette = (frame & 1u) ? 0xd8u : 0xe4u; PbrbGbSetPalettes(g, palette, 0xe4, 0xd0);
        PsTradeGameboy(p, side, scroll & 255u);
        if (icon)PsTradeCircle(p, incoming ? p->serviceEvent.species : p->serviceEvent.itemId, x, y, frame); return;
    }
    if (PsTradeSliding(p) || phase == PST_WENT_TAIL || phase == PST_TRANSFERRED_TAIL) {
        PsTradeGameboy(p, 1, 144);
        if (PsTradeSliding(p)) {
            int left = (int)t * 2;
            for (int y = 0; y < 144; ++y)for (int x = left; x < 160; ++x)PbrbGbPixel(g, x, y, 0, 0);
            PsTradeWindowText(p, left);
        }return;
    }
    if (phase >= PST_ENEMY_INFO && phase <= PST_ENEMY_CRY) {
        unsigned char map[32 * 18]; memset(map, 0x7f, sizeof(map)); PsTradeInfo(p, map, 1, 10); PsTradeMap(p, map, 0);
        if (phase >= PST_ENEMY_LOAD)PbrbGbBitmap(g, p->enemyPicture, 56, 56, 56, 16, 0, 1); return;
    }
    if (phase == PST_CARE || phase == PST_FINAL_DELAY)PbrbGbBitmap(g, p->enemyPicture, 56, 56, 56, 16, 0, 1);
}
static void PsTradeInstantText(PbrbPlay* p, int id) {
    PplayGameText(p, id, 0, PPLAY_AFTER_NONE);
    for (unsigned i = 0; i < PBRB_PLAY_TEXT_MAX && p->text.active && !p->text.waiting; ++i) { p->text.delay = 0; PplayTextTick(p, 0, 0); }
}
static void PsTradeSetPhase(PbrbPlay* p, int phase) {
    p->serviceSelection = phase; p->serviceTimer = 0;
    if (phase == PST_WENT)PsTradeInstantText(p, PBRB_GT_TradeWentToText);
    else if (phase == PST_FOR)PsTradeInstantText(p, PBRB_GT_TradeForText);
    else if (phase == PST_SENDS)PsTradeInstantText(p, PBRB_GT_TradeSendsText);
    else if (phase == PST_FAREWELL)PsTradeInstantText(p, PBRB_GT_TradeWavesFarewellText);
    else if (phase == PST_TRANSFERRED)PsTradeInstantText(p, PBRB_GT_TradeTransferredText);
    else if (phase == PST_CARE)PsTradeInstantText(p, PBRB_GT_TradeTakeCareText);
    if (phase == PST_PLAYER_POOF || phase == PST_PLAYER_DROP || phase == PST_BALL_SHAKE ||
        phase == PST_BALL_TILT || phase == PST_ENEMY_POOF) {
        unsigned id = (phase == PST_PLAYER_DROP) ? 170u : phase == PST_BALL_SHAKE ? 171u : phase == PST_BALL_TILT ? 172u : 173u;
        p->serviceData[44] = (unsigned char)id; PsTradeRender(p);
        PbrbBattleAnimInit(&p->battleAnim, p->gb, id, 0, p->serviceEvent.itemId, p->serviceEvent.species);
    }
    else p->serviceData[44] = 0;
    if (phase == PST_CABLE_OUT)PbrbAudioPlayCry(p->serviceEvent.itemId);
    if (phase == PST_CABLE_OUT || phase == PST_CABLE_IN)PbrbAudioPlaySfx(PBRB_AUDIO_RAW(2, 0x8d));
    if (phase == PST_ENEMY_CRY)PbrbAudioPlayCry(p->serviceEvent.species);
    if (phase == PST_OUT_INIT || phase == PST_RETURN_INIT || phase == PST_CABLE_OUT || phase == PST_CABLE_IN ||
        phase == PST_RETURN_CLEAR || phase == PST_CABLE_CLEAR || phase == PST_ENEMY_INFO ||
        phase == PST_WENT_TAIL || phase == PST_TRANSFERRED_TAIL)p->text.visible = 0;
}
static void PsTradeStart(PbrbPlay* p) {
    p->serviceStep = PSS_TRADE_ANIMATE; p->text.visible = 0;
    PplayCopyName(p->stringBuffer, p->serviceData, 10); PplayCopyName(p->nameBuffer, p->serviceData + 11, 10);
    PbrbGbDecodePokemon(p->gb, p->serviceEvent.itemId, 0, p->picture, sizeof(p->picture));
    PbrbGbDecodePokemon(p->gb, p->serviceEvent.species, 0, p->enemyPicture, sizeof(p->enemyPicture));
    PsTradeSetPhase(p, PST_PLAYER_LOAD);
}
static void PsTradeTick(PbrbPlay* p) {
    int phase = p->serviceSelection;
    if (p->serviceData[44]) {
        PbrbBattleAnimTick(&p->battleAnim); if (!PbrbBattleAnimDone(&p->battleAnim))return;
        PsTradeSetPhase(p, phase + 1); return;
    }
    static const unsigned char duration[] = {
        10,63,80,0,0,4,0,10,48,3,14,48,32,48,32,32,100,200,50,77,10,
        80,80,80,80,50,77,10,14,32,32,48,32,48,1,4,0,3,10,0,100,80,100
    };
    if (phase == PST_BULGE_OUT && p->serviceTimer && p->serviceTimer % 3u == 0)PbrbAudioPlaySfx(PBRB_SFX_TINK);
    if (++p->serviceTimer < duration[phase])return;
    if (phase == PST_FINAL_DELAY) { p->text.visible = 0; p->battleAnim.active = 0; p->serviceStep = PSS_TRADE_COMMIT; return; }
    PsTradeSetPhase(p, phase + 1);
}
static unsigned PsDexRating(PbrbPlay* p) {
    unsigned table = PsRom(p, PSR_DexRatingsTable), owned = PbrbGameDexCount(p->gameState.owned);
    for (unsigned i = 0; i < 16; ++i) { unsigned at = table + i * 3u; if (owned < PsRead(p, at))return PsPointer(p, at + 1, table); }
    return 0;
}
static void PsDexSound(PbrbPlay* p) {
    unsigned owned = PbrbGameDexCount(p->gameState.owned), at = PsRom(p, PSR_OwnedMonValues), i = 0;
    while (i < 6 && owned >= PsRead(p, at + i))++i;
    at = PsRom(p, PSR_PokedexRatingSfxPointers) + i * 2u;
    PbrbAudioPlaySfx(PBRB_AUDIO_RAW(PsRead(p, at + 1), PsRead(p, at)));
}
static void PsHallRecord(PbrbPlay* p) {
    PbrbGameState* s = &p->gameState; unsigned index = s->hallOfFameCount;
    if (s->hallOfFameTotal < 255)++s->hallOfFameTotal;
    if (index >= 50) { memmove(s->hallOfFame, s->hallOfFame + 1, 49u * sizeof(PbrbGameHallRecord)); index = 49; }
    else ++s->hallOfFameCount;
    PbrbGameHallRecord* r = s->hallOfFame + index; memset(r, 0, sizeof(*r)); r->count = s->partyCount;
    for (unsigned i = 0; i < r->count; ++i) {
        r->mons[i].species = s->party[i].species; r->mons[i].level = s->party[i].level;
        memcpy(r->mons[i].nickname, s->party[i].nickname, 11);
    }
}
static void PsHallPicture(PbrbPlay* p) {
    unsigned i = p->serviceData[0]; int trainer = i >= p->gameState.partyCount;
    unsigned species = trainer ? 0 : p->gameState.party[i].species;
    if (trainer)PbrbGbDecodePicture(p->gb, PplayData(p, PBRB_DATA_RedPicFront), p->picture, sizeof(p->picture));
    else PbrbGbDecodePokemon(p->gb, species, 0, p->picture, sizeof(p->picture));
    PplayBackPicture(p, species, trainer, p->backPicture); p->serviceTimer = 0;
    p->serviceStep = PSS_HOF_BACK; p->text.visible = 0;
}
static void PsHallTick(PbrbPlay* p) {
    unsigned i = p->serviceData[0]; int trainer = i >= p->gameState.partyCount;
    switch (p->serviceStep) {
    case PSS_HOF_START:if (++p->serviceTimer >= 100)PsHallPicture(p); break;
    case PSS_HOF_BACK:if (++p->serviceTimer >= 56) { p->serviceTimer = 0; p->serviceStep = PSS_HOF_FRONT; }break;
    case PSS_HOF_FRONT:if (++p->serviceTimer >= 40) {
        p->serviceTimer = 0;
        if (trainer) { p->serviceStep = PSS_HOF_PLAYER_STATS; PplayGameText(p, PBRB_GT_DexSeenOwnedText, 0, PPLAY_AFTER_NONE); }
        else { p->serviceStep = PSS_HOF_INFO; PbrbAudioPlayCry(p->gameState.party[i].species); }
    }break;
    case PSS_HOF_INFO:if (++p->serviceTimer >= 80) { p->serviceTimer = 0; p->serviceStep = PSS_HOF_TITLE; }break;
    case PSS_HOF_TITLE:if (++p->serviceTimer >= 180) { p->serviceTimer = 0; p->serviceStep = PSS_HOF_FADE; }break;
    case PSS_HOF_FADE:if (++p->serviceTimer >= 24) { ++p->serviceData[0]; PsHallPicture(p); }break;
    case PSS_HOF_PLAYER_STATS:if (++p->serviceTimer >= 120) {
        p->serviceTimer = 0;
        PsGameText(p, PBRB_GT_DexRatingText, PSS_HOF_PLAYER_SEEN);
    }break;
    case PSS_HOF_PLAYER_SEEN:if (++p->serviceTimer >= 120) {
        p->serviceTimer = 0;
        PsTextAt(p, PsDexRating(p), PSS_HOF_PLAYER_RATING);
    }break;
    case PSS_HOF_PLAYER_RATING:if (++p->serviceTimer >= 120) {
        p->text.visible = 0; p->serviceTimer = 0;
        p->serviceStep = PSS_HOF_PLAYER_END; PbrbAudioFadeMusic(PBRB_MUSIC_NONE, 10);
    }break;
    case PSS_HOF_PLAYER_END:if (++p->serviceTimer >= 24)PsDone(p, 1); break;
    }
}
static void PsHallRender(PbrbPlay* p) {
    PbrbGb* g = p->gb; unsigned i = p->serviceData[0]; int step = p->serviceStep;
    if (step == PSS_HOF_START) { PbrbGbClear(g, 0); return; }
    if (step == PSS_HOF_FADE || step == PSS_HOF_PLAYER_END) {
        static const unsigned palettes[3] = { 0x90,0x40,0 }; unsigned phase = p->serviceTimer / 8u; if (phase > 2)phase = 2;
        PbrbGbSetPalettes(g, palettes[phase], 0xd0, 0xe4);
    }
    PbrbGbClear(g, 0);
    if (step == PSS_HOF_BACK || step == PSS_HOF_FRONT) {
        unsigned scroll = step == PSS_HOF_BACK ? (192u + p->serviceTimer * 4u) & 255u : 160u - p->serviceTimer * 4u;
        int x = (int)((96u - scroll) & 255u), y = step == PSS_HOF_BACK ? 88 : 40;
        const unsigned char* picture = step == PSS_HOF_BACK ? p->backPicture : p->picture;
        PbrbGbBitmap(g, picture, 56, 56, x, y, 0, 0); PbrbGbBitmap(g, picture, 56, 56, x - 256, y, 0, 0); return;
    }
    PbrbGbBitmap(g, p->picture, 56, 56, 96, 40, 0, 0);
    if (i < p->gameState.partyCount) {
        const PbrbPlayMon* m = p->gameState.party + i; PbrbGbTextBox(g, 0, 16, 12, 11);
        PbrbGbCodes(g, m->nickname, 8, 32, 10); PbrbGbString(g, "LEVEL/", 16, 48);
        PbrbGbString(g, "TYPE1/", 16, 64); PbrbGbString(g, "TYPE2/", 16, 80);
        PplayHudTile(p, 0x6e, 56, 56); PplayNumber(g, m->level, 64, 56, m->level < 100 ? 2 : 3);
        unsigned table = PsRom(p, PSR_TypeNames);
        for (unsigned t = 0; t < 2; ++t) {
            if (t && m->types[0] == m->types[1])continue;
            unsigned at = PsPointer(p, table + m->types[t] * 2u, table); if (at)PbrbGbCodes(g, g->rom + at, 24, 72 + (int)t * 16, 9);
        }
        if (step == PSS_HOF_TITLE || step == PSS_HOF_FADE) {
            PbrbGbTextBox(g, 16, 104, 16, 5);
            PbrbGbCodes(g, g->rom + PsRom(p, PSR_HallOfFameText), 32, 120, 14);
        }
    }
    else {
        PbrbGbTextBox(g, 0, 32, 12, 8); PbrbGbTextBox(g, 40, 0, 11, 4);
        PbrbGbCodes(g, p->player, 56, 16, 7); PbrbGbString(g, "PLAY TIME", 8, 48);
        unsigned minutes = p->frames / 3600u; PplayNumber(g, minutes / 60u, 40, 56, 3);
        PplayHudTile(p, 0x6d, 64, 56); PsDigits(g, minutes % 60u, 72, 56, 2);
        PbrbGbString(g, "MONEY", 8, 72); PbrbGbGlyph(g, 0xf0, 24, 80); PsDigits(g, p->gameState.money, 32, 80, 6);
    }
}
static void PsHallViewPicture(PbrbPlay* p) {
    const PbrbGameHallRecord* r = p->gameState.hallOfFame + p->serviceSelection;
    const PbrbGameHallMon* m = r->mons + p->serviceData[0];
    PbrbGbDecodePokemon(p->gb, m->species, 0, p->picture, sizeof(p->picture));
    PbrbAudioPlayCry(m->species); p->serviceStep = PSS_HOF_VIEW; p->text.visible = 0;
}
static void PsHallViewRender(PbrbPlay* p) {
    const PbrbGameHallRecord* r = p->gameState.hallOfFame + p->serviceSelection;
    const PbrbGameHallMon* m = r->mons + p->serviceData[0]; PbrbGb* g = p->gb;
    PbrbGbClear(g, 0); PbrbGbBitmap(g, p->picture, 56, 56, 96, 40, 0, 0);
    PbrbGbTextBox(g, 0, 16, 12, 11); PbrbGbCodes(g, m->nickname, 8, 32, 10);
    PbrbGbString(g, "LEVEL/", 16, 48); PbrbGbString(g, "TYPE1/", 16, 64); PbrbGbString(g, "TYPE2/", 16, 80);
    PplayHudTile(p, 0x6e, 56, 56); PplayNumber(g, m->level, 64, 56, m->level < 100 ? 2 : 3);
    PbrbGameRom rom = PplayRom(p); PbrbGameBase base;
    if (PbrbGameBaseData(&rom, m->species, &base)) {
        unsigned table = PsRom(p, PSR_TypeNames);
        for (unsigned i = 0; i < 2; ++i) {
            if (i && base.types[0] == base.types[1])continue;
            unsigned at = PsPointer(p, table + base.types[i] * 2u, table); if (at)PbrbGbCodes(g, g->rom + at, 24, 72 + (int)i * 16, 9);
        }
    }
    PbrbGbTextBox(g, 0, 104, 20, 4); PbrbGbCodes(g, g->rom + PsRom(p, PSR_HallOfFameNoText), 8, 120, 18);
    unsigned total = p->gameState.hallOfFameTotal, count = p->gameState.hallOfFameCount;
    PplayNumber(g, (total > count ? total - count : 0) + (unsigned)p->serviceSelection + 1u, 128, 120, 3);
}
static void PsElevatorRender(PbrbPlay* p) {
    PbrbGb* g = p->gb; PbrbGameRom rom = PplayRom(p); unsigned count = p->serviceEvent.dataCount;
    unsigned top = p->serviceSelection > 2 ? (unsigned)p->serviceSelection - 2u : 0;
    PbrbGbTextBox(g, 32, 16, 16, 11);
    for (unsigned row = 0; row < 4 && top + row <= count; ++row) {
        int y = 32 + (int)row * 16;
        if (top + row == count) { PbrbGbString(g, "CANCEL", 48, y); break; }
        unsigned char name[16]; PbrbGameItemName(&rom, PsRead(p, p->serviceEvent.dataOffset + top + row), name);
        PbrbGbCodes(g, name, 48, y, 13);
    }
    PbrbGbGlyph(g, 0xed, 40, 32 + (p->serviceSelection - (int)top) * 16);
    if (top + 3u < count)PbrbGbGlyph(g, 0xee, 144, 88);
}
static int PsPrizeLoad(PbrbPlay* p) {
    unsigned window = (unsigned)p->serviceEvent.value; if (window > 2)return 0;
    unsigned table = p->serviceEvent.dataOffset ? p->serviceEvent.dataOffset : PsRom(p, PSR_PrizeDifferentMenuPtrs) + window * 4u;
    unsigned entries = PsPointer(p, table, table), prices = PsPointer(p, table + 2u, table);
    if (!entries || !prices || entries + 3u >= p->gb->bytes || prices + 6u >= p->gb->bytes)return 0;
    for (unsigned i = 0; i < 3; ++i) {
        p->serviceData[44 + i] = (unsigned char)PsRead(p, entries + i);
        p->serviceData[47 + i * 2] = (unsigned char)PsRead(p, prices + i * 2);
        p->serviceData[48 + i * 2] = (unsigned char)PsRead(p, prices + i * 2 + 1);
    }
    return 1;
}
static unsigned PsPrizePrice(PbrbPlay* p, unsigned i) {
    unsigned a = p->serviceData[47 + i * 2], b = p->serviceData[48 + i * 2];
    return (a >> 4) * 1000u + (a & 15u) * 100u + (b >> 4) * 10u + (b & 15u);
}
static unsigned PsPrizeLevel(PbrbPlay* p, unsigned species) {
    unsigned table = PsRom(p, PSR_PrizeMonLevelDictionary);
    for (unsigned i = 0; i < 6; ++i)if (PsRead(p, table + i * 2u) == species)return PsRead(p, table + i * 2u + 1u);
    return 0;
}
static void PsPrizeRender(PbrbPlay* p) {
    PbrbGb* g = p->gb; PbrbGameRom rom = PplayRom(p); PbrbGbTextBox(g, 0, 16, 18, 10); PsCoins(p);
    for (unsigned i = 0; i < 3; ++i) {
        unsigned char name[16]; unsigned id = p->serviceData[44 + i];
        if (p->serviceEvent.value == 2)PbrbGameItemName(&rom, id, name); else PbrbGameMonName(&rom, id, name);
        PbrbGbCodes(g, name, 16, 32 + (int)i * 16, 13); PsDigits(g, PsPrizePrice(p, i), 104, 40 + (int)i * 16, 4);
    }
    PbrbGbCodes(g, g->rom + PsRom(p, PSR_NoThanksText), 16, 80, 12);
    if (p->serviceStep == PSS_PRIZE_LIST)PbrbGbGlyph(g, 0xed, 8, 32 + p->serviceSelection * 16);
}
static void PsSlotsBegin(PbrbPlay* p);
static void PsSlotsTick(PbrbPlay* p, unsigned keys);
static void PsSlotsRender(PbrbPlay* p);
static void PsVendingWords(PbrbPlay* p, unsigned label, int x, int y) {
    unsigned at = PsRom(p, label); int left = x;
    for (unsigned i = 0; i < 96 && at + i < p->gb->bytes; ++i) {
        unsigned c = PsRead(p, at + i);
        if (c == 0x50)break; if (c == 0x4e) { x = left; y += 16; continue; }
        PbrbGbGlyph(p->gb, (unsigned char)c, x, y); x += 8;
    }
}
static void PsVendingRender(PbrbPlay* p) {
    PsMoney(p); if (!p->serviceData[54])return;
    PbrbGbTextBox(p->gb, 0, 24, 14, 10);
    PsVendingWords(p, PSR_DrinkText, 16, 40); PsVendingWords(p, PSR_DrinkPriceText, 72, 48);
    if (p->serviceStep == PSS_VENDING_MENU && !p->text.active)
        PbrbGbGlyph(p->gb, 0xed, 8, 40 + p->serviceSelection * 16);
}
static int PbrbPlayServiceBegin(PbrbPlay* p, const PbrbWorldEvent* event) {
    int kind = event->kind;
    if (kind<PBRB_WORLD_GIVE_MON || kind>PBRB_WORLD_ITEM_CHOICE)return 0;
    p->serviceEvent = *event; p->serviceStep = PSS_DONE; p->serviceSelection = 0;
    p->serviceValue = 0; p->serviceCost = 0; p->serviceTimer = 0; memset(p->serviceData, 0, 64);
    p->serviceReturn = (event->flags & 0x80000000u) ? PBRB_OVERLAY_PC_MAIN : PBRB_OVERLAY_NONE;
    p->overlay = PBRB_OVERLAY_SERVICE; PbrbGameState* s = &p->gameState;
    switch (kind) {
    case PBRB_WORLD_GIVE_MON:
        PsGiveMon(p, event->species, event->level, 0);
        if (p->serviceValue && event->textOffset)PsTextAt(p, event->textOffset, PSS_GIFT_TEXT); return 1;
    case PBRB_WORLD_TAKE_MONEY: { int ok = s->money >= event->quantity; if (ok)s->money -= event->quantity; PsDone(p, ok); return 1; }
    case PBRB_WORLD_GIVE_COINS:
        /* Original NPC coin gifts use Has9990Coins; one gentleman tests only
           equality, so his bug is retained when bit25 is selected. */
        if ((event->flags & 0x04000000u) && ((event->flags & 0x02000000u) ? s->coins == 9990 : s->coins >= 9990)) {
            PsDone(p, 0); return 1;
        }
        s->coins += event->quantity; if (s->coins > 9999)s->coins = 9999;
        PsDone(p, (event->flags & 0x01000000u) && s->coins == 9999 ? 2 : 1); return 1;
    case PBRB_WORLD_TAKE_COINS: { int ok = s->coins >= event->quantity; if (ok)s->coins -= event->quantity; PsDone(p, ok); return 1; }
    case PBRB_WORLD_TRADE:if (!PsTradeBegin(p))PsDone(p, 0); return 1;
    case PBRB_WORLD_DAYCARE:PsDaycareBegin(p); return 1;
    case PBRB_WORLD_SAFARI:s->safariBalls = event->value ? 30u : 0u; s->safariSteps = event->value ? 500u : 0u; PsDone(p, 1); return 1;
    case PBRB_WORLD_DEX_RATING:p->serviceValue = 1; PsText(p, PSR_DexCompletionText, PSS_DEX_RATING); return 1;
    case PBRB_WORLD_HALL_OF_FAME:
        if ((event->flags & 0x80000000u) && event->value == 1) {
            if (!s->hallOfFameCount) { PsDone(p, 0); return 1; }
            PsGameText(p, PBRB_GT_AccessedHoFPCText, PSS_HOF_VIEW_START); return 1;
        }
        if (!s->partyCount) { PsDone(p, 0); return 1; }PsHallRecord(p); p->serviceStep = PSS_HOF_START;
        p->text.visible = 0; PbrbAudioFadeMusic(PBRB_MUSIC_HALL_OF_FAME, 10); return 1;
    case PBRB_WORLD_PRIZE_MENU:
        if (!PbrbGameCountItem(s->bag, s->bagCount, 0x45)) { PsGameText(p, PBRB_GT_RequireCoinCaseText, PSS_DONE, 1); return 1; }
        if (!PsPrizeLoad(p)) { PsDone(p, 0); return 1; }
        PsGameText(p, PBRB_GT_ExchangeCoinsForPrizesText, PSS_PRIZE_INTRO); return 1;
    case PBRB_WORLD_GAME_CORNER:
        if (event->value == 1)PsText(p, PSR_GameCornerClerk1Text_DoYouNeedSomeGameCoins, PSS_VENDOR_ASK);
        else PsSlotsBegin(p); return 1;
    case PBRB_WORLD_ELEVATOR:
        if (!event->dataCount || event->dataCount > 11 || event->dataOffset + event->dataCount > p->gb->bytes) { PsDone(p, 0); return 1; }
        PsGameText(p, PBRB_GT_WhichFloorText, PSS_ELEVATOR); return 1;
    case PBRB_WORLD_VENDING:
        PsText(p, PSR_VendingMachineText1, PSS_VENDING_MENU); return 1;
    case PBRB_WORLD_ITEM_CHOICE:
        if (event->dataCount > 3 || event->dataOffset + event->dataCount > p->gb->bytes) { PsDone(p, 0); return 1; }
        for (unsigned i = 0; i < event->dataCount; ++i) {
            unsigned item = PsRead(p, event->dataOffset + i);
            if (PbrbGameCountItem(s->bag, s->bagCount, item))p->serviceData[p->serviceValue++] = (unsigned char)item;
        }
        if (!p->serviceValue) { PsDone(p, 0); return 1; }
        p->serviceStep = PSS_ITEM_CHOICE; return 1;
    default:PsDone(p, 0); return 1;
    }
}
static int PbrbPlayServiceTick(PbrbPlay* p, unsigned keys) {
    if (p->overlay != PBRB_OVERLAY_SERVICE)return 0;
    if (p->text.active)return 1;
    PbrbGameState* s = &p->gameState; int answer = 0;
    if (p->serviceStep >= PSS_HOF_START && p->serviceStep <= PSS_HOF_PLAYER_END) { PsHallTick(p); return 1; }
    if (p->serviceStep >= PSS_SLOTS_ASK) { PsSlotsTick(p, keys); return 1; }
    switch (p->serviceStep) {
    case PSS_DONE:PsDone(p, p->serviceValue != 0); break;
    case PSS_GIFT_TEXT:PsGiftAfterText(p); break;
    case PSS_GIFT_ASK:
        answer = PsChoice(p, keys); if (answer < 0)PsDone(p, 1);
        else if (answer > 0) { p->serviceStep = PSS_GIFT_NAMING; PplayStartNaming(p, 4); }break;
    case PSS_DAYCARE_ASK:
        answer = PsChoice(p, keys); if (answer < 0)PsText(p, PSR_DaycareGentlemanText_ComeAgainText, PSS_DONE);
        else if (answer > 0) {
            if (s->partyCount <= 1)PsText(p, PSR_DaycareGentlemanText_OnlyHaveOneMonText, PSS_DONE);
            else { p->serviceSelection = 0; PsText(p, PSR_DaycareGentlemanText_WhichMonText, PSS_DAYCARE_PARTY); }
        }break;
    case PSS_DAYCARE_PARTY:
        answer = PsPartyChoice(p, keys); if (answer < 0)PsText(p, PSR_DaycareGentlemanText_AllRightThenText, PSS_DONE);
        else if (answer > 0) {
            PbrbGameRom rom = PplayRom(p); PbrbPlayMon* m = s->party + answer - 1; int hm = 0;
            for (unsigned i = 0; i < 4; ++i)if (PplayIsHM(&rom, m->moves[i]))hm = 1;
            if (hm)PsText(p, PSR_DaycareGentlemanText_CantAcceptMonWithHMText, PSS_DONE);
            else {
                PplayCopyName(p->nameBuffer, m->nickname, 10); p->serviceData[0] = (unsigned char)(answer - 1);
                PsText(p, PSR_DaycareGentlemanText_WillLookAfterMonText, PSS_DAYCARE_DEPOSIT);
            }
        }break;
    case PSS_DAYCARE_DEPOSIT:
        s->daycare = s->party[p->serviceData[0]]; s->daycareOccupied = 1; s->daycareLevel = s->daycare.level; s->daycareSteps = 0;
        PsRemoveParty(s, p->serviceData[0]); PplaySyncAliases(p); p->serviceValue = 1; PbrbAudioPlayCry(s->daycare.species);
        PsText(p, PSR_DaycareGentlemanText_ComeSeeMeInAWhileText, PSS_DONE); break;
    case PSS_DAYCARE_GROWTH:
        if (s->partyCount >= 6) { p->serviceValue = 0; PsText(p, PSR_DaycareGentlemanText_NoRoomForMonText, PSS_DONE); }
        else { p->serviceSelection = 0; PsText(p, PSR_DaycareGentlemanText_OweMoneyText, PSS_DAYCARE_PAY); }break;
    case PSS_DAYCARE_PAY:
        answer = PsChoice(p, keys); if (answer < 0) { p->serviceValue = 0; PsText(p, PSR_DaycareGentlemanText_AllRightThenText, PSS_DONE); }
        else if (answer > 0) {
            if (s->money < (unsigned)p->serviceCost) { p->serviceValue = 0; PsText(p, PSR_DaycareGentlemanText_NotEnoughMoneyText, PSS_DONE); }
            else PsDaycareWithdraw(p);
        }break;
    case PSS_DAYCARE_RETURN:PbrbAudioPlayCry(s->daycare.species); PsText(p, PSR_DaycareGentlemanText_GotMonBackText, PSS_DONE); break;
    case PSS_TRADE_ASK:
        answer = PsChoice(p, keys); if (answer < 0)PsTextAt(p, PsTradeText(p, 1), PSS_DONE);
        else if (answer > 0) { p->serviceSelection = 0; p->text.visible = 0; p->serviceStep = PSS_TRADE_PARTY; }break;
    case PSS_TRADE_PARTY:
        answer = PsPartyChoice(p, keys); if (answer < 0)PsTextAt(p, PsTradeText(p, 1), PSS_DONE);
        else if (answer > 0) {
            if (s->party[answer - 1].species != p->serviceEvent.itemId)PsTextAt(p, PsTradeText(p, 2), PSS_DONE);
            else {
                p->serviceData[61] = (unsigned char)(answer - 1); p->serviceCost = (int)(PplayRandom(p) & 65535u);
                PsGameText(p, PBRB_GT_ConnectCableText, PSS_TRADE_START, 1);
            }
        }break;
    case PSS_TRADE_START:PsTradeStart(p); break;
    case PSS_TRADE_ANIMATE:PsTradeTick(p); break;
    case PSS_TRADE_COMMIT:
        if (!PsTradeCommit(p)) { PsDone(p, 0); break; }
        PsGameText(p, PBRB_GT_TradedForText, PSS_TRADE_THANKS, 1); PbrbAudioPlaySfx(PBRB_SFX_GET_KEY_ITEM); break;
    case PSS_TRADE_THANKS:PsTextAt(p, PsTradeText(p, 3), PSS_DONE); break;
    case PSS_DEX_RATING:PsTextAt(p, PsDexRating(p), PSS_DEX_SOUND); break;
    case PSS_DEX_SOUND:PsDexSound(p); p->serviceStep = PSS_DEX_WAIT; break;
    case PSS_DEX_WAIT:if ((keys & (PBRB_PAD_A | PBRB_PAD_B)) && !PbrbAudioEffectsBusy())PsDone(p, 1); break;
    case PSS_HOF_VIEW_START:PsHallViewPicture(p); break;
    case PSS_HOF_VIEW:
        if (keys & PBRB_PAD_B) { PsDone(p, 0); break; }
        if (keys & PBRB_PAD_A) {
            ++p->serviceData[0];
            if (p->serviceData[0] >= s->hallOfFame[p->serviceSelection].count) { p->serviceData[0] = 0; ++p->serviceSelection; }
            if ((unsigned)p->serviceSelection >= s->hallOfFameCount)PsDone(p, 0); else PsHallViewPicture(p);
        }break;
    case PSS_ELEVATOR:
        if (keys & PBRB_PAD_B) { PsDone(p, 0); break; }
        if ((keys & PBRB_PAD_UP) && p->serviceSelection > 0)--p->serviceSelection;
        if ((keys & PBRB_PAD_DOWN) && (unsigned)p->serviceSelection < p->serviceEvent.dataCount)++p->serviceSelection;
        if (keys & PBRB_PAD_A)PsDone(p, (unsigned)p->serviceSelection == p->serviceEvent.dataCount ? 0 : p->serviceSelection + 1);
        break;
    case PSS_VENDING_MENU:
        p->serviceData[54] = 1;
        if (keys & PBRB_PAD_UP)p->serviceSelection = (p->serviceSelection + 3) % 4;
        if (keys & PBRB_PAD_DOWN)p->serviceSelection = (p->serviceSelection + 1) % 4;
        if ((keys & PBRB_PAD_B) || ((keys & PBRB_PAD_A) && p->serviceSelection == 3))
            PsText(p, PSR_VendingMachineText7, PSS_DONE);
        else if (keys & PBRB_PAD_A) {
            /* The original always tests 200, including the dearer drinks. */
            if (s->money < 200) { PsText(p, PSR_VendingMachineText4, PSS_DONE); break; }
            unsigned at = PsRom(p, PSR_VendingPrices) + (unsigned)p->serviceSelection * 4u;
            unsigned item = PsRead(p, at), price = 0;
            for (unsigned i = 1; i <= 3; ++i) { unsigned b = PsRead(p, at + i); price = price * 100u + (b >> 4) * 10u + (b & 15u); }
            if (!PbrbGameAddItem(s->bag, &s->bagCount, 20, item, 1)) {
                PsText(p, PSR_VendingMachineText6, PSS_DONE); break;
            }
            PplayMenuName(p, item); p->serviceCost = (int)price; p->serviceTimer = 0;
            p->serviceStep = PSS_VENDING_DELIVERY;
        }break;
    case PSS_VENDING_DELIVERY:
        if ((++p->serviceTimer & 1u) == 0)PbrbAudioPlaySfx(PBRB_SFX_PUSH_BOULDER);
        if (p->serviceTimer >= 120)PsText(p, PSR_VendingMachineText5, PSS_VENDING_PAID);
        break;
    case PSS_VENDING_PAID:
        /* SubBCD saturates a borrowed result at zero, after the received text. */
        s->money = s->money >= (unsigned)p->serviceCost ? s->money - (unsigned)p->serviceCost : 0;
        PsDone(p, 1); break;
    case PSS_ITEM_CHOICE:
        if (keys & PBRB_PAD_B) { PsDone(p, 0); break; }
        if (keys & PBRB_PAD_UP)p->serviceSelection = (p->serviceSelection + p->serviceValue - 1) % p->serviceValue;
        if (keys & PBRB_PAD_DOWN)p->serviceSelection = (p->serviceSelection + 1) % p->serviceValue;
        if (keys & PBRB_PAD_A)PsDone(p, p->serviceData[p->serviceSelection]); break;
    case PSS_PRIZE_INTRO:p->serviceSelection = 0; PsGameText(p, PBRB_GT_WhichPrizeText, PSS_PRIZE_LIST); break;
    case PSS_PRIZE_LIST:
        if (keys & PBRB_PAD_UP)p->serviceSelection = (p->serviceSelection + 3) % 4;
        if (keys & PBRB_PAD_DOWN)p->serviceSelection = (p->serviceSelection + 1) % 4;
        if (keys & PBRB_PAD_B) { PsDone(p, 0); break; }
        if (keys & PBRB_PAD_A) {
            if (p->serviceSelection == 3) { PsDone(p, 0); break; }
            PbrbGameRom rom = PplayRom(p); p->serviceData[43] = (unsigned char)p->serviceSelection;
            unsigned id = p->serviceData[44 + p->serviceSelection]; p->serviceCost = (int)PsPrizePrice(p, (unsigned)p->serviceSelection);
            if (p->serviceEvent.value == 2)PbrbGameItemName(&rom, id, p->nameBuffer); else PbrbGameMonName(&rom, id, p->nameBuffer);
            p->serviceSelection = 0; PsGameText(p, PBRB_GT_SoYouWantPrizeText, PSS_PRIZE_ASK);
        }break;
    case PSS_PRIZE_ASK:
        answer = PsChoice(p, keys); if (answer < 0)PsGameText(p, PBRB_GT_OhFineThenText, PSS_DONE, 1);
        else if (answer > 0) {
            if (s->coins < (unsigned)p->serviceCost) { PsGameText(p, PBRB_GT_SorryNeedMoreCoinsText, PSS_DONE, 1); break; }
            unsigned id = p->serviceData[44 + p->serviceData[43]];
            if (p->serviceEvent.value == 2) {
                if (PbrbGameAddItem(s->bag, &s->bagCount, 20, id, 1)) {
                    s->coins -= (unsigned)p->serviceCost; PsDone(p, 1);
                }
                else PsGameText(p, PBRB_GT_OopsYouDontHaveEnoughRoomText, PSS_DONE, 1);
            }
            else if (PsGiveMon(p, id, PsPrizeLevel(p, id), 1))s->coins -= (unsigned)p->serviceCost;
        }break;
    case PSS_VENDOR_ASK:
        answer = PsChoice(p, keys); if (answer < 0)PsText(p, PSR_GameCornerClerk1Text_PleaseComePlaySometime, PSS_DONE);
        else if (answer > 0) {
            if (!PbrbGameCountItem(s->bag, s->bagCount, 0x45))PsText(p, PSR_GameCornerClerk1Text_DontHaveCoinCase, PSS_DONE);
            else if (s->coins >= 9990)PsText(p, PSR_GameCornerClerk1Text_CoinCaseIsFull, PSS_DONE);
            else if (s->money < 1000)PsText(p, PSR_GameCornerClerk1Text_CantAffordTheCoins, PSS_DONE);
            else {
                s->money -= 1000; s->coins += 50; if (s->coins > 9999)s->coins = 9999; p->serviceValue = 1; PplaySyncAliases(p);
                PsText(p, PSR_GameCornerClerk1Text_ThanksHereAre50Coins, PSS_DONE);
            }
        }break;
    default:break;
    }
    return 1;
}
static int PbrbPlayServiceRender(PbrbPlay* p) {
    if (p->overlay != PBRB_OVERLAY_SERVICE)return 0;
    int step = p->serviceStep;
    if (step == PSS_TRADE_ANIMATE) { PsTradeRender(p); return 1; }
    if (step >= PSS_HOF_START && step <= PSS_HOF_PLAYER_END) { PsHallRender(p); return 1; }
    if (step == PSS_HOF_VIEW) { PsHallViewRender(p); return 1; }
    if (step == PSS_ELEVATOR) { PsElevatorRender(p); return 1; }
    if (step == PSS_ITEM_CHOICE) {
        PbrbGameRom rom = PplayRom(p);
        PbrbGbTextBox(p->gb, 0, 0, 15, p->serviceValue * 2 + 2);
        for (int i = 0; i < p->serviceValue; ++i) {
            unsigned char name[16]; PbrbGameItemName(&rom, p->serviceData[i], name);
            PbrbGbCodes(p->gb, name, 16, 16 + i * 16, 12);
        }
        PbrbGbGlyph(p->gb, 0xed, 8, 16 + p->serviceSelection * 16); return 1;
    }
    if (p->serviceEvent.kind == PBRB_WORLD_VENDING) { PsVendingRender(p); return 1; }
    if (step >= PSS_SLOTS_ASK) { PsSlotsRender(p); return 1; }
    if (step == PSS_DAYCARE_PARTY || step == PSS_TRADE_PARTY) { PsParty(p); return 1; }
    if (p->serviceEvent.kind == PBRB_WORLD_PRIZE_MENU && p->serviceData[44])PsPrizeRender(p);
    if (p->serviceEvent.kind == PBRB_WORLD_GAME_CORNER && p->serviceEvent.value == 1)PsCoins(p);
    if (step == PSS_DAYCARE_PAY)PsMoney(p);
    if (!p->text.active && (step == PSS_DAYCARE_ASK || step == PSS_DAYCARE_PAY || step == PSS_TRADE_ASK ||
        step == PSS_GIFT_ASK || step == PSS_PRIZE_ASK || step == PSS_VENDOR_ASK))PsYesNo(p);
    return 1;
}
/* Slot implementation follows the original reel offsets and stop logic. */
static void PsSlotsBegin(PbrbPlay* p) {
    if (p->serviceEvent.value == 4)PsGameText(p, PBRB_GT_GameCornerOutOfOrderText, PSS_DONE);
    else if (!PbrbGameCountItem(p->gameState.bag, p->gameState.bagCount, 0x45))PsGameText(p, PBRB_GT_GameCornerCoinCaseText, PSS_DONE);
    else if (!p->gameState.coins)PsGameText(p, PBRB_GT_GameCornerNoCoinsText, PSS_DONE);
    else PsGameText(p, PBRB_GT_PlaySlotMachineText, PSS_SLOTS_ASK);
}
/* serviceData for slots: offsets[0..2], slip[3..5], stop6, flags7,
   matchCounter8, reroll9, bet10, symbol11, initialized13, drawn[14..16],
   flashCount18, palettePhase19, frameDivider22, pendingA23, wonText24. */
static unsigned PsSlotByte(PbrbPlay* p) { return PplayRandom(p) & 255u; }
static void PsSlotAnimate(PbrbPlay* p, unsigned wheel) {
    unsigned char* d = p->serviceData; d[14 + wheel] = d[wheel];
    d[wheel] = (unsigned char)((d[wheel] + 1u) % 30u);
}
static unsigned PsSlotTile(PbrbPlay* p, unsigned wheel, unsigned row) {
    return PsRead(p, PsRom(p, PSR_SlotMachineWheel1 + wheel) + p->serviceData[wheel] + row * 2u);
}
static void PsSlotFlags(PbrbPlay* p) {
    unsigned char* d = p->serviceData; if (d[7] & 128u)return;
    if (d[8]) { d[7] |= 64; return; }
    unsigned r = PsSlotByte(p); if (!r) { d[8] = 60; return; }
    if (r > (p->serviceEvent.value == 3 ? 250u : 253u))d[7] |= 128;
    else if (r > 210)d[7] |= 64; else d[7] = 0;
}
static unsigned PsSlotMatch12(PbrbPlay* p, int* matched) {
    static const unsigned rows[5][2] = { {0,0},{0,1},{1,1},{2,1},{2,2} };
    for (unsigned i = 0; i < 5; ++i)if (PsSlotTile(p, 0, rows[i][0]) == PsSlotTile(p, 1, rows[i][1])) {
        *matched = 1; return PsSlotTile(p, 1, rows[i][1]);
    }
    *matched = 0; return PsSlotTile(p, 1, 0);
}
static int PsSlotStop(PbrbPlay* p, unsigned wheel) {
    unsigned char* d = p->serviceData;
    if (d[6] > wheel && (d[wheel] & 1u)) {
        if (wheel == 2)return 1;
        if (!d[3 + wheel])return 0;
        --d[3 + wheel]; int stop = 0;
        if (!wheel) {
            /* Original 7/bar branch compares tiles with2; none are smaller.
               Keep its fall-through rather than changing the intended odds. */
            if (!(d[7] & 128u) && PsSlotTile(p, 0, 1) != 10u)stop = 1;
        }
        else {
            int matched = 0; unsigned tile = PsSlotMatch12(p, &matched);
            stop = (d[7] & 128u) ? tile < 7u : matched;
        }
        if (stop) { d[3 + wheel] = 0; return 0; }
    }
    PsSlotAnimate(p, wheel); return 0;
}
static unsigned PsSlotMatch(PbrbPlay* p) {
    static const unsigned rows[5][3] = { {0,1,2},{2,1,0},{2,2,2},{0,0,0},{1,1,1} };
    unsigned start = p->serviceData[10] == 1 ? 4u : p->serviceData[10] == 2 ? 2u : 0u;
    for (unsigned i = start; i < 5; ++i) {
        unsigned a = PsSlotTile(p, 0, rows[i][0]);
        if (a == PsSlotTile(p, 1, rows[i][1]) && a == PsSlotTile(p, 2, rows[i][2]))return a;
    }
    return 0;
}
static void PsSlotAgain(PbrbPlay* p) {
    p->serviceData[24] = 0; p->serviceData[19] = 0; p->serviceSelection = 0;
    if (!p->gameState.coins) { p->serviceTimer = 0; PsGameText(p, PBRB_GT_OutOfCoinsSlotMachineText, PSS_SLOTS_EMPTY); }
    else PsGameText(p, PBRB_GT_OneMoreGoSlotMachineText, PSS_SLOTS_AGAIN);
}
static void PsSlotReward(PbrbPlay* p, unsigned tile) {
    unsigned char* d = p->serviceData; d[11] = (unsigned char)(tile - 2u); d[24] = 1;
    unsigned table = PsRom(p, PSR_SlotRewardPointers);
    unsigned text = PsPointer(p, table + d[11] + 2u, table); memset(p->stringBuffer, 0x50, sizeof(p->stringBuffer));
    if (text)memcpy(p->stringBuffer, p->gb->rom + text, 4);
    p->serviceCost = tile == 2 ? 300 : tile == 6 ? 100 : tile == 10 ? 8 : 15;
    d[18] = (unsigned char)(tile == 2 ? 20 : tile == 6 ? 8 : tile == 10 ? 2 : 4);
    if (tile == 2) {
        if (PsSlotByte(p) >= 128u)d[7] = 0; d[8] = 0;
        PsGameText(p, PBRB_GT_YeahText, PSS_SLOTS_WIN); PbrbAudioPlaySfx(PBRB_SFX_GET_ITEM2);
    }
    else {
        if (tile == 6) { d[7] = 0; PbrbAudioPlaySfx(PBRB_SFX_GET_KEY_ITEM); }
        else if (d[8])--d[8]; p->serviceStep = PSS_SLOTS_WIN;
    }
    p->serviceTimer = 0; d[19] = 0;
}
static void PsSlotsTick(PbrbPlay* p, unsigned keys) {
    unsigned char* d = p->serviceData; int answer = 0;
    switch (p->serviceStep) {
    case PSS_SLOTS_ASK:
        answer = PsChoice(p, keys); if (answer < 0) { PsDone(p, 0); break; }
        if (answer > 0) {
            d[13] = 1; for (unsigned i = 0; i < 3; ++i) { d[i] = 28; PsSlotAnimate(p, i); }
            p->serviceSelection = 0; p->serviceCost = 0; PsGameText(p, PBRB_GT_BetHowManySlotMachineText, PSS_SLOTS_BET);
        }break;
    case PSS_SLOTS_BET_TEXT:
        p->serviceSelection = 0; p->serviceCost = 0; d[10] = 0; d[24] = 0;
        PsGameText(p, PBRB_GT_BetHowManySlotMachineText, PSS_SLOTS_BET); break;
    case PSS_SLOTS_BET:
        if (keys & PBRB_PAD_B) { PsDone(p, p->serviceValue); break; }
        if (keys & PBRB_PAD_UP)p->serviceSelection = (p->serviceSelection + 2) % 3;
        if (keys & PBRB_PAD_DOWN)p->serviceSelection = (p->serviceSelection + 1) % 3;
        if (keys & PBRB_PAD_A) {
            unsigned bet = 3u - (unsigned)p->serviceSelection;
            if (p->gameState.coins < bet) { p->serviceSelection = 0; PsGameText(p, PBRB_GT_NotEnoughCoinsSlotMachineText, PSS_SLOTS_BET); break; }
            d[10] = (unsigned char)bet; p->gameState.coins -= bet; p->serviceValue = 1; PsSlotFlags(p);
            d[3] = d[4] = d[5] = 4; d[6] = 0; d[22] = 0; d[23] = 0; p->serviceTimer = 0;
            PbrbAudioPlaySfx(PBRB_SFX_SLOTS_SPIN); PsGameText(p, PBRB_GT_StartSlotMachineText, PSS_SLOTS_START);
        }break;
    case PSS_SLOTS_START:
        if ((p->serviceTimer++ & 1u) == 0)for (unsigned i = 0; i < 3; ++i)PsSlotAnimate(p, i);
        if (p->serviceTimer >= 40) { p->serviceTimer = 0; p->serviceStep = PSS_SLOTS_SPIN; }break;
    case PSS_SLOTS_SPIN:
        if (keys & PBRB_PAD_A)d[23] = 1;
        if (++d[22] < 3)break; d[22] = 0;
        if (d[23]) {
            d[23] = 0; unsigned stop = d[6];
            if (!stop || (stop < 3 && !d[stop + 2])) { ++d[6]; PbrbAudioPlaySfx(PBRB_SFX_SLOTS_STOP); }
        }
        PsSlotStop(p, 0); PsSlotStop(p, 1);
        if (PsSlotStop(p, 2)) { p->serviceTimer = 0; p->serviceStep = PSS_SLOTS_MATCH; }break;
    case PSS_SLOTS_MATCH: {
        if (p->serviceTimer) { PsSlotAnimate(p, 2); --p->serviceTimer; break; }
        unsigned tile = PsSlotMatch(p); int allowed = (d[7] & 192u) != 0;
        if (tile && allowed && ((d[7] & 128u) || tile >= 7u)) { PsSlotReward(p, tile); break; }
        if (tile) { p->serviceTimer = 2; break; }
        if (allowed) { d[9] = (unsigned char)(d[9] - 1u); if (d[9]) { p->serviceTimer = 2; break; } }
        PsGameText(p, PBRB_GT_NotThisTimeText, PSS_SLOTS_AGAIN_TEXT); break;
    }
    case PSS_SLOTS_WIN:p->serviceTimer = 0; p->serviceStep = PSS_SLOTS_FLASH; break;
    case PSS_SLOTS_FLASH:
        if ((p->serviceTimer++ % 5u) == 0)d[19] ^= 1;
        if (p->serviceTimer >= d[18] * 5u) {
            d[19] = 0; p->serviceTimer = 0;
            PsGameText(p, PBRB_GT_LinedUpText, PSS_SLOTS_PAY, 1);
            /* The text ASM advances four columns past the winning symbol. */
            if (p->text.count + 4u < PBRB_PLAY_TEXT_MAX) {
                memmove(p->text.tokens + 4, p->text.tokens, p->text.count);
                memset(p->text.tokens, 0x7f, 4); p->text.count += 4;
            }
        }break;
    case PSS_SLOTS_PAY:
        if (p->serviceTimer) { --p->serviceTimer; break; }
        if (p->serviceCost > 0) {
            --p->serviceCost; if (p->gameState.coins < 9999)++p->gameState.coins;
            PbrbAudioPlaySfx(PBRB_SFX_SLOTS_REWARD); if (++d[18] >= 5) { d[18] = 0; d[19] ^= 1; }
            p->serviceTimer = d[11] < 7u ? 3u : 7u;
        }
        else PsSlotAgain(p); break;
    case PSS_SLOTS_AGAIN_TEXT:PsSlotAgain(p); break;
    case PSS_SLOTS_AGAIN:
        answer = PsChoice(p, keys); if (answer < 0)PsDone(p, 1);
        else if (answer > 0)p->serviceStep = PSS_SLOTS_BET_TEXT; break;
    case PSS_SLOTS_EMPTY:if (++p->serviceTimer >= 60)PsDone(p, 1); break;
    default:break;
    }
}
static void PsSlotBackgroundTile(PbrbPlay* p, unsigned tile, int x, int y) {
    if (tile < 0x25u)PbrbGbTile(p->gb, PsRom(p, PSR_SlotMachineTiles1) + tile * 16u, 2, x, y);
    else if (tile < 0x41u)PbrbGbTile(p->gb, PsRom(p, PSR_SlotMachineTiles2) + (tile - 0x25u) * 16u, 2, x, y);
    else PbrbGbGlyph(p->gb, (unsigned char)tile, x, y);
}
static void PsSlotsRender(PbrbPlay* p) {
    unsigned char* d = p->serviceData; PbrbGb* g = p->gb;
    if (!d[13]) { if (!p->text.active)PsYesNo(p); return; }
    unsigned bg = (p->serviceStep == PSS_SLOTS_FLASH && d[19]) ? 0xa4u : 0xe4u;
    unsigned obj = (p->serviceStep == PSS_SLOTS_PAY && d[19]) ? 0xa4u : 0xe4u;
    PbrbGbSetPalettes(g, bg, obj, 0xe4); PbrbGbClear(g, 0);
    unsigned map = PsRom(p, PSR_SlotMachineMap);
    for (unsigned y = 0; y < 12; ++y)for (unsigned x = 0; x < 20; ++x)
        PsSlotBackgroundTile(p, PsRead(p, map + y * 20u + x), (int)x * 8, (int)y * 8);
    for (unsigned line = 0; line < 5; ++line) {
        unsigned need = (line == 2) ? 1u : (line == 1 || line == 3) ? 2u : 3u;
        if (d[10] < need)continue;
        for (unsigned side = 0; side < 2; ++side)for (unsigned row = 0; row < 2; ++row)
            PsSlotBackgroundTile(p, 0x14u + row, 24 + (int)side * 104, 16 + (int)line * 16 + (int)row * 8);
    }
    for (unsigned wheel = 0; wheel < 3; ++wheel) {
        unsigned table = PsRom(p, PSR_SlotMachineWheel1 + wheel);
        for (unsigned row = 0; row < 6; ++row) {
            unsigned tile = PsRead(p, table + d[14 + wheel] + row);
            for (unsigned column = 0; column < 2; ++column)PbrbGbSpriteTile(g,
                PsRom(p, PSR_SlotMachineTiles2) + (tile + column) * 16u,
                40 + (int)wheel * 32 + (int)column * 8, 72 - (int)row * 8, obj, 0x80);
        }
    }
    PplayNumber(g, p->gameState.coins, 40, 8, 4); PsDigits(g, (unsigned)p->serviceCost, 88, 8, 4);
}
static void PbrbPlayServiceForeground(PbrbPlay* p) {
    if (p->overlay != PBRB_OVERLAY_SERVICE)return;
    unsigned char* d = p->serviceData;
    if (p->serviceStep == PSS_TRADE_ANIMATE) {
        if (PsTradeSliding(p))PsTradeRender(p);
        if (d[44]) {
            if (d[44] == 172) {
                for (unsigned tile = 0; tile < 360; ++tile)if (p->battleAnim.displayedMap[tile] == 65535u)
                    for (unsigned y = 0; y < 8; ++y)for (unsigned x = 0; x < 8; ++x)
                        PbrbGbPixel(p->gb, (int)(tile % 20u * 8u + x), (int)(tile / 20u * 8u + y), 0, 0);
                PbrbBattleAnimRenderSprites(&p->battleAnim, p->gb);
            }
            else PbrbBattleAnimRender(&p->battleAnim, p->gb);
        }
        else if (p->serviceSelection == PST_CABLE_OUT && p->battleAnim.move == 170)
            PbrbBattleAnimRenderSprites(&p->battleAnim, p->gb);
        return;
    }
    if (p->serviceEvent.kind == PBRB_WORLD_GAME_CORNER && d[13]) {
        if (!p->text.active && p->serviceStep == PSS_SLOTS_BET) {
            PbrbGbTextBox(p->gb, 112, 88, 6, 7);
            for (unsigned i = 0; i < 3; ++i) {
                PbrbGbGlyph(p->gb, 0xf1, 128, 96 + (int)i * 16);
                PbrbGbGlyph(p->gb, (unsigned char)(0xf9u - i), 136, 96 + (int)i * 16);
            }
            PbrbGbGlyph(p->gb, 0xed, 120, 96 + p->serviceSelection * 16);
        }
        else if (!p->text.active && p->serviceStep == PSS_SLOTS_AGAIN)PsYesNo(p, 112, 96);
        if (d[24] && p->text.visible && p->serviceStep == PSS_SLOTS_PAY) {
            for (unsigned y = 0; y < 2; ++y)for (unsigned x = 0; x < 2; ++x)
                PsSlotBackgroundTile(p, d[11] + 0x25u + (1u - y) * 2u + x, 16 + (int)x * 8, 104 + (int)y * 8);
        }
    }
}
#endif
