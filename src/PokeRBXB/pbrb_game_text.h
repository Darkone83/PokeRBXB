#ifndef PBRB_GAME_TEXT_H_INCLUDED
#define PBRB_GAME_TEXT_H_INCLUDED
/* Original text addresses only; strings remain in the supplied ROM. */
enum PbrbGameText {
    PBRB_GT_CardKeySuccessText1,
    PBRB_GT_CardKeySuccessText2,
    PBRB_GT_CardKeyFailText,
    PBRB_GT_TrainerNameText,
    PBRB_GT_NoNibbleText,
    PBRB_GT_NothingHereText,
    PBRB_GT_ItsABiteText,
    PBRB_GT_ExclamationText,
    PBRB_GT_GroundRoseText,
    PBRB_GT_BoulderText,
    PBRB_GT_MartSignText,
    PBRB_GT_PokeCenterSignText,
    PBRB_GT_FoundItemText,
    PBRB_GT_NoMoreRoomForItemText,
    PBRB_GT_OaksAideHiText,
    PBRB_GT_OaksAideUhOhText,
    PBRB_GT_OaksAideComeBackText,
    PBRB_GT_OaksAideHereYouGoText,
    PBRB_GT_OaksAideGotItemText,
    PBRB_GT_OaksAideNoRoomText,
    PBRB_GT_AIBattleWithdrawText,
    PBRB_GT_AIBattleUseItemText,
    PBRB_GT_TradeWentToText,
    PBRB_GT_TradeForText,
    PBRB_GT_TradeSendsText,
    PBRB_GT_TradeWavesFarewellText,
    PBRB_GT_TradeTransferredText,
    PBRB_GT_TradeTakeCareText,
    PBRB_GT_TradeWillTradeText,
    PBRB_GT_TradeforText,
    PBRB_GT_PlaySlotMachineText,
    PBRB_GT_OutOfCoinsSlotMachineText,
    PBRB_GT_BetHowManySlotMachineText,
    PBRB_GT_StartSlotMachineText,
    PBRB_GT_NotEnoughCoinsSlotMachineText,
    PBRB_GT_OneMoreGoSlotMachineText,
    PBRB_GT_LinedUpText,
    PBRB_GT_NotThisTimeText,
    PBRB_GT_YeahText,
    PBRB_GT_DexSeenOwnedText,
    PBRB_GT_DexRatingText,
    PBRB_GT_GymStatueText1,
    PBRB_GT_GymStatueText2,
    PBRB_GT_ViridianCityPokecenterGuyText,
    PBRB_GT_PewterCityPokecenterGuyText,
    PBRB_GT_CeruleanPokecenterGuyText,
    PBRB_GT_LavenderPokecenterGuyText,
    PBRB_GT_MtMoonPokecenterBenchGuyText,
    PBRB_GT_RockTunnelPokecenterGuyText,
    PBRB_GT_UnusedBenchGuyText1,
    PBRB_GT_UnusedBenchGuyText2,
    PBRB_GT_UnusedBenchGuyText3,
    PBRB_GT_VermilionPokecenterGuyText,
    PBRB_GT_CeladonCityPokecenterGuyText,
    PBRB_GT_FuchsiaCityPokecenterGuyText,
    PBRB_GT_CinnabarPokecenterGuyText,
    PBRB_GT_SaffronCityPokecenterGuyText1,
    PBRB_GT_SaffronCityPokecenterGuyText2,
    PBRB_GT_CeladonCityHotelText,
    PBRB_GT_BookcaseText,
    PBRB_GT_NewBicycleText,
    PBRB_GT_PushStartText,
    PBRB_GT_SaveOptionText,
    PBRB_GT_StrengthsAndWeaknessesText,
    PBRB_GT_TimesUpText,
    PBRB_GT_GameOverText,
    PBRB_GT_CinnabarGymQuizIntroText,
    PBRB_GT_CinnabarQuizQuestionsText1,
    PBRB_GT_CinnabarQuizQuestionsText2,
    PBRB_GT_CinnabarQuizQuestionsText3,
    PBRB_GT_CinnabarQuizQuestionsText4,
    PBRB_GT_CinnabarQuizQuestionsText5,
    PBRB_GT_CinnabarQuizQuestionsText6,
    PBRB_GT_CinnabarGymQuizCorrectText,
    PBRB_GT_CinnabarGymQuizIncorrectText,
    PBRB_GT_MagazinesText,
    PBRB_GT_BillsHouseMonitorText,
    PBRB_GT_BillsHouseInitiatedText,
    PBRB_GT_BillsHousePokemonListText1,
    PBRB_GT_BillsHousePokemonListText2,
    PBRB_GT_OakLabEmailText,
    PBRB_GT_GameCornerCoinCaseText,
    PBRB_GT_GameCornerNoCoinsText,
    PBRB_GT_GameCornerOutOfOrderText,
    PBRB_GT_GameCornerOutToLunchText,
    PBRB_GT_GameCornerSomeonesKeysText,
    PBRB_GT_JustAMomentText,
    PBRB_GT_TurnPageText,
    PBRB_GT_ViridianSchoolNotebookText5,
    PBRB_GT_ViridianSchoolNotebookText1,
    PBRB_GT_ViridianSchoolNotebookText2,
    PBRB_GT_ViridianSchoolNotebookText3,
    PBRB_GT_ViridianSchoolNotebookText4,
    PBRB_GT_EnemiesOnEverySideText,
    PBRB_GT_WhatGoesAroundComesAroundText,
    PBRB_GT_FightingDojoText,
    PBRB_GT_IndigoPlateauHQText,
    PBRB_GT_RedBedroomSNESText,
    PBRB_GT_Route15UpstairsBinocularsText,
    PBRB_GT_AerodactylFossilText,
    PBRB_GT_KabutopsFossilText,
    PBRB_GT_LinkCableHelpText1,
    PBRB_GT_LinkCableHelpText2,
    PBRB_GT_LinkCableInfoText1,
    PBRB_GT_LinkCableInfoText2,
    PBRB_GT_LinkCableInfoText3,
    PBRB_GT_ViridianSchoolBlackboardText1,
    PBRB_GT_ViridianSchoolBlackboardText2,
    PBRB_GT_ViridianBlackboardSleepText,
    PBRB_GT_ViridianBlackboardPoisonText,
    PBRB_GT_ViridianBlackboardPrlzText,
    PBRB_GT_ViridianBlackboardBurnText,
    PBRB_GT_ViridianBlackboardFrozenText,
    PBRB_GT_VermilionGymTrashText,
    PBRB_GT_VermilionGymTrashSuccessText1,
    PBRB_GT_VermilionGymTrashSuccessText2,
    PBRB_GT_VermilionGymTrashSuccessText3,
    PBRB_GT_VermilionGymTrashFailText,
    PBRB_GT_FoundHiddenItemText,
    PBRB_GT_HiddenItemBagFullText,
    PBRB_GT_FoundHiddenCoinsText,
    PBRB_GT_FoundHiddenCoins2Text,
    PBRB_GT_DroppedHiddenCoinsText,
    PBRB_GT_IndigoPlateauStatuesText1,
    PBRB_GT_IndigoPlateauStatuesText2,
    PBRB_GT_IndigoPlateauStatuesText3,
    PBRB_GT_PokemonBooksText,
    PBRB_GT_DiglettSculptureText,
    PBRB_GT_ElevatorText,
    PBRB_GT_TownMapText,
    PBRB_GT_PokemonStuffText,
    PBRB_GT_OutOfSafariBallsText,
    PBRB_GT_WildRanText,
    PBRB_GT_EnemyRanText,
    PBRB_GT_HurtByPoisonText,
    PBRB_GT_HurtByBurnText,
    PBRB_GT_HurtByLeechSeedText,
    PBRB_GT_EnemyMonFaintedText,
    PBRB_GT_MoneyForWinningText,
    PBRB_GT_TrainerDefeatedText,
    PBRB_GT_PlayerMonFaintedText,
    PBRB_GT_UseNextMonText,
    PBRB_GT_Rival1WinText,
    PBRB_GT_PlayerBlackedOutText2,
    PBRB_GT_LinkBattleLostText,
    PBRB_GT_TrainerAboutToUseText,
    PBRB_GT_TrainerSentOutText,
    PBRB_GT_NoWillText,
    PBRB_GT_CantEscapeText,
    PBRB_GT_NoRunningText,
    PBRB_GT_GotAwayText,
    PBRB_GT_ItemsCantBeUsedHereText,
    PBRB_GT_AlreadyOutText,
    PBRB_GT_MoveNoPPText,
    PBRB_GT_MoveDisabledText,
    PBRB_GT_NoMovesLeftText,
    PBRB_GT_MultiHitText,
    PBRB_GT_ScaredText,
    PBRB_GT_GetOutText,
    PBRB_GT_FastAsleepText,
    PBRB_GT_WokeUpText,
    PBRB_GT_IsFrozenText,
    PBRB_GT_FullyParalyzedText,
    PBRB_GT_FlinchedText,
    PBRB_GT_MustRechargeText,
    PBRB_GT_DisabledNoMoreText,
    PBRB_GT_IsConfusedText,
    PBRB_GT_HurtItselfText,
    PBRB_GT_ConfusedNoMoreText,
    PBRB_GT_SavingEnergyText,
    PBRB_GT_UnleashedEnergyText,
    PBRB_GT_ThrashingAboutText,
    PBRB_GT_AttackContinuesText,
    PBRB_GT_CantMoveText,
    PBRB_GT_MoveIsDisabledText,
    PBRB_GT_ActorNameText,
    PBRB_GT_UsedMove1Text,
    PBRB_GT_UsedMove2Text,
    PBRB_GT_UsedInsteadText,
    PBRB_GT_MoveNameText,
    PBRB_GT_EndUsedMove1Text,
    PBRB_GT_EndUsedMove2Text,
    PBRB_GT_EndUsedMove3Text,
    PBRB_GT_EndUsedMove4Text,
    PBRB_GT_EndUsedMove5Text,
    PBRB_GT_AttackMissedText,
    PBRB_GT_KeptGoingAndCrashedText,
    PBRB_GT_UnaffectedText,
    PBRB_GT_DoesntAffectMonText,
    PBRB_GT_CriticalHitText,
    PBRB_GT_OHKOText,
    PBRB_GT_LoafingAroundText,
    PBRB_GT_BeganToNapText,
    PBRB_GT_WontObeyText,
    PBRB_GT_TurnedAwayText,
    PBRB_GT_IgnoredOrdersText,
    PBRB_GT_SubstituteTookDamageText,
    PBRB_GT_SubstituteBrokeText,
    PBRB_GT_BuildingRageText,
    PBRB_GT_MirrorMoveFailedText,
    PBRB_GT_HitXTimesText,
    PBRB_GT_GainedText,
    PBRB_GT_WithExpAllText,
    PBRB_GT_BoostedText,
    PBRB_GT_ExpPointsText,
    PBRB_GT_GrewLevelText,
    PBRB_GT_WildMonAppearedText,
    PBRB_GT_HookedMonAttackedText,
    PBRB_GT_EnemyAppearedText,
    PBRB_GT_TrainerWantsToFightText,
    PBRB_GT_UnveiledGhostText,
    PBRB_GT_GhostCantBeIDdText,
    PBRB_GT_GoText,
    PBRB_GT_DoItText,
    PBRB_GT_GetmText,
    PBRB_GT_EnemysWeakText,
    PBRB_GT_PlayerMon1Text,
    PBRB_GT_PlayerMon2Text,
    PBRB_GT_EnoughText,
    PBRB_GT_OKExclamationText,
    PBRB_GT_GoodText,
    PBRB_GT_ComeBackText,
    PBRB_GT_SuperEffectiveText,
    PBRB_GT_NotVeryEffectiveText,
    PBRB_GT_SafariZoneEatingText,
    PBRB_GT_SafariZoneAngryText,
    PBRB_GT_PickUpPayDayMoneyText,
    PBRB_GT_ClearSaveDataText,
    PBRB_GT_WhichFloorText,
    PBRB_GT_PartyMenuNormalText,
    PBRB_GT_PartyMenuItemUseText,
    PBRB_GT_PartyMenuBattleText,
    PBRB_GT_PartyMenuUseTMText,
    PBRB_GT_PartyMenuSwapMonText,
    PBRB_GT_PotionText,
    PBRB_GT_AntidoteText,
    PBRB_GT_ParlyzHealText,
    PBRB_GT_BurnHealText,
    PBRB_GT_IceHealText,
    PBRB_GT_AwakeningText,
    PBRB_GT_FullHealText,
    PBRB_GT_ReviveText,
    PBRB_GT_RareCandyText,
    PBRB_GT_TurnedOnPC1Text,
    PBRB_GT_AccessedBillsPCText,
    PBRB_GT_AccessedSomeonesPCText,
    PBRB_GT_AccessedMyPCText,
    PBRB_GT_TurnedOnPC2Text,
    PBRB_GT_WhatDoYouWantText,
    PBRB_GT_WhatToDepositText,
    PBRB_GT_DepositHowManyText,
    PBRB_GT_ItemWasStoredText,
    PBRB_GT_NothingToDepositText,
    PBRB_GT_NoRoomToStoreText,
    PBRB_GT_WhatToWithdrawText,
    PBRB_GT_WithdrawHowManyText,
    PBRB_GT_WithdrewItemText,
    PBRB_GT_NothingStoredText,
    PBRB_GT_CantCarryMoreText,
    PBRB_GT_WhatToTossText,
    PBRB_GT_TossHowManyText,
    PBRB_GT_AccessedHoFPCText,
    PBRB_GT_SwitchOnText,
    PBRB_GT_WhatText,
    PBRB_GT_DepositWhichMonText,
    PBRB_GT_MonWasStoredText,
    PBRB_GT_CantDepositLastMonText,
    PBRB_GT_BoxFullText,
    PBRB_GT_MonIsTakenOutText,
    PBRB_GT_NoMonText,
    PBRB_GT_CantTakeMonText,
    PBRB_GT_ReleaseWhichMonText,
    PBRB_GT_OnceReleasedText,
    PBRB_GT_MonWasReleasedText,
    PBRB_GT_RequireCoinCaseText,
    PBRB_GT_ExchangeCoinsForPrizesText,
    PBRB_GT_WhichPrizeText,
    PBRB_GT_HereYouGoText,
    PBRB_GT_SoYouWantPrizeText,
    PBRB_GT_SorryNeedMoreCoinsText,
    PBRB_GT_OopsYouDontHaveEnoughRoomText,
    PBRB_GT_OhFineThenText,
    PBRB_GT_GetDexRatedText,
    PBRB_GT_ClosedOaksPCText,
    PBRB_GT_AccessedOaksPCText,
    PBRB_GT_WhereWouldYouLikeText,
    PBRB_GT_PleaseWaitText,
    PBRB_GT_LinkCanceledText,
    PBRB_GT_OakSpeechText1,
    PBRB_GT_OakSpeechText2A,
    PBRB_GT_OakSpeechText2B,
    PBRB_GT_IntroducePlayerText,
    PBRB_GT_IntroduceRivalText,
    PBRB_GT_OakSpeechText3,
    PBRB_GT_DoYouWantToNicknameText,
    PBRB_GT_YourNameIsText,
    PBRB_GT_HisNameIsText,
    PBRB_GT_WillBeTradedText,
    PBRB_GT_TextIDErrorText,
    PBRB_GT_ContCharText,
    PBRB_GT_FileDataDestroyedText,
    PBRB_GT_WouldYouLikeToSaveText,
    PBRB_GT_GameSavedText,
    PBRB_GT_OlderFileWillBeErasedText,
    PBRB_GT_WhenYouChangeBoxText,
    PBRB_GT_ChooseABoxText,
    PBRB_GT_EvolvedText,
    PBRB_GT_IntoText,
    PBRB_GT_StoppedEvolvingText,
    PBRB_GT_IsEvolvingText,
    PBRB_GT_FellAsleepText,
    PBRB_GT_AlreadyAsleepText,
    PBRB_GT_PoisonedText,
    PBRB_GT_BadlyPoisonedText,
    PBRB_GT_BurnedText,
    PBRB_GT_FrozenText,
    PBRB_GT_FireDefrostedText,
    PBRB_GT_MonsStatsRoseText,
    PBRB_GT_GreatlyRoseText,
    PBRB_GT_RoseText,
    PBRB_GT_MonsStatsFellText,
    PBRB_GT_GreatlyFellText,
    PBRB_GT_FellText,
    PBRB_GT_RanFromBattleText,
    PBRB_GT_RanAwayScaredText,
    PBRB_GT_WasBlownAwayText,
    PBRB_GT_ChargeMoveEffectText,
    PBRB_GT_MadeWhirlwindText,
    PBRB_GT_TookInSunlightText,
    PBRB_GT_LoweredItsHeadText,
    PBRB_GT_SkyAttackGlowingText,
    PBRB_GT_FlewUpHighText,
    PBRB_GT_DugAHoleText,
    PBRB_GT_BecameConfusedText,
    PBRB_GT_MimicLearnedMoveText,
    PBRB_GT_MoveWasDisabledText,
    PBRB_GT_NothingHappenedText,
    PBRB_GT_NoEffectText,
    PBRB_GT_ButItFailedText,
    PBRB_GT_DidntAffectText,
    PBRB_GT_IsUnaffectedText,
    PBRB_GT_ParalyzedMayNotAttackText,
    PBRB_GT_SubstituteText,
    PBRB_GT_HasSubstituteText,
    PBRB_GT_TooWeakSubstituteText,
    PBRB_GT_CoinsScatteredText,
    PBRB_GT_GettingPumpedText,
    PBRB_GT_WasSeededText,
    PBRB_GT_EvadedAttackText,
    PBRB_GT_HitWithRecoilText,
    PBRB_GT_ConvertedTypeText,
    PBRB_GT_StatusChangesEliminatedText,
    PBRB_GT_StartedSleepingEffect,
    PBRB_GT_FellAsleepBecameHealthyText,
    PBRB_GT_RegainedHealthText,
    PBRB_GT_TransformedText,
    PBRB_GT_LightScreenProtectedText,
    PBRB_GT_ReflectGainedArmorText,
    PBRB_GT_ShroudedInMistText,
    PBRB_GT_SuckedHealthText,
    PBRB_GT_DreamWasEatenText,
    PBRB_GT_TradeCenterOpponentText,
    PBRB_GT_ColosseumOpponentText,
    PBRB_GT_PokemartGreetingText,
    PBRB_GT_PokemonFaintedText,
    PBRB_GT_PlayerBlackedOutText,
    PBRB_GT_RepelWoreOffText,
    PBRB_GT_PokemartBuyingGreetingText,
    PBRB_GT_PokemartTellBuyPriceText,
    PBRB_GT_PokemartBoughtItemText,
    PBRB_GT_PokemartNotEnoughMoneyText,
    PBRB_GT_PokemartItemBagFullText,
    PBRB_GT_PokemonSellingGreetingText,
    PBRB_GT_PokemartTellSellPriceText,
    PBRB_GT_PokemartItemBagEmptyText,
    PBRB_GT_PokemartUnsellableItemText,
    PBRB_GT_PokemartThankYouText,
    PBRB_GT_PokemartAnythingElseText,
    PBRB_GT_LearnedMove1Text,
    PBRB_GT_WhichMoveToForgetText,
    PBRB_GT_AbandonLearningText,
    PBRB_GT_DidNotLearnText,
    PBRB_GT_TryingToLearnText,
    PBRB_GT_OneTwoAndText,
    PBRB_GT_PoofText,
    PBRB_GT_ForgotAndText,
    PBRB_GT_HMCantDeleteText,
    PBRB_GT_PokemonCenterWelcomeText,
    PBRB_GT_ShallWeHealYourPokemonText,
    PBRB_GT_NeedYourPokemonText,
    PBRB_GT_PokemonFightingFitText,
    PBRB_GT_PokemonCenterFarewellText,
    PBRB_GT_CableClubNPCAreaReservedFor2FriendsLinkedByCableText,
    PBRB_GT_CableClubNPCWelcomeText,
    PBRB_GT_CableClubNPCPleaseApplyHereHaveToSaveText,
    PBRB_GT_CableClubNPCPleaseWaitText,
    PBRB_GT_CableClubNPCLinkClosedBecauseOfInactivityText,
    PBRB_GT_CableClubNPCPleaseComeAgainText,
    PBRB_GT_CableClubNPCMakingPreparationsText,
    PBRB_GT_UsedStrengthText,
    PBRB_GT_CanMoveBouldersText,
    PBRB_GT_CurrentTooFastText,
    PBRB_GT_CyclingIsFunText,
    PBRB_GT_FlashLightsAreaText,
    PBRB_GT_WarpToLastPokemonCenterText,
    PBRB_GT_CannotUseTeleportNowText,
    PBRB_GT_CannotFlyHereText,
    PBRB_GT_NotHealthyEnoughText,
    PBRB_GT_NewBadgeRequiredText,
    PBRB_GT_CannotUseItemsHereText,
    PBRB_GT_CannotGetOffHereText,
    PBRB_GT_GotMonText,
    PBRB_GT_SentToBoxText,
    PBRB_GT_BoxIsFullText,
    PBRB_GT_ItemUseBallText00,
    PBRB_GT_ItemUseBallText01,
    PBRB_GT_ItemUseBallText02,
    PBRB_GT_ItemUseBallText03,
    PBRB_GT_ItemUseBallText04,
    PBRB_GT_ItemUseBallText05,
    PBRB_GT_ItemUseBallText07,
    PBRB_GT_ItemUseBallText08,
    PBRB_GT_ItemUseBallText06,
    PBRB_GT_SurfingGotOnText,
    PBRB_GT_SurfingNoPlaceToGetOffText,
    PBRB_GT_VitaminStatRoseText,
    PBRB_GT_VitaminNoEffectText,
    PBRB_GT_ThrewBaitText,
    PBRB_GT_ThrewRockText,
    PBRB_GT_PlayedFluteNoEffectText,
    PBRB_GT_FluteWokeUpText,
    PBRB_GT_PlayedFluteHadEffectText,
    PBRB_GT_CoinCaseNumCoinsText,
    PBRB_GT_ItemfinderFoundItemText,
    PBRB_GT_ItemfinderFoundNothingText,
    PBRB_GT_RaisePPWhichTechniqueText,
    PBRB_GT_RestorePPWhichTechniqueText,
    PBRB_GT_PPMaxedOutText,
    PBRB_GT_PPIncreasedText,
    PBRB_GT_PPRestoredText,
    PBRB_GT_BootedUpTMText,
    PBRB_GT_BootedUpHMText,
    PBRB_GT_TeachMachineMoveText,
    PBRB_GT_MonCannotLearnMachineMoveText,
    PBRB_GT_ItemUseNotTimeText,
    PBRB_GT_ItemUseNotYoursToUseText,
    PBRB_GT_ItemUseNoEffectText,
    PBRB_GT_ThrowBallAtTrainerMonText1,
    PBRB_GT_ThrowBallAtTrainerMonText2,
    PBRB_GT_NoCyclingAllowedHereText,
    PBRB_GT_NoSurfingHereText,
    PBRB_GT_BoxFullCannotThrowBallText,
    PBRB_GT_ItemUseText001,
    PBRB_GT_ItemUseText002,
    PBRB_GT_GotOnBicycleText1,
    PBRB_GT_GotOnBicycleText2,
    PBRB_GT_GotOffBicycleText1,
    PBRB_GT_GotOffBicycleText2,
    PBRB_GT_ThrewAwayItemText,
    PBRB_GT_IsItOKToTossItemText,
    PBRB_GT_TooImportantToTossText,
    PBRB_GT_AlreadyKnowsText,
    PBRB_GT_ConnectCableText,
    PBRB_GT_TradedForText,
    PBRB_GT_WannaTrade1Text,
    PBRB_GT_NoTrade1Text,
    PBRB_GT_WrongMon1Text,
    PBRB_GT_Thanks1Text,
    PBRB_GT_AfterTrade1Text,
    PBRB_GT_WannaTrade2Text,
    PBRB_GT_NoTrade2Text,
    PBRB_GT_WrongMon2Text,
    PBRB_GT_Thanks2Text,
    PBRB_GT_AfterTrade2Text,
    PBRB_GT_WannaTrade3Text,
    PBRB_GT_NoTrade3Text,
    PBRB_GT_WrongMon3Text,
    PBRB_GT_Thanks3Text,
    PBRB_GT_AfterTrade3Text,
    PBRB_GT_NothingToCutText,
    PBRB_GT_UsedCutText,
    PBRB_GT_COUNT
};
static const unsigned kPbrbGameText[2][PBRB_GT_COUNT] = {
    {
        0x80000u, /* CardKeySuccessText1 */
        0x80009u, /* CardKeySuccessText2 */
        0x80029u, /* CardKeyFailText */
        0x80045u, /* TrainerNameText */
        0x8004Du, /* NoNibbleText */
        0x80061u, /* NothingHereText */
        0x80082u, /* ItsABiteText */
        0x80093u, /* ExclamationText */
        0x80096u, /* GroundRoseText */
        0x800B1u, /* BoulderText */
        0x800D2u, /* MartSignText */
        0x800FCu, /* PokeCenterSignText */
        0x80119u, /* FoundItemText */
        0x8012Au, /* NoMoreRoomForItemText */
        0x80143u, /* OaksAideHiText */
        0x801E4u, /* OaksAideUhOhText */
        0x80250u, /* OaksAideComeBackText */
        0x8028Cu, /* OaksAideHereYouGoText */
        0x802D9u, /* OaksAideGotItemText */
        0x802ECu, /* OaksAideNoRoomText */
        0x880BEu, /* AIBattleWithdrawText */
        0x880D5u, /* AIBattleUseItemText */
        0x880EFu, /* TradeWentToText */
        0x88103u, /* TradeForText */
        0x88112u, /* TradeSendsText */
        0x88124u, /* TradeWavesFarewellText */
        0x8813Bu, /* TradeTransferredText */
        0x88150u, /* TradeTakeCareText */
        0x8816Au, /* TradeWillTradeText */
        0x88180u, /* TradeforText */
        0x8818Fu, /* PlaySlotMachineText */
        0x881AEu, /* OutOfCoinsSlotMachineText */
        0x881C7u, /* BetHowManySlotMachineText */
        0x881DCu, /* StartSlotMachineText */
        0x881E4u, /* NotEnoughCoinsSlotMachineText */
        0x881F7u, /* OneMoreGoSlotMachineText */
        0x88206u, /* LinedUpText */
        0x88226u, /* NotThisTimeText */
        0x88236u, /* YeahText */
        0x8823Eu, /* DexSeenOwnedText */
        0x88267u, /* DexRatingText */
        0x88275u, /* GymStatueText1 */
        0x882A5u, /* GymStatueText2 */
        0x882D7u, /* ViridianCityPokecenterGuyText */
        0x8830Cu, /* PewterCityPokecenterGuyText */
        0x88353u, /* CeruleanPokecenterGuyText */
        0x88386u, /* LavenderPokecenterGuyText */
        0x883C2u, /* MtMoonPokecenterBenchGuyText */
        0x883FCu, /* RockTunnelPokecenterGuyText */
        0x88426u, /* UnusedBenchGuyText1 */
        0x88442u, /* UnusedBenchGuyText2 */
        0x88460u, /* UnusedBenchGuyText3 */
        0x8848Eu, /* VermilionPokecenterGuyText */
        0x88531u, /* CeladonCityPokecenterGuyText */
        0x8855Fu, /* FuchsiaCityPokecenterGuyText */
        0x885AFu, /* CinnabarPokecenterGuyText */
        0x88621u, /* SaffronCityPokecenterGuyText1 */
        0x88664u, /* SaffronCityPokecenterGuyText2 */
        0x886A4u, /* CeladonCityHotelText */
        0x886C9u, /* BookcaseText */
        0x886E6u, /* NewBicycleText */
        0x886FCu, /* PushStartText */
        0x8871Au, /* SaveOptionText */
        0x88742u, /* StrengthsAndWeaknessesText */
        0x8877Eu, /* TimesUpText */
        0x88798u, /* GameOverText */
        0x887B7u, /* CinnabarGymQuizIntroText */
        0x8886Du, /* CinnabarQuizQuestionsText1 */
        0x88890u, /* CinnabarQuizQuestionsText2 */
        0x888BBu, /* CinnabarQuizQuestionsText3 */
        0x888D5u, /* CinnabarQuizQuestionsText4 */
        0x88915u, /* CinnabarQuizQuestionsText5 */
        0x88949u, /* CinnabarQuizQuestionsText6 */
        0x88964u, /* CinnabarGymQuizCorrectText */
        0x8898Fu, /* CinnabarGymQuizIncorrectText */
        0x889A1u, /* MagazinesText */
        0x889CFu, /* BillsHouseMonitorText */
        0x889FBu, /* BillsHouseInitiatedText */
        0x88A25u, /* BillsHousePokemonListText1 */
        0x88A40u, /* BillsHousePokemonListText2 */
        0x88A60u, /* OakLabEmailText */
        0x88B5Bu, /* GameCornerCoinCaseText */
        0x88B75u, /* GameCornerNoCoinsText */
        0x88B8Fu, /* GameCornerOutOfOrderText */
        0x88BADu, /* GameCornerOutToLunchText */
        0x88BCDu, /* GameCornerSomeonesKeysText */
        0x88BEDu, /* JustAMomentText */
        0x88C6Fu, /* TurnPageText */
        0x88C7Fu, /* ViridianSchoolNotebookText5 */
        0x88CA3u, /* ViridianSchoolNotebookText1 */
        0x88D46u, /* ViridianSchoolNotebookText2 */
        0x88DBDu, /* ViridianSchoolNotebookText3 */
        0x88E2Cu, /* ViridianSchoolNotebookText4 */
        0x88EC1u, /* EnemiesOnEverySideText */
        0x88ED9u, /* WhatGoesAroundComesAroundText */
        0x88EF9u, /* FightingDojoText */
        0x88F08u, /* IndigoPlateauHQText */
        0x88F27u, /* RedBedroomSNESText */
        0x88F58u, /* Route15UpstairsBinocularsText */
        0x88FA7u, /* AerodactylFossilText */
        0x88FD5u, /* KabutopsFossilText */
        0x89001u, /* LinkCableHelpText1 */
        0x89027u, /* LinkCableHelpText2 */
        0x8904Bu, /* LinkCableInfoText1 */
        0x890BDu, /* LinkCableInfoText2 */
        0x890E8u, /* LinkCableInfoText3 */
        0x89110u, /* ViridianSchoolBlackboardText1 */
        0x8914Eu, /* ViridianSchoolBlackboardText2 */
        0x89172u, /* ViridianBlackboardSleepText */
        0x891DEu, /* ViridianBlackboardPoisonText */
        0x8924Bu, /* ViridianBlackboardPrlzText */
        0x892B5u, /* ViridianBlackboardBurnText */
        0x8932Fu, /* ViridianBlackboardFrozenText */
        0x893A7u, /* VermilionGymTrashText */
        0x893C6u, /* VermilionGymTrashSuccessText1 */
        0x89418u, /* VermilionGymTrashSuccessText2 */
        0x89451u, /* VermilionGymTrashSuccessText3 */
        0x8948Cu, /* VermilionGymTrashFailText */
        0x894D0u, /* FoundHiddenItemText */
        0x894E1u, /* HiddenItemBagFullText */
        0x8950Bu, /* FoundHiddenCoinsText */
        0x89523u, /* FoundHiddenCoins2Text */
        0x8953Bu, /* DroppedHiddenCoinsText */
        0x89557u, /* IndigoPlateauStatuesText1 */
        0x89567u, /* IndigoPlateauStatuesText2 */
        0x89596u, /* IndigoPlateauStatuesText3 */
        0x895C1u, /* PokemonBooksText */
        0x895DEu, /* DiglettSculptureText */
        0x895FBu, /* ElevatorText */
        0x89611u, /* TownMapText */
        0x8961Fu, /* PokemonStuffText */
        0x89639u, /* OutOfSafariBallsText */
        0x89666u, /* WildRanText */
        0x89677u, /* EnemyRanText */
        0x89689u, /* HurtByPoisonText */
        0x8969Du, /* HurtByBurnText */
        0x896B3u, /* HurtByLeechSeedText */
        0x896C7u, /* EnemyMonFaintedText */
        0x896DDu, /* MoneyForWinningText */
        0x896F9u, /* TrainerDefeatedText */
        0x8970Cu, /* PlayerMonFaintedText */
        0x8971Au, /* UseNextMonText */
        0x8972Au, /* Rival1WinText */
        0x89748u, /* PlayerBlackedOutText2 */
        0x89772u, /* LinkBattleLostText */
        0x89784u, /* TrainerAboutToUseText */
        0x897B4u, /* TrainerSentOutText */
        0x897C9u, /* NoWillText */
        0x897E3u, /* CantEscapeText */
        0x897F1u, /* NoRunningText */
        0x8981Fu, /* GotAwayText */
        0x89831u, /* ItemsCantBeUsedHereText */
        0x8984Bu, /* AlreadyOutText */
        0x89860u, /* MoveNoPPText */
        0x8987Bu, /* MoveDisabledText */
        0x89892u, /* NoMovesLeftText */
        0x898AAu, /* MultiHitText */
        0x898C7u, /* ScaredText */
        0x898E3u, /* GetOutText */
        0x89901u, /* FastAsleepText */
        0x89914u, /* WokeUpText */
        0x89920u, /* IsFrozenText */
        0x89934u, /* FullyParalyzedText */
        0x89949u, /* FlinchedText */
        0x89956u, /* MustRechargeText */
        0x89968u, /* DisabledNoMoreText */
        0x8997Eu, /* IsConfusedText */
        0x8998Eu, /* HurtItselfText */
        0x899B0u, /* ConfusedNoMoreText */
        0x899C6u, /* SavingEnergyText */
        0x899DBu, /* UnleashedEnergyText */
        0x899F0u, /* ThrashingAboutText */
        0x89A05u, /* AttackContinuesText */
        0x89A1Bu, /* CantMoveText */
        0x89A29u, /* MoveIsDisabledText */
        0x89A40u, /* ActorNameText */
        0x89A44u, /* UsedMove1Text */
        0x89A4Du, /* UsedMove2Text */
        0x89A56u, /* UsedInsteadText */
        0x89A62u, /* MoveNameText */
        0x89A67u, /* EndUsedMove1Text */
        0x89A6Au, /* EndUsedMove2Text */
        0x89A6Du, /* EndUsedMove3Text */
        0x89A70u, /* EndUsedMove4Text */
        0x89A73u, /* EndUsedMove5Text */
        0x89A76u, /* AttackMissedText */
        0x89A89u, /* KeptGoingAndCrashedText */
        0x89AA4u, /* UnaffectedText */
        0x89AB4u, /* DoesntAffectMonText */
        0x89AC9u, /* CriticalHitText */
        0x89AD8u, /* OHKOText */
        0x89AE5u, /* LoafingAroundText */
        0x89AFDu, /* BeganToNapText */
        0x89B10u, /* WontObeyText */
        0x89B20u, /* TurnedAwayText */
        0x89B32u, /* IgnoredOrdersText */
        0x89B47u, /* SubstituteTookDamageText */
        0x89B6Au, /* SubstituteBrokeText */
        0x89B80u, /* BuildingRageText */
        0x89B96u, /* MirrorMoveFailedText */
        0x89BAFu, /* HitXTimesText */
        0x89BC2u, /* GainedText */
        0x89BD0u, /* WithExpAllText */
        0x89BE1u, /* BoostedText */
        0x89BEEu, /* ExpPointsText */
        0x89C01u, /* GrewLevelText */
        0x89C1Du, /* WildMonAppearedText */
        0x89C33u, /* HookedMonAttackedText */
        0x89C4Fu, /* EnemyAppearedText */
        0x89C5Eu, /* TrainerWantsToFightText */
        0x89C73u, /* UnveiledGhostText */
        0x89C9Eu, /* GhostCantBeIDdText */
        0x89CBCu, /* GoText */
        0x89CC3u, /* DoItText */
        0x89CCDu, /* GetmText */
        0x89CD6u, /* EnemysWeakText */
        0x89CF0u, /* PlayerMon1Text */
        0x89CF6u, /* PlayerMon2Text */
        0x89CFDu, /* EnoughText */
        0x89D07u, /* OKExclamationText */
        0x89D0Du, /* GoodText */
        0x89D15u, /* ComeBackText */
        0x89D22u, /* SuperEffectiveText */
        0x89D38u, /* NotVeryEffectiveText */
        0x89D53u, /* SafariZoneEatingText */
        0x89D6Au, /* SafariZoneAngryText */
        0x89D80u, /* PickUpPayDayMoneyText */
        0x89D96u, /* ClearSaveDataText */
        0x89DADu, /* WhichFloorText */
        0x89DC8u, /* PartyMenuNormalText */
        0x89DD8u, /* PartyMenuItemUseText */
        0x89DF1u, /* PartyMenuBattleText */
        0x89E08u, /* PartyMenuUseTMText */
        0x89E1Fu, /* PartyMenuSwapMonText */
        0x89E31u, /* PotionText */
        0x89E4Bu, /* AntidoteText */
        0x89E65u, /* ParlyzHealText */
        0x89E7Du, /* BurnHealText */
        0x89E94u, /* IceHealText */
        0x89EA8u, /* AwakeningText */
        0x89EB6u, /* FullHealText */
        0x89ECDu, /* ReviveText */
        0x89EE2u, /* RareCandyText */
        0x89EFEu, /* TurnedOnPC1Text */
        0x89F13u, /* AccessedBillsPCText */
        0x89F45u, /* AccessedSomeonesPCText */
        0x89F7Au, /* AccessedMyPCText */
        0x89FA9u, /* TurnedOnPC2Text */
        0x89FBEu, /* WhatDoYouWantText */
        0x89FD7u, /* WhatToDepositText */
        0x89FF5u, /* DepositHowManyText */
        0x8A000u, /* ItemWasStoredText */
        0x8A018u, /* NothingToDepositText */
        0x8A036u, /* NoRoomToStoreText */
        0x8A054u, /* WhatToWithdrawText */
        0x8A073u, /* WithdrawHowManyText */
        0x8A07Eu, /* WithdrewItemText */
        0x8A08Fu, /* NothingStoredText */
        0x8A0A9u, /* CantCarryMoreText */
        0x8A0C9u, /* WhatToTossText */
        0x8A0E9u, /* TossHowManyText */
        0x8A0F4u, /* AccessedHoFPCText */
        0x8A131u, /* SwitchOnText */
        0x8A13Du, /* WhatText */
        0x8A144u, /* DepositWhichMonText */
        0x8A159u, /* MonWasStoredText */
        0x8A177u, /* CantDepositLastMonText */
        0x8A198u, /* BoxFullText */
        0x8A1B9u, /* MonIsTakenOutText */
        0x8A1D7u, /* NoMonText */
        0x8A1F6u, /* CantTakeMonText */
        0x8A228u, /* ReleaseWhichMonText */
        0x8A23Du, /* OnceReleasedText */
        0x8A268u, /* MonWasReleasedText */
        0x8A28Eu, /* RequireCoinCaseText */
        0x8A2A9u, /* ExchangeCoinsForPrizesText */
        0x8A2CDu, /* WhichPrizeText */
        0x8A2E7u, /* HereYouGoText */
        0x8A2F6u, /* SoYouWantPrizeText */
        0x8A30Bu, /* SorryNeedMoreCoinsText */
        0x8A329u, /* OopsYouDontHaveEnoughRoomText */
        0x8A34Cu, /* OhFineThenText */
        0x8A35Du, /* GetDexRatedText */
        0x8A37Bu, /* ClosedOaksPCText */
        0x8A39Au, /* AccessedOaksPCText */
        0x8A3D0u, /* WhereWouldYouLikeText */
        0x8A3EDu, /* PleaseWaitText */
        0x8A40Du, /* LinkCanceledText */
        0x8A425u, /* OakSpeechText1 */
        0x8A47Fu, /* OakSpeechText2A */
        0x8A4B3u, /* OakSpeechText2B */
        0x8A519u, /* IntroducePlayerText */
        0x8A534u, /* IntroduceRivalText */
        0x8A597u, /* OakSpeechText3 */
        0x8A605u, /* DoYouWantToNicknameText */
        0x8A62Fu, /* YourNameIsText */
        0x8A64Au, /* HisNameIsText */
        0x8A677u, /* WillBeTradedText */
        0x8A696u, /* TextIDErrorText */
        0x8A6A3u, /* ContCharText */
        0x945F1u, /* FileDataDestroyedText */
        0x9460Eu, /* WouldYouLikeToSaveText */
        0x94630u, /* GameSavedText */
        0x94643u, /* OlderFileWillBeErasedText */
        0x94671u, /* WhenYouChangeBoxText */
        0x946B0u, /* ChooseABoxText */
        0x946C2u, /* EvolvedText */
        0x946CFu, /* IntoText */
        0x946DDu, /* StoppedEvolvingText */
        0x946FBu, /* IsEvolvingText */
        0x94715u, /* FellAsleepText */
        0x94725u, /* AlreadyAsleepText */
        0x94739u, /* PoisonedText */
        0x9474Au, /* BadlyPoisonedText */
        0x9475Eu, /* BurnedText */
        0x9476Du, /* FrozenText */
        0x94782u, /* FireDefrostedText */
        0x94795u, /* MonsStatsRoseText */
        0x947A0u, /* GreatlyRoseText */
        0x947ABu, /* RoseText */
        0x947B3u, /* MonsStatsFellText */
        0x947BEu, /* GreatlyFellText */
        0x947C9u, /* FellText */
        0x947D1u, /* RanFromBattleText */
        0x947E5u, /* RanAwayScaredText */
        0x947F9u, /* WasBlownAwayText */
        0x9480Cu, /* ChargeMoveEffectText */
        0x94810u, /* MadeWhirlwindText */
        0x94824u, /* TookInSunlightText */
        0x94838u, /* LoweredItsHeadText */
        0x9484Cu, /* SkyAttackGlowingText */
        0x9485Au, /* FlewUpHighText */
        0x9486Au, /* DugAHoleText */
        0x94878u, /* BecameConfusedText */
        0x9488Cu, /* MimicLearnedMoveText */
        0x9489Eu, /* MoveWasDisabledText */
        0x948B6u, /* NothingHappenedText */
        0x948C9u, /* NoEffectText */
        0x948D5u, /* ButItFailedText */
        0x948E7u, /* DidntAffectText */
        0x948FBu, /* IsUnaffectedText */
        0x9490Du, /* ParalyzedMayNotAttackText */
        0x9492Fu, /* SubstituteText */
        0x94949u, /* HasSubstituteText */
        0x9495Eu, /* TooWeakSubstituteText */
        0x9497Eu, /* CoinsScatteredText */
        0x9499Bu, /* GettingPumpedText */
        0x949AFu, /* WasSeededText */
        0x949BEu, /* EvadedAttackText */
        0x949D0u, /* HitWithRecoilText */
        0x949E5u, /* ConvertedTypeText */
        0x949FCu, /* StatusChangesEliminatedText */
        0x94A20u, /* StartedSleepingEffect */
        0x94A35u, /* FellAsleepBecameHealthyText */
        0x94A58u, /* RegainedHealthText */
        0x94A6Cu, /* TransformedText */
        0x94A87u, /* LightScreenProtectedText */
        0x94AAEu, /* ReflectGainedArmorText */
        0x94ABFu, /* ShroudedInMistText */
        0x94AD5u, /* SuckedHealthText */
        0x94AECu, /* DreamWasEatenText */
        0x94B01u, /* TradeCenterOpponentText */
        0x94B04u, /* ColosseumOpponentText */
        0xA259Cu, /* PokemartGreetingText */
        0xA25B7u, /* PokemonFaintedText */
        0xA25C5u, /* PlayerBlackedOutText */
        0xA25EFu, /* RepelWoreOffText */
        0xA2608u, /* PokemartBuyingGreetingText */
        0xA2619u, /* PokemartTellBuyPriceText */
        0xA2639u, /* PokemartBoughtItemText */
        0xA2653u, /* PokemartNotEnoughMoneyText */
        0xA2670u, /* PokemartItemBagFullText */
        0xA2690u, /* PokemonSellingGreetingText */
        0xA26AEu, /* PokemartTellSellPriceText */
        0xA26CFu, /* PokemartItemBagEmptyText */
        0xA26F0u, /* PokemartUnsellableItemText */
        0xA270Du, /* PokemartThankYouText */
        0xA2719u, /* PokemartAnythingElseText */
        0xA273Bu, /* LearnedMove1Text */
        0xA2750u, /* WhichMoveToForgetText */
        0xA2771u, /* AbandonLearningText */
        0xA278Au, /* DidNotLearnText */
        0xA27A4u, /* TryingToLearnText */
        0xA2819u, /* OneTwoAndText */
        0xA2827u, /* PoofText */
        0xA2830u, /* ForgotAndText */
        0xA284Du, /* HMCantDeleteText */
        0xA286Du, /* PokemonCenterWelcomeText */
        0xA28B4u, /* ShallWeHealYourPokemonText */
        0xA28CEu, /* NeedYourPokemonText */
        0xA28E8u, /* PokemonFightingFitText */
        0xA2910u, /* PokemonCenterFarewellText */
        0xA292Bu, /* CableClubNPCAreaReservedFor2FriendsLinkedByCableText */
        0xA2969u, /* CableClubNPCWelcomeText */
        0xA2985u, /* CableClubNPCPleaseApplyHereHaveToSaveText */
        0xA29CCu, /* CableClubNPCPleaseWaitText */
        0xA29DBu, /* CableClubNPCLinkClosedBecauseOfInactivityText */
        0xA4000u, /* CableClubNPCPleaseComeAgainText */
        0xA4014u, /* CableClubNPCMakingPreparationsText */
        0xA403Cu, /* UsedStrengthText */
        0xA4051u, /* CanMoveBouldersText */
        0xA4069u, /* CurrentTooFastText */
        0xA4088u, /* CyclingIsFunText */
        0xA40A9u, /* FlashLightsAreaText */
        0xA40CCu, /* WarpToLastPokemonCenterText */
        0xA40EBu, /* CannotUseTeleportNowText */
        0xA4107u, /* CannotFlyHereText */
        0xA411Bu, /* NotHealthyEnoughText */
        0xA4130u, /* NewBadgeRequiredText */
        0xA414Eu, /* CannotUseItemsHereText */
        0xA4168u, /* CannotGetOffHereText */
        0xA4180u, /* GotMonText */
        0xA418Fu, /* SentToBoxText */
        0xA41D6u, /* BoxIsFullText */
        0xA6729u, /* ItemUseBallText00 */
        0xA675Fu, /* ItemUseBallText01 */
        0xA6775u, /* ItemUseBallText02 */
        0xA6791u, /* ItemUseBallText03 */
        0xA67B2u, /* ItemUseBallText04 */
        0xA67CFu, /* ItemUseBallText05 */
        0xA67EEu, /* ItemUseBallText07 */
        0xA6810u, /* ItemUseBallText08 */
        0xA6835u, /* ItemUseBallText06 */
        0xA685Eu, /* SurfingGotOnText */
        0xA686Fu, /* SurfingNoPlaceToGetOffText */
        0xA688Cu, /* VitaminStatRoseText */
        0xA689Eu, /* VitaminNoEffectText */
        0xA68B8u, /* ThrewBaitText */
        0xA68CCu, /* ThrewRockText */
        0xA68DDu, /* PlayedFluteNoEffectText */
        0xA690Cu, /* FluteWokeUpText */
        0xA6928u, /* PlayedFluteHadEffectText */
        0xA6940u, /* CoinCaseNumCoinsText */
        0xA694Fu, /* ItemfinderFoundItemText */
        0xA6981u, /* ItemfinderFoundNothingText */
        0xA69A4u, /* RaisePPWhichTechniqueText */
        0xA69C2u, /* RestorePPWhichTechniqueText */
        0xA69E2u, /* PPMaxedOutText */
        0xA69F9u, /* PPIncreasedText */
        0xA6A0Du, /* PPRestoredText */
        0xA6A1Fu, /* BootedUpTMText */
        0xA6A30u, /* BootedUpHMText */
        0xA6A42u, /* TeachMachineMoveText */
        0xA6A6Eu, /* MonCannotLearnMachineMoveText */
        0xA6AA6u, /* ItemUseNotTimeText */
        0xA6AD0u, /* ItemUseNotYoursToUseText */
        0xA6AE9u, /* ItemUseNoEffectText */
        0xA6B03u, /* ThrowBallAtTrainerMonText1 */
        0xA6B22u, /* ThrowBallAtTrainerMonText2 */
        0xA6B34u, /* NoCyclingAllowedHereText */
        0xA6B4Eu, /* NoSurfingHereText */
        0xA6B69u, /* BoxFullCannotThrowBallText */
        0xA8000u, /* ItemUseText001 */
        0xA8009u, /* ItemUseText002 */
        0xA800Fu, /* GotOnBicycleText1 */
        0xA801Eu, /* GotOnBicycleText2 */
        0xA8024u, /* GotOffBicycleText1 */
        0xA8030u, /* GotOffBicycleText2 */
        0xA803Cu, /* ThrewAwayItemText */
        0xA804Fu, /* IsItOKToTossItemText */
        0xA8068u, /* TooImportantToTossText */
        0xA8088u, /* AlreadyKnowsText */
        0xA809Au, /* ConnectCableText */
        0xA80BCu, /* TradedForText */
        0xA80D8u, /* WannaTrade1Text */
        0xA810Bu, /* NoTrade1Text */
        0xA811Du, /* WrongMon1Text */
        0xA8155u, /* Thanks1Text */
        0xA8162u, /* AfterTrade1Text */
        0xA817Cu, /* WannaTrade2Text */
        0xA81B5u, /* NoTrade2Text */
        0xA81D3u, /* WrongMon2Text */
        0xA8209u, /* Thanks2Text */
        0xA8212u, /* AfterTrade2Text */
        0xA8240u, /* WannaTrade3Text */
        0xA8274u, /* NoTrade3Text */
        0xA8284u, /* WrongMon3Text */
        0xA82BCu, /* Thanks3Text */
        0xA82C9u, /* AfterTrade3Text */
        0xA82F8u, /* NothingToCutText */
        0xA8315u, /* UsedCutText */
    },
    {
        0x80000u, /* CardKeySuccessText1 */
        0x80009u, /* CardKeySuccessText2 */
        0x80029u, /* CardKeyFailText */
        0x80045u, /* TrainerNameText */
        0x8004Du, /* NoNibbleText */
        0x80061u, /* NothingHereText */
        0x80082u, /* ItsABiteText */
        0x80093u, /* ExclamationText */
        0x80096u, /* GroundRoseText */
        0x800B1u, /* BoulderText */
        0x800D2u, /* MartSignText */
        0x800FCu, /* PokeCenterSignText */
        0x80119u, /* FoundItemText */
        0x8012Au, /* NoMoreRoomForItemText */
        0x80143u, /* OaksAideHiText */
        0x801E4u, /* OaksAideUhOhText */
        0x80250u, /* OaksAideComeBackText */
        0x8028Cu, /* OaksAideHereYouGoText */
        0x802D9u, /* OaksAideGotItemText */
        0x802ECu, /* OaksAideNoRoomText */
        0x880BEu, /* AIBattleWithdrawText */
        0x880D5u, /* AIBattleUseItemText */
        0x880EFu, /* TradeWentToText */
        0x88103u, /* TradeForText */
        0x88112u, /* TradeSendsText */
        0x88124u, /* TradeWavesFarewellText */
        0x8813Bu, /* TradeTransferredText */
        0x88150u, /* TradeTakeCareText */
        0x8816Au, /* TradeWillTradeText */
        0x88180u, /* TradeforText */
        0x8818Fu, /* PlaySlotMachineText */
        0x881AEu, /* OutOfCoinsSlotMachineText */
        0x881C7u, /* BetHowManySlotMachineText */
        0x881DCu, /* StartSlotMachineText */
        0x881E4u, /* NotEnoughCoinsSlotMachineText */
        0x881F7u, /* OneMoreGoSlotMachineText */
        0x88206u, /* LinedUpText */
        0x88226u, /* NotThisTimeText */
        0x88236u, /* YeahText */
        0x8823Eu, /* DexSeenOwnedText */
        0x88267u, /* DexRatingText */
        0x88275u, /* GymStatueText1 */
        0x882A5u, /* GymStatueText2 */
        0x882D7u, /* ViridianCityPokecenterGuyText */
        0x8830Cu, /* PewterCityPokecenterGuyText */
        0x88353u, /* CeruleanPokecenterGuyText */
        0x88386u, /* LavenderPokecenterGuyText */
        0x883C2u, /* MtMoonPokecenterBenchGuyText */
        0x883FCu, /* RockTunnelPokecenterGuyText */
        0x88426u, /* UnusedBenchGuyText1 */
        0x88442u, /* UnusedBenchGuyText2 */
        0x88460u, /* UnusedBenchGuyText3 */
        0x8848Eu, /* VermilionPokecenterGuyText */
        0x88531u, /* CeladonCityPokecenterGuyText */
        0x8855Fu, /* FuchsiaCityPokecenterGuyText */
        0x885AFu, /* CinnabarPokecenterGuyText */
        0x88621u, /* SaffronCityPokecenterGuyText1 */
        0x88664u, /* SaffronCityPokecenterGuyText2 */
        0x886A4u, /* CeladonCityHotelText */
        0x886C9u, /* BookcaseText */
        0x886E6u, /* NewBicycleText */
        0x886FCu, /* PushStartText */
        0x8871Au, /* SaveOptionText */
        0x88742u, /* StrengthsAndWeaknessesText */
        0x8877Eu, /* TimesUpText */
        0x88798u, /* GameOverText */
        0x887B7u, /* CinnabarGymQuizIntroText */
        0x8886Du, /* CinnabarQuizQuestionsText1 */
        0x88890u, /* CinnabarQuizQuestionsText2 */
        0x888BBu, /* CinnabarQuizQuestionsText3 */
        0x888D5u, /* CinnabarQuizQuestionsText4 */
        0x88915u, /* CinnabarQuizQuestionsText5 */
        0x88949u, /* CinnabarQuizQuestionsText6 */
        0x88964u, /* CinnabarGymQuizCorrectText */
        0x8898Fu, /* CinnabarGymQuizIncorrectText */
        0x889A1u, /* MagazinesText */
        0x889CFu, /* BillsHouseMonitorText */
        0x889FBu, /* BillsHouseInitiatedText */
        0x88A25u, /* BillsHousePokemonListText1 */
        0x88A40u, /* BillsHousePokemonListText2 */
        0x88A60u, /* OakLabEmailText */
        0x88B5Bu, /* GameCornerCoinCaseText */
        0x88B75u, /* GameCornerNoCoinsText */
        0x88B8Fu, /* GameCornerOutOfOrderText */
        0x88BADu, /* GameCornerOutToLunchText */
        0x88BCDu, /* GameCornerSomeonesKeysText */
        0x88BEDu, /* JustAMomentText */
        0x88C6Fu, /* TurnPageText */
        0x88C7Fu, /* ViridianSchoolNotebookText5 */
        0x88CA3u, /* ViridianSchoolNotebookText1 */
        0x88D46u, /* ViridianSchoolNotebookText2 */
        0x88DBDu, /* ViridianSchoolNotebookText3 */
        0x88E2Cu, /* ViridianSchoolNotebookText4 */
        0x88EC1u, /* EnemiesOnEverySideText */
        0x88ED9u, /* WhatGoesAroundComesAroundText */
        0x88EF9u, /* FightingDojoText */
        0x88F08u, /* IndigoPlateauHQText */
        0x88F27u, /* RedBedroomSNESText */
        0x88F58u, /* Route15UpstairsBinocularsText */
        0x88FA7u, /* AerodactylFossilText */
        0x88FD5u, /* KabutopsFossilText */
        0x89001u, /* LinkCableHelpText1 */
        0x89027u, /* LinkCableHelpText2 */
        0x8904Bu, /* LinkCableInfoText1 */
        0x890BDu, /* LinkCableInfoText2 */
        0x890E8u, /* LinkCableInfoText3 */
        0x89110u, /* ViridianSchoolBlackboardText1 */
        0x8914Eu, /* ViridianSchoolBlackboardText2 */
        0x89172u, /* ViridianBlackboardSleepText */
        0x891DEu, /* ViridianBlackboardPoisonText */
        0x8924Bu, /* ViridianBlackboardPrlzText */
        0x892B5u, /* ViridianBlackboardBurnText */
        0x8932Fu, /* ViridianBlackboardFrozenText */
        0x893A7u, /* VermilionGymTrashText */
        0x893C6u, /* VermilionGymTrashSuccessText1 */
        0x89418u, /* VermilionGymTrashSuccessText2 */
        0x89451u, /* VermilionGymTrashSuccessText3 */
        0x8948Cu, /* VermilionGymTrashFailText */
        0x894D0u, /* FoundHiddenItemText */
        0x894E1u, /* HiddenItemBagFullText */
        0x8950Bu, /* FoundHiddenCoinsText */
        0x89523u, /* FoundHiddenCoins2Text */
        0x8953Bu, /* DroppedHiddenCoinsText */
        0x89557u, /* IndigoPlateauStatuesText1 */
        0x89567u, /* IndigoPlateauStatuesText2 */
        0x89596u, /* IndigoPlateauStatuesText3 */
        0x895C1u, /* PokemonBooksText */
        0x895DEu, /* DiglettSculptureText */
        0x895FBu, /* ElevatorText */
        0x89611u, /* TownMapText */
        0x8961Fu, /* PokemonStuffText */
        0x89639u, /* OutOfSafariBallsText */
        0x89666u, /* WildRanText */
        0x89677u, /* EnemyRanText */
        0x89689u, /* HurtByPoisonText */
        0x8969Du, /* HurtByBurnText */
        0x896B3u, /* HurtByLeechSeedText */
        0x896C7u, /* EnemyMonFaintedText */
        0x896DDu, /* MoneyForWinningText */
        0x896F9u, /* TrainerDefeatedText */
        0x8970Cu, /* PlayerMonFaintedText */
        0x8971Au, /* UseNextMonText */
        0x8972Au, /* Rival1WinText */
        0x89748u, /* PlayerBlackedOutText2 */
        0x89772u, /* LinkBattleLostText */
        0x89784u, /* TrainerAboutToUseText */
        0x897B4u, /* TrainerSentOutText */
        0x897C9u, /* NoWillText */
        0x897E3u, /* CantEscapeText */
        0x897F1u, /* NoRunningText */
        0x8981Fu, /* GotAwayText */
        0x89831u, /* ItemsCantBeUsedHereText */
        0x8984Bu, /* AlreadyOutText */
        0x89860u, /* MoveNoPPText */
        0x8987Bu, /* MoveDisabledText */
        0x89892u, /* NoMovesLeftText */
        0x898AAu, /* MultiHitText */
        0x898C7u, /* ScaredText */
        0x898E3u, /* GetOutText */
        0x89901u, /* FastAsleepText */
        0x89914u, /* WokeUpText */
        0x89920u, /* IsFrozenText */
        0x89934u, /* FullyParalyzedText */
        0x89949u, /* FlinchedText */
        0x89956u, /* MustRechargeText */
        0x89968u, /* DisabledNoMoreText */
        0x8997Eu, /* IsConfusedText */
        0x8998Eu, /* HurtItselfText */
        0x899B0u, /* ConfusedNoMoreText */
        0x899C6u, /* SavingEnergyText */
        0x899DBu, /* UnleashedEnergyText */
        0x899F0u, /* ThrashingAboutText */
        0x89A05u, /* AttackContinuesText */
        0x89A1Bu, /* CantMoveText */
        0x89A29u, /* MoveIsDisabledText */
        0x89A40u, /* ActorNameText */
        0x89A44u, /* UsedMove1Text */
        0x89A4Du, /* UsedMove2Text */
        0x89A56u, /* UsedInsteadText */
        0x89A62u, /* MoveNameText */
        0x89A67u, /* EndUsedMove1Text */
        0x89A6Au, /* EndUsedMove2Text */
        0x89A6Du, /* EndUsedMove3Text */
        0x89A70u, /* EndUsedMove4Text */
        0x89A73u, /* EndUsedMove5Text */
        0x89A76u, /* AttackMissedText */
        0x89A89u, /* KeptGoingAndCrashedText */
        0x89AA4u, /* UnaffectedText */
        0x89AB4u, /* DoesntAffectMonText */
        0x89AC9u, /* CriticalHitText */
        0x89AD8u, /* OHKOText */
        0x89AE5u, /* LoafingAroundText */
        0x89AFDu, /* BeganToNapText */
        0x89B10u, /* WontObeyText */
        0x89B20u, /* TurnedAwayText */
        0x89B32u, /* IgnoredOrdersText */
        0x89B47u, /* SubstituteTookDamageText */
        0x89B6Au, /* SubstituteBrokeText */
        0x89B80u, /* BuildingRageText */
        0x89B96u, /* MirrorMoveFailedText */
        0x89BAFu, /* HitXTimesText */
        0x89BC2u, /* GainedText */
        0x89BD0u, /* WithExpAllText */
        0x89BE1u, /* BoostedText */
        0x89BEEu, /* ExpPointsText */
        0x89C01u, /* GrewLevelText */
        0x89C1Du, /* WildMonAppearedText */
        0x89C33u, /* HookedMonAttackedText */
        0x89C4Fu, /* EnemyAppearedText */
        0x89C5Eu, /* TrainerWantsToFightText */
        0x89C73u, /* UnveiledGhostText */
        0x89C9Eu, /* GhostCantBeIDdText */
        0x89CBCu, /* GoText */
        0x89CC3u, /* DoItText */
        0x89CCDu, /* GetmText */
        0x89CD6u, /* EnemysWeakText */
        0x89CF0u, /* PlayerMon1Text */
        0x89CF6u, /* PlayerMon2Text */
        0x89CFDu, /* EnoughText */
        0x89D07u, /* OKExclamationText */
        0x89D0Du, /* GoodText */
        0x89D15u, /* ComeBackText */
        0x89D22u, /* SuperEffectiveText */
        0x89D38u, /* NotVeryEffectiveText */
        0x89D53u, /* SafariZoneEatingText */
        0x89D6Au, /* SafariZoneAngryText */
        0x89D80u, /* PickUpPayDayMoneyText */
        0x89D96u, /* ClearSaveDataText */
        0x89DADu, /* WhichFloorText */
        0x89DC8u, /* PartyMenuNormalText */
        0x89DD8u, /* PartyMenuItemUseText */
        0x89DF1u, /* PartyMenuBattleText */
        0x89E08u, /* PartyMenuUseTMText */
        0x89E1Fu, /* PartyMenuSwapMonText */
        0x89E31u, /* PotionText */
        0x89E4Bu, /* AntidoteText */
        0x89E65u, /* ParlyzHealText */
        0x89E7Du, /* BurnHealText */
        0x89E94u, /* IceHealText */
        0x89EA8u, /* AwakeningText */
        0x89EB6u, /* FullHealText */
        0x89ECDu, /* ReviveText */
        0x89EE2u, /* RareCandyText */
        0x89EFEu, /* TurnedOnPC1Text */
        0x89F13u, /* AccessedBillsPCText */
        0x89F45u, /* AccessedSomeonesPCText */
        0x89F7Au, /* AccessedMyPCText */
        0x89FA9u, /* TurnedOnPC2Text */
        0x89FBEu, /* WhatDoYouWantText */
        0x89FD7u, /* WhatToDepositText */
        0x89FF5u, /* DepositHowManyText */
        0x8A000u, /* ItemWasStoredText */
        0x8A018u, /* NothingToDepositText */
        0x8A036u, /* NoRoomToStoreText */
        0x8A054u, /* WhatToWithdrawText */
        0x8A073u, /* WithdrawHowManyText */
        0x8A07Eu, /* WithdrewItemText */
        0x8A08Fu, /* NothingStoredText */
        0x8A0A9u, /* CantCarryMoreText */
        0x8A0C9u, /* WhatToTossText */
        0x8A0E9u, /* TossHowManyText */
        0x8A0F4u, /* AccessedHoFPCText */
        0x8A131u, /* SwitchOnText */
        0x8A13Du, /* WhatText */
        0x8A144u, /* DepositWhichMonText */
        0x8A159u, /* MonWasStoredText */
        0x8A177u, /* CantDepositLastMonText */
        0x8A198u, /* BoxFullText */
        0x8A1B9u, /* MonIsTakenOutText */
        0x8A1D7u, /* NoMonText */
        0x8A1F6u, /* CantTakeMonText */
        0x8A228u, /* ReleaseWhichMonText */
        0x8A23Du, /* OnceReleasedText */
        0x8A268u, /* MonWasReleasedText */
        0x8A28Eu, /* RequireCoinCaseText */
        0x8A2A9u, /* ExchangeCoinsForPrizesText */
        0x8A2CDu, /* WhichPrizeText */
        0x8A2E7u, /* HereYouGoText */
        0x8A2F6u, /* SoYouWantPrizeText */
        0x8A30Bu, /* SorryNeedMoreCoinsText */
        0x8A329u, /* OopsYouDontHaveEnoughRoomText */
        0x8A34Cu, /* OhFineThenText */
        0x8A35Du, /* GetDexRatedText */
        0x8A37Bu, /* ClosedOaksPCText */
        0x8A39Au, /* AccessedOaksPCText */
        0x8A3D0u, /* WhereWouldYouLikeText */
        0x8A3EDu, /* PleaseWaitText */
        0x8A40Du, /* LinkCanceledText */
        0x8A425u, /* OakSpeechText1 */
        0x8A47Fu, /* OakSpeechText2A */
        0x8A4B3u, /* OakSpeechText2B */
        0x8A519u, /* IntroducePlayerText */
        0x8A534u, /* IntroduceRivalText */
        0x8A597u, /* OakSpeechText3 */
        0x8A605u, /* DoYouWantToNicknameText */
        0x8A62Fu, /* YourNameIsText */
        0x8A64Au, /* HisNameIsText */
        0x8A677u, /* WillBeTradedText */
        0x8A696u, /* TextIDErrorText */
        0x8A6A3u, /* ContCharText */
        0x945F1u, /* FileDataDestroyedText */
        0x9460Eu, /* WouldYouLikeToSaveText */
        0x94630u, /* GameSavedText */
        0x94643u, /* OlderFileWillBeErasedText */
        0x94671u, /* WhenYouChangeBoxText */
        0x946B0u, /* ChooseABoxText */
        0x946C2u, /* EvolvedText */
        0x946CFu, /* IntoText */
        0x946DDu, /* StoppedEvolvingText */
        0x946FBu, /* IsEvolvingText */
        0x94715u, /* FellAsleepText */
        0x94725u, /* AlreadyAsleepText */
        0x94739u, /* PoisonedText */
        0x9474Au, /* BadlyPoisonedText */
        0x9475Eu, /* BurnedText */
        0x9476Du, /* FrozenText */
        0x94782u, /* FireDefrostedText */
        0x94795u, /* MonsStatsRoseText */
        0x947A0u, /* GreatlyRoseText */
        0x947ABu, /* RoseText */
        0x947B3u, /* MonsStatsFellText */
        0x947BEu, /* GreatlyFellText */
        0x947C9u, /* FellText */
        0x947D1u, /* RanFromBattleText */
        0x947E5u, /* RanAwayScaredText */
        0x947F9u, /* WasBlownAwayText */
        0x9480Cu, /* ChargeMoveEffectText */
        0x94810u, /* MadeWhirlwindText */
        0x94824u, /* TookInSunlightText */
        0x94838u, /* LoweredItsHeadText */
        0x9484Cu, /* SkyAttackGlowingText */
        0x9485Au, /* FlewUpHighText */
        0x9486Au, /* DugAHoleText */
        0x94878u, /* BecameConfusedText */
        0x9488Cu, /* MimicLearnedMoveText */
        0x9489Eu, /* MoveWasDisabledText */
        0x948B6u, /* NothingHappenedText */
        0x948C9u, /* NoEffectText */
        0x948D5u, /* ButItFailedText */
        0x948E7u, /* DidntAffectText */
        0x948FBu, /* IsUnaffectedText */
        0x9490Du, /* ParalyzedMayNotAttackText */
        0x9492Fu, /* SubstituteText */
        0x94949u, /* HasSubstituteText */
        0x9495Eu, /* TooWeakSubstituteText */
        0x9497Eu, /* CoinsScatteredText */
        0x9499Bu, /* GettingPumpedText */
        0x949AFu, /* WasSeededText */
        0x949BEu, /* EvadedAttackText */
        0x949D0u, /* HitWithRecoilText */
        0x949E5u, /* ConvertedTypeText */
        0x949FCu, /* StatusChangesEliminatedText */
        0x94A20u, /* StartedSleepingEffect */
        0x94A35u, /* FellAsleepBecameHealthyText */
        0x94A58u, /* RegainedHealthText */
        0x94A6Cu, /* TransformedText */
        0x94A87u, /* LightScreenProtectedText */
        0x94AAEu, /* ReflectGainedArmorText */
        0x94ABFu, /* ShroudedInMistText */
        0x94AD5u, /* SuckedHealthText */
        0x94AECu, /* DreamWasEatenText */
        0x94B01u, /* TradeCenterOpponentText */
        0x94B04u, /* ColosseumOpponentText */
        0xA259Cu, /* PokemartGreetingText */
        0xA25B7u, /* PokemonFaintedText */
        0xA25C5u, /* PlayerBlackedOutText */
        0xA25EFu, /* RepelWoreOffText */
        0xA2608u, /* PokemartBuyingGreetingText */
        0xA2619u, /* PokemartTellBuyPriceText */
        0xA2639u, /* PokemartBoughtItemText */
        0xA2653u, /* PokemartNotEnoughMoneyText */
        0xA2670u, /* PokemartItemBagFullText */
        0xA2690u, /* PokemonSellingGreetingText */
        0xA26AEu, /* PokemartTellSellPriceText */
        0xA26CFu, /* PokemartItemBagEmptyText */
        0xA26F0u, /* PokemartUnsellableItemText */
        0xA270Du, /* PokemartThankYouText */
        0xA2719u, /* PokemartAnythingElseText */
        0xA273Bu, /* LearnedMove1Text */
        0xA2750u, /* WhichMoveToForgetText */
        0xA2771u, /* AbandonLearningText */
        0xA278Au, /* DidNotLearnText */
        0xA27A4u, /* TryingToLearnText */
        0xA2819u, /* OneTwoAndText */
        0xA2827u, /* PoofText */
        0xA2830u, /* ForgotAndText */
        0xA284Du, /* HMCantDeleteText */
        0xA286Du, /* PokemonCenterWelcomeText */
        0xA28B4u, /* ShallWeHealYourPokemonText */
        0xA28CEu, /* NeedYourPokemonText */
        0xA28E8u, /* PokemonFightingFitText */
        0xA2910u, /* PokemonCenterFarewellText */
        0xA292Bu, /* CableClubNPCAreaReservedFor2FriendsLinkedByCableText */
        0xA2969u, /* CableClubNPCWelcomeText */
        0xA2985u, /* CableClubNPCPleaseApplyHereHaveToSaveText */
        0xA29CCu, /* CableClubNPCPleaseWaitText */
        0xA29DBu, /* CableClubNPCLinkClosedBecauseOfInactivityText */
        0xA4000u, /* CableClubNPCPleaseComeAgainText */
        0xA4014u, /* CableClubNPCMakingPreparationsText */
        0xA403Cu, /* UsedStrengthText */
        0xA4051u, /* CanMoveBouldersText */
        0xA4069u, /* CurrentTooFastText */
        0xA4088u, /* CyclingIsFunText */
        0xA40A9u, /* FlashLightsAreaText */
        0xA40CCu, /* WarpToLastPokemonCenterText */
        0xA40EBu, /* CannotUseTeleportNowText */
        0xA4107u, /* CannotFlyHereText */
        0xA411Bu, /* NotHealthyEnoughText */
        0xA4130u, /* NewBadgeRequiredText */
        0xA414Eu, /* CannotUseItemsHereText */
        0xA4168u, /* CannotGetOffHereText */
        0xA4180u, /* GotMonText */
        0xA418Fu, /* SentToBoxText */
        0xA41D6u, /* BoxIsFullText */
        0xA6729u, /* ItemUseBallText00 */
        0xA675Fu, /* ItemUseBallText01 */
        0xA6775u, /* ItemUseBallText02 */
        0xA6791u, /* ItemUseBallText03 */
        0xA67B2u, /* ItemUseBallText04 */
        0xA67CFu, /* ItemUseBallText05 */
        0xA67EEu, /* ItemUseBallText07 */
        0xA6810u, /* ItemUseBallText08 */
        0xA6835u, /* ItemUseBallText06 */
        0xA685Eu, /* SurfingGotOnText */
        0xA686Fu, /* SurfingNoPlaceToGetOffText */
        0xA688Cu, /* VitaminStatRoseText */
        0xA689Eu, /* VitaminNoEffectText */
        0xA68B8u, /* ThrewBaitText */
        0xA68CCu, /* ThrewRockText */
        0xA68DDu, /* PlayedFluteNoEffectText */
        0xA690Cu, /* FluteWokeUpText */
        0xA6928u, /* PlayedFluteHadEffectText */
        0xA6940u, /* CoinCaseNumCoinsText */
        0xA694Fu, /* ItemfinderFoundItemText */
        0xA6981u, /* ItemfinderFoundNothingText */
        0xA69A4u, /* RaisePPWhichTechniqueText */
        0xA69C2u, /* RestorePPWhichTechniqueText */
        0xA69E2u, /* PPMaxedOutText */
        0xA69F9u, /* PPIncreasedText */
        0xA6A0Du, /* PPRestoredText */
        0xA6A1Fu, /* BootedUpTMText */
        0xA6A30u, /* BootedUpHMText */
        0xA6A42u, /* TeachMachineMoveText */
        0xA6A6Eu, /* MonCannotLearnMachineMoveText */
        0xA6AA6u, /* ItemUseNotTimeText */
        0xA6AD0u, /* ItemUseNotYoursToUseText */
        0xA6AE9u, /* ItemUseNoEffectText */
        0xA6B03u, /* ThrowBallAtTrainerMonText1 */
        0xA6B22u, /* ThrowBallAtTrainerMonText2 */
        0xA6B34u, /* NoCyclingAllowedHereText */
        0xA6B4Eu, /* NoSurfingHereText */
        0xA6B69u, /* BoxFullCannotThrowBallText */
        0xA8000u, /* ItemUseText001 */
        0xA8009u, /* ItemUseText002 */
        0xA800Fu, /* GotOnBicycleText1 */
        0xA801Eu, /* GotOnBicycleText2 */
        0xA8024u, /* GotOffBicycleText1 */
        0xA8030u, /* GotOffBicycleText2 */
        0xA803Cu, /* ThrewAwayItemText */
        0xA804Fu, /* IsItOKToTossItemText */
        0xA8068u, /* TooImportantToTossText */
        0xA8088u, /* AlreadyKnowsText */
        0xA809Au, /* ConnectCableText */
        0xA80BCu, /* TradedForText */
        0xA80D8u, /* WannaTrade1Text */
        0xA810Bu, /* NoTrade1Text */
        0xA811Du, /* WrongMon1Text */
        0xA8155u, /* Thanks1Text */
        0xA8162u, /* AfterTrade1Text */
        0xA817Cu, /* WannaTrade2Text */
        0xA81B5u, /* NoTrade2Text */
        0xA81D3u, /* WrongMon2Text */
        0xA8209u, /* Thanks2Text */
        0xA8212u, /* AfterTrade2Text */
        0xA8240u, /* WannaTrade3Text */
        0xA8274u, /* NoTrade3Text */
        0xA8284u, /* WrongMon3Text */
        0xA82BCu, /* Thanks3Text */
        0xA82C9u, /* AfterTrade3Text */
        0xA82F8u, /* NothingToCutText */
        0xA8315u, /* UsedCutText */
    },
};
#endif
