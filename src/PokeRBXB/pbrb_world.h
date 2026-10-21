#ifndef PBRB_WORLD_H
#define PBRB_WORLD_H
/* Native overworld. Original map, collision, sprite and text bytes
   remain in the verified user's ROM; no copied artwork or project changes. */
#include <stddef.h>

#define PBRB_VIEW_W 160
#define PBRB_VIEW_H 144
#define PBRB_ROM_SIZE 1048576u
#define PBRB_WORLD_ACTORS 16
#define PBRB_WORLD_PATH 64
#define PBRB_WORLD_SAVE_MAX 4096u
#define PBRB_WORLD_MAP_BASE 0x100
#define PBRB_WORLD_EVENT_BYTES 320
#define PBRB_WORLD_WARPS 32
#define PBRB_WORLD_SIGNS 32
#define PBRB_WORLD_MUSIC_ROM_FLAG 0x10000

enum PbrbWorldMap {
    PBRB_MAP_BEDROOM = 0, PBRB_MAP_PALLET = 1, PBRB_MAP_DOWNSTAIRS = 2,
    PBRB_MAP_LAB = 3, PBRB_MAP_BLUE_HOUSE = 4
};
enum PbrbWorldEventKind {
    PBRB_WORLD_NONE = 0, PBRB_WORLD_TEXT, PBRB_WORLD_STARTER_CHOICE,
    PBRB_WORLD_GIVE_STARTER, PBRB_WORLD_BATTLE, PBRB_WORLD_MUSIC,
    PBRB_WORLD_SFX, PBRB_WORLD_PLAYTEST_END, PBRB_WORLD_HEAL,
    PBRB_WORLD_PC, PBRB_WORLD_TOWN_MAP,
    PBRB_WORLD_GIVE_ITEM, PBRB_WORLD_TAKE_ITEM, PBRB_WORLD_SHOP,
    PBRB_WORLD_POKEDEX, PBRB_WORLD_BADGE, PBRB_WORLD_CHOICE,
    PBRB_WORLD_BLACKOUT, PBRB_WORLD_UNSUPPORTED,
    PBRB_WORLD_GIVE_MON, PBRB_WORLD_TAKE_MONEY, PBRB_WORLD_TRADE,
    PBRB_WORLD_DAYCARE, PBRB_WORLD_SAFARI, PBRB_WORLD_GAME_CORNER,
    PBRB_WORLD_PRIZE_MENU, PBRB_WORLD_HALL_OF_FAME, PBRB_WORLD_DEX_RATING,
    PBRB_WORLD_GIVE_COINS, PBRB_WORLD_TAKE_COINS, PBRB_WORLD_ELEVATOR,
    PBRB_WORLD_VENDING, PBRB_WORLD_ITEM_CHOICE,
    PBRB_WORLD_NAME_RATER, PBRB_WORLD_PICTURE
};
enum PbrbWorldEventFlags {
    PBRB_WORLD_AUTO_CLOSE = 1, PBRB_WORLD_KEEP_TEXT = 2,
    PBRB_WORLD_WAIT_SOUND = 4, PBRB_WORLD_WAIT_BUTTON = 8,
    PBRB_WORLD_CONTINUE_TEXT = 0x10,
    PBRB_WORLD_TRAINER = 0x100, PBRB_WORLD_WILD = 0x200,
    PBRB_WORLD_STATIC = 0x400, PBRB_WORLD_CATCH_DEMO = 0x800,
    PBRB_WORLD_FIRST_RIVAL = 0x1000, PBRB_WORLD_SAFARI_BATTLE = 0x2000,
    PBRB_WORLD_GHOST = 0x4000, PBRB_WORLD_NO_CATCH = 0x8000,
    PBRB_WORLD_FADE_MUSIC = 0x10000
};
enum PbrbWorldBattleResult {
    PBRB_WORLD_BATTLE_LOST = 0, PBRB_WORLD_BATTLE_WON = 1,
    PBRB_WORLD_BATTLE_CAUGHT = 2, PBRB_WORLD_BATTLE_FLED = 3
};
enum PbrbWorldFieldMove {
    PBRB_WORLD_FIELD_CUT = 1, PBRB_WORLD_FIELD_SURF = 2,
    PBRB_WORLD_FIELD_STRENGTH = 4, PBRB_WORLD_FIELD_FLASH = 8,
    PBRB_WORLD_FIELD_FLY = 16, PBRB_WORLD_FIELD_TELEPORT = 32,
    PBRB_WORLD_FIELD_DIG = 64
};
/* TEXT offsets point directly at original _Text labels. value is optional
   internal species for name-buffer substitution. AUTO_CLOSE skips the final
   DisplayTextID wait (an original 0x58 prompt still waits); KEEP_TEXT retains
   the box for a following sound. WAIT_SOUND keeps SFX pending until playback
   completes; WAIT_BUTTON then requires a fresh A/B press before closing it.
   Acknowledge TEXT with 0,
   STARTER_CHOICE with 1 for Yes / 0 for No, GIVE_STARTER after party/naming,
   BATTLE after battle using PbrbWorldBattleResult. GIVE_ITEM/TAKE_ITEM take
   result 1 on success / 0 on inventory failure; CHOICE takes 1 Yes / 0 No.
   trainerClass is the original 1-based class (RIVAL1=25); trainerId is its
   original party-table number. SHOP dataOffset addresses its first ROM item,
   dataCount excludes the terminator. World owns quest flags; the caller owns
   party, bag, money, badges and Pokedex and commits those before acknowledgment.
   MUSIC/SFX use pbrb_audio_core.h IDs. FADE_MUSIC uses dataCount as the
   frames-per-volume-step; FADE_MUSIC|WAIT_SOUND waits for a fade to NONE.
   For SFX,
   only unflagged sounds may be acknowledged immediately. Events remain pending
   until acknowledged. Saveable states never contain a pending event. */
struct PbrbWorldEvent {
    int kind; unsigned textOffset; int value; unsigned flags;
    unsigned species, level, trainerClass, trainerId, itemId, quantity;
    unsigned dataOffset, dataCount, winTextOffset, loseTextOffset, reward;
    int objectId;
};
/* Refreshed by the gameplay owner before Tick; these are observations, not a
   second inventory/save copy. keyItems uses the original item ID as bit index. */
struct PbrbWorldPlayerState {
    unsigned badges, pokedex, partyCount, usableCount, leadLevel, ownedCount, money;
    unsigned repelSteps, fieldMoves, surfing, bicycle, safariBalls;
    unsigned char keyItems[32];
};
struct PbrbWorldActor {
    int sprite, x, y, homeX, homeY, facing, movement, range, visible;
    int dx, dy, stepFrame, stepLength, anim, wait;
    unsigned textId, objectFlags, trainerClass, trainerId, itemId;
};
struct PbrbWorldWarp { unsigned char x, y, destination, warp; };
struct PbrbWorldSign { unsigned char x, y, textId; };
struct PbrbWorldConnection { int map, offset, width, height; unsigned blocks; };
struct PbrbWorldBlockChange { unsigned char x, y, block; };
/* Frame-local field presentation. Field actions cannot be saved mid-action;
   these fields are deliberately absent from the serialized world format. */
struct PbrbWorldFieldAnimation {
    int kind, frame, facing, travel;
    int targetMap, targetX, targetY;
};
struct PbrbWorld {
    const unsigned char* rom;
    size_t romSize;
    int game, mapId, widthBlocks, heightBlocks;
    unsigned mapOffset, tilesOffset, blocksOffset, numTiles, collisionOffset;
    int borderBlock, grassTile, counterTile;
    int heroX, heroY, cameraX, cameraY, facing;
    int heroDx, heroDy, heroStepFrame, heroStepLength, heroAnim;
    int turnDelay, wasMoving, warpLock;
    PbrbWorldActor actors[PBRB_WORLD_ACTORS];
    int actorCount;
    unsigned flags, frames, rng;
    int stage, delay, selectedStarter, rivalStarter, interactionActor, interactionFacing;
    unsigned char heroPath[PBRB_WORLD_PATH], actorPath[PBRB_WORLD_PATH];
    int heroPathCount, heroPathIndex, actorPathCount, actorPathIndex, pathActor;
    int transitionMap, transitionX, transitionY, transitionFacing, transitionStage;
    int transitionFrame, transitionMusic, transitionExit;
    PbrbWorldEvent event;
    int romMapId, tileset, mapBank, lastMapRom, lastWarp;
    unsigned headerOffset, objectOffset, textPointersOffset, scriptOffset;
    PbrbWorldWarp warps[PBRB_WORLD_WARPS];
    PbrbWorldSign signs[PBRB_WORLD_SIGNS];
    int warpCount, signCount;
    PbrbWorldConnection connections[4];
    PbrbWorldPlayerState player;
    unsigned char eventFlags[PBRB_WORLD_EVENT_BYTES];
    unsigned short hiddenObjects[256];
    unsigned short defeatedTrainers[256];
    int scriptId, scriptStep, scriptValue, scriptObject, scriptResult, battleReturn;
    int encounterCooldown, steps, ledge, respawnMap, respawnX, respawnY;
    int heroForced, travelMode, fieldActive, lastOutdoorX, lastOutdoorY;
    unsigned visitedTowns, serviceFlags;
    PbrbWorldBlockChange blockChanges[64];
    int blockChangeCount;
    int safariSteps, safariActive;
    unsigned char storyValues[32];
    unsigned char hiddenItems[8];
    int previousMapRom, arrivalWarp, restoring;
    PbrbWorldFieldAnimation fieldAnimation;
};
struct PbrbGb;

int PbrbWorldInit(PbrbWorld* world, const unsigned char* rom, size_t size, int mapId);
int PbrbWorldSelectMap(PbrbWorld* world, int mapId);
/* Legacy local IDs 0..4 are stable. All other maps use MAP_BASE+original ID. */
int PbrbWorldMapFromRom(unsigned romMapId);
int PbrbWorldMapToRom(int mapId);
void PbrbWorldSetPlayerState(PbrbWorld*, const PbrbWorldPlayerState*);
int PbrbWorldCheckEvent(const PbrbWorld*, unsigned originalEventBit);
void PbrbWorldSetEvent(PbrbWorld*, unsigned originalEventBit, int enabled);
/* These actions run through the same frame/event state machine. The caller
   applies blackout healing/money loss once before Blackout. Field capabilities
   and key-item ownership must be refreshed with SetPlayerState first. */
void PbrbWorldBlackout(PbrbWorld*);
int PbrbWorldUseFieldMove(PbrbWorld*, unsigned fieldMove);
int PbrbWorldUseItem(PbrbWorld*, unsigned itemId);
int PbrbWorldFlyTo(PbrbWorld*, unsigned originalTownId);
/* Call once per original Game Boy frame (70224 / 4194304 seconds).
   Held/pressed bits: Right 1, Left 2, Up 4, Down 8, A16, B32, Select64, Start128. */
void PbrbWorldTick(PbrbWorld* world, unsigned held, unsigned pressed);
const PbrbWorldEvent* PbrbWorldPendingEvent(const PbrbWorld* world);
void PbrbWorldAcknowledge(PbrbWorld* world, int result);
void PbrbWorldRender(const PbrbWorld* world, PbrbGb* gb);
void PbrbWorldRenderBGRA(const PbrbWorld* world, unsigned char* outBGRA, size_t bytes);
/* Compatibility helper initiates one collision-checked step; Tick animates it. */
int PbrbWorldMove(PbrbWorld* world, int deltaX, int deltaY);
const char* PbrbWorldMapName(const PbrbWorld* world);
int PbrbWorldCanSave(const PbrbWorld* world);
/* Explicit little-endian, versioned, bounded fields, never a pointer/struct dump.
   Read validates transactionally against the already attached ROM/game. */
size_t PbrbWorldSerialize(const PbrbWorld* world, unsigned char* out, size_t bytes);
int PbrbWorldDeserialize(PbrbWorld* world, const unsigned char* data, size_t bytes);
int PbrbWorldMusic(const PbrbWorld* world);
/* Completed hero half-block, using the original grass encounter tile. */
int PbrbWorldInGrass(const PbrbWorld* world);
int PbrbWorldHasMetBill(const PbrbWorld* world);
#endif
