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
static const char* INTRO_PATH = "D:\\data\\xmv\\intro.xmv";
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
static int g_launcher_fade = 0, g_launcher_fade_first = 0;
static DWORD g_launcher_fade_start = 0;
static const DWORD LAUNCHER_FADE_MS = 300u;
static char g_message[80] = "";
static DWORD g_message_start = 0;
enum LauncherPage {
    LAUNCHER_SELECT, LAUNCHER_OPTIONS, LAUNCHER_DISPLAY, LAUNCHER_AUDIO,
    LAUNCHER_CONTROLLER, LAUNCHER_CONTROLS, LAUNCHER_CALIBRATION, LAUNCHER_DIALOG
};
enum DialogAction { DIALOG_RETRY_GAME, DIALOG_RETRY_OPTIONS };
static LauncherPage g_page = LAUNCHER_SELECT;
static DialogAction g_dialog_action = DIALOG_RETRY_GAME;
static int g_option_row = 0, g_category_row = 0;
static PbrbPreferences g_options = { 0 };
static PbrbSaveSummary g_saveSummary[2];
static int g_saveSummaryStatus[2] = { 0, 0 };
static int g_controller_pause = 0, g_controller_connected = 0, g_controller_armed = 0;
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
/* Global choices belong to the Xbox launcher. These setters configure
   presentation/input only; the original game's OPTION menu is untouched. */
static void ApplyOptionPreview(const PbrbPreferences* p) {
    PbrbSetSmoothFiltering(p->smooth);
    PbrbSetScreenCalibration(p->screenScale, p->screenX, p->screenY);
    PbrbInputSetRumbleLevel(p->rumble);
    PbrbInputSetDpadOnly(p->dpadOnly);
    PbrbAudioSetVolume(p->volume);
    PbrbAudioSetEffects(p->effects);
    PbrbAudioSetMusic(g_ingame ? p->music : p->launcherMusic);
}
static void ApplyPreferences() { ApplyOptionPreview(PbrbGetPreferences()); }
static void RefreshSaveSummaries() {
    g_saveSummaryStatus[0] = PbrbReadSaveSummary(XPD_GAME_RED, &g_saveSummary[0]);
    g_saveSummaryStatus[1] = PbrbReadSaveSummary(XPD_GAME_BLUE, &g_saveSummary[1]);
}
/* Small formatting helpers avoid newer CRT formatted-I/O imports in RXDK. */
static void AppendLabel(char* out, unsigned size, const char* text) {
    unsigned n = 0;
    while (n < size && out[n])++n;
    if (n < size)CopyLabel(out + n, size - n, text);
}
static void NumberLabel(char* out, unsigned size, unsigned value) {
    char reversed[11]; unsigned n = 0, i = 0;
    do { reversed[n++] = (char)('0' + value % 10u); value /= 10u; } while (value && n < sizeof(reversed));
    if (!size)return;
    while (n && i + 1 < size)out[i++] = reversed[--n];
    out[i] = 0;
}
static void PercentLabel(char* out, unsigned size, int value, int signedValue) {
    char number[16];
    NumberLabel(number, sizeof(number), (unsigned)(value < 0 ? -value : value));
    CopyLabel(out, size, value < 0 ? "-" : signedValue && value > 0 ? "+" : "");
    AppendLabel(out, size, number); AppendLabel(out, size, " PERCENT");
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
    PbrbAudioSetMusic(PbrbGetPreferences()->launcherMusic);
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
    PbrbAudioSetMusic(PbrbGetPreferences()->music);
    PbrbAudioSetPaused(0);
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
    g_page = LAUNCHER_SELECT; g_ingame = 1; g_controller_pause = g_controller_armed = 0;
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
static void DrawSaveSummary(int index, float cx) {
    if (g_saveSummaryStatus[index] <= 0) {
        PbrbTextCenter(cx, 510, g_saveSummaryStatus[index] ?
            "SAVE DETAILS UNAVAILABLE" : "NEW ADVENTURE", 0,
            g_saveSummaryStatus[index] ? 1.55f : 1.95f, 0xFFD0DAEA);
        return;
    }
    const PbrbSaveSummary* save = &g_saveSummary[index];
    char badges[32], time[32], number[16];
    CopyLabel(badges, sizeof(badges), "BADGES  ");
    NumberLabel(number, sizeof(number), save->badges);
    AppendLabel(badges, sizeof(badges), number); AppendLabel(badges, sizeof(badges), " / 8");
    CopyLabel(time, sizeof(time), "PLAY TIME  ");
    NumberLabel(number, sizeof(number), save->hours); AppendLabel(time, sizeof(time), number);
    AppendLabel(time, sizeof(time), save->minutes < 10u ? ":0" : ":");
    NumberLabel(number, sizeof(number), save->minutes); AppendLabel(time, sizeof(time), number);
    ArtworkRect(cx - 246, 446, 492, 171, 0x65060B12);
    PbrbTextCenter(cx, 457, save->trainer, 1, 1.9f, 0xFFFFFFFF);
    PbrbTextCenter(cx, 515, badges, 0, 1.7f, 0xFFD4E0EE);
    PbrbTextCenter(cx, 562, time, 0, 1.7f, 0xFFD4E0EE);
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
    DrawSaveSummary(index, cx);
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
    if (g_page != LAUNCHER_SELECT)return;
    // Four compact groups remain inside the approved artwork at SD output.
    PbrbButtonGlyph(288, 809, 'A'); PbrbText(319, 789, "PLAY", 0, 1.8f, 0xFFE3EAF2);
    PbrbButtonGlyph(570, 809, 'X'); PbrbText(601, 789, "OPTIONS", 0, 1.8f, 0xFFE3EAF2);
    PbrbButtonGlyph(878, 809, 'Y'); PbrbText(909, 789, "NEXT SONG", 0, 1.8f, 0xFFE3EAF2);
    PbrbButtonGlyph(1238, 809, 'B'); PbrbText(1269, 789, "EXIT", 0, 1.8f, 0xFFE3EAF2);
    if (g_message[0] && (DWORD)(GetTickCount() - g_message_start) < 4500u) {
        ArtworkRect(338, 743, 996, 42, 0xB2080D15);
        PbrbTextCenter(836, 751, g_message, 0, 1.7f, 0xFFE8EBF4);
    }
}
static int OptionCount() {
    if (g_page == LAUNCHER_DISPLAY)return 3;
    if (g_page == LAUNCHER_CONTROLLER)return 2;
    return 4;
}
static void DrawOptionRow(int row, const char* label, const char* value) {
    const float y = 335.0f + row * 70.0f;
    if (row == g_option_row)ArtworkRect(292, y - 12, 1088, 58, 0xFF233F60);
    PbrbText(332, y, label, 0, 1.9f, 0xFFE3EAF2);
    float scale = 1.85f;
    const int width = PbrbTextWidth(value, 0, scale);
    if (width > 365)scale *= 365.0f / (float)width;
    PbrbTextCenter(1160, y, value, 0, scale,
        row == g_option_row ? 0xFFFFFFFF : 0xFFB4C7DE);
}
static void DrawControls() {
    PbrbText(332, 330, g_options.dpadOnly ? "D-PAD" : "D-PAD / LEFT STICK", 0, 1.8f, 0xFFE3EAF2);
    PbrbText(924, 330, "MOVE / NAVIGATE", 0, 1.8f, 0xFFB4C7DE);
    PbrbButtonGlyph(350, 403, 'A'); PbrbText(386, 383, "CONFIRM / INTERACT", 0, 1.8f, 0xFFE3EAF2);
    PbrbButtonGlyph(965, 403, 'B'); PbrbText(1001, 383, "CANCEL / BACK", 0, 1.8f, 0xFFE3EAF2);
    PbrbText(332, 436, "START", 0, 1.8f, 0xFFE3EAF2);
    PbrbText(924, 436, "GAME MENU", 0, 1.8f, 0xFFB4C7DE);
    PbrbText(332, 489, "BACK", 0, 1.8f, 0xFFE3EAF2);
    PbrbText(924, 489, "SELECT / REORDER", 0, 1.8f, 0xFFB4C7DE);
    PbrbText(332, 542, "BACK + START", 0, 1.8f, 0xFFE3EAF2);
    PbrbText(924, 542, "RETURN TO LAUNCHER", 0, 1.65f, 0xFFB4C7DE);
    PbrbButtonGlyph(350, 615, 'Y'); PbrbText(386, 595, "NEXT SONG IN LAUNCHER", 0, 1.8f, 0xFFE3EAF2);
}
static void DrawCalibrationGuides() {
    const float w = (float)PbrbScreenWidth(), h = (float)PbrbScreenHeight();
    const DWORD color = 0xFF90DAFF;
    PbrbLine(8, 8, w - 8, 8, 2, color); PbrbLine(w - 8, 8, w - 8, h - 8, 2, color);
    PbrbLine(w - 8, h - 8, 8, h - 8, 2, color); PbrbLine(8, h - 8, 8, 8, 2, color);
}
static void DrawModal() {
    ArtworkRect(0, 0, 1672, 941, 0xB8000000);
    ArtworkRect(230, 145, 1212, 655, 0xF00C1527);
    ArtworkBorder(230, 145, 1212, 655, 0xFF91BDE8, 3.0f);
    if (g_page == LAUNCHER_DIALOG) {
        PbrbTextCenter(836, 285, g_dialog_title, 1, 2.35f, 0xFFFFFFFF);
        PbrbTextCenter(836, 405, g_dialog_line1, 0, 1.65f, 0xFFE3EAF2);
        PbrbTextCenter(836, 465, g_dialog_line2, 0, 1.65f, 0xFFE3EAF2);
        PbrbButtonGlyph(562, 710, 'A'); PbrbText(593, 690, "RETRY", 0, 1.8f, 0xFFE3EAF2);
        PbrbButtonGlyph(947, 710, 'B'); PbrbText(978, 690, "BACK", 0, 1.8f, 0xFFE3EAF2);
        return;
    }
    const char* title = g_page == LAUNCHER_OPTIONS ? "OPTIONS" :
        g_page == LAUNCHER_DISPLAY ? "DISPLAY" : g_page == LAUNCHER_AUDIO ? "AUDIO" :
        g_page == LAUNCHER_CONTROLLER ? "CONTROLLER" :
        g_page == LAUNCHER_CALIBRATION ? "SCREEN CALIBRATION" : "CONTROLS";
    PbrbTextCenter(836, 190, title, 2, g_page == LAUNCHER_CALIBRATION ? 2.1f : 2.5f, 0xFFFFFFFF);
    PbrbTextCenter(836, 280, "SHARED BY RED AND BLUE", 0, 1.45f, 0xFFB4C7DE);
    char value[24];
    if (g_page == LAUNCHER_OPTIONS) {
        const char* labels[4] = { "DISPLAY", "AUDIO", "CONTROLLER", "CONTROLS" };
        for (int i = 0; i < 4; ++i)DrawOptionRow(i, labels[i], "OPEN");
    }
    else if (g_page == LAUNCHER_DISPLAY) {
        DrawOptionRow(0, "PIXEL FILTERING", g_options.smooth ? "SMOOTH" : "SHARP");
        DrawOptionRow(1, "COLOR GRAPHICS", g_options.color ? "GBC PALETTE" : "ORIGINAL");
        DrawOptionRow(2, "SCREEN CALIBRATION", "OPEN");
    }
    else if (g_page == LAUNCHER_AUDIO) {
        DrawOptionRow(0, "LAUNCHER MUSIC", g_options.launcherMusic ? "ON" : "OFF");
        DrawOptionRow(1, "GAME MUSIC", g_options.music ? "ON" : "OFF");
        DrawOptionRow(2, "SOUND EFFECTS", g_options.effects ? "ON" : "OFF");
        PercentLabel(value, sizeof(value), g_options.volume * 10, 0);
        DrawOptionRow(3, "MASTER VOLUME", value);
    }
    else if (g_page == LAUNCHER_CONTROLLER) {
        DrawOptionRow(0, "RUMBLE", g_options.rumble == 0 ? "OFF" : g_options.rumble == 1 ? "LOW" : "NORMAL");
        DrawOptionRow(1, "MOVEMENT", g_options.dpadOnly ? "D-PAD ONLY" : "D-PAD + STICK");
    }
    else if (g_page == LAUNCHER_CALIBRATION) {
        PercentLabel(value, sizeof(value), g_options.screenScale, 0); DrawOptionRow(0, "SCREEN SIZE", value);
        PercentLabel(value, sizeof(value), g_options.screenX, 1); DrawOptionRow(1, "HORIZONTAL POSITION", value);
        PercentLabel(value, sizeof(value), g_options.screenY, 1); DrawOptionRow(2, "VERTICAL POSITION", value);
        DrawOptionRow(3, "RESET DISPLAY", "FULL / CENTER");
    }
    else DrawControls();
    if (g_page != LAUNCHER_CONTROLS) {
        PbrbTextCenter(836, 643, g_page == LAUNCHER_CALIBRATION ?
            "Adjust until all four guide edges are visible." :
            "Changes are saved when you leave Options.", 0, 1.5f, 0xFFB4C7DE);
        PbrbButtonGlyph(480, 739, 'A'); PbrbText(511, 719,
            g_page == LAUNCHER_OPTIONS ? "OPEN" : "CHANGE", 0, 1.8f, 0xFFE3EAF2);
    }
    PbrbButtonGlyph(g_page == LAUNCHER_CONTROLS ? 735.0f : 939.0f, 739, 'B');
    PbrbText(g_page == LAUNCHER_CONTROLS ? 766.0f : 970.0f, 719,
        g_page == LAUNCHER_OPTIONS ? "SAVE / BACK" : "BACK", 0, 1.8f, 0xFFE3EAF2);
}
static void OptionBack() {
    if (g_page == LAUNCHER_OPTIONS)SaveOptions();
    else if (g_page == LAUNCHER_CALIBRATION) { g_page = LAUNCHER_DISPLAY; g_option_row = 2; }
    else { g_page = LAUNCHER_OPTIONS; g_option_row = g_category_row; }
}
static int ClampValue(int value, int low, int high) { return value < low ? low : value > high ? high : value; }
static void HandleOptions(const PbrbEvents* input) {
    if (input->exit) { OptionBack(); return; }
    if (g_page == LAUNCHER_CONTROLS)return;
    const int count = OptionCount();
    if (input->up)g_option_row = (g_option_row + count - 1) % count;
    if (input->down)g_option_row = (g_option_row + 1) % count;
    if (!(input->left || input->right || input->accept))return;
    const int delta = input->left ? -1 : 1;
    if (g_page == LAUNCHER_OPTIONS) {
        if (!input->accept)return;
        const LauncherPage pages[4] = { LAUNCHER_DISPLAY, LAUNCHER_AUDIO, LAUNCHER_CONTROLLER, LAUNCHER_CONTROLS };
        g_category_row = g_option_row; g_page = pages[g_option_row]; g_option_row = 0;
    }
    else if (g_page == LAUNCHER_DISPLAY) {
        if (g_option_row == 0)g_options.smooth ^= 1;
        else if (g_option_row == 1)g_options.color ^= 1;
        else if (input->accept) { g_page = LAUNCHER_CALIBRATION; g_option_row = 0; }
    }
    else if (g_page == LAUNCHER_AUDIO) {
        if (g_option_row == 0)g_options.launcherMusic ^= 1;
        else if (g_option_row == 1)g_options.music ^= 1;
        else if (g_option_row == 2)g_options.effects ^= 1;
        else g_options.volume = ClampValue(g_options.volume + delta, 0, 10);
    }
    else if (g_page == LAUNCHER_CONTROLLER) {
        if (g_option_row == 0)g_options.rumble = (g_options.rumble + delta + 3) % 3;
        else g_options.dpadOnly ^= 1;
    }
    else if (g_page == LAUNCHER_CALIBRATION) {
        if (g_option_row == 0)g_options.screenScale = ClampValue(g_options.screenScale + delta, 80, 100);
        else if (g_option_row == 1)g_options.screenX = ClampValue(g_options.screenX + delta, -5, 5);
        else if (g_option_row == 2)g_options.screenY = ClampValue(g_options.screenY + delta, -5, 5);
        else if (input->accept) { g_options.screenScale = 100; g_options.screenX = g_options.screenY = 0; }
    }
    ApplyOptionPreview(&g_options);
    PbrbAudioUi(PBRB_AUDIO_MOVE);
}
static int ControllerNeutral(const PbrbEvents* input) {
    return !input->held && !input->accept && !input->exit && !input->rescan &&
        !input->nextSong && !input->left && !input->right && !input->up && !input->down;
}
static void DrawControllerPause() {
    PbrbSetArtworkCanvas(1672, 941);
    ArtworkRect(0, 0, 1672, 941, 0xB8000000);
    ArtworkRect(230, 260, 1212, 420, 0xF00C1527);
    ArtworkBorder(230, 260, 1212, 420, 0xFF91BDE8, 3.0f);
    PbrbTextCenter(836, 330, g_controller_connected ? "CONTROLLER CONNECTED" :
        "CONTROLLER DISCONNECTED", 1, 2.4f, 0xFFFFFFFF);
    PbrbTextCenter(836, 448, !g_controller_connected ? "Reconnect a controller to continue." :
        (g_controller_pause == 2 || !g_controller_armed) ? "Release the controls to continue." :
        "Your game is paused.", 0, 1.8f, 0xFFE3EAF2);
    if (g_controller_connected && g_controller_pause == 1 && g_controller_armed) {
        PbrbButtonGlyph(680, 593, 'A'); PbrbText(711, 573, "CONTINUE", 0, 2.0f, 0xFFE3EAF2);
    }
    PbrbSetArtworkCanvas(640, 480);
}
static void HandleGameInput(const PbrbEvents* input) {
    g_controller_connected = input->connected;
    if (!input->connected || input->controllerChanged) {
        if (!g_controller_pause) {
            PbrbEngineSetPaused(1); PbrbAudioSetPaused(1); PbrbInputStopRumble();
        }
        g_controller_pause = 1; g_controller_armed = 0; return;
    }
    if (g_controller_pause) {
        // A held across unplug/replug is reported as a new edge by XInput.
        // Require neutral controls first, then a fresh confirmation press.
        if (!g_controller_armed) {
            if (ControllerNeutral(input))g_controller_armed = 1;
            return;
        }
        if (g_controller_pause == 1 && (input->pressed & PBRB_PAD_A))g_controller_pause = 2;
        else if (g_controller_pause == 2 && ControllerNeutral(input)) {
            // Consume reconnect/confirmation input before the game resumes.
            PbrbEngineSetPaused(0); PbrbAudioSetPaused(0); g_controller_pause = g_controller_armed = 0;
        }
        return;
    }
    if (PbrbEngineHandle(input)) {
        PbrbEngineStop(); PbrbInputStopRumble(); g_ingame = 0;
        PbrbReleaseTexture(&g_gameBorder); PbrbSetArtworkCanvas(1672, 941);
        RefreshSaveSummaries(); ApplyPreferences(); StartLauncherMusic();
        SetMessage("RETURNED TO LAUNCHER");
    }
}
static void DrawLauncherFade() {
    if (!g_launcher_fade)return;
    unsigned gain = 0;
    if (g_launcher_fade_first) {
        /* Start only when the fully prepared launcher is actually drawn.
           Its first presented frame is black, even after slow asset reads. */
        g_launcher_fade_first = 0;
        g_launcher_fade_start = GetTickCount();
    }
    else {
        const DWORD elapsed = (DWORD)(GetTickCount() - g_launcher_fade_start);
        if (elapsed >= LAUNCHER_FADE_MS)gain = 256u;
        else {
            const unsigned t = (unsigned)(elapsed * 256u / LAUNCHER_FADE_MS);
            gain = (t * t * (768u - 2u * t) + 32768u) / 65536u;
        }
    }
    PbrbAudioSetTransitionGain(gain);
    if (gain < 256u)PbrbVideoBlackFade(255u - gain * 255u / 256u);
    else g_launcher_fade = 0;
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
        if (g_controller_pause)DrawControllerPause();
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
    if (g_page == LAUNCHER_CALIBRATION)DrawCalibrationGuides();
    DrawLauncherFade();
    PbrbEnd();
}
void __cdecl main() {
    if (!PbrbVideoInit())return;
    PbrbSetArtworkCanvas(1672, 941);
    PbrbInputInit();
    PbrbLoadPreferences();
    /* Play once per application start, before PNG/font loads, pack scans or
       the synth worker compete with the movie for memory and disk time. */
    const PbrbPreferences* startupOptions = PbrbGetPreferences();
    PbrbSetScreenCalibration(startupOptions->screenScale,
        startupOptions->screenX, startupOptions->screenY);
    int introInputGuard = PbrbVideoPlayIntro(INTRO_PATH, startupOptions->volume) != 0;
    const int soundReady = PbrbAudioInit();
    ApplyPreferences();
    /* Silence queued launcher samples until its visual fade starts. This
       transient gain never changes the player's saved master volume. */
    PbrbAudioSetTransitionGain(introInputGuard ? 0u : 256u);
    if (!soundReady)OutputDebugStringA("PokeRBXB: DirectSound unavailable (continuing silent)\n");
    if (!PbrbLoadPNG(BG_PATH, &g_bg))OutputDebugStringA("PokeRBXB: background load failed\n");
    if (!PbrbFontLoad(FONT_PATH))OutputDebugStringA("PokeRBXB: font.DDF missing/invalid\n");
    ScanPacks();
    RefreshSaveSummaries();
    StartLauncherMusic();
    g_launcher_fade = g_launcher_fade_first = introInputGuard;
    for (;;) {
        const PbrbEvents input = PbrbInputPoll();
        if (g_launcher_fade || introInputGuard) {
            /* Consume input throughout the fade and until controls are
               released afterward. Disconnection cannot count as neutral. */
            if (!g_launcher_fade && input.connected && ControllerNeutral(&input))
                introInputGuard = 0;
            Render();
            continue;
        }
        if (g_ingame)HandleGameInput(&input);
        else if (g_page == LAUNCHER_DIALOG) {
            if (input.exit)g_page = LAUNCHER_SELECT;
            else if (input.accept) {
                if (g_dialog_action == DIALOG_RETRY_GAME)TryStartGame();
                else SaveOptions();
            }
        }
        else if (g_page != LAUNCHER_SELECT)HandleOptions(&input);
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
            else if (input.nextSong) {
                if (!PbrbAudioNextLauncherMusic())StartLauncherMusic();
                PbrbAudioUi(PBRB_AUDIO_MOVE);
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
