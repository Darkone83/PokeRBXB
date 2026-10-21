#include "pbrb_world_traversal_data.inl"
static void TraversalBlocks(PbrbWorld* w) {
    int raw = w->romMapId;
    if (raw == 0xa6) {
        unsigned p = WORLD_CINNABARGYMGATECOORDS[GameIndex(w)];
        for (unsigned i = 0; i < 6; ++i) {
            unsigned at = p + i * 4; ChangeBlock(w, w->rom[at], w->rom[at + 1],
                PbrbWorldCheckEvent(w, WE_EVENT_CINNABAR_GYM_GATE0_UNLOCKED + i + 1) ? 14 : w->rom[at + 2]);
        }
    }
    if (raw == 0x6c && PbrbWorldCheckEvent(w, WE_EVENT_VICTORY_ROAD_1_BOULDER_ON_SWITCH))ChangeBlock(w, 4, 6, 0x1d);
    if (raw == 0xc2) {
        if (PbrbWorldCheckEvent(w, WE_EVENT_VICTORY_ROAD_2_BOULDER_ON_SWITCH1))ChangeBlock(w, 3, 4, 0x15);
        if (PbrbWorldCheckEvent(w, WE_EVENT_VICTORY_ROAD_2_BOULDER_ON_SWITCH2))ChangeBlock(w, 11, 7, 0x1d);
    }
    if (raw == 0xc6 && PbrbWorldCheckEvent(w, WE_EVENT_VICTORY_ROAD_3_BOULDER_ON_SWITCH1))ChangeBlock(w, 3, 5, 0x1d);
}
static void TraversalLoaded(PbrbWorld* w) {
    if (w->romMapId == 0xba || w->romMapId == 0xbe)w->serviceFlags &= ~64u;
    if ((w->romMapId == 0x1b && w->heroX == 17 && (w->heroY == 10 || w->heroY == 11)) ||
        (w->romMapId == 0x1d && w->heroX == 33 && (w->heroY == 8 || w->heroY == 9))) {
        w->travelMode = 1; w->serviceFlags |= 64u;
    }
    if (w->romMapId != 0xa1 && w->romMapId != 0xa2)w->storyValues[12] = 0;
    if (w->romMapId == 0x22) {
        PbrbWorldSetEvent(w, WE_EVENT_VICTORY_ROAD_2_BOULDER_ON_SWITCH1, 0);
        PbrbWorldSetEvent(w, WE_EVENT_VICTORY_ROAD_2_BOULDER_ON_SWITCH2, 0);
        PbrbWorldSetEvent(w, WE_EVENT_VICTORY_ROAD_3_BOULDER_ON_SWITCH1, 0);
        PbrbWorldSetEvent(w, WE_EVENT_VICTORY_ROAD_3_BOULDER_ON_SWITCH2, 0); HideObject(w, 0xc6, 9, 0); HideObject(w, 0xc2, 12, 1);
    }
    if (w->romMapId == 0xc2)PbrbWorldSetEvent(w, WE_EVENT_VICTORY_ROAD_1_BOULDER_ON_SWITCH, 0);
    if (w->romMapId == 0xc0)PbrbWorldSetEvent(w, WE_EVENT_IN_SEAFOAM_ISLANDS, 1);
    if (w->romMapId == 0x1f && PbrbWorldCheckEvent(w, WE_EVENT_IN_SEAFOAM_ISLANDS)) {
        PbrbWorldSetEvent(w, WE_EVENT_IN_SEAFOAM_ISLANDS, 0);
        if (!(PbrbWorldCheckEvent(w, WE_EVENT_SEAFOAM3_BOULDER1_DOWN_HOLE) && PbrbWorldCheckEvent(w, WE_EVENT_SEAFOAM3_BOULDER2_DOWN_HOLE))) {
            HideObject(w, 0xc0, 0, 0); HideObject(w, 0xc0, 1, 0);
            HideObject(w, 0x9f, 0, 1); HideObject(w, 0x9f, 1, 1); HideObject(w, 0xa0, 0, 1); HideObject(w, 0xa0, 1, 1);
            HideObject(w, 0xa1, 4, 1); HideObject(w, 0xa1, 5, 1);
        }
        if (!(PbrbWorldCheckEvent(w, WE_EVENT_SEAFOAM4_BOULDER1_DOWN_HOLE) && PbrbWorldCheckEvent(w, WE_EVENT_SEAFOAM4_BOULDER2_DOWN_HOLE))) {
            HideObject(w, 0xa1, 1, 0); HideObject(w, 0xa1, 2, 0); HideObject(w, 0xa2, 0, 1); HideObject(w, 0xa2, 1, 1);
        }
    }
    TraversalBlocks(w);
}
static int TraversalInteraction(PbrbWorld* w, int actor, unsigned id) {
    if (w->romMapId == 0xa6 && actor >= 1 && actor <= 7) { NativeStart(w, WG_CINNABAR_TRAINER, actor); return 1; }
    if (w->romMapId == 0x92 && id == 7) { Emit(w, PBRB_WORLD_TEXT, Story(w, WX_PokemonTower5FPurifiedZoneText), 0); return 1; }
    if ((w->romMapId == 0xba || w->romMapId == 0xbe) && actor == 0) { NativeStart(w, WG_BIKE_GATE, actor); return 1; }
    return 0;
}
static int TraversalHidden(PbrbWorld* w, int x, int y) {
    if (w->romMapId == 0xa6 && w->facing == 1) {
        static const unsigned char coords[6][3] = { {15,7,1},{10,1,0},{9,7,0},{9,13,0},{1,13,1},{1,7,0} };
        for (int i = 0; i < 6; ++i)if (x == coords[i][0] && y == coords[i][1]) {
            NativeStart(w, WG_CINNABAR_QUIZ, -1); w->scriptValue = i; w->battleReturn = coords[i][2]; return 1;
        }
    }
    return 0;
}
static void TraversalArrowPath(PbrbWorld* w, unsigned p) {
    unsigned at = p, n = 0; while (n < 32 && InRom(w, at, 2) && w->rom[at] != 255) { at += 2; ++n; }
    HeroPath(w); while (n--) {
        at -= 2; unsigned mask = w->rom[at]; int d = mask == 128 ? 0 : mask == 64 ? 1 : mask == 32 ? 2 : mask == 16 ? 3 : -1;
        if (d >= 0)AddHero(w, d, w->rom[at + 1]);
    }
}
static int TraversalDungeon(PbrbWorld* w, int dest, unsigned which) {
    unsigned list = WORLD_DUNGEONWARPLIST[GameIndex(w)], data = WORLD_DUNGEONWARPDATA[GameIndex(w)];
    for (unsigned i = 0; i < 32 && InRom(w, list + i * 2, 2) && w->rom[list + i * 2] != 255; ++i) {
        if (w->rom[list + i * 2] == dest && w->rom[list + i * 2 + 1] == which) {
            unsigned p = data + i * 6;
            if (!InRom(w, p, 6))return 0; NativeEnd(w); Transition(w, PbrbWorldMapFromRom(dest), w->rom[p + 3], w->rom[p + 2], 0, WS_IDLE, 0);
            w->storyValues[12] = 1; return 1;
        }
    }
    return 0;
}
static int TraversalBoulderAt(PbrbWorld* w, int x, int y) {
    for (int i = 0; i < w->actorCount; ++i) {
        PbrbWorldActor* a = &w->actors[i];
        if (a->visible && a->sprite == 63 && !a->stepLength && a->x == x && a->y == y)return i;
    }return -1;
}
static int TraversalIdle(PbrbWorld* w) {
    int raw = w->romMapId, x = w->heroX, y = w->heroY;
    if (raw == 0x92) {
        int inside = (x == 10 || x == 11) && (y == 8 || y == 9);
        if (!inside)PbrbWorldSetEvent(w, WE_EVENT_IN_PURIFIED_ZONE, 0);
        else if (!PbrbWorldCheckEvent(w, WE_EVENT_IN_PURIFIED_ZONE)) {
            PbrbWorldSetEvent(w, WE_EVENT_IN_PURIFIED_ZONE, 1);
            NativeStart(w, WG_TOWER_HEAL, -1); return 1;
        }
    }
    for (int i = 0; i < 3; ++i)if (raw == WORLD_ARROW_TABLES[i].map) {
        unsigned p = WORLD_ARROW_TABLES[i].table[GameIndex(w)];
        for (unsigned n = 0; n < 64 && InRom(w, p, 4) && w->rom[p] != 255; ++n, p += 4)if (w->rom[p] == y && w->rom[p + 1] == x) {
            NativeStart(w, WG_SPINNER, -1); w->scriptValue = (int)Address(p / 0x4000u, Read16(w->rom + p + 2)); return 1;
        }
    }
    if (raw == 0x6c && TraversalBoulderAt(w, 17, 13) >= 0)PbrbWorldSetEvent(w, WE_EVENT_VICTORY_ROAD_1_BOULDER_ON_SWITCH, 1);
    if (raw == 0xc2) {
        if (TraversalBoulderAt(w, 1, 16) >= 0)PbrbWorldSetEvent(w, WE_EVENT_VICTORY_ROAD_2_BOULDER_ON_SWITCH1, 1);
        if (TraversalBoulderAt(w, 9, 16) >= 0)PbrbWorldSetEvent(w, WE_EVENT_VICTORY_ROAD_2_BOULDER_ON_SWITCH2, 1);
    }
    if (raw == 0xc6) {
        if (TraversalBoulderAt(w, 3, 5) >= 0)PbrbWorldSetEvent(w, WE_EVENT_VICTORY_ROAD_3_BOULDER_ON_SWITCH1, 1);
        int b = TraversalBoulderAt(w, 23, 15); if (b >= 0) {
            PbrbWorldSetEvent(w, WE_EVENT_VICTORY_ROAD_3_BOULDER_ON_SWITCH2, 1);
            HideObject(w, raw, b, 1); HideObject(w, 0xc2, 12, 0);
        }
        if (x == 23 && y == 15)return TraversalDungeon(w, 0xc2, 2);
    }
    if (raw == 0xd7 && y == 14 && (x == 16 || x == 17 || x == 19))return TraversalDungeon(w, x == 19 ? 0xd6 : 0xa5, x == 16 ? 1 : x == 17 ? 2 : 3);
    static const int seaMaps[5] = { 0xc0,0x9f,0xa0,0xa1,0xa2 };
    static const int holes[4][4] = { {17,6,24,6},{18,6,23,6},{19,6,22,6},{3,16,6,16} };
    static const unsigned ev[4][2] = { {WE_EVENT_SEAFOAM1_BOULDER1_DOWN_HOLE,WE_EVENT_SEAFOAM1_BOULDER2_DOWN_HOLE},
        {WE_EVENT_SEAFOAM2_BOULDER1_DOWN_HOLE,WE_EVENT_SEAFOAM2_BOULDER2_DOWN_HOLE},
        {WE_EVENT_SEAFOAM3_BOULDER1_DOWN_HOLE,WE_EVENT_SEAFOAM3_BOULDER2_DOWN_HOLE},
        {WE_EVENT_SEAFOAM4_BOULDER1_DOWN_HOLE,WE_EVENT_SEAFOAM4_BOULDER2_DOWN_HOLE} };
    for (int f = 0; f < 4; ++f)if (raw == seaMaps[f])for (int h = 0; h < 2; ++h) {
        int hx = holes[f][h * 2], hy = holes[f][h * 2 + 1];
        int b = TraversalBoulderAt(w, hx, hy); if (b >= 0) {
            PbrbWorldSetEvent(w, ev[f][h], 1); HideObject(w, raw, b, 1);
            HideObject(w, seaMaps[f + 1], f == 2 ? h + 4 : h, 0);
        }
        if (x == hx && y == hy)return TraversalDungeon(w, seaMaps[f + 1], (unsigned)h + 1);
    }
    if (raw == 0xa1 && w->storyValues[12]) {
        w->storyValues[12] = 0; w->travelMode = 2;
        if (!(PbrbWorldCheckEvent(w, ev[2][0]) && PbrbWorldCheckEvent(w, ev[2][1])) && (x == 18 || x == 19) && y == 7) {
            NativeStart(w, WG_CURRENT, -1); HeroPath(w); if (x == 19)AddHero(w, 2, 1); AddHero(w, 0, 4); AddHero(w, 3, 2); AddHero(w, 0, 6); return 1;
        }
    }
    if (raw == 0xa1 && x == 15 && y == 8 && !(PbrbWorldCheckEvent(w, ev[2][0]) && PbrbWorldCheckEvent(w, ev[2][1]))) {
        NativeStart(w, WG_CURRENT, -1); HeroPath(w); AddHero(w, 0, 3); AddHero(w, 3, 5); AddHero(w, 0, 6); return 1;
    }
    if (raw == 0xa2 && w->storyValues[12]) {
        w->storyValues[12] = 0; w->travelMode = 2;
        if ((x == 4 || x == 5) && y == 14 && !(PbrbWorldCheckEvent(w, ev[3][0]) && PbrbWorldCheckEvent(w, ev[3][1]))) {
            NativeStart(w, WG_CURRENT, -1); HeroPath(w); AddHero(w, 1, 1); AddHero(w, 3, x == 4 ? 3 : 2); AddHero(w, 1, 3); w->scriptValue = 1; return 1;
        }
    }
    if (raw == 0xa2 && (x == 20 || x == 21) && (y == 16 || y == 17) && !(PbrbWorldCheckEvent(w, ev[2][0]) && PbrbWorldCheckEvent(w, ev[2][1]))) {
        NativeStart(w, WG_CURRENT, -1); HeroPath(w); AddHero(w, 1, y == 17 ? 2 : 1); return 1;
    }
    if ((raw == 0xba || raw == 0xbe) && x == 4 && !HasItem(w, 6) && ((raw == 0xba && y >= 7 && y <= 10) || (raw == 0xbe && y >= 3 && y <= 6))) {
        NativeStart(w, WG_BIKE_GATE, 0); w->scriptValue = 1; return 1;
    }
    TraversalBlocks(w); return 0;
}
static int TraversalScript(PbrbWorld* w) {
    int s = w->scriptStep, a = w->scriptObject;
    switch (w->scriptId) {
    case WG_SPINNER:
        if (s == 0) {
            TraversalArrowPath(w, (unsigned)w->scriptValue); w->scriptValue = w->facing;
            w->scriptStep = 1; Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_ARROW_TILES);
        }
        else if (PathsDone(w))NativeEnd(w); return 1;
    case WG_CURRENT:
        if (PathsDone(w)) { if (w->scriptValue)w->travelMode = 0; NativeEnd(w); }return 1;
    case WG_TOWER_HEAL:
        if (s == 0) { w->scriptStep = 1; Emit(w, PBRB_WORLD_HEAL, 0, 0); }
        else if (s == 1)NativeText(w, WX_PokemonTower5FPurifiedZoneText, 2); else NativeEnd(w); return 1;
    case WG_CINNABAR_TRAINER: {
        int n = a - 1;
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_BEAT_CINNABAR_GYM_TRAINER_0 + n))NativeText(w, WORLD_CINNABAR_TRAINER_TEXT[n][2], 9);
            else NativeText(w, WORLD_CINNABAR_TRAINER_TEXT[n][0], 1);
        }
        else if (s == 1)NativeBattle(w, w->actors[a].trainerClass - 200u, w->actors[a].trainerId, PBRB_WORLD_TRAINER,
            Story(w, WORLD_CINNABAR_TRAINER_TEXT[n][1]), Story(w, WORLD_CINNABAR_TRAINER_TEXT[n][1]), 2);
        else if (s == 2) {
            PbrbWorldSetEvent(w, WE_EVENT_BEAT_CINNABAR_GYM_TRAINER_0 + n, 1);
            PbrbWorldSetEvent(w, WE_EVENT_CINNABAR_GYM_GATE0_UNLOCKED + n, 1); TraversalBlocks(w); NativeSound(w, PBRB_SFX_GO_INSIDE, 9);
        }
        else NativeEnd(w); return 1;
    }
    case WG_CINNABAR_QUIZ: {
        static const int q[6] = { WX_CinnabarQuizQuestionsText1,WX_CinnabarQuizQuestionsText2,
WX_CinnabarQuizQuestionsText3,WX_CinnabarQuizQuestionsText4,WX_CinnabarQuizQuestionsText5,WX_CinnabarQuizQuestionsText6 };
        if (s == 0)NativeText(w, WX_CinnabarGymQuizIntroText, 1);
        else if (s == 1)NativeText(w, q[w->scriptValue], 2);
        else if (s == 2)NativeChoice(w, 3);
        else if (s == 3) {
            if (w->scriptResult == w->battleReturn)NativeSound(w, PBRB_SFX_GET_ITEM, 4);
            else NativeSound(w, PBRB_SFX_DENIED, 6);
        }
        else if (s == 4)NativeText(w, WX_CinnabarGymQuizCorrectText, 5);
        else if (s == 5) {
            PbrbWorldSetEvent(w, WE_EVENT_CINNABAR_GYM_GATE0_UNLOCKED + w->scriptValue + 1, 1);
            TraversalBlocks(w); NativeSound(w, PBRB_SFX_GO_INSIDE, 9);
        }
        else if (s == 6)NativeText(w, WX_CinnabarGymQuizIncorrectText, 7);
        else if (s == 7) {
            int trainer = w->scriptValue + 2;
            if (PbrbWorldCheckEvent(w, WE_EVENT_BEAT_CINNABAR_GYM_TRAINER_0 + trainer - 1)) { NativeEnd(w); return 1; }
            w->scriptObject = trainer; ActorPath(w, trainer); AddActor(w, 2, 1); if (trainer == 3) { AddActor(w, 1, 1); w->facing = 0; }
            else w->facing = 3; w->scriptStep = 8;
        }
        else if (s == 8 && PathsDone(w)) { int actor = w->scriptObject; NativeStart(w, WG_CINNABAR_TRAINER, actor); }
        else if (s == 9)NativeEnd(w); return 1;
    }
    case WG_BIKE_GATE: {
        int west = w->romMapId == 0xba;
        if (s == 0) {
            if (w->scriptValue)NativeText(w, west ? WX_Route16Gate1FGuardWaitUpText : WX_Route18Gate1FGuardExcuseMeText, 1);
            else NativeText(w, HasItem(w, 6) ? (west ? WX_Route16Gate1FGuardCyclingRoadExplanationText : WX_Route18Gate1FGuardCyclingRoadUphillText) :
                (west ? WX_Route16Gate1FGuardNoPedestriansAllowedText : WX_Route18Gate1FGuardYouNeedABicycleText), 9);
        }
        else if (s == 1) { HeroPath(w); AddHero(w, 1, w->heroY - (west ? 7 : 3)); w->scriptStep = 2; }
        else if (s == 2 && PathsDone(w))NativeText(w, west ? WX_Route16Gate1FGuardNoPedestriansAllowedText : WX_Route18Gate1FGuardYouNeedABicycleText, 3);
        else if (s == 3) { HeroPath(w); AddHero(w, 3, 1); w->scriptStep = 4; }
        else if (s == 4) { if (PathsDone(w))NativeEnd(w); }
        else NativeEnd(w); return 1;
    }
    default:return 0;
    }
}
