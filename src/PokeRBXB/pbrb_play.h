#ifndef PBRB_PLAY_H
#define PBRB_PLAY_H
/* Portable native gameplay state. Implementation is included once by
   pbrb_engine.cpp so existing RXDK project file lists need no additions. */
#include "pbrb_world.h"
#include "pbrb_gb.h"
#include "pbrb_game.h"
#include "pbrb_battle.h"
#include "pbrb_battle_anim.h"
#define PBRB_PLAY_SAVE_MAX 65536u
#define PBRB_PLAY_TEXT_MAX 2048u
enum PbrbPlayScene { PBRB_PLAY_OAK, PBRB_PLAY_WORLD, PBRB_PLAY_BATTLE, PBRB_PLAY_CREDITS };
enum PbrbPlayOverlay {
    PBRB_OVERLAY_NONE, PBRB_OVERLAY_NAME_LIST, PBRB_OVERLAY_NAMING,
    PBRB_OVERLAY_START, PBRB_OVERLAY_OPTIONS, PBRB_OVERLAY_YESNO,
    PBRB_OVERLAY_PARTY, PBRB_OVERLAY_ITEM, PBRB_OVERLAY_TRAINER,
    PBRB_OVERLAY_PC, PBRB_OVERLAY_NOTICE, PBRB_OVERLAY_EXIT,
    PBRB_OVERLAY_CONTINUE, PBRB_OVERLAY_MOVES,
    PBRB_OVERLAY_STARTER_DEX, PBRB_OVERLAY_TOWN_MAP,
    PBRB_OVERLAY_PARTY_ACTION, PBRB_OVERLAY_STATUS, PBRB_OVERLAY_ITEM_ACTION,
    PBRB_OVERLAY_ITEM_TARGET, PBRB_OVERLAY_ITEM_MOVES, PBRB_OVERLAY_SHOP,
    PBRB_OVERLAY_SHOP_LIST, PBRB_OVERLAY_QUANTITY, PBRB_OVERLAY_POKEDEX,
    PBRB_OVERLAY_DEX_ENTRY, PBRB_OVERLAY_PC_MAIN, PBRB_OVERLAY_PC_MONS,
    PBRB_OVERLAY_PC_ITEMS, PBRB_OVERLAY_BOX, PBRB_OVERLAY_LEARN,
    PBRB_OVERLAY_EVOLUTION, PBRB_OVERLAY_FIELD_MOVE, PBRB_OVERLAY_SERVICE,
    PBRB_OVERLAY_MIMIC, PBRB_OVERLAY_PROGRESS, PBRB_OVERLAY_SOFTBOILED,
    PBRB_OVERLAY_POKEDEX_SIDE, PBRB_OVERLAY_DEX_AREA,
    PBRB_OVERLAY_PC_MON_ACTION, PBRB_OVERLAY_PC_MON_STATUS, PBRB_OVERLAY_MENU_CONFIRM
};
struct PbrbPlayText {
    unsigned char tokens[PBRB_PLAY_TEXT_MAX], cells[4][18];
    unsigned count, position;
    int active, visible, waiting, row, column, delay, scroll, forceWait, after;
};
struct PbrbPlay {
    PbrbWorld* world;
    PbrbGb* gb;
    unsigned game, frames, rng, trainerID, money;
    unsigned uiFrames; /* animation clock remains live after the saved play-time cap */
    unsigned char player[11], rival[11], nameBuffer[16];
    unsigned char picture[56 * 56], enemyPicture[56 * 56], backPicture[56 * 56];
    PbrbPlayMon starter, enemy;
    PbrbGameState gameState;
    PbrbBattle battle;
    PbrbBattleEvent battleEvent;
    PbrbBattleAnim battleAnim;
    PbrbWorldEvent encounter;
    unsigned char trainerName[16], stringBuffer[32];
    unsigned textNumber, textLevel, textMoney, lastWorldSteps;
    int textUser, textTarget, battleEventActive, battleEventPhase;
    int battleWait, battleVisual, battleVisualSide, battleVisualTime;
    int battleFinished, battleFinishPhase, battleHpTick, battleLearnParty;
    unsigned battleHidden, battleDisplaySpecies[2], battlePictureFlags[2], battleBallItem;
    unsigned battleLastSwitchEnemyHP;
    int battleAlarmDisabled;
    int menuMode, menuPage, menuTarget, menuItem, menuCount, menuReturn;
    int menuCursor, menuSwap, menuSubSelection, pendingMove, pendingParty;
    int itemSwap, moveSwap, itemSlot;
    int learnReturn, evolutionParty, evolutionSpecies, evolutionPhase;
    unsigned char evolutionLevels[6];
    unsigned char pendingMoves[8], pendingMoveCount, pendingMoveIndex;
    int levelReturn, levelCheckEvolution, evolutionReturn;
    int serviceStep, serviceSelection, serviceValue, serviceCost, serviceReturn;
    PbrbWorldEvent serviceEvent, dialogueEvent;
    unsigned serviceTimer;
    unsigned char serviceData[64];
    int endingPending;
    unsigned creditsAt, creditsTexts[5], creditsMon, creditsLines;
    int creditsPhase, creditsTimer, creditsElapsed, creditsCommand, creditsCopyright;
    PbrbPlayText text;
    int scene, overlay, selection, savedSelection, optionSelection;
    int textSpeed, battleAnimation, battleStyle, pcPotion, bagPotion;
    int oakStep, timer, pictureId, pictureX, pictureFlip, fade, pictureMotion;
    int namingWho, namingRow, namingColumn, namingCase, namingLength;
    unsigned char naming[11];
    int question, questionValue, worldEvent, worldSoundPhase, battleStep, battleSelection;
    int endpoint, exitRequested, hadSave, saveInfo, savePending;
    int noticeReturn, exitReturn, exitSelection, textError;
    unsigned repeatHeld, repeatFrames;
};
void PbrbPlayInit(PbrbPlay*, PbrbWorld*, PbrbGb*, unsigned game);
void PbrbPlayNew(PbrbPlay*, int textSpeed, int battleAnimation, int battleStyle, unsigned seed);
void PbrbPlayTick(PbrbPlay*, unsigned held, unsigned pressed);
void PbrbPlayRender(PbrbPlay*);
int PbrbPlayCanSave(const PbrbPlay*);
size_t PbrbPlaySerialize(const PbrbPlay*, unsigned char*, size_t);
int PbrbPlayDeserialize(PbrbPlay*, const unsigned char*, size_t);
int PbrbPlayLoad(PbrbPlay*);
void PbrbPlayRequestExit(PbrbPlay*);
#endif
