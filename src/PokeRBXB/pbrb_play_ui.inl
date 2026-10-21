/* Included once by pbrb_play.inl. Native render translations of
   engine/events/starter_dex.asm, engine/menus/pokedex.asm and
   engine/items/town_map.asm from the matching US/EU Red/Blue builds.
   These are ROM offsets and layout rules, never copied tiles or text. */
#include "pbrb_gb.h"
#include <string.h>

enum {
    PPLAY_UI_DEX_TILES = 0x12488, PPLAY_UI_MAP_TILES = 0x125a8,
    PPLAY_UI_BATTLE_FONT = 0x11ea0, PPLAY_UI_MON_NAMES = 0x1c21e,
    PPLAY_UI_DEX_ORDER = 0x41024, PPLAY_UI_DEX_POINTERS = 0x4047e,
    PPLAY_UI_HEIGHT_WEIGHT = 0x40448, PPLAY_UI_DEX_DIVIDER = 0x4045f,
    PPLAY_UI_MAP_ORDER = 0x70f11, PPLAY_UI_MAP_CURSOR = 0x70f40,
    PPLAY_UI_MAP_RLE = 0x71100, PPLAY_UI_MAP_EXTERNAL = 0x71313,
    PPLAY_UI_MAP_INTERNAL = 0x71382, PPLAY_UI_RED_SPRITE = 0x14180,
    PPLAY_UI_WILD_POINTERS = 0xceeb, PPLAY_UI_NEST_ICON = 0x716be,
    PPLAY_UI_NEST_TEXT = 0x70f89, PPLAY_UI_AREA_UNKNOWN = 0x7124a,
    PBRB_PLAY_TOWN_MAP_LOCATIONS = 47
};
static int PplayUIFits(const PbrbGb* g, unsigned offset, unsigned count) {
    return g && g->rom && offset <= g->bytes && count <= g->bytes - offset;
}
static unsigned PplayUIWord(const unsigned char* b) {
    return (unsigned)b[0] | ((unsigned)b[1] << 8);
}
static unsigned PplayUIAddress(unsigned bank, unsigned address) {
    if (address < 0x4000u)return address;
    if (address >= 0x8000u || bank >= 64u)return 0;
    return bank * 0x4000u + address - 0x4000u;
}
static void PplayUIGlyph(PbrbGb* g, unsigned c, int x, int y, int dex) {
    /* Pokedex replaces $60..$71 while retaining the battle-font number
       marker at $74. Other text continues to use the original font bank. */
    if (dex && c >= 0x60u && c <= 0x71u)
        PbrbGbTile(g, PPLAY_UI_DEX_TILES + (c - 0x60u) * 16u, 2, x, y);
    else if (dex && c == 0x74u)
        PbrbGbTile(g, PPLAY_UI_BATTLE_FONT + (c - 0x62u) * 16u, 2, x, y);
    else PbrbGbGlyph(g, (unsigned char)c, x, y);
}
static unsigned PplayUIString(PbrbGb* g, unsigned at, int x, int y, int dex, unsigned limit) {
    const int firstX = x;
    for (unsigned n = 0; n < limit && PplayUIFits(g, at, 1u); ++n) {
        unsigned c = g->rom[at++];
        if (c == 0x50u)return at;
        if (c == 0x4eu) { x = firstX; y += 16; continue; }
        if (c == 0x54u) {
            const unsigned char expansion[4] = { 0x8f,0x8e,0x8a,0xba };
            for (int i = 0; i < 4; ++i) { PplayUIGlyph(g, expansion[i], x, y, dex); x += 8; }
            continue;
        }
        if (c == 0x4au) {
            PplayUIGlyph(g, 0xe1, x, y, dex); PplayUIGlyph(g, 0xe2, x + 8, y, dex); x += 16;
            continue;
        }
        if (c < 0x60u)return 0;
        PplayUIGlyph(g, c, x, y, dex); x += 8;
    }
    return 0;
}
static void PplayUINumber(PbrbGb* g, unsigned number, int digits, int x, int y, int zeroes) {
    unsigned divisor = 1;
    for (int i = 1; i < digits; ++i)divisor *= 10u;
    int started = zeroes;
    for (int i = 0; i < digits; ++i) {
        const unsigned d = (number / divisor) % 10u;
        if (d || i == digits - 1)started = 1;
        PbrbGbGlyph(g, (unsigned char)(started ? 0xf6u + d : 0x7fu), x + i * 8, y);
        divisor /= 10u;
    }
}
static void PplayUIDexDescription(PbrbGb* g, unsigned at, int wantedPage) {
    int page = 0, x = 8, y = 88;
    for (unsigned budget = 0; budget < 512u && PplayUIFits(g, at, 1u); ++budget) {
        const unsigned c = g->rom[at++];
        if (c == 0u)continue; /* TX_START */
        if (c == 0x50u)return;
        if (c == 0x49u) {
            if (page == wantedPage) { PbrbGbGlyph(g, 0xee, 144, 128); return; }
            ++page; x = 8; y = 88; continue;
        }
        if (c == 0x4eu) { x = 8; y += 16; continue; }
        if (c == 0x5fu) { if (page == wantedPage)PbrbGbGlyph(g, 0xe8, x, y); return; }
        if (c == 0x54u) {
            const unsigned char expansion[4] = { 0x8f,0x8e,0x8a,0xba }; /* # = POKe-accent */
            for (int i = 0; i < 4; ++i) { if (page == wantedPage)PbrbGbGlyph(g, expansion[i], x, y); x += 8; }
            continue;
        }
        if (c < 0x60u)return;
        if (page == wantedPage)PbrbGbGlyph(g, (unsigned char)c, x, y);
        x += 8;
    }
}
static void PbrbPlayDrawDexEntry(PbrbGb* g, unsigned species, int page, int owned) {
    if (!g || !g->pixels)return;
    PbrbGbSetPalettes(g, 0xe4u, 0xd0u, 0xe4u); PbrbGbClear(g, 0);
    if (species < 1u || species>190u ||
        !PplayUIFits(g, PPLAY_UI_DEX_TILES, 18u * 16u) ||
        !PplayUIFits(g, PPLAY_UI_HEIGHT_WEIGHT, 22u) ||
        !PplayUIFits(g, PPLAY_UI_DEX_DIVIDER, 20u) ||
        !PplayUIFits(g, PPLAY_UI_DEX_ORDER + species - 1u, 1u) ||
        !PplayUIFits(g, PPLAY_UI_DEX_POINTERS + (species - 1u) * 2u, 2u) ||
        !PplayUIFits(g, PPLAY_UI_MON_NAMES + (species - 1u) * 10u, 10u))return;
    for (int x = 0; x < 20; ++x) { PplayUIGlyph(g, 0x64, x * 8, 0, 1); PplayUIGlyph(g, 0x6f, x * 8, 136, 1); }
    for (int y = 1; y < 17; ++y) { PplayUIGlyph(g, 0x66, 0, y * 8, 1); PplayUIGlyph(g, 0x67, 152, y * 8, 1); }
    PplayUIGlyph(g, 0x63, 0, 0, 1); PplayUIGlyph(g, 0x65, 152, 0, 1);
    PplayUIGlyph(g, 0x6c, 0, 136, 1); PplayUIGlyph(g, 0x6e, 152, 136, 1);
    for (unsigned x = 0; x < 20u; ++x)PplayUIGlyph(g, g->rom[PPLAY_UI_DEX_DIVIDER + x], (int)x * 8, 72, 1);
    PplayUIString(g, PPLAY_UI_HEIGHT_WEIGHT, 72, 48, 1, 22);
    PplayUIString(g, PPLAY_UI_MON_NAMES + (species - 1u) * 10u, 72, 16, 1, 10);
    const unsigned ptr = PplayUIWord(g->rom + PPLAY_UI_DEX_POINTERS + (species - 1u) * 2u);
    const unsigned entry = PplayUIAddress(16u, ptr);
    if (!entry)return;
    const unsigned stats = PplayUIString(g, entry, 72, 32, 1, 12);
    if (!stats || !PplayUIFits(g, stats, 8u))return;
    PplayUIGlyph(g, 0x74, 16, 64, 1); PplayUIGlyph(g, 0xf2, 24, 64, 1);
    PplayUINumber(g, g->rom[PPLAY_UI_DEX_ORDER + species - 1u], 3, 32, 64, 1);
    if (owned) {
        PplayUINumber(g, g->rom[stats], 2, 96, 48, 0); PplayUIGlyph(g, 0x60, 112, 48, 1);
        PplayUINumber(g, g->rom[stats + 1u], 2, 120, 48, 1); PplayUIGlyph(g, 0x61, 136, 48, 1);
        const unsigned weight = PplayUIWord(g->rom + stats + 2u);
        PplayUINumber(g, weight / 10u, 4, 88, 64, 0); PplayUIGlyph(g, 0xf2, 120, 64, 1);
        PplayUINumber(g, weight % 10u, 1, 128, 64, 1);
    }
    /* The original front picture is flipped and occupies the 7x7 tile
       canvas at (1,1); padding/centering comes from home/pics.asm. */
    unsigned char picture[56 * 56];
    if (PbrbGbDecodePokemon(g, species, 0, picture, sizeof(picture)))
        PbrbGbBitmap(g, picture, 56, 56, 8, 8, 0, 1);
    if (!owned || g->rom[stats + 4u] != 0x17u)return; /* original TX_FAR */
    const unsigned text = PplayUIAddress(g->rom[stats + 7u], PplayUIWord(g->rom + stats + 5u));
    if (text)PplayUIDexDescription(g, text, page > 0 ? 1 : 0);
}
void PbrbPlayDrawStarterDex(PbrbGb* g, unsigned species, int page) {
    PbrbPlayDrawDexEntry(g, species, page, 1);
}

static int PplayUIMapEntry(PbrbGb* g, unsigned map, unsigned* coords, unsigned* name) {
    unsigned at = PPLAY_UI_MAP_EXTERNAL + map * 3u;
    if (map >= 37u) {
        at = PPLAY_UI_MAP_INTERNAL;
        for (unsigned i = 0; i < 64u; ++i, at += 4u) {
            if (!PplayUIFits(g, at, 4u))return 0;
            if (map < g->rom[at]) { ++at; break; }
            if (i == 63u)return 0;
        }
    }
    if (!PplayUIFits(g, at, 3u))return 0;
    *coords = g->rom[at]; *name = PplayUIAddress(28u, PplayUIWord(g->rom + at + 1u));
    return *name && PplayUIFits(g, *name, 1u);
}
static void PplayUIMapSprite(PbrbGb* g, unsigned tileBase, int oneBit, unsigned coords) {
    /* WriteTownMapSpriteOAM subtracts four from x, three from y because
       of the low-byte carry; then the LCD subtracts OAM's (8,16) offset. */
    const int x = (int)(coords & 15u) * 8 + 12, y = (int)(coords >> 4) * 8 + 5;
    for (unsigned tile = 0; tile < 4u; ++tile) {
        const int tx = x + (int)(tile & 1u) * 8, ty = y + (int)(tile >> 1) * 8;
        if (!oneBit)PbrbGbSpriteTile(g, tileBase + tile * 16u, tx, ty, 0xd0u, 0);
        else {
            const unsigned off = tileBase + tile * 8u;
            if (!PplayUIFits(g, off, 8u))return;
            for (int yy = 0; yy < 8; ++yy)for (int xx = 0; xx < 8; ++xx)
                if (g->rom[off + (unsigned)yy] & (0x80u >> xx))PbrbGbPixel(g, tx + xx, ty + yy, 3u, 1u);
        }
    }
}
static int PplayUIMapBackground(PbrbGb* g) {
    if (!g || !g->pixels)return 0;
    PbrbGbSetPalettes(g, 0xe4u, 0xd0u, 0xe4u); PbrbGbClear(g, 0);
    if (!PplayUIFits(g, PPLAY_UI_MAP_TILES, 16u * 16u) ||
        !PplayUIFits(g, PPLAY_UI_MAP_ORDER, PBRB_PLAY_TOWN_MAP_LOCATIONS))return 0;
    /* CompressedMap is nybble tile/run RLE. It writes the complete 20x18
       map into the background tilemap; no recreated map artwork is used. */
    unsigned at = PPLAY_UI_MAP_RLE, position = 0;
    for (unsigned n = 0; n < 360u && PplayUIFits(g, at, 1u); ++n) {
        const unsigned code = g->rom[at++]; if (!code)break;
        const unsigned run = code & 15u, tile = code >> 4;
        if (!run || position + run > 360u)return 0;
        for (unsigned i = 0; i < run; ++i, ++position)
            PbrbGbTile(g, PPLAY_UI_MAP_TILES + tile * 16u, 2, (int)(position % 20u) * 8, (int)(position / 20u) * 8);
    }
    return position == 360u;
}
void PbrbPlayDrawTownMap(PbrbGb* g, int selectedLocation, unsigned frame, unsigned playerMap = 0) {
    if (!PplayUIMapBackground(g))return;
    unsigned map = playerMap;
    if (selectedLocation >= 0) {
        selectedLocation %= PBRB_PLAY_TOWN_MAP_LOCATIONS;
        map = g->rom[PPLAY_UI_MAP_ORDER + (unsigned)selectedLocation];
    }
    unsigned coords = 0, name = 0, playerCoords = 0, playerName = 0;
    if (!PplayUIMapEntry(g, map, &coords, &name) || !PplayUIMapEntry(g, playerMap, &playerCoords, &playerName))return;
    for (int x = 0; x < 20; ++x)PbrbGbGlyph(g, 0x7f, x * 8, 0);
    PplayUIString(g, name, 8, 0, 0, 19);
    /* Player marker is never included in the 36 blinking OAM entries.
       The selected-location cursor is drawn later, as its OAM priority is
       ahead of the player when the two markers overlap. */
    PplayUIMapSprite(g, PPLAY_UI_RED_SPRITE, 0, playerCoords);
    if (frame % 50u < 25u)PplayUIMapSprite(g, PPLAY_UI_MAP_CURSOR, 1, coords);
}

static unsigned PplayUIWildLocations(PbrbGb* g, unsigned species, unsigned char* coords, unsigned capacity) {
    /* FindWildLocationsOfMon checks only land/water slots. Each map survives
       ZeroOutDuplicatesInList once; different floors may share coordinates.
       The original deliberately excludes Cerulean Cave and does not add
       fishing, gifts, trades or static encounters to the nest display. */
    unsigned count = 0;
    for (unsigned map = 0; map < 256u; ++map) {
        unsigned at = PPLAY_UI_WILD_POINTERS + map * 2u; if (!PplayUIFits(g, at, 2u))break;
        if (g->rom[at + 1u] == 255u)break;
        at = PplayUIAddress(3u, PplayUIWord(g->rom + at)); if (!at)continue;
        int found = 0;
        for (unsigned habitat = 0; habitat < 2u; ++habitat) {
            if (!PplayUIFits(g, at, 1u))break;
            if (g->rom[at++]) {
                if (!PplayUIFits(g, at, 20u))break;
                for (unsigned slot = 0; slot < 10u; ++slot)if (g->rom[at + slot * 2u + 1u] == species)found = 1;
                at += 20u;
            }
        }
        if (found && map) {
            unsigned coordinate = 0, name = 0;
            if (PplayUIMapEntry(g, map, &coordinate, &name) && coordinate != 0x19u && count < capacity)
                coords[count++] = (unsigned char)coordinate;
        }
    }
    return count;
}
struct PplayUIMapObject { unsigned offset; int x, y, oneBit; };
static void PplayUIMapObjects(PbrbGb* g, const PplayUIMapObject* objects, unsigned count) {
    /* The one-bit nest icon and two-bit player share original DMG OAM
       priority: ten objects per scanline, lowest X then original OAM order. */
    for (int y = 0; y < 144; ++y) {
        unsigned selected[10], used = 0;
        for (unsigned i = 0; i < count && used < 10u; ++i)if (y >= objects[i].y && y < objects[i].y + 8)selected[used++] = i;
        for (int x = 0; x < 160; ++x) {
            int bestX = 256; unsigned color = 0;
            for (unsigned i = 0; i < used; ++i) {
                const PplayUIMapObject* o = objects + selected[i];
                if (x < o->x || x >= o->x + 8 || o->x >= bestX)continue;
                unsigned row = (unsigned)(y - o->y), at = o->offset + row * (o->oneBit ? 1u : 2u);
                if (!PplayUIFits(g, at, o->oneBit ? 1u : 2u))continue;
                unsigned shift = 7u - (unsigned)(x - o->x), c = (g->rom[at] >> shift) & 1u;
                c = o->oneBit ? c * 3u : c | (((g->rom[at + 1u] >> shift) & 1u) << 1);
                if (c) { bestX = o->x; color = c; }
            }
            if (color)PbrbGbPixel(g, x, y, color, 1u);
        }
    }
}
static void PbrbPlayDrawDexArea(PbrbGb* g, unsigned species, unsigned playerMap, unsigned frame) {
    if (!PplayUIMapBackground(g) || species < 1u || species>190u)return;
    unsigned char coords[36]; unsigned count = PplayUIWildLocations(g, species, coords, 36);
    unsigned name = PPLAY_UI_MON_NAMES + (species - 1u) * 10u, length = 0;
    if (!PplayUIFits(g, name, 10u))return;
    for (int x = 0; x < 20; ++x)PbrbGbGlyph(g, 0x7f, x * 8, 0);
    while (length < 10u && g->rom[name + length] != 0x50u) { PbrbGbGlyph(g, g->rom[name + length], 8 + (int)length * 8, 0); ++length; }
    PplayUIString(g, PPLAY_UI_NEST_TEXT, 8 + (int)length * 8, 0, 0, 8);
    if (!count) { PbrbGbTextBox(g, 8, 56, 17, 4); PplayUIString(g, PPLAY_UI_AREA_UNKNOWN, 16, 72, 0, 16); return; }
    PplayUIMapObject objects[40]; unsigned used = 0;
    if (frame % 50u < 25u)for (unsigned i = 0; i < count; ++i) {
        PplayUIMapObject* o = objects + used++;
        o->offset = PPLAY_UI_NEST_ICON; o->x = (int)(coords[i] & 15u) * 8 + 16; o->y = (int)(coords[i] >> 4) * 8 + 8; o->oneBit = 1;
    }
    unsigned playerCoords = 0, playerName = 0;
    if (PplayUIMapEntry(g, playerMap, &playerCoords, &playerName))for (unsigned i = 0; i < 4u; ++i) {
        PplayUIMapObject* o = objects + used++;
        o->offset = PPLAY_UI_RED_SPRITE + i * 16u; o->x = (int)(playerCoords & 15u) * 8 + 12 + (int)(i & 1u) * 8;
        o->y = (int)(playerCoords >> 4) * 8 + 5 + (int)(i >> 1) * 8; o->oneBit = 0;
    }
    PplayUIMapObjects(g, objects, used);
}
