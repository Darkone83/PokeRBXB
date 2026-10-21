/* Original dialogue dispatch data; IDs/offsets only, no copied media/text. */
enum WorldDialogueAction { WD_CRY = 1, WD_CHOICE, WD_KITCHEN, WD_PICTURE, WD_NOTEBOOK, WD_STATUE, WD_JIGGLY, WD_CABLE_OFFLINE };
struct WdCryRule { unsigned char map, id, species; int text, after; };
static const WdCryRule WD_CRIES[] = {
    {5,5,0x6a,WX_VermilionCityMachopText,WX_VermilionCityMachopStompingTheLandFlatText},
    {6,7,0x6f,WX_CeladonCityPoliwrathText,-1},
    {10,12,0x97,WX_SaffronCityPidgeotText,-1},
    {44,3,0x05,WX_ViridianNicknameHouseSpearowText,-1},
    {55,1,0x03,WX_PewterNidoranHouseNidoranText,-1},
    {90,3,0x54,WX_PokemonFanClubPikachuText,-1},
    {90,4,0x3a,WX_PokemonFanClubSeelText,-1},
    {93,2,0x24,WX_VermilionPidgeyHousePidgeyText,-1},
    {102,8,0x65,WX_SSAnne1FRoomsWigglytuffText,-1},
    {104,8,0x29,WX_SSAnneB1FRoomsMachokeText,-1},
    {128,1,0x4d,WX_CeladonMansion1FMeowthText,-1},
    {128,3,0x04,WX_CeladonMansion1FClefairyText,-1},
    {128,4,0x0f,WX_CeladonMansion1FNidoranFText,-1},
    {149,3,0x2f,WX_MrFujisHousePsyduckText,-1},
    {149,4,0xa7,WX_MrFujisHouseNidorinoText,-1},
    {151,1,0x11,WX_LavenderCuboneHouseCuboneText,-1},
    {175,3,0x28,WX_CopycatsHouse1FChanseyText,-1},
    {179,2,0x24,WX_SaffronPidgeyHousePidgeyText,-1},
    {188,2,0x23,WX_Route16FlyHouseFearowText,-1},
};
struct WdConditionalRule { unsigned char map, id; unsigned event; int before, after; };
static const WdConditionalRule WD_CONDITIONALS[] = {
    {45,10,WE_EVENT_BEAT_VIRIDIAN_GYM_GIOVANNI,WX_ViridianGymGuidePreBattleText,WX_ViridianGymGuidePostBattleText},
    {65,4,WE_EVENT_BEAT_MISTY,WX_CeruleanGymGymGuideChampInMakingText,WX_CeruleanGymGymGuideBeatMistyText},
    {66,3,WE_EVENT_GOT_BICYCLE,WX_BikeShopYoungsterTheseBikesAreExpensiveText,WX_BikeShopYoungsterCoolBikeText},
    {135,7,WE_EVENT_BEAT_ERIKA,WX_GameCornerGymGuideChampInMakingText,WX_GameCornerGymGuideTheyOfferRarePokemonText},
    {150,3,WE_EVENT_RESCUED_MR_FUJI,WX_LavenderMartCooltrainerMReviveText,WX_LavenderMartCooltrainerMNuggetText},
    {151,2,WE_EVENT_RESCUED_MR_FUJI,WX_LavenderCuboneHouseBrunetteGirlPoorCubonesMotherText,WX_LavenderCuboneHouseBrunetteGirlGhostIsGoneText},
    {157,8,WE_EVENT_BEAT_KOGA,WX_FuchsiaGymGymGuideChampInMakingText,WX_FuchsiaGymGymGuideBeatKogaText},
    {166,9,WE_EVENT_BEAT_BLAINE,WX_CinnabarGymGymGuideChampInMakingText,WX_CinnabarGymGymGuideBeatBlaineText},
    {178,9,WE_EVENT_BEAT_SABRINA,WX_SaffronGymGuideChampInMakingText,WX_SaffronGymGuideBeatSabrinaText},
    {208,1,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo3FSilphWorkerMWhatShouldIDoText,WX_SilphCo3FSilphWorkerMYouSavedUsText},
    {209,1,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo4FSilphWorkerMImHidingText,WX_SilphCo4FSilphWorkerMTeamRocketIsGoneText},
    {210,1,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo5FSilphWorkerMThatsYouRightText,WX_SilphCo5FSilphWorkerMYoureOurHeroText},
    {211,1,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo6FSilphWorkerM1TookOverTheBuildingText,WX_SilphCo6FSilphWorkerM1BackToWorkText},
    {211,2,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo6FSilphWorkerMHelpMePleaseText,WX_SilphCo6FSilphWorkerMWeGotEngagedText},
    {211,3,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo6FSilphWorkerF1SuchACowardText,WX_SilphCo6FSilphWorkerF1HaveToMarryHimText},
    {211,4,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo6FSilphWorkerF2TeamRocketConquerWorldText,WX_SilphCo6FSilphWorkerF2TeamRocketRanText},
    {211,5,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo6FSilphWorkerM3TargetedSilphText,WX_SilphCo6FSilphWorkerM3WorkForSilphText},
    {212,2,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo7FSilphWorkerM2AfterTheMasterBallText,WX_SilphCo7FSilphWorkerM2CancelledMasterBallText},
    {212,3,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo7FSilphWorkerM3ItWouldBeBadText,WX_SilphCo7FSilphWorkerM3YouChasedOffTeamRocketText},
    {212,4,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo7FSilphWorkerM4ItsReallyDangerousHereText,WX_SilphCo7FSilphWorkerM4SafeAtLastText},
    {213,1,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo8FSilphWorkerMSilphIsFinishedText,WX_SilphCo8FSilphWorkerMThanksForSavingUsText},
    {234,3,WE_EVENT_BEAT_SILPH_CO_GIOVANNI,WX_SilphCo10FSilphWorkerFImScaredText,WX_SilphCo10FSilphWorkerFQuietAboutMyCryingText},
};
struct WdChoiceRule { unsigned char map, id; int question, yes, no; };
static const WdChoiceRule WD_CHOICES[] = {
    {2,4,WX_PewterCitySuperNerd2DoYouKnowWhatImDoingText,WX_PewterCitySuperNerd2ThatsRightText,WX_PewterCitySuperNerd2ImSprayingRepelText},
    {4,1,WX_LavenderTownLittleGirlDoYouBelieveInGhostsText,WX_LavenderTownLittleGirlSoThereAreBelieversText,WX_LavenderTownLittleGirlHaHaGuessNotText}
};
struct WdGymNames { unsigned city[2], leader[2]; };
static const WdGymNames WD_GYM_NAMES[8] = {
    {{0x5c3adu,0x5c3adu},{0x5c3b9u,0x5c3b9u}}, /* PewterGym_Script */
    {{0x5c6d9u,0x5c6d9u},{0x5c6e7u,0x5c6e7u}}, /* CeruleanGym_Script */
    {{0x5ca55u,0x5ca55u},{0x5ca64u,0x5ca64u}}, /* VermilionGym_Script */
    {{0x48930u,0x48930u},{0x4893du,0x4893du}}, /* CeladonGym_Script */
    {{0x75465u,0x75466u},{0x75472u,0x75473u}}, /* FuchsiaGym_Script */
    {{0x5d033u,0x5d033u},{0x5d040u,0x5d040u}}, /* SaffronGym_Script */
    {{0x7577bu,0x7577cu},{0x7578bu,0x7578cu}}, /* CinnabarGymSetMapAndTiles */
    {{0x748bfu,0x748c0u},{0x748cdu,0x748ceu}}, /* ViridianGym_Script */
};
struct WdHiddenRule { unsigned char map, x, y, kind; int text; };
enum { WDH_TEXT, WDH_UP, WDH_PC, WDH_BENCH, WDH_GYM, WDH_NOTEBOOK, WDH_SCHOOL, WDH_LINK, WDH_FOSSIL, WDH_ARTICUNO, WDH_OAK_POSTER };
static const WdHiddenRule WD_HIDDEN[] = {
    {39,0,1,WDH_UP,WX_BookcaseText},
    {39,1,1,WDH_UP,WX_BookcaseText},
    {39,7,1,WDH_UP,WX_BookcaseText},
    {40,4,0,WDH_TEXT,WX_PushStartText},
    {40,5,0,WDH_OAK_POSTER,0},
    {40,0,1,WDH_UP,WX_OakLabEmailText},
    {40,1,1,WDH_UP,WX_OakLabEmailText},
    {41,0,4,WDH_BENCH,0},
    {41,13,3,WDH_PC,0},
    {43,3,4,WDH_NOTEBOOK,0},
    {43,3,0,WDH_SCHOOL,0},
    {45,15,15,WDH_GYM,0},
    {45,18,15,WDH_GYM,0},
    {52,2,3,WDH_FOSSIL,0},
    {52,2,6,WDH_FOSSIL,1},
    {54,3,10,WDH_GYM,0},
    {54,6,10,WDH_GYM,0},
    {58,0,4,WDH_BENCH,0},
    {58,13,3,WDH_PC,0},
    {64,0,4,WDH_BENCH,0},
    {64,13,3,WDH_PC,0},
    {65,3,11,WDH_GYM,0},
    {65,6,11,WDH_GYM,0},
    {141,0,4,WDH_BENCH,0},
    {141,13,3,WDH_PC,0},
    {89,13,3,WDH_PC,0},
    {89,0,4,WDH_BENCH,0},
    {92,3,14,WDH_GYM,0},
    {92,6,14,WDH_GYM,0},
    {129,0,5,WDH_PC,0},
    {133,0,4,WDH_BENCH,0},
    {133,13,3,WDH_PC,0},
    {134,3,15,WDH_GYM,0},
    {134,6,15,WDH_GYM,0},
    {140,13,3,WDH_PC,0},
    {140,0,4,WDH_BENCH,0},
    {154,13,3,WDH_PC,0},
    {154,0,4,WDH_BENCH,0},
    {157,3,15,WDH_GYM,0},
    {157,6,15,WDH_GYM,0},
    {166,17,13,WDH_GYM,0},
    {171,0,4,WDH_BENCH,0},
    {171,13,3,WDH_PC,0},
    {178,9,15,WDH_GYM,0},
    {68,0,4,WDH_BENCH,0},
    {68,13,3,WDH_PC,0},
    {81,0,4,WDH_BENCH,0},
    {81,13,3,WDH_PC,0},
    {182,0,4,WDH_BENCH,0},
    {182,13,3,WDH_PC,0},
    {223,0,4,WDH_BENCH,0},
    {223,13,3,WDH_PC,0},
    {224,0,4,WDH_BENCH,0},
    {224,13,3,WDH_PC,0},
    {225,0,4,WDH_BENCH,0},
    {225,13,3,WDH_PC,0},
    {185,1,2,WDH_ARTICUNO,0},
    {149,0,1,WDH_TEXT,WX_MagazinesText},
    {149,1,1,WDH_TEXT,WX_MagazinesText},
    {149,7,1,WDH_TEXT,WX_MagazinesText},
    {132,3,0,WDH_LINK,0},
    {132,4,0,WDH_LINK,0},
    {132,3,4,WDH_TEXT,WX_TMNotebookText},
    {177,3,9,WDH_TEXT,WX_FightingDojoText},
    {177,6,9,WDH_TEXT,WX_FightingDojoText},
    {177,4,0,WDH_TEXT,WX_EnemiesOnEverySideText},
    {177,5,0,WDH_TEXT,WX_WhatGoesAroundComesAroundText},
    {174,15,7,WDH_PC,0},
    {170,0,4,WDH_PC,0},
    {170,2,4,WDH_PC,0},
    {66,1,0,WDH_TEXT,WX_NewBicycleText},
    {66,2,1,WDH_TEXT,WX_NewBicycleText},
    {66,1,2,WDH_TEXT,WX_NewBicycleText},
    {66,3,2,WDH_TEXT,WX_NewBicycleText},
    {66,0,4,WDH_TEXT,WX_NewBicycleText},
    {66,1,5,WDH_TEXT,WX_NewBicycleText},
    {235,10,12,WDH_PC,0},
};
struct WdBenchRule { unsigned char map; int text; };
static const WdBenchRule WD_BENCHES[] = {
    {41,WX_ViridianCityPokecenterGuyText},
    {58,WX_PewterCityPokecenterGuyText},
    {64,WX_CeruleanPokecenterGuyText},
    {141,WX_LavenderPokecenterGuyText},
    {89,WX_VermilionPokecenterGuyText},
    {133,WX_CeladonCityPokecenterGuyText},
    {140,WX_CeladonCityHotelText},
    {154,WX_FuchsiaCityPokecenterGuyText},
    {171,WX_CinnabarPokecenterGuyText},
    {68,WX_MtMoonPokecenterBenchGuyText},
    {81,WX_RockTunnelPokecenterGuyText},
};
static void WdStart(PbrbWorld* w, int action, int actor, int parameter = 0) {
    NativeStart(w, WG_DIALOGUE, actor); w->scriptValue = action; w->battleReturn = parameter;
}
static int WdText(PbrbWorld* w, int label) {
    Emit(w, PBRB_WORLD_TEXT, Story(w, label), 0); return 1;
}
static int WdPicture(PbrbWorld* w, int value, unsigned species, int caption = -1, int actor = -1) {
    Emit(w, PBRB_WORLD_PICTURE, caption < 0 ? 0 : Story(w, caption), value);
    w->event.species = species; w->event.objectId = actor; return 1;
}
static int WorldDialogueInteraction(PbrbWorld* w, int actor, unsigned id) {
    const unsigned raw = (unsigned)w->romMapId;
    static const unsigned char cable[12][2] = { {41,4},{58,4},{64,4},{68,6},{81,4},{89,4},{133,4},
        {141,4},{154,4},{171,4},{174,5},{182,4} };
    for (unsigned i = 0; i < 12; ++i)if (raw == cable[i][0] && id == cable[i][1]) {
        WdStart(w, WD_CABLE_OFFLINE, actor); return 1;
    }
    for (unsigned i = 0; i < sizeof(WD_CRIES) / sizeof(*WD_CRIES); ++i)
        if (raw == WD_CRIES[i].map && id == WD_CRIES[i].id) { WdStart(w, WD_CRY, actor, (int)i); return 1; }
    for (unsigned i = 0; i < sizeof(WD_CONDITIONALS) / sizeof(*WD_CONDITIONALS); ++i) {
        const WdConditionalRule& r = WD_CONDITIONALS[i];
        if (raw == r.map && id == r.id)return WdText(w, PbrbWorldCheckEvent(w, r.event) ? r.after : r.before);
    }
    for (unsigned i = 0; i < sizeof(WD_CHOICES) / sizeof(*WD_CHOICES); ++i)
        if (raw == WD_CHOICES[i].map && id == WD_CHOICES[i].id) { WdStart(w, WD_CHOICE, actor, (int)i); return 1; }
    if (raw == 3 && id == 7) {
        unsigned n = RandomByte(w); return WdText(w, n >= 180 ? WX_CeruleanCityCooltrainerF1SlowbroUseSonicboomText :
            n >= 100 ? WX_CeruleanCityCooltrainerF1SlowbroPunchText : WX_CeruleanCityCooltrainerF1SlowbroWithdrawText);
    }
    if (raw == 3 && id == 8) {
        unsigned n = RandomByte(w); return WdText(w, n >= 180 ? WX_CeruleanCitySlowbroTookASnoozeText :
            n >= 120 ? WX_CeruleanCitySlowbroIsLoafingAroundText : n >= 60 ? WX_CeruleanCitySlowbroTurnedAwayText : WX_CeruleanCitySlowbroIgnoredOrdersText);
    }
    if (raw == 7 && id >= 19 && id <= 24) {
        static const unsigned char species[5] = { 0x28,0x06,0x02,0x25,0x13 };
        static const int texts[5] = { WX_FuchsiaCityChanseySignText,WX_FuchsiaCityVoltorbSignText,
            WX_FuchsiaCityKangaskhanSignText,WX_FuchsiaCitySlowpokeSignText,WX_FuchsiaCityLaprasSignText };
        if (id < 24)return WdPicture(w, 0, species[id - 19], texts[id - 19]);
        if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_DOME_FOSSIL))return WdPicture(w, 0, 0x62, WX_FuchsiaCityFossilSignOmanyteText);
        if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_HELIX_FOSSIL))return WdPicture(w, 0, 0x5a, WX_FuchsiaCityFossilSignKabutoText);
        return WdText(w, WX_FuchsiaCityFossilSignUndeterminedText);
    }
    if (raw == 58 && id == 3)return WdPicture(w, 6, 0, WX_PewterPokecenterJigglypuffText, actor);
    if (raw == 90 && (id == 1 || id == 2)) {
        unsigned own = id == 1 ? WE_EVENT_PIKACHU_FAN_BOAST : WE_EVENT_SEEL_FAN_BOAST;
        unsigned other = id == 1 ? WE_EVENT_SEEL_FAN_BOAST : WE_EVENT_PIKACHU_FAN_BOAST;
        int boasts = PbrbWorldCheckEvent(w, own);
        if (boasts)PbrbWorldSetEvent(w, own, 0); else PbrbWorldSetEvent(w, other, 1);
        return WdText(w, id == 1 ? (boasts ? WX_PokemonFanClubPikachuFanBetterText : WX_PokemonFanClubPikachuFanNormalText) :
            (boasts ? WX_PokemonFanClubSeelFanBetterText : WX_PokemonFanClubSeelFanNormalText));
    }
    if (raw == 92 && id == 5)return WdText(w, (w->player.badges & 4u) ? WX_VermilionGymGymGuideBeatLTSurgeText : WX_VermilionGymGymGuideChampInMakingText);
    if (raw == 100 && id == 7) { WdStart(w, WD_KITCHEN, actor); return 1; }
    if (raw == 103 && id == 5)return WdPicture(w, 0, 0x84, WX_SSAnne2FRoomsGentleman3Text);
    if (raw == 130 && id == 4) {
        if (w->player.ownedCount >= 150)return WdPicture(w, 1, 0, WX_CeladonMansion3FGameDesignerCompletedDexText);
        return WdText(w, WX_CeladonMansion3FGameDesignerText);
    }
    if (raw == 176 && id == 7)return WdText(w, w->facing == 1 ? WX_CopycatsHouse2FPCMySecretsText : WX_CopycatsHouse2FPCCantSeeText);
    if (raw == 229 && id == 1) { Emit(w, PBRB_WORLD_NAME_RATER, 0, 0); return 1; }
    if (raw == 230 && id == 1)return WdPicture(w, 3, 0);
    /* The ordinary upstairs binocular signs deliberately do nothing unless
       approached from below; the Articuno lens is a separate hidden event. */
    int text = -1;
    if (raw == 86 && id == 3)text = PbrbWorldCheckEvent(w, WE_EVENT_BEAT_ROUTE12_SNORLAX) ? WX_Route11Gate2FLeftBinocularsNoSnorlaxText : WX_Route11Gate2FLeftBinocularsSnorlaxText;
    if (raw == 86 && id == 4)text = WX_Route11Gate2FRightBinocularsText;
    if (raw == 185 && id == 2)text = WX_Route15Gate2FBinocularsText;
    if (raw == 187 && id == 3)text = WX_Route16Gate2FLeftBinocularsText;
    if (raw == 187 && id == 4)text = WX_Route16Gate2FRightBinocularsText;
    if (raw == 191 && id == 2)text = WX_Route18Gate2FLeftBinocularsText;
    if (raw == 191 && id == 3)text = WX_Route18Gate2FRightBinocularsText;
    if (raw == 195 && id == 2)text = WX_Route12Gate2FLeftBinocularsText;
    if (raw == 195 && id == 3)text = WX_Route12Gate2FRightBinocularsText;
    if (text >= 0) { if (w->facing == 1)WdText(w, text); return 1; }
    return 0;
}
static int WorldDialogueHidden(PbrbWorld* w, int x, int y) {
    for (unsigned i = 0; i < sizeof(WD_HIDDEN) / sizeof(*WD_HIDDEN); ++i) {
        const WdHiddenRule& r = WD_HIDDEN[i]; if (w->romMapId != r.map || x != r.x || y != r.y)continue;
        switch (r.kind) {
        case WDH_TEXT:return WdText(w, r.text);
        case WDH_UP:if (w->facing == 1)return WdText(w, r.text); return 1;
        case WDH_PC:if (w->facing == 1)Emit(w, PBRB_WORLD_PC, 0, 1); return 1;
        case WDH_OAK_POSTER:return WdText(w, w->player.ownedCount < 2 ? WX_SaveOptionText : WX_StrengthsAndWeaknessesText);
        case WDH_GYM: {
            if (w->facing != 1)return 1; int gym = GymIndex(w->romMapId); if (gym < 0)return 0;
            Emit(w, PBRB_WORLD_TEXT, Story(w, (w->player.badges & (1u << gym)) ? WX_GymStatueText2 : WX_GymStatueText1), 0, 0x02000000u);
            w->event.dataOffset = WD_GYM_NAMES[gym].city[GameIndex(w)]; w->event.dataCount = WD_GYM_NAMES[gym].leader[GameIndex(w)]; return 1;
        }
        case WDH_BENCH:
            /* The original misaligned-table read when approached from above
               has no meaningful text; don't perform an unbounded ROM read. */
            if (w->facing != 2)return 1;
            if (w->romMapId == 182)return WdText(w, PbrbWorldCheckEvent(w, WE_EVENT_BEAT_SILPH_CO_GIOVANNI) ? WX_SaffronCityPokecenterGuyText2 : WX_SaffronCityPokecenterGuyText1);
            for (unsigned b = 0; b < sizeof(WD_BENCHES) / sizeof(*WD_BENCHES); ++b)if (WD_BENCHES[b].map == w->romMapId)return WdText(w, WD_BENCHES[b].text);
            return 1;
        case WDH_NOTEBOOK:WdStart(w, WD_NOTEBOOK, -1); return 1;
        case WDH_SCHOOL:return WdPicture(w, 4, 0);
        case WDH_LINK:return WdPicture(w, 5, 0);
        case WDH_FOSSIL:WdStart(w, WD_PICTURE, -1, r.text); return 1;
        case WDH_ARTICUNO:if (w->facing == 1)WdStart(w, WD_PICTURE, -1, 2); return 1;
        default:return 0;
        }
    }
    if (w->facing != 1)return 0;
    unsigned tile = TileAt(w, x * 2, y * 2 + 1);
    if (w->tileset == 23 && tile == 0x30) { WdStart(w, WD_STATUE, -1); return 1; }
    if (w->tileset == 8 && tile == 0x3d) { Emit(w, PBRB_WORLD_TOWN_MAP, Story(w, WX_TownMapText), 0); return 1; }
    if ((w->tileset == 8 && tile == 0x1e) || (w->tileset == 19 && tile == 0x32) ||
        (w->tileset == 1 && tile == 0x32) || (w->tileset == 20 && tile == 0x28) ||
        ((w->tileset == 7 || w->tileset == 5) && tile == 0x1d) || (w->tileset == 12 && tile == 0x22) ||
        (w->tileset == 13 && tile == 0x36))return WdText(w, w->tileset == 19 && TileAt(w, x * 2, y * 2) == 0x38 ? WX_DiglettSculptureText : WX_PokemonBooksText);
    if (((w->tileset == 2 || w->tileset == 6) && (tile == 0x54 || tile == 0x55)) ||
        (w->tileset == 18 && (tile == 0x50 || tile == 0x52)))return WdText(w, WX_PokemonStuffText);
    return 0;
}
static int WorldDialogueScript(PbrbWorld* w) {
    if (w->scriptId != WG_DIALOGUE)return 0;
    int s = w->scriptStep, index = w->battleReturn;
    switch (w->scriptValue) {
    case WD_CRY: {
        const WdCryRule& r = WD_CRIES[index];
        if (s == 0)NativeText(w, r.text, 1, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 1) { MainCry(w, r.species, 2); w->event.flags |= r.after < 0 ? PBRB_WORLD_WAIT_BUTTON : PBRB_WORLD_KEEP_TEXT; }
        else if (s == 2 && r.after >= 0)NativeText(w, r.after, 3, PBRB_WORLD_CONTINUE_TEXT);
        else NativeEnd(w); return 1;
    }
    case WD_CHOICE: {
        const WdChoiceRule& r = WD_CHOICES[index];
        if (s == 0)NativeText(w, r.question, 1, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 1)NativeChoice(w, 2);
        else if (s == 2)NativeText(w, w->scriptResult ? r.yes : r.no, 3);
        else NativeEnd(w); return 1;
    }
    case WD_KITCHEN:
        if (s == 0)NativeText(w, WX_SSAnneKitchenCook7MainCourseIsText, 1, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 1) {
            unsigned n = RandomByte(w); NativeText(w, (n & 128u) ? WX_SSAnneKitchenCook7SalmonDuSaladText :
                (n & 16u) ? WX_SSAnneKitchenCook7EelsAuBarbecueText : WX_SSAnneKitchenCook7PrimeBeefSteakText, 2);
        }
        else NativeEnd(w); return 1;
    case WD_PICTURE:
        if (index == 2) {
            if (s == 0)NativeText(w, WX_Route15UpstairsBinocularsText, 1);
            else if (s == 1)MainCry(w, 0x4a, 2);
            else if (s == 2) { w->scriptStep = 3; WdPicture(w, 2, 0x4a); }
            else NativeEnd(w);
        }
        else {
            if (s == 0) { w->scriptStep = 1; WdPicture(w, 2, index ? 0xb6 : 0xb7); }
            else if (s == 1)NativeText(w, index ? WX_KabutopsFossilText : WX_AerodactylFossilText, 2);
            else NativeEnd(w);
        }return 1;
    case WD_STATUE:
        if (s == 0)NativeText(w, WX_IndigoPlateauStatuesText1, 1, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 1)NativeText(w, (w->heroX & 1) ? WX_IndigoPlateauStatuesText2 : WX_IndigoPlateauStatuesText3, 2);
        else NativeEnd(w); return 1;
    case WD_NOTEBOOK: {
        static const int pages[5] = { WX_ViridianSchoolNotebookText1,WX_ViridianSchoolNotebookText2,
            WX_ViridianSchoolNotebookText3,WX_ViridianSchoolNotebookText4,WX_ViridianSchoolNotebookText5 };
        if (s == 0)NativeText(w, pages[index], index < 3 ? 1 : 4, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 1)NativeText(w, WX_TurnPageText, 2, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 2)NativeChoice(w, 3);
        else if (s == 3) { if (w->scriptResult) { ++w->battleReturn; w->scriptStep = 0; } else NativeEnd(w); }
        else if (s == 4)NativeText(w, pages[4], 5);
        else NativeEnd(w); return 1;
    }
    case WD_CABLE_OFFLINE:
        if (s == 0)NativeText(w, WX_CableClubNPCWelcomeText, 1, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 1) { w->delay = PbrbWorldCheckEvent(w, WE_EVENT_GOT_POKEDEX) ? 89 : 60; w->scriptStep = 2; }
        else if (s == 2)NativeText(w, PbrbWorldCheckEvent(w, WE_EVENT_GOT_POKEDEX) ?
            WX_CableClubNPCAreaReservedFor2FriendsLinkedByCableText : WX_CableClubNPCMakingPreparationsText, 3);
        else NativeEnd(w); return 1;
    default:NativeEnd(w); return 1;
    }
}
