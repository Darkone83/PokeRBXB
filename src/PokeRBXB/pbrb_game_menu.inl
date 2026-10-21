/* Native party, inventory, storage and progression menus. All names, monster
   art, mini sprites, move data and Pokedex entries come from the attached ROM. */
static int PplayDexMenuTick(PbrbPlay*, unsigned);
static int PplayDexMenuRender(PbrbPlay*);
static void PplayMenuText(PbrbPlay* p, int text, int returnTo) {
    p->noticeReturn = returnTo; p->overlay = PBRB_OVERLAY_NOTICE;
    PplayGameText(p, text, 0, PPLAY_AFTER_NOTICE);
}
static void PplayMenuName(PbrbPlay* p, unsigned item) {
    PbrbGameRom rom = PplayRom(p); PbrbGameItemName(&rom, item, p->nameBuffer);
    memset(p->stringBuffer, 0x50, 32); memcpy(p->stringBuffer, p->nameBuffer, 16);
}
static void PplayMenuMoveName(PbrbPlay* p, unsigned move) {
    PbrbGameRom rom = PplayRom(p); unsigned char name[16]; PbrbGameMoveName(&rom, move, name);
    memset(p->stringBuffer, 0x50, 32); memcpy(p->stringBuffer, name, 16);
}
static int PplayListInput(PbrbPlay* p, unsigned keys, unsigned count) {
    if (!count) { p->selection = 0; return 0; }
    if (p->selection < 0 || (unsigned)p->selection >= count)p->selection = 0;
    if (keys & PBRB_PAD_UP)p->selection = (p->selection + (int)count - 1) % (int)count;
    if (keys & PBRB_PAD_DOWN)p->selection = (p->selection + 1) % (int)count;
    return (keys & PBRB_PAD_A) != 0;
}
static void PplayItemSwap(PbrbPlay* p, PbrbGameItem* items, unsigned char* count) {
    if (p->selection < 0 || (unsigned)p->selection >= *count)return;
    if (p->itemSwap < 0 || (unsigned)p->itemSwap >= *count) { p->itemSwap = p->selection; return; }
    if (p->itemSwap == p->selection)return;
    unsigned before = *count;
    if (PbrbGameSwapItems(items, count, (unsigned)p->itemSwap, (unsigned)p->selection)) {
        if (*count != before)p->selection = 0;
        p->itemSwap = -1;
    }
}
static int PplayConsumeBagItem(PbrbPlay* p, unsigned item, unsigned quantity) {
    PbrbGameState* s = &p->gameState;
    if (p->itemSlot < 0)return PbrbGameRemoveItem(s->bag, &s->bagCount, item, quantity);
    unsigned slot = (unsigned)p->itemSlot;
    if (slot >= s->bagCount || s->bag[slot].id != item)return 0;
    return PbrbGameRemoveItemAt(s->bag, &s->bagCount, slot, quantity);
}
static PbrbGameItem* PplayOperationItems(PbrbPlay* p, unsigned char** count) {
    PbrbGameState* s = &p->gameState;
    if (p->menuMode == 2 || p->menuMode == 5) { *count = &s->pcItemCount; return s->pcItems; }
    *count = &s->bagCount; return s->bag;
}
static unsigned PplayOperationQuantity(PbrbPlay* p) {
    if (p->menuMode == 0)return 99;
    unsigned char* count = 0; PbrbGameItem* items = PplayOperationItems(p, &count);
    return p->itemSlot >= 0 && (unsigned)p->itemSlot < *count && items[p->itemSlot].id == p->menuItem ? items[p->itemSlot].count : 0;
}
static int PplayReturnItemList(PbrbPlay* p) {
    p->text.visible = 0; p->itemSwap = -1;
    if (p->menuMode < 2) { p->overlay = PBRB_OVERLAY_SHOP_LIST; p->selection = p->menuTarget; }
    else if (p->menuMode == 4) { p->overlay = PBRB_OVERLAY_ITEM; p->selection = p->menuCursor; }
    else {
        p->menuMode = p->menuMode == 2 ? 0 : p->menuMode == 3 ? 1 : 2;
        p->overlay = PBRB_OVERLAY_PC_ITEMS; p->selection = p->menuTarget;
    }
    p->itemSlot = -1; return p->overlay;
}
static void PplayTransferItems(PbrbPlay* p) {
    PbrbGameState* s = &p->gameState; unsigned item = (unsigned)p->menuItem, quantity = (unsigned)p->menuCount;
    int withdraw = p->menuMode == 2, success = 0;
    if (quantity && quantity <= PplayOperationQuantity(p)) {
        unsigned char* count = 0; PbrbGameItem* source = PplayOperationItems(p, &count);
        success = withdraw ? PbrbGameAddItem(s->bag, &s->bagCount, PBRB_BAG_CAPACITY, item, quantity) :
            PbrbGameAddItem(s->pcItems, &s->pcItemCount, PBRB_PC_ITEM_CAPACITY, item, quantity);
        if (success)PbrbGameRemoveItemAt(source, count, (unsigned)p->itemSlot, quantity);
    }
    PplayMenuName(p, item); p->textNumber = quantity; int returnTo = PplayReturnItemList(p);
    if (success)p->selection = 0;
    PplaySyncAliases(p); PbrbAudioPlaySfx(success ? PBRB_SFX_WITHDRAW_DEPOSIT : PBRB_SFX_DENIED);
    PplayMenuText(p, withdraw ? (success ? PBRB_GT_WithdrewItemText : PBRB_GT_CantCarryMoreText) :
        (success ? PBRB_GT_ItemWasStoredText : PBRB_GT_NoRoomToStoreText), returnTo);
}
static void PplayConfirmItemOperation(PbrbPlay* p) {
    PbrbGameRom rom = PplayRom(p); unsigned item = (unsigned)p->menuItem;
    PplayMenuName(p, item); p->textNumber = (unsigned)p->menuCount;
    p->textMoney = PbrbGameItemPrice(&rom, item) / (p->menuMode == 1 ? 2u : 1u) * (unsigned)p->menuCount;
    p->selection = 0; p->overlay = PBRB_OVERLAY_MENU_CONFIRM;
    PplayGameText(p, p->menuMode == 0 ? PBRB_GT_PokemartTellBuyPriceText :
        p->menuMode == 1 ? PBRB_GT_PokemartTellSellPriceText : PBRB_GT_IsItOKToTossItemText, 0, PPLAY_AFTER_NONE);
}
static void PplayCommitItemOperation(PbrbPlay* p, int yes) {
    PbrbGameState* s = &p->gameState; PbrbGameRom rom = PplayRom(p);
    unsigned item = (unsigned)p->menuItem, quantity = (unsigned)p->menuCount; int mode = p->menuMode;
    if (!yes) { PplayReturnItemList(p); return; }
    if (!quantity || quantity > PplayOperationQuantity(p)) { PplayReturnItemList(p); return; }
    p->text.visible = 0;
    if (mode == 0) {
        unsigned price = PbrbGameItemPrice(&rom, item) * quantity; int result = PBRB_GT_PokemartNotEnoughMoneyText;
        if (price <= s->money) {
            int success = PbrbGameAddItem(s->bag, &s->bagCount, PBRB_BAG_CAPACITY, item, quantity);
            if (success) { s->money -= price; PbrbAudioPlaySfx(PBRB_SFX_PURCHASE); }
            result = success ? PBRB_GT_PokemartBoughtItemText : PBRB_GT_PokemartItemBagFullText;
        }
        int returnTo = PplayReturnItemList(p); PplaySyncAliases(p); PplayMenuText(p, result, returnTo); return;
    }
    if (PbrbGameIsKeyItem(&rom, item)) {
        int returnTo = PplayReturnItemList(p);
        PplayMenuText(p, mode == 1 ? PBRB_GT_PokemartUnsellableItemText : PBRB_GT_TooImportantToTossText, returnTo); return;
    }
    unsigned char* count = 0; PbrbGameItem* items = PplayOperationItems(p, &count);
    int success = PbrbGameRemoveItemAt(items, count, (unsigned)p->itemSlot, quantity);
    if (success && mode == 1) { s->money += PbrbGameItemPrice(&rom, item) / 2u * quantity; if (s->money > 999999u)s->money = 999999u; }
    PplayMenuName(p, item); int returnTo = PplayReturnItemList(p); if (success)p->selection = 0;
    PplaySyncAliases(p);
    if (mode == 1) { if (success)PbrbAudioPlaySfx(PBRB_SFX_PURCHASE); }
    else if (success)PplayMenuText(p, PBRB_GT_ThrewAwayItemText, returnTo);
}
static void PplayMenuBack(PbrbPlay* p) {
    p->overlay = p->scene == PBRB_PLAY_BATTLE ? PBRB_OVERLAY_NONE : PBRB_OVERLAY_START;
    p->selection = p->savedSelection; p->menuMode = 0; p->menuSwap = -1; p->itemSwap = p->moveSwap = -1;
}
static PbrbPlayMon* PplayMenuMon(PbrbPlay* p, unsigned index) {
    if (index >= p->gameState.partyCount)return 0;
    return p->scene == PBRB_PLAY_BATTLE && p->battle.state != PBRB_BATTLE_FINISHED ? p->battle.party[0] + index : p->gameState.party + index;
}
static unsigned PplayFieldMoveFor(unsigned move) {
    switch (move) {
    case 15:return PBRB_WORLD_FIELD_CUT; case 19:return PBRB_WORLD_FIELD_FLY;
    case 57:return PBRB_WORLD_FIELD_SURF; case 70:return PBRB_WORLD_FIELD_STRENGTH;
    case 100:return PBRB_WORLD_FIELD_TELEPORT; case 148:return PBRB_WORLD_FIELD_FLASH;
    case 91:return PBRB_WORLD_FIELD_DIG; default:return 0;
    }
}
static unsigned PplayPartyFieldMoves(const PbrbPlayMon* m, unsigned* moves) {
    unsigned count = 0; for (unsigned i = 0; i < 4; ++i) {
        unsigned move = m->moves[i];
        if (PplayFieldMoveFor(move) || move == 135) {
            unsigned j = 0; while (j < count && moves[j] != move)++j;
            if (j == count)moves[count++] = move;
        }
    }
    return count;
}
static int PplayIsHM(const PbrbGameRom* r, unsigned move) {
    for (unsigned i = 51; i <= 55; ++i)if (PbrbGameTM(r, i) == move)return 1; return 0;
}
static void PplayOpenFly(PbrbPlay* p) {
    PplayWorldPlayer(p);
    if (!PbrbWorldUseFieldMove(p->world, PBRB_WORLD_FIELD_FLY)) {
        PplayMenuText(p, PBRB_GT_ItemUseNotTimeText, PBRB_OVERLAY_PARTY); return;
    }
    p->selection = 0; while (p->selection < 10 && !(p->world->visitedTowns & (1u << p->selection)))++p->selection;
    p->menuMode = 2; p->overlay = PBRB_OVERLAY_TOWN_MAP;
}
static int PplayCheckEvolution(PbrbPlay* p);
static void PplayLearnFinish(PbrbPlay* p, int slot) {
    PbrbGameRom rom = PplayRom(p); PbrbPlayMon* m = PplayMenuMon(p, (unsigned)p->pendingParty);
    if (!m) { PplayMenuBack(p); return; }
    if (slot >= 0 && slot < 4 && PplayIsHM(&rom, m->moves[slot])) { PplayMenuText(p, PBRB_GT_HMCantDeleteText, PBRB_OVERLAY_LEARN); return; }
    if (p->learnReturn == 1) { p->battleEventActive = 0; PplayBattleSubmit(p, PBRB_BATTLE_LEARN, (unsigned)(slot < 0 ? 4 : slot), 0, 0, 0); return; }
    p->overlay = p->learnReturn == 2 ? PBRB_OVERLAY_ITEM : PBRB_OVERLAY_PROGRESS; p->text.visible = 0;
    if (slot < 0) { PplayMenuText(p, PBRB_GT_DidNotLearnText, p->overlay); return; }
    m->moves[slot] = (unsigned char)p->pendingMove; m->pp[slot] = (unsigned char)PbrbGameMaxPP(&rom, (unsigned)p->pendingMove, 0);
    if (p->learnReturn == 2 && p->menuItem >= 0xc9)PplayConsumeBagItem(p, (unsigned)p->menuItem, 1);
    PplaySyncAliases(p); PbrbAudioPlaySfx(PBRB_SFX_LEVEL_UP); PplayMenuText(p, PBRB_GT_LearnedMove1Text, p->overlay);
}
static void PplayOfferMove(PbrbPlay* p, unsigned party, unsigned move, int from) {
    PbrbPlayMon* m = PplayMenuMon(p, party); if (!m)return;
    p->pendingParty = (int)party; p->pendingMove = (int)move; p->learnReturn = from; PplayMenuMoveName(p, move);
    for (int i = 0; i < 4; ++i) { if (m->moves[i] == move) { PplayMenuText(p, PBRB_GT_ItemUseNoEffectText, from == 2 ? PBRB_OVERLAY_ITEM : PBRB_OVERLAY_NONE); return; } }
    for (int i = 0; i < 4; ++i)if (!m->moves[i]) { PplayLearnFinish(p, i); return; }
    p->overlay = PBRB_OVERLAY_NONE; PplayGameText(p, PBRB_GT_TryingToLearnText, 0, PPLAY_AFTER_LEARN);
}
static void PplayQueueLevelMoves(PbrbPlay* p, unsigned party, int evolution, int returnTo) {
    PbrbGameRom rom = PplayRom(p); PbrbPlayMon* m = p->gameState.party + party;
    p->pendingParty = (int)party; p->pendingMoveIndex = 0;
    p->pendingMoveCount = (unsigned char)PbrbGameMovesAtLevel(&rom, m->species, m->level, p->pendingMoves);
    p->levelCheckEvolution = evolution; p->levelReturn = returnTo; p->overlay = PBRB_OVERLAY_PROGRESS;
}
static void PplayLevelProgress(PbrbPlay* p) {
    if (p->pendingParty < 0 || (unsigned)p->pendingParty >= p->gameState.partyCount) { p->overlay = PBRB_OVERLAY_NONE; return; }
    PbrbPlayMon* m = p->gameState.party + p->pendingParty;
    while (p->pendingMoveIndex < p->pendingMoveCount) {
        unsigned move = p->pendingMoves[p->pendingMoveIndex++]; int known = 0;
        for (unsigned i = 0; i < 4; ++i)if (m->moves[i] == move)known = 1;
        if (!known) { PplayOfferMove(p, (unsigned)p->pendingParty, move, 3); return; }
    }
    p->pendingMoveCount = p->pendingMoveIndex = 0; p->overlay = p->levelReturn; p->selection = 0;
    if (p->levelCheckEvolution) { p->levelCheckEvolution = 0; PplayCheckEvolution(p); }
}
static void PplayBeginEvolution(PbrbPlay* p, unsigned party, unsigned species, int item) {
    PbrbPlayMon* m = p->gameState.party + party; p->evolutionParty = (int)party; p->evolutionSpecies = (int)species;
    p->evolutionPhase = 0; p->menuItem = item; p->timer = 50; p->overlay = PBRB_OVERLAY_EVOLUTION;
    p->evolutionReturn = p->scene == PBRB_PLAY_BATTLE ? PBRB_OVERLAY_NONE : PBRB_OVERLAY_ITEM;
    memset(p->stringBuffer, 0x50, 32); memcpy(p->stringBuffer, m->nickname, 11);
    PbrbGbDecodePokemon(p->gb, m->species, 0, p->picture, sizeof(p->picture));
    PbrbGbDecodePokemon(p->gb, species, 0, p->enemyPicture, sizeof(p->enemyPicture));
    PplayGameText(p, PBRB_GT_IsEvolvingText, 0, PPLAY_AFTER_EVOLUTION);
}
static int PplayCheckEvolution(PbrbPlay* p) {
    PbrbGameRom rom = PplayRom(p);
    for (unsigned i = 0; i < p->gameState.partyCount; ++i) {
        PbrbPlayMon* m = p->gameState.party + i;
        if (m->level <= p->evolutionLevels[i])continue; p->evolutionLevels[i] = m->level;
        unsigned next = PbrbGameEvolution(&rom, m->species, m->level, 0, 0);
        if (next) { PplayBeginEvolution(p, i, next, 0); return 1; }
    }return 0;
}
static void PplayEvolutionTick(PbrbPlay* p, unsigned keys) {
    PbrbGameRom rom = PplayRom(p); PbrbPlayMon* m = p->gameState.party + p->evolutionParty;
    if (p->evolutionPhase == 0) {
        if (p->timer > 0) { --p->timer; return; }PbrbPlayRender(p);
        if (!PbrbBattleAnimEvolution(&p->battleAnim, p->gb, m->species, (unsigned)p->evolutionSpecies, p->menuItem != 0)) {
            p->overlay = p->evolutionReturn; PplayNotice(p, "Evolution data error.", "Please reload.", p->overlay); return;
        }
        p->evolutionPhase = 1; return;
    }
    if (p->evolutionPhase == 1) {
        if (keys & PBRB_PAD_B)PbrbBattleAnimCancelEvolution(&p->battleAnim);
        PbrbBattleAnimTick(&p->battleAnim); if (!PbrbBattleAnimDone(&p->battleAnim))return;
        p->battleAnim.active = 0;
        if (p->battleAnim.cancelled) { p->evolutionPhase = 3; PplayGameText(p, PBRB_GT_StoppedEvolvingText, 0, PPLAY_AFTER_EVOLUTION); return; }
        unsigned char oldName[11]; PbrbGameMonName(&rom, m->species, oldName);
        int defaultName = !memcmp(oldName, m->nickname, 11); m->species = (unsigned char)p->evolutionSpecies;
        if (defaultName)PbrbGameMonName(&rom, m->species, m->nickname); PbrbGameRecalculate(&rom, m, 1);
        PbrbGameMarkDex(&rom, &p->gameState, m->species, 1);
        if (p->menuItem)PplayConsumeBagItem(p, (unsigned)p->menuItem, 1);
        PplayMonsterName(p, m->species, p->nameBuffer); PplayGameText(p, PBRB_GT_EvolvedText, 0, PPLAY_AFTER_EVOLUTION);
        PplayAppendGameText(p, PBRB_GT_IntoText); PbrbAudioPlaySfx(PBRB_SFX_GET_ITEM2); p->evolutionPhase = 2; return;
    }
    if (PbrbAudioEffectsBusy())return;
    p->overlay = p->evolutionReturn; p->text.visible = 0; PplaySyncAliases(p);
    PbrbAudioPlayMusic(p->scene == PBRB_PLAY_BATTLE ? PBRB_MUSIC_DEFEATED_TRAINER : PbrbWorldMusic(p->world));
    if (p->evolutionPhase == 2)PplayQueueLevelMoves(p, (unsigned)p->evolutionParty, 0, p->evolutionReturn);
}
static void PplayUseItemOnMon(PbrbPlay* p, unsigned party, unsigned slot) {
    PbrbGameRom rom = PplayRom(p); unsigned item = (unsigned)p->menuItem; PbrbPlayMon* m = PplayMenuMon(p, party); if (!m)return;
    p->menuTarget = (int)party;
    if (p->scene == PBRB_PLAY_BATTLE) { PplayBattleSubmit(p, PBRB_BATTLE_ITEM, 0, item, party, slot); return; }
    if (item >= 0xc4 && item <= 0xfa) {
        unsigned tm = item < 0xc9 ? item - 0xc4 + 51 : item - 0xc9 + 1;
        if (!PbrbGameCanLearnTM(&rom, m->species, tm)) { PplayMenuText(p, PBRB_GT_MonCannotLearnMachineMoveText, PBRB_OVERLAY_ITEM_TARGET); return; }
        PplayOfferMove(p, party, PbrbGameTM(&rom, tm), 2); return;
    }
    if (item == 0x0a || item == 0x20 || item == 0x21 || item == 0x22 || item == 0x2f) {
        unsigned evo = PbrbGameEvolution(&rom, m->species, m->level, item, 0);
        if (evo)PplayBeginEvolution(p, party, evo, (int)item); else PplayMenuText(p, PBRB_GT_ItemUseNoEffectText, PBRB_OVERLAY_ITEM_TARGET); return;
    }
    unsigned oldLevel = m->level, oldHP = m->hp;
    for (unsigned i = 0; i < p->gameState.partyCount; ++i)p->evolutionLevels[i] = p->gameState.party[i].level;
    if (!PbrbGameUseMedicine(&rom, m, item, slot)) { PplayMenuText(p, PBRB_GT_ItemUseNoEffectText, PBRB_OVERLAY_ITEM_TARGET); return; }
    PplayConsumeBagItem(p, item, 1); PplaySyncAliases(p);
    int healHP = (item >= 0x10 && item <= 0x14) || item == 0x35 || item == 0x36 || (item >= 0x3c && item <= 0x3e);
    PbrbAudioPlaySfx(healHP && m->hp > oldHP ? PBRB_SFX_HEAL : PBRB_SFX_HEAL_AILMENT);
    if (m->level > oldLevel) {
        p->evolutionLevels[party] = (unsigned char)oldLevel;
        p->textLevel = m->level; PplayCopyName(p->nameBuffer, m->nickname, 10); PplayQueueLevelMoves(p, party, 1, PBRB_OVERLAY_ITEM);
        PplayMenuText(p, PBRB_GT_GrewLevelText, PBRB_OVERLAY_PROGRESS); return;
    }
    PplayCopyName(p->nameBuffer, m->nickname, 10); p->textNumber = m->hp - oldHP;
    int text = PBRB_GT_PotionText;
    if (item >= 0x0b && item <= 0x0f) {
        static const int cures[] = { PBRB_GT_AntidoteText,PBRB_GT_BurnHealText,
PBRB_GT_IceHealText,PBRB_GT_AwakeningText,PBRB_GT_ParlyzHealText }; text = cures[item - 0x0b];
    }
    else if (item == 0x34 || (item == 0x10 && m->hp == oldHP))text = PBRB_GT_FullHealText;
    else if (item == 0x35 || item == 0x36)text = PBRB_GT_ReviveText;
    else if (item >= 0x23 && item <= 0x27) {
        static const char* stats[] = { "HEALTH","ATTACK","DEFENSE","SPEED","SPECIAL" };
        PplayAsciiName(p->stringBuffer, stats[item - 0x23]); text = PBRB_GT_VitaminStatRoseText;
    }
    else if (item == 0x4f) { PplayMenuMoveName(p, m->moves[slot]); text = PBRB_GT_PPIncreasedText; }
    else if (item >= 0x50 && item <= 0x53)text = PBRB_GT_PPRestoredText;
    PplayMenuText(p, text, PBRB_OVERLAY_ITEM);
}
static void PplayUseItem(PbrbPlay* p) {
    unsigned item = (unsigned)p->menuItem; PplayMenuName(p, item);
    if (p->scene == PBRB_PLAY_BATTLE) {
        if (item <= 4) {
            if (!p->battle.setup.trainerClass && p->gameState.partyCount == 6 && p->gameState.boxCount[p->gameState.currentBox] == 20) { PplayMenuText(p, PBRB_GT_BoxFullCannotThrowBallText, PBRB_OVERLAY_ITEM); return; }
            PplayBattleSubmit(p, PBRB_BATTLE_ITEM, 0, item, 0, 0); return;
        }
        if (item == 0x2e || item == 0x33 || item == 0x37 || item == 0x3a || (item >= 0x41 && item <= 0x44) || item == 0x49) { PplayBattleSubmit(p, PBRB_BATTLE_ITEM, 0, item, p->battle.active[0], 0); return; }
    }
    else {
        if (item == 5) { p->menuMode = 1; p->selection = -1; p->overlay = PBRB_OVERLAY_TOWN_MAP; return; }
        if (item == 0x1e || item == 0x38 || item == 0x39) {
            p->gameState.repelSteps = item == 0x1e ? 100u : item == 0x38 ? 200u : 250u;
            PplayConsumeBagItem(p, item, 1); p->overlay = PBRB_OVERLAY_NONE; return;
        }
        if (item == 6 || item == 0x1d || item == 0x47 || item == 0x49 || (item >= 0x4c && item <= 0x4e)) {
            PplayWorldPlayer(p); if (PbrbWorldUseItem(p->world, item)) {
                if (item == 0x1d)PplayConsumeBagItem(p, item, 1);
                p->overlay = PBRB_OVERLAY_NONE; p->worldEvent = 0; return;
            }
            PplayMenuText(p, PBRB_GT_ItemUseNotTimeText, PBRB_OVERLAY_ITEM); return;
        }
    }
    int target = (item >= 0x0a && item <= 0x14) || (item >= 0x20 && item <= 0x28) || item == 0x2f || item == 0x34 || item == 0x35 || item == 0x36 ||
        (item >= 0x3c && item <= 0x3e) || (item >= 0x4f && item <= 0x53) || (item >= 0xc4 && item <= 0xfa);
    if (!target || !p->gameState.partyCount) { PplayMenuText(p, PBRB_GT_ItemUseNotTimeText, PBRB_OVERLAY_ITEM); return; }
    p->selection = 0; p->overlay = PBRB_OVERLAY_ITEM_TARGET;
}
static void PplayPCTransferMon(PbrbPlay* p, unsigned index) {
    PbrbGameRom rom = PplayRom(p); PbrbGameState* s = &p->gameState; unsigned box = s->currentBox;
    if (p->menuMode == 0) {
        if (index >= s->boxCount[box])return;
        if (s->partyCount >= 6) { PplayMenuText(p, PBRB_GT_CantTakeMonText, PBRB_OVERLAY_PC); return; }
        PbrbPlayMon m = s->boxes[box][index]; PbrbGameRecalculate(&rom, &m, 0); PbrbGameHeal(&rom, &m); s->party[s->partyCount++] = m;
        memmove(s->boxes[box] + index, s->boxes[box] + index + 1, (s->boxCount[box] - index - 1u) * sizeof(m)); --s->boxCount[box];
        memset(s->boxes[box] + s->boxCount[box], 0, sizeof(m)); PplayCopyName(p->nameBuffer, m.nickname, 10); PplayCopyName(p->stringBuffer, m.nickname, 10);
        p->selection = 0; PplayMenuText(p, PBRB_GT_MonIsTakenOutText, PBRB_OVERLAY_PC);
    }
    else {
        if (index >= s->partyCount)return;
        if (s->partyCount <= 1) { PplayMenuText(p, PBRB_GT_CantDepositLastMonText, PBRB_OVERLAY_PC); return; }
        if (s->boxCount[box] >= 20) { PplayMenuText(p, PBRB_GT_BoxFullText, PBRB_OVERLAY_PC); return; }
        PbrbPlayMon m = s->party[index]; PbrbGameRecalculate(&rom, &m, 0); PbrbGameHeal(&rom, &m); s->boxes[box][s->boxCount[box]++] = m;
        memmove(s->party + index, s->party + index + 1, (s->partyCount - index - 1u) * sizeof(m)); --s->partyCount; memset(s->party + s->partyCount, 0, sizeof(m));
        s->active = 0; PplayCopyName(p->nameBuffer, m.nickname, 10); PplayCopyName(p->stringBuffer, m.nickname, 10); p->selection = 0; PplayMenuText(p, PBRB_GT_MonWasStoredText, PBRB_OVERLAY_PC);
    }PplaySyncAliases(p); PbrbAudioPlaySfx(PBRB_SFX_WITHDRAW_DEPOSIT);
}
static int PplayMenuAnswer(PbrbPlay* p, int question, int yes) {
    if (question == PPLAY_QUESTION_CHANGE_BOX) {
        p->text.visible = 0; p->overlay = yes ? PBRB_OVERLAY_BOX : PBRB_OVERLAY_PC;
        p->selection = yes ? p->gameState.currentBox : 0; return 1;
    }
    if (question == PPLAY_QUESTION_BATTLE_SWITCH) {
        p->text.visible = 0;
        if (yes) { p->overlay = PBRB_OVERLAY_PARTY; p->selection = 0; p->menuMode = 4; }
        else PplayBattleSubmit(p, PBRB_BATTLE_DECLINE, 0, 0, 0, 0); return 1;
    }
    if (question == PPLAY_QUESTION_WORLD) { p->text.visible = 0; PbrbWorldAcknowledge(p->world, yes); p->worldEvent = 0; return 1; }
    if (question == PPLAY_QUESTION_LEARN) { p->text.visible = 0; if (yes) { p->overlay = PBRB_OVERLAY_LEARN; p->selection = 0; } else PplayLearnFinish(p, -1); return 1; }
    if (question == PPLAY_QUESTION_TOSS) {
        PplayCommitItemOperation(p, yes); return 1;
    }
    if (question == PPLAY_QUESTION_RELEASE) {
        PbrbGameState* s = &p->gameState; unsigned box = s->currentBox, index = (unsigned)p->menuTarget; p->text.visible = 0;
        if (yes && index < s->boxCount[box]) {
            PplayCopyName(p->nameBuffer, s->boxes[box][index].nickname, 10); PplayCopyName(p->stringBuffer, s->boxes[box][index].nickname, 10);
            memmove(s->boxes[box] + index, s->boxes[box] + index + 1, (s->boxCount[box] - index - 1u) * sizeof(PbrbPlayMon)); --s->boxCount[box];
            memset(s->boxes[box] + s->boxCount[box], 0, sizeof(PbrbPlayMon)); PplayMenuText(p, PBRB_GT_MonWasReleasedText, PBRB_OVERLAY_PC_MONS);
        }
        else p->overlay = PBRB_OVERLAY_PC_MONS; p->selection = 0; return 1;
    }return 0;
}
static int PplayGameplayMenuTick(PbrbPlay* p, unsigned keys) {
    if (PplayDexMenuTick(p, keys))return 1;
    PbrbGameState* s = &p->gameState; PbrbGameRom rom = PplayRom(p); int overlay = p->overlay;
    if (overlay == PBRB_OVERLAY_PROGRESS) { PplayLevelProgress(p); return 1; }
    if (overlay == PBRB_OVERLAY_START) {
        int dex = s->pokedex ? 1 : 0; int accept = PplayListInput(p, keys, 6u + (unsigned)dex);
        if (keys & (PBRB_PAD_B | PBRB_PAD_START)) { p->savedSelection = p->selection; p->overlay = PBRB_OVERLAY_NONE; return 1; }
        if (!accept)return 1; p->savedSelection = p->selection; PbrbAudioPlaySfx(PBRB_SFX_PRESS);
        int entry = p->selection - dex;
        if (entry < 0) { p->overlay = PBRB_OVERLAY_POKEDEX; p->selection = 0; p->menuCursor = 0; p->menuCount = 0; p->menuMode = 0; }
        else if (entry == 0) { if (s->partyCount) { p->overlay = PBRB_OVERLAY_PARTY; p->selection = 0; p->menuMode = 0; p->menuSwap = -1; } }
        else if (entry == 1) { p->overlay = PBRB_OVERLAY_ITEM; p->selection = 0; p->itemSwap = p->itemSlot = -1; }
        else if (entry == 2)p->overlay = PBRB_OVERLAY_TRAINER;
        else if (entry == 3) { if (PbrbPlayCanSave(p)) { p->saveInfo = 1; p->overlay = PBRB_OVERLAY_NONE; PplayText(p, PBRB_DATA_WouldYouLikeToSaveText, 0, PPLAY_AFTER_SAVE); } }
        else if (entry == 4) { p->overlay = PBRB_OVERLAY_OPTIONS; p->optionSelection = 0; }
        else p->overlay = PBRB_OVERLAY_NONE; return 1;
    }
    if (overlay == PBRB_OVERLAY_PARTY || overlay == PBRB_OVERLAY_ITEM_TARGET) {
        int accept = PplayListInput(p, keys, s->partyCount);
        if (keys & PBRB_PAD_B) {
            if (p->scene == PBRB_PLAY_BATTLE && p->battle.state == PBRB_BATTLE_FORCE_SWITCH)return 1;
            if (p->scene == PBRB_PLAY_BATTLE && p->battle.state == PBRB_BATTLE_SWITCH_OFFER) { PplayBattleSubmit(p, PBRB_BATTLE_DECLINE, 0, 0, 0, 0); return 1; }
            if (overlay == PBRB_OVERLAY_ITEM_TARGET) { p->overlay = PBRB_OVERLAY_ITEM; p->selection = 0; }
            else PplayMenuBack(p); return 1;
        }
        if (!accept || !s->partyCount)return 1;
        if (overlay == PBRB_OVERLAY_ITEM_TARGET) {
            p->menuTarget = p->selection;
            if (p->menuItem >= 0x4f && p->menuItem <= 0x51) { p->selection = 0; p->overlay = PBRB_OVERLAY_ITEM_MOVES; }
            else PplayUseItemOnMon(p, (unsigned)p->selection, 0); return 1;
        }
        if (p->scene == PBRB_PLAY_BATTLE) { p->menuTarget = p->selection; p->selection = 0; p->overlay = PBRB_OVERLAY_PARTY_ACTION; return 1; }
        if (p->menuSwap >= 0) {
            PbrbPlayMon temp = s->party[p->menuSwap]; s->party[p->menuSwap] = s->party[p->selection]; s->party[p->selection] = temp;
            s->active = 0; p->menuSwap = -1; PplaySyncAliases(p); PbrbAudioPlaySfx(PBRB_SFX_SWAP); return 1;
        }
        p->menuTarget = p->selection; p->selection = 0; p->overlay = PBRB_OVERLAY_PARTY_ACTION; return 1;
    }
    if (overlay == PBRB_OVERLAY_PARTY_ACTION) {
        int inBattle = p->scene == PBRB_PLAY_BATTLE; unsigned fields[4];
        unsigned count = inBattle ? 0u : PplayPartyFieldMoves(s->party + p->menuTarget, fields);
        int accept = PplayListInput(p, keys, count + 3u);
        if (keys & PBRB_PAD_B) { p->overlay = PBRB_OVERLAY_PARTY; p->selection = p->menuTarget; return 1; }
        if (!accept)return 1;
        if ((unsigned)p->selection < count) {
            unsigned move = fields[p->selection]; PbrbPlayMon* m = s->party + p->menuTarget;
            if (move == 135) {
                if (m->hp <= m->maxHP / 5u) { PplayMenuText(p, PBRB_GT_NotHealthyEnoughText, PBRB_OVERLAY_PARTY); return 1; }
                p->selection = p->menuTarget; p->overlay = PBRB_OVERLAY_SOFTBOILED; return 1;
            }
            PplayWorldPlayer(p); unsigned field = PplayFieldMoveFor(move);
            if (field == PBRB_WORLD_FIELD_FLY)PplayOpenFly(p);
            else if (PbrbWorldUseFieldMove(p->world, field)) { p->overlay = PBRB_OVERLAY_NONE; p->worldEvent = 0; }
            else PplayMenuText(p, PBRB_GT_ItemUseNotTimeText, PBRB_OVERLAY_PARTY); return 1;
        }
        unsigned action = (unsigned)p->selection - count; if (inBattle && action < 2)action ^= 1u;
        if (action == 0) { p->overlay = PBRB_OVERLAY_STATUS; p->menuPage = 0; PbrbAudioPlayCry(s->party[p->menuTarget].species); }
        else if (action == 1) {
            if (inBattle) {
                PbrbPlayMon* m = p->battle.party[0] + p->menuTarget;
                if (!m->hp) { p->selection = p->menuTarget; PplayMenuText(p, PBRB_GT_NoWillText, PBRB_OVERLAY_PARTY); return 1; }
                if ((unsigned)p->menuTarget == p->battle.active[0]) { p->selection = p->menuTarget; PplayMenuText(p, PBRB_GT_AlreadyOutText, PBRB_OVERLAY_PARTY); return 1; }
                PplayBattleSubmit(p, PBRB_BATTLE_SWITCH, (unsigned)p->menuTarget, 0, 0, 0); return 1;
            }
            p->menuSwap = p->menuTarget; p->overlay = PBRB_OVERLAY_PARTY; p->selection = p->menuTarget;
        }
        else { p->overlay = PBRB_OVERLAY_PARTY; p->selection = p->menuTarget; }return 1;
    }
    if (overlay == PBRB_OVERLAY_SOFTBOILED) {
        int accept = PplayListInput(p, keys, s->partyCount);
        if (keys & PBRB_PAD_B) { p->overlay = PBRB_OVERLAY_PARTY; p->selection = p->menuTarget; return 1; }
        if (!accept || p->selection == p->menuTarget)return 1;
        PbrbPlayMon* donor = s->party + p->menuTarget; PbrbPlayMon* target = s->party + p->selection;
        if (!target->hp || target->hp == target->maxHP) { PplayMenuText(p, PBRB_GT_ItemUseNoEffectText, PBRB_OVERLAY_SOFTBOILED); return 1; }
        unsigned amount = donor->maxHP / 5u; if (donor->hp <= amount) { PplayMenuText(p, PBRB_GT_NotHealthyEnoughText, PBRB_OVERLAY_PARTY); return 1; }
        unsigned recovered = target->maxHP - target->hp; if (recovered > amount)recovered = amount;
        donor->hp -= amount; target->hp += recovered; PplayCopyName(p->nameBuffer, target->nickname, 10); p->textNumber = recovered;
        PplaySyncAliases(p); PbrbAudioPlaySfx(PBRB_SFX_HEAL); p->selection = p->menuTarget;
        PplayMenuText(p, PBRB_GT_PotionText, PBRB_OVERLAY_PARTY); return 1;
    }
    if (overlay == PBRB_OVERLAY_STATUS || overlay == PBRB_OVERLAY_PC_MON_STATUS) {
        if (keys & (PBRB_PAD_A | PBRB_PAD_B)) {
            if (!p->menuPage)p->menuPage = 1;
            else if (overlay == PBRB_OVERLAY_PC_MON_STATUS) { p->overlay = PBRB_OVERLAY_PC_MON_ACTION; p->selection = 1; }
            else { p->overlay = PBRB_OVERLAY_PARTY; p->selection = p->menuTarget; }
        }return 1;
    }
    if (overlay == PBRB_OVERLAY_FIELD_MOVE) {
        if (keys & PBRB_PAD_B) { p->overlay = PBRB_OVERLAY_PARTY; p->selection = p->menuTarget; return 1; }
        PbrbPlayMon* m = s->party + p->menuTarget; unsigned n = 0, field[4];
        for (unsigned i = 0; i < 4; ++i)if (PplayFieldMoveFor(m->moves[i]))field[n++] = PplayFieldMoveFor(m->moves[i]);
        if (PplayListInput(p, keys, n) && n) {
            PplayWorldPlayer(p); unsigned move = field[p->selection];
            if (move == PBRB_WORLD_FIELD_FLY)PplayOpenFly(p);
            else if (PbrbWorldUseFieldMove(p->world, move)) { p->overlay = PBRB_OVERLAY_NONE; p->worldEvent = 0; }
            else PplayMenuText(p, PBRB_GT_ItemUseNotTimeText, PBRB_OVERLAY_PARTY);
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_ITEM) {
        int accept = PplayListInput(p, keys, (unsigned)s->bagCount + 1u);
        if ((keys & PBRB_PAD_B) || (accept && p->selection == s->bagCount)) { PplayMenuBack(p); return 1; }
        if (!accept && (keys & PBRB_PAD_SELECT)) { PplayItemSwap(p, s->bag, &s->bagCount); return 1; }
        if (accept) {
            p->itemSwap = -1; p->itemSlot = p->selection; p->menuCursor = p->selection; p->menuItem = s->bag[p->selection].id; p->selection = 0;
            if (p->scene == PBRB_PLAY_BATTLE)PplayUseItem(p); else p->overlay = PBRB_OVERLAY_ITEM_ACTION;
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_ITEM_ACTION) {
        int accept = PplayListInput(p, keys, 3);
        if ((keys & PBRB_PAD_B) || (accept && p->selection == 2)) { p->overlay = PBRB_OVERLAY_ITEM; p->selection = p->menuCursor; return 1; }
        if (accept) {
            if (!p->selection)PplayUseItem(p);
            else {
                if (PbrbGameIsKeyItem(&rom, (unsigned)p->menuItem)) { PplayMenuText(p, PBRB_GT_TooImportantToTossText, PBRB_OVERLAY_ITEM); return 1; }
                p->menuCount = 1; p->menuMode = 4; p->overlay = PBRB_OVERLAY_QUANTITY;
            }
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_ITEM_MOVES || overlay == PBRB_OVERLAY_LEARN) {
        PbrbPlayMon* m = PplayMenuMon(p, (unsigned)(overlay == PBRB_OVERLAY_LEARN ? p->pendingParty : p->menuTarget));
        unsigned n = 0; if (m)while (n < 4 && m->moves[n])++n;
        int accept = PplayListInput(p, keys, n);
        if (keys & PBRB_PAD_B) { if (overlay == PBRB_OVERLAY_LEARN)PplayLearnFinish(p, -1); else { p->overlay = PBRB_OVERLAY_ITEM_TARGET; p->selection = p->menuTarget; }return 1; }
        if (accept && n) { if (overlay == PBRB_OVERLAY_LEARN)PplayLearnFinish(p, p->selection); else PplayUseItemOnMon(p, (unsigned)p->menuTarget, (unsigned)p->selection); }return 1;
    }
    if (overlay == PBRB_OVERLAY_MOVES) {
        unsigned n = 0; while (n < 4 && p->starter.moves[n])++n;
        int accept = PplayListInput(p, keys, n);
        if (keys & PBRB_PAD_B) { p->moveSwap = -1; p->overlay = PBRB_OVERLAY_NONE; return 1; }
        if (!accept && (keys & PBRB_PAD_SELECT) && n) {
            if (p->moveSwap < 0 || (unsigned)p->moveSwap >= n)p->moveSwap = p->selection;
            else {
                if (PbrbBattleSwapMoves(&p->battle, (unsigned)p->moveSwap, (unsigned)p->selection)) {
                    unsigned active = p->battle.active[0]; p->gameState.party[active] = p->battle.party[0][active];
                    p->starter = p->battle.side[0].mon;
                }p->moveSwap = -1;
            }
            return 1;
        }
        if (accept && n) {
            unsigned usable = 0; for (unsigned i = 0; i < n; ++i)usable |= p->starter.pp[i] & 63u;
            if (usable && !(p->starter.pp[p->selection] & 63u)) { PplayMenuText(p, PBRB_GT_MoveNoPPText, PBRB_OVERLAY_MOVES); return 1; }
            p->moveSwap = -1;
            PplayBattleSubmit(p, PBRB_BATTLE_MOVE, (unsigned)p->selection, 0, 0, 0);
        }return 1;
    }
    if (overlay == PBRB_OVERLAY_MIMIC) {
        unsigned n = 0; while (n < 4 && p->battle.side[1].mon.moves[n])++n;
        int accept = PplayListInput(p, keys, n);
        /* The turn has already committed and Mimic passed its hit check.
           The original chooser watches A/Up/Down only; B cannot cancel. */
        if (accept && n)PplayBattleSubmit(p, PBRB_BATTLE_CHOOSE_MIMIC, (unsigned)p->selection, 0, 0, 0); return 1;
    }
    if (overlay == PBRB_OVERLAY_TRAINER) { if (keys & (PBRB_PAD_A | PBRB_PAD_B | PBRB_PAD_START))PplayMenuBack(p); return 1; }
    if (overlay == PBRB_OVERLAY_TOWN_MAP && p->menuMode) {
        int fly = p->menuMode == 2;
        if (fly && (keys & (PBRB_PAD_UP | PBRB_PAD_DOWN))) {
            int delta = (keys & PBRB_PAD_UP) ? 1 : 10;
            for (int tries = 0; tries < 11; ++tries) {
                p->selection = (p->selection + delta) % 11;
                if (p->world->visitedTowns & (1u << p->selection))break;
            }
            PbrbAudioPlaySfx(PBRB_SFX_TINK);
        }
        else if (!fly) {
            if (keys & PBRB_PAD_UP)p->selection = p->selection < 0 ? 1 : (p->selection + 1) % 47;
            if (keys & PBRB_PAD_DOWN)p->selection = p->selection < 0 ? 46 : (p->selection + 46) % 47;
        }
        if (keys & PBRB_PAD_B) { p->overlay = fly ? PBRB_OVERLAY_PARTY : PBRB_OVERLAY_ITEM; p->selection = fly ? p->menuTarget : 0; p->menuMode = 0; return 1; }
        if (keys & PBRB_PAD_A) {
            if (fly) { if (!PbrbWorldFlyTo(p->world, (unsigned)p->selection))return 1; p->overlay = PBRB_OVERLAY_NONE; }
            else { p->overlay = PBRB_OVERLAY_ITEM; p->selection = 0; }p->menuMode = 0;
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_SHOP) {
        int accept = PplayListInput(p, keys, 3);
        if ((keys & PBRB_PAD_B) || (accept && p->selection == 2)) { p->overlay = PBRB_OVERLAY_NONE; p->text.visible = 0; PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0; return 1; }
        if (accept) { p->menuMode = p->selection; p->selection = 0; p->itemSwap = p->itemSlot = -1; p->overlay = PBRB_OVERLAY_SHOP_LIST; }return 1;
    }
    if (overlay == PBRB_OVERLAY_SHOP_LIST) {
        const PbrbWorldEvent* e = PbrbWorldPendingEvent(p->world); unsigned count = p->menuMode == 0 ? (e ? e->dataCount : 0) : s->bagCount;
        int accept = PplayListInput(p, keys, count + 1);
        if ((keys & PBRB_PAD_B) || (accept && (unsigned)p->selection == count)) { p->itemSwap = -1; p->overlay = PBRB_OVERLAY_SHOP; p->selection = 0; return 1; }
        if (!accept && p->menuMode == 1 && (keys & PBRB_PAD_SELECT)) { PplayItemSwap(p, s->bag, &s->bagCount); return 1; }
        if (accept) {
            unsigned item = p->menuMode == 0 ? p->gb->rom[e->dataOffset + (unsigned)p->selection] : s->bag[p->selection].id;
            p->itemSwap = -1; p->itemSlot = p->menuMode == 1 ? p->selection : -1;
            p->menuItem = (int)item; p->menuTarget = p->selection; PplayMenuName(p, item);
            if (p->menuMode == 1 && PbrbGameIsKeyItem(&rom, item)) { PplayMenuText(p, PBRB_GT_PokemartUnsellableItemText, PBRB_OVERLAY_SHOP_LIST); return 1; }
            p->menuCount = 1; p->overlay = PBRB_OVERLAY_QUANTITY;
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_PC_MAIN) {
        unsigned last = 2u + (s->pokedex ? 1u : 0u) + (s->hallOfFameCount ? 1u : 0u);
        int accept = PplayListInput(p, keys, last + 1u);
        if ((keys & PBRB_PAD_B) || (accept && (unsigned)p->selection == last)) { PplayClosePC(p); return 1; }
        if (accept) {
            if (p->selection == 0) { p->overlay = PBRB_OVERLAY_PC; p->menuReturn = 1; }
            else if (p->selection == 1) { p->overlay = PBRB_OVERLAY_PC; p->menuReturn = 0; }
            else {
                PbrbWorldEvent event; memset(&event, 0, sizeof(event)); event.flags = 0x80000000u;
                event.kind = p->selection == 2 && s->pokedex ? PBRB_WORLD_DEX_RATING : PBRB_WORLD_HALL_OF_FAME; event.value = 1;
                PbrbPlayServiceBegin(p, &event); return 1;
            }p->selection = 0;
        }return 1;
    }
    if (overlay == PBRB_OVERLAY_PC) {
        unsigned count = p->menuReturn ? 5u : 4u; int accept = PplayListInput(p, keys, count);
        if ((keys & PBRB_PAD_B) || (accept && (unsigned)p->selection == count - 1u)) {
            if (p->world->mapId == PBRB_MAP_BEDROOM)PplayClosePC(p); else { p->overlay = PBRB_OVERLAY_PC_MAIN; p->selection = 0; }return 1;
        }
        if (accept) {
            if (p->menuReturn) {
                if (p->selection == 3) { p->overlay = PBRB_OVERLAY_NONE; PplayGameText(p, PBRB_GT_WhenYouChangeBoxText, 0, PPLAY_AFTER_CHANGE_BOX); return 1; }
                p->menuMode = p->selection; p->selection = 0; p->overlay = PBRB_OVERLAY_PC_MONS;
            }
            else { p->menuMode = p->selection; p->selection = 0; p->itemSwap = p->itemSlot = -1; p->overlay = PBRB_OVERLAY_PC_ITEMS; }
        }return 1;
    }
    if (overlay == PBRB_OVERLAY_PC_MONS) {
        unsigned count = p->menuMode == 1 ? s->partyCount : s->boxCount[s->currentBox]; int accept = PplayListInput(p, keys, count + 1u);
        if ((keys & PBRB_PAD_B) || (accept && (unsigned)p->selection == count)) { p->overlay = PBRB_OVERLAY_PC; p->selection = 0; return 1; }
        if (accept) {
            p->menuTarget = p->selection;
            if (p->menuMode == 2) {
                p->question = PPLAY_QUESTION_RELEASE; p->selection = 1;
                p->overlay = PBRB_OVERLAY_YESNO;
            }
            else { p->selection = 0; p->overlay = PBRB_OVERLAY_PC_MON_ACTION; }
        }return 1;
    }
    if (overlay == PBRB_OVERLAY_PC_MON_ACTION) {
        int accept = PplayListInput(p, keys, 3);
        if ((keys & PBRB_PAD_B) || (accept && p->selection == 2)) { p->overlay = PBRB_OVERLAY_PC; p->selection = 0; return 1; }
        if (accept) {
            if (!p->selection)PplayPCTransferMon(p, (unsigned)p->menuTarget);
            else {
                unsigned count = p->menuMode == 1 ? s->partyCount : s->boxCount[s->currentBox];
                if (p->menuTarget >= 0 && (unsigned)p->menuTarget < count) {
                    PbrbPlayMon* mon = p->menuMode == 1 ? s->party + p->menuTarget : s->boxes[s->currentBox] + p->menuTarget;
                    p->menuPage = 0; p->overlay = PBRB_OVERLAY_PC_MON_STATUS; PbrbAudioPlayCry(mon->species);
                }
            }
        }return 1;
    }
    if (overlay == PBRB_OVERLAY_PC_ITEMS) {
        PbrbGameItem* items = p->menuMode == 1 ? s->bag : s->pcItems; unsigned char* storedCount = p->menuMode == 1 ? &s->bagCount : &s->pcItemCount;
        unsigned count = *storedCount;
        int accept = PplayListInput(p, keys, count + 1u);
        if ((keys & PBRB_PAD_B) || (accept && (unsigned)p->selection == count)) { p->itemSwap = -1; p->overlay = PBRB_OVERLAY_PC; p->selection = 0; return 1; }
        if (!accept && (keys & PBRB_PAD_SELECT)) { PplayItemSwap(p, items, storedCount); return 1; }
        if (accept) {
            p->itemSwap = -1; p->itemSlot = p->selection; p->menuTarget = p->selection; p->menuItem = items[p->selection].id; p->menuCount = 1;
            int key = PbrbGameIsKeyItem(&rom, (unsigned)p->menuItem);
            if (p->menuMode == 2 && key) { PplayMenuText(p, PBRB_GT_TooImportantToTossText, PBRB_OVERLAY_PC_ITEMS); return 1; }
            p->menuMode = p->menuMode == 0 ? 2 : p->menuMode == 1 ? 3 : 5;
            if (key)PplayTransferItems(p); else p->overlay = PBRB_OVERLAY_QUANTITY;
        }return 1;
    }
    if (overlay == PBRB_OVERLAY_BOX) {
        int accept = PplayListInput(p, keys, 12);
        if (keys & PBRB_PAD_B) { p->overlay = PBRB_OVERLAY_PC; p->selection = 0; return 1; }
        if (accept) {
            unsigned old = s->currentBox; s->currentBox = (unsigned char)p->selection;
            if (!PplaySaveCheckpoint(p)) { s->currentBox = (unsigned char)old; PplayNotice(p, "Save failed.", "Please try again.", PBRB_OVERLAY_BOX); return 1; }
            p->selection = 0; p->overlay = PBRB_OVERLAY_PC; PbrbAudioPlaySfx(PBRB_SFX_SAVE);
        }return 1;
    }
    if (overlay == PBRB_OVERLAY_QUANTITY) {
        unsigned max = PplayOperationQuantity(p);
        if (!max) { PplayReturnItemList(p); return 1; }
        if (keys & PBRB_PAD_UP)++p->menuCount; if (keys & PBRB_PAD_DOWN)--p->menuCount;
        if (keys & PBRB_PAD_RIGHT)p->menuCount += 10; if (keys & PBRB_PAD_LEFT)p->menuCount -= 10;
        if (p->menuCount < 1)p->menuCount = (int)(max ? max : 1); if ((unsigned)p->menuCount > max)p->menuCount = 1;
        if (keys & PBRB_PAD_B) { PplayReturnItemList(p); return 1; }
        if (keys & PBRB_PAD_A) { if (p->menuMode == 2 || p->menuMode == 3)PplayTransferItems(p); else PplayConfirmItemOperation(p); }return 1;
    }
    if (overlay == PBRB_OVERLAY_MENU_CONFIRM) {
        if (p->text.active)return 1;
        if (keys & (PBRB_PAD_UP | PBRB_PAD_DOWN))p->selection ^= 1;
        if (keys & PBRB_PAD_B)PplayCommitItemOperation(p, 0);
        else if (keys & PBRB_PAD_A)PplayCommitItemOperation(p, p->selection == 0);
        return 1;
    }
    if (overlay == PBRB_OVERLAY_EVOLUTION) { PplayEvolutionTick(p, keys); return 1; }
    return 0;
}
static void PplayWorldPlayer(PbrbPlay* p) {
    PbrbWorldPlayerState state; memset(&state, 0, sizeof(state)); PbrbGameState* s = &p->gameState;
    state.badges = s->badges; state.pokedex = s->pokedex; state.partyCount = s->partyCount;
    state.money = s->money;
    state.leadLevel = s->partyCount ? s->party[0].level : 0; state.ownedCount = PbrbGameDexCount(s->owned);
    state.repelSteps = s->repelSteps; state.safariBalls = s->safariBalls; state.surfing = p->world->travelMode == 2; state.bicycle = p->world->travelMode == 1;
    s->surfing = (unsigned char)state.surfing; s->bicycle = (unsigned char)state.bicycle;
    for (unsigned i = 0; i < s->partyCount; ++i) {
        if (s->party[i].hp)++state.usableCount;
        for (unsigned j = 0; j < 4; ++j)state.fieldMoves |= PplayFieldMoveFor(s->party[i].moves[j]);
    }
    for (unsigned i = 0; i < s->bagCount; ++i)state.keyItems[s->bag[i].id / 8] |= (unsigned char)(1u << (s->bag[i].id % 8));
    PbrbWorldSetPlayerState(p->world, &state);
}
static void PplayWorldStep(PbrbPlay* p) {
    unsigned steps = (unsigned)p->world->steps; if (steps == p->lastWorldSteps)return; p->lastWorldSteps = steps;
    PbrbGameState* s = &p->gameState; ++s->steps;
    if (s->daycareOccupied) {
        ++s->daycareSteps; ++s->daycare.experience;
        if (s->daycare.experience > 0x50ffffu)s->daycare.experience = 0x500000u;
    }
    if (PbrbWorldInGrass(p->world))PbrbInputRumble(0x0400, 0x0700, 28);
    if (s->repelSteps) { --s->repelSteps; if (!s->repelSteps && !PbrbWorldPendingEvent(p->world))PplayMenuText(p, PBRB_GT_RepelWoreOffText, PBRB_OVERLAY_NONE); }
    if ((s->steps & 3u) == 0) {
        int poisoned = 0; unsigned alive = 0;
        for (unsigned i = 0; i < s->partyCount; ++i) {
            PbrbPlayMon* m = s->party + i;
            if (m->hp && (m->status & 8u)) { --m->hp; poisoned = 1; if (!m->hp)m->status = 0; }
            if (m->hp)++alive;
        }
        if (poisoned) { PbrbAudioPlaySfx(PBRB_SFX_POISONED); PbrbInputRumble(0x0800, 0x1000, 55); PplaySyncAliases(p); }
        if (poisoned && !alive) {
            s->money /= 2; p->money = s->money; PbrbGameRom rom = PplayRom(p);
            for (unsigned i = 0; i < s->partyCount; ++i)PbrbGameHeal(&rom, s->party + i); PplaySyncAliases(p);
            PplayGameText(p, PBRB_GT_PlayerBlackedOutText2, 0, PPLAY_AFTER_WORLD_BLACKOUT);
        }
    }
}
static void PplayMenuIcon(PbrbPlay* p, const PbrbPlayMon* m, int x, int y, int animated) {
    PbrbGameRom rom = PplayRom(p); unsigned dex = PbrbGameDex(&rom, m->species); if (!dex)return;
    unsigned packed = p->gb->rom[0x7190du + (dex - 1u) / 2u]; unsigned icon = (dex & 1u) ? packed >> 4 : packed & 15u;
    unsigned speed = m->maxHP && m->hp * 5u <= m->maxHP ? 33u : m->maxHP && m->hp * 2u <= m->maxHP ? 17u : 6u;
    unsigned frame = animated && m->hp ? ((p->uiFrames / speed) & 1u) : 0u;
    unsigned base = icon * 4u + ((icon == 1 || icon == 2) ? 0u : frame * 64u);
    if ((icon == 1 || icon == 2) && frame)++y;
    for (unsigned row = 0; row < 2; ++row)for (unsigned column = 0; column < 2; ++column) {
        unsigned tile = base + row * 2u + (icon == 2 ? column : 0u), offset = 0;
        for (unsigned i = 0; i < 28; ++i) {
            const unsigned char* d = p->gb->rom + 0x717c0u + i * 6u;
            unsigned dest = ((unsigned)d[4] | ((unsigned)d[5] << 8)) - 0x8000u; dest /= 16u;
            if (tile >= dest && tile < dest + d[2]) {
                unsigned address = (unsigned)d[0] | ((unsigned)d[1] << 8);
                offset = (unsigned)d[3] * 0x4000u + address - 0x4000u + (tile - dest) * 16u; break;
            }
        }
        if (offset)PbrbGbSpriteTile(p->gb, offset, x + (int)column * 8, y + (int)row * 8, 0xe4, icon != 2 && column ? 0x20u : 0u);
    }
}
static const char* PplayStatus(const PbrbPlayMon* m) {
    if (!m->hp)return "FNT"; if (m->status & 7)return "SLP"; if (m->status & 8)return "PSN";
    if (m->status & 16)return "BRN"; if (m->status & 32)return "FRZ"; if (m->status & 64)return "PAR"; return "";
}
static void PplayMenuPartyRender(PbrbPlay* p) {
    PbrbGb* g = p->gb; PbrbGbClear(g, 0);
    for (unsigned i = 0; i < p->gameState.partyCount; ++i) {
        PbrbPlayMon* m = PplayMenuMon(p, i); int y = (int)i * 16;
        PplayMenuIcon(p, m, 8, y, (int)i == p->selection); PbrbGbCodes(g, m->nickname, 24, y, 10);
        PplayHudTile(p, 0x6e, 104, y); PplayNumber(g, m->level, 112, y, 2); PbrbGbString(g, PplayStatus(m), 136, y);
        PplayHealth(p, m, 32, y + 8, 0); PplayNumber(g, m->hp, 104, y + 8, 3); PbrbGbGlyph(g, 0xf3, 128, y + 8); PplayNumber(g, m->maxHP, 136, y + 8, 3);
    }
    PbrbGbGlyph(g, 0xed, 0, p->selection * 16); if (p->menuSwap >= 0)PbrbGbGlyph(g, 0xec, 0, p->menuSwap * 16);
    PbrbGbTextBox(g, 0, 96, 20, 6);
    int usingItem = p->overlay == PBRB_OVERLAY_ITEM_TARGET || p->overlay == PBRB_OVERLAY_SOFTBOILED;
    PbrbGbString(g, usingItem ? "Use on which" : "Choose a POK", 8, 112);
    if (usingItem) { PbrbGbString(g, "POK", 8, 128); PbrbGbGlyph(g, 0xba, 32, 128); PbrbGbString(g, "MON?", 40, 128); }
    else { PbrbGbGlyph(g, 0xba, 104, 112); PbrbGbString(g, "MON.", 112, 112); }
}
static void PplaySimpleMenu(PbrbPlay* p, const char* const* labels, unsigned count, int x, int y, int width) {
    PbrbGbTextBox(p->gb, x, y, width, (int)count * 2 + 2);
    for (unsigned i = 0; i < count; ++i)PbrbGbString(p->gb, labels[i], x + 16, y + 16 + (int)i * 16);
    PbrbGbGlyph(p->gb, 0xed, x + 8, y + 16 + p->selection * 16);
}
static void PplayItemListRender(PbrbPlay* p, const PbrbGameItem* items, unsigned count, int prices) {
    PbrbGb* g = p->gb; PbrbGameRom rom = PplayRom(p); PbrbGbTextBox(g, 16, 8, 18, 16);
    unsigned top = p->selection >= 7 ? (unsigned)p->selection - 6u : 0u;
    for (unsigned i = 0; i < 7 && top + i <= count; ++i) {
        unsigned row = top + i; int y = 24 + (int)i * 16;
        if (row == count) { PbrbGbString(g, "CANCEL", 32, y); continue; }
        unsigned char name[16]; PbrbGameItemName(&rom, items[row].id, name); PbrbGbCodes(g, name, 32, y, 13);
        if (prices) { PbrbGbGlyph(g, 0xf0, 104, y + 8); PplayNumber(g, PbrbGameItemPrice(&rom, items[row].id) / (prices == 2 ? 2u : 1u), 112, y + 8, 4); }
        else { PbrbGbGlyph(g, 0xf1, 128, y); PplayNumber(g, items[row].count, 136, y, 2); }
    }
    if (p->itemSwap >= 0 && (unsigned)p->itemSwap >= top && (unsigned)p->itemSwap < top + 7u && (unsigned)p->itemSwap < count)
        PbrbGbGlyph(g, 0xec, 24, 24 + (p->itemSwap - (int)top) * 16);
    PbrbGbGlyph(g, 0xed, 24, 24 + (p->selection - (int)top) * 16);
}
static int PplayGameplayMenuRender(PbrbPlay* p) {
    if (PplayDexMenuRender(p))return 1;
    PbrbGb* g = p->gb; PbrbGameState* s = &p->gameState; PbrbGameRom rom = PplayRom(p); int overlay = p->overlay;
    if (overlay == PBRB_OVERLAY_START) {
        const char* labels[7]; unsigned n = 0; if (s->pokedex)labels[n++] = "POKEDEX";
        labels[n++] = "POKEMON"; labels[n++] = "ITEM"; labels[n++] = ""; labels[n++] = "SAVE"; labels[n++] = "OPTION"; labels[n++] = "EXIT";
        PplaySimpleMenu(p, labels, n, 80, 0, 10); int dex = s->pokedex ? 1 : 0;
        if (dex) { PbrbGbString(g, "POK", 96, 16); PbrbGbGlyph(g, 0xba, 120, 16); PbrbGbString(g, "DEX", 128, 16); }
        PbrbGbString(g, "POK", 96, 16 + dex * 16); PbrbGbGlyph(g, 0xba, 120, 16 + dex * 16); PbrbGbString(g, "MON", 128, 16 + dex * 16);
        PbrbGbCodes(g, p->player, 96, 48 + dex * 16, 7); return 1;
    }
    if (overlay == PBRB_OVERLAY_PARTY || overlay == PBRB_OVERLAY_ITEM_TARGET || overlay == PBRB_OVERLAY_SOFTBOILED) { PplayMenuPartyRender(p); return 1; }
    if (overlay == PBRB_OVERLAY_PARTY_ACTION) {
        int selection = p->selection; p->selection = p->menuTarget; PplayMenuPartyRender(p); p->selection = selection;
        unsigned fields[4], count = p->scene == PBRB_PLAY_BATTLE ? 0u : PplayPartyFieldMoves(s->party + p->menuTarget, fields);
        int left = 12; for (unsigned i = 0; i < count; ++i) { int x = fields[i] == 135 ? 8 : (fields[i] == 70 || fields[i] == 100) ? 10 : 12; if (x < left)left = x; }
        int top = count ? 10 - (int)count * 2 : 11, textY = count ? 12 - (int)count * 2 : 12;
        PbrbGbTextBox(g, (left - 1) * 8, top * 8, 21 - left, count ? 8 + (int)count * 2 : 7);
        for (unsigned i = 0; i < count; ++i) { unsigned char name[16]; PbrbGameMoveName(&rom, fields[i], name); PbrbGbCodes(g, name, (left + 1) * 8, (textY + (int)i * 2) * 8, 11); }
        const char* first = p->scene == PBRB_PLAY_BATTLE ? "SWITCH" : "STATS"; const char* second = p->scene == PBRB_PLAY_BATTLE ? "STATS" : "SWITCH";
        PbrbGbString(g, first, (left + 1) * 8, (textY + (int)count * 2) * 8);
        PbrbGbString(g, second, (left + 1) * 8, (textY + (int)count * 2 + 2) * 8);
        PbrbGbString(g, "CANCEL", (left + 1) * 8, (textY + (int)count * 2 + 4) * 8);
        PbrbGbGlyph(g, 0xed, left * 8, (textY + p->selection * 2) * 8); return 1;
    }
    if (overlay == PBRB_OVERLAY_TOWN_MAP && p->menuMode == 2) {
        if (!PplayUIMapBackground(g))return 1;
        unsigned coords = 0, name = 0, playerCoords = 0, playerName = 0;
        if (!PplayUIMapEntry(g, (unsigned)p->selection, &coords, &name) ||
            !PplayUIMapEntry(g, (unsigned)p->world->romMapId, &playerCoords, &playerName))return 1;
        for (int x = 0; x < 20; ++x)PbrbGbGlyph(g, 0x7f, x * 8, 0);
        PbrbGbString(g, "To", 0, 0); PplayUIString(g, name, 24, 0, 0, 15);
        PplayUIMapSprite(g, PPLAY_UI_RED_SPRITE, 0, playerCoords); PplayUIMapSprite(g, 0x14d80u, 0, coords);
        PbrbGbTile(g, 0x71093u, 1, 144, 0); PbrbGbGlyph(g, 0xee, 152, 0); return 1;
    }
    if (overlay == PBRB_OVERLAY_STATUS || overlay == PBRB_OVERLAY_PC_MON_STATUS) {
        PbrbPlayMon* m = 0;
        if (overlay == PBRB_OVERLAY_PC_MON_STATUS) {
            unsigned count = p->menuMode == 1 ? s->partyCount : s->boxCount[s->currentBox];
            if (p->menuTarget >= 0 && (unsigned)p->menuTarget < count)m = p->menuMode == 1 ? s->party + p->menuTarget : s->boxes[s->currentBox] + p->menuTarget;
        }
        else m = PplayMenuMon(p, (unsigned)p->menuTarget);
        if (!m)return 1; PbrbGbClear(g, 0);
        unsigned char picture[56 * 56]; PbrbGbDecodePokemon(g, m->species, 0, picture, sizeof(picture)); PbrbGbBitmap(g, picture, 56, 56, 8, 0, 0, 1);
        /* status_screen.asm's two line boxes retain their original tile
           coordinates. Its vertical tile is the first BattleHudTiles2 tile. */
        for (int row = 1; row < 7; ++row)PplayHudTile(p, 0x73, 152, row * 8);
        PplayHudTile(p, 0x77, 152, 56); for (int x = 72; x < 152; x += 8)PplayHudTile(p, 0x76, x, 56); PplayHudTile(p, 0x6f, 64, 56);
        PbrbGbTile(g, PPLAY_UI_BATTLE_FONT + (0x74u - 0x62u) * 16u, 2, 8, 56); PbrbGbGlyph(g, 0xf2, 16, 56);
        PplayUINumber(g, PbrbGameDex(&rom, m->species), 3, 24, 56, 1);
        if (!p->menuPage) {
            PbrbGbCodes(g, m->nickname, 72, 8, 10); PplayHudTile(p, 0x6e, 112, 16); PplayNumber(g, m->level, 120, 16, m->level == 100 ? 3 : 2);
            PplayHealth(p, m, 88, 24, 1); PplayNumber(g, m->hp, 96, 32, 3); PbrbGbGlyph(g, 0xf3, 120, 32); PplayNumber(g, m->maxHP, 128, 32, 3);
            PbrbGbString(g, "STATUS/", 72, 48); const char* status = PplayStatus(m); PbrbGbString(g, status[0] ? status : "OK", 128, 48);
            for (int row = 9; row < 17; ++row)PplayHudTile(p, 0x73, 152, row * 8);
            PplayHudTile(p, 0x77, 152, 136); for (int x = 104; x < 152; x += 8)PplayHudTile(p, 0x76, x, 136); PplayHudTile(p, 0x6f, 96, 136);
            PbrbGbString(g, "TYPE1/", 80, 72);
            PbrbGameBase base; if (PbrbGameBaseData(&rom, m->species, &base)) {
                PplayTypeName(p, base.types[0], 88, 80);
                if (base.types[1] != base.types[0]) { PbrbGbString(g, "TYPE2/", 80, 88); PplayTypeName(p, base.types[1], 88, 96); }
            }
            PbrbGbTile(g, PPLAY_UI_BATTLE_FONT + (0x73u - 0x62u) * 16u, 2, 80, 104);
            PbrbGbTile(g, PPLAY_UI_BATTLE_FONT + (0x74u - 0x62u) * 16u, 2, 88, 104); PbrbGbGlyph(g, 0xf3, 96, 104);
            PplayUINumber(g, m->ot, 5, 96, 112, 1); PbrbGbString(g, "OT/", 80, 120);
            if (m->originalTrainer[0] == 0x5d)PbrbGbString(g, "TRAINER", 96, 128); else PbrbGbCodes(g, m->originalTrainer, 96, 128, 7);
            PbrbGbTextBox(g, 0, 64, 10, 10); static const char* names[] = { "ATTACK","DEFENSE","SPEED","SPECIAL" }; unsigned values[] = { m->attack,m->defense,m->speed,m->special };
            for (unsigned i = 0; i < 4; ++i) { PbrbGbString(g, names[i], 8, 72 + (int)i * 16); PplayNumber(g, values[i], 48, 80 + (int)i * 16, 3); }
        }
        else {
            unsigned char speciesName[11]; PbrbGameMonName(&rom, m->species, speciesName); PbrbGbCodes(g, speciesName, 72, 8, 10);
            PbrbGbString(g, "EXP POINTS", 72, 24); PplayNumber(g, m->experience, 96, 32, 7); PbrbGbString(g, "LEVEL UP", 72, 40);
            unsigned needed = m->level < 100 ? (PbrbGameExperience(&rom, m->species, m->level + 1u) - m->experience) & 0xffffffu : 0u;
            PplayNumber(g, needed, 56, 48, 7); PplayHudTile(p, 0x70, 112, 48);
            unsigned level = m->level < 100 ? m->level + 1u : 100u; PplayHudTile(p, 0x6e, 128, 48); PplayNumber(g, level, 136, 48, level == 100 ? 3 : 2);
            PbrbGbTextBox(g, 0, 64, 20, 10);
            for (unsigned i = 0; i < 4; ++i) {
                int y = 72 + (int)i * 16; unsigned char name[16]; PbrbGameMoveName(&rom, m->moves[i], name);
                if (m->moves[i]) {
                    PbrbGbCodes(g, name, 16, y, 13); PbrbGbTile(g, 0x12adcu, 1, 88, y + 8); PbrbGbTile(g, 0x12adcu, 1, 96, y + 8);
                    PplayNumber(g, m->pp[i] & 63u, 112, y + 8, 2); PbrbGbGlyph(g, 0xf3, 128, y + 8); PplayNumber(g, PbrbGameMaxPP(&rom, m->moves[i], m->pp[i]), 136, y + 8, 2);
                }
                else { PbrbGbString(g, "-", 16, y); PbrbGbString(g, "--", 88, y + 8); }
            }
        }return 1;
    }
    if (overlay == PBRB_OVERLAY_FIELD_MOVE) {
        PbrbGbTextBox(g, 16, 32, 18, 12); PbrbPlayMon* m = s->party + p->menuTarget; unsigned n = 0;
        for (unsigned i = 0; i < 4; ++i)if (PplayFieldMoveFor(m->moves[i])) { unsigned char name[16]; PbrbGameMoveName(&rom, m->moves[i], name); PbrbGbCodes(g, name, 32, 48 + (int)n * 16, 14); ++n; }
        if (!n)PbrbGbString(g, "No field moves.", 24, 48); else PbrbGbGlyph(g, 0xed, 24, 48 + p->selection * 16); return 1;
    }
    if (overlay == PBRB_OVERLAY_ITEM) { PplayItemListRender(p, s->bag, s->bagCount, 0); return 1; }
    if (overlay == PBRB_OVERLAY_ITEM_ACTION) { PplayItemListRender(p, s->bag, s->bagCount, 0); static const char* labels[] = { "USE","TOSS","CANCEL" }; PplaySimpleMenu(p, labels, 3, 104, 72, 7); return 1; }
    if (overlay == PBRB_OVERLAY_ITEM_MOVES || overlay == PBRB_OVERLAY_LEARN) {
        PbrbPlayMon* m = PplayMenuMon(p, (unsigned)(overlay == PBRB_OVERLAY_LEARN ? p->pendingParty : p->menuTarget)); if (!m)return 1;
        PbrbGbTextBox(g, 16, 32, 18, 12); for (unsigned i = 0; i < 4; ++i) { unsigned char name[16]; PbrbGameMoveName(&rom, m->moves[i], name); PbrbGbCodes(g, name, 32, 48 + (int)i * 16, 14); }
        PbrbGbGlyph(g, 0xed, 24, 48 + p->selection * 16); return 1;
    }
    if (overlay == PBRB_OVERLAY_MIMIC) {
        PbrbGbTextBox(g, 0, 56, 16, 6);
        for (unsigned i = 0; i < 4 && p->battle.side[1].mon.moves[i]; ++i) { unsigned char name[16]; PbrbGameMoveName(&rom, p->battle.side[1].mon.moves[i], name); PbrbGbCodes(g, name, 16, 64 + (int)i * 8, 13); }
        PbrbGbGlyph(g, 0xed, 8, 64 + p->selection * 8);
        PbrbGbTextBox(g, 0, 96, 20, 6); PbrbGbString(g, "WHICH TECHNIQUE?", 8, 112); return 1;
    }
    if (overlay == PBRB_OVERLAY_SHOP) { static const char* labels[] = { "BUY","SELL","QUIT" }; PplaySimpleMenu(p, labels, 3, 0, 0, 8); return 1; }
    if (overlay == PBRB_OVERLAY_SHOP_LIST) {
        if (p->menuMode)PplayItemListRender(p, s->bag, s->bagCount, 2);
        else {
            PbrbGameItem items[32]; const PbrbWorldEvent* e = PbrbWorldPendingEvent(p->world); unsigned count = e ? e->dataCount : 0; if (count > 32)count = 32;
            for (unsigned i = 0; i < count; ++i) { items[i].id = g->rom[e->dataOffset + i]; items[i].count = 1; }PplayItemListRender(p, items, count, 1);
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_QUANTITY) {
        PbrbGbTextBox(g, 0, 0, 13, 5); PbrbGbString(g, "MONEY", 8, 8); PbrbGbGlyph(g, 0xf0, 24, 24); PplayNumber(g, s->money, 32, 24, 6);
        PbrbGbTextBox(g, 16, 56, 18, 8); unsigned char name[16]; PbrbGameItemName(&rom, (unsigned)p->menuItem, name); PbrbGbCodes(g, name, 24, 72, 15);
        PbrbGbGlyph(g, 0xf1, 32, 96); PplayNumber(g, (unsigned)p->menuCount, 40, 96, 2);
        if (p->menuMode < 2) { PbrbGbGlyph(g, 0xf0, 88, 96); PplayNumber(g, PbrbGameItemPrice(&rom, (unsigned)p->menuItem) / (p->menuMode ? 2u : 1u) * (unsigned)p->menuCount, 96, 96, 6); }return 1;
    }
    if (overlay == PBRB_OVERLAY_MENU_CONFIRM) {
        int selection = p->selection;
        p->selection = p->menuMode == 4 ? p->menuCursor : p->menuTarget;
        if (p->menuMode == 0) {
            PbrbGameItem items[32]; const PbrbWorldEvent* e = PbrbWorldPendingEvent(p->world); unsigned count = e ? e->dataCount : 0; if (count > 32)count = 32;
            for (unsigned i = 0; i < count; ++i) { items[i].id = g->rom[e->dataOffset + i]; items[i].count = 1; }PplayItemListRender(p, items, count, 1);
        }
        else if (p->menuMode == 5)PplayItemListRender(p, s->pcItems, s->pcItemCount, 0);
        else PplayItemListRender(p, s->bag, s->bagCount, p->menuMode == 1 ? 2 : 0);
        p->selection = selection;
        if (!p->text.active) {
            PbrbGbTextBox(g, 112, 56, 6, 5); PbrbGbString(g, "YES", 128, 64); PbrbGbString(g, "NO", 128, 80);
            PbrbGbGlyph(g, 0xed, 120, 64 + p->selection * 16);
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_PC_MAIN) {
        const char* labels[5]; unsigned n = 0; labels[n++] = PbrbWorldHasMetBill(p->world) ? "BILL's PC" : "Someone's PC";
        labels[n++] = "PLAYER's PC"; if (s->pokedex)labels[n++] = "PROF.OAK's PC"; if (s->hallOfFameCount)labels[n++] = "HALL OF FAME"; labels[n++] = "LOG OFF";
        PplaySimpleMenu(p, labels, n, 0, 0, 19); return 1;
    }
    if (overlay == PBRB_OVERLAY_PC) {
        static const char* mon[] = { "WITHDRAW PKMN","DEPOSIT PKMN","RELEASE PKMN","CHANGE BOX","SEE YA!" };
        static const char* item[] = { "WITHDRAW ITEM","DEPOSIT ITEM","TOSS ITEM","LOG OFF" }; PplaySimpleMenu(p, p->menuReturn ? mon : item, p->menuReturn ? 5u : 4u, 0, 0, 18); return 1;
    }
    if (overlay == PBRB_OVERLAY_PC_MONS || overlay == PBRB_OVERLAY_PC_MON_ACTION) {
        PbrbGbClear(g, 0); PbrbGbTextBox(g, 0, 0, 20, 18); PbrbPlayMon* mons = p->menuMode == 1 ? s->party : s->boxes[s->currentBox];
        int selected = overlay == PBRB_OVERLAY_PC_MON_ACTION ? p->menuTarget : p->selection;
        unsigned count = p->menuMode == 1 ? s->partyCount : s->boxCount[s->currentBox], top = selected >= 7 ? (unsigned)selected - 6u : 0u;
        for (unsigned i = 0; i < 7 && top + i <= count; ++i) {
            int y = 16 + (int)i * 16; unsigned row = top + i; if (row == count) { PbrbGbString(g, "CANCEL", 24, y); continue; }
            PbrbGbCodes(g, mons[row].nickname, 24, y, 10); PplayHudTile(p, 0x6e, 120, y); PplayNumber(g, mons[row].level, 128, y, 3);
        }
        PbrbGbGlyph(g, overlay == PBRB_OVERLAY_PC_MON_ACTION ? 0xec : 0xed, 8, 16 + (selected - (int)top) * 16);
        if (overlay == PBRB_OVERLAY_PC_MON_ACTION) {
            const char* labels[] = { p->menuMode == 1 ? "DEPOSIT" : "WITHDRAW","STATS","CANCEL" };
            PplaySimpleMenu(p, labels, 3, 72, 80, 11);
        }
        return 1;
    }
    if (overlay == PBRB_OVERLAY_PC_ITEMS) { PplayItemListRender(p, p->menuMode == 1 ? s->bag : s->pcItems, p->menuMode == 1 ? s->bagCount : s->pcItemCount, 0); return 1; }
    if (overlay == PBRB_OVERLAY_BOX) {
        PbrbGbTextBox(g, 0, 96, 20, 6); PbrbGbString(g, "Choose a BOX.", 8, 112);
        PbrbGbTextBox(g, 0, 0, 11, 4); PbrbGbString(g, "BOX No.", 8, 16); PplayNumber(g, s->currentBox + 1u, 64, 16, 2);
        PbrbGbTextBox(g, 88, 0, 9, 14);
        for (unsigned i = 0; i < 12; ++i) {
            int y = 8 + (int)i * 8; PbrbGbString(g, "BOX", 104, y); PplayNumber(g, i + 1u, 128, y, 2);
            if (s->boxCount[i])PplayHudTile(p, 0x78, 144, y);
        }
        PbrbGbGlyph(g, 0xed, 96, 8 + p->selection * 8); return 1;
    }
    if (overlay == PBRB_OVERLAY_EVOLUTION) {
        if (p->evolutionPhase == 1) { PbrbBattleAnimRender(&p->battleAnim, g); return 1; }
        PbrbGbClear(g, 0); unsigned char* image = p->evolutionPhase == 2 ? p->enemyPicture : p->picture;
        PbrbGbBitmap(g, image, 56, 56, 56, 16, 0, 1); PbrbGbTextBox(g, 0, 96, 20, 6); return 1;
    }
    return 0;
}
static void PplayHealingMachineRender(PbrbPlay* p) {
    if (p->worldEvent != PBRB_WORLD_HEAL || p->worldSoundPhase < 4)return;
    unsigned palette = 0xe0; if (p->worldSoundPhase == 6 && p->timer > 0 && ((80 - p->timer) / 10) % 2 == 0)palette ^= 0x28;
    unsigned count = 1u + (unsigned)p->menuCursor; if (count > 7)count = 7;
    for (unsigned i = 0; i < count; ++i) {
        const unsigned char* d = p->gb->rom + 0x704d7u + i * 4u;
        PbrbGbSpriteTile(p->gb, 0x704b7u + (d[2] - 0x7cu) * 16u, (int)d[1] - 8, (int)d[0] - 16, palette, d[3]);
    }
}
#include "pbrb_dex_ui.inl"
