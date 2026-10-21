#include "pbrb_flow.h"
#include <string.h>
/* This remains the pure state manager; the original game renders using the
   ROM-backed PbrbGb module (never the launcher DDF font). */
void PbrbFlowInit(PbrbFlow* f, unsigned game) {
    if (!f)return;
    memset(f, 0, sizeof(*f)); f->screen = PBRB_SCREEN_INTRO;
    f->musicEnabled = 1; f->effectsEnabled = 1; f->battleAnimation = 1; f->textSpeed = 1; f->game = game;
}
void PbrbFlowDispatch(PbrbFlow* f, int action) {
    if (!f)return;
    switch (f->screen) {
    case PBRB_SCREEN_INTRO:
        if (action == PBRB_ACTION_CONFIRM || action == PBRB_ACTION_BACK)
            f->screen = PBRB_SCREEN_TITLE;
        break;
    case PBRB_SCREEN_TITLE:
        if (action == PBRB_ACTION_CONFIRM) { f->screen = PBRB_SCREEN_MENU; f->selected = 0; }
        break;
    case PBRB_SCREEN_MENU:
        if (action == PBRB_ACTION_UP)f->selected = (f->selected + (f->hasSave ? 2 : 1)) % (f->hasSave ? 3 : 2);
        else if (action == PBRB_ACTION_DOWN)f->selected = (f->selected + 1) % (f->hasSave ? 3 : 2);
        else if (action == PBRB_ACTION_BACK) { f->screen = PBRB_SCREEN_TITLE; f->selected = 0; }
        else if (action == PBRB_ACTION_CONFIRM) {
            int choice = f->selected + (f->hasSave ? 0 : 1);
            f->screen = choice == 0 ? PBRB_SCREEN_CONTINUE : choice == 1 ? PBRB_SCREEN_NEW_GAME : PBRB_SCREEN_OPTIONS;
            f->selected = 0;
        }
        break;
    case PBRB_SCREEN_OPTIONS:
        /* Game Boy options rows 3,8,13,16; 1:1 menu layout derived from
           engine/menus/main_menu.asm DisplayOptionMenu. */
        if (action == PBRB_ACTION_UP)f->selected = (f->selected + 3) % 4;
        else if (action == PBRB_ACTION_DOWN)f->selected = (f->selected + 1) % 4;
        else if (action == PBRB_ACTION_LEFT || action == PBRB_ACTION_RIGHT) {
            if (f->selected == 0)f->textSpeed = (f->textSpeed + (action == PBRB_ACTION_RIGHT ? 1 : 2)) % 3;
            else if (f->selected == 1)f->battleAnimation = !f->battleAnimation;
            else if (f->selected == 2)f->battleStyle = !f->battleStyle;
        }
        else if (action == PBRB_ACTION_BACK ||
            (action == PBRB_ACTION_CONFIRM && f->selected == 3)) {
            f->screen = PBRB_SCREEN_MENU; f->selected = f->hasSave ? 2 : 1;
        }
        break;
    }
}
