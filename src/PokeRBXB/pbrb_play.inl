/* Included by pbrb_engine.cpp. This is native C++98 state and rendering;
   no emulator, interpreter for machine code, desktop runtime, or extra media. */
#include "pbrb_play.h"
#include "pbrb_rng.h"
#include "pbrb_play_data.h"
#include "pbrb_game_text.h"
#include "pbrb_audio.h"
#include "pbrb_audio_core.h"
#include "pbrb_packages.h"
#include "pbrb_input.h"
#include <string.h>
#include "pbrb_game.inl"
#include "pbrb_battle.inl"
#include "pbrb_battle_anim.inl"

static int PbrbPlayServiceBegin(PbrbPlay*, const PbrbWorldEvent*);
static int PbrbPlayServiceTick(PbrbPlay*, unsigned);
static int PbrbPlayServiceRender(PbrbPlay*);
static void PbrbPlayServiceForeground(PbrbPlay*);
static const unsigned char* PbrbPlayServiceRam(PbrbPlay*, unsigned);
static void PbrbPlayServiceNamed(PbrbPlay*);
static int PbrbPlayDialogueBegin(PbrbPlay*, const PbrbWorldEvent*);
static int PbrbPlayDialogueTick(PbrbPlay*, unsigned);
static int PbrbPlayDialogueRender(PbrbPlay*);
static void PbrbPlayDialogueNamed(PbrbPlay*);
static void PplayCreditsStart(PbrbPlay*);
static void PplayCreditsTick(PbrbPlay*, unsigned);
static void PplayCreditsRender(PbrbPlay*);
static unsigned PplayHealthPixels(const PbrbPlayMon*);

enum {
    PPLAY_AFTER_NONE, PPLAY_AFTER_WORLD, PPLAY_AFTER_STARTER,
    PPLAY_AFTER_NICKNAME, PPLAY_AFTER_SAVE, PPLAY_AFTER_SAVED,
    PPLAY_AFTER_OVERWRITE, PPLAY_AFTER_NOTICE, PPLAY_AFTER_PC,
    PPLAY_AFTER_PC_WITHDRAW, PPLAY_AFTER_BATTLE, PPLAY_AFTER_TOWN_MAP,
    PPLAY_AFTER_WORLD_ITEM, PPLAY_AFTER_WORLD_CHOICE, PPLAY_AFTER_BATTLE_EVENT,
    PPLAY_AFTER_LEARN, PPLAY_AFTER_EVOLUTION, PPLAY_AFTER_WORLD_BLACKOUT,
    PPLAY_AFTER_BATTLE_SWITCH, PPLAY_AFTER_CHANGE_BOX
};
enum {
    PPLAY_QUESTION_STARTER = 1, PPLAY_QUESTION_NICKNAME,
    PPLAY_QUESTION_SAVE, PPLAY_QUESTION_OVERWRITE, PPLAY_QUESTION_WORLD,
    PPLAY_QUESTION_LEARN, PPLAY_QUESTION_RELEASE, PPLAY_QUESTION_TOSS,
    PPLAY_QUESTION_BATTLE_SWITCH, PPLAY_QUESTION_CHANGE_BOX
};
enum {
    PPLAY_BATTLE_TRANSITION, PPLAY_BATTLE_SLIDE_IN, PPLAY_BATTLE_TRAINER_SOUND,
    PPLAY_BATTLE_PRE_TEXT, PPLAY_BATTLE_WANTS_TEXT, PPLAY_BATTLE_RIVAL_EXIT,
    PPLAY_BATTLE_SENT_TEXT, PPLAY_BATTLE_ENEMY_GROW, PPLAY_BATTLE_ENEMY_CRY,
    PPLAY_BATTLE_ENEMY_HOLD, PPLAY_BATTLE_PLAYER_EXIT, PPLAY_BATTLE_GO_TEXT,
    PPLAY_BATTLE_POOF, PPLAY_BATTLE_POOF_WAIT, PPLAY_BATTLE_PLAYER_GROW,
    PPLAY_BATTLE_PLAYER_CRY, PPLAY_BATTLE_COMMAND
};

static unsigned PplayData(const PbrbPlay* p, int id) {
    return kPbrbPlayData[p->game == 2 ? 1 : 0][id];
}
static PbrbGameRom PplayRom(const PbrbPlay* p) {
    PbrbGameRom r = { p->gb->rom,p->gb->bytes,p->game }; return r;
}
static void PplaySyncAliases(PbrbPlay* p) {
    PbrbGameState* s = &p->gameState; p->money = s->money;
    p->pcPotion = (int)PbrbGameCountItem(s->pcItems, s->pcItemCount, 0x14);
    p->bagPotion = (int)PbrbGameCountItem(s->bag, s->bagCount, 0x14);
    if (s->partyCount && s->active < s->partyCount)p->starter = s->party[s->active];
}
static void PplayStoreActive(PbrbPlay* p) {
    if (p->gameState.partyCount && p->gameState.active < p->gameState.partyCount)
        p->gameState.party[p->gameState.active] = p->starter;
}
static unsigned char PplayChar(char c) {
    if (c >= 'A' && c <= 'Z')return (unsigned char)(0x80 + c - 'A');
    if (c >= 'a' && c <= 'z')return (unsigned char)(0xa0 + c - 'a');
    if (c >= '0' && c <= '9')return (unsigned char)(0xf6 + c - '0');
    switch (c) {
    case ' ':return 0x7f; case '!':return 0xe7; case '?':return 0xe6;
    case '.':return 0xe8; case ',':return 0xf4; case ':':return 0x9c;
    case ';':return 0x9d; case '\'':return 0xe0; case '-':return 0xe3;
    case '/':return 0xf3; case '(':return 0x9a; case ')':return 0x9b;
    default:return 0x7f;
    }
}
static void PplayCopyName(unsigned char* to, const unsigned char* from, int maxLength) {
    memset(to, 0x50, 11);
    if (!from)return;
    for (int n = 0; n < maxLength && n < 10 && from[n] != 0x50; ++n)to[n] = from[n];
}
static void PplayAsciiName(unsigned char* to, const char* from) {
    memset(to, 0x50, 11);
    for (int i = 0; from[i] && i < 10; ++i)to[i] = PplayChar(from[i]);
}
static unsigned PplayRandom(PbrbPlay* p) {
    return PbrbRandomNext(&p->rng);
}
static void PplayMonsterName(PbrbPlay* p, unsigned species, unsigned char* out) {
    memset(out, 0x50, 11);
    if (species < 1u || species>190u)return;
    const unsigned at = PplayData(p, PBRB_DATA_MonsterNames) + (species - 1u) * 10u;
    if (at + 10u > p->gb->bytes)return;
    PplayCopyName(out, p->gb->rom + at, 10);
}
static int PplayAddToken(PbrbPlay* p, unsigned char c) {
    if (p->text.count >= PBRB_PLAY_TEXT_MAX - 1u) { p->textError = 1; return 0; }
    p->text.tokens[p->text.count++] = c; return 1;
}
static void PplayAppendName(PbrbPlay* p, const unsigned char* name) {
    for (int i = 0; i < 10 && name[i] != 0x50; ++i)PplayAddToken(p, name[i]);
}
static void PplayAppendCodes(PbrbPlay* p, const unsigned char* text, unsigned max) {
    for (unsigned i = 0; i < max && text[i] != 0x50; ++i)PplayAddToken(p, text[i]);
}
static void PplayAppendNumber(PbrbPlay* p, unsigned value) {
    unsigned char digits[12]; unsigned n = 0;
    do { digits[n++] = (unsigned char)(0xf6u + value % 10u); value /= 10u; } while (value && n < 12);
    while (n)PplayAddToken(p, digits[--n]);
}
static void PplayAppendAscii(PbrbPlay* p, const char* s) {
    for (; *s; ++s)PplayAddToken(p, PplayChar(*s));
}
static void PplayExpandChar(PbrbPlay* p, unsigned c) {
    switch (c) {
    case 0x52:PplayAppendName(p, p->player); break;
    case 0x53:PplayAppendName(p, p->rival); break;
    case 0x54:PplayAppendAscii(p, "POK"); PplayAddToken(p, 0xba); break;
    case 0x4a:PplayAddToken(p, 0xe1); PplayAddToken(p, 0xe2); break;
    case 0x56:PplayAddToken(p, 0x75); PplayAddToken(p, 0x75); break;
    case 0x59:if (p->textTarget) { PplayAppendAscii(p, "Enemy "); PplayAppendName(p, p->enemy.nickname); }
             else PplayAppendName(p, p->starter.nickname); break;
    case 0x5a:if (p->textUser) { PplayAppendAscii(p, "Enemy "); PplayAppendName(p, p->enemy.nickname); }
             else PplayAppendName(p, p->starter.nickname); break;
    case 0x5b:PplayAppendAscii(p, "PC"); break;
    case 0x5c:PplayAppendAscii(p, "TM"); break;
    case 0x5d:PplayAppendAscii(p, "TRAINER"); break;
    case 0x5e:PplayAppendAscii(p, "ROCKET"); break;
    default:PplayAddToken(p, (unsigned char)c); break;
    }
}
static void PplayExpandRam(PbrbPlay* p, unsigned addr) {
    const unsigned char* service = PbrbPlayServiceRam(p, addr);
    if (service) { PplayAppendName(p, service); return; }
    switch (addr) {
    case 0xcc5b:case 0xcee9:PplayAppendCodes(p, p->nameBuffer, 15); break;
    case 0xcd3d:PplayAppendNumber(p, p->gameState.currentBox + 1u); break;
    case 0xcd3f:
        if (p->menuTarget >= 0 && (unsigned)p->menuTarget < p->gameState.partyCount)PplayAppendName(p, p->gameState.party[p->menuTarget].nickname);
        break;
    case 0xcf5f:case 0xcf70: {
        const PbrbWorldEvent* e = PbrbWorldPendingEvent(p->world);
        unsigned at = e && (e->flags & 0x02000000u) ? (addr == 0xcf5f ? e->dataOffset : e->dataCount) : 0;
        if (at && at < p->gb->bytes)PplayAppendCodes(p, p->gb->rom + at, 31); else p->textError = 1; break;
    }
    case 0xcd6d:PplayAppendCodes(p, p->nameBuffer, 15); break;
    case 0xcf4b:PplayAppendCodes(p, p->stringBuffer, 31); break;
    case 0xcfda:PplayAppendName(p, p->enemy.nickname); break;
    case 0xd009:PplayAppendName(p, p->starter.nickname); break;
    case 0xd04a:PplayAppendCodes(p, p->trainerName, 15); break;
    case 0xd036:
        if (p->pendingParty >= 0 && p->pendingParty < 6) {
            const PbrbPlayMon* m = p->scene == PBRB_PLAY_BATTLE && p->battle.state != PBRB_BATTLE_FINISHED ? p->battle.party[0] + p->pendingParty : p->gameState.party + p->pendingParty;
            PplayAppendName(p, m->nickname);
        }break;
    case 0xde06:PplayAppendName(p, p->battle.captured.nickname); break;
    case 0xd0e1:PplayAppendCodes(p, p->stringBuffer, 31); break;
    case 0xd34a:PplayAppendName(p, p->rival); break;
    case 0xd158:PplayAppendName(p, p->player); break;
    default:p->textError = 1; break;
    }
}
static int PplayExpandText(PbrbPlay* p, unsigned at, int depth) {
    if (depth > 4 || at >= p->gb->bytes) { p->textError = 1; return 0; }
    unsigned budget = 4096;
    while (budget-- && at < p->gb->bytes) {
        unsigned c = p->gb->rom[at++];
        if (c == 0x50)return 1;
        if (c == 0) {
            while (budget-- && at < p->gb->bytes) {
                c = p->gb->rom[at++];
                if (c == 0x50)break;
                PplayExpandChar(p, c);
                if (c == 0x57 || c == 0x58)return 1;
                if (p->textError)return 0;
            }
        }
        else if (c == 1) {
            if (at + 2 > p->gb->bytes)break;
            unsigned addr = p->gb->rom[at] | ((unsigned)p->gb->rom[at + 1] << 8);
            at += 2; PplayExpandRam(p, addr);
        }
        else if (c == 2 || c == 9) {
            if (at + 3 > p->gb->bytes)break;
            unsigned addr = p->gb->rom[at] | ((unsigned)p->gb->rom[at + 1] << 8); at += 3;
            unsigned value = p->textNumber;
            if (addr == 0xd079 || addr == 0xcce5 || addr == 0xff9f)value = p->textMoney;
            else if (addr == 0xd127)value = p->textLevel;
            else if (addr == 0xd11e)value = PbrbGameDexCount(p->gameState.owned);
            else if (addr == 0xcd3f)value = p->textMoney;
            else if (addr == 0xd5a4)value = p->gameState.coins;
            else if (addr == 0xffdb) {
                const PbrbWorldEvent* e = PbrbWorldPendingEvent(p->world);
                value = e && (e->flags & 0x08000000u) ? p->textNumber : PbrbGameDexCount(p->gameState.seen);
            }
            else if (addr == 0xcc5b)value = PbrbGameDexCount(p->gameState.seen);
            else if (addr == 0xffdc || addr == 0xcc5c)value = PbrbGameDexCount(p->gameState.owned);
            else if (addr == 0xffdd)value = PbrbGameDexCount(p->gameState.owned);
            PplayAppendNumber(p, value);
        }
        else if (c == 0x0c) {
            if (at >= p->gb->bytes)break; unsigned count = p->gb->rom[at++];
            for (unsigned n = 0; n < count && n < 18; ++n)PplayAddToken(p, 0x75);
        }
        else if (c == 0x17) {
            if (at + 3 > p->gb->bytes)break;
            unsigned a = p->gb->rom[at] | ((unsigned)p->gb->rom[at + 1] << 8);
            unsigned bank = p->gb->rom[at + 2]; at += 3;
            if (a < 0x4000 || a >= 0x8000 || bank >= 64)break;
            if (!PplayExpandText(p, bank * 0x4000u + a - 0x4000u, depth + 1))return 0;
        }
        else if (c == 5) {
            PplayAddToken(p, 0x4f);
        }
        else if (c == 6 || c == 0x0d) {
            PplayAddToken(p, 0x3b);
        }
        else if (c == 7) {
            PplayAddToken(p, 0x4c);
        }
        else if (c == 0x0a) {
            PplayAddToken(p, 0x3c);
        }
        else if (c == 0x0b || (c >= 0x0e && c <= 0x13)) {
            PplayAddToken(p, 0x3a);
        }
        else if (c == 0x14) {
            PplayAddToken(p, 0x3d);
        }
        else { p->textError = 1; return 0; }
        if (p->textError)return 0;
    }
    p->textError = 1; return 0;
}
static void PplayBeginText(PbrbPlay* p, unsigned at, int forceWait, int after, int preserve) {
    if (!preserve) { memset(p->text.cells, 0x7f, sizeof(p->text.cells)); p->text.row = 1; p->text.column = 0; }
    p->text.count = 0; p->text.position = 0; p->text.active = 1; p->text.visible = 1;
    p->text.waiting = 0; p->text.delay = 3; p->text.scroll = 0; p->text.forceWait = forceWait;
    p->text.after = after; p->textError = 0;
    PplayExpandText(p, at, 0);
    if (!p->text.count || p->text.tokens[p->text.count - 1] < 0x57 ||
        p->text.tokens[p->text.count - 1]>0x58)PplayAddToken(p, 0x57);
    if (p->textError) {
        p->text.count = 0; PplayAppendAscii(p, "Text data error."); PplayAddToken(p, 0x58);
    }
}
static void PplayText(PbrbPlay* p, int id, int wait, int after) {
    PplayBeginText(p, PplayData(p, id), wait, after, 0);
}
static void PplayGameText(PbrbPlay* p, int id, int wait, int after) {
    if (id >= 0 && id < PBRB_GT_COUNT)PplayBeginText(p, kPbrbGameText[p->game == 2 ? 1 : 0][id], wait, after, 0);
}
static void PplayAppendGameText(PbrbPlay* p, int id) {
    if (p->text.count && (p->text.tokens[p->text.count - 1] == 0x57 || p->text.tokens[p->text.count - 1] == 0x58))--p->text.count;
    PplayExpandText(p, kPbrbGameText[p->game == 2 ? 1 : 0][id], 0);
}
static void PplayNotice(PbrbPlay* p, const char* a, const char* b, int returnOverlay) {
    memset(&p->text, 0, sizeof(p->text)); memset(p->text.cells, 0x7f, sizeof(p->text.cells));
    p->text.row = 1; p->text.active = 1; p->text.visible = 1; p->text.after = PPLAY_AFTER_NOTICE;
    p->noticeReturn = returnOverlay; p->overlay = PBRB_OVERLAY_NOTICE;
    PplayAppendAscii(p, a);
    if (b && *b) { PplayAddToken(p, 0x4f); PplayAppendAscii(p, b); }
    PplayAddToken(p, 0x58);
}
static void PplayTextTick(PbrbPlay* p, unsigned held, unsigned pressed) {
    PbrbPlayText* t = &p->text;
    if (!t->active)return;
    if (t->delay > 0) { --t->delay; return; }
    if (t->scroll) {
        memmove(t->cells[0], t->cells[1], 3u * 18u); memset(t->cells[3], 0x7f, 18);
        --t->scroll; t->delay = 4; t->row = 3; t->column = 0; return;
    }
    if (t->waiting) {
        if (!(pressed & (PBRB_PAD_A | PBRB_PAD_B)))return;
        const int w = t->waiting; t->waiting = 0; PbrbAudioPlaySfx(PBRB_SFX_PRESS);
        if (w == 1) { t->active = 0; return; }
        if (w == 2) { memset(t->cells, 0x7f, sizeof(t->cells)); t->row = 1; t->column = 0; t->delay = 19; return; }
        if (w == 3) { t->scroll = 2; return; }
    }
    /* At most one visible glyph per GB frame, even while A or B is held. */
    for (unsigned guard = 0; guard < 32 && t->position < t->count; ++guard) {
        unsigned c = t->tokens[t->position++];
        if (c == 0x57 || c == 0x58) {
            if (c == 0x58 || t->forceWait) { t->waiting = 1; t->delay = 2; }
            else t->active = 0;
            return;
        }
        if (c == 0x51 || c == 0x49) { t->waiting = 2; t->delay = 2; return; }
        if (c == 0x55 || c == 0x4b) { t->waiting = 3; t->delay = 2; return; }
        if (c == 0x4c) { t->scroll = 2; return; }
        if (c == 0x4f) { t->row = 3; t->column = 0; continue; }
        if (c == 0x4e) { t->row += 2; if (t->row > 3)t->row = 3; t->column = 0; continue; }
        if (c == 0x3a) { PbrbAudioPlaySfx(PBRB_SFX_GET_ITEM); continue; }
        if (c == 0x3b) { t->waiting = 4; t->delay = 2; return; }
        if (c == 0x3c) { t->delay = (held & (PBRB_PAD_A | PBRB_PAD_B)) ? 0 : 29; return; }
        if (c == 0x3d) { PbrbAudioPlayCry(0xa8); continue; }
        if (t->column < 18 && t->row >= 0 && t->row < 4)t->cells[t->row][t->column] = (unsigned char)c;
        ++t->column;
        t->delay = (held & (PBRB_PAD_A | PBRB_PAD_B)) ? 0 : p->textSpeed * 2;
        return;
    }
    if (t->position >= t->count)t->active = 0;
}
static void PplayDrawText(PbrbPlay* p) {
    if (!p->text.visible)return;
    PbrbGbTextBox(p->gb, 0, 96, 20, 6);
    for (int y = 0; y < 4; ++y)for (int x = 0; x < 18; ++x)
        PbrbGbGlyph(p->gb, p->text.cells[y][x], 8 + x * 8, 104 + y * 8);
    if (p->text.active && p->text.waiting && ((p->uiFrames / 16u) & 1u) == 0)
        PbrbGbGlyph(p->gb, 0xee, 144, 128);
}
static int PplayPicture(PbrbPlay* p, int kind) {
    p->pictureId = kind; p->pictureX = 48; p->pictureFlip = 0;
    switch (kind) {
    case 1:return PbrbGbDecodePicture(p->gb, PplayData(p, PBRB_DATA_ProfOakPic), p->picture, sizeof(p->picture));
    case 2:p->pictureFlip = 1; return PbrbGbDecodePokemon(p->gb, 0xa7, 0, p->picture, sizeof(p->picture));
    case 3:return PbrbGbDecodePicture(p->gb, PplayData(p, PBRB_DATA_RedPicFront), p->picture, sizeof(p->picture));
    case 4:return PbrbGbDecodePicture(p->gb, PplayData(p, PBRB_DATA_Rival1Pic), p->picture, sizeof(p->picture));
    case 5:return PbrbGbDecodePicture(p->gb, PplayData(p, PBRB_DATA_ShrinkPic1), p->picture, sizeof(p->picture));
    case 6:return PbrbGbDecodePicture(p->gb, PplayData(p, PBRB_DATA_ShrinkPic2), p->picture, sizeof(p->picture));
    default:memset(p->picture, 0, sizeof(p->picture)); return 1;
    }
}
static void PplayStartNaming(PbrbPlay* p, int who) {
    p->namingWho = who; p->namingRow = 0; p->namingColumn = 0; p->namingCase = 0;
    p->namingLength = 0; memset(p->naming, 0x50, 11); p->text.visible = 0;
    p->overlay = PBRB_OVERLAY_NAMING;
}
static void PplayFinishName(PbrbPlay* p) {
    if (p->namingWho == 5) { PbrbPlayDialogueNamed(p); return; }
    if (p->namingWho == 4) { PbrbPlayServiceNamed(p); return; }
    if (p->namingWho < 2 && !p->namingLength)return;
    if (p->namingWho == 0)PplayCopyName(p->player, p->naming, 7);
    else if (p->namingWho == 1)PplayCopyName(p->rival, p->naming, 7);
    else if (p->namingLength) {
        if (p->namingWho == 3)PplayCopyName(p->battle.captured.nickname, p->naming, 10);
        else PplayCopyName(p->starter.nickname, p->naming, 10);
    }
    p->overlay = PBRB_OVERLAY_NONE; p->text.visible = 0;
    if (p->namingWho < 2) { p->pictureX = 48; p->timer = 3; }
    else if (p->namingWho != 3) { PplayStoreActive(p); PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0; }
}
static void PplayNamingTick(PbrbPlay* p, unsigned keys) {
    if (keys & PBRB_PAD_SELECT) { p->namingCase ^= 1; return; }
    if (keys & PBRB_PAD_START) { PplayFinishName(p); return; }
    if (keys & PBRB_PAD_B) { if (p->namingLength)p->naming[--p->namingLength] = 0x50; return; }
    if (keys & PBRB_PAD_UP) { p->namingRow = (p->namingRow + 5) % 6; if (p->namingRow == 5)p->namingColumn = 0; }
    if (keys & PBRB_PAD_DOWN) { p->namingRow = (p->namingRow + 1) % 6; if (p->namingRow == 5)p->namingColumn = 0; }
    if (p->namingRow < 5) {
        if (keys & PBRB_PAD_LEFT)p->namingColumn = (p->namingColumn + 8) % 9;
        if (keys & PBRB_PAD_RIGHT)p->namingColumn = (p->namingColumn + 1) % 9;
    }
    if (!(keys & PBRB_PAD_A))return;
    if (p->namingRow == 5) { p->namingCase ^= 1; return; }
    if (p->namingRow == 4 && p->namingColumn == 8) { PplayFinishName(p); return; }
    int limit = p->namingWho < 2 ? 7 : 10;
    if (p->namingLength >= limit)return;
    unsigned at = PplayData(p, p->namingCase ? PBRB_DATA_LowerCaseAlphabet : PBRB_DATA_UpperCaseAlphabet);
    p->naming[p->namingLength++] = p->gb->rom[at + p->namingRow * 9 + p->namingColumn];
    p->naming[p->namingLength] = 0x50; PbrbAudioPlaySfx(PBRB_SFX_PRESS);
    if (p->namingLength == limit) { p->namingRow = 4; p->namingColumn = 8; }
}
static void PplayDrawNaming(PbrbPlay* p) {
    PbrbGb* g = p->gb; PbrbGbClear(g, 0); PbrbGbTextBox(g, 0, 32, 20, 11);
    if (p->namingWho == 0)PbrbGbString(g, "YOUR NAME?", 0, 8);
    else if (p->namingWho == 1) {
        PbrbGbString(g, "RIVAL", 0, 8); PbrbGbGlyph(g, 0xbd, 40, 8); PbrbGbString(g, " NAME?", 48, 8);
    }
    else {
        PplayMonsterName(p, p->namingWho == 5 ? p->dialogueEvent.species : p->namingWho == 4 ? p->serviceEvent.species : p->namingWho == 3 ? p->battle.captured.species : p->starter.species, p->nameBuffer);
        PbrbGbCodes(g, p->nameBuffer, 32, 8, 10); PbrbGbString(g, "NICKNAME?", 8, 24);
    }
    PbrbGbCodes(g, p->naming, 80, 16, 10);
    int limit = p->namingWho < 2 ? 7 : 10;
    unsigned tiles = PplayData(p, PBRB_DATA_HpBarAndStatusGraphics);
    for (int i = 0; i < limit; ++i)PbrbGbTile(g, tiles + (0x76u - 0x62u + (i == (p->namingLength < limit ? p->namingLength : limit - 1) ? 1u : 0u)) * 16u, 2, 80 + i * 8, 24);
    unsigned at = PplayData(p, p->namingCase ? PBRB_DATA_LowerCaseAlphabet : PBRB_DATA_UpperCaseAlphabet);
    for (int y = 0; y < 5; ++y)for (int x = 0; x < 9; ++x) {
        unsigned c = g->rom[at + y * 9 + x];
        if (c == 0xf0)PbrbGbTile(g, PplayData(p, PBRB_DATA_ED_Tile), 1, 16 + x * 16, 40 + y * 16);
        else PbrbGbGlyph(g, (unsigned char)c, 16 + x * 16, 40 + y * 16);
    }
    PbrbGbCodes(g, g->rom + at + 45, 16, 120, 10);
    PbrbGbGlyph(g, 0xed, 8 + p->namingColumn * 16, 40 + p->namingRow * 16);
}
static void PplayNameListTick(PbrbPlay* p, unsigned keys) {
    if (keys & PBRB_PAD_UP)p->selection = (p->selection + 3) % 4;
    if (keys & PBRB_PAD_DOWN)p->selection = (p->selection + 1) % 4;
    if (!(keys & (PBRB_PAD_A | PBRB_PAD_START)))return;
    PbrbAudioPlaySfx(PBRB_SFX_PRESS);
    if (!p->selection) { PplayStartNaming(p, p->namingWho); return; }
    unsigned at = PplayData(p, p->namingWho ? PBRB_DATA_DefaultNamesRivalList : PBRB_DATA_DefaultNamesPlayerList);
    /* The first terminated list entry is NEW NAME; names are variable length. */
    for (int n = 0; n < p->selection; ++n) {
        while (at < p->gb->bytes && p->gb->rom[at] != 0x50)++at;
        if (at >= p->gb->bytes)return;
        ++at;
    }
    if (at + 8u > p->gb->bytes)return;
    PplayCopyName(p->namingWho ? p->rival : p->player, p->gb->rom + at, 7);
    p->overlay = PBRB_OVERLAY_NONE; p->pictureMotion = 2; p->timer = 31;
}
static void PplayDrawNameList(PbrbPlay* p) {
    PbrbGbTextBox(p->gb, 0, 0, 11, 12); PbrbGbString(p->gb, "NAME", 24, 0);
    unsigned at = PplayData(p, p->namingWho ? PBRB_DATA_DefaultNamesRival : PBRB_DATA_DefaultNamesPlayer);
    int row = 0, col = 0;
    for (unsigned n = 0; n < 70; ++n) {
        unsigned c = p->gb->rom[at + n]; if (c == 0x50)break;
        if (c == 0x4e) { ++row; col = 0; }
        else { PbrbGbGlyph(p->gb, (unsigned char)c, 16 + col * 8, 16 + row * 16); ++col; }
    }
    PbrbGbGlyph(p->gb, 0xed, 8, 16 + p->selection * 16);
}

static void PplayOakTick(PbrbPlay* p) {
    if (p->timer > 0) {
        --p->timer;
        if (p->pictureMotion == 1)p->pictureX = 48 + (18 - p->timer) / 3 * 8;
        else if (p->pictureMotion == 2)p->pictureX = 48 + (p->timer > 18 ? 18 : p->timer) / 3 * 8;
        if (!p->timer) { p->fade = 0; p->pictureMotion = 0; }
        return;
    }
    /* The order, portraits, text addresses, shrink poses and the unusual
       Nidorina cry come directly from engine/movie/oak_speech. */
    switch (p->oakStep++) {
    case 0:PbrbAudioPlayMusic(PBRB_MUSIC_OAK_SPEECH); PplayPicture(p, 1); p->fade = 1; p->timer = 60; break;
    case 1:PplayText(p, PBRB_DATA_OakSpeechText1, 0, PPLAY_AFTER_NONE); break;
    case 2:p->fade = 2; p->timer = 24; break;
    case 3:p->text.visible = 0; PplayPicture(p, 2); p->fade = 3; p->timer = 16; break;
    case 4:PplayText(p, PBRB_DATA_OakSpeechText2A, 0, PPLAY_AFTER_NONE); break;
    case 5:PbrbAudioPlayCry(0xa8); break;
    case 6:if (PbrbAudioEffectsBusy()) { --p->oakStep; break; }
          PplayBeginText(p, PplayData(p, PBRB_DATA_OakSpeechText2B), 0, PPLAY_AFTER_NONE, 1); break;
    case 7:p->fade = 2; p->timer = 24; break;
    case 8:p->text.visible = 0; PplayPicture(p, 3); p->fade = 3; p->timer = 16; break;
    case 9:PplayText(p, PBRB_DATA_IntroducePlayerText, 0, PPLAY_AFTER_NONE); break;
    case 10:p->pictureMotion = 1; p->timer = 18; break;
    case 11:p->overlay = PBRB_OVERLAY_NAME_LIST; p->selection = 0; p->namingWho = 0; break;
    case 12:PplayText(p, PBRB_DATA_YourNameIsText, 0, PPLAY_AFTER_NONE); break;
    case 13:p->fade = 2; p->timer = 24; break;
    case 14:p->text.visible = 0; PplayPicture(p, 4); p->fade = 1; p->timer = 60; break;
    case 15:PplayText(p, PBRB_DATA_IntroduceRivalText, 0, PPLAY_AFTER_NONE); break;
    case 16:p->pictureMotion = 1; p->timer = 18; break;
    case 17:p->overlay = PBRB_OVERLAY_NAME_LIST; p->selection = 0; p->namingWho = 1; break;
    case 18:PplayText(p, PBRB_DATA_HisNameIsText, 0, PPLAY_AFTER_NONE); break;
    case 19:p->fade = 2; p->timer = 24; break;
    case 20:p->text.visible = 0; PplayPicture(p, 3); p->fade = 4; p->timer = 24; break;
    case 21:PplayText(p, PBRB_DATA_OakSpeechText3, 0, PPLAY_AFTER_NONE); break;
    case 22:PbrbAudioPlaySfx(PBRB_SFX_SHRINK); p->text.visible = 0; p->timer = 4; break;
    case 23:PplayPicture(p, 5); p->timer = 4; break;
    case 24:PplayPicture(p, 6); PbrbAudioFadeMusic(PBRB_MUSIC_NONE, 10); p->timer = 20; break;
    case 25:p->pictureId = 7; p->timer = 50; break;
    case 26:p->fade = 2; p->timer = 24; break;
    default:p->scene = PBRB_PLAY_WORLD; p->text.visible = 0; p->pictureId = 0;
        PbrbAudioPlayMusic(PbrbWorldMusic(p->world)); break;
    }
}
static int PplayMakeMon(PbrbPlay* p, PbrbPlayMon* m, unsigned species, int enemy) {
    PbrbGameRom r = PplayRom(p);
    return PbrbGameMakeMon(&r, m, species, 5, &p->rng, p->trainerID, enemy ? p->rival : p->player, enemy);
}
static int PplayBackPicture(PbrbPlay* p, unsigned species, int trainer, unsigned char* out) {
    unsigned char temp[56 * 56];
    int ok = trainer ? PbrbGbDecodePicture(p->gb, PplayData(p, PBRB_DATA_RedPicBack), temp, sizeof(temp)) :
        PbrbGbDecodePokemon(p->gb, species, 1, temp, sizeof(temp));
    if (!ok) { memset(out, 0, 56 * 56); return 0; }
    /* ScaleSpriteByTwo uses the upper-left 28x28 of the 4x4 back sprite. */
    for (int y = 0; y < 56; ++y)for (int x = 0; x < 56; ++x)out[y * 56 + x] = temp[(24 + y / 2) * 56 + 16 + x / 2];
    return 1;
}
#include "pbrb_battle_ui.inl"
enum { PPLAY_MAROWAK_ANIM_FRAMES = 205 };
static int PplayMarowakUnveil(const PbrbPlay* p) {
    return !p->battle.setup.trainerClass && !p->battle.setup.ghost &&
        p->battle.setup.noCatch && p->enemy.species == 0x91u;
}
static void PplayStartBattle(PbrbPlay* p, const PbrbWorldEvent* event) {
    PbrbGameRom rom = PplayRom(p); p->encounter = *event; PplayStoreActive(p);
    if (!p->gameState.partyCount && p->starter.species)PbrbGameAddMon(&rom, &p->gameState, &p->starter);
    unsigned active = 0;
    if (active >= p->gameState.partyCount || !p->gameState.party[active].hp) {
        for (active = 0; active < p->gameState.partyCount && !p->gameState.party[active].hp; ++active) {}
    }
    if (active >= p->gameState.partyCount) { PbrbWorldAcknowledge(p->world, PBRB_WORLD_BATTLE_LOST); p->worldEvent = 0; return; }
    PbrbBattleSetup setup; PbrbBattleDefaultSetup(&setup);
    setup.trainerClass = event->trainerClass; setup.trainerNumber = event->trainerId;
    setup.firstRival = (event->flags & PBRB_WORLD_FIRST_RIVAL) != 0;
    setup.trainerID = p->trainerID; setup.badges = p->gameState.badges;
    setup.expAll = PbrbGameCountItem(p->gameState.bag, p->gameState.bagCount, 0x4b) != 0;
    setup.safari = (event->flags & PBRB_WORLD_SAFARI_BATTLE) != 0; setup.safariBalls = p->gameState.safariBalls;
    setup.ghost = (event->flags & PBRB_WORLD_GHOST) != 0; setup.noCatch = (event->flags & PBRB_WORLD_NO_CATCH) != 0;
    setup.setMode = p->battleStyle;
    PbrbPlayMon enemies[6]; unsigned count = 1, prize = 0; int valid = 0;
    if (setup.trainerClass) {
        valid = PbrbBattleLoadTrainer(&rom, setup.trainerClass, setup.trainerNumber,
            (unsigned)p->world->rivalStarter, enemies, &count, &prize);
    }
    else {
        unsigned species = event->species ? event->species : (unsigned)event->value;
        valid = PbrbGameMakeMon(&rom, enemies, species, event->level ? event->level : 5, &p->rng, 0, 0, 0);
    }
    /* Continue after wild DV generation; an earlier snapshot would replay
       those same two draws at the beginning of battle. */
    setup.seed = p->rng; setup.prizeMoney = event->reward ? event->reward : prize;
    if (!valid || !PbrbBattleBegin(&p->battle, &rom, p->gameState.party, p->gameState.partyCount, active, enemies, count, &setup)) {
        PplayNotice(p, "Battle data error.", "Reload your save.", PBRB_OVERLAY_NONE); return;
    }
    PbrbBattleEvent ignored; while (PbrbBattleNextEvent(&p->battle, &ignored)) {}
    p->gameState.active = (unsigned char)active; p->starter = p->battle.party[0][active]; p->enemy = p->battle.party[1][0];
    p->battleLastSwitchEnemyHP = p->enemy.hp; p->battleAlarmDisabled = 0;
    PbrbAudioSetLowHealth(0);
    /* LoadEnemyMonData marks the real species seen before applying the
       Tower ghost disguise, including encounters without the Silph Scope. */
    PbrbGameMarkDex(&rom, &p->gameState, p->enemy.species, 0);
    for (unsigned i = 0; i < 6; ++i)p->evolutionLevels[i] = i < p->gameState.partyCount ? p->gameState.party[i].level : 0;
    p->battleFinished = 0; p->battleFinishPhase = 0; p->battleEventActive = 0; p->battleEventPhase = 0;
    p->battleWait = 0; p->battleVisual = 0; p->battleHidden = 0; p->evolutionParty = 0;
    p->battleAnim.active = 0; p->battleDisplaySpecies[0] = p->starter.species; p->battleDisplaySpecies[1] = p->enemy.species; p->battleBallItem = 0;
    p->battlePictureFlags[0] = p->battlePictureFlags[1] = 0;
    p->scene = PBRB_PLAY_BATTLE; p->overlay = PBRB_OVERLAY_NONE; p->text.visible = 0;
    p->battleStep = PPLAY_BATTLE_TRANSITION; p->battleSelection = 0; p->timer = 160; p->endpoint = 0;
    memset(p->trainerName, 0x50, 16);
    if (setup.trainerClass == 25 || setup.trainerClass == 42 || setup.trainerClass == 43)memcpy(p->trainerName, p->rival, 11);
    else if (setup.trainerClass)PgameName(&rom, 0x399ffu, setup.trainerClass, p->trainerName);
    if (setup.ghost || PplayMarowakUnveil(p)) {
        /* Both supported ROMs place the unidentified Tower ghost at $0d:66b5.
           Keep its real species in the mechanics, but do not reveal it in UI. */
        PplayAsciiName(p->enemy.nickname, "GHOST");
        PbrbGbDecodePicture(p->gb, 0x366b5u, p->enemyPicture, sizeof(p->enemyPicture));
    }
    else PbrbGbDecodePokemon(p->gb, p->enemy.species, 0, p->enemyPicture, sizeof(p->enemyPicture));
    if (setup.trainerClass) {
        unsigned at = 0x39914u + (setup.trainerClass - 1u) * 5u;
        unsigned address = p->gb->rom[at] | ((unsigned)p->gb->rom[at + 1] << 8);
        PbrbGbDecodePicture(p->gb, 0x4c000u + address - 0x4000u, p->picture, sizeof(p->picture));
    }
    else memcpy(p->picture, p->enemyPicture, sizeof(p->picture));
    PplayBackPicture(p, 0, 1, p->backPicture);
    int music = setup.trainerClass ? PBRB_MUSIC_TRAINER_BATTLE : PBRB_MUSIC_WILD_BATTLE;
    if ((setup.trainerClass >= 33 && setup.trainerClass <= 40) || setup.trainerClass == 44 || setup.trainerClass >= 46)music = PBRB_MUSIC_GYM_BATTLE;
    if (setup.trainerClass == 43)music = PBRB_MUSIC_CHAMPION_BATTLE;
    PbrbAudioPlayMusic(music);
}
static void PplayBattleTick(PbrbPlay* p, unsigned keys) {
    if (p->timer > 0) {
        --p->timer;
        if (p->battleStep == PPLAY_BATTLE_POOF && p->timer == 20)PbrbAudioPlaySfx(PBRB_SFX_BALL_POOF);
        if (p->timer > 0)return;
    }
    switch (p->battleStep) {
    case PPLAY_BATTLE_TRANSITION:p->timer = 79; p->battleStep = PPLAY_BATTLE_SLIDE_IN; return;
    case PPLAY_BATTLE_SLIDE_IN:
        if (p->battle.setup.trainerClass)PbrbAudioPlaySfx(PBRB_SFX_TRAINER_APPEARED);
        else if (!p->battle.setup.ghost && !PplayMarowakUnveil(p))PbrbAudioPlayCry(p->enemy.species);
        p->battleStep = PPLAY_BATTLE_TRAINER_SOUND; return;
    case PPLAY_BATTLE_TRAINER_SOUND:
        if (PbrbAudioEffectsBusy())return;
        p->timer = 20; p->battleStep = PPLAY_BATTLE_PRE_TEXT; return;
    case PPLAY_BATTLE_PRE_TEXT:
        if (p->battle.setup.trainerClass)PplayText(p, PBRB_DATA_TrainerWantsToFightText, 0, PPLAY_AFTER_NONE);
        else if (p->battle.setup.ghost || PplayMarowakUnveil(p))PplayGameText(p, PBRB_GT_EnemyAppearedText, 0, PPLAY_AFTER_NONE);
        else PplayGameText(p, PBRB_GT_WildMonAppearedText, 0, PPLAY_AFTER_NONE);
        p->battleStep = PPLAY_BATTLE_WANTS_TEXT; return;
    case PPLAY_BATTLE_WANTS_TEXT:
        if (PplayMarowakUnveil(p)) {
            if (!p->battleEventPhase) {
                p->battleEventPhase = 1; PplayGameText(p, PBRB_GT_UnveiledGhostText, 0, PPLAY_AFTER_NONE); return;
            }
            if (p->battleEventPhase == 1) {
                /* The original reloads the enemy's identity after the
                   Scope text, then retains that text throughout the reveal. */
                PbrbGameRom rom = PplayRom(p); PbrbGameMonName(&rom, p->enemy.species, p->enemy.nickname);
                PbrbGbDecodePokemon(p->gb, p->enemy.species, 0, p->enemyPicture, sizeof(p->enemyPicture));
                p->battleEventPhase = 2; p->battleVisual = 5; p->timer = PPLAY_MAROWAK_ANIM_FRAMES; return;
            }
            if (p->battleEventPhase == 2) {
                p->battleVisual = 0; memcpy(p->picture, p->enemyPicture, sizeof(p->picture));
                p->battleEventPhase = 3; PplayGameText(p, PBRB_GT_WildMonAppearedText, 0, PPLAY_AFTER_NONE); return;
            }
            if (p->battleEventPhase == 3) {
                p->battleEventPhase = 4; PbrbAudioPlaySfxModified(PBRB_SFX_TRAINER_APPEARED, 0, 0x80); return;
            }
            if (PbrbAudioEffectsBusy())return;
        }
        if (p->battle.setup.ghost && !p->battleEventPhase) {
            p->battleEventPhase = 1; PplayGameText(p, PBRB_GT_GhostCantBeIDdText, 0, PPLAY_AFTER_NONE); return;
        }
        p->battleEventPhase = 0;
        if (!p->battle.setup.trainerClass) { p->timer = 40; p->battleStep = PPLAY_BATTLE_ENEMY_HOLD; return; }
        p->timer = 16; p->battleStep = PPLAY_BATTLE_RIVAL_EXIT; return;
    case PPLAY_BATTLE_RIVAL_EXIT:PplayText(p, PBRB_DATA_TrainerSentOutText, 0, PPLAY_AFTER_NONE); p->battleStep = PPLAY_BATTLE_SENT_TEXT; return;
    case PPLAY_BATTLE_SENT_TEXT:p->timer = 12; p->battleStep = PPLAY_BATTLE_ENEMY_GROW; return;
    case PPLAY_BATTLE_ENEMY_GROW:PbrbAudioPlayCry(p->enemy.species); p->battleStep = PPLAY_BATTLE_ENEMY_CRY; return;
    case PPLAY_BATTLE_ENEMY_CRY:
        if (PbrbAudioEffectsBusy())return;
        p->timer = 40; p->battleStep = PPLAY_BATTLE_ENEMY_HOLD; return;
    case PPLAY_BATTLE_ENEMY_HOLD:
        if (p->battle.setup.safari) {
            p->text.visible = 0; p->endpoint = 1; p->battleStep = PPLAY_BATTLE_COMMAND; return;
        }
        p->timer = 18; p->battleStep = PPLAY_BATTLE_PLAYER_EXIT; return;
    case PPLAY_BATTLE_PLAYER_EXIT:
        PplayText(p, PBRB_DATA_GoText, 0, PPLAY_AFTER_NONE);
        if (p->text.count)p->text.count--; /* append the original second text block */
        PplayExpandText(p, PplayData(p, PBRB_DATA_PlayerMon1Text), 0);
        p->battleStep = PPLAY_BATTLE_GO_TEXT; return;
    case PPLAY_BATTLE_GO_TEXT:PplayBackPicture(p, p->starter.species, 0, p->backPicture);
        p->timer = p->battleAnimation ? 30 : 0; p->battleStep = PPLAY_BATTLE_POOF;
        PplayBattleRefreshAlarm(p); return;
    case PPLAY_BATTLE_POOF:p->battleStep = PPLAY_BATTLE_POOF_WAIT; return;
    case PPLAY_BATTLE_POOF_WAIT:
        if (PbrbAudioEffectsBusy())return;
        p->timer = 12; p->battleStep = PPLAY_BATTLE_PLAYER_GROW; return;
    case PPLAY_BATTLE_PLAYER_GROW:PbrbAudioPlayCry(p->starter.species); p->battleStep = PPLAY_BATTLE_PLAYER_CRY; return;
    case PPLAY_BATTLE_PLAYER_CRY:
        if (PbrbAudioEffectsBusy())return;
        p->text.visible = 0; p->endpoint = 1; p->battleStep = PPLAY_BATTLE_COMMAND; return;
    default:break;
    }
    if (PplayBattleRuntimeTick(p, keys))return;
    if (keys & (PBRB_PAD_UP | PBRB_PAD_DOWN))p->battleSelection ^= 1;
    if (keys & PBRB_PAD_LEFT)p->battleSelection &= 1;
    if (keys & PBRB_PAD_RIGHT)p->battleSelection |= 2;
    if (!(keys & PBRB_PAD_A))return;
    if (p->battleSelection == 0) {
        if (PbrbBattleSkipMoveMenu(&p->battle))PplayBattleSubmit(p, PBRB_BATTLE_MOVE, 0, 0, 0, 0);
        else { p->selection = 0; p->moveSwap = -1; p->overlay = PBRB_OVERLAY_MOVES; }
    }
    else if (p->battleSelection == 1) { p->selection = 0; p->itemSwap = -1; p->itemSlot = -1; p->overlay = PBRB_OVERLAY_ITEM; }
    else if (p->battleSelection == 2) { p->selection = 0; p->overlay = PBRB_OVERLAY_PARTY; }
    else PplayBattleSubmit(p, PBRB_BATTLE_RUN, 0, 0, 0, 0);
}
static void PplayWorldSound(PbrbPlay* p, const PbrbWorldEvent* event) {
    if (event->species)PbrbAudioPlayCry(event->species); else PbrbAudioPlaySfx(event->value);
    if (event->value == PBRB_SFX_CUT)PbrbInputRumble(0x1200, 0x0800, 65);
    else if (event->value == PBRB_SFX_PUSH_BOULDER)PbrbInputRumble(0x1800, 0x0800, 95);
    else if (event->value == PBRB_SFX_LEDGE)PbrbInputRumble(0x0800, 0x0500, 35);
    (void)p;
}
static void PplayWorldEvent(PbrbPlay* p) {
    /* Music and unflagged SFX advance immediately. Waited sounds, story and
       battle events remain pending until their actual interaction completes. */
    for (int guard = 0; guard < 8; ++guard) {
        const PbrbWorldEvent* ev = PbrbWorldPendingEvent(p->world);
        if (!ev)return;
        const int kind = ev->kind, value = ev->value; const unsigned off = ev->textOffset;
        if (kind == PBRB_WORLD_MUSIC) {
            if (!p->worldSoundPhase) {
                if (ev->flags & PBRB_WORLD_FADE_MUSIC)PbrbAudioFadeMusic(value, ev->dataCount);
                else PbrbAudioPlayMusic(value);
                if (ev->flags & PBRB_WORLD_WAIT_SOUND) { p->worldSoundPhase = 1; p->worldEvent = kind; return; }
            }
            else if (PbrbAudioMusicBusy())return;
            p->worldSoundPhase = 0; p->worldEvent = 0; PbrbWorldAcknowledge(p->world, 0); continue;
        }
        if (kind == PBRB_WORLD_SFX) {
            if (ev->flags & PBRB_WORLD_WAIT_SOUND) {
                if (!p->worldSoundPhase) {
                    PplayWorldSound(p, ev); p->worldSoundPhase = 1; p->worldEvent = kind; return;
                }
                if (p->worldSoundPhase == 2 || PbrbAudioEffectsBusy())return;
                if (ev->flags & PBRB_WORLD_WAIT_BUTTON) { p->worldSoundPhase = 2; return; }
                p->worldSoundPhase = 0; p->worldEvent = 0; if (!(ev->flags & PBRB_WORLD_KEEP_TEXT))p->text.visible = 0;
                PbrbWorldAcknowledge(p->world, 0); continue;
            }
            PplayWorldSound(p, ev); PbrbWorldAcknowledge(p->world, 0); continue;
        }
        p->worldEvent = kind;
        if (ev->itemId) {
            PbrbGameRom rom = PplayRom(p); PbrbGameItemName(&rom, ev->itemId, p->nameBuffer);
            memset(p->stringBuffer, 0x50, 32); memcpy(p->stringBuffer, p->nameBuffer, 16);
        }
        else if (ev->species || (value > 0 && value <= 190))PplayMonsterName(p, ev->species ? ev->species : (unsigned)value, p->nameBuffer);
        if ((ev->flags & 0x04000000u) && ev->species) { memset(p->stringBuffer, 0x50, 32); PplayMonsterName(p, ev->species, p->stringBuffer); }
        p->textNumber = ev->quantity; p->textUser = 0; p->textTarget = 1;
        if (kind == PBRB_WORLD_TEXT) { PplayBeginText(p, off, value >= 0 && !(ev->flags & PBRB_WORLD_AUTO_CLOSE), PPLAY_AFTER_WORLD, (ev->flags & PBRB_WORLD_CONTINUE_TEXT) != 0); return; }
        if (kind == PBRB_WORLD_STARTER_CHOICE) {
            p->questionValue = value; PbrbGbDecodePokemon(p->gb, (unsigned)value, 0, p->picture, sizeof(p->picture));
            PbrbAudioPlayCry((unsigned)value); p->text.visible = 0; p->overlay = PBRB_OVERLAY_STARTER_DEX;
            p->selection = 0; p->timer = 10; return;
        }
        if (kind == PBRB_WORLD_GIVE_STARTER) {
            PplayMakeMon(p, &p->starter, (unsigned)value, 0); PplayMonsterName(p, (unsigned)value, p->nameBuffer);
            PbrbGameRom rom = PplayRom(p); if (!p->gameState.partyCount)PbrbGameAddMon(&rom, &p->gameState, &p->starter);
            PplayText(p, PBRB_DATA_DoYouWantToNicknameText, 0, PPLAY_AFTER_NICKNAME); return;
        }
        if (kind == PBRB_WORLD_BATTLE) { PplayStartBattle(p, ev); return; }
        if (kind == PBRB_WORLD_HEAL) {
            if (!p->worldSoundPhase) {
                PbrbGameRom rom = PplayRom(p); for (unsigned i = 0; i < p->gameState.partyCount; ++i)PbrbGameHeal(&rom, p->gameState.party + i); PplaySyncAliases(p);
                if (!(ev->flags & PBRB_WORLD_WAIT_SOUND)) { if (value)PbrbAudioPlaySfx(PBRB_SFX_HEAL); PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0; continue; }
                p->text.visible = 0; p->worldSoundPhase = 4; p->menuCursor = 0; p->timer = 0; PbrbAudioFadeMusic(PBRB_MUSIC_NONE, 4); return;
            }
            if (p->worldSoundPhase == 4) { if (PbrbAudioMusicBusy())return; p->worldSoundPhase = 5; }
            if (p->worldSoundPhase == 5) {
                if (p->timer > 0) { --p->timer; return; }
                if ((unsigned)p->menuCursor < p->gameState.partyCount) { ++p->menuCursor; p->timer = 30; PbrbAudioPlaySfx(PBRB_SFX_HEALING_MACHINE); return; }
                PbrbAudioPlayMusic(PBRB_MUSIC_HEALED); p->worldSoundPhase = 6; p->timer = 80; return;
            }
            if (p->worldSoundPhase == 6) { if (p->timer > 0)--p->timer; if (PbrbAudioMusicBusy() || p->timer > 0)return; p->worldSoundPhase = 7; p->timer = 32; return; }
            if (p->timer > 0) { --p->timer; return; }
            p->worldSoundPhase = 0; p->worldEvent = 0; PbrbWorldAcknowledge(p->world, 0); PbrbAudioPlayMusic(PbrbWorldMusic(p->world)); continue;
        }
        if (kind == PBRB_WORLD_PC) { PplayText(p, PBRB_DATA_TurnedOnPC1Text, 0, PPLAY_AFTER_PC); PbrbAudioPlaySfx(PBRB_SFX_PC_ON); return; }
        if (kind == PBRB_WORLD_TOWN_MAP) { PplayBeginText(p, off, 1, PPLAY_AFTER_TOWN_MAP, 0); return; }
        if (kind == PBRB_WORLD_GIVE_ITEM || kind == PBRB_WORLD_TAKE_ITEM) {
            unsigned quantity = ev->quantity ? ev->quantity : 1u;
            p->menuCount = kind == PBRB_WORLD_GIVE_ITEM ? PbrbGameAddItem(p->gameState.bag, &p->gameState.bagCount, 20, ev->itemId, quantity) :
                PbrbGameRemoveItem(p->gameState.bag, &p->gameState.bagCount, ev->itemId, quantity);
            PplaySyncAliases(p);
            if (off) { PplayBeginText(p, off, 1, PPLAY_AFTER_WORLD_ITEM, 0); return; }
            PbrbWorldAcknowledge(p->world, p->menuCount); p->worldEvent = 0; continue;
        }
        if (kind == PBRB_WORLD_POKEDEX) { p->gameState.pokedex = 1; PbrbWorldAcknowledge(p->world, 1); p->worldEvent = 0; continue; }
        if (kind == PBRB_WORLD_BADGE) { p->gameState.badges |= (unsigned)value; PbrbWorldAcknowledge(p->world, 1); p->worldEvent = 0; continue; }
        if (kind == PBRB_WORLD_SHOP) { p->selection = 0; p->menuMode = 0; p->overlay = PBRB_OVERLAY_SHOP; return; }
        if (kind == PBRB_WORLD_CHOICE) {
            if (off)PplayBeginText(p, off, 0, PPLAY_AFTER_WORLD_CHOICE, 0);
            else { p->question = PPLAY_QUESTION_WORLD; p->selection = 0; p->overlay = PBRB_OVERLAY_YESNO; }return;
        }
        if (kind == PBRB_WORLD_BLACKOUT) {
            p->gameState.money /= 2; p->money = p->gameState.money; PbrbGameRom rom = PplayRom(p);
            for (unsigned i = 0; i < p->gameState.partyCount; ++i)PbrbGameHeal(&rom, p->gameState.party + i); PplaySyncAliases(p);
            PplayGameText(p, PBRB_GT_PlayerBlackedOutText2, 0, PPLAY_AFTER_WORLD_BLACKOUT); return;
        }
        if (PbrbPlayDialogueBegin(p, ev) || PbrbPlayServiceBegin(p, ev))return;
        if (kind == PBRB_WORLD_UNSUPPORTED) { PplayNotice(p, "This interaction", "is not available.", PBRB_OVERLAY_NONE); PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0; return; }
        if (kind == PBRB_WORLD_PLAYTEST_END) { PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0; continue; }
        return;
    }
}

/* Inner gameplay payload. The Xbox layer adds a checksummed, alternating
   generation envelope; this codec independently validates semantic fields. */
struct PplayWriter { unsigned char* data; size_t size, at; int ok; };
struct PplayReader { const unsigned char* data; size_t size, at; int ok; };
static void PplayPut(PplayWriter* w, unsigned v, unsigned bytes) {
    if (!w->ok || bytes > w->size - w->at) { w->ok = 0; return; }
    for (unsigned i = 0; i < bytes; ++i)w->data[w->at++] = (unsigned char)(v >> (i * 8u));
}
static unsigned PplayGet(PplayReader* r, unsigned bytes) {
    if (!r->ok || bytes > r->size - r->at) { r->ok = 0; return 0; }
    unsigned v = 0; for (unsigned i = 0; i < bytes; ++i)v |= (unsigned)r->data[r->at++] << (i * 8u); return v;
}
static void PplayWriteName(PplayWriter* w, const unsigned char* name) {
    for (int i = 0; i < 11; ++i)PplayPut(w, name[i], 1);
}
static void PplayReadName(PplayReader* r, unsigned char* name, int limit) {
    int ended = 0;
    for (int i = 0; i < 11; ++i) {
        name[i] = (unsigned char)PplayGet(r, 1);
        if (name[i] == 0x50)ended = 1;
        else if (ended || i >= limit || name[i] < 0x60)r->ok = 0;
    }
    if (!ended)r->ok = 0;
}
static void PplayWriteMon(PplayWriter* w, const PbrbPlayMon* m) {
    PplayPut(w, m->species, 1); PplayPut(w, m->level, 1); PplayPut(w, m->status, 1);
    for (int i = 0; i < 4; ++i) { PplayPut(w, m->moves[i], 1); PplayPut(w, m->pp[i], 1); }
    PplayPut(w, m->dvs[0], 1); PplayPut(w, m->dvs[1], 1); PplayWriteName(w, m->nickname);
    PplayPut(w, m->hp, 2); PplayPut(w, m->maxHP, 2); PplayPut(w, m->attack, 2);
    PplayPut(w, m->defense, 2); PplayPut(w, m->speed, 2); PplayPut(w, m->special, 2);
    PplayPut(w, m->experience, 4); PplayPut(w, m->ot, 2);
}
static void PplayReadMon(PplayReader* r, PbrbPlayMon* m) {
    memset(m, 0, sizeof(*m)); m->species = (unsigned char)PplayGet(r, 1);
    m->level = (unsigned char)PplayGet(r, 1); m->status = (unsigned char)PplayGet(r, 1);
    for (int i = 0; i < 4; ++i) { m->moves[i] = (unsigned char)PplayGet(r, 1); m->pp[i] = (unsigned char)PplayGet(r, 1); }
    m->dvs[0] = (unsigned char)PplayGet(r, 1); m->dvs[1] = (unsigned char)PplayGet(r, 1);
    PplayReadName(r, m->nickname, 10); m->hp = PplayGet(r, 2); m->maxHP = PplayGet(r, 2);
    m->attack = PplayGet(r, 2); m->defense = PplayGet(r, 2); m->speed = PplayGet(r, 2); m->special = PplayGet(r, 2);
    m->experience = PplayGet(r, 4); m->ot = PplayGet(r, 2);
    if (!m->species) { if (m->level || m->hp || m->maxHP)r->ok = 0; return; }
    if ((m->species != 0x99 && m->species != 0xb0 && m->species != 0xb1) || m->level != 5 ||
        m->status || m->hp > m->maxHP || m->maxHP < 15 || m->maxHP>25 ||
        m->attack < 5 || m->attack>15 || m->defense < 5 || m->defense>15 ||
        m->speed < 5 || m->speed>15 || m->special < 5 || m->special>15 || m->experience != 135)r->ok = 0;
    for (int j = 0; j < 4; ++j)if (m->moves[j] > 165 || m->pp[j] > 40)r->ok = 0;
}
void PbrbPlayInit(PbrbPlay* p, PbrbWorld* world, PbrbGb* gb, unsigned game) {
    if (!p)return;
    memset(p, 0, sizeof(*p)); p->world = world; p->gb = gb; p->game = game;
    p->textSpeed = 1; p->battleAnimation = 1; p->battleStyle = 0; p->pcPotion = 1;
    p->money = 3000; p->rng = 1;
    PplayAsciiName(p->player, game == 2 ? "BLUE" : "RED");
    PplayAsciiName(p->rival, game == 2 ? "RED" : "BLUE");
    memset(p->nameBuffer, 0x50, 11); memset(p->starter.nickname, 0x50, 11);
    memset(p->enemy.nickname, 0x50, 11);
    memset(p->trainerName, 0x50, sizeof(p->trainerName)); memcpy(p->trainerName, p->rival, 11);
    memset(p->stringBuffer, 0x50, sizeof(p->stringBuffer)); p->textTarget = 1; p->menuSwap = -1;
    p->itemSwap = p->moveSwap = p->itemSlot = -1;
    PbrbGameInit(&p->gameState);
}
void PbrbPlayNew(PbrbPlay* p, int speed, int animation, int style, unsigned seed) {
    PbrbWorld* w = p->world; PbrbGb* g = p->gb; unsigned game = p->game;
    PbrbPlayInit(p, w, g, game); PbrbWorldInit(w, g->rom, g->bytes, PBRB_MAP_BEDROOM);
    p->textSpeed = speed >= 0 && speed < 3 ? speed : 1; p->battleAnimation = animation ? 1 : 0; p->battleStyle = style ? 1 : 0;
    p->rng = seed ? seed : 1; p->trainerID = PplayRandom(p) & 65535u;
    PbrbWorldSetRandomState(w, &p->rng);
    p->hadSave = PbrbHasSave(game); p->scene = PBRB_PLAY_OAK;
    PbrbAudioPlayMusic(PBRB_MUSIC_OAK_SPEECH);
}
int PbrbPlayCanSave(const PbrbPlay* p) {
    return p && p->scene == PBRB_PLAY_WORLD && PbrbWorldCanSave(p->world) && !p->worldEvent;
}
size_t PbrbPlaySerialize(const PbrbPlay* p, unsigned char* out, size_t capacity) {
    if (!PbrbPlayCanSave(p) || !out || capacity < 128u)return 0;
    PplayWriter w = { out,capacity,0,1 };
    PplayPut(&w, 0x31544250u, 4); PplayPut(&w, 2, 2); PplayPut(&w, p->game, 1); PplayPut(&w, 0, 1);
    PplayPut(&w, p->frames, 4); PplayPut(&w, p->rng, 4); PplayPut(&w, p->trainerID, 2); PplayPut(&w, p->money, 4);
    PplayPut(&w, (unsigned)p->textSpeed, 1); PplayPut(&w, (unsigned)p->battleAnimation, 1);
    PplayPut(&w, (unsigned)p->battleStyle, 1);
    PplayWriteName(&w, p->player); PplayWriteName(&w, p->rival);
    if (!w.ok || w.size - w.at < 2u)return 0;
    size_t gameLengthAt = w.at; PplayPut(&w, 0, 2);
    size_t gameLength = PbrbGameSerialize(&p->gameState, out + w.at, w.size - w.at);
    if (!gameLength || gameLength > 65535u)return 0;
    out[gameLengthAt] = (unsigned char)gameLength; out[gameLengthAt + 1] = (unsigned char)(gameLength >> 8); w.at += gameLength;
    if (!w.ok || w.size - w.at < 2u)return 0;
    size_t lengthAt = w.at; PplayPut(&w, 0, 2);
    size_t n = PbrbWorldSerialize(p->world, out + w.at, w.size - w.at);
    if (!n || n > PBRB_WORLD_SAVE_MAX)return 0;
    out[lengthAt] = (unsigned char)n; out[lengthAt + 1] = (unsigned char)(n >> 8);
    return w.at + n;
}
int PbrbPlayDeserialize(PbrbPlay* p, const unsigned char* data, size_t bytes) {
    if (!p || !p->world || !p->gb || !data || bytes<100u || bytes>PBRB_PLAY_SAVE_MAX)return 0;
    /* Static staging keeps large scene/pixel buffers off the Xbox stack. */
    static PbrbPlay next; static PbrbWorld nextWorld;
    PbrbPlayInit(&next, p->world, p->gb, p->game); nextWorld = *p->world;
    PplayReader r = { data,bytes,0,1 };
    if (PplayGet(&r, 4) != 0x31544250u)return 0; unsigned version = PplayGet(&r, 2);
    if ((version != 1u && version != 2u) || PplayGet(&r, 1) != p->game || PplayGet(&r, 1))return 0;
    next.frames = PplayGet(&r, 4); next.rng = PplayGet(&r, 4); next.trainerID = PplayGet(&r, 2); next.money = PplayGet(&r, 4);
    /* Keep existing saves readable, including older native saves above the
       original 255:00 cap. Animation timing has its own transient clock. */
    if (next.frames > 255u * 60u * 60u * 60u)next.frames = 255u * 60u * 60u * 60u;
    next.uiFrames = next.frames;
    next.textSpeed = (int)PplayGet(&r, 1); next.battleAnimation = (int)PplayGet(&r, 1); next.battleStyle = (int)PplayGet(&r, 1);
    PbrbGameRom rom = PplayRom(p);
    if (version == 1) { next.pcPotion = (int)PplayGet(&r, 1); next.bagPotion = (int)PplayGet(&r, 1); }
    PplayReadName(&r, next.player, 7); PplayReadName(&r, next.rival, 7);
    if (version == 1) {
        PplayReadMon(&r, &next.starter);
        if (next.pcPotion > 1 || next.bagPotion > 1 || next.pcPotion + next.bagPotion > 1 || next.money != 3000)return 0;
        PbrbGameInit(&next.gameState); next.gameState.pcItemCount = 0;
        if (next.pcPotion)PbrbGameAddItem(next.gameState.pcItems, &next.gameState.pcItemCount, 50, 0x14, 1);
        if (next.bagPotion)PbrbGameAddItem(next.gameState.bag, &next.gameState.bagCount, 20, 0x14, 1);
        if (next.starter.species) {
            PbrbGameBase b; if (!PbrbGameBaseData(&rom, next.starter.species, &b))return 0;
            next.starter.types[0] = b.types[0]; next.starter.types[1] = b.types[1]; next.starter.catchRate = b.catchRate;
            memcpy(next.starter.originalTrainer, next.player, 11); PbrbGameAddMon(&rom, &next.gameState, &next.starter);
        }
    }
    else {
        unsigned length = PplayGet(&r, 2); if (!r.ok || length > bytes - r.at || !PbrbGameDeserialize(&rom, &next.gameState, data + r.at, length))return 0;
        r.at += length; if (next.money != next.gameState.money)return 0;
    }
    unsigned length = PplayGet(&r, 2);
    if (!r.ok || length != bytes - r.at || length > PBRB_WORLD_SAVE_MAX ||
        next.textSpeed > 2 || next.battleAnimation > 1 || next.battleStyle > 1 || next.money > 999999 || next.player[0] == 0x50 || next.rival[0] == 0x50 ||
        !PbrbWorldDeserialize(&nextWorld, data + r.at, length))return 0;
    if (version == 1 && nextWorld.selectedStarter != (int)next.starter.species)return 0;
    PplaySyncAliases(&next); next.lastWorldSteps = (unsigned)nextWorld.steps;
    next.scene = PBRB_PLAY_WORLD; next.overlay = PBRB_OVERLAY_CONTINUE; next.hadSave = 1;
    *p = next; *p->world = nextWorld;
    /* Old saves can contain separate world/game states. Keep the saved
       gameplay state as the owner, without changing saved Pokemon or data. */
    PbrbWorldSetRandomState(p->world, &p->rng); return 1;
}
int PbrbPlayLoad(PbrbPlay* p) {
    static unsigned char buffer[PBRB_PLAY_SAVE_MAX]; unsigned bytes = 0;
    return p && PbrbLoadGame(p->game, buffer, sizeof(buffer), &bytes) && PbrbPlayDeserialize(p, buffer, bytes);
}
/* PC box changes and the ending save a quiescent world snapshot. The live PC
   dialogue remains open; its transient interaction is not written to disk. */
static int PplaySaveCheckpoint(PbrbPlay* p) {
    static PbrbPlay checkpoint; static PbrbWorld world;
    static unsigned char data[PBRB_PLAY_SAVE_MAX];
    checkpoint = *p; world = *p->world; checkpoint.world = &world;
    PbrbWorldSetRandomState(&world, &checkpoint.rng);
    checkpoint.scene = PBRB_PLAY_WORLD; checkpoint.worldEvent = 0;
    if (world.event.kind == PBRB_WORLD_PC)PbrbWorldAcknowledge(&world, 0);
    size_t bytes = PbrbPlaySerialize(&checkpoint, data, sizeof(data));
    if (!bytes || !PbrbSaveGame(p->game, data, (unsigned)bytes))return 0;
    p->hadSave = 1; return 1;
}
static void PplaySave(PbrbPlay* p) {
    static unsigned char buffer[PBRB_PLAY_SAVE_MAX];
    size_t n = PbrbPlaySerialize(p, buffer, sizeof(buffer));
    p->overlay = PBRB_OVERLAY_NONE;
    if (!n || !PbrbSaveGame(p->game, buffer, (unsigned)n)) {
        p->saveInfo = 0; PplayNotice(p, "Save failed.", "Please try again.", PBRB_OVERLAY_START); return;
    }
    p->hadSave = 1; p->savePending = 1; p->timer = 120;
    memset(&p->text, 0, sizeof(p->text)); memset(p->text.cells, 0x7f, sizeof(p->text.cells));
    p->text.visible = 1; const char* s = "Now saving...";
    for (int i = 0; s[i]; ++i)p->text.cells[1][i] = PplayChar(s[i]);
}
static void PplayAfterText(PbrbPlay* p, int after) {
    switch (after) {
    case PPLAY_AFTER_WORLD: {
        const PbrbWorldEvent* ev = PbrbWorldPendingEvent(p->world);
        int keep = ev && (ev->flags & PBRB_WORLD_KEEP_TEXT);
        PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0; if (!keep)p->text.visible = 0;
        break;
    }
    case PPLAY_AFTER_STARTER:p->question = PPLAY_QUESTION_STARTER; p->selection = 0; p->overlay = PBRB_OVERLAY_YESNO; break;
    case PPLAY_AFTER_NICKNAME:p->question = PPLAY_QUESTION_NICKNAME; p->selection = 0; p->overlay = PBRB_OVERLAY_YESNO; break;
    case PPLAY_AFTER_LEARN:p->question = PPLAY_QUESTION_LEARN; p->selection = 0; p->overlay = PBRB_OVERLAY_YESNO; break;
    case PPLAY_AFTER_WORLD_ITEM:PbrbWorldAcknowledge(p->world, p->menuCount); p->worldEvent = 0; p->text.visible = 0; break;
    case PPLAY_AFTER_WORLD_CHOICE:p->question = PPLAY_QUESTION_WORLD; p->selection = 0; p->overlay = PBRB_OVERLAY_YESNO; break;
    case PPLAY_AFTER_BATTLE_EVENT:p->text.visible = 0; break;
    case PPLAY_AFTER_EVOLUTION:p->text.visible = 0; break;
    case PPLAY_AFTER_WORLD_BLACKOUT:p->text.visible = 0; p->worldEvent = 0; PbrbWorldBlackout(p->world); break;
    case PPLAY_AFTER_BATTLE_SWITCH:p->question = PPLAY_QUESTION_BATTLE_SWITCH; p->selection = 0; p->overlay = PBRB_OVERLAY_YESNO; break;
    case PPLAY_AFTER_CHANGE_BOX:p->question = PPLAY_QUESTION_CHANGE_BOX; p->selection = 0; p->overlay = PBRB_OVERLAY_YESNO; break;
    case PPLAY_AFTER_SAVE:p->question = PPLAY_QUESTION_SAVE; p->selection = 0; p->overlay = PBRB_OVERLAY_YESNO; break;
    case PPLAY_AFTER_OVERWRITE:p->question = PPLAY_QUESTION_OVERWRITE; p->selection = 1; p->overlay = PBRB_OVERLAY_YESNO; break;
    case PPLAY_AFTER_SAVED:
        PbrbAudioPlaySfx(PBRB_SFX_SAVE); p->savePending = 2; break;
    case PPLAY_AFTER_PC:p->text.visible = 0; p->selection = 0; p->menuReturn = 0;
        p->overlay = p->world->mapId == PBRB_MAP_BEDROOM ? PBRB_OVERLAY_PC : PBRB_OVERLAY_PC_MAIN; break;
    case PPLAY_AFTER_PC_WITHDRAW:p->text.visible = 0; p->selection = 0; p->overlay = PBRB_OVERLAY_PC; break;
    case PPLAY_AFTER_TOWN_MAP:p->text.visible = 0; p->selection = -1; p->menuMode = 0; p->overlay = PBRB_OVERLAY_TOWN_MAP; break;
    case PPLAY_AFTER_BATTLE:p->text.visible = 0; p->overlay = PBRB_OVERLAY_NONE; break;
    case PPLAY_AFTER_NOTICE:p->text.visible = 0; p->overlay = p->noticeReturn;
        if (p->worldEvent == PBRB_WORLD_TOWN_MAP || p->worldEvent == PBRB_WORLD_PLAYTEST_END) {
            PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0;
        }break;
    default:break;
    }
}
static int PplayMenuAnswer(PbrbPlay*, int, int);
static void PplayAnswer(PbrbPlay* p, int yes) {
    int q = p->question; p->question = 0; p->overlay = PBRB_OVERLAY_NONE;
    if (PplayMenuAnswer(p, q, yes))return;
    if (q == PPLAY_QUESTION_STARTER) { p->text.visible = 0; PbrbWorldAcknowledge(p->world, yes); p->worldEvent = 0; }
    else if (q == PPLAY_QUESTION_NICKNAME) {
        p->text.visible = 0;
        int capture = p->scene == PBRB_PLAY_BATTLE && p->battleFinished == PBRB_BATTLE_CAPTURE;
        if (yes)PplayStartNaming(p, capture ? 3 : 2); else if (!capture) { PplayStoreActive(p); PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0; }
    }
    else if (q == PPLAY_QUESTION_SAVE) {
        if (!yes) { p->text.visible = 0; p->saveInfo = 0; return; }
        if (p->hadSave)PplayText(p, PBRB_DATA_OlderFileWillBeErasedText, 0, PPLAY_AFTER_OVERWRITE);
        else PplaySave(p);
    }
    else if (q == PPLAY_QUESTION_OVERWRITE) {
        if (yes)PplaySave(p); else { p->text.visible = 0; p->saveInfo = 0; }
    }
}

static unsigned PplayMenuKeys(PbrbPlay* p, unsigned held, unsigned pressed) {
    unsigned directions = held & 15u;
    if (!directions || directions != p->repeatHeld) { p->repeatHeld = directions; p->repeatFrames = 0; }
    else ++p->repeatFrames;
    if (p->repeatFrames >= 24u && (p->repeatFrames - 24u) % 6u == 0)pressed |= directions;
    return pressed;
}
/* The platform exit shortcut must not checkpoint a pending item/evolution,
   dialogue or service. The ordinary overworld/start/options states can use
   the same stable-world serializer as the original SAVE menu. */
static int PplayCanSaveAndExit(const PbrbPlay* p) {
    if (!PbrbPlayCanSave(p) || p->savePending || p->endingPending ||
        p->text.active || p->text.visible || p->worldSoundPhase)return 0;
    const int overlay = p->overlay == PBRB_OVERLAY_EXIT ? p->exitReturn : p->overlay;
    return overlay == PBRB_OVERLAY_NONE || overlay == PBRB_OVERLAY_START ||
        overlay == PBRB_OVERLAY_OPTIONS;
}
void PbrbPlayRequestExit(PbrbPlay* p) {
    if (!p || p->overlay == PBRB_OVERLAY_EXIT)return;
    p->exitCanSave = PplayCanSaveAndExit(p); p->exitSaveFailed = 0;
    p->exitReturn = p->overlay; p->exitSelection = p->selection;
    p->overlay = PBRB_OVERLAY_EXIT; p->selection = 2;
    PbrbInputStopRumble();
}
static void PplayCancelExit(PbrbPlay* p) {
    p->overlay = p->exitReturn; p->selection = p->exitSelection;
    p->exitCanSave = p->exitSaveFailed = 0;
}
static void PplaySaveAndExit(PbrbPlay* p) {
    static unsigned char buffer[PBRB_PLAY_SAVE_MAX];
    /* Recheck eligibility before writing. A failed write leaves the complete
       live scene and its previous overlay available to Retry or Cancel. */
    if (!p->exitCanSave || !PplayCanSaveAndExit(p)) { p->exitCanSave = 0; p->selection = 2; return; }
    const size_t bytes = PbrbPlaySerialize(p, buffer, sizeof(buffer));
    if (!bytes || !PbrbSaveGame(p->game, buffer, (unsigned)bytes)) {
        p->exitSaveFailed = 1; return;
    }
    /* PbrbSaveGame returns success only after flush and readback validation. */
    p->hadSave = 1; p->exitSaveFailed = 0; p->exitRequested = 1;
}
static void PplayClosePC(PbrbPlay* p) {
    p->overlay = PBRB_OVERLAY_NONE; p->text.visible = 0;
    PbrbAudioPlaySfx(PBRB_SFX_PC_OFF); PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0;
}
static void PplayOptionsTick(PbrbPlay* p, unsigned k) {
    if (k & PBRB_PAD_UP)p->optionSelection = (p->optionSelection + 3) % 4;
    if (k & PBRB_PAD_DOWN)p->optionSelection = (p->optionSelection + 1) % 4;
    if (k & (PBRB_PAD_LEFT | PBRB_PAD_RIGHT)) {
        if (!p->optionSelection)p->textSpeed = (p->textSpeed + ((k & PBRB_PAD_RIGHT) ? 1 : 2)) % 3;
        else if (p->optionSelection == 1)p->battleAnimation ^= 1;
        else if (p->optionSelection == 2)p->battleStyle ^= 1;
    }
    if ((k & (PBRB_PAD_B | PBRB_PAD_START)) || ((k & PBRB_PAD_A) && p->optionSelection == 3)) {
        p->overlay = PBRB_OVERLAY_START; p->selection = 4 + (p->gameState.pokedex ? 1 : 0);
    }
}
static int PplayGameplayMenuTick(PbrbPlay*, unsigned);
static void PplayWorldPlayer(PbrbPlay*);
static void PplayWorldStep(PbrbPlay*);
void PbrbPlayTick(PbrbPlay* p, unsigned held, unsigned pressed) {
    if (!p || !p->gb || !p->world)return;
    /* VBlank advances the original RNG even while menus/text are waiting.
       Use the engine's gameplay-frame clock, never display/render calls. */
    PbrbWorldSetRandomState(p->world, &p->rng); PplayRandom(p);
    ++p->uiFrames;
    /* TrackPlayTime starts on map entry, after OakSpeech/Continue, and
       stops at 255 hours. Menus and battles still count normally. */
    if (p->scene != PBRB_PLAY_OAK && p->overlay != PBRB_OVERLAY_CONTINUE &&
        p->frames < 255u * 60u * 60u * 60u)++p->frames;
    const unsigned chord = PBRB_PAD_SELECT | PBRB_PAD_START;
    if ((held & chord) == chord && (pressed & chord) && p->overlay != PBRB_OVERLAY_EXIT) {
        PbrbPlayRequestExit(p); return;
    }
    unsigned keys = PplayMenuKeys(p, held, pressed);
    if (p->overlay == PBRB_OVERLAY_EXIT) {
        const int first = p->exitCanSave ? 0 : 1, count = 3 - first;
        if (keys & PBRB_PAD_UP)p->selection = first + (p->selection - first + count - 1) % count;
        else if (keys & PBRB_PAD_DOWN)p->selection = first + (p->selection - first + 1) % count;
        if (keys & PBRB_PAD_B)PplayCancelExit(p);
        else if (keys & PBRB_PAD_A) {
            if (p->selection == 0)PplaySaveAndExit(p);
            else if (p->selection == 1)p->exitRequested = 1;
            else PplayCancelExit(p);
        }
        return;
    }
    if (p->savePending) {
        if (p->savePending == 2) {
            if (PbrbAudioEffectsBusy())return;
            p->savePending = 3; p->timer = 30; return;
        }
        if (p->timer > 0) { --p->timer; return; }
        if (p->savePending == 1) {
            p->savePending = 0; PplayText(p, PBRB_DATA_GameSavedText, 0, PPLAY_AFTER_SAVED);
        }
        else {
            p->savePending = 0; p->text.visible = 0; p->saveInfo = 0; p->overlay = PBRB_OVERLAY_NONE;
        }
        return;
    }
    if (p->text.active) {
        PplayTextTick(p, held, pressed);
        if (!p->text.active) { int after = p->text.after; p->text.after = 0; PplayAfterText(p, after); }
        return;
    }
    if (p->worldSoundPhase == 2) {
        if (pressed & (PBRB_PAD_A | PBRB_PAD_B)) {
            const PbrbWorldEvent* event = PbrbWorldPendingEvent(p->world);
            if (!event || !(event->flags & PBRB_WORLD_KEEP_TEXT))p->text.visible = 0;
            PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0; p->worldSoundPhase = 0;
        }
        return;
    }
    if (p->scene == PBRB_PLAY_CREDITS) { PplayCreditsTick(p, keys); return; }
    if (p->endingPending && PbrbPlayCanSave(p)) { PplayCreditsStart(p); return; }
    if (PbrbPlayDialogueTick(p, keys) || PbrbPlayServiceTick(p, keys))return;
    if (PplayGameplayMenuTick(p, keys))return;
    switch (p->overlay) {
    case PBRB_OVERLAY_NAME_LIST:PplayNameListTick(p, keys); return;
    case PBRB_OVERLAY_NAMING:PplayNamingTick(p, keys); return;
    case PBRB_OVERLAY_YESNO:
        if (keys & (PBRB_PAD_UP | PBRB_PAD_DOWN))p->selection ^= 1;
        if (keys & PBRB_PAD_B)PplayAnswer(p, 0);
        else if (keys & PBRB_PAD_A)PplayAnswer(p, p->selection == 0);
        return;
    case PBRB_OVERLAY_OPTIONS:PplayOptionsTick(p, keys); return;
    case PBRB_OVERLAY_CONTINUE:
        if (keys & PBRB_PAD_A) { p->overlay = PBRB_OVERLAY_NONE; PbrbAudioPlayMusic(PbrbWorldMusic(p->world)); }
        if (keys & PBRB_PAD_B)p->exitRequested = 2;
        return;
    case PBRB_OVERLAY_STARTER_DEX:
        if (p->timer > 0) { --p->timer; return; }
        if (keys & (PBRB_PAD_A | PBRB_PAD_B)) {
            if (!p->selection) { PbrbAudioPlaySfx(PBRB_SFX_PRESS); p->selection = 1; p->timer = 20; return; }
            p->overlay = PBRB_OVERLAY_NONE;
            const PbrbWorldEvent* e = PbrbWorldPendingEvent(p->world);
            if (e)PplayBeginText(p, e->textOffset, 0, PPLAY_AFTER_STARTER, 0);
        }
        return;
    case PBRB_OVERLAY_TOWN_MAP:
        if (keys & PBRB_PAD_UP)p->selection = p->selection < 0 ? 1 : (p->selection + 1) % 47;
        if (keys & PBRB_PAD_DOWN)p->selection = p->selection < 0 ? 46 : (p->selection + 46) % 47;
        if (keys & (PBRB_PAD_A | PBRB_PAD_B)) { p->overlay = PBRB_OVERLAY_NONE; PbrbWorldAcknowledge(p->world, 0); p->worldEvent = 0; }
        return;
    case PBRB_OVERLAY_NOTICE:return;
    default:break;
    }
    if (p->scene == PBRB_PLAY_OAK) { PplayOakTick(p); return; }
    if (p->scene == PBRB_PLAY_BATTLE) { PplayBattleTick(p, keys); return; }
    if ((pressed & PBRB_PAD_START) && PbrbWorldCanSave(p->world) && !p->worldEvent) {
        p->overlay = PBRB_OVERLAY_START; p->selection = p->savedSelection;
        PbrbAudioPlaySfx(PBRB_SFX_START_MENU); return;
    }
    PplayWorldPlayer(p); PbrbWorldTick(p->world, held, pressed); PplayWorldStep(p); PplayWorldEvent(p);
}

static void PplayNumber(PbrbGb* g, unsigned value, int x, int y, int digits) {
    /* Emit original font digits directly. Modern stdio formatting wrappers
       reference UCRT helpers that the legacy RXDK runtime does not supply. */
    unsigned char reversed[3 * sizeof(unsigned)]; unsigned count = 0;
    do {
        reversed[count++] = (unsigned char)(0xf6u + value % 10u); value /= 10u;
    } while (value);
    for (int pad = (int)count; pad < digits; ++pad) { PbrbGbGlyph(g, 0x7f, x, y); x += 8; }
    while (count) { PbrbGbGlyph(g, reversed[--count], x, y); x += 8; }
}
static void PplayHudTile(PbrbPlay* p, unsigned tile, int x, int y) {
    if (tile >= 0x6d && tile <= 0x6f)
        PbrbGbTile(p->gb, PplayData(p, PBRB_DATA_BattleHudTiles1) + (tile - 0x6d) * 8u, 1, x, y);
    else if (tile >= 0x73 && tile <= 0x78)
        PbrbGbTile(p->gb, PplayData(p, PBRB_DATA_BattleHudTiles2) + (tile - 0x73) * 8u, 1, x, y);
    else if (tile >= 0x62 && tile <= 0x78)
        PbrbGbTile(p->gb, PplayData(p, PBRB_DATA_HpBarAndStatusGraphics) + (tile - 0x62) * 16u, 2, x, y);
    else PbrbGbGlyph(p->gb, (unsigned char)tile, x, y);
}
static unsigned PplayHealthPixels(const PbrbPlayMon* m) {
    if (!m->hp || !m->maxHP)return 0;
    unsigned numerator = m->hp * 48u, divisor = m->maxHP;
    /* The original Divide routine accepts a one-byte divisor. Both values
       are truncated by four for max HP above 255 before the division. */
    if (divisor > 255u) { numerator >>= 2; divisor >>= 2; }
    unsigned pixels = numerator / divisor; return pixels ? pixels : 1u;
}
static void PplayHealth(PbrbPlay* p, const PbrbPlayMon* m, int x, int y, int battle = 1) {
    PplayHudTile(p, 0x71, x, y); PplayHudTile(p, 0x62, x + 8, y);
    unsigned pixels = PplayHealthPixels(m);
    for (unsigned i = 0; i < 6; ++i) {
        unsigned n = pixels > 8 ? 8 : pixels; pixels -= n;
        PplayHudTile(p, 0x63 + n, x + 16 + (int)i * 8, y);
    }
    PplayHudTile(p, battle ? 0x6d : 0x6c, x + 64, y);
}
static int PplayCenteredNameX(const unsigned char* name, int x) {
    int n = 0; while (n < 10 && name[n] != 0x50)++n;
    return x + (n <= 2 ? 16 : n <= 4 ? 8 : 0);
}
static void PplayBattleStatus(PbrbPlay* p, const PbrbPlayMon* m, int x, int y) {
    /* The battle HUD keeps an ailment at zero HP and never substitutes FNT.
       PrintStatusAilment also defines the priority for overlapping bits. */
    const char* status = (m->status & 8u) ? "PSN" : (m->status & 16u) ? "BRN" :
        (m->status & 32u) ? "FRZ" : (m->status & 64u) ? "PAR" : (m->status & 7u) ? "SLP" : 0;
    if (status) { PbrbGbString(p->gb, status, x + 8, y); return; }
    if (m->level < 100u) { PplayHudTile(p, 0x6e, x, y); x += 8; }
    PplayNumber(p->gb, m->level, x, y, m->level < 10u ? 1 : m->level < 100u ? 2 : 3);
}
static void PplayBattleHud(PbrbPlay* p, int player, int health) {
    if (player) {
        PplayHudTile(p, 0x73, 144, 80); PplayHudTile(p, 0x77, 144, 88);
        for (int i = 0; i < 8; ++i)PplayHudTile(p, 0x76, 136 - i * 8, 88);
        PplayHudTile(p, 0x6f, 72, 88);
        if (health) {
            PbrbGbCodes(p->gb, p->starter.nickname, PplayCenteredNameX(p->starter.nickname, 80), 56, 10);
            PplayBattleStatus(p, &p->starter, 112, 64);
            PplayHealth(p, &p->starter, 80, 72);
            PplayNumber(p->gb, p->starter.hp, 88, 80, 3); PbrbGbGlyph(p->gb, 0xf3, 112, 80); PplayNumber(p->gb, p->starter.maxHP, 120, 80, 3);
        }
    }
    else {
        PplayHudTile(p, 0x73, 8, 16); PplayHudTile(p, 0x74, 8, 24);
        for (int i = 0; i < 8; ++i)PplayHudTile(p, 0x76, 16 + i * 8, 24);
        PplayHudTile(p, 0x78, 80, 24);
        if (health) {
            PbrbGbCodes(p->gb, p->enemy.nickname, PplayCenteredNameX(p->enemy.nickname, 8), 0, 10);
            PplayBattleStatus(p, &p->enemy, 32, 8);
            PplayHealth(p, &p->enemy, 16, 16, 0);
        }
    }
}
static void PplayPartyBalls(PbrbPlay* p, int player) {
    unsigned at = PplayData(p, PBRB_DATA_PokeballTileGraphics);
    unsigned side = player ? 0u : 1u;
    for (unsigned i = 0; i < 6; ++i) {
        unsigned tile = i >= p->battle.count[side] ? 3u :
            !p->battle.party[side][i].hp ? 2u : p->battle.party[side][i].status ? 1u : 0u;
        PbrbGbSpriteTile(p->gb, at + tile * 16u, player ? 88 + (int)i * 8 : 64 - (int)i * 8, player ? 80 : 16, 0xe4, 0);
    }
}
#include "pbrb_battle_entry.inl"
static void PplayGrowPicture(PbrbPlay* p, const unsigned char* picture, int player, int elapsed) {
    if (elapsed < 3) { PbrbBattleEntryBall(p->gb, p->backPicture, player ? 32 : 120, player ? 88 : 48); return; }
    int side = elapsed < 7 ? 3 : 5;
    unsigned at = PplayData(p, side == 3 ? PBRB_DATA_DownscaledMonTiles_3x3 : PBRB_DATA_DownscaledMonTiles_5x5);
    const int x = player ? (side == 3 ? 24 : 16) : (side == 3 ? 112 : 104);
    const int y = player ? (side == 3 ? 72 : 56) : (side == 3 ? 32 : 16);
    for (int ty = 0; ty < side; ++ty)for (int tx = 0; tx < side; ++tx) {
        unsigned tile = p->gb->rom[at + ty * side + tx]; if (tile < 0x31 || tile>0x61)continue;
        tile -= 0x31; const int sx = (int)(tile / 7u) * 8, sy = (int)(tile % 7u) * 8;
        for (int yy = 0; yy < 8; ++yy)for (int xx = 0; xx < 8; ++xx)
            PbrbGbPixel(p->gb, x + tx * 8 + xx, y + ty * 8 + yy, picture[(sy + yy) * 56 + sx + xx], 0);
    }
}
static void PplayBattlePicture(PbrbPlay* p, unsigned side, int x, int y) {
    const unsigned flags = p->battlePictureFlags[side];
    if (flags & PBRB_BP_HIDDEN)return;
    if (flags & PBRB_BP_SUBSTITUTE) {
        /* The front/back Substitute tiles occupy different columns of the
           original seven-tile picture area. These offsets match PbanPicture. */
        for (unsigned tile = 0; tile < 4; ++tile)
            PbrbGbTile(p->gb, 0x14780u + (side ? tile : tile + 4u) * 16u, 2,
                x + (side ? 16 : 24) + (int)(tile & 1u) * 8, y + 32 + (int)(tile / 2u) * 8);
    }
    else if (flags & PBRB_BP_MINIMIZED) {
        for (unsigned row = 0; row < 5; ++row) {
            unsigned bits = p->gb->rom[0x795c4u + row];
            for (unsigned column = 0; column < 8; ++column)
                PbrbGbPixel(p->gb, x + 24 + (int)column, y + 34 + (int)row,
                    ((bits >> (7u - column)) & 1u) * 3u, 0);
        }
    }
    else PbrbGbBitmap(p->gb, side ? p->enemyPicture : p->backPicture, 56, 56, x, y, 0, 0);
}
static void PplayMarowakPicture(PbrbPlay* p, unsigned elapsed) {
    /* MarowakAnim: 7-frame ghost copy, Delay3, 34-frame ChangeMonPic,
       eight 10-frame flashes, ghost fade, 7-frame hidden picture copy,
       four 10-frame fade-in stages, then Delay3 and ClearSprites. */
    unsigned frame = elapsed + 1u, palette = 0xe4u;
    if (frame >= 45u && frame < 125u)palette = ((frame - 45u) / 10u) & 1u ? 0xe4u : 0x64u;
    else if (frame >= 135u && frame < 145u)palette = 0x90u;
    else if (frame >= 145u && frame < 155u)palette = 0x40u;
    else if (frame >= 155u && frame < 182u)palette = 0;
    else if (frame >= 182u && frame < 192u)palette = 0x40u;
    else if (frame >= 192u && frame < 202u)palette = 0x90u;
    const unsigned char* picture = frame < 155u ? p->picture : p->enemyPicture;
    if (frame < 8u || frame >= 205u) { PbrbGbBitmap(p->gb, picture, 56, 56, 96, 0, 0, 0); return; }
    /* CopyMonPicFromBGToSpriteVRAM covers the six-by-six interior with
       OBP1 sprites. Color zero remains transparent over the white BG. */
    PbrbGbSetPalettes(p->gb, 0xe4, 0xd0, palette);
    for (unsigned y = 8; y < 56; ++y)for (unsigned x = 8; x < 56; ++x)
        PbrbGbPixel(p->gb, 96 + (int)x, (int)y, picture[y * 56u + x], 2);
    PbrbGbSetPalettes(p->gb, 0xe4, 0xd0, 0xe4);
}
static void PplayDrawBattle(PbrbPlay* p) {
    PbrbGb* g = p->gb; PbrbGbClear(g, 0);
    if (p->battleStep == PPLAY_BATTLE_TRANSITION) {
        PbrbWorldRender(p->world, g);
        /* Inward spiral follows the original tile walk, seven tiles per
           Delay3 transfer; the remaining transition setup occupies 7 frames. */
        unsigned elapsed = (unsigned)(160 - p->timer);
        unsigned tiles = elapsed > 7u ? (elapsed - 7u) / 3u * 7u : 0u;
        if (tiles > 360u)tiles = 360u;
        int x = 0, y = 0, left = 0, right = 19, top = 0, bottom = 17;
        unsigned placed = 0;
        while (left <= right && top <= bottom && placed < tiles) {
            for (y = top; y < bottom && placed < tiles; ++y, ++placed)for (int yy = 0; yy < 8; ++yy)for (int xx = 0; xx < 8; ++xx)PbrbGbPixel(g, left * 8 + xx, y * 8 + yy, 3, 0);
            for (x = left; x < right && placed < tiles; ++x, ++placed)for (int yy = 0; yy < 8; ++yy)for (int xx = 0; xx < 8; ++xx)PbrbGbPixel(g, x * 8 + xx, bottom * 8 + yy, 3, 0);
            for (y = bottom; y > top && placed < tiles; --y, ++placed)for (int yy = 0; yy < 8; ++yy)for (int xx = 0; xx < 8; ++xx)PbrbGbPixel(g, right * 8 + xx, y * 8 + yy, 3, 0);
            for (x = right; x > left && placed < tiles; --x, ++placed)for (int yy = 0; yy < 8; ++yy)for (int xx = 0; xx < 8; ++xx)PbrbGbPixel(g, x * 8 + xx, top * 8 + yy, 3, 0);
            ++left; --right; ++top; --bottom;
            if (left == right && top == bottom && placed < tiles) { for (int yy = 0; yy < 8; ++yy)for (int xx = 0; xx < 8; ++xx)PbrbGbPixel(g, left * 8 + xx, top * 8 + yy, 3, 0); ++placed; }
        }
        return;
    }
    PbrbGbTextBox(g, 0, 96, 20, 6);
    int enemyX = 96, playerX = 8;
    if (p->battleStep == PPLAY_BATTLE_SLIDE_IN) {
        int t = p->timer - 3; if (t > 72)t = 72; if (t < 0)t = 0; enemyX -= t * 2; playerX += t * 2;
    }
    if (p->battleStep == PPLAY_BATTLE_RIVAL_EXIT)enemyX += ((16 - p->timer) / 2 + 1) * 8;
    if (p->battleVisual == 5)PplayMarowakPicture(p, (unsigned)(PPLAY_MAROWAK_ANIM_FRAMES - p->timer));
    else if (p->battleStep <= PPLAY_BATTLE_RIVAL_EXIT)PbrbGbBitmap(g, p->picture, 56, 56, enemyX, 0, 0, 0);
    else if (p->battleStep == PPLAY_BATTLE_ENEMY_GROW)PplayGrowPicture(p, p->enemyPicture, 0, 12 - p->timer);
    else if (p->battleStep >= PPLAY_BATTLE_ENEMY_CRY && !(p->battleHidden & 2u) &&
        !(p->battleVisual == 2 && p->battleVisualSide == 1 && p->battleVisualTime > 0 && (p->uiFrames & 2u)))
        PplayBattlePicture(p, 1, 96, 0);
    if (p->battleStep == PPLAY_BATTLE_PLAYER_EXIT)playerX -= ((18 - p->timer) / 2 + 1) * 8;
    if ((p->battleStep <= PPLAY_BATTLE_PLAYER_EXIT || p->battleStep >= PPLAY_BATTLE_PLAYER_CRY) && !(p->battleHidden & 1u) &&
        !(p->battleVisual == 2 && p->battleVisualSide == 0 && p->battleVisualTime > 0 && (p->uiFrames & 2u))) {
        if (p->battleStep == PPLAY_BATTLE_SLIDE_IN)PbrbBattleEntryPlayer(g, p->backPicture, playerX, 40);
        else PplayBattlePicture(p, 0, playerX, 40);
    }
    else if (p->battleStep == PPLAY_BATTLE_PLAYER_GROW)PplayGrowPicture(p, p->backPicture, 1, 12 - p->timer);
    if (p->battleStep >= PPLAY_BATTLE_WANTS_TEXT) {
        const int initial = p->battleStep == PPLAY_BATTLE_WANTS_TEXT;
        if (!(p->battleHidden & 2u) && ((initial && p->battle.setup.trainerClass) || p->battleStep >= PPLAY_BATTLE_ENEMY_HOLD))
            PplayBattleHud(p, 0, p->battleStep >= PPLAY_BATTLE_ENEMY_HOLD);
        if (!(p->battleHidden & 1u) && ((initial && !p->battle.setup.ghost && !PplayMarowakUnveil(p)) ||
            (p->battleStep >= PPLAY_BATTLE_POOF && !p->battle.setup.safari)))
            PplayBattleHud(p, 1, p->battleStep >= PPLAY_BATTLE_POOF);
        if (initial && p->battle.setup.trainerClass)PplayPartyBalls(p, 0);
        if (initial && !p->battle.setup.ghost && !PplayMarowakUnveil(p))PplayPartyBalls(p, 1);
        if (p->battleEventActive && p->battleEvent.type == PBRB_BE_SWITCH_OFFER) {
            PplayBattleHud(p, 0, 0); PplayPartyBalls(p, 0);
        }
    }
    if (p->battleStep == PPLAY_BATTLE_POOF && p->battleAnimation)
        PbrbBattleEntryPoof(g, (unsigned)(30 - p->timer));
    if (p->battleVisual == 4) {
        int column = (24 - p->battleVisualTime) / 4;
        if (column < 0)column = 0; if (column > 5)column = 5;
        PbrbGbBitmap(g, p->picture, 56, 56, 152 - column * 8, 0, 0, 0);
    }
    if (p->endpoint && p->overlay == PBRB_OVERLAY_NONE && !p->text.visible) {
        if (p->battle.setup.safari) {
            PbrbGbString(g, "BALL", 16, 112); PbrbGbGlyph(g, 0xf1, 48, 112);
            PplayNumber(g, p->battle.safariBalls, 56, 112, 2); PbrbGbString(g, "BAIT", 112, 112);
            PbrbGbString(g, "THROW ROCK", 16, 128); PbrbGbString(g, "RUN", 112, 128);
            PbrbGbGlyph(g, 0xed, (p->battleSelection & 2) ? 104 : 8, (p->battleSelection & 1) ? 128 : 112);
        }
        else {
            PbrbGbTextBox(g, 64, 96, 12, 6); PbrbGbString(g, "FIGHT", 80, 112);
            PbrbGbGlyph(g, 0xe1, 128, 112); PbrbGbGlyph(g, 0xe2, 136, 112);
            PbrbGbString(g, "ITEM", 80, 128); PbrbGbString(g, "RUN", 128, 128);
            PbrbGbGlyph(g, 0xed, (p->battleSelection & 2) ? 120 : 72, (p->battleSelection & 1) ? 128 : 112);
        }
    }
}
static void PplaySaveInfo(PbrbPlay* p, int y) {
    PbrbGb* g = p->gb; PbrbGbTextBox(g, 32, y, 16, 10);
    PbrbGbString(g, "PLAYER", 40, y + 16); PbrbGbCodes(g, p->player, 96, y + 16, 7);
    unsigned badges = 0; for (unsigned i = 0; i < 8; ++i)badges += (p->gameState.badges >> i) & 1u;
    PbrbGbString(g, "BADGES", 40, y + 32); PplayNumber(g, badges, 136, y + 32, 2);
    PbrbGbString(g, "POK", 40, y + 48); PbrbGbGlyph(g, 0xba, 64, y + 48); PbrbGbString(g, "DEX", 72, y + 48);
    PplayNumber(g, PbrbGameDexCount(p->gameState.owned), 128, y + 48, 3);
    PbrbGbString(g, "TIME", 40, y + 64);
    unsigned minutes = (p->frames / 60u) / 60u; PplayNumber(g, minutes / 60u, 104, y + 64, 3);
    PbrbGbGlyph(g, 0x6d, 128, y + 64);
    unsigned minute = minutes % 60u;
    PbrbGbGlyph(g, (unsigned char)(0xf6u + minute / 10u), 136, y + 64);
    PbrbGbGlyph(g, (unsigned char)(0xf6u + minute % 10u), 144, y + 64);
}
static void PplayDrawStart(PbrbPlay* p) {
    PbrbGbTextBox(p->gb, 80, 0, 10, 14);
    PbrbGbString(p->gb, "POK", 96, 16); PbrbGbGlyph(p->gb, 0xba, 120, 16); PbrbGbString(p->gb, "MON", 128, 16);
    PbrbGbString(p->gb, "ITEM", 96, 32); PbrbGbCodes(p->gb, p->player, 96, 48, 7);
    PbrbGbString(p->gb, "SAVE", 96, 64); PbrbGbString(p->gb, "OPTION", 96, 80); PbrbGbString(p->gb, "EXIT", 96, 96);
    PbrbGbGlyph(p->gb, 0xed, 88, 16 + p->selection * 16);
}
static void PplayMoveName(PbrbPlay* p, unsigned move, unsigned char* out) {
    memset(out, 0x50, 16); if (!move || move > 165)return;
    unsigned at = PplayData(p, PBRB_DATA_MoveNames);
    for (unsigned i = 1; i < move; ++i) { while (at < p->gb->bytes && p->gb->rom[at] != 0x50)++at; if (at >= p->gb->bytes)return; ++at; }
    for (unsigned j = 0; j < 15 && at < p->gb->bytes && p->gb->rom[at] != 0x50; ++j, ++at)out[j] = p->gb->rom[at];
}
static void PplayTypeName(PbrbPlay* p, unsigned type, int x, int y) {
    if (type > 26)return; unsigned at = 0x27daeu + type * 2u;
    unsigned address = p->gb->rom[at] | ((unsigned)p->gb->rom[at + 1] << 8);
    if (address >= 0x4000u && address < 0x8000u)PbrbGbCodes(p->gb, p->gb->rom + 0x20000u + address, x, y, 9);
}
static void PplayDrawMoves(PbrbPlay* p) {
    /* The original move list uses four single-spaced rows below TYPE/PP.
       Its top border joins the bottom of the information box at y=96. */
    PbrbGbTextBox(p->gb, 32, 96, 16, 6);
    for (int i = 0; i < 4; ++i) {
        unsigned char name[16]; PplayMoveName(p, p->starter.moves[i], name);
        if (p->starter.moves[i])PbrbGbCodes(p->gb, name, 48, 104 + i * 8, 13);
        else PbrbGbString(p->gb, "-", 48, 104 + i * 8);
    }
    if (p->moveSwap >= 0 && p->moveSwap < 4)PbrbGbGlyph(p->gb, 0xec, 40, 104 + p->moveSwap * 8);
    PbrbGbGlyph(p->gb, 0xed, 40, 104 + p->selection * 8);
    PbrbGbTextBox(p->gb, 0, 64, 11, 5);
    unsigned move = p->starter.moves[p->selection];
    if (p->battle.side[0].disabledTurns && p->battle.side[0].disabledSlot == (unsigned)p->selection) {
        PbrbGbString(p->gb, "disabled!", 8, 80); return;
    }
    PbrbGbString(p->gb, "TYPE/", 8, 72);
    PplayNumber(p->gb, p->starter.pp[p->selection] & 63u, 40, 88, 2);
    PbrbGbGlyph(p->gb, 0xf3, 56, 88);
    PbrbGameRom rom = PplayRom(p);
    unsigned max = PbrbGameMaxPP(&rom, move, p->starter.pp[p->selection]);
    PplayNumber(p->gb, max, 64, 88, 2); PbrbGameMove data;
    if (PbrbGameMoveData(&rom, move, &data))PplayTypeName(p, data.type, 16, 80);
}
/* Original ROM-backed StarterDex and town-map helpers are defined separately
   to keep the UI data/layout review independent of the scene state machine. */
#include "pbrb_play_ui.inl"
#include "pbrb_game_menu.inl"
#include "pbrb_dialogue_ui.inl"
#include "pbrb_services.inl"
#include "pbrb_credits.inl"
void PbrbPlayRender(PbrbPlay* p) {
    if (!p || !p->gb || !p->world)return;
    PbrbGb* g = p->gb; PbrbGbSetPalettes(g, 0xe4, 0xd0, 0xe4);
    if (p->scene == PBRB_PLAY_CREDITS)PplayCreditsRender(p);
    else if (p->scene == PBRB_PLAY_WORLD)PbrbWorldRender(p->world, g);
    else if (p->scene == PBRB_PLAY_BATTLE)PplayDrawBattle(p);
    else {
        unsigned pal = 0xe4;
        if (p->fade == 1) { static const unsigned v[6] = { 0x54,0xa8,0xfc,0xf8,0xf4,0xe4 }; int i = (60 - p->timer) / 10; if (i > 5)i = 5; pal = v[i]; }
        if (p->fade == 2) { static const unsigned v[3] = { 0x90,0x40,0 }; int i = (24 - p->timer) / 8; if (i > 2)i = 2; pal = v[i]; }
        if (p->fade == 4) { static const unsigned v[3] = { 0x40,0x90,0xe4 }; int i = (24 - p->timer) / 8; if (i > 2)i = 2; pal = v[i]; }
        PbrbGbSetPalettes(g, pal, 0xd0, 0xe4); PbrbGbClear(g, 0);
        if (p->pictureId == 7) {
            unsigned at = PplayData(p, PBRB_DATA_RedSprite);
            for (int i = 0; i < 4; ++i)PbrbGbSpriteTile(g, at + i * 16u, 64 + (i % 2) * 8, 60 + (i / 2) * 8, 0xd0, 0);
        }
        else if (p->pictureId) {
            int x = p->pictureX; if (p->fade == 3)x = 48 + (p->timer > 1 ? p->timer - 1 : 0) * 8;
            PbrbGbBitmap(g, p->picture, 56, 56, x, 32, 0, p->pictureFlip);
        }
    }
    PplayHealingMachineRender(p);
    if (p->saveInfo)PplaySaveInfo(p, 0);
    if (PbrbPlayDialogueRender(p) || PbrbPlayServiceRender(p)) {}
    else if (PplayGameplayMenuRender(p)) {}
    else if (p->overlay == PBRB_OVERLAY_NAME_LIST)PplayDrawNameList(p);
    else if (p->overlay == PBRB_OVERLAY_NAMING)PplayDrawNaming(p);
    else if (p->overlay == PBRB_OVERLAY_START)PplayDrawStart(p);
    else if (p->overlay == PBRB_OVERLAY_OPTIONS)PbrbGbOptions(g, p->optionSelection, p->textSpeed, p->battleAnimation, p->battleStyle);
    else if (p->overlay == PBRB_OVERLAY_CONTINUE) { PbrbGbClear(g, 0); PbrbGbGameMenu(g, 0, 1); PplaySaveInfo(p, 56); }
    else if (p->overlay == PBRB_OVERLAY_STARTER_DEX) {
        PbrbPlayDrawStarterDex(g, (unsigned)p->questionValue, p->selection);
        /* PageChar clears the seven description rows for twenty frames. */
        if (p->selection == 1 && p->timer > 0)
            for (int y = 80; y < 136; ++y)for (int x = 8; x < 152; ++x)PbrbGbPixel(g, x, y, 0, 0);
    }
    else if (p->overlay == PBRB_OVERLAY_TOWN_MAP)PbrbPlayDrawTownMap(g, p->selection, p->uiFrames, (unsigned)p->world->romMapId);
    else if (p->overlay == PBRB_OVERLAY_TRAINER) {
        PbrbGbClear(g, 0); PbrbGbTextBox(g, 0, 0, 20, 12); PbrbGbString(g, "NAME", 16, 16); PbrbGbCodes(g, p->player, 56, 16, 7);
        PbrbGbString(g, "MONEY", 16, 40); PbrbGbGlyph(g, 0xf0, 72, 40); PplayNumber(g, p->money, 80, 40, 6);
        unsigned badges = 0; for (unsigned i = 0; i < 8; ++i)badges += (p->gameState.badges >> i) & 1u;
        PbrbGbString(g, "BADGES", 16, 64); PplayNumber(g, badges, 120, 64, 1);
        PbrbGbString(g, "ID", 16, 80); PplayNumber(g, p->trainerID, 72, 80, 5);
    }
    else if (p->overlay == PBRB_OVERLAY_MOVES)PplayDrawMoves(p);
    const PbrbWorldEvent* worldQuestion = PbrbWorldPendingEvent(p->world);
    if (p->scene == PBRB_PLAY_WORLD && worldQuestion && worldQuestion->kind == PBRB_WORLD_CHOICE) {
        if (worldQuestion->flags & 0x01000000u) {
            PbrbGbTextBox(g, 0, 0, 17, 6); PbrbGbString(g, "BICYCLE", 16, 16); PbrbGbString(g, "CANCEL", 16, 32);
            PbrbGbGlyph(g, 0xf0, 64, 24); PbrbGbString(g, "1000000", 72, 24);
            PbrbGbGlyph(g, 0xed, 8, 16 + p->selection * 16);
        }
        else if (worldQuestion->flags & 0x00800000u) {
            PbrbGbTextBox(g, 80, 0, 10, 4); PbrbGbString(g, "MONEY", 88, 0);
            PbrbGbGlyph(g, 0xf0, 96, 16); PplayNumber(g, p->gameState.money, 104, 16, 6);
        }
    }
    PplayDrawText(p);
    PbrbPlayServiceForeground(p);
    if (p->scene == PBRB_PLAY_BATTLE && p->battleAnim.active && p->overlay != PBRB_OVERLAY_EVOLUTION)PbrbBattleAnimRender(&p->battleAnim, g);
    if (p->scene == PBRB_PLAY_BATTLE)PplayBattleForeground(p);
    if (p->overlay == PBRB_OVERLAY_YESNO && !(p->question == PPLAY_QUESTION_WORLD && worldQuestion &&
        (worldQuestion->flags & 0x01000000u))) {
        int x = (p->question == PPLAY_QUESTION_SAVE || p->question == PPLAY_QUESTION_OVERWRITE ||
            p->question == PPLAY_QUESTION_BATTLE_SWITCH) ? 0 : 112;
        PbrbGbTextBox(g, x, 56, 6, 5); PbrbGbString(g, "YES", x + 16, 64); PbrbGbString(g, "NO", x + 16, 80);
        PbrbGbGlyph(g, 0xed, x + 8, 64 + p->selection * 16);
    }
    if (p->overlay == PBRB_OVERLAY_EXIT) {
        PbrbGbSetPalettes(g, 0xe4, 0xd0, 0xe4); PbrbGbTextBox(g, 0, 8, 20, 16);
        PbrbGbString(g, "Return to launcher", 8, 16);
        if (p->exitSaveFailed) {
            PbrbGbString(g, "Save failed.", 8, 32); PbrbGbString(g, "Retry or cancel.", 8, 48);
        }
        else PbrbGbString(g, p->exitCanSave ? "Save before exit?" : "Cannot save here.", 8, 40);
        PbrbGbString(g, p->exitCanSave ? "Save and return" : "Save unavailable", 24, 64);
        PbrbGbString(g, "Without saving", 24, 88); PbrbGbString(g, "Cancel", 24, 112);
        PbrbGbGlyph(g, 0xed, 8, 64 + p->selection * 24);
    }
}



