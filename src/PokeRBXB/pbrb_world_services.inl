/* Native NPC service/quest dispatch. Only original IDs, event flags and ROM
   addresses are compiled; dialogue, prizes, slot positions and trade records
   stay in the verified ROM. See scripts/{Daycare,GameCorner,GameCornerPrizeRoom,
   CinnabarLabFossilRoom,FightingDojo,SilphCo7F}.asm and engine/events ASM sources.
   Persistent extension bytes:14..15 completed trades,16 rods/Lapras,
   18..19 hidden coins,20 fossil item,21 fossil species. */
#include "pbrb_service_data.h"
enum WorldServiceAction {
    WSA_TRADE = 1, WSA_ITEM, WSA_ROD, WSA_EEVEE,
    WSA_LAPRAS, WSA_DOJO_MASTER, WSA_DOJO_MON, WSA_FOSSIL, WSA_COPYCAT,
    WSA_COINS, WSA_HIDDEN_COINS
};
static unsigned WsRom(const PbrbWorld* w, unsigned id) {
    return id < PSR_COUNT ? kPbrbServiceRom[GameIndex(w)][id] : 0;
}
static void WsStart(PbrbWorld* w, int action, int actor, int param = 0) {
    NativeStart(w, WG_SERVICES, actor); w->scriptValue = action; w->battleReturn = param;
}
static void WsGiveMon(PbrbWorld* w, unsigned species, unsigned level, int next) {
    w->scriptStep = next; Emit(w, PBRB_WORLD_GIVE_MON, WsRom(w, PSR_GotMonText), (int)species);
    w->event.species = species; w->event.level = level;
}
static void WsFossilText(PbrbWorld* w, int text, int next) {
    NativeText(w, text, next, 0x04000000u); w->event.itemId = w->storyValues[20];
    w->event.species = w->storyValues[21];
}
struct WsTradeRule { unsigned char map, actor, trade; };
static const WsTradeRule WORLD_SERVICE_TRADES[] = {
    {0x56,0,0},{0x30,1,1},{0xaa,1,3},{0xc4,0,4},{0xbf,0,5},
    {0x3f,1,6},{0xa8,1,7},{0xa8,2,8},{0x47,0,9}
};
struct WsItemRule { int map, actor, item, event, before, received, full, after, sound; };
static const WsItemRule WORLD_SERVICE_ITEMS[] = {
    {0x8a,4,0x45,WE_EVENT_GOT_COIN_CASE,WX_CeladonDinerGymGuideImFlatOutBustedText,
        WX_CeladonDinerGymGuideReceivedCoinCaseText,WX_CeladonDinerGymGuideCoinCaseNoRoomText,
        WX_CeladonDinerGymGuideWinItBackText,PBRB_SFX_GET_KEY_ITEM},
    {0x7c,0,218,WE_EVENT_GOT_TM18,WX_CeladonMart3FClerkTM18PreReceiveText,
        WX_CeladonMart3FClerkReceivedTM18Text,WX_CeladonMart3FClerkTM18NoRoomText,
        WX_CeladonMart3FClerkTM18ExplanationText,PBRB_SFX_GET_ITEM},
    {0xa9,0,235,WE_EVENT_GOT_TM35,WX_CinnabarLabMetronomeRoomScientist1Text,
        WX_CinnabarLabMetronomeRoomScientist1ReceivedTM35Text,WX_CinnabarLabMetronomeRoomScientist1TM35NoRoomText,
        WX_CinnabarLabMetronomeRoomScientist1TM35ExplanationText,PBRB_SFX_GET_ITEM},
    {0xb7,0,229,WE_EVENT_GOT_TM29,WX_MrPsychicsHouseMrPsychicYouWantedThisText,
        WX_MrPsychicsHouseMrPsychicReceivedTM29Text,WX_MrPsychicsHouseMrPsychicTM29NoRoomText,
        WX_MrPsychicsHouseMrPsychicTM29ExplanationText,PBRB_SFX_GET_ITEM},
    {0xc3,0,239,WE_EVENT_GOT_TM39,WX_Route12Gate2FBrunetteGirlYouCanHaveThisText,
        WX_Route12Gate2FBrunetteGirlReceivedTM39Text,WX_Route12Gate2FBrunetteGirlTM39NoRoomText,
        WX_Route12Gate2FBrunetteGirlTM39ExplanationText,PBRB_SFX_GET_ITEM},
    {0x34,2,0x1f,WE_EVENT_GOT_OLD_AMBER,WX_Museum1FScientist2TakeThisToAPokemonLabText,
        WX_Museum1FScientist2ReceivedOldAmberText,WX_Museum1FScientist2YouDontHaveSpaceText,
        WX_Museum1FScientist2GetTheOldAmberCheckText,PBRB_SFX_GET_ITEM},
    {6,4,241,WE_EVENT_GOT_TM41,WX_CeladonCityGramps3Text,
        WX_CeladonCityGramps3ReceivedTM41Text,WX_CeladonCityGramps3TM41NoRoomText,
        WX_CeladonCityGramps3TM41ExplanationText,PBRB_SFX_GET_ITEM},
    {0xcf,0,236,WE_EVENT_GOT_TM36,WX_SilphCo2FSilphWorkerFPleaseTakeThisText,
        WX_SilphCo2FSilphWorkerFReceivedTM36Text,WX_SilphCo2FSilphWorkerFTM36NoRoomText,
        WX_SilphCo2FSilphWorkerFTM36ExplanationText,PBRB_SFX_GET_ITEM}
};
struct WsRodRule { int map, item, before, received, refused, after, full, following; };
static const WsRodRule WORLD_SERVICE_RODS[] = {
    {0xa3,0x4c,WX_VermilionOldRodHouseFishingGuruDoYouLikeToFishText,
        WX_VermilionOldRodHouseFishingGuruTakeThisText,WX_VermilionOldRodHouseFishingGuruThatsSoDisappointingText,
        WX_VermilionOldRodHouseFishingGuruHowAreTheFishBitingText,WX_VermilionOldRodHouseFishingGuruNoRoomText,
        WX_VermilionOldRodHouseFishingGuruFishingIsAWayOfLifeText},
    {0xa4,0x4d,WX_FuchsiaGoodRodHouseFishingGuruText,
        WX_FuchsiaGoodRodHouseFishingGuruReceivedGoodRodText,WX_FuchsiaGoodRodHouseFishingGuruThatsSoDisappointingText,
        WX_FuchsiaGoodRodHouseFishingGuruHowAreTheFishText,WX_FuchsiaGoodRodHouseFishingGuruNoRoomText,-1},
    {0xbd,0x4e,WX_Route12SuperRodHouseFishingGuruDoYouLikeToFishText,
        WX_Route12SuperRodHouseFishingGuruReceivedSuperRodText,WX_Route12SuperRodHouseFishingGuruThatsDisappointingText,
        WX_Route12SuperRodHouseFishingGuruTryFishingText,WX_Route12SuperRodHouseFishingGuruNoRoomText,
        WX_Route12SuperRodHouseFishingGuruFishingWayOfLifeText}
};
struct WsCoinRule { int actor, event, amount, before, received, full, after; };
static const WsCoinRule WORLD_SERVICE_COINS[] = {
    {4,WE_EVENT_GOT_10_COINS,10,WX_GameCornerFishingGuruWantToPlayText,
        WX_GameCornerFishingGuruReceived10CoinsText,WX_GameCornerFishingGuruDontNeedMyCoinsText,
        WX_GameCornerFishingGuruWinsComeAndGoText},
    {8,WE_EVENT_GOT_20_COINS_2,20,WX_GameCornerClerk2WantSomeCoinsText,
        WX_GameCornerClerk2Received20CoinsText,WX_GameCornerClerk2YouHaveLotsOfCoinsText,
        WX_GameCornerClerk2INeedMoreCoinsText},
    {9,WE_EVENT_GOT_20_COINS,20,WX_GameCornerGentlemanThrowingMeOffText,
        WX_GameCornerGentlemanReceived20CoinsText,WX_GameCornerGentlemanYouGotYourOwnCoinsText,
        WX_GameCornerGentlemanCloselyWatchTheReelsText}
};
static int WorldServicesInteraction(PbrbWorld* w, int actor, unsigned id) {
    int map = w->romMapId;
    for (unsigned i = 0; i < sizeof(WORLD_SERVICE_TRADES) / sizeof(WORLD_SERVICE_TRADES[0]); ++i)
        if (WORLD_SERVICE_TRADES[i].map == map && WORLD_SERVICE_TRADES[i].actor == actor) {
            WsStart(w, WSA_TRADE, actor, WORLD_SERVICE_TRADES[i].trade); return 1;
        }
    for (unsigned i = 0; i < sizeof(WORLD_SERVICE_ITEMS) / sizeof(WORLD_SERVICE_ITEMS[0]); ++i)
        if (WORLD_SERVICE_ITEMS[i].map == map && WORLD_SERVICE_ITEMS[i].actor == actor) { WsStart(w, WSA_ITEM, actor, (int)i); return 1; }
    for (unsigned i = 0; i < 3; ++i)if (WORLD_SERVICE_RODS[i].map == map && actor == 0) { WsStart(w, WSA_ROD, actor, (int)i); return 1; }
    if (map == 0x48 && actor == 0) { Emit(w, PBRB_WORLD_DAYCARE, 0, 0); return 1; }
    if (map == 0x87) {
        if (actor == 1) { Emit(w, PBRB_WORLD_GAME_CORNER, 0, 1); return 1; }
        for (int i = 0; i < 3; ++i)if (WORLD_SERVICE_COINS[i].actor == actor) { WsStart(w, WSA_COINS, actor, i); return 1; }
    }
    if (map == 0x89 && actor < 0 && id >= 3 && id <= 5) {
        Emit(w, PBRB_WORLD_PRIZE_MENU, 0, (int)id - 3);
        w->event.dataOffset = WsRom(w, PSR_PrizeDifferentMenuPtrs) + (id - 3u) * 4u; w->event.dataCount = 3; return 1;
    }
    if (map == 0x84 && actor == 1) { WsStart(w, WSA_EEVEE, actor); return 1; }
    if (map == 0xd4 && actor == 0) { WsStart(w, WSA_LAPRAS, actor); return 1; }
    if (map == 0xaa && actor == 0) { WsStart(w, WSA_FOSSIL, actor); return 1; }
    if (map == 0xb1) {
        if (actor == 0) { WsStart(w, WSA_DOJO_MASTER, actor); return 1; }
        if (actor == 5 || actor == 6) { WsStart(w, WSA_DOJO_MON, actor, actor == 5 ? 0x2b : 0x2c); return 1; }
    }
    if (map == 0xb0 && actor == 0) { WsStart(w, WSA_COPYCAT, actor); return 1; }
    return 0;
}
static int WorldServicesHidden(PbrbWorld* w, int x, int y) {
    if (w->romMapId != 0x87)return 0;
    unsigned table = WsRom(w, PSR_HiddenEventsFor_GAME_CORNER);
    for (unsigned i = 0; i < 48; ++i) {
        unsigned at = table + i * 6u; if (!InRom(w, at, 6))return 0;
        if (w->rom[at] != (unsigned)y || w->rom[at + 1] != (unsigned)x)continue;
        unsigned arg = w->rom[at + 2];
        if (i < 36) {
            if (arg >= 0xfd) {
                Emit(w, PBRB_WORLD_TEXT, Story(w, arg == 0xfd ? WX_GameCornerOutOfOrderText :
                    arg == 0xfe ? WX_GameCornerOutToLunchText : WX_GameCornerSomeonesKeysText), 0); return 1;
            }
            Emit(w, PBRB_WORLD_GAME_CORNER, 0, w->storyValues[0] == i + 1u ? 3 : 2); return 1;
        }
        unsigned coin = i - 36u;
        if (!HasItem(w, 0x45) || (w->storyValues[18 + coin / 8] & (1u << (coin % 8))))return 1;
        WsStart(w, WSA_HIDDEN_COINS, -1, (int)coin); return 1;
    }
    return 0;
}
static int WorldServicesIdle(PbrbWorld* w) {
    if (w->romMapId == 0xb1 && w->heroX == 4 && w->heroY == 3 &&
        !PbrbWorldCheckEvent(w, WE_EVENT_BEAT_KARATE_MASTER) &&
        !PbrbWorldCheckEvent(w, WE_EVENT_DEFEATED_FIGHTING_DOJO)) {
        w->facing = 3; NativeFace(w, 0); WsStart(w, WSA_DOJO_MASTER, 0); return 1;
    }
    return 0;
}
static void WorldServicesLoaded(PbrbWorld* w) {
    /* Original fossil timer expires only after stepping outside onto Cinnabar. */
    if (w->romMapId == 8)PbrbWorldSetEvent(w, WE_EVENT_LAB_STILL_REVIVING_FOSSIL, 0);
}
static int WorldServicesScript(PbrbWorld* w) {
    if (w->scriptId != WG_SERVICES)return 0;
    int step = w->scriptStep, param = w->battleReturn;
    if (step == 99) { NativeEnd(w); return 1; }
    switch (w->scriptValue) {
    case WSA_TRADE:
        if (step == 0) {
            unsigned at = WsRom(w, PSR_TradeMons) + (unsigned)param * 14u;
            int complete = (w->storyValues[14 + (unsigned)param / 8] & (1u << ((unsigned)param % 8))) != 0;
            w->scriptStep = 1; Emit(w, PBRB_WORLD_TRADE, 0, param, complete ? 0x02000000u : 0);
            if (InRom(w, at, 2)) { w->event.itemId = w->rom[at]; w->event.species = w->rom[at + 1]; }
        }
        else { if (w->scriptResult)w->storyValues[14 + (unsigned)param / 8] |= (unsigned char)(1u << ((unsigned)param % 8)); NativeEnd(w); }break;
    case WSA_ITEM: {
        const WsItemRule* r = WORLD_SERVICE_ITEMS + param;
        if (step == 0)NativeText(w, PbrbWorldCheckEvent(w, r->event) ? r->after : r->before,
            PbrbWorldCheckEvent(w, r->event) ? 99 : 1);
        else if (step == 1)NativeItem(w, (unsigned)r->item, 1, 2);
        else if (step == 2) {
            if (!w->scriptResult)NativeText(w, r->full, 99);
            else {
                PbrbWorldSetEvent(w, (unsigned)r->event, 1); if (r->item == 0x1f)HideObject(w, 0x34, 4, 1);
                NativeText(w, r->received, 3, PBRB_WORLD_KEEP_TEXT);
            }
        }
        else if (step == 3)NativeSound(w, r->sound, 99); break;
    }
    case WSA_ROD: {
        const WsRodRule* r = WORLD_SERVICE_RODS + param; unsigned mask = 1u << (unsigned)param;
        if (step == 0)NativeText(w, (w->storyValues[16] & mask) ? r->after : r->before, (w->storyValues[16] & mask) ? 99 : 1);
        else if (step == 1)NativeChoice(w, 2);
        else if (step == 2) { if (w->scriptResult)NativeItem(w, (unsigned)r->item, 1, 3); else NativeText(w, r->refused, 99); }
        else if (step == 3) {
            if (!w->scriptResult)NativeText(w, r->full, 99);
            else { w->storyValues[16] |= (unsigned char)mask; NativeText(w, r->received, 4, PBRB_WORLD_KEEP_TEXT); }
        }
        else if (step == 4)NativeSound(w, PBRB_SFX_GET_ITEM, r->following >= 0 ? 5 : 99);
        else if (step == 5)NativeText(w, r->following, 99); break;
    }
    case WSA_EEVEE:
        if (step == 0)WsGiveMon(w, 0x66, 25, 1);
        else { if (w->scriptResult)HideObject(w, 0x84, 1, 1); NativeEnd(w); }break;
    case WSA_LAPRAS:
        if (step == 0) {
            if (w->storyValues[16] & 8u)NativeText(w, PbrbWorldCheckEvent(w, WE_EVENT_BEAT_SILPH_CO_GIOVANNI) ?
                WX_SilphCo7FSilphWorkerM1SavedText : WX_SilphCo7FSilphWorkerM1IsOurPresidentOkText, 99);
            else NativeText(w, WX_SilphCo7FSilphWorkerM1HaveThisPokemonText, 1);
        }
        else if (step == 1)WsGiveMon(w, 0x13, 15, 2);
        else if (step == 2) { if (w->scriptResult) { w->storyValues[16] |= 8; NativeText(w, WX_SilphCo7FSilphWorkerM1LaprasDescriptionText, 99); } else NativeEnd(w); }break;
    case WSA_DOJO_MASTER:
        if (step == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_DEFEATED_FIGHTING_DOJO))NativeText(w, WX_FightingDojoKarateMasterStayAndTrainWithUsText, 99);
            else if (PbrbWorldCheckEvent(w, WE_EVENT_BEAT_KARATE_MASTER))NativeText(w, WX_FightingDojoKarateMasterIWillGiveYouAPokemonText, 99);
            else NativeText(w, WX_FightingDojoKarateMasterText, 1);
        }
        else if (step == 1)NativeBattle(w, w->actors[0].trainerClass - 200u, w->actors[0].trainerId, PBRB_WORLD_TRAINER,
            Story(w, WX_FightingDojoKarateMasterDefeatedText), Story(w, WX_FightingDojoKarateMasterDefeatedText), 2);
        else if (step == 2) {
            if (w->scriptResult == PBRB_WORLD_BATTLE_WON) {
                PbrbWorldSetEvent(w, WE_EVENT_BEAT_KARATE_MASTER, 1);
                for (unsigned i = WE_EVENT_BEAT_FIGHTING_DOJO_TRAINER_0; i <= WE_EVENT_BEAT_FIGHTING_DOJO_TRAINER_3; ++i)PbrbWorldSetEvent(w, i, 1);
                w->defeatedTrainers[0xb1] |= 31;
                NativeText(w, WX_FightingDojoKarateMasterIWillGiveYouAPokemonText, 99);
            }
            else NativeEnd(w);
        }break;
    case WSA_DOJO_MON:
        if (step == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_HITMONLEE) || PbrbWorldCheckEvent(w, WE_EVENT_GOT_HITMONCHAN))
                NativeText(w, WX_FightingDojoBetterNotGetGreedyText, 99);
            else {
                w->scriptStep = 1; Emit(w, PBRB_WORLD_STARTER_CHOICE, Story(w, param == 0x2b ?
                    WX_FightingDojoHitmonleePokeBallText : WX_FightingDojoHitmonchanPokeBallText), param);
            }
        }
        else if (step == 1) { if (w->scriptResult)WsGiveMon(w, (unsigned)param, 30, 2); else NativeEnd(w); }
        else if (step == 2) {
            if (w->scriptResult) {
                PbrbWorldSetEvent(w, param == 0x2b ? WE_EVENT_GOT_HITMONLEE : WE_EVENT_GOT_HITMONCHAN, 1);
                PbrbWorldSetEvent(w, WE_EVENT_DEFEATED_FIGHTING_DOJO, 1); HideObject(w, 0xb1, w->scriptObject, 1);
            }NativeEnd(w);
        }break;
    case WSA_FOSSIL:
        if (step == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_GAVE_FOSSIL_TO_LAB)) {
                if (PbrbWorldCheckEvent(w, WE_EVENT_LAB_STILL_REVIVING_FOSSIL))NativeText(w, WX_CinnabarLabFossilRoomScientist1GoForAWalkText, 99);
                else WsFossilText(w, WX_CinnabarLabFossilRoomScientist1FossilIsBackToLifeText, 20);
            }
            else NativeText(w, WX_CinnabarLabFossilRoomScientist1Text, 1);
        }
        else if (step == 1) {
            if (!HasItem(w, 0x29) && !HasItem(w, 0x2a) && !HasItem(w, 0x1f))
                NativeText(w, WX_CinnabarLabFossilRoomScientist1NoFossilsText, 99);
            else { w->scriptStep = 2; Emit(w, PBRB_WORLD_ITEM_CHOICE, 0, 0); w->event.dataOffset = WsRom(w, PSR_FossilsList); w->event.dataCount = 3; }
        }
        else if (step == 2) {
            if (!w->scriptResult)NativeText(w, WX_CinnabarLabFossilRoomScientist1ComeAgainText, 99);
            else {
                w->storyValues[20] = (unsigned char)w->scriptResult;
                w->storyValues[21] = (unsigned char)(w->scriptResult == 0x29 ? 0x5a : w->scriptResult == 0x2a ? 0x62 : 0xab);
                WsFossilText(w, WX_CinnabarLabFossilRoomScientist1SeesFossilText, 3);
            }
        }
        else if (step == 3)NativeChoice(w, 4);
        else if (step == 4) {
            if (w->scriptResult)WsFossilText(w, WX_CinnabarLabFossilRoomScientist1TakesFossilText, 5);
            else NativeText(w, WX_CinnabarLabFossilRoomScientist1ComeAgainText, 99);
        }
        else if (step == 5)NativeItem(w, w->storyValues[20], 1, 6, 1);
        else if (step == 6) { if (w->scriptResult)NativeText(w, WX_CinnabarLabFossilRoomScientist1GoForAWalkText2, 7); else NativeEnd(w); }
        else if (step == 7) { PbrbWorldSetEvent(w, WE_EVENT_GAVE_FOSSIL_TO_LAB, 1); PbrbWorldSetEvent(w, WE_EVENT_LAB_STILL_REVIVING_FOSSIL, 1); NativeEnd(w); }
        else if (step == 20) { PbrbWorldSetEvent(w, WE_EVENT_LAB_HANDING_OVER_FOSSIL_MON, 1); WsGiveMon(w, w->storyValues[21], 30, 21); }
        else if (step == 21) {
            if (w->scriptResult) {
                PbrbWorldSetEvent(w, WE_EVENT_GAVE_FOSSIL_TO_LAB, 0); PbrbWorldSetEvent(w, WE_EVENT_LAB_STILL_REVIVING_FOSSIL, 0);
                PbrbWorldSetEvent(w, WE_EVENT_LAB_HANDING_OVER_FOSSIL_MON, 0);
            }NativeEnd(w);
        }break;
    case WSA_COPYCAT:
        if (step == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_TM31))NativeText(w, WX_CopycatsHouse2FCopycatTM31Explanation2Text, 99);
            else NativeText(w, WX_CopycatsHouse2FCopycatDoYouLikePokemonText, 1);
        }
        else if (step == 1) { if (HasItem(w, 0x33))NativeText(w, WX_CopycatsHouse2FCopycatTM31PreReceiveText, 2); else NativeEnd(w); }
        else if (step == 2)NativeItem(w, 231, 1, 3);
        else if (step == 3) {
            if (w->scriptResult)NativeText(w, WX_CopycatsHouse2FCopycatReceivedTM31Text, 4, PBRB_WORLD_KEEP_TEXT);
            else NativeText(w, WX_CopycatsHouse2FCopycatTM31NoRoomText, 99);
        }
        else if (step == 4)NativeSound(w, PBRB_SFX_GET_ITEM, 5);
        else if (step == 5)NativeText(w, WX_CopycatsHouse2FCopycatTM31Explanation1Text, 6);
        else if (step == 6)NativeItem(w, 0x33, 1, 7, 1);
        else if (step == 7) { PbrbWorldSetEvent(w, WE_EVENT_GOT_TM31, 1); NativeEnd(w); }break;
    case WSA_COINS: {
        const WsCoinRule* r = WORLD_SERVICE_COINS + param;
        if (step == 0)NativeText(w, PbrbWorldCheckEvent(w, r->event) ? r->after : r->before,
            PbrbWorldCheckEvent(w, r->event) ? 99 : 1);
        else if (step == 1) {
            if (!HasItem(w, 0x45))NativeText(w, WX_GameCornerOopsForgotCoinCaseText, 99);
            else { w->scriptStep = 2; Emit(w, PBRB_WORLD_GIVE_COINS, 0, 0, param == 2 ? 0x06000000u : 0x04000000u); w->event.quantity = (unsigned)r->amount; }
        }
        else if (step == 2) {
            if (!w->scriptResult)NativeText(w, r->full, 99);
            else { PbrbWorldSetEvent(w, r->event, 1); NativeText(w, r->received, 3, PBRB_WORLD_KEEP_TEXT); }
        }
        else if (step == 3)NativeSound(w, PBRB_SFX_GET_ITEM, 99); break;
    }
    case WSA_HIDDEN_COINS:
        if (step == 0) {
            unsigned at = WsRom(w, PSR_HiddenEventsFor_GAME_CORNER) + (36u + (unsigned)param) * 6u;
            unsigned amount = InRom(w, at + 2, 1) ? w->rom[at + 2] - 0x3bu : 0;
            if (amount == 40)amount = 20;/* Original .bcd40 branch is unreachable. */
            w->scriptStep = 1; Emit(w, PBRB_WORLD_GIVE_COINS, 0, 0, 0x01000000u); w->event.quantity = amount;
        }
        else if (step == 1) {
            w->storyValues[18 + (unsigned)param / 8] |= (unsigned char)(1u << ((unsigned)param % 8));
            w->battleReturn = w->scriptResult; NativeText(w, w->scriptResult == 2 ? WX_FoundHiddenCoins2Text : WX_FoundHiddenCoinsText, 2, PBRB_WORLD_KEEP_TEXT);
        }
        else if (step == 2)NativeSound(w, PBRB_SFX_GET_ITEM2, 3);
        else if (step == 3) { if (param == 2)NativeText(w, WX_DroppedHiddenCoinsText, 99); else NativeEnd(w); }break;
    default:NativeEnd(w); break;
    }
    return 1;
}
