#pragma once
#ifndef PBRB_INTRO_H
#define PBRB_INTRO_H
#include "pbrb_gb.h"
/* Shared cues from intro.asm / splash.asm. Copyright lasts 180 frames,
   followed by 6 white setup frames and 67 black-bar setup frames.
   LCD-off loading is rounded from original elapsed CPU cycles, not
   emulator screenshot counts, which include partial LCD-restart frames.
   The star is 40 movement + 30 flash + 144 small-star + 40 hold frames,
   then Delay3. */
enum {
    PBRB_INTRO_STAR_START = 180 + 6 + 67,
    PBRB_INTRO_MUSIC_START = PBRB_INTRO_STAR_START + 254,
    PBRB_INTRO_BATTLE_START = PBRB_INTRO_MUSIC_START + 3,
    PBRB_INTRO_BATTLE_FRAMES = 537,
    PBRB_INTRO_FADE_START = PBRB_INTRO_BATTLE_START + PBRB_INTRO_BATTLE_FRAMES,
    PBRB_INTRO_END = PBRB_INTRO_FADE_START + 24 + 1
};
/* Native ROM-based intro timeline. DMG frames, separate from display refresh.
   See engine/movie/splash.asm and engine/movie/intro.asm in pret/pokered. */
struct PbrbIntro {
    unsigned frame;
    unsigned game;
};
void PbrbIntroReset(PbrbIntro* intro, unsigned game);
void PbrbIntroTick(PbrbIntro* intro);
int PbrbIntroComplete(const PbrbIntro* intro);
int PbrbIntroCanSkip(const PbrbIntro* intro);
int PbrbIntroAssetsValid(const unsigned char* rom, unsigned bytes, unsigned game);
void PbrbIntroRender(PbrbGb* gb, const PbrbIntro* intro);
#endif
