/* Original single-player quests. Gameplay commits inventory/money; the world
   commits quest bits after the corresponding successful acknowledgment. */
enum WorldAuxiliaryAction {
    WAA_AIDE = 1, WAA_FLY, WAA_FAN_CLUB, WAA_BICYCLE,
    WAA_MAGIKARP, WAA_DRINK, WAA_SHIP
};
struct WorldAideRule { int map, actor, requirement, item, event, explanation; };
static const WorldAideRule WORLD_AIDES[3] = {
    {0x31,0,10,0xc8,WE_EVENT_GOT_HM05,WX_Route2GateOaksAideFlashExplanationText},
    {0x56,1,30,0x47,WE_EVENT_GOT_ITEMFINDER,WX_Route11Gate2FOaksAideItemfinderDescriptionText},
    {0xb9,0,50,0x4b,WE_EVENT_GOT_EXP_ALL,WX_Route15Gate2FOaksAideExpAllText}
};
struct WorldDrinkRule { int item, tm, event, thanks, received, explanation; };
static const WorldDrinkRule WORLD_DRINKS[3] = {
    {0x3c,213,WE_EVENT_GOT_TM13,WX_CeladonMartRoofLittleGirlYayFreshWaterText,
        WX_CeladonMartRoofLittleGirlReceivedTM13Text,WX_CeladonMartRoofLittleGirlTM13ExplanationText},
    {0x3d,248,WE_EVENT_GOT_TM48,WX_CeladonMartRoofLittleGirlYaySodaPopText,
        WX_CeladonMartRoofLittleGirlReceivedTM48Text,WX_CeladonMartRoofLittleGirlTM48ExplanationText},
    {0x3e,249,WE_EVENT_GOT_TM49,WX_CeladonMartRoofLittleGirlYayLemonadeText,
        WX_CeladonMartRoofLittleGirlReceivedTM49Text,WX_CeladonMartRoofLittleGirlTM49ExplanationText}
};
static void WaStart(PbrbWorld* w, int action, int actor, int parameter = 0) {
    NativeStart(w, WG_AUXILIARY, actor); w->scriptValue = action; w->battleReturn = parameter;
}
static void WaAideText(PbrbWorld* w, int text, int next, unsigned flags = 0) {
    const WorldAideRule* r = WORLD_AIDES + w->battleReturn;
    NativeText(w, text, next, flags | 0x08000000u);
    w->event.quantity = (unsigned)r->requirement; w->event.itemId = (unsigned)r->item;
}
static void WaRemoveShipWarp(PbrbWorld* w) {
    for (int i = 0; i < w->warpCount; ++i)if (w->warps[i].destination == 0x5f) {
        for (int j = i + 1; j < w->warpCount; ++j)w->warps[j - 1] = w->warps[j];
        --w->warpCount; --i;
    }
}
static int WorldAuxiliaryInteraction(PbrbWorld* w, int actor, unsigned id) {
    for (int i = 0; i < 3; ++i)if (w->romMapId == WORLD_AIDES[i].map && actor == WORLD_AIDES[i].actor) {
        WaStart(w, WAA_AIDE, actor, i); return 1;
    }
    if (w->romMapId == 0xbc && actor == 0) { WaStart(w, WAA_FLY, actor); return 1; }
    if (w->romMapId == 0x5a && actor == 4) { WaStart(w, WAA_FAN_CLUB, actor); return 1; }
    if (w->romMapId == 0x42 && actor == 0) { WaStart(w, WAA_BICYCLE, actor); return 1; }
    if (w->romMapId == 0x44 && actor == 3) { WaStart(w, WAA_MAGIKARP, actor); return 1; }
    if (w->romMapId == 0x7e && actor == 1) { WaStart(w, WAA_DRINK, actor); return 1; }
    (void)id; return 0;
}
static int WorldAuxiliaryHidden(PbrbWorld*, int, int) { return 0; }
static int WorldAuxiliaryIdle(PbrbWorld* w) {
    if (w->romMapId == 0x5e && w->previousMapRom == 0x5f && w->arrivalWarp == 1 &&
        PbrbWorldCheckEvent(w, WE_EVENT_GOT_HM01) &&
        !PbrbWorldCheckEvent(w, WE_EVENT_WALKED_OUT_OF_DOCK)) {
        WaStart(w, WAA_SHIP, -1);
        if (PbrbWorldCheckEvent(w, WE_EVENT_SS_ANNE_LEFT))w->scriptStep = 7;
        return 1;
    }
    return 0;
}
static void WorldAuxiliaryLoaded(PbrbWorld* w) {
    if (w->romMapId == 0x5e && PbrbWorldCheckEvent(w, WE_EVENT_SS_ANNE_LEFT))WaRemoveShipWarp(w);
    if (!w->restoring && w->previousMapRom == 0x5e && w->romMapId == 5 &&
        PbrbWorldCheckEvent(w, WE_EVENT_STARTED_WALKING_OUT_OF_DOCK))
        PbrbWorldSetEvent(w, WE_EVENT_WALKED_OUT_OF_DOCK, 1);
}
static int WorldAuxiliaryScript(PbrbWorld* w) {
    if (w->scriptId != WG_AUXILIARY)return 0;
    const int s = w->scriptStep;
    if (s == 99) { NativeEnd(w); return 1; }
    switch (w->scriptValue) {
    case WAA_AIDE: {
        const WorldAideRule* r = WORLD_AIDES + w->battleReturn;
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, (unsigned)r->event))WaAideText(w, r->explanation, 99);
            else WaAideText(w, WX_OaksAideHiText, 1);
        }
        else if (s == 1)NativeChoice(w, 2);
        else if (s == 2) {
            if (!w->scriptResult)WaAideText(w, WX_OaksAideComeBackText, 99);
            else if (w->player.ownedCount < (unsigned)r->requirement)WaAideText(w, WX_OaksAideUhOhText, 99);
            else WaAideText(w, WX_OaksAideHereYouGoText, 3);
        }
        else if (s == 3)NativeItem(w, (unsigned)r->item, 1, 4);
        else if (s == 4) {
            if (!w->scriptResult)WaAideText(w, WX_OaksAideNoRoomText, 99);
            else {
                PbrbWorldSetEvent(w, (unsigned)r->event, 1);
                WaAideText(w, WX_OaksAideGotItemText, 5, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            }
        }
        else if (s == 5)NativeSound(w, PBRB_SFX_GET_ITEM, 6);
        else if (s == 6)WaAideText(w, r->explanation, 99);
        break;
    }
    case WAA_FLY:
        if (s == 0)NativeText(w, PbrbWorldCheckEvent(w, WE_EVENT_GOT_HM02) ?
            WX_Route16FlyHouseBrunetteGirlHM02ExplanationText : WX_Route16FlyHouseBrunetteGirlText,
            PbrbWorldCheckEvent(w, WE_EVENT_GOT_HM02) ? 99 : 1);
        else if (s == 1)NativeItem(w, 0xc5, 1, 2);
        else if (s == 2) {
            if (!w->scriptResult)NativeText(w, WX_Route16FlyHouseBrunetteGirlHM02NoRoomText, 99);
            else {
                PbrbWorldSetEvent(w, WE_EVENT_GOT_HM02, 1);
                NativeText(w, WX_Route16FlyHouseBrunetteGirlReceivedHM02Text, 3, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            }
        }
        else if (s == 3)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 99, 1);
        break;
    case WAA_FAN_CLUB:
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_BIKE_VOUCHER) || HasItem(w, 6) || HasItem(w, 0x2d))
                NativeText(w, WX_PokemonFanClubChairFinalText, 99);
            else NativeText(w, WX_PokemonFanClubChairmanIntroText, 1);
        }
        else if (s == 1)NativeChoice(w, 2);
        else if (s == 2)NativeText(w, w->scriptResult ? WX_PokemonFanClubChairmanStoryText : WX_PokemonFanClubNoStoryText,
            w->scriptResult ? 3 : 99);
        else if (s == 3)NativeItem(w, 0x2d, 1, 4);
        else if (s == 4) {
            if (!w->scriptResult)NativeText(w, WX_PokemonFanClubBagFullText, 99);
            else {
                PbrbWorldSetEvent(w, WE_EVENT_GOT_BIKE_VOUCHER, 1);
                NativeText(w, WX_PokemonFanClubReceivedBikeVoucherText, 5, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            }
        }
        else if (s == 5)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 6);
        else if (s == 6)NativeText(w, WX_PokemonFanClubExplainBikeVoucherText, 99);
        break;
    case WAA_BICYCLE:
        if (s == 0) {
            if (PbrbWorldCheckEvent(w, WE_EVENT_GOT_BICYCLE))NativeText(w, WX_BikeShopClerkHowDoYouLikeYourBicycleText, 99);
            else if (HasItem(w, 0x2d))NativeText(w, WX_BikeShopClerkOhThatsAVoucherText, 1);
            else NativeText(w, WX_BikeShopClerkWelcomeText, 10);
        }
        /* The original checks bag room before consuming the voucher. */
        else if (s == 1)NativeItem(w, 6, 1, 2);
        else if (s == 2) { if (!w->scriptResult)NativeText(w, WX_BikeShopBagFullText, 99); else NativeItem(w, 0x2d, 1, 3, 1); }
        else if (s == 3) {
            PbrbWorldSetEvent(w, WE_EVENT_GOT_BICYCLE, 1);
            NativeText(w, WX_BikeShopExchangedVoucherText, 4, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        }
        else if (s == 4)NativeSound(w, PBRB_SFX_GET_KEY_ITEM, 99, 1);
        else if (s == 10) { w->scriptStep = 11; Emit(w, PBRB_WORLD_CHOICE, Story(w, WX_BikeShopClerkDoYouLikeItText), 0, 0x01000000u); }
        else if (s == 11) { if (w->scriptResult)NativeText(w, WX_BikeShopCantAffordText, 12); else w->scriptStep = 12; }
        else if (s == 12)NativeText(w, WX_BikeShopComeAgainText, 99);
        break;
    case WAA_MAGIKARP:
        if (s == 0)NativeText(w, PbrbWorldCheckEvent(w, WE_EVENT_BOUGHT_MAGIKARP) ?
            WX_MtMoonPokecenterMagikarpSalesmanNoRefundsText : WX_MtMoonPokecenterMagikarpSalesmanIGotADealText,
            PbrbWorldCheckEvent(w, WE_EVENT_BOUGHT_MAGIKARP) ? 99 : 1);
        else if (s == 1) { NativeChoice(w, 2); w->event.flags |= 0x00800000u; }
        else if (s == 2) {
            if (!w->scriptResult)NativeText(w, WX_MtMoonPokecenterMagikarpSalesmanNoText, 99);
            else if (w->player.money < 500)NativeText(w, WX_MtMoonPokecenterMagikarpSalesmanNoMoneyText, 99);
            else { w->scriptStep = 3; Emit(w, PBRB_WORLD_GIVE_MON, 0x4fe39u, 0x85); w->event.species = 0x85; w->event.level = 5; }
        }
        else if (s == 3) {
            if (!w->scriptResult)NativeEnd(w);
            else { w->scriptStep = 4; Emit(w, PBRB_WORLD_TAKE_MONEY, 0, 0); w->event.quantity = 500; }
        }
        else if (s == 4) { if (w->scriptResult)PbrbWorldSetEvent(w, WE_EVENT_BOUGHT_MAGIKARP, 1); NativeEnd(w); }
        break;
    case WAA_DRINK: {
        int index = w->battleReturn; const WorldDrinkRule* r = WORLD_DRINKS + (index >= 0 && index < 3 ? index : 0);
        if (s == 0) {
            if (!HasItem(w, 0x3c) && !HasItem(w, 0x3d) && !HasItem(w, 0x3e))NativeText(w, WX_CeladonMartRoofLittleGirlImThirstyText, 99);
            else NativeText(w, WX_CeladonMartRoofLittleGirlGiveHerADrinkText, 1);
        }
        else if (s == 1)NativeChoice(w, 2);
        else if (s == 2) { if (!w->scriptResult)NativeEnd(w); else NativeText(w, WX_CeladonMartRoofLittleGirlGiveHerWhichDrinkText, 3, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT); }
        else if (s == 3) { w->scriptStep = 4; Emit(w, PBRB_WORLD_ITEM_CHOICE, 0, 0); w->event.dataOffset = 0x48408; w->event.dataCount = 3; }
        else if (s == 4) {
            int chosen = w->scriptResult - 0x3c; if (chosen < 0 || chosen>2) { NativeEnd(w); return 1; }
            w->battleReturn = chosen; r = WORLD_DRINKS + chosen;
            if (PbrbWorldCheckEvent(w, (unsigned)r->event))NativeText(w, WX_CeladonMartRoofLittleGirlImNotThirstyText, 99);
            else NativeText(w, r->thanks, 5);
        }
        else if (s == 5)NativeItem(w, (unsigned)r->item, 1, 6, 1);
        else if (s == 6) { if (!w->scriptResult)NativeEnd(w); else NativeItem(w, (unsigned)r->tm, 1, 7); }
        else if (s == 7) {
            if (!w->scriptResult)NativeText(w, WX_CeladonMartRoofLittleGirlNoRoomText, 99);
            else NativeText(w, r->received, 8, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
        }
        else if (s == 8)NativeSound(w, PBRB_SFX_GET_ITEM, 9);
        else if (s == 9) { PbrbWorldSetEvent(w, (unsigned)r->event, 1); NativeText(w, r->explanation, 99); }
        break;
    }
    case WAA_SHIP:
        if (s == 0) { PbrbWorldSetEvent(w, WE_EVENT_SS_ANNE_LEFT, 1); w->scriptStep = 1; Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_STOP_ALL); }
        else if (s == 1) { w->scriptStep = 2; w->delay = 120; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_SURFING); }
        else if (s == 2) { w->scriptStep = 3; w->delay = 3; w->battleReturn = 0; }
        else if (s == 3) { w->scriptStep = 4; Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_SS_ANNE_HORN); }
        else if (s == 4) {
            if (++w->battleReturn >= 1024) {
                w->scriptStep = 5; w->delay = 120;
                Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_SS_ANNE_HORN);
            }
        }
        else if (s == 5) { WaRemoveShipWarp(w); for (int x = 2; x < 6; ++x)ChangeBlock(w, x, 5, 0x0d); w->scriptStep = 7; }
        else if (s == 7) {
            PbrbWorldSetEvent(w, WE_EVENT_STARTED_WALKING_OUT_OF_DOCK, 1);
            HeroPath(w); AddHero(w, 1, 3); w->scriptStep = 8;
        }
        else if (s == 8 && PathsDone(w)) { PbrbWorldSetEvent(w, WE_EVENT_WALKED_OUT_OF_DOCK, 1); NativeEnd(w); }
        break;
    default:NativeEnd(w); break;
    }
    return 1;
}
static void WorldAuxiliaryRender(const PbrbWorld* w, PbrbGb* g) {
    if (w->romMapId != 0x5e || w->scriptId != WG_AUXILIARY || w->scriptValue != WAA_SHIP || w->scriptStep < 4)return;
    const int shift = w->scriptStep == 4 ? w->battleReturn / 8 : 160;
    unsigned char row[160];
    for (int y = 80; y < 128; ++y) {
        memcpy(row, g->background + y * 160, 160);
        for (int x = 0; x < 160; ++x) {
            unsigned c;
            if (x + shift < 160)c = row[x + shift];
            else {
                unsigned off = w->tilesOffset + 0x14u * 16u + (unsigned)(y & 7) * 2u; unsigned bit = 7u - (unsigned)(x & 7);
                c = InRom(w, off, 2) ? ((w->rom[off] >> bit) & 1u) | (((w->rom[off + 1] >> bit) & 1u) << 1) : 0;
            }
            PbrbGbPixel(g, x, y, c, 0);
        }
    }
    if (w->scriptStep == 4) {
        /* Four identical original smoke tiles per puff, OBP1=$00. Eight
           column redraws emit a puff and drift it twice per eight-frame step. */
        int count = w->battleReturn / 128 + 1; if (count > 8)count = 8;
        for (int puff = 0; puff < count; ++puff) {
            int x = 64 - puff * 16 + 2 * ((w->battleReturn - puff * 128) / 8 + 1);
            for (int tile = 0; tile < 4; ++tile)PbrbGbSpriteTile(g, 0x79fddu, x + (tile & 1) * 8, 84 + (tile / 2) * 8, 0, 0x10);
        }
    }
}
