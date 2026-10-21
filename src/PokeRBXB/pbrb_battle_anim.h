#ifndef PBRB_BATTLE_ANIM_H_INCLUDED
#define PBRB_BATTLE_ANIM_H_INCLUDED
#include "pbrb_gb.h"
/* Native execution of the original animation data language, not CPU code.
   This state owns its captured frame and should live in the persistent game
   state, not on the Xbox thread stack. No graphics are embedded here. */
#define PBRB_BATTLE_ANIM_OPS 2048
struct PbrbBattleAnimOp {
    unsigned short kind, a, b, c, frames;
};
struct PbrbBattleAnim {
    PbrbGb* gb;
    PbrbBattleAnimOp ops[PBRB_BATTLE_ANIM_OPS];
    unsigned count, next, remaining, elapsed, errors, move, enemyTurn;
    unsigned species[2], actorSpecies, targetSpecies, item, shakes;
    unsigned char base[PBRB_GB_W * PBRB_GB_H];
    unsigned char picture[2][56 * 56], original[2][56 * 56], scratch[56 * 56];
    unsigned short tilemap[20 * 18], displayedMap[20 * 18], hudWindowMap[20 * 18];
    unsigned char tiles[128][64], oam[40][4], displayedOam[40][4], motion[40];
    unsigned oamDest, palette, objectPalette, savedPalette;
    unsigned active, endWait, captured, compilePalette;
    unsigned loadSet, loadPart, loadActive, waterX, fallingCount;
    unsigned pictureCopySprites, displayedPalette, displayedObjectPalette, transferPhase;
    unsigned transferEnabled, evolution, forced, canCancel, cancelled;
    int scrollX, scrollY, wavePhase, hudShake, blank;
    int displayedX, displayedY, displayedWave, displayedHud, displayedBlank;
};
/* Call after drawing the stable battle frame; userSpecies is the actor's
   original internal species, targetSpecies the other side's species. */
int PbrbBattleAnimInit(PbrbBattleAnim* animation, PbrbGb* gb,
    unsigned move, int enemyTurn, unsigned userSpecies, unsigned targetSpecies,
    int substitute = 0, int minimized = 0);
void PbrbBattleAnimTick(PbrbBattleAnim* animation);
void PbrbBattleAnimRender(const PbrbBattleAnim* animation, PbrbGb* gb);
/* Trade cable scenes can render their full scrolling ROM tilemap first,
   then overlay only the animation's original OAM. */
void PbrbBattleAnimRenderSprites(const PbrbBattleAnim* animation, PbrbGb* gb);
int PbrbBattleAnimDone(const PbrbBattleAnim* animation);
/* Original ball-toss script sequence. result: 0 caught, 1 broke free,
   2 missed, 3 blocked by trainer, 4 Ghost Marowak dodges. shakes is 0..3. */
int PbrbBattleAnimCapture(PbrbBattleAnim* animation, PbrbGb* gb,
    unsigned item, unsigned shakes, unsigned result,
    unsigned playerSpecies, unsigned enemySpecies);
/* These helpers own only the original visual/audio sequence; caller owns
   text, battle mechanics, music changes and whether the battle continues. */
int PbrbBattleAnimFaint(PbrbBattleAnim* animation, PbrbGb* gb,
    unsigned side, unsigned species, int trainerBattle);
int PbrbBattleAnimSendOut(PbrbBattleAnim* animation, PbrbGb* gb,
    unsigned side, unsigned species, int trainerBattle);
int PbrbBattleAnimRetreat(PbrbBattleAnim* animation, PbrbGb* gb);
/* Trainer AI withdraws the old front picture with the original eight
   two-frame tile shifts, keeping its HUD until the next send-out text. */
int PbrbBattleAnimEnemyWithdraw(PbrbBattleAnim* animation, PbrbGb* gb);
int PbrbBattleAnimSubstituteBreak(PbrbBattleAnim* animation, PbrbGb* gb,
    unsigned side, unsigned species, int minimized);
/* The original generic impact after a successful damaging move. side is
   the damaged side, factor uses 10 for neutral damage; move selects the
   original effect-dependent blink/shake (0 reuses the preceding move). */
int PbrbBattleAnimHit(PbrbBattleAnim* animation, PbrbGb* gb,
    unsigned side, unsigned factor, unsigned move = 0);
/* Call after IsEvolvingText and its 50-frame pause. The helper owns the
   original centered/flipped ROM pictures, music, cries and cancellation
   windows. Caller applies the species change only when done && !cancelled. */
int PbrbBattleAnimEvolution(PbrbBattleAnim* animation, PbrbGb* gb,
    unsigned oldSpecies, unsigned newSpecies, int forced);
int PbrbBattleAnimCancelEvolution(PbrbBattleAnim* animation);
#endif
