/* Native translation of the opening map/script path in pret/pokered.
   Map headers, object events, collision lists, sprites and text are read from
   the verified Red/Blue ROM. Positions below are original script coordinates,
   not substitute maps. The private symbol tables below are generated from
   independently rebuilt pokered/pokeblue .sym files. */
#include "pbrb_world.h"

   /* Keep the private map/text tables in this translation unit so every build
      compiles their declarations with their users. Only ROM addresses and
      fingerprints are embedded here, never game media or text. */
enum WorldTextId {
    WT_PalletTownOakHeyWaitDontGoOutText,
    WT_PalletTownOakItsUnsafeText,
    WT_PalletTownGirlText,
    WT_PalletTownFisherText,
    WT_PalletTownOaksLabSignText,
    WT_PalletTownSignText,
    WT_PalletTownPlayersHouseSignText,
    WT_PalletTownRivalsHouseSignText,
    WT_RedsHouse1FMomWakeUpText,
    WT_RedsHouse1FMomYouShouldRestText,
    WT_RedsHouse1FMomLookingGreatText,
    WT_RedsHouse1FTVStandByMeMovieText,
    WT_RedsHouse1FTVWrongSideText,
    WT_BluesHouseDaisyRivalAtLabText,
    WT_BluesHouseDaisyOfferMapText,
    WT_BluesHouseDaisyBagFullText,
    WT_BluesHouseDaisyUseMapText,
    WT_BluesHouseDaisyWalkingText,
    WT_BluesHouseTownMapText,
    WT_OaksLabRivalGrampsIsntAroundText,
    WT_OaksLabRivalGoAheadAndChooseText,
    WT_OaksLabRivalMyPokemonLooksStrongerText,
    WT_OaksLabThoseArePokeBallsText,
    WT_OaksLabYouWantCharmanderText,
    WT_OaksLabYouWantSquirtleText,
    WT_OaksLabYouWantBulbasaurText,
    WT_OaksLabMonEnergeticText,
    WT_OaksLabReceivedMonText,
    WT_OaksLabLastMonText,
    WT_OaksLabOak1WhichPokemonDoYouWantText,
    WT_OaksLabOak1YourPokemonCanFightText,
    WT_OaksLabOak1RaiseYourYoungPokemonText,
    WT_OaksLabOak1DeliverParcelText,
    WT_OaksLabOak1ParcelThanksText,
    WT_OaksLabOak1PokemonAroundTheWorldText,
    WT_OaksLabOak1ReceivedPokeballsText,
    WT_OaksLabGivePokeballsExplanationText,
    WT_OaksLabOak1ComeSeeMeSometimesText,
    WT_OaksLabOak1HowIsYourPokedexComingText,
    WT_OaksLabPokedexText,
    WT_OaksLabOak2Text,
    WT_OaksLabGirlText,
    WT_OaksLabRivalFedUpWithWaitingText,
    WT_OaksLabOakChooseMonText,
    WT_OaksLabRivalWhatAboutMeText,
    WT_OaksLabOakBePatientText,
    WT_OaksLabOakDontGoAwayYetText,
    WT_OaksLabRivalIllTakeThisOneText,
    WT_OaksLabRivalReceivedMonText,
    WT_OaksLabRivalIllTakeYouOnText,
    WT_OaksLabRivalIPickedTheWrongPokemonText,
    WT_OaksLabRivalAmIGreatOrWhatText,
    WT_OaksLabRivalSmellYouLaterText,
    WT_OaksLabRivalGrampsText,
    WT_OaksLabRivalWhatDidYouCallMeForText,
    WT_OaksLabOakIHaveARequestText,
    WT_OaksLabOakMyInventionPokedexText,
    WT_OaksLabOakGotPokedexText,
    WT_OaksLabOakThatWasMyDreamText,
    WT_OaksLabRivalLeaveItAllToMeText,
    WT_OaksLabScientistText,
    WT_RedBedroomSNESText,
    WT_PokemonBooksText,
    WT_TownMapText,
    WT_COUNT
};
static const unsigned WORLD_TEXT[2][WT_COUNT] = {
    {
        0xa4245u, /* _PalletTownOakHeyWaitDontGoOutText */
        0xa4264u, /* _PalletTownOakItsUnsafeText */
        0xa42dcu, /* _PalletTownGirlText */
        0xa431du, /* _PalletTownFisherText */
        0xa4374u, /* _PalletTownOaksLabSignText */
        0xa438bu, /* _PalletTownSignText */
        0xa43b6u, /* _PalletTownPlayersHouseSignText */
        0xa43c1u, /* _PalletTownRivalsHouseSignText */
        0x94b07u, /* _RedsHouse1FMomWakeUpText */
        0x94b6du, /* _RedsHouse1FMomYouShouldRestText */
        0x94b94u, /* _RedsHouse1FMomLookingGreatText */
        0x94bd7u, /* _RedsHouse1FTVStandByMeMovieText */
        0x94c29u, /* _RedsHouse1FTVWrongSideText */
        0x94c3cu, /* _BluesHouseDaisyRivalAtLabText */
        0x94c5du, /* _BluesHouseDaisyOfferMapText */
        0x94cadu, /* _BluesHouseDaisyBagFullText */
        0x94cd0u, /* _BluesHouseDaisyUseMapText */
        0x94cfdu, /* _BluesHouseDaisyWalkingText */
        0x94d3bu, /* _BluesHouseTownMapText */
        0x94d5bu, /* _OaksLabRivalGrampsIsntAroundText */
        0x94d79u, /* _OaksLabRivalGoAheadAndChooseText */
        0x94dbdu, /* _OaksLabRivalMyPokemonLooksStrongerText */
        0x94ddfu, /* _OaksLabThoseArePokeBallsText */
        0x94e06u, /* _OaksLabYouWantCharmanderText */
        0x94e2fu, /* _OaksLabYouWantSquirtleText */
        0x94e57u, /* _OaksLabYouWantBulbasaurText */
        0x94e80u, /* _OaksLabMonEnergeticText */
        0x94ea0u, /* _OaksLabReceivedMonText */
        0x94eb6u, /* _OaksLabLastMonText */
        0x94ed2u, /* _OaksLabOak1WhichPokemonDoYouWantText */
        0x94ef8u, /* _OaksLabOak1YourPokemonCanFightText */
        0x94f36u, /* _OaksLabOak1RaiseYourYoungPokemonText */
        0x94f69u, /* _OaksLabOak1DeliverParcelText */
        0x9500fu, /* _OaksLabOak1ParcelThanksText */
        0x95045u, /* _OaksLabOak1PokemonAroundTheWorldText */
        0x9506du, /* _OaksLabOak1ReceivedPokeballsText */
        0x950f2u, /* _OaksLabGivePokeballsExplanationText */
        0x9519eu, /* _OaksLabOak1ComeSeeMeSometimesText */
        0x951e9u, /* _OaksLabOak1HowIsYourPokedexComingText */
        0x95236u, /* _OaksLabPokedexText */
        0x95268u, /* _OaksLabOak2Text */
        0x9526bu, /* _OaksLabGirlText */
        0x952bbu, /* _OaksLabRivalFedUpWithWaitingText */
        0x952dfu, /* _OaksLabOakChooseMonText */
        0x953dcu, /* _OaksLabRivalWhatAboutMeText */
        0x953fcu, /* _OaksLabOakBePatientText */
        0x95427u, /* _OaksLabOakDontGoAwayYetText */
        0x95444u, /* _OaksLabRivalIllTakeThisOneText */
        0x95461u, /* _OaksLabRivalReceivedMonText */
        0x95477u, /* _OaksLabRivalIllTakeYouOnText */
        0x954b6u, /* _OaksLabRivalIPickedTheWrongPokemonText */
        0x954e4u, /* _OaksLabRivalAmIGreatOrWhatText */
        0x95502u, /* _OaksLabRivalSmellYouLaterText */
        0x95551u, /* _OaksLabRivalGrampsText */
        0x9555du, /* _OaksLabRivalWhatDidYouCallMeForText */
        0x9557bu, /* _OaksLabOakIHaveARequestText */
        0x955a8u, /* _OaksLabOakMyInventionPokedexText */
        0x9562au, /* _OaksLabOakGotPokedexText */
        0x95664u, /* _OaksLabOakThatWasMyDreamText */
        0x95741u, /* _OaksLabRivalLeaveItAllToMeText */
        0x957ebu, /* _OaksLabScientistText */
        0x88f27u, /* _RedBedroomSNESText */
        0x895c1u, /* _PokemonBooksText */
        0x89611u, /* _TownMapText */
    },
    {
        0xa4245u, /* _PalletTownOakHeyWaitDontGoOutText */
        0xa4264u, /* _PalletTownOakItsUnsafeText */
        0xa42dcu, /* _PalletTownGirlText */
        0xa431du, /* _PalletTownFisherText */
        0xa4374u, /* _PalletTownOaksLabSignText */
        0xa438bu, /* _PalletTownSignText */
        0xa43b6u, /* _PalletTownPlayersHouseSignText */
        0xa43c1u, /* _PalletTownRivalsHouseSignText */
        0x94b07u, /* _RedsHouse1FMomWakeUpText */
        0x94b6du, /* _RedsHouse1FMomYouShouldRestText */
        0x94b94u, /* _RedsHouse1FMomLookingGreatText */
        0x94bd7u, /* _RedsHouse1FTVStandByMeMovieText */
        0x94c29u, /* _RedsHouse1FTVWrongSideText */
        0x94c3cu, /* _BluesHouseDaisyRivalAtLabText */
        0x94c5du, /* _BluesHouseDaisyOfferMapText */
        0x94cadu, /* _BluesHouseDaisyBagFullText */
        0x94cd0u, /* _BluesHouseDaisyUseMapText */
        0x94cfdu, /* _BluesHouseDaisyWalkingText */
        0x94d3bu, /* _BluesHouseTownMapText */
        0x94d5bu, /* _OaksLabRivalGrampsIsntAroundText */
        0x94d79u, /* _OaksLabRivalGoAheadAndChooseText */
        0x94dbdu, /* _OaksLabRivalMyPokemonLooksStrongerText */
        0x94ddfu, /* _OaksLabThoseArePokeBallsText */
        0x94e06u, /* _OaksLabYouWantCharmanderText */
        0x94e2fu, /* _OaksLabYouWantSquirtleText */
        0x94e57u, /* _OaksLabYouWantBulbasaurText */
        0x94e80u, /* _OaksLabMonEnergeticText */
        0x94ea0u, /* _OaksLabReceivedMonText */
        0x94eb6u, /* _OaksLabLastMonText */
        0x94ed2u, /* _OaksLabOak1WhichPokemonDoYouWantText */
        0x94ef8u, /* _OaksLabOak1YourPokemonCanFightText */
        0x94f36u, /* _OaksLabOak1RaiseYourYoungPokemonText */
        0x94f69u, /* _OaksLabOak1DeliverParcelText */
        0x9500fu, /* _OaksLabOak1ParcelThanksText */
        0x95045u, /* _OaksLabOak1PokemonAroundTheWorldText */
        0x9506du, /* _OaksLabOak1ReceivedPokeballsText */
        0x950f2u, /* _OaksLabGivePokeballsExplanationText */
        0x9519eu, /* _OaksLabOak1ComeSeeMeSometimesText */
        0x951e9u, /* _OaksLabOak1HowIsYourPokedexComingText */
        0x95236u, /* _OaksLabPokedexText */
        0x95268u, /* _OaksLabOak2Text */
        0x9526bu, /* _OaksLabGirlText */
        0x952bbu, /* _OaksLabRivalFedUpWithWaitingText */
        0x952dfu, /* _OaksLabOakChooseMonText */
        0x953dcu, /* _OaksLabRivalWhatAboutMeText */
        0x953fcu, /* _OaksLabOakBePatientText */
        0x95427u, /* _OaksLabOakDontGoAwayYetText */
        0x95444u, /* _OaksLabRivalIllTakeThisOneText */
        0x95461u, /* _OaksLabRivalReceivedMonText */
        0x95477u, /* _OaksLabRivalIllTakeYouOnText */
        0x954b6u, /* _OaksLabRivalIPickedTheWrongPokemonText */
        0x954e4u, /* _OaksLabRivalAmIGreatOrWhatText */
        0x95502u, /* _OaksLabRivalSmellYouLaterText */
        0x95551u, /* _OaksLabRivalGrampsText */
        0x9555du, /* _OaksLabRivalWhatDidYouCallMeForText */
        0x9557bu, /* _OaksLabOakIHaveARequestText */
        0x955a8u, /* _OaksLabOakMyInventionPokedexText */
        0x9562au, /* _OaksLabOakGotPokedexText */
        0x95664u, /* _OaksLabOakThatWasMyDreamText */
        0x95741u, /* _OaksLabRivalLeaveItAllToMeText */
        0x957ebu, /* _OaksLabScientistText */
        0x88f27u, /* _RedBedroomSNESText */
        0x895c1u, /* _PokemonBooksText */
        0x89611u, /* _TownMapText */
    },
};
struct WorldMapSpec { unsigned header, blocks, objects, crc; int startX, startY; const char* name; };
static const WorldMapSpec WORLD_MAPS[2][7] = {
    {
        {0x5c0a4u,0x5c010u,0x5c0d0u,0x1821b0edu,3,6,"RedsHouse2F"},
        {0x182a1u,0x182fdu,0x182c3u,0x735e33f9u,5,6,"PalletTown"},
        {0x4815cu,0x48209u,0x481e4u,0x8b285648u,7,1,"RedsHouse1F"},
        {0x1cb02u,0x1c1c0u,0x1d40au,0x7c29cefau,5,11,"OaksLab"},
        {0x19b2fu,0x19bf6u,0x19bceu,0xae15e1f9u,2,7,"BluesHouse"},
        {0x1c0c3u,0x1c0fcu,0x1c0e5u,0x49f37244u,10,35,"Route1"},
        {0x54fffu,0x5506du,0x55021u,0xa925b606u,10,0,"Route21"},
    },
    {
        {0x5c0a4u,0x5c010u,0x5c0d0u,0x1821b0edu,3,6,"RedsHouse2F"},
        {0x182a1u,0x182fdu,0x182c3u,0x735e33f9u,5,6,"PalletTown"},
        {0x4815cu,0x48209u,0x481e4u,0x8b285648u,7,1,"RedsHouse1F"},
        {0x1cb02u,0x1c1c0u,0x1d40au,0x7c29cefau,5,11,"OaksLab"},
        {0x19b2fu,0x19bf6u,0x19bceu,0xae15e1f9u,2,7,"BluesHouse"},
        {0x1c0c3u,0x1c0fcu,0x1c0e5u,0x49f37244u,10,35,"Route1"},
        {0x54fffu,0x5506du,0x55021u,0xa925b606u,10,0,"Route21"},
    },
};
static const unsigned WORLD_TILESETS[2] = { 0xc7beu,0xc7beu };
static const unsigned WORLD_SPRITES[2] = { 0x17b27u,0x17b27u };
static const unsigned WORLD_FLOWER[2] = { 0x1f19u,0x1f19u };
static const unsigned WORLD_SHOCK[2] = { 0x17cbdu,0x17cbdu };

#include "pbrb_gb.h"
#include "pbrb_audio_core.h"
#include <string.h>
#include "pbrb_world_catalog.inl"

enum {
    WF_OAK_APPEARED = 1, WF_FOLLOWED_OAK = 2, WF_CHOSE_STARTER = 4,
    WF_RIVAL_CHOSE = 8, WF_BATTLED = 16, WF_END_REPORTED = 32
};
enum {
    WS_IDLE = 0, WS_OAK_HEY, WS_OAK_BUBBLE_START, WS_OAK_BUBBLE,
    WS_OAK_APPROACH_START, WS_OAK_APPROACH, WS_OAK_UNSAFE,
    WS_OAK_ALIGN, WS_PLAYER_ALIGN, WS_ESCORT_START, WS_ESCORT,
    WS_LAB_OAK_START, WS_LAB_OAK_WALK, WS_LAB_PLAYER_START,
    WS_LAB_PLAYER_WALK, WS_LAB_MUSIC, WS_LAB_SPEECH1,
    WS_LAB_SPEECH2, WS_LAB_SPEECH3, WS_LAB_SPEECH4,
    WS_FORCE_BACK, WS_FORCE_BACK_WAIT, WS_STARTER_CHOICE,
    WS_STARTER_ENERGETIC, WS_STARTER_RECEIVED, WS_STARTER_SOUND,
    WS_STARTER_GIVE, WS_RIVAL_PICK_START, WS_RIVAL_PICK_WALK,
    WS_RIVAL_PICK_TEXT, WS_RIVAL_RECEIVED, WS_RIVAL_SOUND,
    WS_CHALLENGE_TEXT, WS_CHALLENGE_APPROACH_START,
    WS_CHALLENGE_APPROACH, WS_BATTLE_WAIT, WS_AFTER_HEAL,
    WS_AFTER_DELAY, WS_AFTER_TEXT, WS_RIVAL_EXIT_START,
    WS_RIVAL_EXIT, WS_AFTER_MUSIC, WS_END, WS_MOM_HEAL,
    WS_MOM_GOOD, WS_NATIVE_SCRIPT
};
static const int DX[4] = { 0,0,-1,1 }, DY[4] = { 1,-1,0,0 };
static const unsigned STARTERS[3] = { 0xb0u,0xb1u,0x99u };
static const unsigned LEGACY_MAPS[5] = { 0x26u,0u,0x25u,0x28u,0x27u };
int PbrbWorldMapFromRom(unsigned raw) {
    if (raw > 255u)return -1;
    for (int i = 0; i < 5; ++i)if (LEGACY_MAPS[i] == raw)return i;
    return PBRB_WORLD_MAP_BASE + (int)raw;
}
int PbrbWorldMapToRom(int map) {
    if (map >= 0 && map < 5)return (int)LEGACY_MAPS[map];
    return map >= PBRB_WORLD_MAP_BASE && map < PBRB_WORLD_MAP_BASE + 256 ? map - PBRB_WORLD_MAP_BASE : -1;
}
void PbrbWorldSetPlayerState(PbrbWorld* w, const PbrbWorldPlayerState* player) {
    if (w && player)w->player = *player;
}
int PbrbWorldCheckEvent(const PbrbWorld* w, unsigned bit) {
    return w && bit < PBRB_WORLD_EVENT_BYTES * 8u && (w->eventFlags[bit / 8u] & (1u << (bit & 7u))) != 0;
}
void PbrbWorldSetEvent(PbrbWorld* w, unsigned bit, int enabled) {
    if (!w || bit >= PBRB_WORLD_EVENT_BYTES * 8u)return;
    if (enabled)w->eventFlags[bit / 8u] |= (unsigned char)(1u << (bit & 7u));
    else w->eventFlags[bit / 8u] &= (unsigned char)~(1u << (bit & 7u));
}
static unsigned Crc32(const unsigned char* p, unsigned n) {
    unsigned c = 0xffffffffu;
    for (unsigned i = 0; i < n; ++i) { c ^= p[i]; for (int j = 0; j < 8; ++j)c = (c >> 1) ^ (0xedb88320u & (0u - (c & 1u))); }
    return c ^ 0xffffffffu;
}
static int InRom(const PbrbWorld* w, unsigned p, unsigned n) {
    return p <= w->romSize && n <= w->romSize - p;
}
static unsigned Read16(const unsigned char* p) { return p[0] | ((unsigned)p[1] << 8); }
static unsigned Address(unsigned bank, unsigned ptr) { return ptr < 0x4000u ? ptr : bank * 0x4000u + (ptr & 0x3fffu); }
static int Abs(int n) { return n < 0 ? -n : n; }
static int FloorDiv(int n, int d) { return n >= 0 ? n / d : -((-n + d - 1) / d); }
static int GameIndex(const PbrbWorld* w) { return w->game == 2 ? 1 : 0; }
static const WorldMapSpec* Spec(const PbrbWorld* w, int m) { return &WORLD_MAPS[GameIndex(w)][m]; }
static unsigned Text(const PbrbWorld* w, int id) { return WORLD_TEXT[GameIndex(w)][id]; }
static void Emit(PbrbWorld* w, int kind, unsigned text, int value, unsigned flags = 0) {
    memset(&w->event, 0, sizeof(w->event)); w->event.objectId = -1;
    w->event.kind = kind; w->event.textOffset = text; w->event.value = value; w->event.flags = flags;
}
static void Say(PbrbWorld* w, int text, int next, int species, unsigned flags = 0) {
    w->stage = next; Emit(w, PBRB_WORLD_TEXT, Text(w, text), species, flags);
}
static unsigned RandomByte(PbrbWorld* w) {
    /* Persisted deterministic native RNG. GB DIV depends on instruction timing;
       scenery movement uses this stream while authored scripts never do. */
    w->rng = w->rng * 1664525u + 1013904223u;
    return w->rng >> 24;
}
static int HeroPixels(const PbrbWorld* w) {
    return w->heroStepLength ? (w->heroStepFrame / 2) * 32 / w->heroStepLength : 0;
}
static int ActorPixels(const PbrbWorldActor* a) {
    return a->stepLength ? (a->stepFrame / 2) * (a->stepLength == 16 ? 2 : 1) : 0;
}
static void Camera(PbrbWorld* w) {
    int p = HeroPixels(w);
    w->cameraX = w->heroX * 16 + w->heroDx * p - 64;
    w->cameraY = w->heroY * 16 + w->heroDy * p - 64;
}
struct WorldRawMap {
    int raw, bank, width, height, tileset;
    unsigned header, blocks, objects, texts, script, connectionOffset, connections;
};
static int RawMap(const PbrbWorld* w, int raw, WorldRawMap* m) {
    /* MapHeaderPointers and MapHeaderBanks, original ROM tables. */
    if (!w || !m || raw < 0 || raw >= 248 || !WORLD_CATALOG_MAPS[raw].valid)return 0;
    const unsigned banks = 0xc23du, pointers = 0x1aeu;
    if (!InRom(w, banks + (unsigned)raw, 1) || !InRom(w, pointers + (unsigned)raw * 2u, 2))return 0;
    unsigned bank = w->rom[banks + raw], header = Address(bank, Read16(w->rom + pointers + raw * 2u));
    if (!InRom(w, header, 12))return 0;
    const unsigned char* h = w->rom + header;
    if (!h[1] || !h[2] || h[0] > 23 || (h[9] & 0xf0u))return 0;
    unsigned p = header + 10;
    for (unsigned bit = 8; bit; bit >>= 1)if (h[9] & bit)p += 11;
    if (!InRom(w, p, 2))return 0;
    m->raw = raw; m->bank = (int)bank; m->width = h[2]; m->height = h[1]; m->tileset = h[0]; m->header = header;
    m->blocks = Address(bank, Read16(h + 3)); m->texts = Address(bank, Read16(h + 5));
    m->script = Address(bank, Read16(h + 7)); m->objects = Address(bank, Read16(w->rom + p));
    m->connectionOffset = header + 10; m->connections = h[9];
    return InRom(w, m->blocks, (unsigned)m->width * m->height) && InRom(w, m->objects, 2);
}
static unsigned BlockAt(const PbrbWorld* w, int bx, int by) {
    for (int pi = w->blockChangeCount - 1; pi >= 0; --pi)
        if (w->blockChanges[pi].x == bx && w->blockChanges[pi].y == by)return w->blockChanges[pi].block;
    if (bx >= 0 && by >= 0 && bx < w->widthBlocks && by < w->heightBlocks)
        return w->rom[w->mapOffset + by * w->widthBlocks + bx];
    unsigned block = (unsigned)w->borderBlock;
    /* The original map loader copies three-block connection strips in this
       order. Offsets are derived from each ROM connection record. */
    static const int order[4] = { 1,0,2,3 };
    for (int i = 0; i < 4; ++i) {
        int d = order[i]; const PbrbWorldConnection* c = &w->connections[d];
        if (c->map < 0)continue;
        int x = bx, y = by, connected = 0;
        if (d == 1 && by < 0 && by >= -3) { x -= c->offset; y += c->height; connected = 1; }
        if (d == 0 && by >= w->heightBlocks && by < w->heightBlocks + 3) { x -= c->offset; y -= w->heightBlocks; connected = 1; }
        if (d == 2 && bx < 0 && bx >= -3) { x += c->width; y -= c->offset; connected = 1; }
        if (d == 3 && bx >= w->widthBlocks && bx < w->widthBlocks + 3) { x -= w->widthBlocks; y -= c->offset; connected = 1; }
        if (connected && x >= 0 && y >= 0 && x < c->width && y < c->height)
            block = w->rom[c->blocks + (unsigned)y * c->width + x];
    }
    return block;
}
static unsigned TileAt(const PbrbWorld* w, int tx, int ty) {
    int bx = FloorDiv(tx, 4), by = FloorDiv(ty, 4);
    unsigned offset = w->blocksOffset + BlockAt(w, bx, by) * 16u + (ty - by * 4) * 4 + (tx - bx * 4);
    return InRom(w, offset, 1) ? w->rom[offset] : 0;
}
int PbrbWorldHasMetBill(const PbrbWorld* w) {
    return w && PbrbWorldCheckEvent(w, WE_EVENT_MET_BILL);
}
int PbrbWorldInGrass(const PbrbWorld* w) {
    return w && w->rom && (w->tileset == 0 || w->tileset == 3) && w->grassTile != 255 &&
        TileAt(w, w->heroX * 2 + 1, w->heroY * 2 + 1) == (unsigned)w->grassTile;
}
static int ConnectionTarget(const PbrbWorld* w, int x, int y, int* map, int* nx, int* ny) {
    int d = y < 0 ? 1 : y >= w->heightBlocks * 2 ? 0 : x < 0 ? 2 : x >= w->widthBlocks * 2 ? 3 : -1;
    if (d < 0)return 0;
    const PbrbWorldConnection* c = &w->connections[d]; if (c->map < 0)return 0;
    int tx = x, ty = y;
    if (d == 1) { tx -= c->offset * 2; ty += c->height * 2; }
    if (d == 0) { tx -= c->offset * 2; ty -= w->heightBlocks * 2; }
    if (d == 2) { tx += c->width * 2; ty -= c->offset * 2; }
    if (d == 3) { tx -= w->widthBlocks * 2; ty -= c->offset * 2; }
    if (tx < 0 || ty < 0 || tx >= c->width * 2 || ty >= c->height * 2)return 0;
    if (map)*map = PbrbWorldMapFromRom((unsigned)c->map); if (nx)*nx = tx; if (ny)*ny = ty; return 1;
}
static int SurfWaterTile(const PbrbWorld* w, unsigned tile) {
    /* CollisionCheckOnWater keeps Surf active on the eastern shore tiles.
       In SHIP_PORT, $32 is the boarding platform and ends Surf instead. */
    return tile == 0x14u || tile == 0x48u || (tile == 0x32u && w->tileset != 14);
}
static int TilePairCollision(const PbrbWorld* w, int fromX, int fromY, int toX, int toY, int surfing) {
    /* The original pair tables are shared by the verified Red/Blue layouts.
       They supplement the destination-only collision lists at cave/forest
       elevation boundaries; either direction through a pair is blocked. */
    unsigned from = TileAt(w, fromX * 2, fromY * 2 + 1), to = TileAt(w, toX * 2, toY * 2 + 1);
    unsigned p = surfing ? 0x0ca0u : 0x0c7eu;
    for (unsigned i = 0; i < 16 && InRom(w, p, 1) && w->rom[p] != 255u; ++i, p += 3u) {
        if (!InRom(w, p, 3))return 1;
        if (w->rom[p] == w->tileset &&
            ((from == w->rom[p + 1] && to == w->rom[p + 2]) || (from == w->rom[p + 2] && to == w->rom[p + 1])))return 1;
    }
    return 0;
}
static int TilePassable(const PbrbWorld* w, int x, int y, int surfing) {
    if ((x < 0 || y < 0 || x >= w->widthBlocks * 2 || y >= w->heightBlocks * 2) &&
        !ConnectionTarget(w, x, y, 0, 0, 0))return 0;
    unsigned tile = TileAt(w, x * 2, y * 2 + 1);
    if (surfing && SurfWaterTile(w, tile))return 1;
    for (unsigned i = 0; i < 96 && InRom(w, w->collisionOffset + i, 1); ++i) {
        unsigned t = w->rom[w->collisionOffset + i]; if (t == 255)return 0; if (t == tile)return 1;
    }
    return 0;
}
static int Passable(const PbrbWorld* w, int x, int y) {
    return TilePassable(w, x, y, w->travelMode == 2);
}
static int ActorAt(const PbrbWorld* w, int x, int y, int ignore) {
    for (int i = 0; i < w->actorCount; ++i) {
        const PbrbWorldActor* a = &w->actors[i]; if (i == ignore || !a->visible)continue;
        if (a->x == x && a->y == y)return i;
        if (a->stepLength && a->x + a->dx == x && a->y + a->dy == y)return i;
    }
    return -1;
}
static int BallForSpecies(int species) {
    for (int i = 0; i < 3; ++i)if ((int)STARTERS[i] == species)return i + 1;
    return -1;
}
static void ApplyVisibility(PbrbWorld* w) {
    if (w->mapId == PBRB_MAP_PALLET && w->actorCount >= 1)
        w->actors[0].visible = (w->flags & WF_OAK_APPEARED) && !(w->flags & WF_FOLLOWED_OAK);
    if (w->mapId == PBRB_MAP_LAB && w->actorCount >= 11) {
        w->actors[0].visible = !(w->flags & WF_BATTLED);
        w->actors[4].visible = (w->flags & WF_FOLLOWED_OAK) != 0;
        w->actors[7].visible = (w->flags & WF_OAK_APPEARED) && !(w->flags & WF_FOLLOWED_OAK);
        int ball = BallForSpecies(w->selectedStarter);
        if ((w->flags & WF_CHOSE_STARTER) && ball >= 0)w->actors[ball].visible = 0;
        ball = BallForSpecies(w->rivalStarter);
        if ((w->flags & WF_RIVAL_CHOSE) && ball >= 0)w->actors[ball].visible = 0;
        if ((w->flags & WF_RIVAL_CHOSE) && !(w->flags & WF_BATTLED)) {
            w->actors[0].x = ball + 5; w->actors[0].y = 4; w->actors[0].facing = 1;
        }
    }
    if (w->mapId == PBRB_MAP_BLUE_HOUSE && w->actorCount >= 2) {
        int walking = PbrbWorldCheckEvent(w, WE_EVENT_DAISY_WALKING);
        w->actors[0].visible = !walking; w->actors[1].visible = walking;
    }
}
static void InitializeObjects(PbrbWorld* w) {
    memset(w->hiddenObjects, 0, sizeof(w->hiddenObjects));
    unsigned p = 0xcaeau; /* ToggleableObjectStates: map, one-based object, OFF/ON. */
    for (unsigned i = 0; i < 256 && InRom(w, p, 3); ++i, p += 3) {
        unsigned raw = w->rom[p], id = w->rom[p + 1];
        if (raw == 255)break;
        if (id && id <= 16 && w->rom[p + 2] == 0x11)
            w->hiddenObjects[raw] |= (unsigned short)(1u << (id - 1u));
    }
}
static void HideObject(PbrbWorld* w, int raw, int object, int hide) {
    if (raw < 0 || raw>255 || object < 0 || object >= 16)return;
    unsigned bit = 1u << object;
    if (hide)w->hiddenObjects[raw] |= (unsigned short)bit;
    else w->hiddenObjects[raw] &= (unsigned short)~bit;
    if (w->romMapId == raw && object < w->actorCount)w->actors[object].visible = !hide;
}
static void StoryMapLoaded(PbrbWorld* w);
static int BikeAllowed(const PbrbWorld* w) {
    return w->romMapId == 0x22 || w->romMapId == 9 || w->tileset == 0 ||
        w->tileset == 3 || w->tileset == 11 || w->tileset == 14 || w->tileset == 17;
}
static int LoadMap(PbrbWorld* w, int mapId, int x, int y, int facing) {
    WorldRawMap m; if (!RawMap(w, PbrbWorldMapToRom(mapId), &m))return 0;
    unsigned ts = WORLD_TILESETS[GameIndex(w)] + (unsigned)m.tileset * 12u;
    if (!InRom(w, ts, 12))return 0;
    const unsigned char* t = w->rom + ts;
    unsigned blocks = Address(t[0], Read16(t + 1)), gfx = Address(t[0], Read16(t + 3));
    unsigned collision = Address(t[0], Read16(t + 5));
    if (!InRom(w, blocks, 256u * 16u) || !InRom(w, gfx, 96u * 16u) || !InRom(w, collision, 96u))return 0;
    if (mapId < 5) {
        const WorldMapSpec* s = Spec(w, mapId);
        if (Crc32(w->rom + m.blocks, (unsigned)m.width * m.height) != s->crc)return 0;
    }
    w->previousMapRom = w->romMapId; w->mapId = mapId; w->romMapId = m.raw; w->tileset = m.tileset; w->mapBank = m.bank;
    /* These entrance scripts explicitly reset wLastMap after an underground
       passage or Diglett's Cave is traversed from its opposite entrance. */
    if (m.raw == 0x2e)w->lastMapRom = 0x0d;
    if (m.raw == 0x55)w->lastMapRom = 0x16;
    if (m.raw == 0x47)w->lastMapRom = 0x10;
    if (m.raw == 0x4a)w->lastMapRom = 0x11;
    if (m.raw == 0x4d || m.raw == 0x4e)w->lastMapRom = 0x12;
    if (m.raw == 0x50)w->lastMapRom = 0x13;
    w->headerOffset = m.header; w->objectOffset = m.objects; w->textPointersOffset = m.texts; w->scriptOffset = m.script;
    w->widthBlocks = m.width; w->heightBlocks = m.height;
    w->mapOffset = m.blocks; w->tilesOffset = gfx; w->blocksOffset = blocks;
    w->collisionOffset = collision; w->numTiles = 96; w->counterTile = t[7]; w->grassTile = t[10];
    w->borderBlock = w->rom[m.objects];
    w->blockChangeCount = 0;
    if (m.raw < 11)w->visitedTowns |= 1u << m.raw;
    if (!w->restoring) {
        w->fieldActive &= ~PBRB_WORLD_FIELD_STRENGTH;
        if (m.raw < 0x25 || (m.raw == 0x52 && w->previousMapRom < 0x25))w->fieldActive &= ~PBRB_WORLD_FIELD_FLASH;
        if (!BikeAllowed(w) && w->travelMode == 1)w->travelMode = 0;
    }
    unsigned cp = m.connectionOffset;
    static const int dirs[4] = { 1,0,2,3 };
    for (int ci = 0; ci < 4; ++ci) {
        int d = dirs[ci]; PbrbWorldConnection* c = &w->connections[d];
        memset(c, 0, sizeof(*c)); c->map = -1;
        if (m.connections & (8u >> ci)) {
            if (!InRom(w, cp, 11))return 0;
            WorldRawMap next;
            if (RawMap(w, w->rom[cp], &next)) {
                int offset = (int)(signed char)w->rom[cp + (d < 2 ? 8 : 7)];
                c->map = next.raw; c->offset = -offset / 2; c->width = next.width; c->height = next.height; c->blocks = next.blocks;
                const WorldCatalogMap* cm = &WORLD_CATALOG_MAPS[m.raw];
                for (unsigned k = 0; k < cm->connectionCount; ++k) {
                    const WorldCatalogConnection* cc = &WORLD_CATALOG_CONNECTIONS[cm->connectionFirst + k];
                    if (cc->direction == d && (cc->destination != next.raw || cc->offset != c->offset))return 0;
                }
            }
            cp += 11;
        }
    }
    w->heroX = x; w->heroY = y; w->facing = facing; w->heroDx = w->heroDy = 0;
    w->heroStepFrame = w->heroStepLength = w->heroAnim = 0; w->turnDelay = 0; w->wasMoving = 0;
    w->heroPathCount = w->heroPathIndex = w->actorPathCount = w->actorPathIndex = 0;
    w->pathActor = -1; w->actorCount = 0; w->interactionActor = -1; w->interactionFacing = 0; w->warpLock = 1;
    memset(w->actors, 0, sizeof(w->actors));
    unsigned p = m.objects + 1;
    if (!InRom(w, p, 1) || w->rom[p] > PBRB_WORLD_WARPS)return 0;
    w->warpCount = w->rom[p++];
    for (int wi = 0; wi < w->warpCount; ++wi) {
        if (!InRom(w, p, 4))return 0;
        PbrbWorldWarp* a = &w->warps[wi]; a->y = w->rom[p]; a->x = w->rom[p + 1]; a->warp = w->rom[p + 2]; a->destination = w->rom[p + 3]; p += 4;
    }
    if (!InRom(w, p, 1) || w->rom[p] > PBRB_WORLD_SIGNS)return 0;
    w->signCount = w->rom[p++];
    for (int si = 0; si < w->signCount; ++si) {
        if (!InRom(w, p, 3))return 0;
        PbrbWorldSign* a = &w->signs[si]; a->y = w->rom[p]; a->x = w->rom[p + 1]; a->textId = w->rom[p + 2]; p += 3;
    }
    if (!InRom(w, p, 1) || w->rom[p] > PBRB_WORLD_ACTORS)return 0;
    w->actorCount = w->rom[p++];
    for (int i = 0; i < w->actorCount; ++i) {
        if (!InRom(w, p, 6))return 0;
        PbrbWorldActor* a = &w->actors[i];
        a->sprite = w->rom[p]; a->y = a->homeY = (int)w->rom[p + 1] - 4;
        a->x = a->homeX = (int)w->rom[p + 2] - 4; a->movement = w->rom[p + 3]; a->range = w->rom[p + 4];
        a->facing = (a->range >= 0xd0 && a->range <= 0xd3) ? a->range - 0xd0 : 0;
        a->visible = (w->hiddenObjects[m.raw] & (1u << i)) == 0; a->wait = 2;
        unsigned text = w->rom[p + 5]; a->textId = text & 63u; a->objectFlags = text & 0xc0u; p += 6;
        if (text & 0x40) { if (!InRom(w, p, 2))return 0; a->trainerClass = w->rom[p++]; a->trainerId = w->rom[p++]; }
        else if (text & 0x80) { if (!InRom(w, p, 1))return 0; a->itemId = w->rom[p++]; }
    }
    ApplyVisibility(w); StoryMapLoaded(w); Camera(w); return 1;
}
int PbrbWorldMusic(const PbrbWorld* w) {
    if (!w)return PBRB_MUSIC_NONE;
    if ((w->flags & WF_OAK_APPEARED) && !(w->flags & WF_FOLLOWED_OAK))return PBRB_MUSIC_OAK;
    if (w->travelMode == 1)return PBRB_MUSIC_BIKE;
    if (w->travelMode == 2)return PBRB_MUSIC_SURFING;
    if (w->mapId < 5)return w->mapId == PBRB_MAP_LAB ? PBRB_MUSIC_LAB : PBRB_MUSIC_PALLET;
    unsigned p = 0xc04du + (unsigned)w->romMapId * 2u;
    if (!InRom(w, p, 2))return PBRB_MUSIC_NONE;
    return PBRB_WORLD_MUSIC_ROM_FLAG | ((int)w->rom[p + 1] << 8) | w->rom[p];
}
int PbrbWorldSelectMap(PbrbWorld* w, int mapId) {
    if (!w || !w->rom || PbrbWorldMapToRom(mapId) < 0)return 0;
    int x = 0, y = 0;
    if (mapId < 5) { const WorldMapSpec* s = Spec(w, mapId); x = s->startX; y = s->startY; }
    else {
        WorldRawMap m; if (!RawMap(w, PbrbWorldMapToRom(mapId), &m))return 0;
        if (w->rom[m.objects + 1]) { y = w->rom[m.objects + 2]; x = w->rom[m.objects + 3]; }
    }
    w->stage = WS_IDLE; memset(&w->event, 0, sizeof(w->event)); w->transitionFrame = 0;
    memset(&w->fieldAnimation, 0, sizeof(w->fieldAnimation));
    return LoadMap(w, mapId, x, y, mapId == PBRB_MAP_BEDROOM ? 1 : 0);
}
int PbrbWorldInit(PbrbWorld* w, const unsigned char* rom, size_t bytes, int mapId) {
    if (!w || !rom || bytes != PBRB_ROM_SIZE)return 0;
    int game = !memcmp(rom + 0x134, "POKEMON RED", 11) ? 1 : (!memcmp(rom + 0x134, "POKEMON BLUE", 12) ? 2 : 0);
    if (!game)return 0;
    memset(w, 0, sizeof(*w)); w->rom = rom; w->romSize = bytes; w->game = game; w->rng = 0x52424e31u;
    w->lastMapRom = 0; w->respawnMap = PBRB_MAP_DOWNSTAIRS; w->respawnX = 3; w->respawnY = 5;
    InitializeObjects(w);
    if (!PbrbWorldSelectMap(w, mapId)) { memset(w, 0, sizeof(*w)); return 0; }return 1;
}
static void HeroPath(PbrbWorld* w) { w->heroPathCount = w->heroPathIndex = 0; }
static void ActorPath(PbrbWorld* w, int actor) { w->actorPathCount = w->actorPathIndex = 0; w->pathActor = actor; }
static void AddHero(PbrbWorld* w, int dir, int count) { while (count-- > 0 && w->heroPathCount < PBRB_WORLD_PATH)w->heroPath[w->heroPathCount++] = (unsigned char)dir; }
static void AddActor(PbrbWorld* w, int dir, int count) { while (count-- > 0 && w->actorPathCount < PBRB_WORLD_PATH)w->actorPath[w->actorPathCount++] = (unsigned char)dir; }
static void Approach(PbrbWorld* w, int actor, int x, int y) {
    ActorPath(w, actor); int ax = w->actors[actor].x, ay = w->actors[actor].y;
    /* FindPathToPlayer reduces the larger remaining axis, X on ties. */
    while ((ax != x || ay != y) && w->actorPathCount < PBRB_WORLD_PATH) {
        int d = Abs(x - ax) >= Abs(y - ay) ? (x < ax ? 2 : 3) : (y < ay ? 1 : 0);
        AddActor(w, d, 1); ax += DX[d]; ay += DY[d];
    }
}
static int PathsDone(const PbrbWorld* w) {
    if (w->heroStepLength || w->heroPathIndex < w->heroPathCount)return 0;
    if (w->actorPathIndex < w->actorPathCount)return 0;
    return w->pathActor < 0 || !w->actors[w->pathActor].stepLength;
}
static int StartHero(PbrbWorld* w, int dir, int forced) {
    w->facing = dir; int x = w->heroX + DX[dir], y = w->heroY + DY[dir], pushing = 0;
    if (!forced) {
        if (TilePairCollision(w, w->heroX, w->heroY, x, y, w->travelMode == 2))return 0;
        int actor = ActorAt(w, x, y, -1);
        if (actor >= 0) {
            PbrbWorldActor* a = &w->actors[actor];
            if (a->sprite != 63 || !(w->fieldActive & PBRB_WORLD_FIELD_STRENGTH) || a->stepLength ||
                !TilePassable(w, x + DX[dir], y + DY[dir], 0) ||
                TilePairCollision(w, w->heroX, w->heroY, x + DX[dir], y + DY[dir], 0) ||
                ActorAt(w, x + DX[dir], y + DY[dir], actor) >= 0)return 0;
            a->facing = dir; a->dx = DX[dir]; a->dy = DY[dir]; a->stepFrame = 0; a->stepLength = 16;
            pushing = 1;
            Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_PUSH_BOULDER);
        }
        if (!Passable(w, x, y)) {
            static const unsigned char ledges[8][3] = { {0,0x2c,0x37},{0,0x39,0x36},{0,0x39,0x37},
                {2,0x2c,0x27},{2,0x39,0x27},{3,0x2c,0x0d},{3,0x2c,0x1d},{3,0x39,0x0d} };
            unsigned from = TileAt(w, w->heroX * 2, w->heroY * 2 + 1), to = TileAt(w, x * 2, y * 2 + 1); int jump = 0;
            if (w->tileset == 0 && actor < 0 && Passable(w, x + DX[dir], y + DY[dir]) && ActorAt(w, x + DX[dir], y + DY[dir], -1) < 0)
                for (int li = 0; li < 8; ++li)if (ledges[li][0] == dir && ledges[li][1] == from && ledges[li][2] == to)jump = 1;
            if (!jump)return 0;
            w->ledge = 2; HeroPath(w); AddHero(w, dir, 1); forced = 1;
            Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_LEDGE);
        }
    }
    w->heroDx = DX[dir]; w->heroDy = DY[dir]; w->heroStepFrame = 0; w->heroStepLength = 16; w->wasMoving = 1;
    if (!forced && !pushing && w->travelMode == 1 && (w->romMapId != 0x1c || dir == 0))w->heroStepLength = 8;
    w->heroForced = forced;
    return 1;
}
int PbrbWorldMove(PbrbWorld* w, int dx, int dy) {
    if (!w || !w->rom || w->event.kind || w->stage != WS_IDLE || w->heroStepLength || w->transitionFrame ||
        (dx && dy) || (!dx && !dy))return 0;
    return StartHero(w, dx < 0 ? 2 : dx>0 ? 3 : dy < 0 ? 1 : 0, 0);
}
static void StartActor(PbrbWorld* w, int actor, int dir, int sync) {
    PbrbWorldActor* a = &w->actors[actor]; a->facing = dir; a->dx = DX[dir]; a->dy = DY[dir];
    a->stepFrame = 0; a->stepLength = sync ? 16 : 32;
}
static void Transition(PbrbWorld* w, int map, int x, int y, int face, int stage, int exitDoor) {
    w->transitionMap = map; w->transitionX = x; w->transitionY = y; w->transitionFacing = face;
    w->transitionStage = stage; w->transitionExit = exitDoor; w->transitionFrame = 1;
    w->heroPathIndex = w->heroPathCount; w->actorPathIndex = w->actorPathCount;
    Emit(w, PBRB_WORLD_SFX, 0, w->mapId == PBRB_MAP_PALLET ? PBRB_SFX_GO_INSIDE : PBRB_SFX_GO_OUTSIDE);
}
static int CheckConnection(PbrbWorld* w) {
    int map, x, y;
    if (!ConnectionTarget(w, w->heroX, w->heroY, &map, &x, &y))return 0;
    int face = w->facing; unsigned oldFrames = w->frames;
    if (!LoadMap(w, map, x, y, face))return 0;
    w->frames = oldFrames; w->warpLock = 0; w->encounterCooldown = 3;
    if (w->romMapId < 0x25)w->lastMapRom = w->romMapId;
    Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w)); return 1;
}
static int CheckWarp(PbrbWorld* w) {
    int x = w->heroX, y = w->heroY, atWarp = 0;
    for (int i = 0; i < w->warpCount; ++i) {
        const PbrbWorldWarp* a = &w->warps[i]; if (a->x != x || a->y != y)continue;
        atWarp = 1; if (w->warpLock)return 0;
        /* The bottom-edge doorway is crossed while facing down. Stairs,
           ladders and interior warp pads retain the approach direction. */
        if (y == w->heightBlocks * 2 - 1 && w->romMapId >= 0x25 && w->facing != 0)return 0;
        int raw = a->destination == 255 ? w->lastMapRom : (int)a->destination;
        WorldRawMap dest; if (!RawMap(w, raw, &dest))return 0;
        unsigned p = dest.objects + 1; if (!InRom(w, p, 1) || a->warp >= w->rom[p])return 0;
        p += 1u + (unsigned)a->warp * 4u; if (!InRom(w, p, 4))return 0;
        int dy = w->rom[p], dx = w->rom[p + 1], face = w->facing, exitDoor = 0;
        if (w->romMapId < 0x25) {
            w->lastMapRom = w->romMapId; w->lastOutdoorX = x; w->lastOutdoorY = y;
            face = dy == 0 ? 0 : dy == dest.height * 2 - 1 ? 1 : dx == 0 ? 3 : dx == dest.width * 2 - 1 ? 2 : 1;
        }
        else if (raw < 0x25) {
            if (y == 0)face = 1;
            else if (y == w->heightBlocks * 2 - 1)face = 0;
            else if (x == 0)face = 2;
            else if (x == w->widthBlocks * 2 - 1)face = 3;
            else face = w->facing;
            exitDoor = face + 1;
        }
        w->lastWarp = i; w->arrivalWarp = a->warp;
        if (raw == 0xcb || raw == 0xec || raw == 0x7f) { w->storyValues[8] = (unsigned char)w->romMapId; w->storyValues[9] = (unsigned char)i; }
        int stage = w->stage == WS_ESCORT && raw == 0x28 ? WS_LAB_OAK_START : WS_IDLE;
        Transition(w, PbrbWorldMapFromRom((unsigned)raw), dx, dy, face, stage, exitDoor); return 1;
    }
    if (!atWarp)w->warpLock = 0;
    return 0;
}
static void GeneralStep(PbrbWorld* w);
static void GeneralIdle(PbrbWorld* w);
static int WorldScriptSyncActors(const PbrbWorld* w);
enum WorldFieldAnimationKind {
    WFA_NONE = 0, WFA_CUT_TREE, WFA_CUT_GRASS, WFA_FISHING, WFA_FLY, WFA_ESCAPE
};
static void FieldAnimationStart(PbrbWorld*, int);
static int FieldAnimationTick(PbrbWorld*);
static void CheckTriggers(PbrbWorld* w) {
    if (w->stage != WS_IDLE || w->event.kind || w->heroStepLength)return;
    if (w->mapId == PBRB_MAP_PALLET && w->heroY == 1 && !(w->flags & WF_FOLLOWED_OAK)) {
        w->flags |= WF_OAK_APPEARED; w->stage = WS_OAK_HEY;
        Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_OAK); return;
    }
    if (w->mapId == PBRB_MAP_LAB && w->heroY == 6 && (w->flags & WF_FOLLOWED_OAK) && !(w->flags & WF_BATTLED)) {
        w->actors[0].facing = w->actors[4].facing = 0;
        if (!(w->flags & WF_RIVAL_CHOSE)) { Say(w, WT_OaksLabOakDontGoAwayYetText, WS_FORCE_BACK, 0); return; }
        w->facing = 1; w->stage = WS_CHALLENGE_TEXT; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_RIVAL);
    }
}
static void AdvanceMotion(PbrbWorld* w) {
    /* The overworld loop updates sprites every two VBlanks. Player/synced
       followers advance two pixels; ordinary NPCs advance one pixel. */
    if (w->frames & 1u)return;
    if (w->heroStepLength) {
        w->heroStepFrame += 2; w->heroAnim = (w->heroAnim + 1) & 15;
        if (w->heroStepFrame >= w->heroStepLength) {
            w->heroX += w->heroDx; w->heroY += w->heroDy;
            w->heroStepLength = w->heroStepFrame = 0; w->heroDx = w->heroDy = 0;
            if (w->ledge)--w->ledge;
            if (w->steps < 0x7fffffff)++w->steps;
            Camera(w); if (CheckConnection(w) || CheckWarp(w))return; CheckTriggers(w);
            if (!w->event.kind && w->stage == WS_IDLE && !w->heroForced)GeneralStep(w);
        }
    }
    for (int i = 0; i < w->actorCount; ++i) {
        PbrbWorldActor* a = &w->actors[i]; if (!a->visible || !a->stepLength)continue;
        a->stepFrame += 2; a->anim = (a->anim + 1) & 15;
        if (a->stepFrame >= a->stepLength) {
            a->x += a->dx; a->y += a->dy; a->stepLength = a->stepFrame = 0; a->dx = a->dy = 0;
            int r = RandomByte(w) & 127; a->wait = (r ? r : 256) * 2;
        }
    }
    Camera(w);
}
static void StartPaths(PbrbWorld* w) {
    if (w->heroPathIndex < w->heroPathCount && !w->heroStepLength)
        StartHero(w, w->heroPath[w->heroPathIndex++], 1);
    if (w->pathActor >= 0 && w->actorPathIndex < w->actorPathCount && !w->actors[w->pathActor].stepLength)
        StartActor(w, w->pathActor, w->actorPath[w->actorPathIndex++], w->stage == WS_ESCORT || WorldScriptSyncActors(w));
}
static void AmbientActors(PbrbWorld* w) {
    if (w->frames & 1u)return;
    for (int i = 0; i < w->actorCount; ++i) {
        PbrbWorldActor* a = &w->actors[i];
        if (!a->visible || a->stepLength || a->sprite >= 61 || i == w->pathActor)continue;
        int sx = a->x * 16 - w->cameraX, sy = a->y * 16 - w->cameraY - 4;
        if (sx < -16 || sx >= 160 || sy < -16 || sy >= 144)continue;
        a->anim = 0; if (a->wait > 0) { a->wait -= 2; continue; }
        if (w->heroStepLength)continue;
        unsigned r = RandomByte(w); int dir = (int)(r >> 6);
        if (a->range >= 0xd0 && a->range <= 0xd3)dir = a->range - 0xd0;
        else if (a->range == 1) { if (dir == 2)dir = 1; else if (dir == 3)dir = 0; }
        else if (a->range == 2) { if (dir == 0)dir = 2; else if (dir == 1)dir = 3; }
        a->facing = dir; int x = a->x + DX[dir], y = a->y + DY[dir];
        int delay = (int)(RandomByte(w) & 127); a->wait = (delay ? delay : 256) * 2;
        int xDisplacement = (a->x - a->homeX + 8) & 255;
        int yDisplacement = (a->y - a->homeY + 8) & 255;
        /* Preserve the original asymmetric displacement checks, including
           its missing rightward bound; collision and screen bounds still apply. */
        if (a->movement != 254 || !Passable(w, x, y) ||
            sx + DX[dir] < 0 || sx + DX[dir] >= 144 || sy + 4 + DY[dir] < 0 || sy + 4 + DY[dir] >= 128 ||
            (DY[dir] < 0 ? yDisplacement == 0 : ((yDisplacement + DY[dir]) & 255) < 5) ||
            (DX[dir] < 0 && xDisplacement == 0) ||
            ActorAt(w, x, y, i) >= 0 || (x == w->heroX && y == w->heroY))continue;
        StartActor(w, i, dir, 0);
    }
}
#include "pbrb_world_story_text.inl"
#include "pbrb_world_gameplay.inl"
static void Interact(PbrbWorld* w) {
    int x = w->heroX + DX[w->facing], y = w->heroY + DY[w->facing];
    if (GeneralInteract(w, x, y))return;
    if (w->mapId == PBRB_MAP_BEDROOM) {
        if (x == 0 && y == 1 && w->facing == 1) { Emit(w, PBRB_WORLD_PC, 0, 0); return; }
        if (x == 3 && y == 5) { Say(w, WT_RedBedroomSNESText, WS_IDLE, 0); return; }
    }
    /* Bookshelf hidden events take precedence over object/sign events. */
    if (w->facing == 1) {
        unsigned t = TileAt(w, x * 2, y * 2 + 1);
        if ((w->mapId == PBRB_MAP_DOWNSTAIRS && t == 0x32) ||
            (w->mapId == PBRB_MAP_LAB && t == 0x1d) ||
            (w->mapId == PBRB_MAP_BLUE_HOUSE && t == 0x1e)) {
            Say(w, WT_PokemonBooksText, WS_IDLE, 0); return;
        }
        if (w->mapId == PBRB_MAP_BLUE_HOUSE && t == 0x3d) {
            Emit(w, PBRB_WORLD_TOWN_MAP, Text(w, WT_TownMapText), 0); return;
        }
    }
    int i = ActorAt(w, x, y, -1);
    if (i < 0 && w->counterTile != 255 && TileAt(w, x * 2, y * 2 + 1) == (unsigned)w->counterTile)
        i = ActorAt(w, x + DX[w->facing], y + DY[w->facing], -1);
    if (i >= 0) {
        PbrbWorldActor* a = &w->actors[i];
        if (a->stepLength)return;
        w->interactionFacing = a->facing;
        if (a->sprite < 61) { a->facing = w->facing ^ 1; a->anim = 0; }
        w->interactionActor = i;
        if (w->mapId == PBRB_MAP_DOWNSTAIRS) {
            if (w->flags & WF_CHOSE_STARTER)Say(w, WT_RedsHouse1FMomYouShouldRestText, WS_MOM_HEAL, 0);
            else Say(w, WT_RedsHouse1FMomWakeUpText, WS_IDLE, 0);
            return;
        }
        if (w->mapId == PBRB_MAP_PALLET) {
            Say(w, i == 1 ? WT_PalletTownGirlText : i == 2 ? WT_PalletTownFisherText : WT_PalletTownOakItsUnsafeText, WS_IDLE, 0); return;
        }
        if (w->mapId == PBRB_MAP_BLUE_HOUSE) {
            Say(w, i == 2 ? WT_BluesHouseTownMapText : WT_BluesHouseDaisyRivalAtLabText, WS_IDLE, 0); return;
        }
        if (w->mapId == PBRB_MAP_LAB) {
            if (i == 0) {
                Say(w, (w->flags & WF_CHOSE_STARTER) ? WT_OaksLabRivalMyPokemonLooksStrongerText :
                    (w->flags & WF_FOLLOWED_OAK) ? WT_OaksLabRivalGoAheadAndChooseText : WT_OaksLabRivalGrampsIsntAroundText, WS_IDLE, 0); return;
            }
            if (i >= 1 && i <= 3) {
                if (!(w->flags & WF_FOLLOWED_OAK)) { Say(w, WT_OaksLabThoseArePokeBallsText, WS_IDLE, 0); return; }
                if (w->flags & WF_CHOSE_STARTER) { Say(w, WT_OaksLabLastMonText, WS_IDLE, 0); return; }
                static const int question[3] = { WT_OaksLabYouWantCharmanderText,WT_OaksLabYouWantSquirtleText,WT_OaksLabYouWantBulbasaurText };
                w->selectedStarter = STARTERS[i - 1]; w->stage = WS_STARTER_CHOICE;
                Emit(w, PBRB_WORLD_STARTER_CHOICE, Text(w, question[i - 1]), w->selectedStarter); return;
            }
            if (i == 4) {
                Say(w, (w->flags & WF_BATTLED) ? WT_OaksLabOak1RaiseYourYoungPokemonText :
                    (w->flags & WF_CHOSE_STARTER) ? WT_OaksLabOak1YourPokemonCanFightText : WT_OaksLabOak1WhichPokemonDoYouWantText, WS_IDLE, 0); return;
            }
            Say(w, i == 5 || i == 6 ? WT_OaksLabPokedexText : i == 7 ? WT_OaksLabOak2Text : i == 8 ? WT_OaksLabGirlText : WT_OaksLabScientistText, WS_IDLE, 0); return;
        }
    }
    if (w->mapId == PBRB_MAP_PALLET) {
        int text = -1;
        if (x == 13 && y == 13)text = WT_PalletTownOaksLabSignText;
        if (x == 7 && y == 9)text = WT_PalletTownSignText;
        if (x == 3 && y == 5)text = WT_PalletTownPlayersHouseSignText;
        if (x == 11 && y == 5)text = WT_PalletTownRivalsHouseSignText;
        if (text >= 0)Say(w, text, WS_IDLE, 0);
    }
    if (w->mapId == PBRB_MAP_DOWNSTAIRS && x == 3 && y == 1)
        Say(w, w->facing == 1 ? WT_RedsHouse1FTVStandByMeMovieText : WT_RedsHouse1FTVWrongSideText, WS_IDLE, 0);
}
static void Script(PbrbWorld* w) {
    if (w->delay > 0) { --w->delay; return; }
    switch (w->stage) {
    case WS_IDLE: CheckTriggers(w); if (w->stage == WS_IDLE && !w->event.kind)GeneralIdle(w); break;
    case WS_NATIVE_SCRIPT: GeneralScript(w); break;
    case WS_OAK_HEY:
        Say(w, WT_PalletTownOakHeyWaitDontGoOutText, WS_OAK_BUBBLE_START, 0, PBRB_WORLD_AUTO_CLOSE); break;
    case WS_OAK_BUBBLE_START:
        w->delay = 10; w->stage = WS_OAK_BUBBLE; break;
    case WS_OAK_BUBBLE:
        w->delay = 60; w->stage = WS_OAK_APPROACH_START; break;
    case WS_OAK_APPROACH_START:
        w->facing = 0; w->actors[0].visible = 1; w->actors[0].facing = 1;
        Approach(w, 0, w->heroX, w->heroY + 1); w->delay = 3; w->stage = WS_OAK_APPROACH; break;
    case WS_OAK_APPROACH:
        if (PathsDone(w)) { w->facing = 0; Say(w, WT_PalletTownOakItsUnsafeText, WS_OAK_ALIGN, 0); }break;
    case WS_OAK_ALIGN:
        ActorPath(w, 0); if (w->heroX == 11)AddActor(w, 2, 1);
        w->stage = WS_PLAYER_ALIGN; break;
    case WS_PLAYER_ALIGN:
        if (PathsDone(w)) { HeroPath(w); if (w->heroX == 11)AddHero(w, 2, 1); w->stage = WS_ESCORT_START; }break;
    case WS_ESCORT_START:
        if (!PathsDone(w))break;
        HeroPath(w); ActorPath(w, 0);
        AddHero(w, 0, 6); AddHero(w, 2, 1); AddHero(w, 0, 5); AddHero(w, 3, 3); AddHero(w, 1, 1);
        AddActor(w, 0, 5); AddActor(w, 2, 1); AddActor(w, 0, 5); AddActor(w, 3, 3); AddActor(w, 1, 1);
        w->stage = WS_ESCORT; break;
    case WS_ESCORT:
        if (w->actorPathIndex >= w->actorPathCount && !w->actors[0].stepLength)w->actors[0].visible = 0;
        break; /* The original lab door warp advances this script. */
    case WS_LAB_OAK_START:
        ActorPath(w, 7); AddActor(w, 1, 3); w->stage = WS_LAB_OAK_WALK; break;
    case WS_LAB_OAK_WALK:
        if (PathsDone(w)) {
            w->actors[7].visible = 0; w->actors[4].visible = 1; w->actors[4].facing = 0;
            w->stage = WS_LAB_PLAYER_START; w->delay = 3;
        }break;
    case WS_LAB_PLAYER_START:
        HeroPath(w); ActorPath(w, -1); AddHero(w, 1, 8); w->actors[0].facing = 0;
        w->stage = WS_LAB_PLAYER_WALK; break;
    case WS_LAB_PLAYER_WALK:
        if (PathsDone(w)) { w->flags |= WF_FOLLOWED_OAK; w->actors[0].facing = 1; w->heroAnim = 0; w->stage = WS_LAB_MUSIC; }break;
    case WS_LAB_MUSIC:
        w->stage = WS_LAB_SPEECH1; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_LAB); break;
    case WS_LAB_SPEECH1:
        Say(w, WT_OaksLabRivalFedUpWithWaitingText, WS_LAB_SPEECH2, 0); w->delay = 3; break;
    case WS_LAB_SPEECH2:
        Say(w, WT_OaksLabOakChooseMonText, WS_LAB_SPEECH3, 0); w->delay = 3; break;
    case WS_LAB_SPEECH3:
        Say(w, WT_OaksLabRivalWhatAboutMeText, WS_LAB_SPEECH4, 0); w->delay = 3; break;
    case WS_LAB_SPEECH4:
        Say(w, WT_OaksLabOakBePatientText, WS_IDLE, 0); w->pathActor = -1; break;
    case WS_FORCE_BACK:
        HeroPath(w); AddHero(w, 1, 1); w->stage = WS_FORCE_BACK_WAIT; break;
    case WS_FORCE_BACK_WAIT:
        if (PathsDone(w)) { w->stage = WS_IDLE; w->delay = 3; }break;
    case WS_STARTER_CHOICE:break; /* Parent owns picture, cry and Yes/No. */
    case WS_STARTER_ENERGETIC:
        Say(w, WT_OaksLabMonEnergeticText, WS_STARTER_RECEIVED, 0); break;
    case WS_STARTER_RECEIVED:
        Say(w, WT_OaksLabReceivedMonText, WS_STARTER_SOUND, w->selectedStarter,
            PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT); break;
    case WS_STARTER_SOUND:
        w->stage = WS_STARTER_GIVE; Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_GET_KEY_ITEM, PBRB_WORLD_WAIT_SOUND); break;
    case WS_STARTER_GIVE:
        Emit(w, PBRB_WORLD_GIVE_STARTER, 0, w->selectedStarter); break;
    case WS_RIVAL_PICK_START:
        ActorPath(w, 0);
        if (w->selectedStarter == 0x99) {
            if (w->heroX == 9) { w->actors[0].x = 5; w->actors[0].y = 4; AddActor(w, 3, 1); }
            else { AddActor(w, 0, 1); AddActor(w, 3, 2); }
        }
        else {
            AddActor(w, 0, w->heroY == 4 ? 2 : 1); AddActor(w, 3, w->selectedStarter == 0xb0 ? 3 : 4);
            if (w->heroY == 4)AddActor(w, 1, 1);
        }
        w->stage = WS_RIVAL_PICK_WALK; break;
    case WS_RIVAL_PICK_WALK:
        if (PathsDone(w)) { w->actors[0].facing = 1; Say(w, WT_OaksLabRivalIllTakeThisOneText, WS_RIVAL_PICK_TEXT, 0); }break;
    case WS_RIVAL_PICK_TEXT: {
        int b = BallForSpecies(w->rivalStarter); if (b >= 0)w->actors[b].visible = 0;
        w->flags |= WF_RIVAL_CHOSE; w->delay = 3; w->stage = WS_RIVAL_RECEIVED; break;
    }
    case WS_RIVAL_RECEIVED:
        Say(w, WT_OaksLabRivalReceivedMonText, WS_RIVAL_SOUND, w->rivalStarter,
            PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT); break;
    case WS_RIVAL_SOUND:
        w->pathActor = -1; w->stage = WS_IDLE;
        Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_GET_KEY_ITEM, PBRB_WORLD_WAIT_SOUND | PBRB_WORLD_WAIT_BUTTON); break;
    case WS_CHALLENGE_TEXT:
        Say(w, WT_OaksLabRivalIllTakeYouOnText, WS_CHALLENGE_APPROACH_START, 0); break;
    case WS_CHALLENGE_APPROACH_START:
        Approach(w, 0, w->heroX, w->heroY - 1); w->stage = WS_CHALLENGE_APPROACH; break;
    case WS_CHALLENGE_APPROACH:
        if (PathsDone(w)) {
            w->actors[0].facing = 0; w->facing = 1; w->stage = WS_BATTLE_WAIT;
            Emit(w, PBRB_WORLD_BATTLE, 0, w->rivalStarter, PBRB_WORLD_TRAINER | PBRB_WORLD_FIRST_RIVAL);
            w->event.species = (unsigned)w->rivalStarter; w->event.level = 5;
            w->event.trainerClass = 25; w->event.trainerId = w->rivalStarter == 0xb1 ? 1 : w->rivalStarter == 0x99 ? 2 : 3;
            w->event.winTextOffset = Text(w, WT_OaksLabRivalIPickedTheWrongPokemonText);
            w->event.loseTextOffset = Text(w, WT_OaksLabRivalAmIGreatOrWhatText);
            w->event.reward = 175;
        }break;
    case WS_BATTLE_WAIT:break;
    case WS_AFTER_HEAL:
        w->stage = WS_AFTER_DELAY; Emit(w, PBRB_WORLD_HEAL, 0, 0); break;
    case WS_AFTER_DELAY:
        w->actors[0].facing = 0; w->delay = 20; w->stage = WS_AFTER_TEXT; break;
    case WS_AFTER_TEXT:
        Say(w, WT_OaksLabRivalSmellYouLaterText, WS_RIVAL_EXIT_START, 0); break;
    case WS_RIVAL_EXIT_START:
        ActorPath(w, 0); AddActor(w, w->heroX == 4 ? 3 : 2, 1); AddActor(w, 0, 5);
        w->stage = WS_RIVAL_EXIT; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_RIVAL); break;
    case WS_RIVAL_EXIT:
        w->facing = w->actorPathIndex <= 1 ? (w->heroX == 4 ? 3 : 2) : 0;
        if (PathsDone(w)) { w->actors[0].visible = 0; w->pathActor = -1; w->stage = WS_AFTER_MUSIC; }break;
    case WS_AFTER_MUSIC:
        w->stage = WS_END; Emit(w, PBRB_WORLD_MUSIC, 0, PBRB_MUSIC_LAB); break;
    case WS_END:
        w->flags |= WF_END_REPORTED; w->stage = WS_IDLE; break;
    case WS_MOM_HEAL:
        w->stage = WS_MOM_GOOD; Emit(w, PBRB_WORLD_HEAL, 0, 1); break;
    case WS_MOM_GOOD:
        Say(w, WT_RedsHouse1FMomLookingGreatText, WS_IDLE, 0); break;
    default:break;
    }
}
static void TickTransition(PbrbWorld* w) {
    ++w->transitionFrame;
    if (w->transitionFrame == 33) {
        if (!LoadMap(w, w->transitionMap, w->transitionX, w->transitionY, w->transitionFacing)) {
            w->transitionFrame = 0; Emit(w, PBRB_WORLD_PLAYTEST_END, 0, -1); return;
        }
        w->stage = w->transitionStage;
    }
    if (w->transitionFrame >= 65) {
        w->transitionFrame = 0;
        if (w->transitionExit) { HeroPath(w); AddHero(w, w->transitionExit - 1, 1); }
        Emit(w, PBRB_WORLD_MUSIC, 0, PbrbWorldMusic(w));
    }
}
void PbrbWorldTick(PbrbWorld* w, unsigned held, unsigned pressed) {
    if (!w || !w->rom)return;
    ++w->frames;
    if (w->event.kind)return;
    if (w->transitionFrame) { TickTransition(w); return; }
    if (FieldAnimationTick(w))return;
    int wasWalking = w->heroStepLength;
    AdvanceMotion(w);
    if (w->event.kind || w->transitionFrame || (wasWalking && !w->heroStepLength))return;
    Script(w);
    if (w->event.kind)return;
    if (!w->delay)StartPaths(w);
    if (w->stage != WS_IDLE || w->heroStepLength || w->heroPathIndex < w->heroPathCount)return;
    w->heroPathCount = w->heroPathIndex = 0;
    AmbientActors(w);
    if (w->turnDelay) { --w->turnDelay; return; }
    if (pressed & 16u) { w->heroAnim = 0; w->wasMoving = 0; Interact(w); return; }
    int d = (held & 8u) ? 0 : (held & 4u) ? 1 : (held & 2u) ? 2 : (held & 1u) ? 3 : -1;
    if (d < 0 && w->romMapId == 0x1c && !(held & 63u))d = 0;
    if (d < 0) { w->heroAnim = 0; w->wasMoving = 0; return; }
    if (!w->wasMoving && d != w->facing) { w->facing = d; w->turnDelay = 1; return; }
    if (!StartHero(w, d, 0)) {
        w->heroAnim = 0; w->wasMoving = 0; w->turnDelay = 7;
        /* Handle an exit when loading on a boundary warp before moving. */
        if (w->romMapId >= 0x25 && w->heroY == w->heightBlocks * 2 - 1 && d == 0) {
            w->warpLock = 0; if (CheckWarp(w))return;
        }
        Emit(w, PBRB_WORLD_SFX, 0, PBRB_SFX_COLLISION);
    }
}
const PbrbWorldEvent* PbrbWorldPendingEvent(const PbrbWorld* w) {
    return w && w->event.kind ? &w->event : 0;
}
void PbrbWorldAcknowledge(PbrbWorld* w, int result) {
    if (!w || !w->event.kind)return;
    if (GeneralAcknowledge(w, result))return;
    int kind = w->event.kind; w->event.kind = 0; w->event.textOffset = 0; w->event.value = 0; w->event.flags = 0;
    if (kind == PBRB_WORLD_STARTER_CHOICE) {
        if (result) {
            w->flags |= WF_CHOSE_STARTER; int b = BallForSpecies(w->selectedStarter);
            if (b >= 0)w->actors[b].visible = 0;
            w->rivalStarter = w->selectedStarter == 0xb0 ? 0xb1 : w->selectedStarter == 0xb1 ? 0x99 : 0xb0;
            w->stage = WS_STARTER_ENERGETIC;
        }
        else { w->selectedStarter = 0; w->stage = WS_IDLE; }
    }
    else if (kind == PBRB_WORLD_GIVE_STARTER)w->stage = WS_RIVAL_PICK_START;
    else if (kind == PBRB_WORLD_BATTLE) {
        w->flags |= WF_BATTLED;
        PbrbWorldSetEvent(w, WE_EVENT_BATTLED_RIVAL_IN_OAKS_LAB, 1); w->stage = WS_AFTER_HEAL;
    }
    /* CloseTextDisplay restores the pre-dialogue facing. Keep it through
       multi-part interactions such as Mom's rest/heal/goodbye sequence. */
    if (w->stage == WS_IDLE && w->interactionActor >= 0) {
        if (w->interactionActor < w->actorCount && w->actors[w->interactionActor].sprite < 61)
            w->actors[w->interactionActor].facing = w->interactionFacing;
        w->interactionActor = -1;
    }
    w->heroAnim = 0; w->wasMoving = 0;
}
static unsigned ColorIndex(const unsigned char* tile, int x, int y) {
    unsigned bit = 7u - (unsigned)(x & 7);
    return ((tile[y * 2] >> bit) & 1u) | (((tile[y * 2 + 1] >> bit) & 1u) << 1);
}
static void AddOam(PbrbGbOam* out, unsigned* count, unsigned gfx, int x, int y, int flip, int grass) {
    for (int ty = 0; ty < 2; ++ty)for (int tx = 0; tx < 2; ++tx) {
        int px = x + (flip ? 1 - tx : tx) * 8, py = y + ty * 8;
        if (px <= -8 || px >= 160 || py <= -8 || py >= 144 || *count >= 40)continue;
        PbrbGbOam* o = out + (*count)++; o->offset = gfx + (unsigned)(ty * 2 + tx) * 16u;
        o->x = (unsigned char)(px + 8); o->y = (unsigned char)(py + 16);
        o->attributes = (unsigned char)((flip ? 0x20 : 0) | (grass && ty ? 0x80 : 0));
    }
}
static void SpriteOam(const PbrbWorld* w, PbrbGbOam* out, unsigned* count, int sprite,
    int x, int y, int facing, int anim, int grass) {
    if (sprite < 1 || sprite>72)return;
    unsigned off = WORLD_SPRITES[GameIndex(w)] + (unsigned)(sprite - 1) * 4u;
    if (!InRom(w, off, 4))return;
    const unsigned char* p = w->rom + off;
    unsigned gfx = Address(p[3], Read16(p));
    int flip = 0;
    if (p[2] != 64) {
        int frame = (anim / 4) & 3;
        gfx += (unsigned)(facing < 2 ? facing : 2) * 64u;
        if (frame & 1)gfx += 192u;
        flip = facing == 3 || (facing < 2 && frame == 3);
    }
    if (InRom(w, gfx, 64))AddOam(out, count, gfx, x, y, flip, grass);
}
#include "pbrb_world_field.inl"
void PbrbWorldRender(const PbrbWorld* w, PbrbGb* g) {
    if (!w || !w->rom || !g || !g->pixels)return;
    unsigned bg = 0xe4, obj0 = 0xd0, obj1 = 0xe0;
    if ((w->romMapId == 0x52 || w->romMapId == 0xe8) && !(w->fieldActive & PBRB_WORLD_FIELD_FLASH)) {
        bg = 0xfe; obj0 = 0xfe; obj1 = 0xf8;
    }
    if (w->stage == WS_NATIVE_SCRIPT && w->scriptId == WG_MUSEUM && w->scriptValue == 2 && w->scriptStep == 2 && w->delay > 0) {
        static const unsigned white[4][3] = { {0xe4,0xd0,0xe0},{0x90,0x80,0x90},{0x40,0x40,0x40},{0,0,0} };
        int t = 51 - w->delay, index = t < 24 ? 1 + t / 8 : t < 27 ? 3 : 2 - (t - 27) / 8;
        if (index < 0)index = 0; if (index > 3)index = 3;
        bg = white[index][0]; obj0 = white[index][1]; obj1 = white[index][2];
    }
    if (w->transitionFrame) {
        static const unsigned fade[4][3] = { {0xe4,0xd0,0xe0},{0xf9,0xe4,0xe4},{0xfe,0xfe,0xf8},{0xff,0xff,0xff} };
        int t = w->transitionFrame <= 32 ? (w->transitionFrame - 1) / 8 : 3 - (w->transitionFrame - 33) / 8;
        if (t < 0)t = 0;
        if (t > 3)t = 3;
        bg = fade[t][0]; obj0 = fade[t][1]; obj1 = fade[t][2];
    }
    FieldAnimationPalettes(w, &bg, &obj0, &obj1);
    PbrbGbSetPalettes(g, bg, obj0, obj1);
    for (int y = 0; y < 144; ++y)for (int x = 0; x < 160; ++x) {
        int wx = x + w->cameraX, wy = y + w->cameraY;
        int tx = FloorDiv(wx, 8), ty = FloorDiv(wy, 8);
        unsigned tile = TileAt(w, tx, ty), off = w->tilesOffset + tile * 16u;
        int gx = wx - tx * 8, gy = wy - ty * 8;
        if (w->stage == WS_NATIVE_SCRIPT && w->scriptId == WG_SPINNER && w->scriptStep == 1 &&
            ((w->heroPathCount - w->heroPathIndex) & 1)) {
            static const unsigned char facility[4] = { 0x20,0x21,0x30,0x31 };
            static const unsigned char gym[4] = { 0x4c,0x3c,0x4d,0x3d };
            const unsigned char* arrows = w->tileset == 22 ? facility : gym;
            for (unsigned ai = 0; ai < 4; ++ai)if (tile == arrows[ai])off = WORLD_SPINNERARROWANIMTILES[GameIndex(w)] + ai * 16u;
        }
        if (w->tileset == 0 || w->mapId == PBRB_MAP_LAB) {
            unsigned phase = (w->frames / 21u) & 7u;
            if (tile == 3u && w->frames >= 21u) {
                unsigned flower = phase & 3u; off = WORLD_FLOWER[GameIndex(w)] + (flower < 2u ? 0u : flower - 1u) * 16u;
            }
            if (tile == 0x14u) { int shift = phase < 4u ? (int)phase : 6 - (int)phase; gx = (gx - shift + 8) & 7; }
        }
        unsigned c = InRom(w, off, 16) ? ColorIndex(w->rom + off, gx, gy) : 0;
        PbrbGbPixel(g, x, y, c, 0);
    }
    WorldAuxiliaryRender(w, g);
    PbrbGbOam oam[40]; unsigned count = 0;
    if (w->stage == WS_OAK_APPROACH_START && w->delay > 0)
        AddOam(oam, &count, WORLD_SHOCK[GameIndex(w)], 64, 44, 0, 0);
    int heroY = 60;
    if (w->ledge) { int t = (2 - w->ledge) * 16 + w->heroStepFrame; heroY -= t < 16 ? t / 2 : (32 - t) / 2; }
    if (FieldAnimationPlayer(w, oam, &count)) {
        /* The original field animation supplies the player's OAM. */
    }
    else if (w->travelMode == 1) {
        unsigned frame = (unsigned)(w->heroAnim / 4) & 3u;
        unsigned gfx = 0x14000u + (unsigned)(w->facing < 2 ? w->facing : 2) * 64u + (frame & 1u ? 192u : 0u);
        int flip = w->facing == 3 || (w->facing < 2 && frame == 3u);
        AddOam(oam, &count, gfx, 64, heroY, flip, 0);
    }
    else {
        int facing = w->facing, anim = w->heroAnim;
        if (w->stage == WS_NATIVE_SCRIPT && w->scriptId == WG_SPINNER && w->scriptStep == 1) {
            static const int rotate[4] = { 2,3,1,0 }; facing = w->scriptValue; anim = 0;
            for (int si = 0; si < (w->heroAnim & 3); ++si)facing = rotate[facing];
        }
        SpriteOam(w, oam, &count, w->travelMode == 2 ? 60 : 1, 64, heroY, facing, anim,
            TileAt(w, w->heroX * 2, w->heroY * 2 + 1) == (unsigned)w->grassTile);
    }
    for (int i = 0; i < w->actorCount; ++i) {
        const PbrbWorldActor* a = &w->actors[i]; if (!a->visible)continue;
        int p = ActorPixels(a), x = a->x * 16 + a->dx * p - w->cameraX, y = a->y * 16 + a->dy * p - w->cameraY - 4;
        if (x <= -16 || x >= 160 || y <= -16 || y >= 144)continue;
        SpriteOam(w, oam, &count, a->sprite, x, y, a->facing, a->anim,
            TileAt(w, a->x * 2, a->y * 2 + 1) == (unsigned)w->grassTile);
    }
    FieldAnimationOverlay(w, oam, &count);
    PbrbGbDrawOam(g, oam, count, obj0, obj1);
}
void PbrbWorldRenderBGRA(const PbrbWorld* w, unsigned char* out, size_t bytes) {
    if (!w || !out || bytes < 160u * 144u * 4u)return;
    PbrbGb g; memset(&g, 0, sizeof(g)); g.rom = w->rom; g.bytes = w->romSize; g.game = w->game; g.pixels = out;
    PbrbWorldRender(w, &g);
}
const char* PbrbWorldMapName(const PbrbWorld* w) {
    if (!w || w->romMapId < 0 || w->romMapId>255)return "";
    return WORLD_CATALOG_MAPS[w->romMapId].name;
}
int PbrbWorldCanSave(const PbrbWorld* w) {
    if (!w || !w->rom || w->stage != WS_IDLE || w->event.kind || w->transitionFrame || w->delay || w->fieldAnimation.kind ||
        w->heroStepLength || w->heroPathIndex < w->heroPathCount || w->actorPathIndex < w->actorPathCount)return 0;
    /* NPC mid-step state is explicitly serialized below. */
    return 1;
}
struct WorldWriter { unsigned char* p; size_t n, at; int ok; };
static void WriteByte(WorldWriter* s, unsigned n) { if (s->at >= s->n) { s->ok = 0; return; }s->p[s->at++] = (unsigned char)n; }
static void WriteWord(WorldWriter* s, unsigned n) { WriteByte(s, n); WriteByte(s, n >> 8); }
static void WriteDword(WorldWriter* s, unsigned n) { WriteWord(s, n); WriteWord(s, n >> 16); }
struct WorldReader { const unsigned char* p; size_t n, at; int ok; };
static unsigned ReadByte(WorldReader* s) { if (s->at >= s->n) { s->ok = 0; return 0; }return s->p[s->at++]; }
static unsigned ReadWord(WorldReader* s) { unsigned n = ReadByte(s); return n | (ReadByte(s) << 8); }
static unsigned ReadDword(WorldReader* s) { unsigned n = ReadWord(s); return n | (ReadWord(s) << 16); }
size_t PbrbWorldSerialize(const PbrbWorld* w, unsigned char* out, size_t bytes) {
    if (!PbrbWorldCanSave(w) || !out)return 0;
    WorldWriter s = { out,bytes,0,1 };
    WriteDword(&s, 0x31574e50u); WriteWord(&s, 3); WriteByte(&s, w->game); WriteByte(&s, w->romMapId);
    WriteDword(&s, w->flags); WriteDword(&s, w->frames); WriteDword(&s, w->rng);
    WriteByte(&s, w->heroX); WriteByte(&s, w->heroY); WriteByte(&s, w->facing); WriteByte(&s, w->warpLock);
    WriteByte(&s, w->selectedStarter); WriteByte(&s, w->rivalStarter); WriteByte(&s, w->actorCount); WriteByte(&s, 0);
    for (unsigned ei = 0; ei < PBRB_WORLD_EVENT_BYTES; ++ei)WriteByte(&s, w->eventFlags[ei]);
    for (unsigned mi = 0; mi < 256; ++mi)WriteWord(&s, w->hiddenObjects[mi]);
    for (unsigned ti = 0; ti < 256; ++ti)WriteWord(&s, w->defeatedTrainers[ti]);
    WriteByte(&s, w->lastMapRom); WriteByte(&s, w->lastWarp); WriteByte(&s, w->encounterCooldown);
    WriteByte(&s, w->travelMode); WriteByte(&s, w->fieldActive);
    WriteByte(&s, PbrbWorldMapToRom(w->respawnMap)); WriteByte(&s, w->respawnX); WriteByte(&s, w->respawnY);
    WriteByte(&s, w->lastOutdoorX); WriteByte(&s, w->lastOutdoorY);
    WriteWord(&s, w->visitedTowns); WriteDword(&s, w->serviceFlags); WriteDword(&s, (unsigned)w->steps);
    WriteWord(&s, w->safariSteps); WriteByte(&s, w->safariActive);
    for (unsigned sv = 0; sv < 32; ++sv)WriteByte(&s, w->storyValues[sv]);
    for (unsigned hi = 0; hi < 8; ++hi)WriteByte(&s, w->hiddenItems[hi]);
    for (int i = 0; i < w->actorCount; ++i) {
        const PbrbWorldActor* a = &w->actors[i];
        WriteByte(&s, a->sprite); WriteByte(&s, a->x); WriteByte(&s, a->y);
        WriteByte(&s, a->homeX); WriteByte(&s, a->homeY); WriteByte(&s, a->facing);
        WriteByte(&s, a->movement); WriteByte(&s, a->range); WriteByte(&s, a->visible);
        WriteByte(&s, a->dx + 1); WriteByte(&s, a->dy + 1); WriteByte(&s, a->stepFrame);
        WriteByte(&s, a->stepLength); WriteByte(&s, a->anim); WriteWord(&s, a->wait);
    }
    WriteByte(&s, w->blockChangeCount);
    for (int pi = 0; pi < w->blockChangeCount; ++pi) {
        WriteByte(&s, w->blockChanges[pi].x);
        WriteByte(&s, w->blockChanges[pi].y); WriteByte(&s, w->blockChanges[pi].block);
    }
    if (!s.ok || s.at + 4 > bytes)return 0;
    unsigned crc = Crc32(out, (unsigned)s.at); WriteDword(&s, crc);
    return s.ok ? s.at : 0;
}
int PbrbWorldDeserialize(PbrbWorld* w, const unsigned char* data, size_t bytes) {
    if (!w || !w->rom || !data || bytes<32 || bytes>PBRB_WORLD_SAVE_MAX)return 0;
    if (Crc32(data, (unsigned)bytes - 4) != (unsigned)(data[bytes - 4] | ((unsigned)data[bytes - 3] << 8) |
        ((unsigned)data[bytes - 2] << 16) | ((unsigned)data[bytes - 1] << 24)))return 0;
    WorldReader s = { data,bytes - 4,0,1 };
    if (ReadDword(&s) != 0x31574e50u)return 0;
    unsigned version = ReadWord(&s); if (version != 1 && version != 2 && version != 3)return 0;
    unsigned game = ReadByte(&s), mapByte = ReadByte(&s);
    int map = version == 1 ? (int)mapByte : PbrbWorldMapFromRom(mapByte);
    if (game != (unsigned)w->game || (version == 1 && mapByte > 4) || map < 0)return 0;
    PbrbWorld n; memset(&n, 0, sizeof(n)); n.rom = w->rom; n.romSize = w->romSize; n.game = w->game;
    n.flags = ReadDword(&s); n.frames = ReadDword(&s); n.rng = ReadDword(&s);
    int x = (int)ReadByte(&s), y = (int)ReadByte(&s), facing = (int)ReadByte(&s), lock = (int)ReadByte(&s);
    n.selectedStarter = (int)ReadByte(&s); n.rivalStarter = (int)ReadByte(&s);
    unsigned actors = ReadByte(&s), reserved = ReadByte(&s);
    n.respawnMap = PBRB_MAP_DOWNSTAIRS; n.respawnX = 3; n.respawnY = 5;
    InitializeObjects(&n);
    if (version >= 2) {
        for (unsigned ei = 0; ei < PBRB_WORLD_EVENT_BYTES; ++ei)n.eventFlags[ei] = (unsigned char)ReadByte(&s);
        for (unsigned mi = 0; mi < 256; ++mi)n.hiddenObjects[mi] = (unsigned short)ReadWord(&s);
        for (unsigned ti = 0; ti < 256; ++ti)n.defeatedTrainers[ti] = (unsigned short)ReadWord(&s);
        n.lastMapRom = (int)ReadByte(&s); n.lastWarp = (int)ReadByte(&s); n.encounterCooldown = (int)ReadByte(&s);
        n.travelMode = (int)ReadByte(&s); n.fieldActive = (int)ReadByte(&s);
        n.respawnMap = PbrbWorldMapFromRom(ReadByte(&s)); n.respawnX = (int)ReadByte(&s); n.respawnY = (int)ReadByte(&s);
        n.lastOutdoorX = (int)ReadByte(&s); n.lastOutdoorY = (int)ReadByte(&s);
        n.visitedTowns = ReadWord(&s); n.serviceFlags = ReadDword(&s); n.steps = (int)ReadDword(&s);
        n.safariSteps = (int)ReadWord(&s); n.safariActive = (int)ReadByte(&s);
        for (unsigned sv = 0; sv < 32; ++sv)n.storyValues[sv] = (unsigned char)ReadByte(&s);
        if (version >= 3)for (unsigned hi = 0; hi < 8; ++hi)n.hiddenItems[hi] = (unsigned char)ReadByte(&s);
        if (n.safariSteps > 502 || n.safariActive > 1)return 0;
        WorldRawMap respawn;
        if (n.lastMapRom >= 0x25 || n.lastWarp >= PBRB_WORLD_WARPS || n.encounterCooldown > 3 || n.travelMode > 2 ||
            (n.fieldActive & ~(PBRB_WORLD_FIELD_FLASH | PBRB_WORLD_FIELD_STRENGTH)) || n.visitedTowns > 2047 || n.steps < 0 ||
            !RawMap(&n, PbrbWorldMapToRom(n.respawnMap), &respawn) || n.respawnX >= respawn.width * 2 || n.respawnY >= respawn.height * 2)return 0;
    }
    n.restoring = 1;
    if (!s.ok || reserved || n.flags > 63u || facing > 3 || lock > 1 || actors > PBRB_WORLD_ACTORS ||
        !LoadMap(&n, (int)map, x, y, facing) || actors != (unsigned)n.actorCount ||
        x >= n.widthBlocks * 2 || y >= n.heightBlocks * 2)return 0;
    if ((n.flags & WF_CHOSE_STARTER) && (BallForSpecies(n.selectedStarter) < 0 ||
        n.rivalStarter != (n.selectedStarter == 0xb0 ? 0xb1 : n.selectedStarter == 0xb1 ? 0x99 : 0xb0)))return 0;
    if (!(n.flags & WF_CHOSE_STARTER) && (n.selectedStarter || n.rivalStarter))return 0;
    if ((n.flags & WF_FOLLOWED_OAK) && !(n.flags & WF_OAK_APPEARED))return 0;
    if ((n.flags & WF_CHOSE_STARTER) && !(n.flags & WF_FOLLOWED_OAK))return 0;
    if ((n.flags & WF_RIVAL_CHOSE) && !(n.flags & WF_CHOSE_STARTER))return 0;
    if ((n.flags & WF_BATTLED) && !(n.flags & WF_RIVAL_CHOSE))return 0;
    if ((n.flags & WF_END_REPORTED) && !(n.flags & WF_BATTLED))return 0;
    for (unsigned i = 0; i < actors; ++i) {
        PbrbWorldActor* a = &n.actors[i];
        int sprite = (int)ReadByte(&s); a->x = (int)ReadByte(&s); a->y = (int)ReadByte(&s);
        int homeX = (int)ReadByte(&s), homeY = (int)ReadByte(&s); a->facing = (int)ReadByte(&s);
        int movement = (int)ReadByte(&s), range = (int)ReadByte(&s); a->visible = (int)ReadByte(&s);
        a->dx = (int)ReadByte(&s) - 1; a->dy = (int)ReadByte(&s) - 1; a->stepFrame = (int)ReadByte(&s);
        a->stepLength = (int)ReadByte(&s); a->anim = (int)ReadByte(&s); a->wait = (int)ReadWord(&s);
        if (sprite != a->sprite || homeX != a->homeX || homeY != a->homeY || movement != a->movement || range != a->range ||
            a->x >= n.widthBlocks * 2 || a->y >= n.heightBlocks * 2 || a->facing > 3 || a->visible > 1 ||
            a->dx < -1 || a->dx>1 || a->dy < -1 || a->dy>1 || (a->dx && a->dy) || a->anim > 15 || a->wait > 512 ||
            (a->stepLength != 0 && a->stepLength != 32) || (a->stepLength && a->stepFrame >= a->stepLength) ||
            (!a->stepLength && (a->dx || a->dy || a->stepFrame)) ||
            (a->stepLength && !a->dx && !a->dy) ||
            (a->stepLength && !Passable(&n, a->x + a->dx, a->y + a->dy)))return 0;
    }
    if (version >= 2) {
        n.blockChangeCount = (int)ReadByte(&s); if (n.blockChangeCount > 64)return 0;
        for (int pi = 0; pi < n.blockChangeCount; ++pi) {
            PbrbWorldBlockChange* b = &n.blockChanges[pi];
            b->x = (unsigned char)ReadByte(&s); b->y = (unsigned char)ReadByte(&s); b->block = (unsigned char)ReadByte(&s);
            if (b->x >= n.widthBlocks || b->y >= n.heightBlocks)return 0;
        }
    }
    else SyncOpeningEvents(&n);
    if (!s.ok || s.at != bytes - 4)return 0;
    n.restoring = 0; n.warpLock = lock; n.stage = WS_IDLE; Camera(&n); *w = n; return 1;
}
