#ifndef PBRB_INPUT_H_INCLUDED
#define PBRB_INPUT_H_INCLUDED

/* Match the original PAD_* bit positions. The native gameplay receives raw
   held/edge states and applies its own frame-based input repeat/movement. */
enum PbrbPadKey {
    PBRB_PAD_RIGHT = 1, PBRB_PAD_LEFT = 2, PBRB_PAD_UP = 4,
    PBRB_PAD_DOWN = 8, PBRB_PAD_A = 16, PBRB_PAD_B = 32,
    PBRB_PAD_SELECT = 64, PBRB_PAD_START = 128
};
struct PbrbEvents {
    /* Legacy launcher/menu events. rescan is now the X / Options edge. */
    int left, right, up, down, accept, rescan, exit;
    unsigned held, pressed, released;
};
void PbrbInputInit();
void PbrbInputShutdown();
PbrbEvents PbrbInputPoll();
void PbrbInputSetRumbleEnabled(int enabled);
void PbrbInputRumble(unsigned leftMotor, unsigned rightMotor, unsigned milliseconds);
void PbrbInputStopRumble();

#endif
