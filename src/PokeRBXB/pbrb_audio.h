#ifndef PBRB_AUDIO_H
#define PBRB_AUDIO_H
#include <stddef.h>
#include "pbrb_audio_core.h"
/* Xbox DirectSound output; ROM-backed music, cries and sound effects.
   Audio failure is nonfatal; the game remains usable without sound. */
int PbrbAudioInit();
void PbrbAudioShutdown();
int PbrbAudioReady();
/* Original WaitForSound semantics: active external SFX, excluding drum tails. */
int PbrbAudioEffectsBusy(void);
/* Finite music/jingle wait; false on a missing or stalled output device. */
int PbrbAudioMusicBusy(void);
/* Load only ROM bank 02 from XPD. No external sound files or packer changes. */
int PbrbAudioLoadUiPack(const char* xpdPath);
/* Launcher owns one verified ROM until Stop/Shutdown. A random looping game
   theme is chosen on each visit, avoiding the previous successful selection.
   The name is empty when music/output is inactive; no new media is needed. */
int PbrbAudioStartLauncherMusic(const char* xpdPath, unsigned seed);
/* Uses the already attached ROM; a muted song can still be selected. */
int PbrbAudioNextLauncherMusic();
const char* PbrbAudioLauncherTrackName();
void PbrbAudioStopLauncherMusic();
int PbrbAudioStartTitle(const unsigned char* rom, size_t bytes);
int PbrbAudioAttachIntro(const unsigned char* rom, size_t bytes);
int PbrbAudioStartIntroMusic(void);
int PbrbAudioPlayMusic(int music); /* PbrbMusicId or PBRB_AUDIO_RAW; NONE retains ROM */
int PbrbAudioFadeMusic(int music, unsigned framesPerStep);
int PbrbAudioPlayCry(unsigned species); /* original internal species ID */
int PbrbAudioPlaySfx(int effect); /* PbrbSoundId or PBRB_AUDIO_RAW */
int PbrbAudioPlaySfxModified(int effect, unsigned frequencyModifier,
    unsigned tempoModifier);
void PbrbAudioStopTitle();
/* LCD-off intervals stop VBlank sequencing, but not the hardware audio. */
void PbrbAudioPauseSequencer(unsigned frames);
/* Global output preferences are controlled by the launcher. Mutes and
   master volume (0..10) keep sequencing and sound-wait durations intact. */
void PbrbAudioSetVolume(int volume);
/* Reconnect pause freezes the synth; rendering continues as silence. */
void PbrbAudioSetPaused(int paused);
void PbrbAudioSetMusic(int enabled);
void PbrbAudioSetEffects(int enabled);
void PbrbAudioSetLowHealth(int enabled);
void PbrbAudioUi(int kind); /* PBRB_AUDIO_MOVE / PBRB_AUDIO_CONFIRM */
#endif
