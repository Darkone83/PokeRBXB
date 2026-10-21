/* Native quest gates and original puzzle state. All prose, party definitions,
   floors, doors and item IDs are resolved against the verified Red/Blue ROM. */
#include "pbrb_world_puzzles.inl"
enum WorldPuzzleAction {
    WP_CARD_KEY = 1, WP_MANSION, WP_SURGE, WP_ELEVATOR,
    WP_WARDEN, WP_SNORLAX, WP_CINNABAR_LOCK, WP_ELITE_ENTRY, WP_TOWER_HEAL
};
struct WorldGiftRule {
    int map, actor, item, event, before, received, full, after, sound, following;
};
static const WorldGiftRule WORLD_MAIN_GIFTS[] = {
    {0xde,0,0xc6,WE_EVENT_GOT_HM03,WX_SafariZoneSecretHouseFishingGuruYouHaveWonText,
        WX_SafariZoneSecretHouseFishingGuruReceivedHM03Text,WX_SafariZoneSecretHouseFishingGuruHM03NoRoomText,
        WX_SafariZoneSecretHouseFishingGuruHM03ExplanationText,PBRB_SFX_GET_ITEM,-1},
    {0x95,4,0x49,WE_EVENT_GOT_POKE_FLUTE,WX_MrFujisHouseMrFujiIThinkThisMayHelpYourQuestText,
        WX_MrFujisHouseMrFujiReceivedPokeFluteText,WX_MrFujisHouseMrFujiPokeFluteNoRoomText,
        WX_MrFujisHouseMrFujiHasMyFluteHelpedYouText,PBRB_SFX_GET_KEY_ITEM,WX_MrFujisHouseMrFujiPokeFluteExplanationText},
    {0xeb,0,1,WE_EVENT_GOT_MASTER_BALL,WX_SilphCo11FSilphPresidentText,
        WX_SilphCo11FSilphPresidentReceivedMasterBallText,WX_SilphCo11FSilphPresidentNoRoomText,
        WX_SilphCo11FSilphPresidentMasterBallDescriptionText,PBRB_SFX_GET_KEY_ITEM,-1}
};
static int MainSilphFloor(int map) { return (map >= 0xcf && map <= 0xd5) || (map >= 0xe9 && map <= 0xeb); }
static int MainSafariMap(int map) { return map >= 0xd9 && map <= 0xe1; }
static void TraversalBlocks(PbrbWorld* w);
static void TraversalLoaded(PbrbWorld* w);
static int TraversalIdle(PbrbWorld* w);
static int TraversalHidden(PbrbWorld* w, int x, int y);
static int TraversalInteraction(PbrbWorld* w, int actor, unsigned id);
static int TraversalScript(PbrbWorld* w);
static void MainPuzzleBlocks(PbrbWorld* w) {
    TraversalBlocks(w);
    for (unsigned i = 0; i < sizeof(WORLD_SILPH_DOORS) / sizeof(WORLD_SILPH_DOORS[0]); ++i) {
        const WorldDoorRule* d = &WORLD_SILPH_DOORS[i]; if (d->map != w->romMapId)continue;
        ChangeBlock(w, d->x, d->y, PbrbWorldCheckEvent(w, d->event) ? (d->map == 0xeb ? 3 : 14) : d->closed);
    }
    int raw = w->romMapId;
    if (raw == 0x87)ChangeBlock(w, 8, 2, PbrbWorldCheckEvent(w, WE_EVENT_FOUND_ROCKET_HIDEOUT) ? 0x43 : 0x2a);
    if (raw == 0xc7)ChangeBlock(w, 12, 8, (PbrbWorldCheckEvent(w, WE_EVENT_ENTERED_ROCKET_HIDEOUT) || PbrbWorldCheckEvent(w, WE_EVENT_BEAT_ROCKET_HIDEOUT_1_TRAINER_4)) ? 0xe : 0x54);
    if (raw == 0xca) {
        if (!w->restoring && PbrbWorldCheckEvent(w, WE_EVENT_BEAT_ROCKET_HIDEOUT_4_TRAINER_0) &&
            PbrbWorldCheckEvent(w, WE_EVENT_BEAT_ROCKET_HIDEOUT_4_TRAINER_1))
            PbrbWorldSetEvent(w, WE_EVENT_ROCKET_HIDEOUT_4_DOOR_UNLOCKED, 1);
        ChangeBlock(w, 12, 5, PbrbWorldCheckEvent(w, WE_EVENT_ROCKET_HIDEOUT_4_DOOR_UNLOCKED) ? 0xe : 0x2d);
    }
    if (raw == 0x5c)ChangeBlock(w, 2, 2, PbrbWorldCheckEvent(w, WE_EVENT_2ND_LOCK_OPENED) ? 5 : 0x24);
    int on = PbrbWorldCheckEvent(w, WE_EVENT_MANSION_SWITCH_ON);
    if (raw == 0xa5) {
        ChangeBlock(w, 12, 6, on ? 0x2d : 0xe); ChangeBlock(w, 8, 3, on ? 0xe : 0x2d);
        ChangeBlock(w, 10, 8, on ? 0xe : 0x2d); ChangeBlock(w, 13, 13, on ? 0xe : 0x2d);
    }
    if (raw == 0xd6) { ChangeBlock(w, 4, 2, on ? 0x5f : 0xe); ChangeBlock(w, 9, 4, on ? 0xe : 0x54); ChangeBlock(w, 3, 11, on ? 0xe : 0x5f); }
    if (raw == 0xd7) { ChangeBlock(w, 7, 2, on ? 0x5f : 0xe); ChangeBlock(w, 7, 5, on ? 0xe : 0x5f); }
    if (raw == 0xd8) {
        ChangeBlock(w, 13, 8, on ? 0x2d : 0xe); ChangeBlock(w, 6, 11, on ? 0x5f : 0xe);
        ChangeBlock(w, 4, 3, on ? 0xe : 0x5f); ChangeBlock(w, 8, 8, on ? 0xe : 0x54);
    }
    if (raw == 0xf5 || raw == 0xf6 || raw == 0xf7) {
        unsigned ev = raw == 0xf5 ? WE_EVENT_BEAT_LORELEIS_ROOM_TRAINER_0 :
            raw == 0xf6 ? WE_EVENT_BEAT_BRUNOS_ROOM_TRAINER_0 : WE_EVENT_BEAT_AGATHAS_ROOM_TRAINER_0;
        ChangeBlock(w, 2, 0, PbrbWorldCheckEvent(w, ev) ? (raw == 0xf7 ? 14 : 5) : (raw == 0xf7 ? 0x3b : 0x24));
    }
    if (raw == 0x71) {
        int locked = PbrbWorldCheckEvent(w, WE_EVENT_LANCES_ROOM_LOCK_DOOR);
        ChangeBlock(w, 2, 6, locked ? 0x72 : 0x31); ChangeBlock(w, 3, 6, locked ? 0x73 : 0x32);
    }
}
static void MainElevatorWarps(PbrbWorld* w) {
    if (w->romMapId != 0xcb && w->romMapId != 0xec && w->romMapId != 0x7f)return;
    if (!WORLD_CATALOG_MAPS[w->storyValues[8]].valid)return;
    for (int i = 0; i < 2 && i < w->warpCount; ++i) { w->warps[i].destination = w->storyValues[8]; w->warps[i].warp = w->storyValues[9]; }
}
static void MainResetElite(PbrbWorld* w) {
    static const unsigned events[] = { WE_EVENT_BEAT_LORELEIS_ROOM_TRAINER_0,WE_EVENT_BEAT_BRUNOS_ROOM_TRAINER_0,
        WE_EVENT_BEAT_AGATHAS_ROOM_TRAINER_0,WE_EVENT_BEAT_LANCES_ROOM_TRAINER_0,WE_EVENT_BEAT_LANCE,
        WE_EVENT_LANCES_ROOM_LOCK_DOOR,WE_EVENT_AUTOWALKED_INTO_LORELEIS_ROOM,
        WE_EVENT_AUTOWALKED_INTO_BRUNOS_ROOM,WE_EVENT_AUTOWALKED_INTO_AGATHAS_ROOM,WE_EVENT_BEAT_CHAMPION_RIVAL };
    for (unsigned i = 0; i < sizeof(events) / sizeof(events[0]); ++i)PbrbWorldSetEvent(w, events[i], 0);
    w->defeatedTrainers[0xf5] = w->defeatedTrainers[0xf6] = w->defeatedTrainers[0xf7] = w->defeatedTrainers[0x71] = 0;
}
static void MainFinishHall(PbrbWorld* w) {
    MainResetElite(w); w->serviceFlags = (w->serviceFlags & ~16u) | 32u;
    PbrbWorldSetEvent(w, WE_EVENT_BEAT_CHAMPION_RIVAL, 1); HideObject(w, 3, 9, 1);
    NativeEnd(w); w->travelMode = w->fieldActive = 0; w->transitionFrame = 0; w->delay = 0;
    w->respawnMap = PBRB_MAP_PALLET; w->respawnX = 5; w->respawnY = 6; w->lastMapRom = 0;
    LoadMap(w, PBRB_MAP_BEDROOM, 3, 6, 0); w->warpLock = 0;
}
static void StoryMapLoaded(PbrbWorld* w) {
    if (w->restoring) { MainPuzzleBlocks(w); MainElevatorWarps(w); WorldAuxiliaryLoaded(w); return; }
    if (w->romMapId == 0xf5 && !(w->serviceFlags & 16u)) { MainResetElite(w); w->serviceFlags |= 16u; }
    WorldServicesLoaded(w); WorldAuxiliaryLoaded(w);
    TraversalLoaded(w); MainPuzzleBlocks(w); MainElevatorWarps(w);
    w->storyValues[10] = 0;
    if (w->romMapId == 2)PbrbWorldSetEvent(w, WE_EVENT_BOUGHT_MUSEUM_TICKET, 0);
    if (w->romMapId == 8)PbrbWorldSetEvent(w, WE_EVENT_MANSION_SWITCH_ON, 0);
    if (w->romMapId == 0x87) { unsigned lucky = RandomByte(w); w->storyValues[0] = (unsigned char)((lucky < 7 ? 8 : lucky) >> 3); }
    if (w->romMapId == 0x5c && !PbrbWorldCheckEvent(w, WE_EVENT_1ST_LOCK_OPENED))w->storyValues[1] = (unsigned char)(RandomByte(w) & 14u);
}
static void MainSetToggle(PbrbWorld* w, unsigned toggle, int hide) {
    unsigned off = 0xcaeau + toggle * 3u; if (!InRom(w, off, 3))return;
    unsigned map = w->rom[off], obj = w->rom[off + 1]; if (obj && obj <= 16)HideObject(w, (int)map, (int)obj - 1, hide);
}
static void MainTeamRocketLeaves(PbrbWorld* w) {
    /* The original table is a list of toggle IDs, not object indices. */
    static const unsigned hideOffsets[2] = { 0x6219bu,0x6219bu };
    static const unsigned showOffsets[2] = { 0x62194u,0x62194u };
    unsigned p = hideOffsets[GameIndex(w)];
    for (unsigned n = 0; n < 64 && InRom(w, p + n, 1) && w->rom[p + n] != 255; ++n)MainSetToggle(w, w->rom[p + n], 1);
    p = showOffsets[GameIndex(w)];
    for (unsigned n = 0; n < 16 && InRom(w, p + n, 1) && w->rom[p + n] != 255; ++n)MainSetToggle(w, w->rom[p + n], 0);
}
static void MainCry(PbrbWorld* w, unsigned species, int next) {
    w->scriptStep = next; Emit(w, PBRB_WORLD_SFX, 0, 0, PBRB_WORLD_WAIT_SOUND); w->event.species = species;
}
static void MainGiftStart(PbrbWorld* w, int index, int actor) { NativeStart(w, WG_GIFT, actor); w->scriptValue = index; }
static void MainPuzzleStart(PbrbWorld* w, int action, int actor, int param = 0) {
    NativeStart(w, WG_PUZZLE, actor); w->scriptValue = action; w->battleReturn = param;
}
static int MainInteraction(PbrbWorld* w, int actor, unsigned id) {
    if (TraversalInteraction(w, actor, id))return 1;
    int raw = w->romMapId;
    if ((raw == 0x34 && actor == 0) || (raw == 2 && actor == 2) || (raw == 0xe9 && actor == 0)) {
        NativeStart(w, WG_MUSEUM, actor); w->scriptValue = raw == 2 ? 1 : raw == 0xe9 ? 2 : 0; return 1;
    }
    if (raw == 0x7e && actor < 0 && id >= 3 && id <= 5) { Emit(w, PBRB_WORLD_VENDING, 0, 0); return 1; }
    for (unsigned i = 0; i < sizeof(WORLD_MAIN_GIFTS) / sizeof(WORLD_MAIN_GIFTS[0]); ++i)
        if (WORLD_MAIN_GIFTS[i].map == raw && WORLD_MAIN_GIFTS[i].actor == actor) { MainGiftStart(w, (int)i, actor); return 1; }
    if ((raw == 0x46 || raw == 0x49 || raw == 0x4c || raw == 0x4f) && actor == 0) { NativeStart(w, WG_SAFFRON_GATE, actor); return 1; }
    if (raw == 0x87 && actor == 10) { NativeStart(w, WG_BOSS, actor); w->scriptValue = 0; return 1; }
    if (raw == 0x87 && actor < 0 && id == 12) { NativeStart(w, WG_ROCKET_BUTTON, -1); return 1; }
    if (raw == 0xca && actor == 0) { NativeStart(w, WG_BOSS, actor); w->scriptValue = 1; return 1; }
    if (raw == 0xca && actor == 3 && PbrbWorldCheckEvent(w, WE_EVENT_BEAT_ROCKET_HIDEOUT_4_TRAINER_2)) {
        NativeStart(w, WG_BOSS, actor); w->scriptValue = 3; return 1;
    }
    if (raw == 0xeb && actor == 2) { NativeStart(w, WG_BOSS, actor); w->scriptValue = 2; return 1; }
    if (raw == 0x94 && actor == 3) { NativeStart(w, WG_FUJI, actor); return 1; }
    if (raw == 0x95 && actor < 2) {
        Emit(w, PBRB_WORLD_TEXT, Story(w, actor == 0 ?
            (PbrbWorldCheckEvent(w, WE_EVENT_RESCUED_MR_FUJI) ? WX_MrFujisHouseSuperNerdMrFujiHadBeenPrayingText : WX_MrFujisHouseSuperNerdMrFujiIsntHereText) :
            (PbrbWorldCheckEvent(w, WE_EVENT_RESCUED_MR_FUJI) ? WX_MrFujisHouseLittleGirlPokemonAreNiceToHugText : WX_MrFujisHouseLittleGirlThisIsMrFujisHouseText)), 0); return 1;
    }
    if (raw == 0x9b && actor == 0) { MainPuzzleStart(w, WP_WARDEN, actor); return 1; }
    if (raw == 0x9b && id >= 4) { Emit(w, PBRB_WORLD_TEXT, Story(w, id == 4 ? WX_WardensHouseDisplayPhotosAndFossilsText : WX_WardensHouseDisplayMerchandiseText), 0); return 1; }
    if (raw == 0x9c && actor == 0) { NativeStart(w, WG_SAFARI, actor); return 1; }
    if (raw == 0x9c && actor == 1) { NativeStart(w, WG_SAFARI, actor); w->scriptStep = 30; return 1; }
    for (int e = 0; e < 3; ++e)if (raw == WORLD_ELEVATORS[e].map) { MainPuzzleStart(w, WP_ELEVATOR, actor, e); return 1; }
    if (raw == 0xc1 && actor == 0) { NativeStart(w, WG_LEAGUE_GATE, actor); w->scriptValue = 0; return 1; }
    if (raw == 0x22 && actor >= 0 && actor < 7) { NativeStart(w, WG_LEAGUE_GATE, actor); w->scriptValue = 7 - actor; return 1; }
    if ((raw == 0xf5 || raw == 0xf6 || raw == 0xf7 || raw == 0x71) && actor == 0) { NativeStart(w, WG_ELITE, actor); return 1; }
    if (raw == 0x78 && actor == 0) { NativeStart(w, WG_CHAMPION, actor); w->scriptStep = 1; return 1; }
    if (actor >= 0 && w->actors[actor].sprite == 63) { Emit(w, PBRB_WORLD_TEXT, Story(w, WX_BoulderText), 0); return 1; }
    return 0;
}
static int MainHidden(PbrbWorld* w, int x, int y) {
    if (TraversalHidden(w, x, y))return 1;
    if (MainSilphFloor(w->romMapId)) {
        unsigned tile = TileAt(w, x * 2, y * 2 + 1);
        if (tile == 0x18u || tile == 0x24u || (w->romMapId == 0xeb && tile == 0x5eu)) {
            if (!HasItem(w, 0x30)) { Emit(w, PBRB_WORLD_TEXT, Story(w, WX_CardKeyFailText), 0); return 1; }
            for (unsigned i = 0; i < sizeof(WORLD_SILPH_DOORS) / sizeof(WORLD_SILPH_DOORS[0]); ++i) {
                const WorldDoorRule* d = &WORLD_SILPH_DOORS[i];
                if (d->map == w->romMapId && d->x == x / 2 && d->y == y / 2) { MainPuzzleStart(w, WP_CARD_KEY, -1, (int)i); return 1; }
            }
        }
    }
    if (w->facing == 1 && ((w->romMapId == 0xa5 && x == 2 && y == 5) ||
        (w->romMapId == 0xd6 && x == 2 && y == 11) ||
        (w->romMapId == 0xd7 && x == 10 && y == 5) ||
        (w->romMapId == 0xd8 && ((x == 20 && y == 3) || (x == 18 && y == 25))))) {
        MainPuzzleStart(w, WP_MANSION, -1); return 1;
    }
    if (w->romMapId == 0x5c && x >= 1 && x <= 9 && (x & 1) && y >= 7 && y <= 11 && (y & 1)) {
        MainPuzzleStart(w, WP_SURGE, -1, (x / 2) * 3 + (y - 7) / 2); return 1;
    }
    return 0;
}
static int MainIdle(PbrbWorld* w) {
    if (TraversalIdle(w))return 1;
    int raw = w->romMapId, x = w->heroX, y = w->heroY;
    if (raw == 0x34 && (x == 9 || x == 10) && y == 4 && !PbrbWorldCheckEvent(w, WE_EVENT_BOUGHT_MUSEUM_TICKET)) {
        NativeStart(w, WG_MUSEUM, 0); return 1;
    }
    MainPuzzleBlocks(w);
    if (raw == 0xc1)w->lastMapRom = y < 4 ? 0x22 : 0x21;
    if ((raw == 0x46 || raw == 0x49 || raw == 0x4c || raw == 0x4f) && !(w->serviceFlags & 8u)) {
        int hit = (raw == 0x46 && (x == 3 || x == 4) && y == 3) || (raw == 0x49 && (x == 3 || x == 4) && y == 2) ||
            (raw == 0x4c && x == 3 && (y == 3 || y == 4)) || (raw == 0x4f && x == 2 && (y == 3 || y == 4));
        if (hit) { NativeStart(w, WG_SAFFRON_GATE, 0); w->scriptValue = 1; return 1; }
    }
    if (raw == 0xeb && !PbrbWorldCheckEvent(w, WE_EVENT_BEAT_SILPH_CO_GIOVANNI) && ((x == 6 && y == 13) || (x == 7 && y == 12))) {
        NativeStart(w, WG_BOSS, 2); w->scriptValue = 2; w->battleReturn = 1; return 1;
    }
    if (raw == 0x93 && x == 10 && y == 16 && !PbrbWorldCheckEvent(w, WE_EVENT_BEAT_GHOST_MAROWAK)) {
        NativeStart(w, WG_TOWER_GHOST, -1); return 1;
    }
    if (raw == 8 && x == 18 && y == 4 && !HasItem(w, 0x2b)) { MainPuzzleStart(w, WP_CINNABAR_LOCK, -1); return 1; }
    if (raw == 0x9c) {
        if (w->safariActive && y == 1) { NativeStart(w, WG_SAFARI, 0); w->scriptStep = PbrbWorldCheckEvent(w, WE_EVENT_SAFARI_GAME_OVER) ? 20 : 15; return 1; }
        if (!w->safariActive && (x == 3 || x == 4) && y == 2) { NativeStart(w, WG_SAFARI, 0); w->scriptValue = 1; return 1; }
    }
    if (MainSafariMap(raw) && w->safariActive && (!w->safariSteps || !w->player.safariBalls)) {
        NativeStart(w, WG_SAFARI, -1); w->scriptStep = 40; return 1;
    }
    if (raw == 0xc1 && (x == 4 || x == 5) && y == 2 && !w->storyValues[10]) {
        NativeStart(w, WG_LEAGUE_GATE, 0); w->scriptValue = 0; return 1;
    }
    if (raw == 0x22) {
        static const int guardY[7] = { 35,56,85,96,105,119,136 };
        for (int i = 0; i < 7; ++i)if (y == guardY[i] && (i || x < 14) &&
            !PbrbWorldCheckEvent(w, WE_EVENT_PASSED_CASCADEBADGE_CHECK + 6 - i)) {
            NativeStart(w, WG_LEAGUE_GATE, i); w->scriptValue = 7 - i; return 1;
        }
    }
    if ((raw == 0xf5 || raw == 0xf6 || raw == 0xf7) && (x == 4 || x == 5) && (y == 10 || y == 11)) {
        MainPuzzleStart(w, WP_ELITE_ENTRY, -1, raw - 0xf5); return 1;
    }
    if (raw == 0x71 && !PbrbWorldCheckEvent(w, WE_EVENT_BEAT_LANCE)) {
        if (x == 24 && y == 16) { MainPuzzleStart(w, WP_ELITE_ENTRY, -1, 3); return 1; }
        if ((x == 5 || x == 6) && y == 11 && !PbrbWorldCheckEvent(w, WE_EVENT_LANCES_ROOM_LOCK_DOOR)) {
            PbrbWorldSetEvent(w, WE_EVENT_LANCES_ROOM_LOCK_DOOR, 1); Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_GO_INSIDE); return 1;
        }
        if ((x == 5 && y == 1) || (x == 6 && y == 2)) { NativeStart(w, WG_ELITE, 0); return 1; }
    }
    if (raw == 0x78 && !PbrbWorldCheckEvent(w, WE_EVENT_BEAT_CHAMPION_RIVAL) && y >= 6) { NativeStart(w, WG_CHAMPION, 0); return 1; }
    if (raw == 0x76) { NativeStart(w, WG_HALL, 0); return 1; }
    return 0;
}
#include "pbrb_world_traversal.inl"
static int MainScript(PbrbWorld* w) {
    if (TraversalScript(w))return 1;
    int s = w->scriptStep, a = w->scriptObject, v = w->scriptValue;
    switch (w->scriptId) {
    case WG_MUSEUM:
        if (v == 0) {
            if (s == 0) {
                int x = w->heroX, y = w->heroY;
                if ((x == 13 && y == 4) || (x == 12 && y == 3))NativeText(w, WX_Museum1FScientist1DoYouKnowWhatAmberIsText, 20, PBRB_WORLD_AUTO_CLOSE);
                else if (PbrbWorldCheckEvent(w, WE_EVENT_BOUGHT_MUSEUM_TICKET))NativeText(w, WX_Museum1FScientist1TakePlentyOfTimeText, 99);
                else if (y != 4)NativeText(w, WX_Museum1FScientist1GoToOtherSideText, 99);
                else NativeText(w, WX_Museum1FScientist1WouldYouLikeToComeInText, 1, PBRB_WORLD_AUTO_CLOSE);
            }
            else if (s == 1)NativeChoice(w, 2);
            else if (s == 2) {
                if (!w->scriptResult)w->scriptStep = 10;
                else if (w->player.money < 50)NativeText(w, WX_Museum1FScientist1DontHaveEnoughMoneyText, 10);
                else NativeText(w, WX_Museum1FScientist1ThankYouText, 3);
            }
            else if (s == 3) { w->scriptStep = 4; Emit(w, PBRB_WORLD_TAKE_MONEY, 0, 50); w->event.quantity = 50; }
            else if (s == 4) {
                if (w->scriptResult) { PbrbWorldSetEvent(w, WE_EVENT_BOUGHT_MUSEUM_TICKET, 1); NativeSound(w, PBRB_SFX_PURCHASE, 99); }
                else NativeText(w, WX_Museum1FScientist1DontHaveEnoughMoneyText, 10);
            }
            else if (s == 10)NativeText(w, WX_Museum1FScientist1ComeAgainText, 11);
            else if (s == 11) { HeroPath(w); AddHero(w, 0, 1); w->scriptStep = 12; }
            else if (s == 12) { if (PathsDone(w))NativeEnd(w); }
            else if (s == 20)NativeChoice(w, 21);
            else if (s == 21)NativeText(w, w->scriptResult ? WX_Museum1FScientist1TheresALabSomewhereText : WX_Museum1FScientist1AmberIsFossilizedTreeSapText, 99);
            else NativeEnd(w);
        }
        else if (v == 1) {
            if (s == 0)NativeText(w, WX_PewterCitySuperNerd1DidYouCheckOutMuseumText, 1, PBRB_WORLD_AUTO_CLOSE);
            else if (s == 1)NativeChoice(w, 2);
            else if (s == 2)NativeText(w, w->scriptResult ? WX_PewterCitySuperNerd1WerentThoseFossilsAmazingText : WX_PewterCitySuperNerd1YouHaveToGoText, w->scriptResult ? 99 : 3);
            else if (s == 3) {
                HeroPath(w); int x = w->heroX, y = w->heroY;
                if (y == 18)AddHero(w, 1, 2);
                else if (y == 16) { AddHero(w, 2, 1); AddHero(w, 3, 1); }
                else { AddHero(w, x == 26 ? 3 : 2, 1); AddHero(w, 1, 1); }
                AddHero(w, 1, 5); AddHero(w, 2, 13); AddHero(w, 1, 3);
                ActorPath(w, 2); AddActor(w, 1, 6); AddActor(w, 2, 13); AddActor(w, 1, 3); AddActor(w, 2, 1);
                w->scriptStep = 4; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_MUSEUM);
            }
            else if (s == 4) { if (PathsDone(w)) { w->actors[2].facing = 1; w->scriptStep = 5; Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w)); } }
            else if (s == 5)NativeText(w, WX_PewterCitySuperNerd1ItsRightHereText, 6);
            else if (s == 6) { ActorPath(w, 2); AddActor(w, 0, 4); w->scriptStep = 7; }
            else if (s == 7) { if (PathsDone(w)) { w->actors[2].x = w->actors[2].homeX; w->actors[2].y = w->actors[2].homeY; NativeEnd(w); } }
            else NativeEnd(w);
        }
        else {
            if (s == 0)NativeText(w, PbrbWorldCheckEvent(w, WE_EVENT_BEAT_SILPH_CO_GIOVANNI) ? WX_SilphCo9FNurseThankYouText : WX_SilphCo9FNurseYouLookTiredText,
                PbrbWorldCheckEvent(w, WE_EVENT_BEAT_SILPH_CO_GIOVANNI) ? 99 : 1);
            else if (s == 1) { w->scriptStep = 2; w->delay = 51; Emit(w, PBRB_WORLD_HEAL, 0, 0); }
            else if (s == 2)NativeText(w, WX_SilphCo9FNurseDontGiveUpText, 99);
            else NativeEnd(w);
        }return 1;
    case WG_GIFT: {
        const WorldGiftRule* r = &WORLD_MAIN_GIFTS[v];
        if (s == 0) { if (PbrbWorldCheckEvent(w, (unsigned)r->event))NativeText(w, r->after, 9); else NativeText(w, r->before, 1); }
        else if (s == 1)NativeItem(w, (unsigned)r->item, 1, 2);
        else if (s == 2) {
            if (!w->scriptResult)NativeText(w, r->full, 9); else {
                PbrbWorldSetEvent(w, (unsigned)r->event, 1); NativeText(w, r->received, 3, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT); w->event.itemId = (unsigned)r->item;
            }
        }
        else if (s == 3)NativeSound(w, r->sound, 4, r->following < 0);
        else if (s == 4 && r->following >= 0)NativeText(w, r->following, 9); else NativeEnd(w); return 1;
    }
    case WG_SAFFRON_GATE:
        if (s == 0) {
            if (w->serviceFlags & 8u)NativeText(w, WX_SaffronGateGuardThanksForTheDrinkText, 9);
            else {
                unsigned drink = HasItem(w, 0x3c) ? 0x3c : HasItem(w, 0x3d) ? 0x3d : HasItem(w, 0x3e) ? 0x3e : 0;
                if (drink)NativeItem(w, drink, 1, 1, 1); else NativeText(w, WX_SaffronGateGuardGeeImThirstyText, 6);
            }
        }
        else if (s == 1) { if (w->scriptResult)NativeText(w, WX_SaffronGateGuardImParchedText, 2, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT); else w->scriptStep = 6; }
        else if (s == 2)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 3);
        else if (s == 3) { w->serviceFlags |= 8u; NativeText(w, WX_SaffronGateGuardYouCanGoOnThroughText, 9); }
        else if (s == 6) { HeroPath(w); AddHero(w, w->romMapId == 0x46 ? 1 : w->romMapId == 0x49 ? 0 : w->romMapId == 0x4c ? 2 : 3, 1); w->scriptStep = 7; }
        else if (s == 7) { if (PathsDone(w))NativeEnd(w); }
        else NativeEnd(w); return 1;
    case WG_ROCKET_BUTTON:
        if (s == 0)NativeText(w, WX_GameCornerPosterSwitchBehindPosterText, 1, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 1)NativeSound(w, PBRB_SFX_SWITCH, 2);
        else if (s == 2) { PbrbWorldSetEvent(w, WE_EVENT_FOUND_ROCKET_HIDEOUT, 1); MainPuzzleBlocks(w); NativeSound(w, PBRB_SFX_GO_INSIDE, 3); }
        else NativeEnd(w); return 1;
    case WG_BOSS:
        if (v == 3) {
            if (s == 0)NativeText(w, WX_RocketHideoutB4FRocket3AfterBattleText, 1);
            else { PbrbWorldSetEvent(w, WE_EVENT_ROCKET_DROPPED_LIFT_KEY, 1); HideObject(w, 0xca, 8, 0); NativeEnd(w); }return 1;
        }
        if (s == 0)NativeText(w, v == 0 ? WX_GameCornerRocketImGuardingThisPosterText : v == 1 ?
            WX_RocketHideoutB4FGiovanniImpressedYouGotHereText : WX_SilphCo11FGiovanniText, 1);
        else if (s == 1) {
            if (v == 2 && w->battleReturn) { ActorPath(w, a); AddActor(w, 0, 3); w->scriptStep = 2; }
            else w->scriptStep = 2;
        }
        else if (s == 2 && PathsDone(w)) {
            if (v == 2) { w->facing = w->heroX == 6 ? 1 : 2; w->actors[a].facing = w->facing ^ 1; }
            unsigned end = Story(w, v == 0 ? WX_GameCornerRocketBattleEndText : v == 1 ? WX_RocketHideoutB4FGiovanniWhatCannotBeText : WX_SilphCo11FGiovanniILostAgainText);
            NativeBattle(w, w->actors[a].trainerClass - 200u, w->actors[a].trainerId, PBRB_WORLD_TRAINER, end, end, 3);
        }
        else if (s == 3)NativeText(w, v == 0 ? WX_GameCornerRocketAfterBattleText : v == 1 ?
            WX_RocketHideoutB4FGiovanniHopeWeMeetAgainText : WX_SilphCo11FGiovanniYouRuinedOurPlansText, 4);
        else if (s == 4) {
            if (v == 0) {
                ActorPath(w, a); if (w->heroY == 6 || w->heroX == 8)AddActor(w, 3, 5);
                else { AddActor(w, 0, 1); AddActor(w, 3, 2); AddActor(w, 1, 1); AddActor(w, 3, 4); }w->scriptStep = 5;
            }
            else {
                if (v == 1) { PbrbWorldSetEvent(w, WE_EVENT_BEAT_ROCKET_HIDEOUT_GIOVANNI, 1); HideObject(w, 0xca, 0, 1); HideObject(w, 0xca, 7, 0); }
                else { PbrbWorldSetEvent(w, WE_EVENT_BEAT_SILPH_CO_GIOVANNI, 1); MainTeamRocketLeaves(w); }w->delay = 20; w->scriptStep = 5;
            }
        }
        else if (s == 5 && PathsDone(w)) { if (v == 0)HideObject(w, 0x87, 10, 1); NativeEnd(w); }return 1;
    case WG_TOWER_GHOST:
        if (s == 0)NativeText(w, WX_PokemonTower6FBeGoneText, 1);
        else if (s == 1)NativeBattle(w, 0x91, 30, PBRB_WORLD_STATIC | PBRB_WORLD_NO_CATCH | (!HasItem(w, 0x48) ? PBRB_WORLD_GHOST : 0), 0, 0, 2);
        else if (s == 2) {
            if (w->scriptResult == PBRB_WORLD_BATTLE_WON && HasItem(w, 0x48)) {
                PbrbWorldSetEvent(w, WE_EVENT_BEAT_GHOST_MAROWAK, 1); NativeText(w, WX_PokemonTower6FGhostWasCubonesMotherText, 3);
            }
            else { HeroPath(w); AddHero(w, 3, 1); w->scriptStep = 6; }
        }
        else if (s == 3)MainCry(w, 0x91, 4);
        else if (s == 4) { w->delay = 30; w->scriptStep = 5; }
        else if (s == 5)NativeText(w, WX_PokemonTower6FSoulWasCalmedText, 6);
        else if (s == 6 && PathsDone(w))NativeEnd(w); return 1;
    case WG_FUJI:
        if (s == 0)NativeText(w, WX_PokemonTower7FMrFujiRescueText, 1);
        else {
            PbrbWorldSetEvent(w, WE_EVENT_RESCUED_MR_FUJI, 1); PbrbWorldSetEvent(w, WE_EVENT_RESCUED_MR_FUJI_2, 1);
            HideObject(w, 0x94, 3, 1); HideObject(w, 0x95, 4, 0); HideObject(w, 10, 13, 1); HideObject(w, 10, 14, 0);
            w->lastMapRom = 4; NativeEnd(w); Transition(w, PbrbWorldMapFromRom(0x95), 3, 7, 1, WS_IDLE, 0);
        }return 1;
    case WG_SAFARI:
        if (s == 0) { NativeText(w, WX_SafariZoneGateSafariZoneWorker1Text, v ? 1 : 99); }
        else if (s == 1) { if (w->heroX == 3) { HeroPath(w); AddHero(w, 3, 1); }w->facing = 3; w->scriptStep = 2; }
        else if (s == 2 && PathsDone(w))NativeText(w, WX_SafariZoneGateSafariZoneWorker1WouldYouLikeToJoinText, 3);
        else if (s == 3)NativeChoice(w, 4);
        else if (s == 4) {
            if (!w->scriptResult)NativeText(w, WX_SafariZoneGateSafariZoneWorker1PleaseComeAgainText, 12);
            else { w->scriptStep = 5; Emit(w, PBRB_WORLD_TAKE_MONEY, 0, 500); w->event.quantity = 500; }
        }
        else if (s == 5) {
            if (w->scriptResult)NativeText(w, WX_SafariZoneGateSafariZoneWorker1ThatllBe500PleaseText, 6, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            else NativeText(w, WX_SafariZoneGateSafariZoneWorker1NotEnoughMoneyText, 12);
        }
        else if (s == 6)NativeSound(w, PBRB_SFX_GET_ITEM, 7);
        else if (s == 7)NativeText(w, WX_SafariZoneGateSafariZoneWorker1CallYouOnThePAText, 8);
        else if (s == 8) {
            w->safariActive = 1; w->safariSteps = 500; PbrbWorldSetEvent(w, WE_EVENT_IN_SAFARI_ZONE, 1);
            PbrbWorldSetEvent(w, WE_EVENT_SAFARI_GAME_OVER, 0); w->scriptStep = 9; Emit(w, PBRB_WORLD_SAFARI, 0, 1);
        }
        else if (s == 9) { HeroPath(w); AddHero(w, 1, 3); w->scriptStep = 10; }
        else if (s == 10) { if (PathsDone(w))NativeEnd(w); }
        else if (s == 12) { HeroPath(w); AddHero(w, 0, 1); w->scriptStep = 10; }
        else if (s == 15)NativeText(w, WX_SafariZoneGateSafariZoneWorker1LeavingEarlyText, 16);
        else if (s == 16)NativeChoice(w, 17);
        else if (s == 17) {
            if (w->scriptResult)NativeText(w, WX_SafariZoneGateSafariZoneWorker1ReturnSafariBallsText, 21);
            else NativeText(w, WX_SafariZoneGateSafariZoneWorker1GoodLuckText, 18);
        }
        else if (s == 18) { HeroPath(w); AddHero(w, 1, 1); w->scriptStep = 10; }
        else if (s == 20)NativeText(w, WX_SafariZoneGateSafariZoneWorker1GoodHaulComeAgainText, 21);
        else if (s == 21) {
            w->safariActive = 0; w->safariSteps = 0; PbrbWorldSetEvent(w, WE_EVENT_IN_SAFARI_ZONE, 0);
            PbrbWorldSetEvent(w, WE_EVENT_SAFARI_GAME_OVER, 0); w->scriptStep = 22; Emit(w, PBRB_WORLD_SAFARI, 0, 0);
        }
        else if (s == 22) { HeroPath(w); AddHero(w, 0, 3); w->scriptStep = 10; }
        else if (s == 30)NativeText(w, WX_SafariZoneGateSafariZoneWorker2FirstTimeHereText, 31);
        else if (s == 31)NativeChoice(w, 32);
        else if (s == 32)NativeText(w, w->scriptResult ? WX_SafariZoneGateSafariZoneWorker2SafariZoneExplanationText : WX_SafariZoneGateSafariZoneWorker2YoureARegularHereText, 99);
        else if (s == 40)NativeSound(w, PBRB_SFX_SAFARI_PA, 43);
        else if (s == 43) { if (w->player.safariBalls)NativeText(w, WX_TimesUpText, 41); else w->scriptStep = 41; }
        else if (s == 41)NativeText(w, WX_GameOverText, 42);
        else if (s == 42) {
            PbrbWorldSetEvent(w, WE_EVENT_SAFARI_GAME_OVER, 1); NativeEnd(w); w->lastMapRom = 7;
            Transition(w, PbrbWorldMapFromRom(0x9c), 4, 1, 0, WS_IDLE, 0);
        }
        else NativeEnd(w); return 1;
    case WG_LEAGUE_GATE:
        if (s == 0) {
            int have = (w->player.badges & (1u << v)) != 0;
            NativeText(w, v ? (have ? WX_Route23OhThatIsTheBadgeText : WX_Route23YouDontHaveTheBadgeYetText) :
                (have ? WX_Route22GateGuardGoRightAheadText : WX_Route22GateGuardNoBoulderbadgeText), have ? 1 : 4, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            w->event.itemId = (unsigned)(0x15 + v);
        }
        else if (s == 1)NativeSound(w, PBRB_SFX_GET_ITEM, 2, v == 0);
        else if (s == 2) {
            if (v) { PbrbWorldSetEvent(w, WE_EVENT_PASSED_CASCADEBADGE_CHECK + v - 1, 1); NativeText(w, WX_Route23GoRightAheadText, 8); }
            else { w->storyValues[10] = 1; NativeEnd(w); }
        }
        else if (s == 4)NativeSound(w, PBRB_SFX_DENIED, 5);
        else if (s == 5) { if (!v)NativeText(w, WX_Route22GateGuardICantLetYouPassText, 6); else w->scriptStep = 6; }
        else if (s == 6) { HeroPath(w); AddHero(w, 0, 1); w->scriptStep = 7; }
        else if (s == 7) { if (PathsDone(w))NativeEnd(w); }
        else NativeEnd(w); return 1;
    case WG_PUZZLE:
        if (v == WP_CARD_KEY) {
            const WorldDoorRule* d = &WORLD_SILPH_DOORS[w->battleReturn];
            if (s == 0)NativeText(w, WX_CardKeySuccessText1, 1, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            else if (s == 1)NativeSound(w, PBRB_SFX_GET_ITEM, 2);
            else if (s == 2)NativeText(w, WX_CardKeySuccessText2, 3);
            else if (s == 3) { PbrbWorldSetEvent(w, d->event, 1); MainPuzzleBlocks(w); NativeSound(w, PBRB_SFX_GO_INSIDE, 4); }
            else NativeEnd(w);
        }
        else if (v == WP_MANSION) {
            if (s == 0)NativeText(w, WX_PokemonMansion1FSwitchText, 1);
            else if (s == 1)NativeChoice(w, 2);
            else if (s == 2) {
                if (w->scriptResult)NativeText(w, WX_PokemonMansion1FSwitchPressedText, 3, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
                else NativeText(w, WX_PokemonMansion1FSwitchNotPressedText, 9);
            }
            else if (s == 3) {
                PbrbWorldSetEvent(w, WE_EVENT_MANSION_SWITCH_ON, !PbrbWorldCheckEvent(w, WE_EVENT_MANSION_SWITCH_ON));
                MainPuzzleBlocks(w); NativeSound(w, PBRB_SFX_GO_INSIDE, 9);
            }
            else NativeEnd(w);
        }
        else if (v == WP_SURGE) {
            if (s == 0) {
                int can = w->battleReturn;
                if (PbrbWorldCheckEvent(w, WE_EVENT_2ND_LOCK_OPENED))NativeText(w, WX_VermilionGymTrashText, 9);
                else if (PbrbWorldCheckEvent(w, WE_EVENT_1ST_LOCK_OPENED)) {
                    if (can == w->storyValues[2]) {
                        PbrbWorldSetEvent(w, WE_EVENT_2ND_LOCK_OPENED, 1);
                        NativeText(w, WX_VermilionGymTrashSuccessText3, 3, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
                    }
                    else {
                        PbrbWorldSetEvent(w, WE_EVENT_1ST_LOCK_OPENED, 0); w->storyValues[1] = (unsigned char)(RandomByte(w) & 14u);
                        NativeText(w, WX_VermilionGymTrashFailText, 4, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
                    }
                }
                else if (can == w->storyValues[1]) {
                    PbrbWorldSetEvent(w, WE_EVENT_1ST_LOCK_OPENED, 1);
                    unsigned p = 0x5de7du + (unsigned)can * 5u; unsigned r = RandomByte(w); r = ((r << 4) | (r >> 4)) & 255u;
                    unsigned offset = ((r & w->rom[p]) - 1u) & 255u;
                    w->storyValues[2] = (unsigned char)(w->rom[p + 1 + offset] & 15u);
                    NativeText(w, WX_VermilionGymTrashSuccessText1, 2, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
                }
                else NativeText(w, WX_VermilionGymTrashText, 9);
            }
            else if (s == 2)NativeSound(w, PBRB_SFX_SWITCH, 9);
            else if (s == 3) { MainPuzzleBlocks(w); NativeSound(w, PBRB_SFX_GO_INSIDE, 9); }
            else if (s == 4)NativeSound(w, PBRB_SFX_DENIED, 9); else NativeEnd(w);
        }
        else if (v == WP_ELEVATOR) {
            const WorldElevatorRule* e = &WORLD_ELEVATORS[w->battleReturn];
            if (s == 0) {
                if (e->map == 0xcb && !HasItem(w, 0x4a))NativeText(w, WX_RocketHideoutElevatorAppearsToNeedKeyText, 9);
                else {
                    int current = 0; unsigned p = e->warps[GameIndex(w)]; for (unsigned i = 0; i < e->count; ++i)
                        if (w->rom[p + i * 2 + 1] == w->storyValues[8])current = (int)i;
                    w->scriptStep = 1; Emit(w, PBRB_WORLD_ELEVATOR, 0, current);
                    w->event.dataOffset = e->floors[GameIndex(w)]; w->event.dataCount = e->count;
                }
            }
            else if (s == 1) {
                int choice = w->scriptResult;
                if (choice<1 || choice>e->count) { NativeEnd(w); return 1; }
                unsigned p = e->warps[GameIndex(w)] + (unsigned)(choice - 1) * 2u;
                w->storyValues[8] = w->rom[p + 1]; w->storyValues[9] = w->rom[p]; MainElevatorWarps(w);
                w->storyValues[11] = 100; w->scriptStep = 2; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_NONE);
            }
            else if (s == 2) {
                if (w->storyValues[11]) { --w->storyValues[11]; w->delay = 2; Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_COLLISION); }
                else NativeSound(w, PBRB_SFX_SAFARI_PA, 3);
            }
            else if (s == 3) { w->scriptStep = 4; Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w)); }
            else NativeEnd(w);
        }
        else if (v == WP_WARDEN) {
            if (s == 0) {
                if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_HM04))NativeText(w, WX_WardensHouseWardenHM04ExplanationText, 99);
                else if (HasItem(w, 0x40))NativeText(w, WX_WardensHouseWardenGaveTheGoldTeethText, 3, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
                else if (PbrbWorldCheckEvent(w, WE_EVENT_GAVE_GOLD_TEETH))w->scriptStep = 6;
                else NativeText(w, WX_WardensHouseWardenGibberish1Text, 1);
            }
            else if (s == 1)NativeChoice(w, 2);
            else if (s == 2)NativeText(w, w->scriptResult ? WX_WardensHouseWardenGibberish2Text : WX_WardensHouseWardenGibberish3Text, 99);
            else if (s == 3)NativeSound(w, PBRB_SFX_GET_ITEM, 4);
            else if (s == 4)NativeText(w, WX_WardensHouseWardenTeethPoppedInHisTeethText, 5);
            else if (s == 5)NativeItem(w, 0x40, 1, 6, 1);
            else if (s == 6) { PbrbWorldSetEvent(w, WE_EVENT_GAVE_GOLD_TEETH, 1); NativeText(w, WX_WardensHouseWardenThanksText, 7); }
            else if (s == 7)NativeItem(w, 0xc7, 1, 8);
            else if (s == 8) {
                if (w->scriptResult) {
                    PbrbWorldSetEvent(w, WE_EVENT_GOT_HM04, 1);
                    NativeText(w, WX_WardensHouseWardenReceivedHM04Text, 9, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
                }
                else NativeText(w, WX_WardensHouseWardenHM04NoRoomText, 99);
            }
            else if (s == 9)NativeSound(w, PBRB_SFX_GET_ITEM, 99, 1); else NativeEnd(w);
        }
        else if (v == WP_CINNABAR_LOCK) {
            if (s == 0)NativeText(w, WX_CinnabarIslandDoorIsLockedText, 1);
            else if (s == 1) { HeroPath(w); AddHero(w, 0, 1); w->scriptStep = 2; }
            else if (PathsDone(w))NativeEnd(w);
        }
        else if (v == WP_ELITE_ENTRY) {
            int room = w->battleReturn;
            static const unsigned ev[3] = { WE_EVENT_AUTOWALKED_INTO_LORELEIS_ROOM,WE_EVENT_AUTOWALKED_INTO_BRUNOS_ROOM,WE_EVENT_AUTOWALKED_INTO_AGATHAS_ROOM };
            static const int noRun[3] = { WX_LoreleisRoomLoreleiDontRunAwayText,WX_BrunosRoomBrunoDontRunAwayText,WX_AgathasRoomAgathaDontRunAwayText };
            if (s == 0) {
                if (room == 3) { HeroPath(w); AddHero(w, 2, 6); AddHero(w, 0, 7); AddHero(w, 2, 12); AddHero(w, 1, 12); w->scriptStep = 3; }
                else if (w->heroY == 11 && !PbrbWorldCheckEvent(w, ev[room])) {
                    PbrbWorldSetEvent(w, ev[room], 1);
                    HeroPath(w); AddHero(w, 1, 6); w->scriptStep = 3;
                }
                else NativeText(w, noRun[room], 1);
            }
            else if (s == 1) { HeroPath(w); AddHero(w, 1, 1); w->scriptStep = 3; }
            else if (s == 3 && PathsDone(w))NativeEnd(w);
        }
        else if (v == WP_SNORLAX) {
            int route = w->romMapId == 0x17 ? 12 : 16;
            if (s == 0)NativeText(w, a >= 0 ? WX_PlayedFluteHadEffectText : WX_PlayedFluteNoEffectText, a >= 0 ? 1 : 9);
            else if (s == 1)NativeSound(w, PBRB_SFX_POKEFLUTE, 2);
            else if (s == 2) {
                if (a < 0)NativeEnd(w);
                else NativeText(w, route == 12 ? WX_Route12SnorlaxWokeUpText : WX_Route16SnorlaxWokeUpText, 3);
            }
            else if (s == 3) { HideObject(w, w->romMapId, a, 1); NativeBattle(w, 0x84, 30, PBRB_WORLD_STATIC, 0, 0, 4); }
            else if (s == 4) {
                PbrbWorldSetEvent(w, route == 12 ? WE_EVENT_BEAT_ROUTE12_SNORLAX : WE_EVENT_BEAT_ROUTE16_SNORLAX, 1);
                if (w->scriptResult != PBRB_WORLD_BATTLE_CAUGHT)NativeText(w, route == 12 ? WX_Route12SnorlaxCalmedDownText : WX_Route16SnorlaxReturnedToMountainsText, 9);
                else NativeEnd(w);
            }
            else NativeEnd(w);
        }
        else NativeEnd(w); return 1;
    case WG_TOWER_EXIT:
        if (s == 0) { const WorldCatalogText* t = CatalogText(w, w->actors[a].textId); NativeOffset(w, t->afterText[GameIndex(w)], 1); }
        else if (s == 1) {
            ActorPath(w, a);
            if (a == 0 && w->heroX == 9) { AddActor(w, 3, 1); AddActor(w, 0, 5); AddActor(w, 2, 1); }
            else if (a == 0 && w->heroX == 10) { AddActor(w, 0, 1); AddActor(w, 3, 1); AddActor(w, 0, 4); }
            else if (a == 1 && w->heroX == 12) { AddActor(w, 2, 1); AddActor(w, 0, 6); }
            else if (a == 1 && w->heroX == 11) { AddActor(w, 0, 3); AddActor(w, 2, 1); AddActor(w, 0, 2); }
            else if (a == 2 && w->heroX == 9) { AddActor(w, 3, 1); AddActor(w, 0, 6); }
            else AddActor(w, 0, 5); w->scriptStep = 2;
        }
        else if (s == 2 && PathsDone(w)) { HideObject(w, 0x94, a, 1); NativeEnd(w); }return 1;
    case WG_ELITE: {
        const WorldCatalogText* t = CatalogText(w, 1);
        if (s == 0) {
            if (TrainerDefeated(w, 0, t))NativeOffset(w, t->afterText[GameIndex(w)], 9);
            else NativeOffset(w, t->text[GameIndex(w)], 1);
        }
        else if (s == 1)NativeBattle(w, w->actors[0].trainerClass - 200u, w->actors[0].trainerId, PBRB_WORLD_TRAINER,
            t->endText[GameIndex(w)], t->endText[GameIndex(w)], 2);
        else if (s == 2) {
            PbrbWorldSetEvent(w, t->event, 1); w->defeatedTrainers[w->romMapId] |= 1;
            if (w->romMapId == 0x71)PbrbWorldSetEvent(w, WE_EVENT_BEAT_LANCE, 1);
            MainPuzzleBlocks(w); NativeOffset(w, t->afterText[GameIndex(w)], 9);
        }
        else NativeEnd(w); return 1;
    }
    case WG_CHAMPION:
        if (s == 0) { HeroPath(w); AddHero(w, 1, 3); AddHero(w, 3, 1); AddHero(w, 1, 1); w->scriptStep = 1; }
        else if (s == 1 && PathsDone(w))NativeText(w, WX_ChampionsRoomRivalIntroText, 2);
        else if (s == 2) {
            unsigned party = w->rivalStarter == 0xb1 ? 1u : w->rivalStarter == 0x99 ? 2u : 3u;
            NativeBattle(w, 43, party, PBRB_WORLD_TRAINER, Story(w, WX_RivalDefeatedText), Story(w, WX_RivalVictoryText), 3);
        }
        else if (s == 3) { PbrbWorldSetEvent(w, WE_EVENT_BEAT_CHAMPION_RIVAL, 1); NativeText(w, WX_ChampionsRoomRivalAfterBattleText, 4); }
        else if (s == 4)NativeText(w, WX_ChampionsRoomOakText, 5);
        else if (s == 5) { HideObject(w, 0x78, 1, 0); ActorPath(w, 1); AddActor(w, 1, 5); w->scriptStep = 6; }
        else if (s == 6 && PathsDone(w)) { w->facing = 2; w->actors[0].facing = 2; w->actors[1].facing = 0; NativeText(w, WX_ChampionsRoomOakCongratulatesPlayerText, 7); }
        else if (s == 7) { w->actors[1].facing = 3; NativeText(w, WX_ChampionsRoomOakDisappointedWithRivalText, 8); }
        else if (s == 8) { w->actors[1].facing = 0; NativeText(w, WX_ChampionsRoomOakComeWithMeText, 9); }
        else if (s == 9) { ActorPath(w, 1); AddActor(w, 1, 2); w->scriptStep = 10; }
        else if (s == 10 && PathsDone(w)) { HideObject(w, 0x78, 1, 1); HeroPath(w); AddHero(w, 2, 1); AddHero(w, 1, 4); w->scriptStep = 11; }
        else if (s == 11 && PathsDone(w))NativeEnd(w); return 1;
    case WG_HALL:
        if (s == 0) { HeroPath(w); AddHero(w, 1, 5); w->scriptStep = 1; }
        else if (s == 1 && PathsDone(w)) { w->facing = 3; w->actors[0].facing = 2; NativeText(w, WX_HallOfFameOakText, 2); }
        else if (s == 2) { w->scriptStep = 3; Emit(w, PBRB_WORLD_HALL_OF_FAME, 0, 0); }
        return 1;
    default:return 0;
    }
}
