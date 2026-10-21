#include <xtl.h>
#include "pbrb_input.h"
#include <string.h>

static HANDLE g_pads[4] = { 0,0,0,0 };
static int g_prev_x = 0, g_prev_y = 0;
static DWORD g_last_x = 0, g_last_y = 0;
static DWORD g_prev_keys = 0;
static unsigned g_prev_held = 0;
static int g_last_port = -1;
static int g_rumble_enabled = 0;

/* Feedback must outlive XInputSetState: RXDK can finish it asynchronously.
   Each pad owns its own buffer; never clear or reuse a pending request. */
static XINPUT_FEEDBACK g_feedback[4];
static unsigned g_motor_left[4], g_motor_right[4];
static unsigned g_sent_left[4], g_sent_right[4];
static DWORD g_rumble_start[4], g_rumble_duration[4];

static void PumpRumble() {
    const DWORD now = GetTickCount();
    for (int i = 0; i < 4; ++i) {
        if (!g_pads[i])continue;
        if (!g_rumble_enabled || (g_rumble_duration[i] &&
            (DWORD)(now - g_rumble_start[i]) >= g_rumble_duration[i])) {
            g_motor_left[i] = g_motor_right[i] = 0;
            g_rumble_duration[i] = 0;
        }
        if (g_feedback[i].Header.dwStatus == ERROR_IO_PENDING)continue;
        if (g_motor_left[i] == g_sent_left[i] &&
            g_motor_right[i] == g_sent_right[i])continue;
        ZeroMemory(&g_feedback[i], sizeof(g_feedback[i]));
        g_feedback[i].Rumble.wLeftMotorSpeed = (WORD)g_motor_left[i];
        g_feedback[i].Rumble.wRightMotorSpeed = (WORD)g_motor_right[i];
        const DWORD result = XInputSetState(g_pads[i], &g_feedback[i]);
        if (result == ERROR_SUCCESS || result == ERROR_IO_PENDING) {
            g_sent_left[i] = g_motor_left[i];
            g_sent_right[i] = g_motor_right[i];
        }
    }
}
void PbrbInputStopRumble() {
    for (int i = 0; i < 4; ++i) {
        g_motor_left[i] = g_motor_right[i] = 0;
        g_rumble_duration[i] = 0;
    }
    PumpRumble();
}
void PbrbInputSetRumbleEnabled(int enabled) {
    g_rumble_enabled = enabled != 0;
    if (!g_rumble_enabled)PbrbInputStopRumble();
}
void PbrbInputRumble(unsigned left, unsigned right, unsigned milliseconds) {
    if (!g_rumble_enabled || g_last_port < 0 || !g_pads[g_last_port])return;
    if (!milliseconds || (!left && !right)) { PbrbInputStopRumble(); return; }
    const int p = g_last_port;
    g_motor_left[p] = left < 65536u ? left : 65535u;
    g_motor_right[p] = right < 65536u ? right : 65535u;
    /* Bounded pulses cannot leave a motor running if a caller supplies a
       corrupt duration. The regular input poll stops an expired pulse. */
    g_rumble_duration[p] = milliseconds < 1000u ? milliseconds : 1000u;
    g_rumble_start[p] = GetTickCount();
    PumpRumble();
}
void PbrbInputInit() {
    XDEVICE_PREALLOC_TYPE t;
    ZeroMemory(&t, sizeof(t)); t.DeviceType = XDEVICE_TYPE_GAMEPAD; t.dwPreallocCount = 4;
    XInitDevices(1, &t);
    memset(g_pads, 0, sizeof(g_pads));
    memset(g_feedback, 0, sizeof(g_feedback));
    memset(g_motor_left, 0, sizeof(g_motor_left));
    memset(g_motor_right, 0, sizeof(g_motor_right));
    memset(g_sent_left, 0, sizeof(g_sent_left));
    memset(g_sent_right, 0, sizeof(g_sent_right));
    memset(g_rumble_duration, 0, sizeof(g_rumble_duration));
    g_prev_x = g_prev_y = 0; g_prev_keys = g_prev_held = 0;
    g_last_x = g_last_y = 0; g_last_port = -1; g_rumble_enabled = 0;
}
void PbrbInputShutdown() {
    PbrbInputStopRumble();
    for (int i = 0; i < 4; ++i)if (g_pads[i]) {
        /* Close cancels outstanding feedback. Static buffers remain alive
           until the application exits, including an in-flight request. */
        XInputClose(g_pads[i]); g_pads[i] = NULL;
    }
    g_prev_held = g_prev_keys = 0; g_last_port = -1;
}
PbrbEvents PbrbInputPoll() {
    PbrbEvents e; memset(&e, 0, sizeof(e));
    const DWORD present = XGetDevices(XDEVICE_TYPE_GAMEPAD);
    int port = -1;
    for (int i = 0; i < 4; ++i) {
        const int inserted = (present & (1u << i)) != 0;
        if (!inserted && g_pads[i]) {
            XInputClose(g_pads[i]); g_pads[i] = NULL;
            g_motor_left[i] = g_motor_right[i] = 0;
            g_sent_left[i] = g_sent_right[i] = 0;
            g_rumble_duration[i] = 0;
        }
        if (inserted && !g_pads[i]) {
            /* XInputClose has completed before this buffer can be reused. */
            ZeroMemory(&g_feedback[i], sizeof(g_feedback[i]));
            g_pads[i] = XInputOpen(XDEVICE_TYPE_GAMEPAD, i, XDEVICE_NO_SLOT, NULL);
        }
        if (port < 0 && g_pads[i])port = i;
    }
    if (port != g_last_port) {
        PbrbInputStopRumble();
        e.released = g_prev_held;
        g_prev_keys = g_prev_held = 0; g_prev_x = g_prev_y = 0;
        g_last_port = port;
    }
    PumpRumble();
    if (port < 0)return e;
    XINPUT_STATE st; ZeroMemory(&st, sizeof(st));
    if (XInputGetState(g_pads[port], &st) != ERROR_SUCCESS) {
        e.released |= g_prev_held;
        g_prev_keys = g_prev_held = 0; g_prev_x = g_prev_y = 0;
        PbrbInputStopRumble();
        return e;
    }
    const DWORD b = st.Gamepad.wButtons;
    const BYTE* a = st.Gamepad.bAnalogButtons;
    DWORD keys = 0;
    if (a[XINPUT_GAMEPAD_A] > 30 || (b & 0x1000))keys |= 1;
    if (a[XINPUT_GAMEPAD_B] > 30 || (b & 0x2000))keys |= 2;
    if (a[XINPUT_GAMEPAD_X] > 30 || (b & 0x4000))keys |= 4;
    if (b & XINPUT_GAMEPAD_START)keys |= 8;
    e.accept = ((keys & 1) && !(g_prev_keys & 1)) || ((keys & 8) && !(g_prev_keys & 8));
    e.exit = (keys & 2) && !(g_prev_keys & 2);
    e.rescan = (keys & 4) && !(g_prev_keys & 4);
    g_prev_keys = keys;
    int dirx = 0, diry = 0;
    if ((b & XINPUT_GAMEPAD_DPAD_LEFT) || st.Gamepad.sThumbLX < -16000)dirx = -1;
    else if ((b & XINPUT_GAMEPAD_DPAD_RIGHT) || st.Gamepad.sThumbLX > 16000)dirx = 1;
    if ((b & XINPUT_GAMEPAD_DPAD_UP) || st.Gamepad.sThumbLY > 16000)diry = -1;
    else if ((b & XINPUT_GAMEPAD_DPAD_DOWN) || st.Gamepad.sThumbLY < -16000)diry = 1;

    /* Gameplay edges are independent of the launcher's time-based repeat.
       Start and Back remain distinct from A and B for the original menus. */
    if (dirx < 0)e.held |= PBRB_PAD_LEFT;
    if (dirx > 0)e.held |= PBRB_PAD_RIGHT;
    if (diry < 0)e.held |= PBRB_PAD_UP;
    if (diry > 0)e.held |= PBRB_PAD_DOWN;
    if (keys & 1)e.held |= PBRB_PAD_A;
    if (keys & 2)e.held |= PBRB_PAD_B;
    if (keys & 8)e.held |= PBRB_PAD_START;
    if (b & XINPUT_GAMEPAD_BACK)e.held |= PBRB_PAD_SELECT;
    e.pressed = e.held & ~g_prev_held;
    e.released |= g_prev_held & ~e.held;
    g_prev_held = e.held;

    const DWORD now = GetTickCount();
    if (dirx && (dirx != g_prev_x || (DWORD)(now - g_last_x) >= 180u)) {
        if (dirx < 0)e.left = 1; else e.right = 1;
        g_last_x = now;
    }
    if (diry && (diry != g_prev_y || (DWORD)(now - g_last_y) >= 180u)) {
        if (diry < 0)e.up = 1; else e.down = 1;
        g_last_y = now;
    }
    g_prev_x = dirx; g_prev_y = diry;
    return e;
}
