#include "pbrb_world_mandatory.inl"
/* Story-specific native rules. Text IDs resolve to the original ROM bodies. */
struct WorldRivalRule {
    int map, actor, event, trainer, party, pre, win, lose, after;
};
static const WorldRivalRule WORLD_RIVALS[5] = {
    {3,0,WE_EVENT_BEAT_CERULEAN_RIVAL,25,7,WX_CeruleanCityRivalPreBattleText,WX_CeruleanCityRivalDefeatedText,WX_CeruleanCityRivalVictoryText,WX_CeruleanCityRivalIWentToBillsText},
    {0x60,1,-1,42,1,WX_SSAnne2FRivalText,WX_SSAnne2FRivalDefeatedText,WX_SSAnne2FRivalVictoryText,WX_SSAnne2FRivalCutMasterText},
    {0x8f,0,WE_EVENT_BEAT_POKEMON_TOWER_RIVAL,42,4,WX_PokemonTower2FRivalWhatBringsYouHereText,WX_PokemonTower2FRivalDefeatedText,WX_PokemonTower2FRivalVictoryText,WX_PokemonTower2FRivalHowsYourDexText},
    {0xd4,8,WE_EVENT_BEAT_SILPH_CO_RIVAL,42,7,WX_SilphCo7FRivalWaitedHereText,WX_SilphCo7FRivalDefeatedText,WX_SilphCo7FRivalVictoryText,WX_SilphCo7FRivalGoodLuckToYouText},
    {0x21,1,WE_EVENT_BEAT_ROUTE22_RIVAL_2ND_BATTLE,42,10,WX_Route22RivalBeforeBattleText2,WX_Route22Rival2DefeatedText,WX_Route22Rival2VictoryText,WX_Route22RivalAfterBattleText2}
};
static int RivalDone(const PbrbWorld* w, int i) {
    return WORLD_RIVALS[i].event < 0 ? (w->serviceFlags & 2u) != 0 : PbrbWorldCheckEvent(w, (unsigned)WORLD_RIVALS[i].event);
}
static void StartRival(PbrbWorld* w, int i, int approach) {
    const WorldRivalRule* r = &WORLD_RIVALS[i]; NativeStart(w, WG_STORY_RIVAL, r->actor);
    w->scriptValue = i; w->scriptStep = approach ? -3 : 0; w->battleReturn = w->heroY;
}
static int StoryInteraction(PbrbWorld* w, int actor, unsigned id) {
    int raw = w->romMapId;
    if (MainInteraction(w, actor, id))return 1;
    for (int ri = 0; ri < 5; ++ri)if (WORLD_RIVALS[ri].map == raw && WORLD_RIVALS[ri].actor == actor) {
        if (!RivalDone(w, ri))StartRival(w, ri, 0);
        else Emit(w, PBRB_WORLD_TEXT, Story(w, WORLD_RIVALS[ri].after), 0); return 1;
    }
    if (raw == 0x58) {
        if (actor == 0) { NativeStart(w, WG_BILL_HELP, actor); return 1; }
        if (actor == 1) { NativeStart(w, WG_BILL_TICKET, actor); return 1; }
    }
    if (raw == 0x3d) {
        if (actor == 0) { NativeStart(w, WG_STORY_TRAINER, actor); w->scriptValue = 0; return 1; }
        if (actor == 5 || actor == 6) { NativeStart(w, WG_FOSSIL_PICK, actor); return 1; }
    }
    if (raw == 3 && actor == 1) { NativeStart(w, WG_STORY_TRAINER, actor); w->scriptValue = 1; return 1; }
    if (raw == 0x23 && actor == 0) { NativeStart(w, WG_NUGGET, actor); return 1; }
    if (raw == 0x65 && actor == 0) {
        NativeStart(w, WG_CAPTAIN, actor);
        if (!PbrbWorldCheckEvent(w, WE_EVENT_RUBBED_CAPTAINS_BACK))w->actors[0].facing = w->interactionFacing;
        return 1;
    }
    if (raw == 5 && actor == 2) { NativeStart(w, WG_TICKET_GATE, actor); return 1; }
    if (raw == 5 && actor == 1) {
        Emit(w, PBRB_WORLD_TEXT, Story(w, PbrbWorldCheckEvent(w, WE_EVENT_SS_ANNE_LEFT) ?
            WX_VermilionCityGambler1SSAnneDepartedText : WX_VermilionCityGambler1DidYouSeeText), 0); return 1;
    }
    if ((raw == 0x3e || raw == 0x45) && actor == 0) {
        Emit(w, PBRB_WORLD_TEXT, Story(w, HasItem(w, 228) ?
            WX_CeruleanTrashedHouseFishingGuruWhatsLostIsLostText : WX_CeruleanTrashedHouseFishingGuruTheyStoleATMText), 0); return 1;
    }
    (void)id; return 0;
}
static int StoryHidden(PbrbWorld* w, int x, int y) {
    if (MainHidden(w, x, y))return 1;
    if (w->romMapId == 0x58 && x == 1 && y == 4 && w->facing == 1) {
        if (PbrbWorldCheckEvent(w, WE_EVENT_BILL_SAID_USE_CELL_SEPARATOR) &&
            !PbrbWorldCheckEvent(w, WE_EVENT_USED_CELL_SEPARATOR_ON_BILL))NativeStart(w, WG_BILL_MACHINE, -1);
        else Emit(w, PBRB_WORLD_TEXT, Story(w, WX_BillsHouseMonitorText), 0);
        return 1;
    }
    return 0;
}
static int StoryIdle(PbrbWorld* w) {
    if (MainIdle(w))return 1;
    int raw = w->romMapId, x = w->heroX, y = w->heroY;
    if (raw == 0x24) {
        if (PbrbWorldCheckEvent(w, WE_EVENT_MET_BILL_2) && PbrbWorldCheckEvent(w, WE_EVENT_GOT_SS_TICKET) &&
            !PbrbWorldCheckEvent(w, WE_EVENT_LEFT_BILLS_HOUSE_AFTER_HELPING)) {
            PbrbWorldSetEvent(w, WE_EVENT_LEFT_BILLS_HOUSE_AFTER_HELPING, 1);
            HideObject(w, 0x23, 0, 1); HideObject(w, 0x58, 1, 1); HideObject(w, 0x58, 2, 0);
        }
        else if (!PbrbWorldCheckEvent(w, WE_EVENT_MET_BILL_2)) {
            PbrbWorldSetEvent(w, WE_EVENT_BILL_SAID_USE_CELL_SEPARATOR, 0); HideObject(w, 0x58, 0, 0);
        }
    }
    if (raw == 0x3d && x == 13 && y == 8 && !PbrbWorldCheckEvent(w, WE_EVENT_BEAT_MT_MOON_EXIT_SUPER_NERD)) {
        NativeStart(w, WG_STORY_TRAINER, 0); w->scriptValue = 0; return 1;
    }
    if (raw == 3) {
        if (x == 30 && (y == 7 || y == 9) && !PbrbWorldCheckEvent(w, WE_EVENT_BEAT_CERULEAN_ROCKET_THIEF)) {
            NativeStart(w, WG_STORY_TRAINER, 1); w->scriptValue = 1; w->facing = y == 7 ? 0 : 1; w->actors[1].facing = w->facing ^ 1; return 1;
        }
        if ((x == 20 || x == 21) && y == 6 && !RivalDone(w, 0)) { StartRival(w, 0, 1); return 1; }
    }
    if (raw == 0x23 && x == 10 && y == 15 && !PbrbWorldCheckEvent(w, WE_EVENT_GOT_NUGGET)) {
        NativeStart(w, WG_NUGGET, 0); return 1;
    }
    if (raw == 0x60 && (x == 36 || x == 37) && y == 8 && !RivalDone(w, 1)) { StartRival(w, 1, 1); return 1; }
    if (raw == 0x8f && ((x == 15 && y == 5) || (x == 14 && y == 6)) && !RivalDone(w, 2)) { StartRival(w, 2, 1); return 1; }
    if (raw == 0xd4 && x == 3 && (y == 2 || y == 3) && !RivalDone(w, 3)) { StartRival(w, 3, 1); return 1; }
    if (raw == 0x21 && x == 29 && (y == 4 || y == 5) && PbrbWorldCheckEvent(w, WE_EVENT_2ND_ROUTE22_RIVAL_BATTLE) && !RivalDone(w, 4)) {
        StartRival(w, 4, 1); return 1;
    }
    if (raw == 5 && x == 18 && y == 30 && w->facing == 0 && !(w->serviceFlags & 4u)) {
        NativeStart(w, WG_TICKET_GATE, 2); w->scriptValue = 1; return 1;
    }
    if (raw == 5 && (x != 18 || y != 30))w->serviceFlags &= ~4u;
    return 0;
}
static int StoryScript(PbrbWorld* w) {
    if (MainScript(w))return 1;
    int s = w->scriptStep, a = w->scriptObject;
    switch (w->scriptId) {
    case WG_BILL_HELP:
        if (s == 0)NativeText(w, WX_BillsHouseBillImNotAPokemonText, 1);
        else if (s == 1)NativeChoice(w, 2);
        else if (s == 2) { if (w->scriptResult)w->scriptStep = 3; else NativeText(w, WX_BillsHouseBillNoYouGottaHelpText, 3); }
        else if (s == 3)NativeText(w, WX_BillsHouseBillUseSeparationSystemText, 4);
        else if (s == 4) {
            ActorPath(w, 0);
            if (w->facing == 0) { AddActor(w, 3, 1); AddActor(w, 1, 2); AddActor(w, 2, 1); AddActor(w, 1, 1); }
            else AddActor(w, 1, 3); w->scriptStep = 5;
        }
        else if (s == 5 && PathsDone(w)) { HideObject(w, 0x58, 0, 1); PbrbWorldSetEvent(w, WE_EVENT_BILL_SAID_USE_CELL_SEPARATOR, 1); NativeEnd(w); }
        return 1;
    case WG_BILL_MACHINE:
        if (s == 0)NativeText(w, WX_BillsHouseInitiatedText, 1, PBRB_WORLD_AUTO_CLOSE);
        else if (s == 1) { NativeSound(w, PBRB_SFX_SWITCH, 2); w->delay = 76; }
        else if (s == 2) { NativeSound(w, PBRB_SFX_TINK, 3); w->delay = 32; }
        else if (s == 3) { NativeSound(w, PBRB_SFX_SHRINK, 4); w->delay = 80; }
        else if (s == 4) { NativeSound(w, PBRB_SFX_TINK, 5); w->delay = 48; }
        else if (s == 5) { NativeSound(w, PBRB_SFX_GET_ITEM, 6); w->delay = 32; }
        else if (s == 6) {
            PbrbWorldSetEvent(w, WE_EVENT_USED_CELL_SEPARATOR_ON_BILL, 1); HideObject(w, 0x58, 1, 0);
            w->actors[1].x = 1; w->actors[1].y = 2; w->delay = 8; w->scriptStep = 7;
            Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w));
        }
        else if (s == 7) { ActorPath(w, 1); AddActor(w, 0, 1); AddActor(w, 3, 3); AddActor(w, 0, 1); w->scriptStep = 8; }
        else if (s == 8 && PathsDone(w)) { PbrbWorldSetEvent(w, WE_EVENT_MET_BILL, 1); PbrbWorldSetEvent(w, WE_EVENT_MET_BILL_2, 1); NativeEnd(w); }
        return 1;
    case WG_BILL_TICKET:
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_SS_TICKET))w->scriptStep = 4;
            else NativeText(w, WX_BillsHouseBillThankYouText, 1);
        }
        else if (s == 1)NativeItem(w, 0x3f, 1, 2);
        else if (s == 2) {
            if (w->scriptResult) {
                PbrbWorldSetEvent(w, WE_EVENT_GOT_SS_TICKET, 1); HideObject(w, 3, 5, 0); HideObject(w, 3, 10, 1);
                NativeText(w, WX_SSTicketReceivedText, 3, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            }
            else NativeText(w, WX_SSTicketNoRoomText, 5);
        }
        else if (s == 3)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 4, 1);
        else if (s == 4)NativeText(w, WX_BillsHouseBillWhyDontYouGoInsteadOfMeText, 5);
        else NativeEnd(w); return 1;
    case WG_FOSSIL_PICK:
        if (s == 0)NativeText(w, a == 5 ? WX_MtMoonB2FDomeFossilYouWantText : WX_MtMoonB2FHelixFossilYouWantText, 1, PBRB_WORLD_AUTO_CLOSE);
        else if (s == 1)NativeChoice(w, 2);
        else if (s == 2) { if (!w->scriptResult) { NativeEnd(w); return 1; }NativeItem(w, a == 5 ? 0x29u : 0x2au, 1, 3); }
        else if (s == 3) {
            if (w->scriptResult) {
                HideObject(w, 0x3d, a, 1);
                PbrbWorldSetEvent(w, a == 5 ? WE_EVENT_GOT_DOME_FOSSIL : WE_EVENT_GOT_HELIX_FOSSIL, 1);
                NativeText(w, WX_MtMoonB2FReceivedFossilText, 4, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT); w->event.itemId = a == 5 ? 0x29 : 0x2a;
            }
            else NativeText(w, WX_MtMoonB2FYouHaveNoRoomText, 8);
        }
        else if (s == 4)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 5, 1);
        else if (s == 5) { ActorPath(w, 0); if (a == 5)AddActor(w, 3, 1); AddActor(w, 1, 1); w->scriptStep = 6; }
        else if (s == 6 && PathsDone(w))NativeText(w, WX_MtMoonB2FSuperNerdThenThisIsMineText, 7, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 7) { HideObject(w, 0x3d, a == 5 ? 6 : 5, 1); NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 8, 1); }
        else if (s == 8)NativeEnd(w); return 1;
    case WG_STORY_TRAINER:
        if (w->scriptValue == 0) {
            if (s == 0) {
                if (PbrbWorldCheckEvent(w, WE_EVENT_BEAT_MT_MOON_EXIT_SUPER_NERD))
                    NativeText(w, PbrbWorldCheckEvent(w, WE_EVENT_GOT_DOME_FOSSIL) || PbrbWorldCheckEvent(w, WE_EVENT_GOT_HELIX_FOSSIL) ?
                        WX_MtMoonB2FSuperNerdTheresAPokemonLabText : WX_MtMoonB2fSuperNerdEachTakeOneText, 3);
                else NativeText(w, WX_MtMoonB2FSuperNerdTheyreBothMineText, 1);
            }
            else if (s == 1)NativeBattle(w, w->actors[0].trainerClass - 200u, w->actors[0].trainerId, PBRB_WORLD_TRAINER,
                Story(w, WX_MtMoonB2FSuperNerdOkIllShareText), Story(w, WX_MtMoonB2FSuperNerdOkIllShareText), 2);
            else if (s == 2) { PbrbWorldSetEvent(w, WE_EVENT_BEAT_MT_MOON_EXIT_SUPER_NERD, 1); NativeEnd(w); }
            else NativeEnd(w);
        }
        else if (w->scriptValue == 1) {
            if (s == 0) {
                if (PbrbWorldCheckEvent(w, WE_EVENT_BEAT_CERULEAN_ROCKET_THIEF))w->scriptStep = 2;
                else NativeText(w, WX_CeruleanCityRocketText, 1);
            }
            else if (s == 1)NativeBattle(w, w->actors[a].trainerClass - 200u, w->actors[a].trainerId, PBRB_WORLD_TRAINER,
                Story(w, WX_CeruleanCityRocketIGiveUpText), Story(w, WX_CeruleanCityRocketIGiveUpText), 2);
            else if (s == 2) { PbrbWorldSetEvent(w, WE_EVENT_BEAT_CERULEAN_ROCKET_THIEF, 1); NativeText(w, WX_CeruleanCityRocketIllReturnTheTMText, 3); }
            else if (s == 3)NativeItem(w, 228, 1, 4);
            else if (s == 4) {
                if (w->scriptResult)NativeText(w, WX_CeruleanCityRocketReceivedTM28Text, 5, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
                else NativeText(w, WX_CeruleanCityRocketTM28NoRoomText, 8);
            }
            else if (s == 5)NativeSound(w, PBRB_SFX_GET_ITEM, 6);
            else if (s == 6)NativeText(w, WX_CeruleanCityRocketIBetterGetMovingText, 7);
            else if (s == 7) { HideObject(w, 3, 1, 1); NativeEnd(w); }
            else NativeEnd(w);
        }
        else NativeEnd(w); return 1;
    case WG_NUGGET:
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_NUGGET))NativeText(w, WX_Route24CooltrainerM1YouCouldBecomeATopLeaderText, 10);
            else NativeText(w, WX_Route24CooltrainerM1YouBeatOurContestText, 1, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        }
        else if (s == 1)NativeSound(w, PBRB_SFX_GET_ITEM, 2);
        else if (s == 2)NativeText(w, WX_Route24CooltrainerM1YouJustEarnedAPrizeText, 3);
        else if (s == 3)NativeItem(w, 0x31, 1, 4);
        else if (s == 4) {
            if (w->scriptResult) {
                PbrbWorldSetEvent(w, WE_EVENT_GOT_NUGGET, 1);
                NativeText(w, WX_Route24CooltrainerM1ReceivedNuggetText, 5, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            }
            else NativeText(w, WX_Route24CooltrainerM1NoRoomText, 11);
        }
        else if (s == 5)NativeSound(w, PBRB_SFX_GET_ITEM, 6, 1);
        else if (s == 6)NativeText(w, WX_Route24CooltrainerM1JoinTeamRocketText, 7);
        else if (s == 7)NativeBattle(w, w->actors[0].trainerClass - 200u, w->actors[0].trainerId, PBRB_WORLD_TRAINER,
            Story(w, WX_Route24CooltrainerM1DefeatedText), Story(w, WX_Route24CooltrainerM1DefeatedText), 8);
        else if (s == 8) { PbrbWorldSetEvent(w, WE_EVENT_BEAT_ROUTE24_ROCKET, 1); NativeText(w, WX_Route24CooltrainerM1YouCouldBecomeATopLeaderText, 10); }
        else if (s == 11) { HeroPath(w); AddHero(w, 0, 1); w->scriptStep = 12; }
        else if (s == 12) { if (PathsDone(w))NativeEnd(w); }
        else NativeEnd(w); return 1;
    case WG_CAPTAIN:
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_HM01))NativeText(w, WX_SSAnneCaptainsRoomCaptainNotSickAnymoreText, 8);
            else NativeText(w, WX_SSAnneCaptainsRoomRubCaptainsBackText, 1);
        }
        else if (s == 1) { PbrbWorldSetEvent(w, WE_EVENT_RUBBED_CAPTAINS_BACK, 1); NativeSound(w, PBRB_SFX_HEAL, 2); }
        else if (s == 2)NativeText(w, WX_SSAnneCaptainsRoomCaptainIFeelMuchBetterText, 3);
        else if (s == 3)NativeItem(w, 0xc4, 1, 4);
        else if (s == 4) {
            if (w->scriptResult) {
                PbrbWorldSetEvent(w, WE_EVENT_GOT_HM01, 1);
                NativeText(w, WX_SSAnneCaptainsRoomCaptainReceivedHM01Text, 5, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            }
            else NativeText(w, WX_SSAnneCaptainsRoomCaptainHM01NoRoomText, 8);
        }
        else if (s == 5)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 8, 1);
        else NativeEnd(w); return 1;
    case WG_TICKET_GATE:
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_SS_ANNE_LEFT))NativeText(w, WX_VermilionCitySailor1ShipSetSailText, 3);
            else if (!w->scriptValue)NativeText(w, WX_VermilionCitySailor1WelcomeToSSAnneText, 5);
            else NativeText(w, WX_VermilionCitySailor1DoYouHaveATicketText, 1);
        }
        else if (s == 1)NativeText(w, HasItem(w, 0x3f) ? WX_VermilionCitySailor1FlashedTicketText : WX_VermilionCitySailor1YouNeedATicketText, 2);
        else if (s == 2) { if (HasItem(w, 0x3f)) { w->serviceFlags |= 4u; NativeEnd(w); } else w->scriptStep = 3; }
        else if (s == 3) { if (w->scriptValue) { HeroPath(w); AddHero(w, 1, 1); w->scriptStep = 4; } else NativeEnd(w); }
        else if (s == 4) { if (PathsDone(w))NativeEnd(w); }
        else NativeEnd(w); return 1;
    case WG_STORY_RIVAL: {
        int ri = w->scriptValue; const WorldRivalRule* r = &WORLD_RIVALS[ri];
        if (s == -3) { HideObject(w, r->map, r->actor, 0); w->scriptStep = -2; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_RIVAL); }
        else if (s == -2) {
            ActorPath(w, a);
            if (ri == 0) { w->actors[a].x = w->heroX; AddActor(w, 0, 3); w->facing = 1; }
            else if (ri == 1) { AddActor(w, 0, w->heroX == 37 ? 4 : 3); w->facing = w->heroX == 37 ? 2 : 1; }
            else if (ri == 2) { w->facing = w->heroX == 15 ? 2 : 1; w->actors[a].facing = w->facing ^ 1; }
            else if (ri == 3) { w->facing = 0; NativeText(w, WX_SilphCo7FRivalText, -1); return 1; }
            else if (ri == 4) { AddActor(w, 3, w->heroY == 4 ? 4 : 3); w->facing = w->heroY == 4 ? 0 : 2; }
            w->scriptStep = 0;
        }
        else if (s == -1) { ActorPath(w, a); AddActor(w, 1, w->heroY == 2 ? 4 : 3); w->scriptStep = 0; }
        else if (s == 0 && PathsDone(w)) { w->actors[a].facing = w->facing ^ 1; NativeText(w, r->pre, 1); }
        else if (s == 1) {
            unsigned party = (unsigned)r->party + (w->rivalStarter == 0xb1 ? 0u : w->rivalStarter == 0x99 ? 1u : 2u);
            NativeBattle(w, (unsigned)r->trainer, party, PBRB_WORLD_TRAINER, Story(w, r->win), Story(w, r->lose), 2);
        }
        else if (s == 2) {
            if (r->event >= 0)PbrbWorldSetEvent(w, (unsigned)r->event, 1); else w->serviceFlags |= 2u;
            NativeText(w, r->after, 3);
        }
        else if (s == 3) {
            ActorPath(w, a);
            if (ri == 0) { AddActor(w, w->heroX == 20 ? 3 : 2, 1); AddActor(w, 0, 6); }
            else if (ri == 1) { if (w->heroX != 37) { AddActor(w, 3, 1); AddActor(w, 0, 1); }AddActor(w, 0, 4); }
            else if (ri == 2) {
                if (w->heroX == 15) { AddActor(w, 0, 2); AddActor(w, 3, 4); AddActor(w, 0, 2); }
                else { AddActor(w, 3, 1); AddActor(w, 0, 2); AddActor(w, 3, 1); AddActor(w, 0, 2); AddActor(w, 3, 2); }
            }
            else if (ri == 3) { if (w->battleReturn == 3) { AddActor(w, 3, 2); } else { AddActor(w, 2, 1); AddActor(w, 1, 2); AddActor(w, 3, 3); } }
            else { if (w->battleReturn == 4) { AddActor(w, 3, 2); AddActor(w, 0, 5); } else { AddActor(w, 1, 1); AddActor(w, 3, 3); AddActor(w, 0, 6); } }
            w->scriptStep = 4; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_RIVAL);
        }
        else if (s == 4 && PathsDone(w)) { HideObject(w, r->map, a, 1); if (ri == 4) { PbrbWorldSetEvent(w, WE_EVENT_2ND_ROUTE22_RIVAL_BATTLE, 0); PbrbWorldSetEvent(w, WE_EVENT_ROUTE22_RIVAL_WANTS_BATTLE, 0); }w->scriptStep = 5; Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w)); }
        else if (s == 5)NativeEnd(w); return 1;
    }
    default:return 0;
    }
}
