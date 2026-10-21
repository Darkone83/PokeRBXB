#include <xtl.h>
#include "pbrb_video.h"
#include "pbrb_texture.h"
#include "pbrb_ddf.h"
#include "pbrb_input.h"
#include "pbrb_glyphs.h"
#include "pbrb_packages.h"
#include "xpd_reader.h"
#include "pbrb_engine.h"
#include "pbrb_audio.h"
#include "pbrb_audio_core.h"

// Install-relative Xbox paths. XBE at D:\default.xbe, resources in D:\data.
static const char* BG_PATH = "D:\\data\\img\\launch_bg.png";
static const char* FONT_PATH = "D:\\data\\font\\font.DDF";
static const char* RED_PATH = "D:\\data\\red.xpd";
static const char* BLUE_PATH = "D:\\data\\blue.xpd";
static const char* RED_BORDER = "D:\\data\\img\\red_border.png";
static const char* BLUE_BORDER = "D:\\data\\img\\blue_border.png";
static PbrbTexture g_bg = { 0 };
static PbrbTexture g_gameBorder = { 0 };
static float g_borderU0 = 0, g_borderV0 = 0, g_borderU1 = 1, g_borderV1 = 1;
static PbrbPackStatus g_status[2] = { PBRB_MISSING,PBRB_MISSING };
static int g_selected = 0;
static int g_ingame = 0;
static char g_message[80] = "";
static DWORD g_message_start = 0;
enum LauncherPage { LAUNCHER_SELECT, LAUNCHER_OPTIONS, LAUNCHER_DIALOG };
enum DialogAction { DIALOG_RETRY_GAME, DIALOG_RETRY_OPTIONS };
static LauncherPage g_page = LAUNCHER_SELECT;
static DialogAction g_dialog_action = DIALOG_RETRY_GAME;
static int g_option_row = 0;
static PbrbPreferences g_options = { 0, 0, 0 };
static char g_dialog_title[80] = "";
static char g_dialog_line1[128] = "";
static char g_dialog_line2[128] = "";

static void ConfigureGameBorder() {
    const float sw = (float)PbrbScreenWidth(), sh = (float)PbrbScreenHeight();
    PbrbSetGameArea(0, 0, sw, sh);
    if (!g_gameBorder.gpu)return;

    // Measured from the approved, unchanged 1448x1086 PNGs. The bounds
    // contain every pixel with alpha > 1 in either file; the UVs omit
    // only negligible-alpha outer padding. Use common bounds so both
    // versions have identical game placement, including at 720p.
    const float left = 11.0f, top = 38.0f;
    const float right = 1440.0f, bottom = 1020.0f;
    const float artWidth = right - left, artHeight = bottom - top;
    const float u = (float)g_gameBorder.width / g_gameBorder.padded_width;
    const float v = (float)g_gameBorder.height / g_gameBorder.padded_height;
    g_borderU0 = left / 1448.0f * u; g_borderV0 = top / 1086.0f * v;
    g_borderU1 = right / 1448.0f * u; g_borderV1 = bottom / 1086.0f * v;

    // The common clear rectangle is (180,178)-(1268,880). Inset it by
    // two source pixels at each edge. Every pixel inside has alpha <= 1;
    // rendering the opaque game after the border excludes that faint noise.
    const float gameLeft = 182.0f, gameTop = 180.0f;
    const float gameRight = 1266.0f, gameBottom = 878.0f;
    PbrbSetGameArea((gameLeft - left) * sw / artWidth,
        (gameTop - top) * sh / artHeight,
        (gameRight - gameLeft) * sw / artWidth,
        (gameBottom - gameTop) * sh / artHeight);
}
static void SetMessage(const char* text) {
    int i = 0;
    while (text && text[i] && i < (int)sizeof(g_message) - 1) { g_message[i] = text[i]; ++i; }
    g_message[i] = 0; g_message_start = GetTickCount();
}
static void CopyLabel(char* out, unsigned size, const char* text) {
    unsigned i = 0;
    if (!size)return;
    while (text && text[i] && i + 1 < size) { out[i] = text[i]; ++i; }
    out[i] = 0;
}
static void ShowDialog(DialogAction action, const char* title,
    const char* line1, const char* line2) {
    g_dialog_action = action;
    CopyLabel(g_dialog_title, sizeof(g_dialog_title), title);
    CopyLabel(g_dialog_line1, sizeof(g_dialog_line1), line1);
    CopyLabel(g_dialog_line2, sizeof(g_dialog_line2), line2);
    g_page = LAUNCHER_DIALOG;
    PbrbInputStopRumble();
}
static void ApplyPreferences() {
    const PbrbPreferences* p = PbrbGetPreferences();
    PbrbSetSmoothFiltering(p->smooth);
    PbrbInputSetRumbleEnabled(p->rumble);
}
static void SaveOptions() {
    const int saved = PbrbSavePreferences(&g_options);
    ApplyPreferences();
    if (!saved) {
        ShowDialog(DIALOG_RETRY_OPTIONS, "OPTIONS COULD NOT BE SAVED",
            PbrbSaveError(), "The selected options still apply for this session.");
        return;
    }
    g_page = LAUNCHER_SELECT;
    SetMessage("OPTIONS SAVED");
}
static void StartLauncherMusic() {
    if (!PbrbAudioReady())return;
    /* Prefer the selected version, but either verified pack supplies the
       shared soundtrack when only one game is installed. */
    for (int attempt = 0; attempt < 2; ++attempt) {
        const int index = attempt ? 1 - g_selected : g_selected;
        if (g_status[index] == PBRB_READY &&
            PbrbAudioStartLauncherMusic(index ? BLUE_PATH : RED_PATH,
                (unsigned)GetTickCount()))return;
    }
}
static void TryStartGame() {
    const char* path = g_selected ? BLUE_PATH : RED_PATH;
    const unsigned game = g_selected ? XPD_GAME_BLUE : XPD_GAME_RED;
    /* The launcher is quiet about package diagnostics. Recheck the actual
       selected file at launch so a newly copied/replaced pack is noticed. */
    g_status[g_selected] = PbrbInspectPack(path, game);
    if (g_status[g_selected] != PBRB_READY) {
        ShowDialog(DIALOG_RETRY_GAME,
            g_status[g_selected] == PBRB_MISSING ? "GAME DATA IS MISSING" : "GAME DATA IS INVALID",
            g_selected ? "Copy a valid blue.xpd to D:\\data\\blue.xpd." :
            "Copy a valid red.xpd to D:\\data\\red.xpd.",
            "Use the data folder beside default.xbe, then retry.");
        return;
    }
    /* Release the launcher's owned ROM before the engine allocates its
       game data, keeping the transition safe on a stock 64 MB Xbox. */
    PbrbAudioStopLauncherMusic();
    PbrbAudioUi(PBRB_AUDIO_CONFIRM);
    if (!PbrbEngineStart(path, game)) {
        PbrbEngineStop();
        StartLauncherMusic();
        ShowDialog(DIALOG_RETRY_GAME, "THE GAME COULD NOT START",
            "Check the game data and available memory, then retry.",
            g_selected ? "If needed, copy a fresh blue.xpd to D:\\data\\blue.xpd." :
            "If needed, copy a fresh red.xpd to D:\\data\\red.xpd.");
        return;
    }
    g_page = LAUNCHER_SELECT; g_ingame = 1;
    PbrbSetArtworkCanvas(640, 480);
    /* Only retain the selected frame: on stock 64 MB hardware loading
       both borders wastes texture memory. Original media is unchanged. */
    const char* borderPath = g_selected ? BLUE_BORDER : RED_BORDER;
    if (!PbrbLoadPNG(borderPath, &g_gameBorder))
        OutputDebugStringA("PokeRBXB: selected border load failed\n");
    ConfigureGameBorder();
}
static void ScanPacks() {
    g_status[0] = PbrbInspectPack(RED_PATH, XPD_GAME_RED);
    g_status[1] = PbrbInspectPack(BLUE_PATH, XPD_GAME_BLUE);
}
static void ArtworkRect(float x, float y, float w, float h, DWORD c) {
    PbrbPoint p = PbrbPt(x, y); const float s = PbrbScale();
    PbrbRectangle(p.x, p.y, w * s, h * s, c);
}
static void ArtworkBorder(float x, float y, float w, float h, DWORD c, float thick) {
    PbrbPoint a = PbrbPt(x, y), b = PbrbPt(x + w, y + h);
    const float t = thick * PbrbScale();
    PbrbLine(a.x, a.y, b.x, a.y, t, c);
    PbrbLine(b.x, a.y, b.x, b.y, t, c);
    PbrbLine(b.x, b.y, a.x, b.y, t, c);
    PbrbLine(a.x, b.y, a.x, a.y, t, c);
}
static void DrawSlot(int index) {
    const float cx = index ? 1230.0f : 430.0f;
    const float left = index ? 896.0f : 78.0f;
    const float right = index ? 1552.0f : 762.0f;
    const DWORD emphasis = index ? 0xFF529CFF : 0xFFFF6767;
    const char* name = index ? "POKEMON BLUE" : "POKEMON RED";
    if (index == g_selected) {
        ArtworkRect(left + 8, 136, right - left - 16, 525, index ? 0x172C63C8 : 0x17B83B3B);
        ArtworkBorder(left + 18, 143, right - left - 36, 515, emphasis, 4.0f);
        // Distinct short corner accents without obscuring the illustrated frame.
        ArtworkRect(left + 21, 146, 83, 6, emphasis);
        ArtworkRect(right - 104, 146, 83, 6, emphasis);
    }
    PbrbTextCenter(cx, 340, name, 2, 3.1f, 0xB9000000); // offset shadow below
    PbrbTextCenter(cx, 335, name, 2, 3.1f, index == g_selected ? 0xFFFFFFFF : 0xFFD7DCEA);
}
static void DrawNowPlaying() {
    if (g_page != LAUNCHER_SELECT)return;
    const char* name = PbrbAudioLauncherTrackName();
    if (!name || !name[0])return;
    /* Keep the current song between the game cards and the toast/footer.
       DDF glyph bounds are taller than their nominal tier size. */
    float scale = 1.8f;
    const int width = PbrbTextWidth(name, 0, scale);
    if (width > 1100)scale *= 1100.0f / (float)width;
    PbrbTextCenter(836, 670, "NOW PLAYING", 0, 1.1f, 0xFFB4C7DE);
    PbrbTextCenter(838, 699, name, 0, scale, 0xC0000000);
    PbrbTextCenter(836, 697, name, 0, scale, 0xFFF1F4FA);
}
static void DrawFooter() {
    // Embedded controller symbols remain legible with any DDF font.
    PbrbButtonGlyph(376.0f, 809.0f, 'A'); PbrbText(407.0f, 789.0f, "PLAY", 0, 2.0f, 0xFFE3EAF2);
    PbrbButtonGlyph(699.0f, 809.0f, 'X'); PbrbText(730.0f, 789.0f, "OPTIONS", 0, 2.0f, 0xFFE3EAF2);
    PbrbButtonGlyph(1014.0f, 809.0f, 'B'); PbrbText(1045.0f, 789.0f, "EXIT", 0, 2.0f, 0xFFE3EAF2);
    if (g_message[0] && (DWORD)(GetTickCount() - g_message_start) < 4500u) {
        ArtworkRect(338, 743, 996, 42, 0xB2080D15);
        PbrbTextCenter(836, 751, g_message, 0, 1.7f, 0xFFE8EBF4);
    }
}
static void DrawModal() {
    ArtworkRect(0, 0, 1672, 941, 0xB8000000);
    ArtworkRect(230, 205, 1212, 520, 0xF00C1527);
    ArtworkBorder(230, 205, 1212, 520, 0xFF91BDE8, 3.0f);
    if (g_page == LAUNCHER_OPTIONS) {
        PbrbTextCenter(836, 255, "OPTIONS", 2, 2.8f, 0xFFFFFFFF);
        const char* labels[3] = { "PIXEL FILTERING", "CONTROLLER RUMBLE", "COLOR GRAPHICS" };
        const char* values[3] = { g_options.smooth ? "SMOOTH" : "SHARP",
            g_options.rumble ? "ON" : "OFF", g_options.color ? "GBC PALETTE" : "ORIGINAL" };
        for (int i = 0; i < 3; ++i) {
            const float y = 355.0f + i * 78.0f;
            if (i == g_option_row)ArtworkRect(292, y - 12, 1088, 60, 0xFF233F60);
            PbrbText(332, y, labels[i], 0, 1.9f, 0xFFE3EAF2);
            PbrbTextCenter(1155, y, values[i], 1, 2.0f,
                i == g_option_row ? 0xFFFFFFFF : 0xFFB4C7DE);
        }
        PbrbTextCenter(836, 603, "Saved with your Red and Blue progress.", 0, 1.55f, 0xFFB4C7DE);
        PbrbButtonGlyph(515, 674, 'A'); PbrbText(546, 654, "CHANGE", 0, 1.8f, 0xFFE3EAF2);
        PbrbButtonGlyph(939, 674, 'B'); PbrbText(970, 654, "SAVE / BACK", 0, 1.8f, 0xFFE3EAF2);
    }
    else {
        PbrbTextCenter(836, 285, g_dialog_title, 1, 2.35f, 0xFFFFFFFF);
        PbrbTextCenter(836, 405, g_dialog_line1, 0, 1.65f, 0xFFE3EAF2);
        PbrbTextCenter(836, 465, g_dialog_line2, 0, 1.65f, 0xFFE3EAF2);
        PbrbButtonGlyph(562, 650, 'A'); PbrbText(593, 630, "RETRY", 0, 1.8f, 0xFFE3EAF2);
        PbrbButtonGlyph(947, 650, 'B'); PbrbText(978, 630, "BACK", 0, 1.8f, 0xFFE3EAF2);
    }
}
static void Render() {
    if (g_ingame) {
        PbrbBegin();
        if (g_gameBorder.gpu)
            PbrbSpriteUV(&g_gameBorder, 0, 0,
                (float)PbrbScreenWidth(), (float)PbrbScreenHeight(),
                g_borderU0, g_borderV0, g_borderU1, g_borderV1, 0xFFFFFFFF);
        // Draw the original framebuffer last, inside the measured opening.
        PbrbEngineDraw();
        PbrbEnd();
        return;
    }
    PbrbBegin();
    if (g_bg.gpu) {
        PbrbPoint p = PbrbPt(0, 0); const float s = PbrbScale();
        PbrbSprite(&g_bg, p.x, p.y, 1672.0f * s, 941.0f * s, 0xFFFFFFFF);
    }
    else {
        ArtworkRect(0, 0, 836, 941, 0xFF330D16); ArtworkRect(836, 0, 836, 941, 0xFF0D1A40);
    }
    DrawSlot(0); DrawSlot(1); DrawNowPlaying(); DrawFooter();
    if (g_page != LAUNCHER_SELECT)DrawModal();
    PbrbEnd();
}
void __cdecl main() {
    if (!PbrbVideoInit())return;
    PbrbSetArtworkCanvas(1672, 941);
    PbrbInputInit();
    PbrbLoadPreferences();
    ApplyPreferences();
    const int soundReady = PbrbAudioInit();
    if (!soundReady)OutputDebugStringA("PokeRBXB: DirectSound unavailable (continuing silent)\n");
    if (!PbrbLoadPNG(BG_PATH, &g_bg))OutputDebugStringA("PokeRBXB: background load failed\n");
    if (!PbrbFontLoad(FONT_PATH))OutputDebugStringA("PokeRBXB: font.DDF missing/invalid\n");
    ScanPacks();
    StartLauncherMusic();
    for (;;) {
        const PbrbEvents input = PbrbInputPoll();
        if (g_ingame) {
            if (PbrbEngineHandle(&input)) {
                PbrbEngineStop();
                PbrbInputStopRumble();
                g_ingame = 0;
                PbrbReleaseTexture(&g_gameBorder);
                PbrbSetArtworkCanvas(1672, 941);
                StartLauncherMusic();
                SetMessage("RETURNED TO LAUNCHER");
            }
        }
        else if (g_page == LAUNCHER_DIALOG) {
            if (input.exit)g_page = LAUNCHER_SELECT;
            else if (input.accept) {
                if (g_dialog_action == DIALOG_RETRY_GAME)TryStartGame();
                else SaveOptions();
            }
        }
        else if (g_page == LAUNCHER_OPTIONS) {
            if (input.up)g_option_row = (g_option_row + 2) % 3;
            if (input.down)g_option_row = (g_option_row + 1) % 3;
            if (input.exit)SaveOptions();
            else if (input.left || input.right || input.accept) {
                if (g_option_row == 0)g_options.smooth = !g_options.smooth;
                else if (g_option_row == 1)g_options.rumble = !g_options.rumble;
                else g_options.color = !g_options.color;
                PbrbAudioUi(PBRB_AUDIO_MOVE);
            }
        }
        else {
            int previous = g_selected;
            if (input.left)g_selected = 0;
            if (input.right)g_selected = 1;
            if (previous != g_selected) {
                /* Keep the current song playing while changing selection. */
                PbrbAudioUi(PBRB_AUDIO_MOVE);
            }
            if (input.rescan) {
                g_options = *PbrbGetPreferences();
                g_option_row = 0; g_page = LAUNCHER_OPTIONS;
                PbrbAudioUi(PBRB_AUDIO_CONFIRM);
            }
            else if (input.accept)TryStartGame();
            else if (input.exit)break;
        }
        Render();
    }
    PbrbEngineStop();
    PbrbInputStopRumble();
    PbrbAudioShutdown();
    PbrbReleaseTexture(&g_bg);
    PbrbReleaseTexture(&g_gameBorder);
    PbrbFontShutdown();
    PbrbInputShutdown();
    PbrbVideoShutdown();
    // NULL target returns to the configured Xbox dashboard.
    XLaunchNewImage(NULL, NULL);
}
