#pragma once
#ifndef PBRB_TITLE_H
#define PBRB_TITLE_H
#include <stddef.h>
/* The original title-logo Y-scroll is faithfully table-driven from
   engine/movie/title.asm. One tick = one GB frame (70224 CPU cycles).
   Separating frame updates from 480i/480p/576i/720p display is essential. */
struct PbrbTitle {
    int frame;
    int logoScrollY;
    int bounceDone;
    int versionX;
    int versionVisible;
    unsigned monSpecies, random;
    int monScrollX, monVisible, ballY;
    unsigned idlePhase, idleFrame, loadFrames;
};
enum PbrbTitleIdlePhase {
    PBRB_TITLE_IDLE_NONE, PBRB_TITLE_IDLE_HOLD, PBRB_TITLE_IDLE_OUT,
    PBRB_TITLE_IDLE_POLL, PBRB_TITLE_IDLE_BALL, PBRB_TITLE_IDLE_COPY,
    PBRB_TITLE_IDLE_LOAD, PBRB_TITLE_IDLE_IN
};
void PbrbTitleReset(PbrbTitle* t);
void PbrbTitleTick(PbrbTitle* t);
void PbrbTitleBeginIdle(PbrbTitle* t, const unsigned char* rom, size_t bytes, unsigned seed);
/* Returns nonzero when the displayed species needs its original ROM picture. */
int PbrbTitleIdleTick(PbrbTitle* t, const unsigned char* rom, size_t bytes, int game);
int PbrbTitleCanAccept(const PbrbTitle* t);
#endif
