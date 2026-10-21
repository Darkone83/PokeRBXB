#include "pbrb_engine.h"
#include "pbrb_gb.h"
#include "pbrb_title.h"
#include "pbrb_intro.h"
#include "pbrb_texture.h"
#include "pbrb_audio.h"
#include "pbrb_audio_core.h"
#include "pbrb_packages.h"
#include "pbrb_play.h"
#include "xpd_reader.h"
#include <xtl.h>
#include <stdlib.h>
#include <string.h>
#include "pbrb_play.inl"

/* Stock 64MB safe: do not put a 160x144x4 buffer on the Xbox stack. */
static PbrbWorld s_world;
static PbrbGb s_gb;
static PbrbTitle s_title;
static PbrbIntro s_intro;
static PbrbPlay s_play;
static PbrbTexture s_frame;
static unsigned char* s_rom = 0;
static unsigned s_game = 0;
static int s_running = 0;
static PbrbFlow s_flow;
static unsigned char s_pixels[PBRB_GB_RGBA_BYTES];
static DWORD s_lastMs = 0;
static unsigned s_accumCycles = 0;
static unsigned s_held = 0, s_pressed = 0;
static int s_dirty = 1;
static int s_songStarted = 0;
static unsigned s_titlePrep = 0;
static unsigned s_titleExit = 0, s_titleWhite = 0;

static int RefreshTexture() {
    PbrbGbSetPalettes(&s_gb, 0xe4, 0xd0, 0xe4);
    if (s_flow.screen == PBRB_SCREEN_GAMEPLAY)
        PbrbPlayRender(&s_play);
    else if (s_flow.screen == PBRB_SCREEN_INTRO)
        PbrbIntroRender(&s_gb, &s_intro);
    else if (s_flow.screen == PBRB_SCREEN_TITLE) {
        if (s_titlePrep > 1u || s_titleExit == 2u)PbrbGbClear(&s_gb, 0);
        else PbrbGbTitle(&s_gb, s_title.logoScrollY, s_title.versionX,
            s_title.versionVisible, 1, s_title.monScrollX, s_title.ballY, s_title.monVisible);
    }
    else if (s_flow.screen == PBRB_SCREEN_MENU)
        PbrbGbGameMenu(&s_gb, s_flow.selected, s_flow.hasSave);
    else if (s_flow.screen == PBRB_SCREEN_OPTIONS)
        PbrbGbOptions(&s_gb, s_flow.selected, s_flow.textSpeed,
            s_flow.battleAnimation, s_flow.battleStyle);
    else {
        PbrbGbClear(&s_gb, 0);
    }
    if (s_flow.screen == PBRB_SCREEN_MENU && s_flow.saveLoadError) {
        PbrbGbTextBox(&s_gb, 0, 88, 20, 7);
        PbrbGbString(&s_gb, "Save could not", 8, 104);
        PbrbGbString(&s_gb, "be loaded.", 8, 120);
    }
    return PbrbUploadBGRA(s_pixels, PBRB_GB_W, PBRB_GB_H, &s_frame);
}
void PbrbEngineStop() {
    PbrbInputStopRumble(); PbrbAudioStopTitle(); PbrbReleaseTexture(&s_frame);
    if (s_rom)free(s_rom);
    s_rom = 0; s_game = 0; s_running = 0; s_titlePrep = s_titleExit = s_titleWhite = 0;
    memset(&s_world, 0, sizeof(s_world)); memset(&s_gb, 0, sizeof(s_gb));
    memset(&s_play, 0, sizeof(s_play)); s_pressed = s_held = 0;
}
static int GameRomRevision(const unsigned char* data, unsigned game) {
    /* Native scenes use original ROM addresses. Reject other revisions as
       data errors before any table/picture/text pointer can be followed. */
    unsigned crc = 0xffffffffu;
    for (unsigned i = 0; i < PBRB_ROM_SIZE; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b)crc = (crc >> 1) ^ ((crc & 1u) ? 0xedb88320u : 0u);
    }
    return ~crc == (game == XPD_GAME_RED ? 0x9f7fdd53u : 0xd6da8a1au);
}
int PbrbEngineStart(const char* xpdPath, unsigned game) {
    XpdArchive archive; XpdResource resource; uint32_t size = 0;
    void* content = 0; int ok = 0;
    PbrbEngineStop();
    if (!xpdPath || (game != XPD_GAME_RED && game != XPD_GAME_BLUE))return 0;
    memset(&archive, 0, sizeof(archive));
    if (!xpd_open(&archive, xpdPath))return 0;
    if (archive.game == game && xpd_find(&archive, "rom/full.gb", &resource) &&
        resource.size == PBRB_ROM_SIZE &&
        xpd_read_alloc(&archive, "rom/full.gb", &content, &size) && size == PBRB_ROM_SIZE) {
        s_rom = (unsigned char*)content; content = 0;
        if (GameRomRevision(s_rom, game) && PbrbIntroAssetsValid(s_rom, PBRB_ROM_SIZE, game) &&
            PbrbWorldInit(&s_world, s_rom, PBRB_ROM_SIZE, PBRB_MAP_BEDROOM) &&
            PbrbGbInit(&s_gb, s_rom, PBRB_ROM_SIZE, (int)game, s_pixels, sizeof(s_pixels))) {
            /* This attaches the original bank-31 SFX without depending on a
               desktop packer or extra media files. Failure leaves gameplay
               intact, but flags a sound-data revision mismatch. */
            if (!PbrbAudioAttachIntro(s_rom, PBRB_ROM_SIZE))
                OutputDebugStringA("PokeRBXB: intro SFX ROM signature failed\n");
            s_game = game; PbrbFlowInit(&s_flow, game); PbrbTitleReset(&s_title); PbrbIntroReset(&s_intro, game);
            PbrbGbSetColorMode(&s_gb, PbrbGetPreferences()->color);
            PbrbPlayInit(&s_play, &s_world, &s_gb, game); s_flow.hasSave = PbrbHasSave(game);
            s_songStarted = 0; s_dirty = 1; s_lastMs = GetTickCount(); s_accumCycles = 0;
            if (RefreshTexture()) { s_running = 1; ok = 1; }
        }
    }
    if (content)free(content);
    xpd_close(&archive);
    if (!ok)PbrbEngineStop();
    return ok;
}
int PbrbEngineIsRunning() { return s_running; }
static void ChangeScreen(int previous) {
    if (s_flow.screen == PBRB_SCREEN_TITLE && previous != PBRB_SCREEN_TITLE) {
        PbrbTitleReset(&s_title); s_songStarted = 0;
        s_titleExit = s_titleWhite = 0;
        /* Returning from the menu/credits always begins with this version's
           starter, even when the previous idle loop had selected another. */
        PbrbGbDecodePokemon(&s_gb, s_rom[0x4588u], 0, s_gb.titleMon, sizeof(s_gb.titleMon));
        /* The original prepares the title while the final intro notes
           continue. Elapsed CPU cycles round to 50 white frames in Red,
           49 in Blue, then one extra held title image before the bounce.
           LCD restarts make screenshot counts differ from elapsed time. */
        s_titlePrep = previous == PBRB_SCREEN_INTRO ? (s_game == XPD_GAME_RED ? 51u : 50u) : 0u;
        if (previous != PBRB_SCREEN_INTRO)PbrbAudioStopTitle();
    }
    if (s_flow.screen == PBRB_SCREEN_NEW_GAME) {
        PbrbPlayNew(&s_play, s_flow.textSpeed, s_flow.battleAnimation, s_flow.battleStyle, (unsigned)GetTickCount());
        s_flow.screen = PBRB_SCREEN_GAMEPLAY; s_pressed = 0; s_accumCycles = 0; s_lastMs = GetTickCount();
    }
    else if (s_flow.screen == PBRB_SCREEN_CONTINUE) {
        if (PbrbPlayLoad(&s_play)) { s_flow.screen = PBRB_SCREEN_GAMEPLAY; s_flow.saveLoadError = 0; }
        else { s_flow.screen = PBRB_SCREEN_MENU; s_flow.selected = 0; s_flow.saveLoadError = 1; }
        s_pressed = 0; s_accumCycles = 0; s_lastMs = GetTickCount();
    }
    s_dirty = 1;
}
static void Action(int action) {
    const int old = s_flow.screen;
    if (old == PBRB_SCREEN_TITLE && action == PBRB_ACTION_CONFIRM) {
        if (!s_titleExit && s_songStarted && PbrbTitleCanAccept(&s_title)) {
            PbrbAudioPlayCry(s_title.monSpecies); s_titleExit = 1;
        }
        return;
    }
    if (action == PBRB_ACTION_CONFIRM && s_flow.effectsEnabled)PbrbAudioUi(PBRB_AUDIO_CONFIRM);
    if ((action == PBRB_ACTION_UP || action == PBRB_ACTION_DOWN) && s_flow.effectsEnabled)
        PbrbAudioUi(PBRB_AUDIO_MOVE);
    PbrbFlowDispatch(&s_flow, action); ChangeScreen(old);
}
static void ClockGame() {
    DWORD now = GetTickCount(), diff = now - s_lastMs;
    s_lastMs = now;
    if (diff > 100u)diff = 100u;
    /* Exact 70224/4194304-second frame ratio. Accumulate clock cycles in
       thousandths so millisecond host timing does not round every GB frame. */
    s_accumCycles += (unsigned)diff * 4194304u;
    unsigned budget = 0;
    while (s_accumCycles >= 70224000u && budget++ < 6u) {
        s_accumCycles -= 70224000u;
        if (s_flow.screen == PBRB_SCREEN_GAMEPLAY) {
            unsigned pressed = s_pressed; s_pressed = 0;
            PbrbPlayTick(&s_play, s_held, pressed); s_dirty = 1;
        }
        else if (s_flow.screen == PBRB_SCREEN_INTRO) {
            PbrbIntroTick(&s_intro);
            const unsigned fi = s_intro.frame;
            const unsigned start = PBRB_INTRO_BATTLE_START;
            if (fi == PBRB_INTRO_STAR_START)PbrbAudioUi(PBRB_AUDIO_SHOOTING_STAR);
            if (fi == PBRB_INTRO_MUSIC_START)PbrbAudioStartIntroMusic();
            if (fi >= start) {
                const unsigned bt = fi - start;
                if (bt == 80u || bt == 140u || bt == 274u || bt == 397u)
                    PbrbAudioUi(PBRB_AUDIO_INTRO_HIP);
                else if (bt == 105u || bt == 165u || bt == 422u)
                    PbrbAudioUi(PBRB_AUDIO_INTRO_HOP);
                else if (bt == 220u)PbrbAudioUi(PBRB_AUDIO_INTRO_RAISE);
                else if (bt == 258u)PbrbAudioUi(PBRB_AUDIO_INTRO_CRASH);
                else if (bt == 517u)PbrbAudioUi(PBRB_AUDIO_INTRO_LUNGE);
            }
            s_dirty = 1;
            if (PbrbIntroComplete(&s_intro)) {
                int previous = s_flow.screen;
                s_flow.screen = PBRB_SCREEN_TITLE;
                ChangeScreen(previous);
            }
        }
        else if (s_flow.screen == PBRB_SCREEN_TITLE) {
            if (s_titleExit) {
                if (s_titleExit == 1u && !PbrbAudioEffectsBusy()) {
                    /* The original waits for the current mon's cry, then
                       whites out while clearing both maps and loading the
                       main-menu font. The measured original no-save path
                       takes 61 displayed GB frames in both ROMs. */
                    s_titleExit = 2; s_titleWhite = 61;
                }
                else if (s_titleExit == 2u && !--s_titleWhite) {
                    s_titleExit = 0;
                    PbrbFlowDispatch(&s_flow, PBRB_ACTION_CONFIRM);
                    ChangeScreen(PBRB_SCREEN_TITLE);
                }
                s_dirty = 1; continue;
            }
            if (s_titlePrep) {
                const unsigned total = s_game == XPD_GAME_RED ? 51u : 50u;
                const unsigned elapsed = total - s_titlePrep + 1u;
                /* Two original LCD-off loads suppress the VBlank music
                   sequencer. The APU itself continues sounding. These
                   are measured missed audio updates, not LCDC samples. */
                if (elapsed == 1u)PbrbAudioPauseSequencer(5u);
                else if (elapsed == 10u)PbrbAudioPauseSequencer(5u);
                --s_titlePrep; s_dirty = 1; continue;
            }
            /* The rendered bounce frame zero follows the first palette
               reveal by one frame. Original sound commands therefore
               precede the corresponding visible change by one tick. */
            if (s_title.frame < 101) { PbrbTitleTick(&s_title); s_dirty = 1; }
            if (s_title.frame == 19u)PbrbAudioUi(PBRB_AUDIO_INTRO_CRASH);
            if (s_title.frame == 67u)PbrbAudioUi(PBRB_AUDIO_INTRO_WHOOSH);
            if (!s_songStarted && s_title.frame >= 101 && !PbrbAudioEffectsBusy()) {
                PbrbAudioStartTitle(s_rom, PBRB_ROM_SIZE); s_songStarted = 1;
                PbrbTitleBeginIdle(&s_title, s_rom, PBRB_ROM_SIZE, (unsigned)GetTickCount());
            }
            else if (s_songStarted) {
                if (PbrbTitleIdleTick(&s_title, s_rom, PBRB_ROM_SIZE, (int)s_game))
                    PbrbGbDecodePokemon(&s_gb, s_title.monSpecies, 0, s_gb.titleMon, sizeof(s_gb.titleMon));
                s_dirty = 1;
            }
        }
    }
    if (budget > 6u)s_accumCycles = 0;
}
int PbrbEngineHandle(const PbrbEvents* in) {
    if (!s_running || !in)return 0;
    s_held = in->held;
    if (s_flow.screen == PBRB_SCREEN_GAMEPLAY)s_pressed |= in->pressed;
    else s_pressed = 0;
    ClockGame();
    if (s_flow.screen == PBRB_SCREEN_GAMEPLAY) {
        if (s_play.exitRequested == 1)return 1;
        if (s_play.exitRequested == 3) {
            s_play.exitRequested = 0; s_flow.screen = PBRB_SCREEN_TITLE;
            s_flow.hasSave = PbrbHasSave(s_game); s_flow.saveLoadError = 0; ChangeScreen(PBRB_SCREEN_GAMEPLAY); return 0;
        }
        if (s_play.exitRequested == 2) {
            s_play.exitRequested = 0; s_flow.screen = PBRB_SCREEN_MENU; s_flow.selected = 0;
            s_flow.hasSave = PbrbHasSave(s_game); s_dirty = 1;
        }
        return 0;
    }
    if (in->exit) {
        if (s_flow.screen == PBRB_SCREEN_TITLE)return 1;
        if (s_flow.screen == PBRB_SCREEN_INTRO) {
            if (PbrbIntroCanSkip(&s_intro))Action(PBRB_ACTION_BACK);
            return 0;
        }
        Action(PBRB_ACTION_BACK); return 0;
    }
    if (s_flow.screen == PBRB_SCREEN_INTRO) {
        if (in->accept && PbrbIntroCanSkip(&s_intro))
            Action(PBRB_ACTION_CONFIRM);
        return 0;
    }
    if (s_flow.screen == PBRB_SCREEN_TITLE) {
        if (in->accept)Action(PBRB_ACTION_CONFIRM);
        return 0;
    }
    if (s_flow.screen == PBRB_SCREEN_OPTIONS && (in->pressed & PBRB_PAD_START)) {
        Action(PBRB_ACTION_BACK); return 0;
    }
    if (in->left)Action(PBRB_ACTION_LEFT);
    if (in->right)Action(PBRB_ACTION_RIGHT);
    if (in->up)Action(PBRB_ACTION_UP);
    if (in->down)Action(PBRB_ACTION_DOWN);
    if (in->accept)Action(PBRB_ACTION_CONFIRM);
    return 0;
}
void PbrbEngineDraw() {
    if (!s_running)return;
    if (s_dirty) { RefreshTexture(); s_dirty = 0; }
    if (s_frame.gpu) {
        /* main.cpp supplies the measured clear opening in the selected
           border. The video layer fits the largest physical 10:9 picture
           inside it, including non-square SD output pixels. */
        PbrbRect r = PbrbGameRect();
        PbrbSpriteGame(&s_frame, r.x, r.y, r.w, r.h, 0xFFFFFFFF);
    }
}
