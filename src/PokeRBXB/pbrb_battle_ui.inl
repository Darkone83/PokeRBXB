/* Native battle presentation consumes the mechanics event stream. It keeps
   displayed HP separate from the already-resolved turn until each animation
   and original text prompt completes. */
static void PplayBattleSubmit(PbrbPlay* p, int type, unsigned slot, unsigned item, unsigned target, unsigned moveSlot) {
    PbrbBattleCommand c; c.type = type; c.slot = slot; c.item = item; c.target = target; c.moveSlot = moveSlot;
    if (!PbrbBattleSubmit(&p->battle, &c) && p->battle.eventRead >= p->battle.eventCount)return;
    p->overlay = PBRB_OVERLAY_NONE; p->text.visible = 0; p->endpoint = 0;
    p->battleEventActive = 0; p->battleEventPhase = 0; p->battleWait = 0;
}
static void PplayBattleSnapshot(PbrbPlay* p, const PbrbBattleEvent* e, int hp) {
    PbrbPlayMon* display[2] = { &p->starter,&p->enemy };
    for (unsigned side = 0; side < 2; ++side) {
        PbrbPlayMon* m = display[side]; unsigned index = e->active[side];
        if (index >= p->battle.count[side])continue;
        unsigned oldHP = m->hp; *m = p->battle.party[side][index]; m->maxHP = e->maxHP[side];
        m->level = (unsigned char)e->level[side]; m->status = e->status[side]; m->hp = hp ? e->hp[side] : oldHP;
        p->battlePictureFlags[side] = e->pictureFlags[side];
        if (side && p->battle.setup.ghost)PplayAsciiName(m->nickname, "GHOST");
        if (e->displaySpecies[side] && p->battleDisplaySpecies[side] != e->displaySpecies[side]) {
            p->battleDisplaySpecies[side] = e->displaySpecies[side];
            if (side) { if (!p->battle.setup.ghost)PbrbGbDecodePokemon(p->gb, e->displaySpecies[side], 0, p->enemyPicture, sizeof(p->enemyPicture)); }
            else PplayBackPicture(p, e->displaySpecies[side], 0, p->backPicture);
        }
    }
}
static void PplayBattleRefreshAlarm(PbrbPlay* p) {
    int hidden = (p->battleHidden & 1u) != 0;
    if (p->battleEventActive && p->battleEvent.side == 0 &&
        (p->battleEvent.type == PBRB_BE_FAINT || p->battleEvent.type == PBRB_BE_SEND_OUT))hidden = 1;
    PbrbAudioSetLowHealth(!p->battleAlarmDisabled && !p->battleFinished && !p->battle.setup.safari &&
        !hidden && p->starter.hp && PplayHealthPixels(&p->starter) <= 9u);
}
static void PplayBattleForeground(PbrbPlay* p) {
    if (p->scene != PBRB_PLAY_BATTLE || !p->battleEventActive ||
        p->battleEvent.type != PBRB_BE_LEVEL || p->battleEventPhase != 2)return;
    /* PrintStatsBox LEVEL_UP_STATS_BOX: interior 9x8 at tile (9,2),
       names at (11,3), three-digit values at (15,4), two rows apart. */
    PbrbGb* g = p->gb; PbrbGbTextBox(g, 72, 16, 11, 10);
    static const char* names[] = { "ATTACK","DEFENSE","SPEED","SPECIAL" };
    for (unsigned i = 0; i < 4; ++i) {
        unsigned n = p->battleEvent.statValues[i]; char number[4];
        if (n > 999)n = 999; number[0] = n >= 100 ? (char)('0' + n / 100u) : ' ';
        number[1] = n >= 10 ? (char)('0' + (n / 10u) % 10u) : ' '; number[2] = (char)('0' + n % 10u); number[3] = 0;
        PbrbGbString(g, names[i], 88, 24 + (int)i * 16); PbrbGbString(g, number, 120, 32 + (int)i * 16);
    }
}
static int PplayBattleMessageText(const PbrbBattleEvent* e) {
    switch (e->message) {
    case PBRB_BM_ASLEEP:return PBRB_GT_FastAsleepText;
    case PBRB_BM_WOKE:return PBRB_GT_WokeUpText;
    case PBRB_BM_FROZEN:return PBRB_GT_IsFrozenText;
    case PBRB_BM_PARALYZED:return PBRB_GT_FullyParalyzedText;
    case PBRB_BM_CONFUSED:return PBRB_GT_IsConfusedText;
    case PBRB_BM_CONFUSION_START:return PBRB_GT_BecameConfusedText;
    case PBRB_BM_CONFUSION_END:return PBRB_GT_ConfusedNoMoreText;
    case PBRB_BM_FLINCHED:return PBRB_GT_FlinchedText;
    case PBRB_BM_RECHARGE:return PBRB_GT_MustRechargeText;
    case PBRB_BM_DISABLED:return PBRB_GT_MoveIsDisabledText;
    case PBRB_BM_DISABLE_END:return PBRB_GT_DisabledNoMoreText;
    case PBRB_BM_CHARGING:
        switch (e->move) {
        case 13:return PBRB_GT_MadeWhirlwindText; case 19:return PBRB_GT_FlewUpHighText;
        case 76:return PBRB_GT_TookInSunlightText; case 91:return PBRB_GT_DugAHoleText;
        case 130:return PBRB_GT_LoweredItsHeadText; default:return PBRB_GT_SkyAttackGlowingText;
        }
    case PBRB_BM_TRAPPED:return PBRB_GT_CantMoveText;
    case PBRB_BM_BIDE:return PBRB_GT_SavingEnergyText;
    case PBRB_BM_BIDE_RELEASE:return PBRB_GT_UnleashedEnergyText;
    case PBRB_BM_SUBSTITUTE:return PBRB_GT_SubstituteText;
    case PBRB_BM_SUBSTITUTE_DAMAGED:return PBRB_GT_SubstituteTookDamageText;
    case PBRB_BM_SUBSTITUTE_BROKE:return PBRB_GT_SubstituteBrokeText;
    case PBRB_BM_SEEDED:return PBRB_GT_WasSeededText;
    case PBRB_BM_LEECH_SEED_HURT:return PBRB_GT_HurtByLeechSeedText;
    case PBRB_BM_POISON:return PBRB_GT_HurtByPoisonText;
    case PBRB_BM_BURN:return PBRB_GT_HurtByBurnText;
    case PBRB_BM_RECOIL:return PBRB_GT_HitWithRecoilText;
    case PBRB_BM_DRAIN:return PBRB_GT_SuckedHealthText;
    case PBRB_BM_MIST:return PBRB_GT_ShroudedInMistText;
    case PBRB_BM_FOCUS:return PBRB_GT_GettingPumpedText;
    case PBRB_BM_LIGHT_SCREEN:return PBRB_GT_LightScreenProtectedText;
    case PBRB_BM_REFLECT:return PBRB_GT_ReflectGainedArmorText;
    case PBRB_BM_HAZE:return PBRB_GT_StatusChangesEliminatedText;
    case PBRB_BM_TRANSFORM:return PBRB_GT_TransformedText;
    case PBRB_BM_CONVERSION:return PBRB_GT_ConvertedTypeText;
    case PBRB_BM_PAY_DAY:return PBRB_GT_CoinsScatteredText;
    case PBRB_BM_NO_PP:return PBRB_GT_MoveNoPPText;
    case PBRB_BM_CANNOT_RUN:return PBRB_GT_NoRunningText;
    case PBRB_BM_RUN_FAILED:return PBRB_GT_CantEscapeText;
    case PBRB_BM_BALL_BLOCKED:return PBRB_GT_ItemUseBallText00;
    case PBRB_BM_BALL_MISSED:return PBRB_GT_ItemUseBallText01;
    case PBRB_BM_BALL_BROKE:return e->value >= 3 ? PBRB_GT_ItemUseBallText04 : e->value == 2 ? PBRB_GT_ItemUseBallText03 : PBRB_GT_ItemUseBallText02;
    case PBRB_BM_SAFARI_BAIT:return PBRB_GT_ThrewBaitText;
    case PBRB_BM_SAFARI_ROCK:return PBRB_GT_ThrewRockText;
    case PBRB_BM_SAFARI_EATING:return PBRB_GT_SafariZoneEatingText;
    case PBRB_BM_SAFARI_ANGRY:return PBRB_GT_SafariZoneAngryText;
    case PBRB_BM_SAFARI_NO_BALLS:return PBRB_GT_OutOfSafariBallsText;
    case PBRB_BM_DISOBEY:return PBRB_GT_WontObeyText;
    case PBRB_BM_SELF_HIT:return PBRB_GT_HurtItselfText;
    case PBRB_BM_HIT_COUNT:return PBRB_GT_HitXTimesText;
    default:return PBRB_GT_NothingHappenedText;
    }
}
static void PplayBattleSendText(PbrbPlay* p, const PbrbBattleEvent* e) {
    PbrbGameRom rom = PplayRom(p); PplayBattleSnapshot(p, e, 1); p->battleHidden |= 1u << e->side; p->battleEventPhase = 1;
    if (e->side) {
        p->battleLastSwitchEnemyHP = e->hp[1]; PbrbGameMarkDex(&rom, &p->gameState, e->species, 0);
        PplayText(p, PBRB_DATA_TrainerSentOutText, 0, PPLAY_AFTER_BATTLE_EVENT);
    }
    else {
        unsigned percent = 100, divisor = (e->maxHP[1] / 4u) & 255u;
        if (e->hp[1]) { p->battleLastSwitchEnemyHP = e->hp[1]; percent = divisor ? (e->hp[1] * 25u / divisor) & 255u : 100u; }
        int text = percent >= 70 ? PBRB_GT_GoText : percent >= 40 ? PBRB_GT_DoItText : percent >= 10 ? PBRB_GT_GetmText : PBRB_GT_EnemysWeakText;
        PplayGameText(p, text, 0, PPLAY_AFTER_BATTLE_EVENT); PplayAppendGameText(p, PBRB_GT_PlayerMon1Text);
    }
}
static void PplayBattleRetreatText(PbrbPlay* p) {
    unsigned change = (p->battleLastSwitchEnemyHP - p->enemy.hp) & 65535u;
    unsigned divisor = (p->enemy.maxHP / 4u) & 255u;
    unsigned percent = divisor ? (change * 25u / divisor) & 255u : 0u;
    PplayGameText(p, PBRB_GT_PlayerMon2Text, 0, PPLAY_AFTER_BATTLE_EVENT);
    if (!percent)PplayAppendGameText(p, PBRB_GT_EnoughText);
    else if (percent >= 70)PplayAppendGameText(p, PBRB_GT_GoodText);
    else if (percent >= 30)PplayAppendGameText(p, PBRB_GT_OKExclamationText);
    PplayAppendGameText(p, PBRB_GT_ComeBackText);
}
static int PplayBattleFollowsTrainerItem(const PbrbPlay* p, const PbrbBattleEvent* e) {
    if (!e->side || p->battle.eventRead < 2u)return 0;
    const PbrbBattleEvent* previous = p->battle.events + p->battle.eventRead - 2u;
    return previous->type == PBRB_BE_ITEM && previous->side == 1;
}
static void PplayBattleStartAnimation(PbrbPlay* p, const PbrbBattleEvent* e) {
    if (p->battleAnimation) {
        PbrbPlayRender(p); PbrbBattleAnimInit(&p->battleAnim, p->gb, e->value, e->side,
            e->displaySpecies[e->side], e->displaySpecies[1 - e->side]);
    }
    else p->battleWait = 30;
}
static void PplayBattleEventStart(PbrbPlay* p, const PbrbBattleEvent* e) {
    PbrbGameRom rom = PplayRom(p); p->battleEventPhase = 1; p->battleEventActive = 1;
    p->text.visible = 0; p->textUser = e->side; p->textTarget = 1 - e->side;
    p->textNumber = e->value; p->textLevel = e->level[0]; p->battleVisual = 0;
    if (e->move) {
        unsigned char name[16]; PbrbGameMoveName(&rom, e->move, name);
        memset(p->stringBuffer, 0x50, 32); memcpy(p->stringBuffer, name, 16); memcpy(p->nameBuffer, name, 16);
    }
    if (e->type != PBRB_BE_HP && e->type != PBRB_BE_SEND_OUT &&
        !(e->type == PBRB_BE_MESSAGE && e->message == PBRB_BM_SUBSTITUTE_BROKE))PplayBattleSnapshot(p, e, 1);
    switch (e->type) {
    case PBRB_BE_MOVE:
        PplayGameText(p, PBRB_GT_ActorNameText, 0, PPLAY_AFTER_BATTLE_EVENT);
        PplayAppendGameText(p, PBRB_GT_UsedMove1Text); PplayAppendGameText(p, PBRB_GT_MoveNameText);
        PplayAppendGameText(p, PBRB_GT_EndUsedMove1Text); p->battleVisual = 1; p->battleVisualSide = e->side; break;
    case PBRB_BE_HP:
        PplayBattleSnapshot(p, e, 0); p->battleHpTick = 0;
        if (e->value < e->value2) {
            unsigned factor = 10;
            for (unsigned n = p->battle.eventRead; n < p->battle.eventCount; ++n) {
                const PbrbBattleEvent* following = p->battle.events + n;
                if (following->type == PBRB_BE_EFFECTIVE) { factor = following->value; break; }
                if (following->type != PBRB_BE_CRITICAL)break;
            }
            if (e->move) { PbrbPlayRender(p); PbrbBattleAnimHit(&p->battleAnim, p->gb, (unsigned)e->side, factor, e->move); }
            unsigned damage = e->value2 - e->value;
            unsigned motor = e->maxHP[e->side] ? 0x1000u + damage * 0x5000u / e->maxHP[e->side] : 0x1800u; if (motor > 0x6000)motor = 0x6000;
            PbrbInputRumble(e->side ? motor / 3u : motor, e->side ? motor : 0x1000u, 70u + (damage > 10 ? 40u : 0u));
            p->battleVisual = 0; p->battleVisualTime = 0;
        }
        else if (e->value > e->value2)PbrbAudioPlaySfx(PBRB_SFX_HEAL);
        break;
    case PBRB_BE_MISS:PplayGameText(p, PBRB_GT_AttackMissedText, 0, PPLAY_AFTER_BATTLE_EVENT); break;
    case PBRB_BE_FAILED:PplayGameText(p, PBRB_GT_ButItFailedText, 0, PPLAY_AFTER_BATTLE_EVENT); break;
    case PBRB_BE_CRITICAL:PplayGameText(p, e->value ? PBRB_GT_OHKOText : PBRB_GT_CriticalHitText, 0, PPLAY_AFTER_BATTLE_EVENT); break;
    case PBRB_BE_EFFECTIVE:
        if (e->value == 10) { p->battleEventActive = 0; break; }
        PplayGameText(p, e->value == 0 ? PBRB_GT_DoesntAffectMonText : e->value > 10 ? PBRB_GT_SuperEffectiveText : PBRB_GT_NotVeryEffectiveText, 0, PPLAY_AFTER_BATTLE_EVENT);
        break;
    case PBRB_BE_STATUS: {
        if (!e->value && PplayBattleFollowsTrainerItem(p, e)) { p->battleEventActive = 0; break; }
        p->textTarget = e->side; p->textUser = 1 - e->side;
        int id = PBRB_GT_RegainedHealthText;
        if (e->value & 7)id = PBRB_GT_FellAsleepText;
        else if (e->value & 8)id = e->message == PBRB_BM_TOXIC ? PBRB_GT_BadlyPoisonedText : PBRB_GT_PoisonedText;
        else if (e->value & 16)id = PBRB_GT_BurnedText;
        else if (e->value & 32)id = PBRB_GT_FrozenText;
        else if (e->value & 64)id = PBRB_GT_ParalyzedMayNotAttackText;
        PplayGameText(p, id, 0, PPLAY_AFTER_BATTLE_EVENT); break;
    }
    case PBRB_BE_STAT: {
        if (PplayBattleFollowsTrainerItem(p, e)) { p->battleEventActive = 0; break; }
        static const char* names[] = { "ATTACK","DEFENSE","SPEED","SPECIAL","ACCURACY","EVADE" };
        memset(p->stringBuffer, 0x50, 32); if (e->value < 6)for (unsigned i = 0; names[e->value][i]; ++i)p->stringBuffer[i] = PplayChar(names[e->value][i]);
        p->textTarget = e->side; p->textUser = e->side;
        PplayGameText(p, e->message < 0 ? PBRB_GT_MonsStatsFellText : PBRB_GT_MonsStatsRoseText, 0, PPLAY_AFTER_BATTLE_EVENT);
        if (e->message <= -2)PplayAppendGameText(p, PBRB_GT_GreatlyFellText);
        if (e->message >= 2)PplayAppendGameText(p, PBRB_GT_GreatlyRoseText);
        PplayAppendGameText(p, e->message < 0 ? PBRB_GT_FellText : PBRB_GT_RoseText); break;
    }
    case PBRB_BE_FAINT:
        p->battleVisual = 3; p->battleVisualSide = e->side; p->battleEventPhase = 2;
        PbrbPlayRender(p); PbrbBattleAnimFaint(&p->battleAnim, p->gb, (unsigned)e->side, e->species, p->battle.setup.trainerClass != 0);
        PbrbInputRumble(0x1800, 0x0800, 100); break;
    case PBRB_BE_SEND_OUT:
        if (e->side && p->enemy.hp && !(p->battleHidden & 2u)) {
            p->battleEventPhase = 6; PplayGameText(p, PBRB_GT_AIBattleWithdrawText, 0, PPLAY_AFTER_BATTLE_EVENT);
        }
        else if (!e->side && p->starter.hp && !(p->battleHidden & 1u)) {
            p->battleEventPhase = 4; PbrbAudioSetLowHealth(0); PplayBattleRetreatText(p);
        }
        else PplayBattleSendText(p, e); break;
    case PBRB_BE_EXP:
        if (e->party >= 0 && e->party < 6)PplayCopyName(p->nameBuffer, p->battle.party[0][e->party].nickname, 10);
        PplayGameText(p, PBRB_GT_GainedText, 0, PPLAY_AFTER_BATTLE_EVENT);
        if (e->message == 2)PplayAppendGameText(p, PBRB_GT_WithExpAllText);
        else if (e->message == 1)PplayAppendGameText(p, PBRB_GT_BoostedText);
        PplayAppendGameText(p, PBRB_GT_ExpPointsText); break;
    case PBRB_BE_LEVEL:
        if (e->party >= 0 && e->party < 6)PplayCopyName(p->nameBuffer, p->battle.party[0][e->party].nickname, 10);
        p->textLevel = e->value; PplayGameText(p, PBRB_GT_GrewLevelText, 0, PPLAY_AFTER_BATTLE_EVENT);
        /* The shared decoder's generic sound token would play the item
           jingle. Own this text's sound_level_up at its original endpoint. */
        for (unsigned i = 0; i < p->text.count; ++i)if (p->text.tokens[i] == 0x3a) {
            memmove(p->text.tokens + i, p->text.tokens + i + 1, p->text.count - i - 1); --p->text.count; --i;
        }
        break;
    case PBRB_BE_LEARN_OFFER:
        p->pendingParty = e->party; p->pendingMove = (int)e->move; p->learnReturn = 1;
        PplayGameText(p, PBRB_GT_TryingToLearnText, 0, PPLAY_AFTER_LEARN); break;
    case PBRB_BE_LEARNED:
        if (e->message == PBRB_BM_MIMIC_LEARNED) { PplayGameText(p, PBRB_GT_MimicLearnedMoveText, 0, PPLAY_AFTER_BATTLE_EVENT); break; }
        p->pendingParty = e->party; PplayGameText(p, PBRB_GT_LearnedMove1Text, 1, PPLAY_AFTER_BATTLE_EVENT); PbrbAudioPlaySfx(PBRB_SFX_LEVEL_UP); break;
    case PBRB_BE_MIMIC_OFFER:
        p->selection = 0; p->overlay = PBRB_OVERLAY_MIMIC; p->battleEventActive = 0; break;
    case PBRB_BE_ANIMATION:
        if ((p->battle.eventRead > 1u && p->battle.events[p->battle.eventRead - 2u].type == PBRB_BE_MESSAGE) || e->value == PBMOVE_MIMIC)p->text.visible = 1;
        if (e->message > 0) { p->battleEventPhase = 4; p->battleWait = e->message; }
        else PplayBattleStartAnimation(p, e);
        break;
    case PBRB_BE_ITEM:
        if (e->side == 0 && e->value && e->item) {
            if (p->itemSlot < 0)PbrbGameRemoveItem(p->gameState.bag, &p->gameState.bagCount, e->item, 1);
            else if ((unsigned)p->itemSlot < p->gameState.bagCount && p->gameState.bag[p->itemSlot].id == e->item)
                PbrbGameRemoveItemAt(p->gameState.bag, &p->gameState.bagCount, (unsigned)p->itemSlot, 1);
            else { p->battle.error = 1; break; }
        }
        PplaySyncAliases(p); PplayBattleSnapshot(p, e, 1);
        if (e->side) {
            PbrbGameItemName(&rom, e->item, p->nameBuffer);
            PplayGameText(p, PBRB_GT_AIBattleUseItemText, 0, PPLAY_AFTER_BATTLE_EVENT);
        }
        else if ((e->item >= 1 && e->item <= 4) || e->item == 8) { p->battleBallItem = e->item; p->battleWait = 0; }
        else p->battleWait = 8;
        break;
    case PBRB_BE_CAPTURE:
        p->battleEventPhase = 2; PbrbPlayRender(p);
        PbrbBattleAnimCapture(&p->battleAnim, p->gb, e->item, e->value2 > 3u ? 3u : e->value2,
            e->value ? 0u : e->value2 ? 1u : 2u, p->battleDisplaySpecies[0], p->battleDisplaySpecies[1]); break;
    case PBRB_BE_RESULT:p->battleFinished = (int)e->value; p->battleEventActive = 0; break;
    case PBRB_BE_FORCE_SWITCH:p->battleEventActive = 0; break;
    case PBRB_BE_SWITCH_OFFER:
        PbrbGameMonName(&rom, e->species, p->enemy.nickname);
        PplayGameText(p, PBRB_GT_TrainerAboutToUseText, 0, PPLAY_AFTER_BATTLE_SWITCH); break;
    case PBRB_BE_MESSAGE: {
        int id = PplayBattleMessageText(e);
        if (e->message == PBRB_BM_MIST && PplayBattleFollowsTrainerItem(p, e)) { p->battleEventActive = 0; break; }
        if (e->message == PBRB_BM_SUBSTITUTE_DAMAGED || e->message == PBRB_BM_SUBSTITUTE_BROKE) { p->textTarget = e->side; p->textUser = 1 - e->side; }
        if (e->message == PBRB_BM_TRANSFORM)PbrbGameMonName(&rom, e->value, p->nameBuffer);
        if (p->battle.setup.ghost && e->message == PBRB_BM_NOTHING_HAPPENED)id = e->side ? PBRB_GT_GetOutText : PBRB_GT_ScaredText;
        if (e->message == PBRB_BM_BALL_BLOCKED || (e->message == PBRB_BM_BALL_MISSED && (p->battle.setup.ghost || p->battle.setup.noCatch))) {
            p->battleEventPhase = 2; PbrbPlayRender(p); PbrbBattleAnimCapture(&p->battleAnim, p->gb, p->battleBallItem, 0,
                e->message == PBRB_BM_BALL_BLOCKED ? 3u : 4u, p->battleDisplaySpecies[0], p->battleDisplaySpecies[1]); break;
        }
        if (e->message == PBRB_BM_CHARGING) { PplayGameText(p, PBRB_GT_ChargeMoveEffectText, 0, PPLAY_AFTER_BATTLE_EVENT); PplayAppendGameText(p, id); }
        else PplayGameText(p, id, 0, PPLAY_AFTER_BATTLE_EVENT); break;
    }
    default:p->battleEventActive = 0; break;
    }
}
static void PplayBattleLeave(PbrbPlay* p) {
    PbrbAudioSetLowHealth(0);
    int result = p->battleFinished == PBRB_BATTLE_WIN ? PBRB_WORLD_BATTLE_WON :
        p->battleFinished == PBRB_BATTLE_LOSE ? PBRB_WORLD_BATTLE_LOST :
        p->battleFinished == PBRB_BATTLE_CAPTURE ? PBRB_WORLD_BATTLE_CAUGHT : PBRB_WORLD_BATTLE_FLED;
    p->rng = p->battle.rng; p->scene = PBRB_PLAY_WORLD; p->overlay = PBRB_OVERLAY_NONE;
    p->endpoint = 0; p->text.visible = 0; p->worldEvent = 0; p->textUser = 0; p->textTarget = 1;
    p->battleEventActive = 0; p->battleVisual = 0; p->battleAnim.active = 0;
    if (p->battle.setup.safari) { p->gameState.safariBalls = p->battle.safariBalls; p->world->player.safariBalls = p->battle.safariBalls; }
    PplaySyncAliases(p);
    PbrbWorldAcknowledge(p->world, result); PbrbAudioPlayMusic(PbrbWorldMusic(p->world)); PbrbInputStopRumble();
}
/* Evolution and capture naming continue through the shared gameplay menus. */
static int PplayCheckEvolution(PbrbPlay* p);
static void PplayBattleFinish(PbrbPlay* p) {
    PbrbGameRom rom = PplayRom(p);
    switch (p->battleFinishPhase) {
    case 0:
        for (unsigned i = 0; i < p->gameState.partyCount; ++i)p->gameState.party[i] = p->battle.party[0][i];
        p->gameState.active = (unsigned char)p->battle.active[0]; p->battleFinishPhase = 1;
        if (p->battleFinished == PBRB_BATTLE_WIN) {
            p->battleAlarmDisabled = 1; PbrbAudioSetLowHealth(0);
            unsigned cls = p->battle.setup.trainerClass;
            int gym = (cls >= PBTRAINER_BROCK && cls <= PBTRAINER_KOGA) || cls == PBTRAINER_BLAINE || cls == PBTRAINER_SABRINA ||
                (cls == PBTRAINER_GIOVANNI && p->battle.setup.trainerNumber == 3) || cls == PBTRAINER_RIVAL3;
            PbrbAudioPlayMusic(!cls ? PBRB_MUSIC_DEFEATED_WILD : gym ? PBRB_MUSIC_DEFEATED_GYM : PBRB_MUSIC_DEFEATED_TRAINER);
            if (p->battle.setup.trainerClass) { PplayGameText(p, PBRB_GT_TrainerDefeatedText, 0, PPLAY_AFTER_BATTLE_EVENT); return; }
        }
        else if ((p->battleFinished == PBRB_BATTLE_FLEE || p->battleFinished == PBRB_BATTLE_ENEMY_FLED) &&
            !(p->battle.setup.safari && !p->battle.safariBalls)) {
            PplayGameText(p, p->battleFinished == PBRB_BATTLE_ENEMY_FLED ? PBRB_GT_WildRanText : PBRB_GT_GotAwayText, 0, PPLAY_AFTER_BATTLE_EVENT); return;
        }
        break;
    case 1:
        if (p->battle.setup.trainerClass &&
            (p->battleFinished == PBRB_BATTLE_WIN ||
                (p->battleFinished == PBRB_BATTLE_LOSE && p->battle.setup.trainerClass == 25)) &&
            p->battleVisual != 4) {
            /* ScrollTrainerPicAfterBattle exposes six columns at four frames
               each, then waits forty frames before the trainer's end text. */
            p->battleHidden |= 2u; p->battleVisual = 4; p->battleVisualTime = 24;
            p->battleWait = 64; return;
        }
        p->battleFinishPhase = 2;
        if (p->battleFinished == PBRB_BATTLE_WIN && p->encounter.winTextOffset) {
            PplayBeginText(p, p->encounter.winTextOffset, 1, PPLAY_AFTER_BATTLE_EVENT, 0); return;
        }
        if (p->battleFinished == PBRB_BATTLE_LOSE && p->encounter.loseTextOffset) {
            PplayBeginText(p, p->encounter.loseTextOffset, 1, PPLAY_AFTER_BATTLE_EVENT, 0); return;
        }
        break;
    case 2:
        p->battleFinishPhase = 3;
        if (p->battleFinished == PBRB_BATTLE_WIN) {
            unsigned prize = p->battle.setup.prizeMoney;
            p->gameState.money += prize; if (p->gameState.money > 999999)p->gameState.money = 999999; p->money = p->gameState.money;
            if (prize) { p->textMoney = prize; PplayGameText(p, PBRB_GT_MoneyForWinningText, 0, PPLAY_AFTER_BATTLE_EVENT); return; }
        }
        break;
    case 3:
        p->battleFinishPhase = 4;
        if ((p->battleFinished == PBRB_BATTLE_WIN || p->battleFinished == PBRB_BATTLE_CAPTURE) && p->battle.payday) {
            p->gameState.money += p->battle.payday; if (p->gameState.money > 999999)p->gameState.money = 999999;
            p->money = p->gameState.money; p->textMoney = p->battle.payday;
            PplayGameText(p, PBRB_GT_PickUpPayDayMoneyText, 0, PPLAY_AFTER_BATTLE_EVENT); return;
        }
        break;
    case 4:
        p->battleFinishPhase = 5;
        if (p->battleFinished == PBRB_BATTLE_CAPTURE) {
            p->battle.captured.ot = p->trainerID; memcpy(p->battle.captured.originalTrainer, p->player, 11);
            unsigned dex = PbrbGameDex(&rom, p->battle.captured.species);
            if (dex && !(p->gameState.owned[(dex - 1) / 8] & (1u << ((dex - 1) % 8)))) {
                p->menuMode = 3; p->questionValue = p->battle.captured.species; p->selection = 0; p->overlay = PBRB_OVERLAY_DEX_ENTRY; return;
            }
        }break;
    case 5:
        p->battleFinishPhase = 6;
        if (p->battleFinished == PBRB_BATTLE_CAPTURE) {
            p->questionValue = 3; PplayMonsterName(p, p->battle.captured.species, p->nameBuffer);
            PplayText(p, PBRB_DATA_DoYouWantToNicknameText, 0, PPLAY_AFTER_NICKNAME); return;
        }
        break;
    case 6:
        p->battleFinishPhase = 7;
        if (p->battleFinished == PBRB_BATTLE_CAPTURE) {
            int boxed = p->gameState.partyCount >= 6;
            if (!PbrbGameAddMon(&rom, &p->gameState, &p->battle.captured)) {
                PplayGameText(p, PBRB_GT_BoxIsFullText, 0, PPLAY_AFTER_BATTLE_EVENT); return;
            }
            if (boxed) { PplayGameText(p, PBRB_GT_ItemUseBallText07, 0, PPLAY_AFTER_BATTLE_EVENT); return; }
        }break;
    case 7:
        if (p->battleFinished == PBRB_BATTLE_WIN && PplayCheckEvolution(p))return;
        p->battleFinishPhase = 8; break;
    default:PplayBattleLeave(p); return;
    }
}
static int PplayBattleRuntimeTick(PbrbPlay* p, unsigned keys) {
    PplayBattleRefreshAlarm(p);
    if (p->battle.error) {
        p->endpoint = 0; p->selection = 0; p->exitReturn = PBRB_OVERLAY_NONE; p->exitSelection = 0;
        PplayNotice(p, "Battle data error.", "Please reload.", PBRB_OVERLAY_EXIT); return 1;
    }
    if (p->battleAnim.active) { PbrbBattleAnimTick(&p->battleAnim); if (!PbrbBattleAnimDone(&p->battleAnim))return 1; }
    if (p->battleWait > 0) { --p->battleWait; if (p->battleVisualTime > 0)--p->battleVisualTime; return 1; }
    if (p->battleEventActive) {
        PbrbBattleEvent* e = &p->battleEvent;
        if (e->type == PBRB_BE_HP) {
            if (p->battleEventPhase == 1) {
                PbrbPlayMon* m = e->side ? &p->enemy : &p->starter;
                if (m->hp != e->hp[e->side]) {
                    unsigned before = PplayHealthPixels(m), after = before;
                    do {
                        if (m->hp < e->hp[e->side])++m->hp; else --m->hp;
                        after = PplayHealthPixels(m);
                    } while (e->side && m->hp != e->hp[e->side] && before == after);
                    if (!e->side)PplayBattleRefreshAlarm(p);
                    unsigned delta = before > after ? before - after : after - before;
                    unsigned delay = (e->side ? 0u : 1u) + delta * 2u; p->battleWait = delay ? (int)delay - 1 : 0; return 1;
                }
                p->battleEventPhase = 2; p->battleWait = e->side ? 4 : 5; return 1;
            }
        }
        if (e->type == PBRB_BE_LEVEL) {
            if (p->battleEventPhase == 1) { p->battleEventPhase = 3; p->text.visible = 1; PbrbAudioPlaySfx(PBRB_SFX_LEVEL_UP); return 1; }
            if (p->battleEventPhase == 3) { if (PbrbAudioEffectsBusy())return 1; p->battleEventPhase = 2; return 1; }
            if (!(keys & (PBRB_PAD_A | PBRB_PAD_B)))return 1;
            /* Consume this acknowledgement before the next command menu.
               A must not also open FIGHT on the same frame. */
            p->battleEventActive = 0; p->battleVisual = 0; p->text.visible = 0; return 1;
        }
        if (e->type == PBRB_BE_ANIMATION) {
            if (p->battleEventPhase == 4) { p->battleEventPhase = 1; PplayBattleStartAnimation(p, e); return 1; }
            if (p->battleEventPhase == 1) { p->battleEventPhase = 2; if (e->value2) { p->battleWait = (int)e->value2; return 1; } }
        }
        if (e->type == PBRB_BE_FAINT && p->battleEventPhase == 2) {
            p->battleHidden |= 1u << e->side; p->battleEventPhase = 3; p->battleVisual = 0;
            if (e->side && !p->battle.setup.trainerClass) { p->battleAlarmDisabled = 1; PbrbAudioSetLowHealth(0); PbrbAudioPlayMusic(PBRB_MUSIC_DEFEATED_WILD); }
            PplayGameText(p, e->side ? PBRB_GT_EnemyMonFaintedText : PBRB_GT_PlayerMonFaintedText, 0, PPLAY_AFTER_BATTLE_EVENT); return 1;
        }
        if (e->type == PBRB_BE_SEND_OUT) {
            if (p->battleEventPhase == 6) { p->battleEventPhase = 7; PbrbPlayRender(p); PbrbBattleAnimEnemyWithdraw(&p->battleAnim, p->gb); return 1; }
            if (p->battleEventPhase == 7) { p->battleHidden |= 2u; PplayBattleSendText(p, e); return 1; }
            if (p->battleEventPhase == 4) { p->battleEventPhase = 5; PbrbPlayRender(p); PbrbBattleAnimRetreat(&p->battleAnim, p->gb); return 1; }
            if (p->battleEventPhase == 5) { p->battleHidden |= 1u; PplayBattleSendText(p, e); return 1; }
            if (p->battleEventPhase == 1) {
                p->battleEventPhase = 2; PbrbPlayRender(p);
                PbrbBattleAnimSendOut(&p->battleAnim, p->gb, (unsigned)e->side, e->species, p->battle.setup.trainerClass != 0); return 1;
            }
            p->battleHidden &= ~(1u << e->side);
        }
        if (e->type == PBRB_BE_MOVE && p->battleEventPhase == 1) {
            p->battleEventPhase = 2;
            /* Mimic animates after its hit test and committed move choice,
               in a separate ANIMATION event. Never reveal it prematurely. */
            if (e->move == PBMOVE_MIMIC) { p->battleWait = 50; return 1; }
            unsigned animation = e->move; int charging = 0, failed = 0, missed = 0;
            if (p->battle.eventRead < p->battle.eventCount) {
                const PbrbBattleEvent* next = p->battle.events + p->battle.eventRead;
                charging = next->type == PBRB_BE_MESSAGE && next->message == PBRB_BM_CHARGING && next->side == e->side;
                failed = next->type == PBRB_BE_FAILED; missed = next->type == PBRB_BE_MISS;
            }
            if (charging)animation = e->move == 19u ? 100u : e->move == 91u ? 192u : e->side ? 175u : 174u;
            /* Both original attack paths skip an accuracy miss animation,
               except Selfdestruct and Explosion, which still explode. */
            if (missed && e->move != 120u && e->move != 153u) { p->battleWait = 30; return 1; }
            /* A failed Minimize/Substitute must not briefly replace the
               live picture. The ordinary miss/failure prompt owns the wait. */
            if (failed && (e->move == 107u || e->move == 164u)) { p->battleWait = 30; return 1; }
            if (p->battleAnimation) {
                PbrbPlayRender(p); PbrbBattleAnimInit(&p->battleAnim, p->gb, animation, e->side,
                    p->battleDisplaySpecies[e->side], p->battleDisplaySpecies[1 - e->side],
                    !charging && (p->battlePictureFlags[e->side] & PBRB_BP_SUBSTITUTE) != 0,
                    (p->battlePictureFlags[e->side] & PBRB_BP_MINIMIZED) != 0); return 1;
            }
            p->battleWait = 30; return 1;
        }
        if (e->type == PBRB_BE_CAPTURE && p->battleEventPhase == 2) {
            p->battleEventPhase = 3;
            if (e->value) {
                p->battleHidden |= 2u; PbrbAudioPlaySfx(PBRB_SFX_CAUGHT_MON);
                PplayGameText(p, PBRB_GT_ItemUseBallText05, 1, PPLAY_AFTER_BATTLE_EVENT); PbrbInputRumble(0x2000, 0x1000, 160); return 1;
            }
        }
        if (e->type == PBRB_BE_MESSAGE && p->battleEventPhase == 2) {
            p->battleEventPhase = 3; PplayGameText(p, PplayBattleMessageText(e), 0, PPLAY_AFTER_BATTLE_EVENT); return 1;
        }
        if (e->type == PBRB_BE_MESSAGE && e->message == PBRB_BM_SUBSTITUTE_BROKE && p->battleEventPhase == 1) {
            p->battleEventPhase = 3; PbrbPlayRender(p);
            PbrbBattleAnimSubstituteBreak(&p->battleAnim, p->gb, (unsigned)e->side, e->displaySpecies[e->side],
                (e->pictureFlags[e->side] & PBRB_BP_MINIMIZED) != 0);
            PplayBattleSnapshot(p, e, 1); return 1;
        }
        p->battleEventActive = 0; p->battleVisual = 0; p->text.visible = 0;
    }
    if (PbrbBattleNextEvent(&p->battle, &p->battleEvent)) { PplayBattleEventStart(p, &p->battleEvent); return 1; }
    if (p->battle.state == PBRB_BATTLE_FINISHED) { if (!p->battleFinished)p->battleFinished = p->battle.outcome; PplayBattleFinish(p); return 1; }
    if (p->battle.state == PBRB_BATTLE_FORCE_SWITCH) { p->menuMode = 1; p->selection = 0; p->overlay = PBRB_OVERLAY_PARTY; return 1; }
    if (p->battle.state == PBRB_BATTLE_LEARN_MOVE || p->battle.state == PBRB_BATTLE_SWITCH_OFFER || p->battle.state == PBRB_BATTLE_MIMIC) { return 1; }
    if (PbrbBattleForcedMove(&p->battle)) { PplayBattleSubmit(p, PBRB_BATTLE_MOVE, 0, 0, 0, 0); return 1; }
    /* Transform and Mimic alter the volatile move set only. The party copy
       remains the permanent save data and is restored when switching out. */
    memcpy(p->starter.moves, p->battle.side[0].mon.moves, 4);
    memcpy(p->starter.pp, p->battle.side[0].mon.pp, 4);
    memcpy(p->starter.types, p->battle.side[0].mon.types, 2);
    p->endpoint = 1; p->text.visible = 0;
    if (p->battle.setup.safari) {
        if (keys & (PBRB_PAD_UP | PBRB_PAD_DOWN))p->battleSelection ^= 1;
        if (keys & PBRB_PAD_LEFT)p->battleSelection &= 1; if (keys & PBRB_PAD_RIGHT)p->battleSelection |= 2;
        if (keys & PBRB_PAD_A) {
            int c = p->battleSelection == 0 ? PBRB_BATTLE_SAFARI_BALL : p->battleSelection == 1 ? PBRB_BATTLE_SAFARI_ROCK : p->battleSelection == 2 ? PBRB_BATTLE_SAFARI_BAIT : PBRB_BATTLE_RUN;
            PplayBattleSubmit(p, c, 0, 0, 0, 0);
        }return 1;
    }
    return 0;
}
