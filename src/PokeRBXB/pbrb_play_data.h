#ifndef POKERBXB_NATIVE_PLAY_ROM_DATA_H_INCLUDED
#define POKERBXB_NATIVE_PLAY_ROM_DATA_H_INCLUDED
/* Addresses only, from the matching US/EU Red and Blue disassembly builds.
   All pictures, text and tables are read from the player-supplied ROM. */
enum PbrbPlayData {
    PBRB_DATA_ProfOakPic,
    PBRB_DATA_RedPicFront,
    PBRB_DATA_Rival1Pic,
    PBRB_DATA_ShrinkPic1,
    PBRB_DATA_ShrinkPic2,
    PBRB_DATA_RedPicBack,
    PBRB_DATA_RedSprite,
    PBRB_DATA_LowerCaseAlphabet,
    PBRB_DATA_UpperCaseAlphabet,
    PBRB_DATA_ED_Tile,
    PBRB_DATA_DefaultNamesPlayer,
    PBRB_DATA_DefaultNamesRival,
    PBRB_DATA_DefaultNamesPlayerList,
    PBRB_DATA_DefaultNamesRivalList,
    PBRB_DATA_DownscaledMonTiles_5x5,
    PBRB_DATA_DownscaledMonTiles_3x3,
    PBRB_DATA_MonsterNames,
    PBRB_DATA_MoveNames,
    PBRB_DATA_BaseStats,
    PBRB_DATA_PokedexOrder,
    PBRB_DATA_Moves,
    PBRB_DATA_HpBarAndStatusGraphics,
    PBRB_DATA_BattleHudTiles1,
    PBRB_DATA_BattleHudTiles2,
    PBRB_DATA_BattleHudTiles3,
    PBRB_DATA_PokeballTileGraphics,
    PBRB_DATA_OakSpeechText1,
    PBRB_DATA_OakSpeechText2A,
    PBRB_DATA_OakSpeechText2B,
    PBRB_DATA_IntroducePlayerText,
    PBRB_DATA_IntroduceRivalText,
    PBRB_DATA_OakSpeechText3,
    PBRB_DATA_YourNameIsText,
    PBRB_DATA_HisNameIsText,
    PBRB_DATA_DoYouWantToNicknameText,
    PBRB_DATA_TrainerSentOutText,
    PBRB_DATA_TrainerWantsToFightText,
    PBRB_DATA_GoText,
    PBRB_DATA_PlayerMon1Text,
    PBRB_DATA_WouldYouLikeToSaveText,
    PBRB_DATA_GameSavedText,
    PBRB_DATA_OlderFileWillBeErasedText,
    PBRB_DATA_WhatDoYouWantText,
    PBRB_DATA_WithdrewItemText,
    PBRB_DATA_NothingStoredText,
    PBRB_DATA_CantEscapeText,
    PBRB_DATA_TurnedOnPC1Text,
    PBRB_DATA_COUNT
};
static const unsigned kPbrbPlayData[2][PBRB_DATA_COUNT] = {
    {
        0x4E15Fu, /* ProfOakPic */
        0x12EDEu, /* RedPicFront */
        0x4E049u, /* Rival1Pic */
        0x12FE8u, /* ShrinkPic1 */
        0x13042u, /* ShrinkPic2 */
        0x33E0Au, /* RedPicBack */
        0x14180u, /* RedSprite */
        0x679Eu, /* LowerCaseAlphabet */
        0x67D6u, /* UpperCaseAlphabet */
        0x6767u, /* ED_Tile */
        0x6AA8u, /* DefaultNamesPlayer */
        0x6ABEu, /* DefaultNamesRival */
        0x6AF2u, /* DefaultNamesPlayerList */
        0x6B08u, /* DefaultNamesRivalList */
        0x79B02u, /* DownscaledMonTiles_5x5 */
        0x79B1Bu, /* DownscaledMonTiles_3x3 */
        0x1C21Eu, /* MonsterNames */
        0xB0000u, /* MoveNames */
        0x383DEu, /* BaseStats */
        0x41024u, /* PokedexOrder */
        0x38000u, /* Moves */
        0x11EA0u, /* HpBarAndStatusGraphics */
        0x12080u, /* BattleHudTiles1 */
        0x12098u, /* BattleHudTiles2 */
        0x120B0u, /* BattleHudTiles3 */
        0x3A97Eu, /* PokeballTileGraphics */
        0x8A425u, /* _OakSpeechText1 */
        0x8A47Fu, /* _OakSpeechText2A */
        0x8A4B3u, /* _OakSpeechText2B */
        0x8A519u, /* _IntroducePlayerText */
        0x8A534u, /* _IntroduceRivalText */
        0x8A597u, /* _OakSpeechText3 */
        0x8A62Fu, /* _YourNameIsText */
        0x8A64Au, /* _HisNameIsText */
        0x8A605u, /* _DoYouWantToNicknameText */
        0x897B4u, /* _TrainerSentOutText */
        0x89C5Eu, /* _TrainerWantsToFightText */
        0x89CBCu, /* _GoText */
        0x89CF0u, /* _PlayerMon1Text */
        0x9460Eu, /* _WouldYouLikeToSaveText */
        0x94630u, /* _GameSavedText */
        0x94643u, /* _OlderFileWillBeErasedText */
        0x89FBEu, /* _WhatDoYouWantText */
        0x8A07Eu, /* _WithdrewItemText */
        0x8A08Fu, /* _NothingStoredText */
        0x897E3u, /* _CantEscapeText */
        0x89EFEu, /* _TurnedOnPC1Text */
    },
    {
        0x4E15Fu, /* ProfOakPic */
        0x12EDEu, /* RedPicFront */
        0x4E049u, /* Rival1Pic */
        0x12FE8u, /* ShrinkPic1 */
        0x13042u, /* ShrinkPic2 */
        0x33E0Au, /* RedPicBack */
        0x14180u, /* RedSprite */
        0x679Eu, /* LowerCaseAlphabet */
        0x67D6u, /* UpperCaseAlphabet */
        0x6767u, /* ED_Tile */
        0x6AA8u, /* DefaultNamesPlayer */
        0x6AC0u, /* DefaultNamesRival */
        0x6AF2u, /* DefaultNamesPlayerList */
        0x6B0Au, /* DefaultNamesRivalList */
        0x79B02u, /* DownscaledMonTiles_5x5 */
        0x79B1Bu, /* DownscaledMonTiles_3x3 */
        0x1C21Eu, /* MonsterNames */
        0xB0000u, /* MoveNames */
        0x383DEu, /* BaseStats */
        0x41024u, /* PokedexOrder */
        0x38000u, /* Moves */
        0x11EA0u, /* HpBarAndStatusGraphics */
        0x12080u, /* BattleHudTiles1 */
        0x12098u, /* BattleHudTiles2 */
        0x120B0u, /* BattleHudTiles3 */
        0x3A97Eu, /* PokeballTileGraphics */
        0x8A425u, /* _OakSpeechText1 */
        0x8A47Fu, /* _OakSpeechText2A */
        0x8A4B3u, /* _OakSpeechText2B */
        0x8A519u, /* _IntroducePlayerText */
        0x8A534u, /* _IntroduceRivalText */
        0x8A597u, /* _OakSpeechText3 */
        0x8A62Fu, /* _YourNameIsText */
        0x8A64Au, /* _HisNameIsText */
        0x8A605u, /* _DoYouWantToNicknameText */
        0x897B4u, /* _TrainerSentOutText */
        0x89C5Eu, /* _TrainerWantsToFightText */
        0x89CBCu, /* _GoText */
        0x89CF0u, /* _PlayerMon1Text */
        0x9460Eu, /* _WouldYouLikeToSaveText */
        0x94630u, /* _GameSavedText */
        0x94643u, /* _OlderFileWillBeErasedText */
        0x89FBEu, /* _WhatDoYouWantText */
        0x8A07Eu, /* _WithdrewItemText */
        0x8A08Fu, /* _NothingStoredText */
        0x897E3u, /* _CantEscapeText */
        0x89EFEu, /* _TurnedOnPC1Text */
    },
};
#endif
