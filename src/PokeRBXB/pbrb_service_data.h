#ifndef PBRB_SERVICE_DATA_H_INCLUDED
#define PBRB_SERVICE_DATA_H_INCLUDED
/* Original US/EU Red/Blue ROM labels, derived from pokered symbols.
   Only addresses are compiled here; texts, graphics and tables stay in ROM. */
enum PbrbServiceRomLabel {
    PSR_DaycareGentlemanText_IntroText,
    PSR_DaycareGentlemanText_WhichMonText,
    PSR_DaycareGentlemanText_WillLookAfterMonText,
    PSR_DaycareGentlemanText_ComeSeeMeInAWhileText,
    PSR_DaycareGentlemanText_MonHasGrownText,
    PSR_DaycareGentlemanText_OweMoneyText,
    PSR_DaycareGentlemanText_GotMonBackText,
    PSR_DaycareGentlemanText_MonNeedsMoreTimeText,
    PSR_DaycareGentlemanText_AllRightThenText,
    PSR_DaycareGentlemanText_ComeAgainText,
    PSR_DaycareGentlemanText_NoRoomForMonText,
    PSR_DaycareGentlemanText_OnlyHaveOneMonText,
    PSR_DaycareGentlemanText_CantAcceptMonWithHMText,
    PSR_DaycareGentlemanText_HeresYourMonText,
    PSR_DaycareGentlemanText_NotEnoughMoneyText,
    PSR_TradeMons,
    PSR_InGameTradeTextPointers,
    PSR_InGameTrade_TrainerString,
    PSR_DexCompletionText,
    PSR_DexRatingsTable,
    PSR_OwnedMonValues,
    PSR_PokedexRatingSfxPointers,
    PSR_PrizeDifferentMenuPtrs,
    PSR_PrizeMonLevelDictionary,
    PSR_NoThanksText,
    PSR_SlotMachineMap,
    PSR_SlotMachineWheel1,
    PSR_SlotMachineWheel2,
    PSR_SlotMachineWheel3,
    PSR_SlotMachineTiles1,
    PSR_SlotMachineTiles2,
    PSR_SlotRewardPointers,
    PSR_TypeNames,
    PSR_HallOfFameText,
    PSR_HoFMonInfoText,
    PSR_HoFPlayTimeText,
    PSR_HoFMoneyText,
    PSR_HallOfFameNoText,
    PSR_GameCornerClerk1Text_DoYouNeedSomeGameCoins,
    PSR_GameCornerClerk1Text_ThanksHereAre50Coins,
    PSR_GameCornerClerk1Text_PleaseComePlaySometime,
    PSR_GameCornerClerk1Text_CantAffordTheCoins,
    PSR_GameCornerClerk1Text_CoinCaseIsFull,
    PSR_GameCornerClerk1Text_DontHaveCoinCase,
    PSR_TradingAnimationGraphics,
    PSR_TradingAnimationGraphics2,
    PSR_GameBoyTiles,
    PSR_LinkCableTiles,
    PSR_Trade_MonInfoText,
    PSR_MonPartySpritePointers,
    PSR_MonPartyData,
    PSR_Trade_CircleOAMBlocks,
    PSR_VendingMachineText1,
    PSR_VendingMachineText4,
    PSR_VendingMachineText5,
    PSR_VendingMachineText6,
    PSR_VendingMachineText7,
    PSR_DrinkText,
    PSR_DrinkPriceText,
    PSR_VendingPrices,
    PSR_SilphCoElevatorFloors,
    PSR_FossilsList,
    PSR_GotMonText,
    PSR_HiddenEventsFor_GAME_CORNER,
    PSR_COUNT
};
static const unsigned kPbrbServiceRom[2][PSR_COUNT] = {
    {
        0x5640fu, /* DaycareGentlemanText.IntroText */
        0x56414u, /* DaycareGentlemanText.WhichMonText */
        0x56419u, /* DaycareGentlemanText.WillLookAfterMonText */
        0x5641eu, /* DaycareGentlemanText.ComeSeeMeInAWhileText */
        0x56423u, /* DaycareGentlemanText.MonHasGrownText */
        0x56428u, /* DaycareGentlemanText.OweMoneyText */
        0x5642du, /* DaycareGentlemanText.GotMonBackText */
        0x56432u, /* DaycareGentlemanText.MonNeedsMoreTimeText */
        0x56437u, /* DaycareGentlemanText.AllRightThenText */
        0x5643bu, /* DaycareGentlemanText.ComeAgainText */
        0x56440u, /* DaycareGentlemanText.NoRoomForMonText */
        0x56445u, /* DaycareGentlemanText.OnlyHaveOneMonText */
        0x5644au, /* DaycareGentlemanText.CantAcceptMonWithHMText */
        0x5644fu, /* DaycareGentlemanText.HeresYourMonText */
        0x56454u, /* DaycareGentlemanText.NotEnoughMoneyText */
        0x71b7bu, /* TradeMons */
        0x71d64u, /* InGameTradeTextPointers */
        0x71d59u, /* InGameTrade_TrainerString */
        0x441ccu, /* DexCompletionText */
        0x441d1u, /* DexRatingsTable */
        0x7d170u, /* OwnedMonValues */
        0x7d162u, /* PokedexRatingSfxPointers */
        0x52843u, /* PrizeDifferentMenuPtrs */
        0x5298au, /* PrizeMonLevelDictionary */
        0x5284fu, /* NoThanksText */
        0x378f5u, /* SlotMachineMap */
        0x379e5u, /* SlotMachineWheel1 */
        0x37a09u, /* SlotMachineWheel2 */
        0x37a2du, /* SlotMachineWheel3 */
        0x37a51u, /* SlotMachineTiles1 */
        0x78bdeu, /* SlotMachineTiles2 */
        0x37678u, /* SlotRewardPointers */
        0x27daeu, /* TypeNames */
        0x7026bu, /* HallOfFameText */
        0x70329u, /* HoFMonInfoText */
        0x703eau, /* HoFPlayTimeText */
        0x703f4u, /* HoFMoneyText */
        0x76670u, /* HallOfFameNoText */
        0x48d22u, /* GameCornerClerk1Text.DoYouNeedSomeGameCoins */
        0x48d27u, /* GameCornerClerk1Text.ThanksHereAre50Coins */
        0x48d2cu, /* GameCornerClerk1Text.PleaseComePlaySometime */
        0x48d31u, /* GameCornerClerk1Text.CantAffordTheCoins */
        0x48d36u, /* GameCornerClerk1Text.CoinCaseIsFull */
        0x48d3bu, /* GameCornerClerk1Text.DontHaveCoinCase */
        0x3a9beu, /* TradingAnimationGraphics */
        0x3acceu, /* TradingAnimationGraphics2 */
        0x79c20u, /* GameBoyTiles */
        0x79c50u, /* LinkCableTiles */
        0x427e5u, /* Trade_MonInfoText */
        0x717c0u, /* MonPartySpritePointers */
        0x7190du, /* MonPartyData */
        0x41574u, /* Trade_CircleOAMBlocks */
        0x74f99u, /* VendingMachineText1 */
        0x74fd3u, /* VendingMachineText4 */
        0x74fd8u, /* VendingMachineText5 */
        0x74fddu, /* VendingMachineText6 */
        0x74fe2u, /* VendingMachineText7 */
        0x74f9eu, /* DrinkText */
        0x74fc3u, /* DrinkPriceText */
        0x75000u, /* VendingPrices */
        0x45804u, /* SilphCoElevatorFloors */
        0x75d68u, /* FossilsList */
        0x4fe39u, /* GotMonText */
        0x46ca6u, /* HiddenEventsFor_GAME_CORNER */
    },
    {
        0x5640fu, /* DaycareGentlemanText.IntroText */
        0x56414u, /* DaycareGentlemanText.WhichMonText */
        0x56419u, /* DaycareGentlemanText.WillLookAfterMonText */
        0x5641eu, /* DaycareGentlemanText.ComeSeeMeInAWhileText */
        0x56423u, /* DaycareGentlemanText.MonHasGrownText */
        0x56428u, /* DaycareGentlemanText.OweMoneyText */
        0x5642du, /* DaycareGentlemanText.GotMonBackText */
        0x56432u, /* DaycareGentlemanText.MonNeedsMoreTimeText */
        0x56437u, /* DaycareGentlemanText.AllRightThenText */
        0x5643bu, /* DaycareGentlemanText.ComeAgainText */
        0x56440u, /* DaycareGentlemanText.NoRoomForMonText */
        0x56445u, /* DaycareGentlemanText.OnlyHaveOneMonText */
        0x5644au, /* DaycareGentlemanText.CantAcceptMonWithHMText */
        0x5644fu, /* DaycareGentlemanText.HeresYourMonText */
        0x56454u, /* DaycareGentlemanText.NotEnoughMoneyText */
        0x71b7bu, /* TradeMons */
        0x71d64u, /* InGameTradeTextPointers */
        0x71d59u, /* InGameTrade_TrainerString */
        0x441ccu, /* DexCompletionText */
        0x441d1u, /* DexRatingsTable */
        0x7d170u, /* OwnedMonValues */
        0x7d162u, /* PokedexRatingSfxPointers */
        0x52843u, /* PrizeDifferentMenuPtrs */
        0x5298au, /* PrizeMonLevelDictionary */
        0x5284fu, /* NoThanksText */
        0x378f5u, /* SlotMachineMap */
        0x379e5u, /* SlotMachineWheel1 */
        0x37a09u, /* SlotMachineWheel2 */
        0x37a2du, /* SlotMachineWheel3 */
        0x37a51u, /* SlotMachineTiles1 */
        0x78bdeu, /* SlotMachineTiles2 */
        0x37678u, /* SlotRewardPointers */
        0x27daeu, /* TypeNames */
        0x7026bu, /* HallOfFameText */
        0x70329u, /* HoFMonInfoText */
        0x703eau, /* HoFPlayTimeText */
        0x703f4u, /* HoFMoneyText */
        0x76671u, /* HallOfFameNoText */
        0x48d22u, /* GameCornerClerk1Text.DoYouNeedSomeGameCoins */
        0x48d27u, /* GameCornerClerk1Text.ThanksHereAre50Coins */
        0x48d2cu, /* GameCornerClerk1Text.PleaseComePlaySometime */
        0x48d31u, /* GameCornerClerk1Text.CantAffordTheCoins */
        0x48d36u, /* GameCornerClerk1Text.CoinCaseIsFull */
        0x48d3bu, /* GameCornerClerk1Text.DontHaveCoinCase */
        0x3a9beu, /* TradingAnimationGraphics */
        0x3acceu, /* TradingAnimationGraphics2 */
        0x79c20u, /* GameBoyTiles */
        0x79c50u, /* LinkCableTiles */
        0x427e5u, /* Trade_MonInfoText */
        0x717c0u, /* MonPartySpritePointers */
        0x7190du, /* MonPartyData */
        0x41574u, /* Trade_CircleOAMBlocks */
        0x74f9au, /* VendingMachineText1 */
        0x74fd4u, /* VendingMachineText4 */
        0x74fd9u, /* VendingMachineText5 */
        0x74fdeu, /* VendingMachineText6 */
        0x74fe3u, /* VendingMachineText7 */
        0x74f9fu, /* DrinkText */
        0x74fc4u, /* DrinkPriceText */
        0x75001u, /* VendingPrices */
        0x45804u, /* SilphCoElevatorFloors */
        0x75d69u, /* FossilsList */
        0x4fe39u, /* GotMonText */
        0x46ca6u, /* HiddenEventsFor_GAME_CORNER */
    },
};
#endif
#pragma once
