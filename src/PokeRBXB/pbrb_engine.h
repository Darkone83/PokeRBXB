#ifndef PBRB_ENGINE_H
#define PBRB_ENGINE_H
#include "pbrb_world.h"
#include "pbrb_video.h"
#include "pbrb_input.h"
#include "pbrb_flow.h"
/* Native original intro/title, Oak/naming and opening gameplay through the
   first rival battle command menu. Back+Start asks to return to the launcher;
   B remains the original in-game cancel button. */
int PbrbEngineStart(const char* xpdPath, unsigned expectedGame);
void PbrbEngineStop();
int PbrbEngineIsRunning();
int PbrbEngineHandle(const PbrbEvents* input); /* returns 1 = back to launcher */
void PbrbEngineDraw(); /* between PbrbBegin/PbrbEnd */
#endif
