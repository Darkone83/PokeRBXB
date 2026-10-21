#ifndef PBRB_GAME_H_INCLUDED
#define PBRB_GAME_H_INCLUDED
/* Native game data shared by the world, battle and menu implementations.
   IDs are the original Red/Blue internal IDs; data stays in the user's ROM. */
#include <stddef.h>
#define PBRB_PARTY_CAPACITY 6
#define PBRB_BOX_COUNT 12
#define PBRB_BOX_CAPACITY 20
#define PBRB_BAG_CAPACITY 20
#define PBRB_PC_ITEM_CAPACITY 50
struct PbrbGameRom { const unsigned char* data; size_t bytes; unsigned game; };
struct PbrbPlayMon {
    unsigned char species, level, status, moves[4], pp[4], dvs[2], nickname[11];
    unsigned hp, maxHP, attack, defense, speed, special, experience, ot;
    unsigned short statExp[5]; /* HP, Attack, Defense, Speed, Special. */
    unsigned char types[2], catchRate, originalTrainer[11];
};
struct PbrbGameMove {
    unsigned char id, effect, power, type, accuracy, pp;
};
struct PbrbGameBase {
    unsigned char dex, hp, attack, defense, speed, special;
    unsigned char types[2], catchRate, experience, growth, moves[4];
};
struct PbrbGameItem { unsigned char id, count; };
struct PbrbGameHallMon { unsigned char species, level, nickname[11]; };
struct PbrbGameHallRecord { unsigned char count; PbrbGameHallMon mons[6]; };
struct PbrbGameState {
    PbrbPlayMon party[PBRB_PARTY_CAPACITY];
    PbrbPlayMon boxes[PBRB_BOX_COUNT][PBRB_BOX_CAPACITY];
    unsigned char partyCount, boxCount[PBRB_BOX_COUNT], currentBox, active;
    PbrbGameItem bag[PBRB_BAG_CAPACITY], pcItems[PBRB_PC_ITEM_CAPACITY];
    unsigned char bagCount, pcItemCount, seen[19], owned[19];
    unsigned badges, pokedex, money, coins, repelSteps, steps;
    unsigned safariBalls, safariSteps, hallOfFameCount, hallOfFameTotal;
    unsigned char bicycle, surfing, lastHealMap, lastHealWarp;
    PbrbPlayMon daycare;
    unsigned daycareOccupied, daycareLevel, daycareSteps;
    PbrbGameHallRecord hallOfFame[50];
};
int PbrbGameValidSpecies(const PbrbGameRom*, unsigned species);
unsigned PbrbGameDex(const PbrbGameRom*, unsigned species);
unsigned PbrbGameSpecies(const PbrbGameRom*, unsigned dex);
int PbrbGameBaseData(const PbrbGameRom*, unsigned species, PbrbGameBase*);
int PbrbGameMoveData(const PbrbGameRom*, unsigned move, PbrbGameMove*);
void PbrbGameMonName(const PbrbGameRom*, unsigned species, unsigned char out[11]);
void PbrbGameMoveName(const PbrbGameRom*, unsigned move, unsigned char out[16]);
void PbrbGameItemName(const PbrbGameRom*, unsigned item, unsigned char out[16]);
unsigned PbrbGameExperience(const PbrbGameRom*, unsigned species, unsigned level);
void PbrbGameRecalculate(const PbrbGameRom*, PbrbPlayMon*, int preserveDamage);
int PbrbGameMakeMon(const PbrbGameRom*, PbrbPlayMon*, unsigned species, unsigned level,
    unsigned* rng, unsigned ot, const unsigned char* trainerName, int trainer);
unsigned PbrbGameMovesAtLevel(const PbrbGameRom*, unsigned species, unsigned level,
    unsigned char out[8]);
unsigned PbrbGameEvolution(const PbrbGameRom*, unsigned species, unsigned level,
    unsigned item, int trade);
int PbrbGameCanLearnTM(const PbrbGameRom*, unsigned species, unsigned tm);
unsigned PbrbGameTM(const PbrbGameRom*, unsigned tm);
unsigned PbrbGameMaxPP(const PbrbGameRom*, unsigned move, unsigned ppByte);
void PbrbGameHeal(const PbrbGameRom*, PbrbPlayMon*);
void PbrbGameInit(PbrbGameState*);
void PbrbGameMarkDex(const PbrbGameRom*, PbrbGameState*, unsigned species, int owned);
unsigned PbrbGameDexCount(const unsigned char bits[19]);
int PbrbGameAddMon(const PbrbGameRom*, PbrbGameState*, const PbrbPlayMon*);
/* Inventory entries are stacks, not unique item IDs. User-selected actions
   remove a specific entry; scripted exchanges may remove/count by ID. */
int PbrbGameAddItem(PbrbGameItem*, unsigned char*, unsigned capacity, unsigned item, unsigned count);
int PbrbGameRemoveItemAt(PbrbGameItem*, unsigned char*, unsigned slot, unsigned count);
int PbrbGameRemoveItem(PbrbGameItem*, unsigned char*, unsigned item, unsigned count);
unsigned PbrbGameCountItem(const PbrbGameItem*, unsigned count, unsigned item);
int PbrbGameSwapItems(PbrbGameItem*, unsigned char*, unsigned first, unsigned second);
int PbrbGameIsKeyItem(const PbrbGameRom*, unsigned item);
unsigned PbrbGameItemPrice(const PbrbGameRom*, unsigned item);
int PbrbGameUseMedicine(const PbrbGameRom*, PbrbPlayMon*, unsigned item, unsigned moveSlot);
int PbrbGameValidateMon(const PbrbGameRom*, const PbrbPlayMon*);
size_t PbrbGameSerialize(const PbrbGameState*, unsigned char*, size_t);
int PbrbGameDeserialize(const PbrbGameRom*, PbrbGameState*, const unsigned char*, size_t);
#endif
