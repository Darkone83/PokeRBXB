#ifndef PBRB_BATTLE_H_INCLUDED
#define PBRB_BATTLE_H_INCLUDED
#include "pbrb_game.h"
/* Native Red/Blue mechanics. Side 0 is the player; side 1 the opponent.
   A submitted command resolves logical actions, never presentation frames.
   Drain every event before submitting another command. */
enum PbrbBattleState {
    PBRB_BATTLE_COMMAND, PBRB_BATTLE_FORCE_SWITCH,
    PBRB_BATTLE_LEARN_MOVE, PBRB_BATTLE_FINISHED, PBRB_BATTLE_SWITCH_OFFER,
    PBRB_BATTLE_MIMIC
};
enum PbrbBattleOutcome {
    PBRB_BATTLE_NONE, PBRB_BATTLE_WIN, PBRB_BATTLE_LOSE,
    PBRB_BATTLE_CAPTURE, PBRB_BATTLE_FLEE, PBRB_BATTLE_ENEMY_FLED
};
enum PbrbBattleCommandType {
    PBRB_BATTLE_MOVE, PBRB_BATTLE_SWITCH,
    PBRB_BATTLE_ITEM, PBRB_BATTLE_RUN, PBRB_BATTLE_LEARN,
    PBRB_BATTLE_SAFARI_BALL, PBRB_BATTLE_SAFARI_BAIT, PBRB_BATTLE_SAFARI_ROCK,
    PBRB_BATTLE_DECLINE, PBRB_BATTLE_CHOOSE_MIMIC
};
enum PbrbBattleEventType {
    PBRB_BE_SEND_OUT, PBRB_BE_MOVE, PBRB_BE_HP,
    PBRB_BE_STATUS, PBRB_BE_STAT, PBRB_BE_MISS, PBRB_BE_FAILED,
    PBRB_BE_CRITICAL, PBRB_BE_EFFECTIVE, PBRB_BE_FAINT, PBRB_BE_FORCE_SWITCH,
    PBRB_BE_EXP, PBRB_BE_LEVEL, PBRB_BE_LEARN_OFFER, PBRB_BE_LEARNED,
    PBRB_BE_ITEM, PBRB_BE_CAPTURE, PBRB_BE_RESULT, PBRB_BE_MESSAGE,
    PBRB_BE_SWITCH_OFFER, PBRB_BE_MIMIC_OFFER, PBRB_BE_ANIMATION
};
/* Persistent battle-picture state, captured with each event. The mechanics
   may already have resolved a later turn action when presentation reads it. */
enum PbrbBattlePictureFlags {
    PBRB_BP_SUBSTITUTE = 1u, PBRB_BP_HIDDEN = 2u,
    PBRB_BP_MINIMIZED = 4u
};
enum PbrbBattleMessage {
    PBRB_BM_NONE, PBRB_BM_ASLEEP, PBRB_BM_WOKE,
    PBRB_BM_FROZEN, PBRB_BM_PARALYZED, PBRB_BM_CONFUSED, PBRB_BM_CONFUSION_END,
    PBRB_BM_FLINCHED, PBRB_BM_RECHARGE, PBRB_BM_DISABLED, PBRB_BM_DISABLE_END,
    PBRB_BM_CHARGING, PBRB_BM_TRAPPED, PBRB_BM_BIDE, PBRB_BM_BIDE_RELEASE,
    PBRB_BM_SUBSTITUTE, PBRB_BM_SUBSTITUTE_BROKE, PBRB_BM_SEEDED,
    PBRB_BM_POISON, PBRB_BM_BURN, PBRB_BM_RECOIL, PBRB_BM_DRAIN,
    PBRB_BM_MIST, PBRB_BM_FOCUS, PBRB_BM_LIGHT_SCREEN, PBRB_BM_REFLECT,
    PBRB_BM_HAZE, PBRB_BM_TRANSFORM, PBRB_BM_CONVERSION, PBRB_BM_PAY_DAY,
    PBRB_BM_NO_PP, PBRB_BM_CANNOT_RUN, PBRB_BM_RUN_FAILED,
    PBRB_BM_BALL_BLOCKED, PBRB_BM_BALL_MISSED, PBRB_BM_BALL_BROKE,
    PBRB_BM_SAFARI_BAIT, PBRB_BM_SAFARI_ROCK, PBRB_BM_SAFARI_WATCHING,
    PBRB_BM_DISOBEY, PBRB_BM_SELF_HIT, PBRB_BM_NOTHING_HAPPENED,
    PBRB_BM_TOXIC, PBRB_BM_HIT_COUNT, PBRB_BM_SAFARI_EATING,
    PBRB_BM_SAFARI_ANGRY, PBRB_BM_SUBSTITUTE_DAMAGED,
    PBRB_BM_SAFARI_NO_BALLS, PBRB_BM_CONFUSION_START,
    PBRB_BM_LEECH_SEED_HURT, PBRB_BM_MIMIC_LEARNED
};
struct PbrbBattleCommand {
    int type; unsigned slot, item, target, moveSlot;
};
struct PbrbBattleSetup {
    unsigned trainerClass, trainerNumber, trainerID, badges, seed;
    unsigned prizeMoney, safariBalls;
    int firstRival, safari, ghost, expAll, setMode, noCatch;
};
struct PbrbBattleEvent {
    int type, side, party, message;
    unsigned move, species, value, value2, item;
    /* Snapshots at this event, even if a later action already resolved.
       HP value is the new HP; value2 is previous HP. Item value=consumed.
       STAT value=stat 0..5, value2=new stage 1..13, message=signed change.
       EFFECTIVE value=original last type-table match; RESULT value=outcome.
       ANIMATION value=ROM ID, message=lead-in, value2=trailing delay frames.
       LEVEL statValues are the unmodified party stats at this event. */
    unsigned hp[2], maxHP[2], active[2], displaySpecies[2], level[2], pictureFlags[2];
    unsigned statValues[4];
    unsigned char status[2];
};
#define PBRB_BATTLE_EVENTS 256
struct PbrbBattleSide {
    PbrbPlayMon mon;
    unsigned stats[4]; /* current Attack, Defense, Speed, Special */
    unsigned char stages[6]; /* 1..13, neutral 7 */
    unsigned flags, substituteHP, toxicCounter, confusion, disabledMove,
        disabledTurns, chargeMove, lockMove, lockTurns, trapTurns, trapDamage,
        bideTurns, bideDamage, lastMove, lastDamage, lastDamageMove,
        aiUses, turnsOut, mimicSlot, lockSlot, rageAccuracy, disabledSlot;
};
struct PbrbBattle {
    PbrbGameRom rom;
    PbrbBattleSetup setup;
    PbrbPlayMon party[2][PBRB_PARTY_CAPACITY];
    unsigned count[2], active[2];
    PbrbBattleSide side[2];
    unsigned rng, turn, runAttempts, payday, participants, awarded, fought;
    int state, outcome, error;
    PbrbBattleEvent events[PBRB_BATTLE_EVENTS];
    unsigned eventRead, eventCount;
    PbrbPlayMon captured;
    unsigned expRemaining[6], expTarget[6], expParty, learnCount, learnIndex;
    unsigned char learnMoves[8];
    int resumeAfterLearn;
    unsigned safariBait, safariAnger, safariRate, safariBalls;
    unsigned pendingEnemy, faintMask, currentMove[2], currentSlot[2], lastDamage;
    unsigned char expStats[5];
    unsigned expBase, expPass, expMask;
    /* A successful player Mimic pauses its already-committed action after
       the hit test. Resume the residual/remaining enemy action only once. */
    unsigned mimicPendingSlot, mimicEnemySlot;
    int mimicEnemyPending;
};
void PbrbBattleDefaultSetup(PbrbBattleSetup*);
int PbrbBattleBegin(PbrbBattle*, const PbrbGameRom*,
    const PbrbPlayMon* player, unsigned playerCount, unsigned active,
    const PbrbPlayMon* enemy, unsigned enemyCount, const PbrbBattleSetup*);
/* 1 accepted, 0 invalid command/state. Invalid commands do not spend a turn. */
int PbrbBattleSubmit(PbrbBattle*, const PbrbBattleCommand*);
int PbrbBattleNextEvent(PbrbBattle*, PbrbBattleEvent*);
const PbrbPlayMon* PbrbBattlePlayerParty(const PbrbBattle*);
/* Rage, recharge, Thrash/Petal Dance and charge turns skip the command menu. */
int PbrbBattleForcedMove(const PbrbBattle*);
/* FIGHT advances directly while sleeping/frozen, using Bide/trapping, or
   held by the opponent's trapping move. Main-menu items/switch/run remain. */
int PbrbBattleSkipMoveMenu(const PbrbBattle*);
/* SELECT swaps both volatile and party move/PP slots, as the original does
   even while transformed. Returns 0 outside an idle command menu. */
int PbrbBattleSwapMoves(PbrbBattle*, unsigned first, unsigned second);
/* Original class IDs 1..47 and one-based trainer number; no ROM ID offset. */
int PbrbBattleLoadTrainer(const PbrbGameRom*, unsigned trainerClass, unsigned number,
    unsigned rivalStarter, PbrbPlayMon out[6], unsigned* count, unsigned* prizeMoney);
/* Encounter selection only: rate and ten original (level,species) slots.
   World supplies its current map encounter table and applies repel. */
int PbrbBattleWildEncounter(const PbrbGameRom*, unsigned tableOffset,
    unsigned* rng, PbrbPlayMon* out);
#endif
