#ifndef PBRB_FLOW_H
#define PBRB_FLOW_H
/* Pure game-flow state: no Xbox headers, renderer or audio dependencies.
   State for original intro, title, native menus and opening-game entry. */
enum PbrbFlowScreen {
    PBRB_SCREEN_TITLE = 0,
    PBRB_SCREEN_MENU = 1,
    PBRB_SCREEN_OPTIONS = 2,
    PBRB_SCREEN_NEW_GAME = 3,
    PBRB_SCREEN_GAMEPLAY = 4,
    PBRB_SCREEN_INTRO = 5,
    PBRB_SCREEN_CONTINUE = 6
};
enum PbrbFlowAction {
    PBRB_ACTION_UP = 1, PBRB_ACTION_DOWN = 2, PBRB_ACTION_CONFIRM = 3,
    PBRB_ACTION_BACK = 4, PBRB_ACTION_DEBUG = 5,
    PBRB_ACTION_LEFT = 6, PBRB_ACTION_RIGHT = 7
};
struct PbrbFlow {
    int screen, selected, musicEnabled, effectsEnabled;
    int textSpeed, battleAnimation, battleStyle;
    int hasSave, saveLoadError;
    unsigned game;
};
void PbrbFlowInit(PbrbFlow* f, unsigned game);
void PbrbFlowDispatch(PbrbFlow* f, int action);
#endif
