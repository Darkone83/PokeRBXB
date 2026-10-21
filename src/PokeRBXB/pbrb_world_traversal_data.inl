/* Original arrow movement and dungeon-warp data addresses. */
struct WorldArrowTable { int map; unsigned table[2]; };
static const WorldArrowTable WORLD_ARROW_TABLES[] = {
    {0xc8,{0x44e6du,0x44e6du}},
    {0xc9,{0x4526bu,0x4526bu}},
    {0x2d,{0x74916u,0x74917u}},
};
static const unsigned WORLD_DUNGEONWARPLIST[2] = { 0x63bfu,0x63bfu };
static const unsigned WORLD_DUNGEONWARPDATA[2] = { 0x63d8u,0x63d8u };
static const unsigned WORLD_SPINNERARROWANIMTILES[2] = { 0x45087u,0x45087u };
static const unsigned WORLD_CINNABARGYMGATECOORDS[2] = { 0x1eb48u,0x1eb48u };
static const int WORLD_CINNABAR_TRAINER_TEXT[7][3] = {
    {WX_CinnabarGymSuperNerd1BattleText,WX_CinnabarGymSuperNerd1EndBattleText,WX_CinnabarGymSuperNerd1AfterBattleText},
    {WX_CinnabarGymSuperNerd2BattleText,WX_CinnabarGymSuperNerd2EndBattleText,WX_CinnabarGymSuperNerd2AfterBattleText},
    {WX_CinnabarGymSuperNerd3BattleText,WX_CinnabarGymSuperNerd3EndBattleText,WX_CinnabarGymSuperNerd3AfterBattleText},
    {WX_CinnabarGymSuperNerd4BattleText,WX_CinnabarGymSuperNerd4EndBattleText,WX_CinnabarGymSuperNerd4AfterBattleText},
    {WX_CinnabarGymSuperNerd5BattleText,WX_CinnabarGymSuperNerd5EndBattleText,WX_CinnabarGymSuperNerd5AfterBattleText},
    {WX_CinnabarGymSuperNerd6BattleText,WX_CinnabarGymSuperNerd6EndBattleText,WX_CinnabarGymSuperNerd6AfterBattleText},
    {WX_CinnabarGymSuperNerd7BattleText,WX_CinnabarGymSuperNerd7EndBattleText,WX_CinnabarGymSuperNerd7AfterBattleText},
};
