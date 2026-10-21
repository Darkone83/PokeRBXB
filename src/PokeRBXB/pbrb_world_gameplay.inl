/* Native map rules and authored story state machines. ROM text/music/object
   tables remain the source of data; no Game Boy instructions execute here. */
enum WorldNativeScript {
    WG_NONE = 0, WG_TRAINER, WG_ITEM, WG_NURSE, WG_MART_PARCEL,
    WG_OAK_PARCEL, WG_OAK_BALLS, WG_DAISY_MAP, WG_ROUTE1_SAMPLE,
    WG_OLD_MAN_GATE, WG_OLD_MAN_DEMO, WG_VIRIDIAN_QUESTION,
    WG_VIRIDIAN_TM, WG_GYM_LOCK, WG_ROUTE22_RIVAL, WG_GYM_LEADER,
    WG_GYM_GUIDE, WG_PEWTER_ESCORT, WG_FIELD, WG_FISHING,
    WG_SIMPLE_GIFT, WG_STATIC, WG_WAIT, WG_TEXT_SEQUENCE,
    WG_BILL_HELP, WG_BILL_MACHINE, WG_BILL_TICKET, WG_FOSSIL_PICK,
    WG_STORY_TRAINER, WG_NUGGET, WG_CAPTAIN, WG_TICKET_GATE,
    WG_SHIP_LEAVE, WG_STORY_RIVAL, WG_GIFT, WG_MUSEUM,
    WG_SAFFRON_GATE, WG_SAFARI, WG_FUJI, WG_ROCKET_BUTTON,
    WG_BOSS, WG_TOWER_GHOST, WG_PUZZLE, WG_LEAGUE_GATE,
    WG_ELITE, WG_CHAMPION, WG_HALL, WG_TOWER_EXIT, WG_SPINNER, WG_CURRENT, WG_TOWER_HEAL, WG_CINNABAR_TRAINER, WG_CINNABAR_QUIZ, WG_BIKE_GATE, WG_SERVICES, WG_DIALOGUE, WG_AUXILIARY, WG_HIDDEN_ITEM, WG_ITEMFINDER
};
static int WorldScriptSyncActors(const PbrbWorld* w) {
    return w->stage == WS_NATIVE_SCRIPT && (w->scriptId == WG_PEWTER_ESCORT ||
        (w->scriptId == WG_MUSEUM && w->scriptValue == 1));
}
#include "pbrb_world_gyms.inl"
static int HasItem(const PbrbWorld* w, unsigned item) {
    return item < 256 && (w->player.keyItems[item / 8] & (1u << (item & 7))) != 0;
}
static const WorldCatalogText* CatalogText(const PbrbWorld* w, unsigned id) {
    const WorldCatalogMap* m = &WORLD_CATALOG_MAPS[w->romMapId];
    if (!id || id > m->textCount)return 0;
    return &WORLD_CATALOG_TEXTS[m->textFirst + id - 1];
}
static void NativeEnd(PbrbWorld* w) {
    if (w->interactionActor >= 0 && w->interactionActor < w->actorCount &&
        w->actors[w->interactionActor].sprite < 61)
        w->actors[w->interactionActor].facing = w->interactionFacing;
    w->interactionActor = -1; w->pathActor = -1; w->stage = WS_IDLE;
    w->scriptId = w->scriptStep = 0; w->heroAnim = 0; w->wasMoving = 0;
    w->heroPathCount = w->heroPathIndex = 0; w->actorPathCount = w->actorPathIndex = 0;
    w->fieldAnimation.kind = WFA_NONE;
}
static void NativeStart(PbrbWorld* w, int script, int actor) {
    w->stage = WS_NATIVE_SCRIPT; w->scriptId = script; w->scriptStep = 0;
    w->scriptObject = actor; w->scriptValue = 0; w->scriptResult = 0; w->battleReturn = 0;
    w->heroAnim = 0; w->wasMoving = 0;
}
static void NativeText(PbrbWorld* w, int text, int next, unsigned flags = 0) {
    w->scriptStep = next; Emit(w, PBRB_WORLD_TEXT, Story(w, text), 0, flags);
}
static void NativeOffset(PbrbWorld* w, unsigned offset, int next, unsigned flags = 0) {
    w->scriptStep = next; Emit(w, PBRB_WORLD_TEXT, offset, 0, flags);
}
static void NativeItem(PbrbWorld* w, unsigned item, unsigned count, int next, int remove = 0) {
    w->scriptStep = next; Emit(w, remove ? PBRB_WORLD_TAKE_ITEM : PBRB_WORLD_GIVE_ITEM, 0, (int)item);
    w->event.itemId = item; w->event.quantity = count;
}
static void NativeSound(PbrbWorld* w, int sound, int next, int waitButton = 0) {
    w->scriptStep = next; Emit(w, PBRB_WORLD_SFX, 0, sound,
        PBRB_WORLD_WAIT_SOUND | (waitButton ? PBRB_WORLD_WAIT_BUTTON : 0));
}
static void NativeChoice(PbrbWorld* w, int next) {
    w->scriptStep = next; Emit(w, PBRB_WORLD_CHOICE, 0, 0);
}
static void NativeBattle(PbrbWorld* w, unsigned cl, unsigned id, unsigned flags,
    unsigned winText, unsigned loseText, int next) {
    w->scriptStep = next; Emit(w, PBRB_WORLD_BATTLE, 0, 0, flags);
    w->event.trainerClass = cl; w->event.trainerId = id; w->event.objectId = w->scriptObject;
    w->event.winTextOffset = winText; w->event.loseTextOffset = loseText;
    if (!(flags & PBRB_WORLD_TRAINER)) {
        w->event.species = cl; w->event.level = id;
        w->event.value = (int)cl; w->event.trainerClass = w->event.trainerId = 0;
    }
}
static int NativeFacingActor(PbrbWorld* w, int x, int y) {
    int i = ActorAt(w, x, y, -1);
    unsigned tile = TileAt(w, x * 2, y * 2 + 1);
    unsigned t = WORLD_TILESETS[GameIndex(w)] + (unsigned)w->tileset * 12u;
    if (i < 0 && InRom(w, t + 7, 3))for (unsigned k = 7; k < 10; ++k)
        if (w->rom[t + k] != 255 && tile == w->rom[t + k]) {
            i = ActorAt(w, x + DX[w->facing], y + DY[w->facing], -1); break;
        }
    return i;
}
static void NativeFace(PbrbWorld* w, int actor) {
    if (actor < 0 || actor >= w->actorCount)return;
    PbrbWorldActor* a = &w->actors[actor];
    w->interactionFacing = a->facing; w->interactionActor = actor;
    if (a->sprite < 61) { a->facing = w->facing ^ 1; a->anim = 0; }
}
static int TrainerDefeated(const PbrbWorld* w, int actor, const WorldCatalogText* t) {
    return (w->defeatedTrainers[w->romMapId] & (1u << actor)) ||
        (t && t->event < WE_NUM_EVENTS && PbrbWorldCheckEvent(w, t->event));
}
static void Unsupported(PbrbWorld* w, unsigned id, unsigned offset) {
    Emit(w, PBRB_WORLD_UNSUPPORTED, 0, (int)id); w->event.dataOffset = offset;
    w->event.dataCount = (unsigned)w->romMapId;
}
static unsigned NativeMartList(const PbrbWorld* w, unsigned script, unsigned* count) {
    /* text_mart, item-count, items..., ff */
    if (!InRom(w, script, 3) || w->rom[script] != 0xfe)return 0;
    unsigned n = w->rom[script + 1]; if (!n || n > 32 || !InRom(w, script + 2, n + 1) || w->rom[script + 2 + n] != 255)return 0;
    *count = n; return script + 2;
}
static void OpenShop(PbrbWorld* w, unsigned offset, unsigned count) {
    Emit(w, PBRB_WORLD_SHOP, 0, 0); w->event.dataOffset = offset; w->event.dataCount = count;
}
static void SyncOpeningEvents(PbrbWorld* w) {
    if (w->flags & WF_OAK_APPEARED)PbrbWorldSetEvent(w, WE_EVENT_OAK_APPEARED_IN_PALLET, 1);
    if (w->flags & WF_FOLLOWED_OAK) {
        PbrbWorldSetEvent(w, WE_EVENT_FOLLOWED_OAK_INTO_LAB, 1);
        PbrbWorldSetEvent(w, WE_EVENT_FOLLOWED_OAK_INTO_LAB_2, 1);
        PbrbWorldSetEvent(w, WE_EVENT_OAK_ASKED_TO_CHOOSE_MON, 1);
    }
    if (w->flags & WF_CHOSE_STARTER)PbrbWorldSetEvent(w, WE_EVENT_GOT_STARTER, 1);
    if (w->flags & WF_BATTLED)PbrbWorldSetEvent(w, WE_EVENT_BATTLED_RIVAL_IN_OAKS_LAB, 1);
}
static void StartNormalTrainer(PbrbWorld* w, int actor, int noticed) {
    const WorldCatalogText* t = CatalogText(w, w->actors[actor].textId);
    if (!t)return;
    NativeStart(w, WG_TRAINER, actor);
    w->scriptValue = (int)(t - WORLD_CATALOG_TEXTS);
    if (noticed) {
        w->interactionActor = -1; w->scriptStep = -3;
        w->facing = w->actors[actor].facing ^ 1;
    }
}
static int WorldServicesInteraction(PbrbWorld*, int, unsigned);
static int WorldServicesHidden(PbrbWorld*, int, int);
static int WorldServicesIdle(PbrbWorld*);
static void WorldServicesLoaded(PbrbWorld*);
static int WorldServicesScript(PbrbWorld*);
static int WorldDialogueInteraction(PbrbWorld*, int, unsigned);
static int WorldDialogueHidden(PbrbWorld*, int, int);
static int WorldDialogueScript(PbrbWorld*);
static int WorldAuxiliaryInteraction(PbrbWorld*, int, unsigned);
static int WorldAuxiliaryHidden(PbrbWorld*, int, int);
static int WorldAuxiliaryIdle(PbrbWorld*);
static void WorldAuxiliaryLoaded(PbrbWorld*);
static int WorldAuxiliaryScript(PbrbWorld*);
static void WorldAuxiliaryRender(const PbrbWorld*, PbrbGb*);
static int NativeSpecialInteraction(PbrbWorld* w, int actor, unsigned id);
static int StoryInteraction(PbrbWorld* w, int actor, unsigned id);
static int StoryHidden(PbrbWorld* w, int x, int y);
static int StoryIdle(PbrbWorld* w);
static int StoryScript(PbrbWorld* w);
#include "pbrb_world_hidden.inl"
static int GeneralInteract(PbrbWorld* w, int x, int y) {
    if (WorldHiddenItemInteraction(w, x, y))return 1;
    if (WorldServicesHidden(w, x, y) || WorldDialogueHidden(w, x, y) || WorldAuxiliaryHidden(w, x, y) || StoryHidden(w, x, y))return 1;
    int actor = NativeFacingActor(w, x, y);
    if (actor >= 0) {
        if (w->actors[actor].stepLength)return 1;
        /* The tested opening keeps its original authored interaction path. */
        if (w->mapId < 5 && !(w->mapId == PBRB_MAP_LAB && actor == 4 && (w->flags & WF_BATTLED)) &&
            !(w->mapId == PBRB_MAP_BLUE_HOUSE && actor < 2 && PbrbWorldCheckEvent(w, WE_EVENT_GOT_POKEDEX)))return 0;
        NativeFace(w, actor);
        const WorldCatalogText* t = CatalogText(w, w->actors[actor].textId);
        if (NativeSpecialInteraction(w, actor, w->actors[actor].textId))return 1;
        if (!t) { Unsupported(w, w->actors[actor].textId, 0); return 1; }
        if (t->kind == WCT_TRAINER) {
            if (TrainerDefeated(w, actor, t))Emit(w, PBRB_WORLD_TEXT, t->afterText[GameIndex(w)], 0);
            else StartNormalTrainer(w, actor, 0);
        }
        else if (t->kind == WCT_ITEM) { NativeStart(w, WG_ITEM, actor); w->scriptValue = (int)w->actors[actor].itemId; }
        else if (t->kind == WCT_SHOP)OpenShop(w, t->aux[GameIndex(w)], t->count);
        else if (t->kind == WCT_HEAL)NativeStart(w, WG_NURSE, actor);
        else if (t->kind == WCT_PC)Emit(w, PBRB_WORLD_PC, 0, 1);
        else if (t->kind == WCT_TEXT)Emit(w, PBRB_WORLD_TEXT, t->text[GameIndex(w)], 0);
        else Unsupported(w, t->id, t->script[GameIndex(w)]);
        return 1;
    }
    if (w->mapId < 5)return 0;
    for (int i = 0; i < w->signCount; ++i)if (w->signs[i].x == x && w->signs[i].y == y) {
        unsigned id = w->signs[i].textId; const WorldCatalogText* t = CatalogText(w, id);
        if (NativeSpecialInteraction(w, -1, id))return 1;
        if (t && t->kind == WCT_TEXT)Emit(w, PBRB_WORLD_TEXT, t->text[GameIndex(w)], 0);
        else Unsupported(w, id, t ? t->script[GameIndex(w)] : 0);
        return 1;
    }
    /* The common Pokemon Center PC is a tile hidden event, not an NPC. */
    if (w->tileset == 6 && w->facing == 1 && TileAt(w, x * 2, y * 2 + 1) == 0x13u) { Emit(w, PBRB_WORLD_PC, 0, 1); return 1; }
    return 0;
}
static int NativeSpecialInteraction(PbrbWorld* w, int actor, unsigned id) {
    int raw = w->romMapId;
    if (WorldServicesInteraction(w, actor, id) || WorldDialogueInteraction(w, actor, id) || WorldAuxiliaryInteraction(w, actor, id) || StoryInteraction(w, actor, id))return 1;
    if (raw == 0x28 && actor == 4 && (w->flags & WF_BATTLED)) {
        if (HasItem(w, 0x46)) { NativeStart(w, WG_OAK_PARCEL, actor); return 1; }
        if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_POKEDEX)) {
            if (!HasItem(w, 4) && PbrbWorldCheckEvent(w, WE_EVENT_BEAT_ROUTE22_RIVAL_1ST_BATTLE) &&
                !PbrbWorldCheckEvent(w, WE_EVENT_GOT_POKEBALLS_FROM_OAK))NativeStart(w, WG_OAK_BALLS, actor);
            else Emit(w, PBRB_WORLD_TEXT, Story(w, HasItem(w, 4) ? WX_OaksLabOak1ComeSeeMeSometimesText :
                WX_OaksLabOak1PokemonAroundTheWorldText), 0);
        }
        else Emit(w, PBRB_WORLD_TEXT, Story(w, WX_OaksLabOak1RaiseYourYoungPokemonText), 0);
        return 1;
    }
    if (raw == 0x27 && actor == 0 && PbrbWorldCheckEvent(w, WE_EVENT_GOT_POKEDEX)) {
        if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_TOWN_MAP))Emit(w, PBRB_WORLD_TEXT, Story(w, WX_BluesHouseDaisyUseMapText), 0);
        else NativeStart(w, WG_DAISY_MAP, actor); return 1;
    }
    if (raw == 0x27 && actor == 1) { Emit(w, PBRB_WORLD_TEXT, Story(w, WX_BluesHouseDaisyWalkingText), 0); return 1; }
    if (raw == 0x0c && actor == 0) {
        if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_POTION_SAMPLE))
            Emit(w, PBRB_WORLD_TEXT, Story(w, WX_Route1Youngster1AlsoGotPokeballsText), 0);
        else NativeStart(w, WG_ROUTE1_SAMPLE, actor); return 1;
    }
    if (raw == 0x2a && actor == 0) {
        if (PbrbWorldCheckEvent(w, WE_EVENT_OAK_GOT_PARCEL)) {
            /* ViridianMartClerkText's original secondary text table. */
            unsigned p = 0x2442u, n = 0;
            p = NativeMartList(w, p, &n);
            if (p)OpenShop(w, p, n); else Unsupported(w, id, 0);
        }
        else Emit(w, PBRB_WORLD_TEXT, Story(w, WX_ViridianMartClerkSayHiToOakText), 0);
        return 1;
    }
    if (raw == 1) {
        if (actor == 1) {
            Emit(w, PBRB_WORLD_TEXT, Story(w, (w->player.badges & 127u) == 127u ||
                PbrbWorldCheckEvent(w, WE_EVENT_BEAT_VIRIDIAN_GYM_GIOVANNI) ?
                WX_ViridianCityGambler1GymLeaderReturnedText : WX_ViridianCityGambler1GymAlwaysClosedText), 0); return 1;
        }
        if (actor == 2) { NativeStart(w, WG_VIRIDIAN_QUESTION, actor); return 1; }
        if (actor == 3) {
            Emit(w, PBRB_WORLD_TEXT, Story(w, PbrbWorldCheckEvent(w, WE_EVENT_GOT_POKEDEX) ?
                WX_ViridianCityGirlWhenIGoShopText : WX_ViridianCityGirlHasntHadHisCoffeeYetText), 0); return 1;
        }
        if (actor == 4) { NativeStart(w, WG_OLD_MAN_GATE, actor); return 1; }
        if (actor == 5) { NativeStart(w, WG_VIRIDIAN_TM, actor); return 1; }
        if (actor == 6) { NativeStart(w, WG_OLD_MAN_DEMO, actor); return 1; }
    }
    if (raw == 0x21 && actor == 0 && PbrbWorldCheckEvent(w, WE_EVENT_1ST_ROUTE22_RIVAL_BATTLE)) {
        NativeStart(w, WG_ROUTE22_RIVAL, actor); return 1;
    }
    int gym = GymIndex(raw);
    if (gym >= 0 && actor == 0) { NativeStart(w, WG_GYM_LEADER, actor); w->scriptValue = gym; return 1; }
    if (raw == 0x36 && actor == 2) { NativeStart(w, WG_GYM_GUIDE, actor); return 1; }
    if (raw == 2 && actor == 4) { NativeStart(w, WG_PEWTER_ESCORT, actor); return 1; }
    return 0;
}
static void GeneralIdle(PbrbWorld* w) {
    if (w->heroStepLength || w->event.kind || w->stage != WS_IDLE || w->heroPathIndex < w->heroPathCount)return;
    SyncOpeningEvents(w);
    if (WorldServicesIdle(w) || WorldAuxiliaryIdle(w) || StoryIdle(w))return;
    if (w->mapId == PBRB_MAP_BLUE_HOUSE)PbrbWorldSetEvent(w, WE_EVENT_ENTERED_BLUES_HOUSE, 1);
    if (w->mapId == PBRB_MAP_PALLET && !PbrbWorldCheckEvent(w, WE_EVENT_DAISY_WALKING) &&
        PbrbWorldCheckEvent(w, WE_EVENT_GOT_TOWN_MAP) && PbrbWorldCheckEvent(w, WE_EVENT_ENTERED_BLUES_HOUSE)) {
        PbrbWorldSetEvent(w, WE_EVENT_DAISY_WALKING, 1); HideObject(w, 0x27, 0, 1); HideObject(w, 0x27, 1, 0);
    }
    if (w->romMapId == 0x2a && !PbrbWorldCheckEvent(w, WE_EVENT_GOT_OAKS_PARCEL)) {
        NativeStart(w, WG_MART_PARCEL, 0); return;
    }
    if (w->romMapId == 1) {
        if (!PbrbWorldCheckEvent(w, WE_EVENT_GOT_POKEDEX) && w->heroX == 19 && w->heroY == 9) {
            NativeStart(w, WG_OLD_MAN_GATE, 4); return;
        }
        if ((w->player.badges & 127u) == 127u)PbrbWorldSetEvent(w, WE_EVENT_VIRIDIAN_GYM_OPEN, 1);
        if (w->heroX == 32 && w->heroY == 8 && !PbrbWorldCheckEvent(w, WE_EVENT_VIRIDIAN_GYM_OPEN)) {
            NativeStart(w, WG_GYM_LOCK, -1); return;
        }
    }
    if (w->romMapId == 0x21 && PbrbWorldCheckEvent(w, WE_EVENT_ROUTE22_RIVAL_WANTS_BATTLE) &&
        PbrbWorldCheckEvent(w, WE_EVENT_1ST_ROUTE22_RIVAL_BATTLE) && w->heroX == 29 && (w->heroY == 4 || w->heroY == 5)) {
        NativeStart(w, WG_ROUTE22_RIVAL, 0); w->scriptStep = -2; return;
    }
    if (w->romMapId == 2 && !PbrbWorldCheckEvent(w, WE_EVENT_BEAT_BROCK) &&
        ((w->heroY == 17 && (w->heroX == 35 || w->heroX == 36)) || (w->heroX == 37 && (w->heroY == 18 || w->heroY == 19)))) {
        NativeStart(w, WG_PEWTER_ESCORT, 4); return;
    }
    if (!w->player.usableCount)return;
    for (int i = 0; i < w->actorCount; ++i) {
        PbrbWorldActor* a = &w->actors[i]; if (!a->visible || a->stepLength || a->trainerClass < 201)continue;
        const WorldCatalogText* t = CatalogText(w, a->textId);
        if (!t || t->kind != WCT_TRAINER || TrainerDefeated(w, i, t))continue;
        int dx = w->heroX - a->x, dy = w->heroY - a->y, dist = Abs(dx) + Abs(dy);
        if (dist<1 || dist>(int)t->view || (dx && dy) || dx != DX[a->facing] * dist || dy != DY[a->facing] * dist)continue;
        int clear = 1; for (int s = 1; s < dist; ++s) {
            int x = a->x + DX[a->facing] * s, y = a->y + DY[a->facing] * s;
            if (!Passable(w, x, y) || ActorAt(w, x, y, i) >= 0) { clear = 0; break; }
        }
        if (clear) { StartNormalTrainer(w, i, 1); return; }
    }
}
static void GeneralStep(PbrbWorld* w) {
    if (w->safariActive && w->safariSteps > 0)--w->safariSteps;
    int leftWater = w->travelMode == 2 && !SurfWaterTile(w, TileAt(w, w->heroX * 2, w->heroY * 2 + 1));
    if (leftWater)w->travelMode = 0;
    GeneralIdle(w); if (w->event.kind || w->stage != WS_IDLE)return;
    if (leftWater) { Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w)); return; }
    if (w->encounterCooldown) { --w->encounterCooldown; return; }
    if (!w->player.usableCount || PbrbWorldCheckEvent(w, WE_EVENT_IN_PURIFIED_ZONE))return;
    if (w->romMapId == 0x3d && w->heroX >= 11 && w->heroX <= 14 && w->heroY >= 5 && w->heroY <= 8 && PbrbWorldCheckEvent(w, WE_EVENT_BEAT_MT_MOON_EXIT_SUPER_NERD))return;
    unsigned pointer = 0xceebu + (unsigned)w->romMapId * 2u;
    if (!InRom(w, pointer, 2))return;
    unsigned data = Address(3, Read16(w->rom + pointer)); if (!InRom(w, data, 1))return;
    unsigned grassRate = w->rom[data++], grass = data;
    if (grassRate)data += 20;
    if (!InRom(w, data, 1))return;
    unsigned waterRate = w->rom[data++], water = data;
    unsigned left = TileAt(w, w->heroX * 2, w->heroY * 2 + 1), right = TileAt(w, w->heroX * 2 + 1, w->heroY * 2 + 1);
    unsigned rate = 0;
    if (right == (unsigned)w->grassTile)rate = grassRate;
    else if (right == 0x14u)rate = waterRate;
    else if (w->romMapId >= 0x25 && w->tileset != 3)rate = grassRate;
    if (!rate || RandomByte(w) >= rate)return;
    unsigned table = left == 0x14u ? water : grass; if (!InRom(w, table, 20))return;
    static const unsigned threshold[10] = { 50,101,140,165,190,215,228,241,252,255 };
    unsigned r = RandomByte(w), slot = 0; while (slot<9 && r>threshold[slot])++slot;
    unsigned level = w->rom[table + slot * 2], species = w->rom[table + slot * 2 + 1];
    if (!species || species > 190 || !level || level > 100 || (w->player.repelSteps && w->player.leadLevel > level))return;
    NativeStart(w, WG_WAIT, -1); w->scriptStep = 1;
    unsigned battleFlags = PBRB_WORLD_WILD;
    if (w->safariActive)battleFlags |= PBRB_WORLD_SAFARI_BATTLE;
    if (w->romMapId >= 0x90 && w->romMapId <= 0x94 && !HasItem(w, 0x48))battleFlags |= PBRB_WORLD_GHOST | PBRB_WORLD_NO_CATCH;
    NativeBattle(w, species, level, battleFlags, 0, 0, 1); w->encounterCooldown = 3;
}
static void NativeParcelDone(PbrbWorld* w) {
    PbrbWorldSetEvent(w, WE_EVENT_GOT_POKEDEX, 1); PbrbWorldSetEvent(w, WE_EVENT_OAK_GOT_PARCEL, 1);
    PbrbWorldSetEvent(w, WE_EVENT_1ST_ROUTE22_RIVAL_BATTLE, 1);
    PbrbWorldSetEvent(w, WE_EVENT_2ND_ROUTE22_RIVAL_BATTLE, 0);
    PbrbWorldSetEvent(w, WE_EVENT_ROUTE22_RIVAL_WANTS_BATTLE, 1);
    HideObject(w, 1, 4, 1); HideObject(w, 1, 6, 0); HideObject(w, 0x21, 0, 0);
}
static int ChangeBlock(PbrbWorld* w, int x, int y, unsigned block) {
    if (x < 0 || y < 0 || x >= w->widthBlocks || y >= w->heightBlocks || block>255)return 0;
    for (int i = 0; i < w->blockChangeCount; ++i)if (w->blockChanges[i].x == x && w->blockChanges[i].y == y) {
        w->blockChanges[i].block = (unsigned char)block; return 1;
    }
    if (w->blockChangeCount >= 64)return 0;
    PbrbWorldBlockChange* b = &w->blockChanges[w->blockChangeCount++];
    b->x = (unsigned char)x; b->y = (unsigned char)y; b->block = (unsigned char)block; return 1;
}
static int EscapeAllowed(const PbrbWorld* w) {
    return w->romMapId != 0xf7 && (w->tileset == 3 || w->tileset == 15 || w->tileset == 16 || w->tileset == 17 || w->tileset == 22);
}
static int FieldWater(const PbrbWorld* w) {
    static const unsigned char waterSets[9] = { 0,3,5,7,13,14,17,22,23 }; int valid = 0;
    for (int i = 0; i < 9; ++i)if (w->tileset == waterSets[i])valid = 1;
    int x = w->heroX + DX[w->facing], y = w->heroY + DY[w->facing];
    /* GetTileAndCoordsInFrontOfPlayer samples the lower-left background
       tile, not the lower-right encounter tile. Fishing uses this same
       shore test without a sprite collision check. */
    unsigned tile = TileAt(w, x * 2, y * 2 + 1);
    return valid && (tile == 0x14u || (w->tileset != 14 && (tile == 0x32u || tile == 0x48u)));
}
#include "pbrb_world_progression.inl"
#include "pbrb_world_services.inl"
#include "pbrb_world_dialogue.inl"
#include "pbrb_world_auxiliary.inl"
static void GeneralScript(PbrbWorld* w) {
    if (WorldHiddenItemScript(w))return;
    if (WorldServicesScript(w) || WorldDialogueScript(w) || WorldAuxiliaryScript(w) || StoryScript(w))return;
    int s = w->scriptStep, g = GameIndex(w), a = w->scriptObject;
    switch (w->scriptId) {
    case WG_TRAINER: {
        const WorldCatalogText* t = &WORLD_CATALOG_TEXTS[w->scriptValue];
        if (a < 0 || a >= w->actorCount) { NativeEnd(w); break; }
        if (s == -3) {
            w->scriptStep = -2; unsigned cl = w->actors[a].trainerClass - 200u;
            Emit(w, PBRB_WORLD_MUSIC, 0, cl == 30 ? PBRB_MUSIC_MEET_EVIL :
                cl == 3 || cl == 4 || cl == 6 || cl == 18 || cl == 33 ? PBRB_MUSIC_MEET_FEMALE : PBRB_MUSIC_MEET_MALE);
        }
        else if (s == -2) { w->scriptStep = -1; w->delay = 30; Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_TRAINER_APPEARED); }
        else if (s == -1) { Approach(w, a, w->heroX - DX[w->actors[a].facing], w->heroY - DY[w->actors[a].facing]); w->scriptStep = 0; }
        else if (s == 0 && PathsDone(w))NativeOffset(w, t->text[g], 1);
        else if (s == 1 && w->actors[a].trainerClass < 200 && t->count)MainCry(w, w->actors[a].trainerClass, 4);
        else if (s == 1 || s == 4) {
            unsigned cl = w->actors[a].trainerClass;
            NativeBattle(w, cl >= 200 ? cl - 200 : cl, w->actors[a].trainerId, cl >= 200 ? PBRB_WORLD_TRAINER : PBRB_WORLD_STATIC,
                cl >= 200 ? t->endText[g] : 0, cl >= 200 ? t->endText[g] : 0, 2);
        }
        else if (s == 2) {
            if (w->scriptResult != PBRB_WORLD_BATTLE_LOST) {
                w->defeatedTrainers[w->romMapId] |= (unsigned short)(1u << a);
                if (t->event < WE_NUM_EVENTS)PbrbWorldSetEvent(w, t->event, 1);
                if (w->actors[a].trainerClass < 200)HideObject(w, w->romMapId, a, 1);
            }
            if (w->romMapId == 0x94) { NativeStart(w, WG_TOWER_EXIT, a); }
            else NativeEnd(w);
        }
        break;
    }
    case WG_ITEM:
        if (s == 0)NativeItem(w, (unsigned)w->scriptValue, 1, 1);
        else if (s == 1) {
            if (w->scriptResult) {
                HideObject(w, w->romMapId, a, 1);
                NativeText(w, WX_FoundItemText, 2, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
                w->event.itemId = (unsigned)w->scriptValue;
            }
            else NativeText(w, WX_NoMoreRoomForItemText, 3);
        }
        else if (s == 2)NativeSound(w, PBRB_SFX_GET_ITEM, 3, 1);
        else NativeEnd(w); break;
    case WG_NURSE:
        if (s == 0)NativeText(w, WX_PokemonCenterWelcomeText, 1);
        else if (s == 1) { if (!(w->serviceFlags & 1u))NativeText(w, WX_ShallWeHealYourPokemonText, 2); else w->scriptStep = 2; }
        else if (s == 2)NativeChoice(w, 3);
        else if (s == 3) {
            w->serviceFlags |= 1u;
            if (w->scriptResult)NativeText(w, WX_NeedYourPokemonText, 4); else w->scriptStep = 7;
        }
        else if (s == 4) {
            w->respawnMap = PbrbWorldMapFromRom((unsigned)w->lastMapRom);
            w->respawnX = w->lastOutdoorX; w->respawnY = w->lastOutdoorY + 1;
            w->scriptStep = 5; Emit(w, PBRB_WORLD_HEAL, 0, 1, PBRB_WORLD_WAIT_SOUND);
        }
        else if (s == 5)NativeText(w, WX_PokemonFightingFitText, 6);
        else if (s == 6) { w->delay = 20; w->scriptStep = 7; }
        else if (s == 7)NativeText(w, WX_PokemonCenterFarewellText, 8);
        else NativeEnd(w); break;
    case WG_MART_PARCEL:
        if (s == 0)NativeText(w, WX_ViridianMartClerkYouCameFromPalletTownText, 1);
        else if (s == 1) { HeroPath(w); AddHero(w, 2, 1); AddHero(w, 1, 2); w->scriptStep = 2; }
        else if (s == 2 && PathsDone(w)) { w->delay = 3; w->scriptStep = 3; }
        else if (s == 3)NativeText(w, WX_ViridianMartClerkParcelQuestText, 4, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 4)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 5, 1);
        else if (s == 5)NativeItem(w, 0x46, 1, 6);
        else if (s == 6) {
            if (w->scriptResult) { PbrbWorldSetEvent(w, WE_EVENT_GOT_OAKS_PARCEL, 1); NativeEnd(w); }
            else NativeText(w, WX_NoMoreRoomForItemText, 7);
        }
        else if (s == 7) { PbrbWorldSetEvent(w, WE_EVENT_GOT_OAKS_PARCEL, 1); NativeEnd(w); }break;
    case WG_OAK_PARCEL:
        if (s == 0)NativeText(w, WX_OaksLabOak1DeliverParcelText, 1, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 1)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 2);
        else if (s == 2)NativeText(w, WX_OaksLabOak1ParcelThanksText, 3);
        else if (s == 3)NativeItem(w, 0x46, 1, 4, 1);
        else if (s == 4) { w->scriptStep = 5; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_RIVAL); }
        else if (s == 5)NativeText(w, WX_OaksLabRivalGrampsText, 6);
        else if (s == 6) {
            int dy = w->heroY == 3 ? 4 : w->heroY == 1 ? 2 : 3;
            w->scriptValue = dy; HideObject(w, 0x28, 0, 0); w->actors[0].x = w->heroY == 2 ? (w->heroX == 4 ? 5 : 3) : 4;
            w->actors[0].y = 3 + dy; ActorPath(w, 0); AddActor(w, 1, dy); w->scriptStep = 7;
        }
        else if (s == 7 && PathsDone(w)) {
            w->actors[0].facing = 1; w->actors[4].facing = 0;
            w->scriptStep = 8; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_LAB);
        }
        else if (s == 8)NativeText(w, WX_OaksLabRivalWhatDidYouCallMeForText, 9);
        else if (s == 9)NativeText(w, WX_OaksLabOakIHaveARequestText, 10);
        else if (s == 10)NativeText(w, WX_OaksLabOakMyInventionPokedexText, 11);
        else if (s == 11) {
            HideObject(w, 0x28, 5, 1); HideObject(w, 0x28, 6, 1);
            NativeText(w, WX_OaksLabOakGotPokedexText, 12, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        }
        else if (s == 12)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 13, 1);
        else if (s == 13) { NativeParcelDone(w); w->scriptStep = 14; Emit(w, PBRB_WORLD_POKEDEX, 0, 1); }
        else if (s == 14)NativeText(w, WX_OaksLabOakThatWasMyDreamText, 15);
        else if (s == 15) { w->actors[0].facing = 3; NativeText(w, WX_OaksLabRivalLeaveItAllToMeText, 16); }
        else if (s == 16) {
            ActorPath(w, 0); AddActor(w, 0, w->scriptValue); w->scriptStep = 17;
            Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_RIVAL);
        }
        else if (s == 17 && PathsDone(w)) {
            HideObject(w, 0x28, 0, 1); w->scriptStep = 18;
            Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_LAB);
        }
        else if (s == 18)NativeEnd(w); break;
    case WG_OAK_BALLS:
        if (s == 0) { PbrbWorldSetEvent(w, WE_EVENT_GOT_POKEBALLS_FROM_OAK, 1); NativeItem(w, 4, 5, 1); }
        else if (s == 1)NativeText(w, WX_OaksLabOak1ReceivedPokeballsText, 2, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        else if (s == 2)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 3);
        else if (s == 3)NativeText(w, WX_OaksLabGivePokeballsExplanationText, 4);
        else NativeEnd(w); break;
    case WG_DAISY_MAP:
        if (s == 0)NativeText(w, WX_BluesHouseDaisyOfferMapText, 1);
        else if (s == 1)NativeItem(w, 5, 1, 2);
        else if (s == 2) {
            if (w->scriptResult) {
                PbrbWorldSetEvent(w, WE_EVENT_GOT_TOWN_MAP, 1); HideObject(w, 0x27, 2, 1);
                NativeText(w, WX_GotMapText, 3, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            }
            else NativeText(w, WX_BluesHouseDaisyBagFullText, 4);
        }
        else if (s == 3)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 4, 1);
        else NativeEnd(w); break;
    case WG_ROUTE1_SAMPLE:
        if (s == 0) { PbrbWorldSetEvent(w, WE_EVENT_GOT_POTION_SAMPLE, 1); NativeText(w, WX_Route1Youngster1MartSampleText, 1); }
        else if (s == 1)NativeItem(w, 0x14, 1, 2);
        else if (s == 2) {
            if (w->scriptResult)NativeText(w, WX_Route1Youngster1GotPotionText, 3, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            else NativeText(w, WX_Route1Youngster1NoRoomText, 4);
        }
        else if (s == 3)NativeSound(w, PBRB_SFX_GET_ITEM, 4, 1);
        else NativeEnd(w); break;
    case WG_OLD_MAN_GATE:
    case WG_GYM_LOCK:
        if (s == 0)NativeText(w, w->scriptId == WG_GYM_LOCK ? WX_ViridianCityGymLockedText : WX_ViridianCityOldManSleepyPrivatePropertyText, 1);
        else if (s == 1) { HeroPath(w); AddHero(w, 0, 1); w->scriptStep = 2; }
        else if (s == 2 && PathsDone(w))NativeEnd(w); break;
    case WG_VIRIDIAN_QUESTION:
        if (s == 0)NativeText(w, WX_ViridianCityYoungster2YouWantToKnowAboutText, 1);
        else if (s == 1)NativeChoice(w, 2);
        else if (s == 2)NativeText(w, w->scriptResult ? WX_ViridianCityYoungster2CaterpieAndWeedleDescriptionText : WX_ViridianCityYoungster2OkThenText, 3);
        else NativeEnd(w); break;
    case WG_VIRIDIAN_TM:
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_TM42))NativeText(w, WX_ViridianCityFisherTM42ExplanationText, 4);
            else NativeText(w, WX_ViridianCityFisherYouCanHaveThisText, 1);
        }
        else if (s == 1)NativeItem(w, 242, 1, 2);
        else if (s == 2) {
            if (w->scriptResult) {
                PbrbWorldSetEvent(w, WE_EVENT_GOT_TM42, 1);
                NativeText(w, WX_ViridianCityFisherReceivedTM42Text, 3, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            }
            else NativeText(w, WX_ViridianCityFisherTM42NoRoomText, 4);
        }
        else if (s == 3)NativeSound(w, PBRB_SFX_GET_ITEM2, 4, 1);
        else NativeEnd(w); break;
    case WG_OLD_MAN_DEMO:
        if (s == 0)NativeText(w, WX_ViridianCityOldManHadMyCoffeeNowText, 1);
        else if (s == 1)NativeChoice(w, 2);
        else if (s == 2) {
            if (w->scriptResult)NativeText(w, WX_ViridianCityOldManTimeIsMoneyText, 5);
            else NativeText(w, WX_ViridianCityOldManKnowHowToCatchPokemonText, 3);
        }
        else if (s == 3)NativeBattle(w, 0x70, 5, PBRB_WORLD_WILD | PBRB_WORLD_CATCH_DEMO, 0, 0, 4);
        else if (s == 4)NativeText(w, WX_ViridianCityOldManYouNeedToWeakenTheTargetText, 5);
        else NativeEnd(w); break;
    case WG_ROUTE22_RIVAL:
        if (s == -2) { w->scriptStep = -1; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_RIVAL); }
        else if (s == -1) {
            w->scriptValue = w->heroY; ActorPath(w, 0); AddActor(w, 3, w->heroY == 4 ? 4 : 3);
            w->facing = w->heroY == 4 ? 0 : 2; w->scriptStep = 0;
        }
        else if (s == 0 && PathsDone(w))NativeText(w, WX_Route22RivalBeforeBattleText1, 1);
        else if (s == 1)NativeBattle(w, 25, w->rivalStarter == 0xb1 ? 4 : w->rivalStarter == 0x99 ? 5 : 6, PBRB_WORLD_TRAINER,
            Story(w, WX_Route22Rival1DefeatedText), Story(w, WX_Route22Rival1VictoryText), 2);
        else if (s == 2) {
            PbrbWorldSetEvent(w, WE_EVENT_BEAT_ROUTE22_RIVAL_1ST_BATTLE, 1);
            NativeText(w, WX_Route22RivalAfterBattleText1, 3);
        }
        else if (s == 3) {
            ActorPath(w, 0); if (w->scriptValue == 4) { AddActor(w, 3, 2); AddActor(w, 0, 5); }
            else { AddActor(w, 1, 1); AddActor(w, 3, 3); AddActor(w, 0, 6); }w->scriptStep = 4;
            Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_RIVAL);
        }
        else if (s == 4 && PathsDone(w)) {
            HideObject(w, 0x21, 0, 1);
            PbrbWorldSetEvent(w, WE_EVENT_1ST_ROUTE22_RIVAL_BATTLE, 0);
            PbrbWorldSetEvent(w, WE_EVENT_ROUTE22_RIVAL_WANTS_BATTLE, 0); w->scriptStep = 5;
            Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w));
        }
        else if (s == 5)NativeEnd(w); break;
    case WG_GYM_LEADER: {
        int gi = w->scriptValue; const WorldGymRule* r = &WORLD_GYMS[gi];
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, (unsigned)r->beatEvent)) {
                if (PbrbWorldCheckEvent(w, (unsigned)r->tmEvent))NativeText(w, r->after, 99);
                else w->scriptStep = 4;
            }
            else NativeText(w, r->pre, 1);
        }
        else if (s == 1)NativeBattle(w, w->actors[a].trainerClass - 200u, w->actors[a].trainerId,
            PBRB_WORLD_TRAINER, Story(w, r->win), Story(w, r->win), 2);
        else if (s == 2) { if (r->badgeSound >= 0)NativeSound(w, r->badgeSound, 3); else w->scriptStep = 3; }
        else if (s == 3) { if (r->winExtra >= 0)NativeText(w, r->winExtra, 4); else w->scriptStep = 4; }
        else if (s == 4)NativeText(w, r->info, 5);
        else if (s == 5) { PbrbWorldSetEvent(w, (unsigned)r->beatEvent, 1); NativeItem(w, (unsigned)r->tm, 1, 6); }
        else if (s == 6) {
            if (w->scriptResult) {
                PbrbWorldSetEvent(w, (unsigned)r->tmEvent, 1);
                NativeText(w, r->received, 7, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT); w->event.itemId = (unsigned)r->tm;
            }
            else NativeText(w, r->noRoom, 9);
        }
        else if (s == 7)NativeSound(w, r->itemSound, 8, r->explanation < 0);
        else if (s == 8) { if (r->explanation >= 0)NativeText(w, r->explanation, 9); else w->scriptStep = 9; }
        else if (s == 9) {
            for (int ti = 0; ti < w->actorCount; ++ti) {
                const WorldCatalogText* t = CatalogText(w, w->actors[ti].textId);
                if (t && t->kind == WCT_TRAINER && t->event < WE_NUM_EVENTS)PbrbWorldSetEvent(w, t->event, 1);
            }
            if (gi == 0) {
                HideObject(w, 2, 4, 1); HideObject(w, 0x21, 0, 1);
                PbrbWorldSetEvent(w, WE_EVENT_1ST_ROUTE22_RIVAL_BATTLE, 0);
                PbrbWorldSetEvent(w, WE_EVENT_ROUTE22_RIVAL_WANTS_BATTLE, 0);
            }
            if (gi == 7) {
                HideObject(w, 0x21, 1, 0); PbrbWorldSetEvent(w, WE_EVENT_2ND_ROUTE22_RIVAL_BATTLE, 1);
                PbrbWorldSetEvent(w, WE_EVENT_ROUTE22_RIVAL_WANTS_BATTLE, 1);
            }
            w->scriptStep = 99; Emit(w, PBRB_WORLD_BADGE, 0, 1 << gi);
        }
        else NativeEnd(w); break;
    }
    case WG_GYM_GUIDE:
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_BEAT_BROCK))NativeText(w, WX_PewterGymGuidePostBattleText, 4);
            else NativeText(w, WX_PewterGymGuidePreAdviceText, 1);
        }
        else if (s == 1)NativeChoice(w, 2);
        else if (s == 2)NativeText(w, w->scriptResult ? WX_PewterGymGuideBeginAdviceText : WX_PewterGymGuideFreeServiceText, 3);
        else if (s == 3)NativeText(w, WX_PewterGymGuideAdviceText, 4);
        else NativeEnd(w); break;
    case WG_PEWTER_ESCORT:
        if (s == 0)NativeText(w, WX_PewterCityYoungsterYoureATrainerFollowMeText, 1);
        else if (s == 1) {
            /* Original player RLE is consumed in reverse. Position adjustment
               aligns the player behind the guide before their common route. */
            HeroPath(w); int x = w->heroX, y = w->heroY;
            if (x == 34 && y == 16) { AddHero(w, 3, 1); AddHero(w, 0, 2); AddHero(w, 2, 1); }
            else if (x == 35 && y == 17) { AddHero(w, 2, 1); AddHero(w, 3, 1); AddHero(w, 0, 1); AddHero(w, 2, 1); }
            else if (x == 37 && y == 18)AddHero(w, 2, 3);
            else if (x == 37 && y == 19) { AddHero(w, 2, 1); AddHero(w, 1, 1); AddHero(w, 2, 2); }
            else if (x == 36 && y == 17) { AddHero(w, 2, 1); AddHero(w, 0, 1); AddHero(w, 2, 1); }
            AddHero(w, 2, 14); AddHero(w, 1, 5); AddHero(w, 2, 11); AddHero(w, 0, 5); AddHero(w, 3, 2);
            ActorPath(w, 4); AddActor(w, 0, 2); AddActor(w, 2, 15); AddActor(w, 1, 5); AddActor(w, 2, 11); AddActor(w, 0, 5); AddActor(w, 3, 3);
            w->scriptStep = 2; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_MUSEUM);
        }
        else if (s == 2 && PathsDone(w)) {
            w->actors[4].facing = 2; w->scriptStep = 3;
            Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w));
        }
        else if (s == 3)NativeText(w, WX_PewterCityYoungsterGoTakeOnBrockText, 4);
        else if (s == 4) { ActorPath(w, 4); AddActor(w, 3, 5); w->scriptStep = 5; }
        else if (s == 5 && PathsDone(w)) { w->actors[4].x = w->actors[4].homeX; w->actors[4].y = w->actors[4].homeY; NativeEnd(w); }
        break;
    case WG_FIELD: {
        int move = w->scriptValue;
        if (move == PBRB_WORLD_FIELD_CUT) {
            if (s == 0)NativeText(w, WX_UsedCutText, 1);
            else if (s == 1) {
                int tx = w->heroX + DX[w->facing], ty = w->heroY + DY[w->facing];
                int kind = TileAt(w, tx * 2, ty * 2 + 1) == 0x52u ? WFA_CUT_GRASS : WFA_CUT_TREE;
                int x = tx / 2, y = ty / 2;
                unsigned block = BlockAt(w, x, y), p = 0xf100u;
                for (int i = 0; i < 16 && InRom(w, p, 2); ++i, p += 2) {
                    if (w->rom[p] == 255)break;
                    if (w->rom[p] == block) { ChangeBlock(w, x, y, w->rom[p + 1]); break; }
                }
                w->scriptStep = 2; FieldAnimationStart(w, kind);
            }
            else if (s == 2) { w->scriptStep = 3; Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_CUT); }
            else NativeEnd(w);
        }
        else if (move == PBRB_WORLD_FIELD_SURF) {
            if (s == 0)NativeText(w, WX_SurfingGotOnText, 1);
            else if (s == 1) { w->travelMode = 2; HeroPath(w); AddHero(w, w->facing, 1); w->scriptStep = 2; }
            else if (s == 2 && PathsDone(w)) { w->scriptStep = 3; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_SURFING); }
            else if (s == 3)NativeEnd(w);
        }
        else if (move == PBRB_WORLD_FIELD_STRENGTH) {
            if (s == 0)NativeText(w, WX_UsedStrengthText, 1);
            else if (s == 1) { w->fieldActive |= PBRB_WORLD_FIELD_STRENGTH; NativeText(w, WX_CanMoveBouldersText, 2); }
            else NativeEnd(w);
        }
        else if (move == PBRB_WORLD_FIELD_FLASH) {
            if (s == 0) { w->fieldActive |= PBRB_WORLD_FIELD_FLASH; NativeText(w, WX_FlashLightsAreaText, 1); }
            else NativeEnd(w);
        }
        else if (move == PBRB_WORLD_FIELD_TELEPORT) {
            if (s == 0)NativeText(w, WX_WarpToLastPokemonCenterText, 1);
            else if (s == 1) { w->delay = 3; w->scriptStep = 2; }
            else if (s == 2) {
                w->scriptStep = 3; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_NONE,
                    PBRB_WORLD_FADE_MUSIC | PBRB_WORLD_WAIT_SOUND); w->event.dataCount = 4;
            }
            else if (s == 3) { w->scriptStep = 4; FieldAnimationStart(w, WFA_ESCAPE); }
            else { NativeEnd(w); Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w)); }
        }
        else if (move == PBRB_WORLD_FIELD_DIG || move == PBRB_WORLD_FIELD_FLY) {
            if (s == 0) { w->delay = 3; w->scriptStep = 1; }
            else if (s == 1) {
                w->scriptStep = 2; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_NONE,
                    PBRB_WORLD_FADE_MUSIC | PBRB_WORLD_WAIT_SOUND); w->event.dataCount = 4;
            }
            else if (s == 2) { w->scriptStep = 3; FieldAnimationStart(w, move == PBRB_WORLD_FIELD_FLY ? WFA_FLY : WFA_ESCAPE); }
            else { NativeEnd(w); Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w)); }
        }
        else if (move == 0x106) {
            if (s == 0) { w->battleReturn = w->travelMode == 1 ? 0 : 1; NativeText(w, w->battleReturn ? WX_GotOnBicycleText1 : WX_GotOffBicycleText1, 1); }
            else if (s == 1)NativeText(w, w->battleReturn ? WX_GotOnBicycleText2 : WX_GotOffBicycleText2, 2);
            else if (s == 2) {
                w->travelMode = w->battleReturn; w->scriptStep = 3;
                Emit(w, PBRB_WORLD_MUSIC, 0, w->travelMode ? PBRB_MUSIC_BIKE : PbrbWorldMusic(w));
            }
            else NativeEnd(w);
        }
        else NativeEnd(w); break;
    }
    case WG_FISHING:
        if (s == 0) { NativeText(w, WX_ItemUseText001, 1); w->event.itemId = (unsigned)w->scriptValue; }
        else if (s == 1) { NativeText(w, WX_ItemUseText002, 2); w->event.itemId = (unsigned)w->scriptValue; }
        else if (s == 2) { w->scriptStep = 3; FieldAnimationStart(w, WFA_FISHING); Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_HEAL_AILMENT); }
        else if (s == 3)NativeText(w, a > 0 ? WX_ItsABiteText : a == -2 ? WX_NothingHereText : WX_NoNibbleText, 4);
        else if (s == 4 && a > 0) {
            w->fieldAnimation.kind = WFA_NONE; NativeBattle(w, (unsigned)a, (unsigned)w->battleReturn,
                PBRB_WORLD_WILD | ((w->romMapId >= 0xd9 && w->romMapId < 0xdd) ? PBRB_WORLD_SAFARI_BATTLE : 0), 0, 0, 5);
        }
        else NativeEnd(w); break;
    case WG_WAIT:NativeEnd(w); break;
    default:NativeEnd(w); break;
    }
}
static int GeneralAcknowledge(PbrbWorld* w, int result) {
    if (w->stage != WS_NATIVE_SCRIPT)return 0;
    int kind = w->event.kind; unsigned flags = w->event.flags;
    memset(&w->event, 0, sizeof(w->event)); w->scriptResult = result;
    if (kind == PBRB_WORLD_BATTLE) {
        w->encounterCooldown = 3;
        if (result == PBRB_WORLD_BATTLE_LOST && !(flags & PBRB_WORLD_CATCH_DEMO)) {
            Emit(w, PBRB_WORLD_BLACKOUT, 0, 0); return 1;
        }
    }
    if (kind == PBRB_WORLD_HALL_OF_FAME) { MainFinishHall(w); return 1; }
    if (kind == PBRB_WORLD_UNSUPPORTED)NativeEnd(w);
    w->heroAnim = 0; w->wasMoving = 0; return 1;
}
void PbrbWorldBlackout(PbrbWorld* w) {
    if (!w || !w->rom)return;
    memset(&w->event, 0, sizeof(w->event)); NativeEnd(w); w->travelMode = w->fieldActive = 0;
    w->encounterCooldown = 3; MainResetElite(w); w->serviceFlags &= ~16u;
    Transition(w, w->respawnMap, w->respawnX, w->respawnY, 0, WS_IDLE, 0);
}
int PbrbWorldUseFieldMove(PbrbWorld* w, unsigned move) {
    if (!w || !PbrbWorldCanSave(w) || !(w->player.fieldMoves & move))return 0;
    if (move == PBRB_WORLD_FIELD_FLY)return w->romMapId < 0x25 && (w->player.badges & 4u) != 0;
    if (move == PBRB_WORLD_FIELD_CUT) {
        if (!(w->player.badges & 2u))return 0;
        int x = w->heroX + DX[w->facing], y = w->heroY + DY[w->facing]; unsigned t = TileAt(w, x * 2, y * 2 + 1);
        if (!((w->tileset == 0 && (t == 0x3du || t == 0x52u)) || (w->tileset == 7 && t == 0x50u)))return 0;
    }
    else if (move == PBRB_WORLD_FIELD_SURF) {
        int x = w->heroX + DX[w->facing], y = w->heroY + DY[w->facing];
        if (!(w->player.badges & 16u) || w->travelMode == 2 || !FieldWater(w) ||
            TilePairCollision(w, w->heroX, w->heroY, x, y, 1))return 0;
    }
    else if (move == PBRB_WORLD_FIELD_STRENGTH) { if (!(w->player.badges & 8u))return 0; }
    else if (move == PBRB_WORLD_FIELD_FLASH) { if (!(w->player.badges & 1u))return 0; }
    else if (move == PBRB_WORLD_FIELD_TELEPORT) { if (w->romMapId >= 0x25)return 0; }
    else if (move == PBRB_WORLD_FIELD_DIG) { if (!EscapeAllowed(w))return 0; }
    else return 0;
    NativeStart(w, WG_FIELD, -1); w->scriptValue = (int)move; return 1;
}
int PbrbWorldUseItem(PbrbWorld* w, unsigned item) {
    if (!w || !PbrbWorldCanSave(w) || !HasItem(w, item))return 0;
    if (item == 0x47) { NativeStart(w, WG_ITEMFINDER, -1); w->scriptValue = WorldHiddenItemNear(w); return 1; }
    if (item == 0x49) {
        int actor = -1;
        if (w->romMapId == 0x17 && !PbrbWorldCheckEvent(w, WE_EVENT_BEAT_ROUTE12_SNORLAX) &&
            ((w->heroX == 9 && w->heroY == 62) || (w->heroX == 10 && (w->heroY == 61 || w->heroY == 63)) || (w->heroX == 11 && w->heroY == 62)))actor = 0;
        if (w->romMapId == 0x1b && !PbrbWorldCheckEvent(w, WE_EVENT_BEAT_ROUTE16_SNORLAX) &&
            (w->heroX == 25 || w->heroX == 27) && w->heroY == 10)actor = 6;
        MainPuzzleStart(w, WP_SNORLAX, actor); return 1;
    }
    if (item == 6) {
        if (w->serviceFlags & 64u) { Emit(w, PBRB_WORLD_TEXT, Story(w, WX_CyclingIsFunText), 0); return 1; }
        if (!BikeAllowed(w) || w->travelMode == 2)return 0;
        NativeStart(w, WG_FIELD, -1); w->scriptValue = 0x106; return 1;
    }
    if (item == 0x1d) {
        if (!EscapeAllowed(w))return 0;
        NativeStart(w, WG_FIELD, -1); w->scriptValue = PBRB_WORLD_FIELD_DIG; return 1;
    }
    if (item >= 0x4c && item <= 0x4e) {
        if (w->travelMode == 2 || !FieldWater(w))return 0;
        int species = 0, level = 0;
        if (item == 0x4c) { species = 0x85; level = 5; }
        else {
            unsigned data = 0xe27fu, count = 2;
            if (item == 0x4e) {
                data = 0; unsigned p = 0xe919u;
                for (int i = 0; i < 64 && InRom(w, p, 3); ++i, p += 3) {
                    if (w->rom[p] == 255)break;
                    if (w->rom[p] == w->romMapId) { data = Address(3, Read16(w->rom + p + 1)); break; }
                }
                if (data && InRom(w, data, 1)) { count = w->rom[data++]; if (!count || count > 4)data = 0; }
                if (!data)species = -2;
            }
            if (data && InRom(w, data, count * 2))for (int r = 0; r < 64; ++r) {
                unsigned n = RandomByte(w);
                if (n & 1u)break; n = (n >> 1) & 3u;
                if (n < count) { level = w->rom[data + n * 2]; species = w->rom[data + n * 2 + 1]; break; }
            }
        }
        NativeStart(w, WG_FISHING, species); w->scriptValue = (int)item; w->battleReturn = level; return 1;
    }
    return 0;
}
int PbrbWorldFlyTo(PbrbWorld* w, unsigned town) {
    if (!w || !PbrbWorldUseFieldMove(w, PBRB_WORLD_FIELD_FLY) || town >= 11 || !(w->visitedTowns & (1u << town)))return 0;
    NativeStart(w, WG_FIELD, -1); w->scriptValue = PBRB_WORLD_FIELD_FLY; w->battleReturn = (int)town; return 1;
}
