/* Original Silph Co. callback door coordinates and event bits. */
struct WorldDoorRule { unsigned char map, x, y, closed; unsigned short event; };
static const WorldDoorRule WORLD_SILPH_DOORS[] = {
    {0xcf,2,2,0x54,WE_EVENT_SILPH_CO_2_UNLOCKED_DOOR1},
    {0xcf,2,5,0x54,WE_EVENT_SILPH_CO_2_UNLOCKED_DOOR2},
    {0xd0,4,4,0x5f,WE_EVENT_SILPH_CO_3_UNLOCKED_DOOR1},
    {0xd0,8,4,0x5f,WE_EVENT_SILPH_CO_3_UNLOCKED_DOOR2},
    {0xd1,2,6,0x54,WE_EVENT_SILPH_CO_4_UNLOCKED_DOOR1},
    {0xd1,6,4,0x54,WE_EVENT_SILPH_CO_4_UNLOCKED_DOOR2},
    {0xd2,3,2,0x5f,WE_EVENT_SILPH_CO_5_UNLOCKED_DOOR1},
    {0xd2,3,6,0x5f,WE_EVENT_SILPH_CO_5_UNLOCKED_DOOR2},
    {0xd2,7,5,0x5f,WE_EVENT_SILPH_CO_5_UNLOCKED_DOOR3},
    {0xd3,2,6,0x5f,WE_EVENT_SILPH_CO_6_UNLOCKED_DOOR},
    {0xd4,5,3,0x54,WE_EVENT_SILPH_CO_7_UNLOCKED_DOOR1},
    {0xd4,10,2,0x54,WE_EVENT_SILPH_CO_7_UNLOCKED_DOOR2},
    {0xd4,10,6,0x54,WE_EVENT_SILPH_CO_7_UNLOCKED_DOOR3},
    {0xd5,3,4,0x5f,WE_EVENT_SILPH_CO_8_UNLOCKED_DOOR},
    {0xe9,1,4,0x5f,WE_EVENT_SILPH_CO_9_UNLOCKED_DOOR1},
    {0xe9,9,2,0x54,WE_EVENT_SILPH_CO_9_UNLOCKED_DOOR2},
    {0xe9,9,5,0x54,WE_EVENT_SILPH_CO_9_UNLOCKED_DOOR3},
    {0xe9,5,6,0x5f,WE_EVENT_SILPH_CO_9_UNLOCKED_DOOR4},
    {0xea,5,4,0x54,WE_EVENT_SILPH_CO_10_UNLOCKED_DOOR},
    {0xeb,3,6,0x20,WE_EVENT_SILPH_CO_11_UNLOCKED_DOOR},
};
struct WorldElevatorRule { unsigned char map, count; unsigned floors[2], warps[2]; };
static const WorldElevatorRule WORLD_ELEVATORS[] = {
    {0x7f,5,{0x48644u,0x48644u},{0x4864au,0x4864au}},
    {0xcb,3,{0x45755u,0x45755u},{0x45759u,0x45759u}},
    {0xec,11,{0x45805u,0x45805u},{0x45811u,0x45811u}},
};
