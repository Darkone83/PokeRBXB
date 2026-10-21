#ifndef PBRB_ENGINE_H
#define PBRB_ENGINE_H
#include "pbrb_world.h"
#include "pbrb_video.h"
#include "pbrb_input.h"
#include "pbrb_flow.h"
/* Native intro/title and gameplay. Back+Start opens the launcher-return
   prompt; B remains the original in-game cancel button. */
int PbrbEngineStart(const char* xpdPath, unsigned expectedGame);
void PbrbEngineStop();
int PbrbEngineIsRunning();
/* Platform pause retains the scene and excludes elapsed wall time on resume.
   The launcher owns the reconnect prompt, audio pause and global settings. */
void PbrbEngineSetPaused(int paused);
int PbrbEngineHandle(const PbrbEvents* input); /* returns 1 = back to launcher */
void PbrbEngineDraw(); /* between PbrbBegin/PbrbEnd */
#endif
