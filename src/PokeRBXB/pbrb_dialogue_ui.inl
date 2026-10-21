/* ROM offsets for native Name Rater and reading/picture screens. */
enum {
    WDUI_NameRatersHouseNameRaterWantMeToRateText,
    WDUI_NameRatersHouseNameRaterWhichPokemonText,
    WDUI_NameRatersHouseNameRaterGiveItANiceNameText,
    WDUI_NameRatersHouseNameRaterWhatShouldWeNameItText,
    WDUI_NameRatersHouseNameRaterPokemonHasBeenRenamedText,
    WDUI_NameRatersHouseNameRaterComeAnyTimeYouLikeText,
    WDUI_NameRatersHouseNameRaterATrulyImpeccableNameText,
    WDUI_CeruleanBadgeHouseMiddleAgedManText,
    WDUI_CeruleanBadgeHouseMiddleAgedManWhichBadgeText,
    WDUI_CeruleanBadgeHouseMiddleAgedManVisitAnyTimeText,
    WDUI_CeruleanBadgeHouseBoulderBadgeText,
    WDUI_CeruleanBadgeHouseCascadeBadgeText,
    WDUI_CeruleanBadgeHouseThunderBadgeText,
    WDUI_CeruleanBadgeHouseRainbowBadgeText,
    WDUI_CeruleanBadgeHouseSoulBadgeText,
    WDUI_CeruleanBadgeHouseMarshBadgeText,
    WDUI_CeruleanBadgeHouseVolcanoBadgeText,
    WDUI_CeruleanBadgeHouseEarthBadgeText,
    WDUI_ViridianSchoolBlackboardText1,
    WDUI_ViridianSchoolBlackboardText2,
    WDUI_ViridianBlackboardSleepText,
    WDUI_ViridianBlackboardPoisonText,
    WDUI_ViridianBlackboardPrlzText,
    WDUI_ViridianBlackboardBurnText,
    WDUI_ViridianBlackboardFrozenText,
    WDUI_LinkCableHelpText1,
    WDUI_LinkCableHelpText2,
    WDUI_LinkCableInfoText1,
    WDUI_LinkCableInfoText2,
    WDUI_LinkCableInfoText3,
    WDUI_HowToLinkText,
    WDUI_StatusAilmentText1,
    WDUI_StatusAilmentText2,
    WDUI_DiplomaTextPointersAndCoords,
    WDUI_TrainerInfoTextBoxTileGraphics,
    WDUI_CircleTile,
    WDUI_PlayerCharacterTitleGraphics,
    WDUI_FossilAerodactylPic,
    WDUI_FossilKabutopsPic,
    WDUI_COUNT
};
static const unsigned WDUI_ROM[2][WDUI_COUNT] = {
    {
        0x9a308u, /* _NameRatersHouseNameRaterWantMeToRateText */
        0x9a361u, /* _NameRatersHouseNameRaterWhichPokemonText */
        0x9a37fu, /* _NameRatersHouseNameRaterGiveItANiceNameText */
        0x9a3e5u, /* _NameRatersHouseNameRaterWhatShouldWeNameItText */
        0x9a404u, /* _NameRatersHouseNameRaterPokemonHasBeenRenamedText */
        0x9a44cu, /* _NameRatersHouseNameRaterComeAnyTimeYouLikeText */
        0x9a46bu, /* _NameRatersHouseNameRaterATrulyImpeccableNameText */
        0x990c9u, /* _CeruleanBadgeHouseMiddleAgedManText */
        0x9913au, /* _CeruleanBadgeHouseMiddleAgedManWhichBadgeText */
        0x99170u, /* _CeruleanBadgeHouseMiddleAgedManVisitAnyTimeText */
        0x99192u, /* _CeruleanBadgeHouseBoulderBadgeText */
        0x991f2u, /* _CeruleanBadgeHouseCascadeBadgeText */
        0x9925du, /* _CeruleanBadgeHouseThunderBadgeText */
        0x992b8u, /* _CeruleanBadgeHouseRainbowBadgeText */
        0x9932au, /* _CeruleanBadgeHouseSoulBadgeText */
        0x99388u, /* _CeruleanBadgeHouseMarshBadgeText */
        0x993c7u, /* _CeruleanBadgeHouseVolcanoBadgeText */
        0x993f5u, /* _CeruleanBadgeHouseEarthBadgeText */
        0x89110u, /* _ViridianSchoolBlackboardText1 */
        0x8914eu, /* _ViridianSchoolBlackboardText2 */
        0x89172u, /* _ViridianBlackboardSleepText */
        0x891deu, /* _ViridianBlackboardPoisonText */
        0x8924bu, /* _ViridianBlackboardPrlzText */
        0x892b5u, /* _ViridianBlackboardBurnText */
        0x8932fu, /* _ViridianBlackboardFrozenText */
        0x89001u, /* _LinkCableHelpText1 */
        0x89027u, /* _LinkCableHelpText2 */
        0x8904bu, /* _LinkCableInfoText1 */
        0x890bdu, /* _LinkCableInfoText2 */
        0x890e8u, /* _LinkCableInfoText3 */
        0x5dca8u, /* HowToLinkText */
        0x5ddacu, /* StatusAilmentText1 */
        0x5ddbbu, /* StatusAilmentText2 */
        0x56784u, /* DiplomaTextPointersAndCoords */
        0x2fb98u, /* TrainerInfoTextBoxTileGraphics */
        0x2fd88u, /* CircleTile */
        0x126a8u, /* PlayerCharacterTitleGraphics */
        0x36536u, /* FossilAerodactylPic */
        0x2f9e8u, /* FossilKabutopsPic */
    },
    {
        0x9a308u, /* _NameRatersHouseNameRaterWantMeToRateText */
        0x9a361u, /* _NameRatersHouseNameRaterWhichPokemonText */
        0x9a37fu, /* _NameRatersHouseNameRaterGiveItANiceNameText */
        0x9a3e5u, /* _NameRatersHouseNameRaterWhatShouldWeNameItText */
        0x9a404u, /* _NameRatersHouseNameRaterPokemonHasBeenRenamedText */
        0x9a44cu, /* _NameRatersHouseNameRaterComeAnyTimeYouLikeText */
        0x9a46bu, /* _NameRatersHouseNameRaterATrulyImpeccableNameText */
        0x990c9u, /* _CeruleanBadgeHouseMiddleAgedManText */
        0x9913au, /* _CeruleanBadgeHouseMiddleAgedManWhichBadgeText */
        0x99170u, /* _CeruleanBadgeHouseMiddleAgedManVisitAnyTimeText */
        0x99192u, /* _CeruleanBadgeHouseBoulderBadgeText */
        0x991f2u, /* _CeruleanBadgeHouseCascadeBadgeText */
        0x9925du, /* _CeruleanBadgeHouseThunderBadgeText */
        0x992b8u, /* _CeruleanBadgeHouseRainbowBadgeText */
        0x9932au, /* _CeruleanBadgeHouseSoulBadgeText */
        0x99388u, /* _CeruleanBadgeHouseMarshBadgeText */
        0x993c7u, /* _CeruleanBadgeHouseVolcanoBadgeText */
        0x993f5u, /* _CeruleanBadgeHouseEarthBadgeText */
        0x89110u, /* _ViridianSchoolBlackboardText1 */
        0x8914eu, /* _ViridianSchoolBlackboardText2 */
        0x89172u, /* _ViridianBlackboardSleepText */
        0x891deu, /* _ViridianBlackboardPoisonText */
        0x8924bu, /* _ViridianBlackboardPrlzText */
        0x892b5u, /* _ViridianBlackboardBurnText */
        0x8932fu, /* _ViridianBlackboardFrozenText */
        0x89001u, /* _LinkCableHelpText1 */
        0x89027u, /* _LinkCableHelpText2 */
        0x8904bu, /* _LinkCableInfoText1 */
        0x890bdu, /* _LinkCableInfoText2 */
        0x890e8u, /* _LinkCableInfoText3 */
        0x5dca8u, /* HowToLinkText */
        0x5ddacu, /* StatusAilmentText1 */
        0x5ddbbu, /* StatusAilmentText2 */
        0x56784u, /* DiplomaTextPointersAndCoords */
        0x2fb98u, /* TrainerInfoTextBoxTileGraphics */
        0x2fd88u, /* CircleTile */
        0x126a8u, /* PlayerCharacterTitleGraphics */
        0x36536u, /* FossilAerodactylPic */
        0x2f9e8u, /* FossilKabutopsPic */
    },
};
/* Native UI for Name Rater and the original non-inventory reading screens.
   PICTURE values: 0 Pokedex, 1 diploma, 2 boxed sprite, 3 badge guide,
   4 school blackboard, 5 cable help, 6 Jigglypuff's song. */
enum PbrbDialogueStep {
    PDU_DONE, PDU_NAME_ASK, PDU_NAME_PARTY, PDU_NAME_RENAME,
    PDU_NAME_OPEN, PDU_NAME_EDIT, PDU_PICTURE_OPEN, PDU_PICTURE_WAIT,
    PDU_PICTURE_RETURN, PDU_READ_PROMPT, PDU_READ_LIST,
    PDU_SONG_STOP, PDU_SONG_START, PDU_SONG_SPIN, PDU_SONG_RETURN
};
static unsigned PduRom(const PbrbPlay* p, unsigned id) {
    return id < WDUI_COUNT ? WDUI_ROM[p->game == 2 ? 1 : 0][id] : 0;
}
static void PduText(PbrbPlay* p, unsigned id, int next, int wait = 0) {
    p->menuMode = next; PplayBeginText(p, PduRom(p, id), wait, PPLAY_AFTER_NONE, 0);
}
static void PduDone(PbrbPlay* p, int result) {
    p->overlay = PBRB_OVERLAY_NONE; p->text.visible = 0; p->selection = 0;
    PplaySyncAliases(p); PplayWorldPlayer(p);
    PbrbWorldAcknowledge(p->world, result); p->worldEvent = 0;
    p->dialogueEvent.kind = PBRB_WORLD_NONE;
}
static int PduChoice(PbrbPlay* p, unsigned keys) {
    if (keys & (PBRB_PAD_UP | PBRB_PAD_DOWN))p->selection ^= 1;
    if (keys & PBRB_PAD_B)return -1;
    return (keys & PBRB_PAD_A) ? (p->selection ? -1 : 1) : 0;
}
static void PduYesNo(PbrbPlay* p) {
    PbrbGbTextBox(p->gb, 112, 56, 6, 5); PbrbGbString(p->gb, "YES", 128, 64);
    PbrbGbString(p->gb, "NO", 128, 80); PbrbGbGlyph(p->gb, 0xed, 120, 64 + p->selection * 16);
}
static void PduPictureOpen(PbrbPlay* p) {
    int value = p->dialogueEvent.value; p->text.visible = 0; p->selection = 0;
    p->menuMode = PDU_PICTURE_WAIT; p->timer = 3;
    if (value == 0) {
        PbrbGameRom rom = PplayRom(p);
        unsigned dex = PbrbGameDex(&rom, p->dialogueEvent.species);
        if (!dex || dex > 151) { PduDone(p, 0); return; }
        p->menuTarget = (p->gameState.owned[(dex - 1u) / 8u] >> (dex - 1u) % 8u) & 1u;
        PbrbAudioPlayCry(p->dialogueEvent.species);
    }
    else if (value == 2) {
        unsigned sp = p->dialogueEvent.species;
        int ok = sp == 0xb7 || sp == 0xb6 ? PbrbGbDecodePicture(p->gb, PduRom(p, sp == 0xb7 ? WDUI_FossilAerodactylPic : WDUI_FossilKabutopsPic), p->picture, sizeof(p->picture)) :
            PbrbGbDecodePokemon(p->gb, sp, 0, p->picture, sizeof(p->picture));
        if (!ok)PduDone(p, 0);
    }
}
static int PbrbPlayDialogueBegin(PbrbPlay* p, const PbrbWorldEvent* event) {
    if (event->kind != PBRB_WORLD_NAME_RATER && event->kind != PBRB_WORLD_PICTURE)return 0;
    p->dialogueEvent = *event; p->overlay = PBRB_OVERLAY_SERVICE; p->selection = 0;
    p->menuTarget = 0; p->menuCursor = 0; p->menuReturn = 0; p->text.visible = 0;
    if (event->kind == PBRB_WORLD_NAME_RATER) {
        PduText(p, WDUI_NameRatersHouseNameRaterWantMeToRateText, PDU_NAME_ASK); return 1;
    }
    const int value = event->value;
    if (value >= 3 && value <= 5) {
        unsigned first = value == 3 ? WDUI_CeruleanBadgeHouseMiddleAgedManText :
            value == 4 ? WDUI_ViridianSchoolBlackboardText1 : WDUI_LinkCableHelpText1;
        PduText(p, first, PDU_READ_PROMPT); return 1;
    }
    if (value == 6) {
        p->menuMode = PDU_SONG_STOP;
        if (event->textOffset)PplayBeginText(p, event->textOffset, 0, PPLAY_AFTER_NONE, 0);
        return 1;
    }
    if (value < 0 || value>2) { PduDone(p, 0); return 1; }
    p->menuMode = PDU_PICTURE_OPEN;
    if (event->textOffset)PplayBeginText(p, event->textOffset, 1, PPLAY_AFTER_NONE, 0);
    else PduPictureOpen(p);
    return 1;
}
static void PbrbPlayDialogueNamed(PbrbPlay* p) {
    if (p->dialogueEvent.kind != PBRB_WORLD_NAME_RATER)return;
    p->overlay = PBRB_OVERLAY_SERVICE;
    if (p->namingLength && p->menuTarget >= 0 && (unsigned)p->menuTarget < p->gameState.partyCount) {
        PbrbPlayMon* m = p->gameState.party + p->menuTarget;
        PplayCopyName(m->nickname, p->naming, 10); PplayCopyName(p->nameBuffer, m->nickname, 10);
        PplaySyncAliases(p); p->menuReturn = 1;
        PduText(p, WDUI_NameRatersHouseNameRaterPokemonHasBeenRenamedText, PDU_DONE, 1);
    }
    else PduText(p, WDUI_NameRatersHouseNameRaterComeAnyTimeYouLikeText, PDU_DONE, 1);
}
static void PduReadingTick(PbrbPlay* p, unsigned keys) {
    int type = p->dialogueEvent.value;
    if (p->menuMode == PDU_READ_PROMPT) {
        unsigned prompt = type == 3 ? WDUI_CeruleanBadgeHouseMiddleAgedManWhichBadgeText :
            type == 4 ? WDUI_ViridianSchoolBlackboardText2 : WDUI_LinkCableHelpText2;
        p->menuReturn = 1; PduText(p, prompt, PDU_READ_LIST); return;
    }
    if (p->menuMode != PDU_READ_LIST)return;
    if (type == 3) {
        if ((keys & PBRB_PAD_UP) && p->selection > 0)--p->selection;
        if ((keys & PBRB_PAD_DOWN) && p->selection < 8)++p->selection;
        if (p->selection < p->menuTarget)p->menuTarget = p->selection;
        if (p->selection >= p->menuTarget + 4)p->menuTarget = p->selection - 3;
        if ((keys & PBRB_PAD_B) || ((keys & PBRB_PAD_A) && p->selection == 8)) {
            p->menuReturn = 0; PduText(p, WDUI_CeruleanBadgeHouseMiddleAgedManVisitAnyTimeText, PDU_DONE, 1);
        }
        else if (keys & PBRB_PAD_A)PduText(p, WDUI_CeruleanBadgeHouseBoulderBadgeText + (unsigned)p->selection, PDU_READ_PROMPT);
    }
    else if (type == 4) {
        int row = p->selection % 3, col = p->selection / 3;
        if (keys & PBRB_PAD_UP)row = (row + 2) % 3;
        if (keys & PBRB_PAD_DOWN)row = (row + 1) % 3;
        if (keys & PBRB_PAD_LEFT)col = 0;
        if (keys & PBRB_PAD_RIGHT)col = 1;
        p->selection = col * 3 + row;
        if ((keys & PBRB_PAD_B) || ((keys & PBRB_PAD_A) && p->selection == 5))PduDone(p, 1);
        else if (keys & PBRB_PAD_A)PduText(p, WDUI_ViridianBlackboardSleepText + (unsigned)p->selection, PDU_READ_PROMPT);
    }
    else {
        if (keys & PBRB_PAD_UP)p->selection = (p->selection + 3) % 4;
        if (keys & PBRB_PAD_DOWN)p->selection = (p->selection + 1) % 4;
        if ((keys & PBRB_PAD_B) || ((keys & PBRB_PAD_A) && p->selection == 3))PduDone(p, 1);
        else if (keys & PBRB_PAD_A)PduText(p, WDUI_LinkCableInfoText1 + (unsigned)p->selection, PDU_READ_PROMPT);
    }
}
static int PbrbPlayDialogueTick(PbrbPlay* p, unsigned keys) {
    if (p->dialogueEvent.kind == PBRB_WORLD_NONE || p->overlay != PBRB_OVERLAY_SERVICE)return 0;
    if (p->text.active)return 1;
    const int kind = p->dialogueEvent.kind, value = p->dialogueEvent.value;
    if (kind == PBRB_WORLD_NAME_RATER) {
        int answer;
        switch (p->menuMode) {
        case PDU_NAME_ASK:
            answer = PduChoice(p, keys);
            if (answer < 0)PduText(p, WDUI_NameRatersHouseNameRaterComeAnyTimeYouLikeText, PDU_DONE, 1);
            else if (answer > 0) { p->selection = 0; PduText(p, WDUI_NameRatersHouseNameRaterWhichPokemonText, PDU_NAME_PARTY); }break;
        case PDU_NAME_PARTY: {
            p->text.visible = 0;
            unsigned count = p->gameState.partyCount;
            if (keys & PBRB_PAD_B) { PduText(p, WDUI_NameRatersHouseNameRaterComeAnyTimeYouLikeText, PDU_DONE, 1); break; }
            if (!count) { PduText(p, WDUI_NameRatersHouseNameRaterComeAnyTimeYouLikeText, PDU_DONE, 1); break; }
            if (keys & PBRB_PAD_UP)p->selection = (p->selection + (int)count - 1) % (int)count;
            if (keys & PBRB_PAD_DOWN)p->selection = (p->selection + 1) % (int)count;
            if (keys & PBRB_PAD_A) {
                p->menuTarget = p->selection; p->pendingParty = p->selection; p->selection = 0;
                const PbrbPlayMon* m = p->gameState.party + p->menuTarget;
                PplayCopyName(p->nameBuffer, m->nickname, 10); p->dialogueEvent.species = m->species;
                if (m->ot != p->trainerID || memcmp(m->originalTrainer, p->player, 11))
                    PduText(p, WDUI_NameRatersHouseNameRaterATrulyImpeccableNameText, PDU_DONE, 1);
                else PduText(p, WDUI_NameRatersHouseNameRaterGiveItANiceNameText, PDU_NAME_RENAME);
            }break;
        }
        case PDU_NAME_RENAME:
            answer = PduChoice(p, keys);
            if (answer < 0)PduText(p, WDUI_NameRatersHouseNameRaterComeAnyTimeYouLikeText, PDU_DONE, 1);
            else if (answer > 0)PduText(p, WDUI_NameRatersHouseNameRaterWhatShouldWeNameItText, PDU_NAME_OPEN); break;
        case PDU_NAME_OPEN:p->menuMode = PDU_NAME_EDIT; PplayStartNaming(p, 5); break;
        case PDU_DONE:PduDone(p, p->menuReturn); break;
        default:break;
        }return 1;
    }
    if (value >= 3 && value <= 5) {
        if (p->menuMode == PDU_DONE)PduDone(p, 1); else PduReadingTick(p, keys); return 1;
    }
    if (value == 6) {
        if (p->menuMode == PDU_SONG_STOP) { PbrbAudioPlaySfx(PBRB_SFX_STOP_ALL); p->timer = 32; p->menuMode = PDU_SONG_START; }
        else if (p->menuMode == PDU_SONG_START) {
            if (p->timer > 0)--p->timer;
            if (!p->timer) { PbrbAudioPlayMusic(PBRB_MUSIC_JIGGLYPUFF); p->timer = 24; p->menuMode = PDU_SONG_SPIN; }
        }
        else if (p->menuMode == PDU_SONG_SPIN) {
            if (p->timer > 0)--p->timer;
            if (!p->timer) {
                if (PbrbAudioMusicBusy()) {
                    static const int turn[4] = { 2,3,1,0 }; int a = p->dialogueEvent.objectId;
                    if (a >= 0 && a < p->world->actorCount)p->world->actors[a].facing = turn[p->world->actors[a].facing & 3];
                    p->timer = 24;
                }
                else { p->timer = 48; p->menuMode = PDU_SONG_RETURN; }
            }
        }
        else if (p->menuMode == PDU_SONG_RETURN) {
            if (p->timer > 0)--p->timer;
            if (!p->timer) { PbrbAudioPlayMusic(PbrbWorldMusic(p->world)); PduDone(p, 1); }
        }return 1;
    }
    if (p->menuMode == PDU_PICTURE_OPEN) { PduPictureOpen(p); return 1; }
    if (p->menuMode == PDU_PICTURE_RETURN) { if (p->timer > 0)--p->timer; if (!p->timer)PduDone(p, 1); return 1; }
    if (p->menuMode == PDU_PICTURE_WAIT) {
        if (p->timer > 0) { --p->timer; return 1; }
        if (keys & (PBRB_PAD_A | PBRB_PAD_B)) {
            if (value == 0 && p->menuTarget && p->selection == 0) { p->selection = 1; return 1; }
            if (value == 0) {
                PbrbGameRom rom = PplayRom(p); unsigned dex = PbrbGameDex(&rom, p->dialogueEvent.species);
                if (dex >= 1 && dex <= 151)p->gameState.seen[(dex - 1u) / 8u] |= (unsigned char)(1u << ((dex - 1u) % 8u));
                p->menuMode = PDU_PICTURE_RETURN; p->timer = 10; p->text.visible = 0;
            }
            else PduDone(p, 1);
        }
    }
    return 1;
}
static void PduDiploma(PbrbPlay* p) {
    PbrbGb* g = p->gb; PbrbGbClear(g, 0);
    const unsigned border = PduRom(p, WDUI_TrainerInfoTextBoxTileGraphics);
    for (int y = 0; y < 18; ++y)for (int x = 0; x < 20; ++x) {
        unsigned c = y == 0 ? (x == 0 ? 0x78u : x == 19 ? 0x7au : 0x79u) :
            y == 17 ? (x == 0 ? 0x7cu : x == 19 ? 0x7du : 0x76u) : x == 0 ? 0x7bu : x == 19 ? 0x77u : 0x7fu;
        if (c != 0x7f)PbrbGbTile(g, border + (c - 0x76u) * 16u, 2, x * 8, y * 8);
    }
    const unsigned table = PduRom(p, WDUI_DiplomaTextPointersAndCoords);
    for (unsigned i = 0; i < 5; ++i) {
        unsigned at = table + i * 4u; if (!PplayUIFits(g, at, 4))break;
        unsigned text = PplayUIAddress(table / 0x4000u, PplayUIWord(g->rom + at));
        unsigned cell = PplayUIWord(g->rom + at + 2u) - 0xc3a0u;
        int x = (int)(cell % 20u) * 8, y = (int)(cell / 20u) * 8, first = x;
        for (unsigned budget = 0; budget < 128 && PplayUIFits(g, text, 1); ++budget) {
            unsigned c = g->rom[text++]; if (c == 0x50)break;
            if (c == 0x4e) { x = first; y += 16; continue; }
            if (c == 0x70)PbrbGbTile(g, PduRom(p, WDUI_CircleTile), 2, x, y);
            else if (c == 0x54) { PbrbGbString(g, "POK", x, y); PbrbGbGlyph(g, 0xba, x + 24, y); x += 24; }
            else PbrbGbGlyph(g, (unsigned char)c, x, y);
            x += 8;
        }
    }
    PbrbGbCodes(g, p->player, 80, 32, 7);
    PbrbGbOam oam[35]; unsigned art = PduRom(p, WDUI_PlayerCharacterTitleGraphics);
    for (unsigned i = 0; i < 35; ++i) {
        oam[i].offset = art + i * 16u; oam[i].x = (unsigned char)(123 + (i % 5u) * 8u);
        oam[i].y = (unsigned char)(96 + (i / 5u) * 8u); oam[i].attributes = 0x80;
    }
    PbrbGbDrawOam(g, oam, 35, 0x90, 0xe4);
}
static void PduReadingRender(PbrbPlay* p) {
    if (!p->menuReturn)return;
    PbrbGb* g = p->gb; const int type = p->dialogueEvent.value;
    if (type == 3) {
        PbrbGbTextBox(g, 32, 16, 16, 11); PbrbGameRom rom = PplayRom(p);
        for (int i = 0; i < 4 && p->menuTarget + i < 9; ++i) {
            unsigned item = (unsigned)(p->menuTarget + i);
            if (item == 8)PbrbGbString(g, "CANCEL", 48, 32 + i * 16);
            else { unsigned char name[16]; PbrbGameItemName(&rom, 21u + item, name); PbrbGbCodes(g, name, 48, 32 + i * 16, 13); }
        }
        PbrbGbGlyph(g, 0xed, 40, 32 + (p->selection - p->menuTarget) * 16);
        if (p->menuTarget < 5)PbrbGbGlyph(g, 0xee, 144, 88);
    }
    else if (type == 4) {
        PbrbGbTextBox(g, 0, 0, 12, 8);
        PplayUIString(g, PduRom(p, WDUI_StatusAilmentText1), 8, 16, 0, 32);
        PplayUIString(g, PduRom(p, WDUI_StatusAilmentText2), 48, 16, 0, 32);
        PbrbGbGlyph(g, 0xed, p->selection >= 3 ? 48 : 8, 16 + (p->selection % 3) * 16);
    }
    else {
        PbrbGbTextBox(g, 0, 0, 15, 10);
        PplayUIString(g, PduRom(p, WDUI_HowToLinkText), 16, 16, 0, 80);
        PbrbGbGlyph(g, 0xed, 8, 16 + p->selection * 16);
    }
}
static int PbrbPlayDialogueRender(PbrbPlay* p) {
    if (p->dialogueEvent.kind == PBRB_WORLD_NONE || p->overlay != PBRB_OVERLAY_SERVICE)return 0;
    if (p->dialogueEvent.kind == PBRB_WORLD_NAME_RATER) {
        if (p->menuMode == PDU_NAME_PARTY && !p->text.active) { int swap = p->menuSwap; p->menuSwap = -1; PplayMenuPartyRender(p); p->menuSwap = swap; }
        else if (!p->text.active && (p->menuMode == PDU_NAME_ASK || p->menuMode == PDU_NAME_RENAME))PduYesNo(p);
        return 1;
    }
    int value = p->dialogueEvent.value;
    if (value >= 3 && value <= 5) { PduReadingRender(p); return 1; }
    if (p->menuMode != PDU_PICTURE_WAIT)return 1;
    if (value == 0)PbrbPlayDrawDexEntry(p->gb, p->dialogueEvent.species, p->selection, p->menuTarget);
    else if (value == 1)PduDiploma(p);
    else if (value == 2) { PbrbGbTextBox(p->gb, 48, 32, 9, 10); PbrbGbBitmap(p->gb, p->picture, 56, 56, 56, 40, 0, 0); }
    return 1;
}
