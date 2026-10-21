#ifndef PBRB_AUDIO_CORE_H
#define PBRB_AUDIO_CORE_H
#include <stddef.h>
/* Native Gen-I ROM sequencer and PCM APU. Timing/commands follow pokered's
   three audio engines; the PCM output backend remains Xbox DirectSound. */
#define PBRB_AUDIO_RATE 44100
#define PBRB_AUDIO_CHANNELS 2
#define PBRB_AUDIO_MOVE 0
#define PBRB_AUDIO_CONFIRM 1
#define PBRB_AUDIO_INTRO_HIP 2
#define PBRB_AUDIO_INTRO_HOP 3
#define PBRB_AUDIO_INTRO_RAISE 4
#define PBRB_AUDIO_INTRO_CRASH 5
#define PBRB_AUDIO_INTRO_LUNGE 6
#define PBRB_AUDIO_INTRO_WHOOSH 7
#define PBRB_AUDIO_SHOOTING_STAR 8
   /* Original ROM IDs for map headers, text commands and animation sounds.
      Valid audio banks are $02, $08 and $1f. APIs reject unknown header starts. */
#define PBRB_AUDIO_RAW(bank,sound) (0x10000 | ((int)(bank) << 8) | (int)(sound))

      /* Semantic IDs deliberately keep bank-specific ROM sound numbers out of the
         game/UI code. Oak's opening speech uses Routes2; his Pallet escort uses
         MeetProfOak. ROM internal species IDs (not Pokedex numbers) identify cries. */
enum PbrbMusicId {
    PBRB_MUSIC_NONE = 0, PBRB_MUSIC_TITLE, PBRB_MUSIC_INTRO,
    PBRB_MUSIC_OAK_SPEECH, PBRB_MUSIC_PALLET, PBRB_MUSIC_OAK,
    PBRB_MUSIC_LAB, PBRB_MUSIC_RIVAL, PBRB_MUSIC_TRAINER_BATTLE,
    PBRB_MUSIC_POKECENTER, PBRB_MUSIC_HEALED, PBRB_MUSIC_DEFEATED_TRAINER,
    PBRB_MUSIC_WILD_BATTLE, PBRB_MUSIC_GYM_BATTLE, PBRB_MUSIC_CHAMPION_BATTLE,
    PBRB_MUSIC_DEFEATED_WILD, PBRB_MUSIC_DEFEATED_GYM, PBRB_MUSIC_EVOLUTION,
    PBRB_MUSIC_HALL_OF_FAME, PBRB_MUSIC_CREDITS, PBRB_MUSIC_BIKE,
    PBRB_MUSIC_SURFING, PBRB_MUSIC_JIGGLYPUFF, PBRB_MUSIC_GYM,
    PBRB_MUSIC_CITIES1, PBRB_MUSIC_CITIES2, PBRB_MUSIC_CELADON,
    PBRB_MUSIC_CINNABAR, PBRB_MUSIC_VERMILION, PBRB_MUSIC_LAVENDER,
    PBRB_MUSIC_SS_ANNE, PBRB_MUSIC_MUSEUM, PBRB_MUSIC_SAFARI,
    PBRB_MUSIC_ROUTES1, PBRB_MUSIC_ROUTES3, PBRB_MUSIC_ROUTES4,
    PBRB_MUSIC_INDIGO_PLATEAU, PBRB_MUSIC_GAME_CORNER,
    PBRB_MUSIC_DUNGEON1, PBRB_MUSIC_DUNGEON2, PBRB_MUSIC_DUNGEON3,
    PBRB_MUSIC_MANSION, PBRB_MUSIC_POKEMON_TOWER, PBRB_MUSIC_SILPH_CO,
    PBRB_MUSIC_MEET_EVIL, PBRB_MUSIC_MEET_FEMALE, PBRB_MUSIC_MEET_MALE,
    PBRB_MUSIC_COUNT
};
enum PbrbSoundId {
    PBRB_SFX_PRESS = 0, PBRB_SFX_START_MENU, PBRB_SFX_COLLISION,
    PBRB_SFX_GO_INSIDE, PBRB_SFX_GO_OUTSIDE, PBRB_SFX_SHRINK,
    PBRB_SFX_SAVE, PBRB_SFX_GET_ITEM, PBRB_SFX_HEAL,
    PBRB_SFX_PC_ON, PBRB_SFX_PC_OFF,
    PBRB_SFX_TRAINER_APPEARED, PBRB_SFX_BALL_POOF, PBRB_SFX_GET_KEY_ITEM,
    PBRB_SFX_DAMAGE, PBRB_SFX_NOT_VERY_EFFECTIVE, PBRB_SFX_SUPER_EFFECTIVE,
    PBRB_SFX_FAINT_FALL, PBRB_SFX_FAINT_THUD, PBRB_SFX_BALL_TOSS,
    PBRB_SFX_CAUGHT_MON, PBRB_SFX_LEVEL_UP, PBRB_SFX_DEX_PAGE_ADDED,
    PBRB_SFX_RUN, PBRB_SFX_HEAL_AILMENT, PBRB_SFX_GET_ITEM2, PBRB_SFX_TINK,
    PBRB_SFX_POISONED, PBRB_SFX_LEDGE, PBRB_SFX_PURCHASE, PBRB_SFX_DENIED,
    PBRB_SFX_CUT, PBRB_SFX_FLY, PBRB_SFX_TELEPORT_EXIT, PBRB_SFX_TELEPORT_ENTER,
    PBRB_SFX_HEALING_MACHINE, PBRB_SFX_POKEFLUTE, PBRB_SFX_SAFARI_PA,
    PBRB_SFX_WITHDRAW_DEPOSIT, PBRB_SFX_SWAP, PBRB_SFX_SWITCH, PBRB_SFX_ENTER_PC,
    PBRB_SFX_PUSH_BOULDER, PBRB_SFX_ARROW_TILES, PBRB_SFX_SS_ANNE_HORN,
    PBRB_SFX_SLOTS_STOP, PBRB_SFX_SLOTS_REWARD, PBRB_SFX_SLOTS_SPIN,
    PBRB_SFX_STOP_ALL, /* original $ff; stops voices without detaching ROM */
    PBRB_SFX_COUNT,
    PBRB_SFX_HIT = PBRB_SFX_DAMAGE, PBRB_SFX_INEFFECTIVE = PBRB_SFX_NOT_VERY_EFFECTIVE,
    PBRB_SFX_FAINT = PBRB_SFX_FAINT_FALL, PBRB_SFX_CAPTURE = PBRB_SFX_CAUGHT_MON
};

struct PbrbVoice {
    unsigned pc, bank, callReturn[8], loopPc, remaining;
    unsigned frequency, slideCurrent, slideTarget, noteSerial;
    unsigned char channel, enabled, source, callDepth, loopRemaining;
    unsigned char octave, noteType, volume, envelope, duty, waveInstrument;
    unsigned char fraction, perfectPitch, dutyPattern, rotateDuty;
    unsigned char vibratoDelay, vibratoWait, vibratoDepth, vibratoRate;
    unsigned char vibratoCounter, vibratoDirection;
    unsigned char slideActive, slideDecreasing, slideLength, slideStep;
    unsigned char slideFraction, slideAccumulator, lastNote, drum;
    unsigned char executeMusic, cry, soundId;
};
struct PbrbApuChannel {
    unsigned phase, frequency, noise, sweepShadow;
    unsigned long long step;
    unsigned char active, owner, channel, duty, waveInstrument, panning;
    unsigned char volume, envelope, envelopeTimer, envelopeEnabled;
    unsigned char noiseMode, sweep, sweepTimer, sweepEnabled;
    unsigned char waveData[32];
};
struct PbrbSynth {
    const unsigned char* rom;
    size_t romSize;
    PbrbVoice song[4], effect[4];
    PbrbApuChannel apu[4];
    unsigned char menuData[9][48], menuLength[9], wave[9][32];
    unsigned tempo, sampleRate, frameClock, apuClock, apuFrame, tick, decodeErrors;
    unsigned sfxTempo, musicBank, musicHeader, sequencerPause;
    unsigned fadeCounter, fadeReload;
    unsigned char panning, masterVolume, savedVolume, cryFrequency, cryTempo;
    unsigned char lowHealthAlarm; /* original enable bit/timer, $ff requests silence */
    int filterIn[2], filterOut[2];
    int musicOn, effectsOn, menuLoaded, songValid, musicId, fadeTarget, fadeActive;
};
void PbrbSynthInit(PbrbSynth* synth, int sampleRate);
int PbrbSynthLoadMenuBank(PbrbSynth* synth, const unsigned char* bank, int bankBytes);
int PbrbSynthStartTitle(PbrbSynth* synth, const unsigned char* rom, size_t bytes);
int PbrbSynthAttachIntro(PbrbSynth* synth, const unsigned char* rom, size_t bytes);
int PbrbSynthStartIntroMusic(PbrbSynth* synth);
/* The attached ROM must remain alive until StopTitle; NONE only stops music.
   Selecting the already active song preserves its position. */
int PbrbSynthPlayMusic(PbrbSynth* synth, int music);
int PbrbSynthFadeMusic(PbrbSynth* synth, int music, unsigned framesPerStep);
int PbrbSynthPlayCry(PbrbSynth* synth, unsigned species);
int PbrbSynthPlaySfx(PbrbSynth* synth, int effect);
/* Original wFrequencyModifier/wTempoModifier bytes used by move animations.
   Neutral values are 0 and $80; raw SFX IDs use PBRB_AUDIO_RAW(bank,sound). */
int PbrbSynthPlaySfxModified(PbrbSynth* synth, int effect,
    unsigned frequencyModifier, unsigned tempoModifier);
void PbrbSynthStopTitle(PbrbSynth* synth);
/* Skip upcoming VBlank sequencer ticks while hardware audio keeps running.
   Replaces any pending pause; zero resumes at the next normal boundary. */
void PbrbSynthPauseSequencer(PbrbSynth* synth, unsigned frames);
void PbrbSynthSetMusicEnabled(PbrbSynth* synth, int enabled);
void PbrbSynthSetEffectsEnabled(PbrbSynth* synth, int enabled);
/* Repeated enable calls preserve the two-tone phase; disable is applied at
   the next VBlank sequencer boundary, as Music_DoLowHealthAlarm does. */
void PbrbSynthSetLowHealth(PbrbSynth* synth, int enabled);
void PbrbSynthUi(PbrbSynth* synth, int effect);
int PbrbSynthEffectsBusy(const PbrbSynth* synth);
int PbrbSynthMusicBusy(const PbrbSynth* synth);
void PbrbSynthRender(PbrbSynth* synth, short* out, int frames);
#endif
