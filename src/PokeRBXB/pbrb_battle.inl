/* Included once by the native play implementation. Rules translated from
   engine/battle/{core,effects,trainer_ai,experience}.asm and item_effects.asm.
   No machine-code interpreter, frame clock, formatted CRT, or platform API. */
#ifndef PBRB_BATTLE_INL_INCLUDED
#define PBRB_BATTLE_INL_INCLUDED
#include "pbrb_battle.h"
#include "pbrb_battle_data.h"
#include "pbrb_rng.h"
#include <string.h>
enum {
    PBF_MIST = 1u, PBF_FOCUS = 2u, PBF_REFLECT = 4u, PBF_SCREEN = 8u,
    PBF_SEED = 16u, PBF_TOXIC = 32u, PBF_RECHARGE = 64u, PBF_FLINCH = 128u,
    PBF_INVULNERABLE = 256u, PBF_TRANSFORM = 512u, PBF_RAGE = 1024u,
    PBF_XACCURACY = 2048u, PBF_TRAPPED_THIS_TURN = 8192u,
    PBF_SUBSTITUTE = 16384u, PBF_MINIMIZE = 32768u
};
static unsigned PbMin(unsigned a, unsigned b) { return a < b ? a : b; }
static unsigned PbMax(unsigned a, unsigned b) { return a > b ? a : b; }
static unsigned PbRandom(unsigned* state) {
    return PbrbRandomNext(state) & 255u;
}
static unsigned PbRand(PbrbBattle* b) { return PbRandom(&b->rng); }
static unsigned PbBound(PbrbBattle* b, unsigned limit) {
    if (!limit)return 0; unsigned n; do { n = PbRand(b); } while (n >= limit); return n;
}
/* Original percent operands truncate against 255, not 256: 25 percent is
   63, 25 percent + 1 is 64, and 50 percent + 1 is 128. */
static const unsigned char* PbBytes(const PbrbGameRom* r, unsigned at, unsigned n) {
    return r && r->data && at <= r->bytes && n <= r->bytes - at ? r->data + at : 0;
}
static unsigned PbTable(const PbrbGameRom* r, int id) { return kPbrbBattleRom[r->game == 2 ? 1 : 0][id]; }
static unsigned PbPointer(unsigned bank, unsigned lo, unsigned hi) {
    unsigned p = lo | (hi << 8); return p >= 0x4000u && p < 0x8000u ? bank * 0x4000u + p - 0x4000u : 0;
}
static void PbSync(PbrbBattle* b, int s) {
    PbrbBattleSide* v = &b->side[s]; PbrbPlayMon* m = &b->party[s][b->active[s]];
    m->hp = v->mon.hp; m->status = v->mon.status;
    if (!(v->flags & PBF_TRANSFORM))memcpy(m->pp, v->mon.pp, 4);
}
static PbrbBattleEvent* PbEvent(PbrbBattle* b, int type, int s, unsigned value, unsigned value2) {
    if (b->eventCount >= PBRB_BATTLE_EVENTS) { b->error = 1; return 0; }
    PbrbBattleEvent* e = &b->events[b->eventCount++]; memset(e, 0, sizeof(*e));
    e->type = type; e->side = s; e->party = s >= 0 ? (int)b->active[s] : -1; e->value = value; e->value2 = value2;
    if (s >= 0)e->species = b->side[s].mon.species;
    for (int i = 0; i < 2; ++i) {
        e->hp[i] = b->side[i].mon.hp; e->maxHP[i] = b->side[i].mon.maxHP;
        e->status[i] = b->side[i].mon.status; e->active[i] = b->active[i];
        e->displaySpecies[i] = b->side[i].mon.species; e->level[i] = b->side[i].mon.level;
        unsigned flags = b->side[i].flags;
        e->pictureFlags[i] = ((flags & PBF_SUBSTITUTE) ? (unsigned)PBRB_BP_SUBSTITUTE : 0u) |
            ((flags & PBF_INVULNERABLE) ? (unsigned)PBRB_BP_HIDDEN : 0u) | ((flags & PBF_MINIMIZE) ? (unsigned)PBRB_BP_MINIMIZED : 0u);
    }
    return e;
}
static void PbMessage(PbrbBattle* b, int s, int message, unsigned value = 0) {
    PbrbBattleEvent* e = PbEvent(b, PBRB_BE_MESSAGE, s, value, 0); if (e) {
        e->message = message;
        if (message == PBRB_BM_CHARGING || message == PBRB_BM_DISABLED)e->move = value;
    }
}
static void PbAnimation(PbrbBattle* b, int actor, unsigned animation, unsigned delay = 0, unsigned before = 0) {
    PbrbBattleEvent* e = PbEvent(b, PBRB_BE_ANIMATION, actor, animation, delay); if (e)e->message = (int)before;
}
static void PbFailed(PbrbBattle* b, int s) { PbEvent(b, PBRB_BE_FAILED, s, 0, 0); }
static unsigned PbBaseStat(const PbrbPlayMon* m, unsigned stat) {
    return stat == 0 ? m->attack : stat == 1 ? m->defense : stat == 2 ? m->speed : m->special;
}
static unsigned PbRatio(PbrbBattle* b, unsigned value, unsigned stage) {
    stage = PbMax(1, PbMin(stage, 13)); const unsigned char* p = PbBytes(&b->rom, PbTable(&b->rom, PBD_StatModifierRatios) + (stage - 1) * 2, 2);
    if (!p || !p[1]) { b->error = 1; return value; }return PbMax(1, value * p[0] / p[1]);
}
static void PbBadges(PbrbBattle* b) {
    for (unsigned i = 0; i < 4; ++i)if (b->setup.badges & (1u << (i * 2u)))
        b->side[0].stats[i] = PbMin(999, b->side[0].stats[i] + b->side[0].stats[i] / 8u);
}
static void PbStatusPenalty(PbrbBattle* b, int s) {
    PbrbBattleSide* v = &b->side[s];
    if (v->mon.status & 16u)v->stats[0] = PbMax(1, v->stats[0] / 2u);
    if (v->mon.status & 64u)v->stats[2] = PbMax(1, v->stats[2] / 4u);
}
static void PbInitialStats(PbrbBattle* b, int s) {
    PbrbBattleSide* v = &b->side[s];
    for (unsigned i = 0; i < 4; ++i)v->stats[i] = PbMin(999, PbRatio(b, PbBaseStat(&v->mon, i), v->stages[i]));
    PbStatusPenalty(b, s); if (!s)PbBadges(b);
}
static void PbSend(PbrbBattle* b, int s, unsigned slot, int event) {
    b->active[s] = slot; PbrbBattleSide* v = &b->side[s]; memset(v, 0, sizeof(*v)); v->mon = b->party[s][slot];
    memset(v->stages, 7, 6); v->mimicSlot = v->disabledSlot = 4;
    if (s && b->setup.trainerClass) {
        const unsigned char* p = PbBytes(&b->rom,
            PbTable(&b->rom, PBD_TrainerAIPointers) + (b->setup.trainerClass - 1u) * 3u, 1); v->aiUses = p ? p[0] : 0;
    }
    PbInitialStats(b, s); b->side[1 - s].trapTurns = 0;
    if (!s) { b->participants |= 1u << slot; b->fought |= 1u << slot; }
    else if (event) { b->participants = b->fought = 1u << b->active[0]; b->side[0].lastMove = 0; }
    b->faintMask &= ~(1u << s);
    if (event)PbEvent(b, PBRB_BE_SEND_OUT, s, slot, 0);
}
static int PbAlive(const PbrbBattle* b, int s) {
    for (unsigned i = 0; i < b->count[s]; ++i)if (b->party[s][i].hp)return 1; return 0;
}
static unsigned PbNextAlive(const PbrbBattle* b, int s) {
    for (unsigned i = 0; i < b->count[s]; ++i)if (b->party[s][i].hp && i != b->active[s])return i; return 6;
}
static void PbFinish(PbrbBattle* b, int outcome) {
    PbSync(b, 0); PbSync(b, 1); b->outcome = outcome; b->state = PBRB_BATTLE_FINISHED;
    PbEvent(b, PBRB_BE_RESULT, -1, (unsigned)outcome,
        outcome == PBRB_BATTLE_WIN ? b->setup.prizeMoney + b->payday : 0);
}
static void PbHP(PbrbBattle* b, int s, unsigned hp, int message = 0) {
    unsigned old = b->side[s].mon.hp; hp = PbMin(hp, b->side[s].mon.maxHP);
    if (message)PbMessage(b, s, message);
    b->side[s].mon.hp = hp; PbSync(b, s);
    if (old != hp) { PbrbBattleEvent* e = PbEvent(b, PBRB_BE_HP, s, hp, old); if (e)e->message = message; }
}
static unsigned PbHit(PbrbBattle* b, int s, unsigned damage, unsigned move, int bypass = 0) {
    PbrbBattleSide* v = &b->side[s];
    if ((v->flags & PBF_SUBSTITUTE) && !bypass) {
        unsigned old = v->substituteHP; int broke = damage > old;
        PbMessage(b, s, PBRB_BM_SUBSTITUTE_DAMAGED, old);
        v->substituteHP = broke ? 0 : old - damage; if (broke)v->flags &= ~PBF_SUBSTITUTE;
        if (broke)PbMessage(b, s, PBRB_BM_SUBSTITUTE_BROKE, v->substituteHP);
        b->lastDamage = damage; return damage; /* Original does not clamp wDamage for substitutes. */
    }
    unsigned dealt = PbMin(damage, v->mon.hp); v->lastDamage = dealt; v->lastDamageMove = move; b->lastDamage = dealt;
    unsigned before = b->eventCount; PbHP(b, s, v->mon.hp - dealt);
    if (b->eventCount > before && b->events[b->eventCount - 1u].type == PBRB_BE_HP)
        b->events[b->eventCount - 1u].move = move;
    return dealt;
}
static void PbStatus(PbrbBattle* b, int s, unsigned status) {
    b->side[s].mon.status = (unsigned char)status; PbStatusPenalty(b, s); PbSync(b, s);
    PbEvent(b, PBRB_BE_STATUS, s, status, 0);
}
static int PbType(const PbrbPlayMon* m, unsigned type) { return m->types[0] == type || m->types[1] == type; }
static unsigned PbEffectiveness(PbrbBattle* b, unsigned type, int target, int ai) {
    unsigned factor = ai ? 16 : 10, at = PbTable(&b->rom, PBD_TypeEffects);
    for (unsigned n = 0; n < 128; ++n, at += 3) {
        const unsigned char* p = PbBytes(&b->rom, at, 3);
        if (!p) { b->error = 1; break; }if (p[0] == 255)break;
        if (p[0] == type && PbType(&b->side[target].mon, p[1])) {
            if (ai)return p[2]; factor = factor * p[2] / 10u;
        }
    }
    return factor;
}
/* Damage applies every matching pair, but the original feedback variable
   is overwritten by each match. Do not modernize the dual-type text/SFX. */
static unsigned PbEffectivenessFeedback(PbrbBattle* b, unsigned type, int target) {
    unsigned factor = 10, at = PbTable(&b->rom, PBD_TypeEffects);
    for (unsigned n = 0; n < 128; ++n, at += 3) {
        const unsigned char* p = PbBytes(&b->rom, at, 3);
        if (!p) { b->error = 1; break; }if (p[0] == 255)break;
        if (p[0] == type && PbType(&b->side[target].mon, p[1]))factor = p[2];
    }
    return factor;
}
static int PbAccuracy(PbrbBattle* b, int s, const PbrbGameMove* m) {
    PbrbBattleSide* a = &b->side[s]; PbrbBattleSide* d = &b->side[1 - s];
    if (m->effect == PBFX_DREAM_EATER_EFFECT && !(d->mon.status & 7u))return 0;
    if (m->effect == PBFX_SWIFT_EFFECT)return 1;
    if (d->flags & PBF_INVULNERABLE)return 0;
    if (((m->effect >= 0x12 && m->effect <= 0x19) || (m->effect >= 0x3a && m->effect <= 0x41)) && (d->flags & PBF_MIST))return 0;
    if (a->flags & PBF_XACCURACY)return 1;
    unsigned original = (m->id == PBMOVE_RAGE && (a->flags & PBF_RAGE)) ? a->rageAccuracy : m->accuracy;
    unsigned chance = PbRatio(b, original, a->stages[4]); chance = PbRatio(b, chance, 14u - d->stages[5]);
    if (m->id == PBMOVE_RAGE)a->rageAccuracy = PbMin(255, chance);
    return PbRand(b) < PbMin(255, chance); /* includes original 1/256 miss */
}
static int PbStat(PbrbBattle* b, int actor, int target, unsigned stat, int delta, int sideEffect = 0) {
    PbrbBattleSide* v = &b->side[target]; if (stat >= 6)return 0;
    int old = v->stages[stat], next = old + delta; if (next < 1)next = 1; if (next > 13)next = 13;
    if (next == old || (stat < 4 && (delta > 0 ? v->stats[stat] == 999 : v->stats[stat] == 1))) { if (!sideEffect)PbFailed(b, actor); return 0; }
    v->stages[stat] = (unsigned char)next;
    if (stat < 4)v->stats[stat] = PbMin(999, PbRatio(b, PbBaseStat(&v->mon, stat), (unsigned)next));
    if (!target)PbBadges(b);
    /* Original applies these to the opponent of the move user even for
       self-stat raises: retain the badge/burn/paralysis reapplication bug. */
    PbStatusPenalty(b, 1 - actor); PbrbBattleEvent* e = PbEvent(b, PBRB_BE_STAT, target, stat, (unsigned)next);
    if (e)e->message = delta; return 1;
}
static int PbCritical(PbrbBattle* b, int s, const PbrbGameMove* m) {
    if (!m->power)return 0; PbrbGameBase base;
    if (!PbrbGameBaseData(&b->rom, b->side[s].mon.species, &base)) { b->error = 1; return 0; }
    unsigned chance = base.speed / 2u;
    if (b->side[s].flags & PBF_FOCUS)chance /= 2u; else chance = PbMin(255, chance * 2u);
    unsigned at = PbTable(&b->rom, PBD_HighCriticalMoves); int high = 0;
    for (unsigned n = 0; n < 16; ++n) { const unsigned char* p = PbBytes(&b->rom, at + n, 1); if (!p || *p == 255)break; if (*p == m->id) { high = 1; break; } }
    chance = high ? PbMin(255, PbMin(255, chance * 2u) * 2u) : chance / 2u;
    unsigned r = PbRand(b); r = ((r << 3) | (r >> 5)) & 255u; return r < chance;
}
static unsigned PbDamage(PbrbBattle* b, int s, const PbrbGameMove* m, int critical, int self = 0) {
    PbrbBattleSide* a = &b->side[s]; PbrbBattleSide* d = &b->side[self ? s : 1 - s];
    unsigned offensive = m->type >= 20 ? 3 : 0, defensive = m->type >= 20 ? 3 : 1;
    unsigned attack = critical ? PbBaseStat(&a->mon, offensive) : a->stats[offensive];
    unsigned defense = critical ? PbBaseStat(&d->mon, defensive) : d->stats[defensive];
    if (!critical && !self && (d->flags & (m->type >= 20 ? PBF_SCREEN : PBF_REFLECT)))defense *= 2;
    if (attack > 255 || defense > 255) { attack = PbMax(1, attack / 4u) & 255u; defense = (defense / 4u) & 255u; }
    if (m->effect == PBFX_EXPLODE_EFFECT)defense = PbMax(1, defense / 2u);
    /* Original can hang when the scaled divisor becomes zero. Native code
       reports an invalid arithmetic state instead of freezing the console. */
    if (!defense) { b->error = 2; return 0; }
    unsigned level = (a->mon.level * (critical ? 2u : 1u)) & 255u;
    unsigned damage = PbMin(997, ((level * 2u / 5u + 2u) * m->power * attack / defense) / 50u) + 2u;
    if (self)return damage;
    if (PbType(&a->mon, m->type))damage += damage / 2u;
    unsigned at = PbTable(&b->rom, PBD_TypeEffects);
    for (unsigned n = 0; n < 128; ++n, at += 3) {
        const unsigned char* p = PbBytes(&b->rom, at, 3); if (!p || p[0] == 255)break;
        if (p[0] == m->type && PbType(&d->mon, p[1]))damage = damage * p[2] / 10u;
    }
    if (damage > 1) { unsigned r; do { r = PbRand(b); r = ((r >> 1) | (r << 7)) & 255u; } while (r < 217); damage = damage * r / 255u; }
    return damage;
}
static void PbSetTrainerMove(const PbrbGameRom* r, PbrbPlayMon* m, unsigned move) {
    PbrbGameMove d; if (PbrbGameMoveData(r, move, &d)) { m->moves[2] = (unsigned char)move; m->pp[2] = d.pp; }
}
int PbrbBattleLoadTrainer(const PbrbGameRom* r, unsigned cls, unsigned number,
    unsigned rivalStarter, PbrbPlayMon out[6], unsigned* count, unsigned* prize) {
    if (count)*count = 0; if (prize)*prize = 0;
    if (!r || !out || !count || cls < 1 || cls>47 || number < 1 || number>255)return 0;
    unsigned base = PbTable(r, PBD_TrainerDataPointers); const unsigned char* p = PbBytes(r, base + (cls - 1) * 2, 2);
    if (!p)return 0; unsigned bank = base / 0x4000u, at = PbPointer(bank, p[0], p[1]), end = PbTable(r, PBD_TrainerAIPointers);
    if (!at)return 0;
    for (unsigned c = 0; c < 47; ++c) {
        p = PbBytes(r, base + c * 2, 2); if (!p)return 0;
        unsigned q = PbPointer(bank, p[0], p[1]); if (q > at && q < end)end = q;
    }
    for (unsigned n = 1; n < number; ++n) { while (at < end) { p = PbBytes(r, at++, 1); if (!p)return 0; if (!*p)break; }if (at >= end)return 0; }
    p = PbBytes(r, at++, 1); if (!p || at > end)return 0; unsigned level = *p; int special = level == 255;
    unsigned rng = 0x3152u, total = 0; memset(out, 0, sizeof(PbrbPlayMon) * 6);
    while (at < end) {
        p = PbBytes(r, at++, 1); if (!p)return 0; if (!*p)break;
        unsigned species = *p; if (special) { level = *p; p = PbBytes(r, at++, 1); if (!p || at > end)return 0; species = *p; }
        if (total == 6 || !PbrbGameMakeMon(r, &out[total], species, level, &rng, 0, 0, 1))return 0; ++total;
    }
    if (!total || at > end)return 0;
    if (special) {
        unsigned lone = 0;
        if (cls >= PBTRAINER_BROCK && cls <= PBTRAINER_KOGA)lone = cls - PBTRAINER_BROCK + 1;
        else if (cls == PBTRAINER_SABRINA)lone = 6; else if (cls == PBTRAINER_BLAINE)lone = 7;
        else if (cls == PBTRAINER_GIOVANNI && number == 3)lone = 8;
        if (lone) {
            p = PbBytes(r, PbTable(r, PBD_LoneMoves) + (lone - 1) * 2, 2); if (!p)return 0;
            if (p[0] < total)PbSetTrainerMove(r, &out[p[0]], p[1]);
        }
        else if (cls == PBTRAINER_RIVAL3 && total == 6) {
            PbSetTrainerMove(r, &out[0], PBMOVE_SKY_ATTACK);
            unsigned dex = PbrbGameDex(r, rivalStarter); unsigned move = dex == 1 || dex == 2 || dex == 3 ? PBMOVE_MEGA_DRAIN :
                dex == 4 || dex == 5 || dex == 6 ? PBMOVE_FIRE_BLAST : PBMOVE_BLIZZARD;
            PbSetTrainerMove(r, &out[5], move);
        }
        else {
            unsigned a = PbTable(r, PBD_TeamMoves); for (unsigned i = 0; i < 5; ++i, a += 2) {
                p = PbBytes(r, a, 2); if (!p)return 0;
                if (p[0] == 255)break; if (p[0] == cls && total >= 5) { PbSetTrainerMove(r, &out[4], p[1]); break; }
            }
        }
    }
    p = PbBytes(r, PbTable(r, PBD_TrainerPicAndMoneyPointers) + (cls - 1) * 5 + 2, 2); if (!p)return 0;
    unsigned money = ((p[0] >> 4) * 1000 + (p[0] & 15) * 100 + (p[1] >> 4) * 10 + (p[1] & 15)) * out[total - 1].level;
    *count = total; if (prize)*prize = money; return 1;
}
int PbrbBattleWildEncounter(const PbrbGameRom* r, unsigned at, unsigned* rng, PbrbPlayMon* out) {
    const unsigned char* p = PbBytes(r, at, 1); if (!p || !rng || !out || !*p)return 0;
    if (PbRandom(rng) >= *p)return 0; p = PbBytes(r, at + 1, 20); if (!p)return 0;
    unsigned random = PbRandom(rng), slot = 9;
    const unsigned char* chance = PbBytes(r, PbTable(r, PBD_WildMonEncounterSlotChances), 20); if (!chance)return 0;
    for (unsigned i = 0; i < 10; ++i)if (random <= chance[i * 2]) { slot = chance[i * 2 + 1] / 2u; break; }
    return slot < 10 && PbrbGameMakeMon(r, out, p[slot * 2 + 1], p[slot * 2], rng, 0, 0, 0);
}
static void PbMoveEvent(PbrbBattle* b, int s, unsigned move) {
    PbrbBattleEvent* e = PbEvent(b, PBRB_BE_MOVE, s, 0, 0); if (e)e->move = move; b->side[s].lastMove = move;
}
static void PbClearMulti(PbrbBattle* b, int s) {
    PbrbBattleSide* v = &b->side[s]; v->chargeMove = v->lockMove = v->lockTurns = v->trapTurns = v->bideTurns = v->bideDamage = 0;
    /* The original leaves the invulnerability bit behind when interrupted. */
}
static void PbConfusion(PbrbBattle* b, int s) {
    if (b->side[s].confusion)return; b->side[s].confusion = (PbRand(b) & 3u) + 2u; PbMessage(b, s, PBRB_BM_CONFUSION_START);
}
static void PbSelfHit(PbrbBattle* b, int s) {
    PbrbGameMove m; memset(&m, 0, sizeof(m)); m.power = 40; m.type = PBTYPE_NORMAL;
    unsigned damage = PbDamage(b, s, &m, 0, 1); PbMessage(b, s, PBRB_BM_SELF_HIT); PbClearMulti(b, s);
    if (b->side[s].flags & PBF_SUBSTITUTE) {/* Original confusion damages the other substitute. */
        unsigned hp = b->side[1 - s].substituteHP; PbMessage(b, 1 - s, PBRB_BM_SUBSTITUTE_DAMAGED, hp);
        b->side[1 - s].substituteHP = damage > hp ? 0 : hp - damage;
        if (damage > hp)b->side[1 - s].flags &= ~PBF_SUBSTITUTE;
        if (damage > hp)PbMessage(b, 1 - s, PBRB_BM_SUBSTITUTE_BROKE, b->side[1 - s].substituteHP);
    }
    else PbHit(b, s, damage, 0, 1);
}
static int PbDisobedience(PbrbBattle* b, unsigned* slot) {
    PbrbBattleSide* v = &b->side[0]; if (v->mon.ot == b->setup.trainerID)return 1;
    unsigned cap = (b->setup.badges & 128) ? 101 : (b->setup.badges & 32) ? 70 : (b->setup.badges & 8) ? 50 : (b->setup.badges & 2) ? 30 : 10;
    if (v->mon.level <= cap)return 1; unsigned sum = PbMin(255, cap + v->mon.level), r;
    do { r = PbRand(b); r = ((r << 4) | (r >> 4)) & 255u; } while (r >= sum); if (r < cap)return 1;
    PbMessage(b, 0, PBRB_BM_DISOBEY);
    if (PbBound(b, sum) < cap) {
        unsigned n = 0;
        for (unsigned i = 0; i < 4; ++i)if (i != *slot && v->mon.moves[i] && (v->mon.pp[i] & 63u) && !v->disabledMove)++n;
        if (n) { unsigned next; do { next = PbRand(b) & 3u; } while (next == *slot || !v->mon.moves[next] || !(v->mon.pp[next] & 63u)); *slot = next; return 1; }return 0;
    }
    unsigned delta = v->mon.level - cap; r = PbRand(b); r = ((r << 4) | (r >> 4)) & 255u;
    if (r < delta) {
        unsigned turns; do { turns = ((PbRand(b) * 2u) & 255u); turns = ((turns << 4) | (turns >> 4)) & 7u; } while (!turns);
        PbStatus(b, 0, turns); return 0;
    }
    if (r - delta < delta)PbSelfHit(b, 0); return 0;
}
static int PbCanAct(PbrbBattle* b, int s, unsigned move) {
    PbrbBattleSide* v = &b->side[s];
    if (v->mon.status & 7u) {
        unsigned status = (v->mon.status & 7u) - 1u; v->mon.status = (unsigned char)status; PbSync(b, s);
        if (status && !s)PbAnimation(b, s, 188, 0, 3);
        PbMessage(b, s, status ? PBRB_BM_ASLEEP : PBRB_BM_WOKE);
        if (status && s)PbAnimation(b, s, 189, 0, 3); v->lastMove = 0; return 0;
    }
    if (v->mon.status & 32u) { PbMessage(b, s, PBRB_BM_FROZEN); v->lastMove = 0; return 0; }
    if (b->side[1 - s].trapTurns || (v->flags & PBF_TRAPPED_THIS_TURN)) { PbMessage(b, s, PBRB_BM_TRAPPED); return 0; }
    if (v->flags & PBF_FLINCH) { v->flags &= ~PBF_FLINCH; PbMessage(b, s, PBRB_BM_FLINCHED); return 0; }
    if (v->flags & PBF_RECHARGE) { v->flags &= ~PBF_RECHARGE; PbMessage(b, s, PBRB_BM_RECHARGE); return 0; }
    if (v->disabledTurns && !--v->disabledTurns) { v->disabledMove = 0; v->disabledSlot = 4; PbMessage(b, s, PBRB_BM_DISABLE_END); }
    if (v->confusion) {
        if (!--v->confusion)PbMessage(b, s, PBRB_BM_CONFUSION_END); else {
            PbMessage(b, s, PBRB_BM_CONFUSED); PbAnimation(b, s, s ? 191u : 190u, 0, 3);
            if (PbRand(b) >= 128u) { PbSelfHit(b, s); return 0; }
        }
    }
    if (v->disabledMove && v->disabledMove == move) { PbMessage(b, s, PBRB_BM_DISABLED, move); return 0; }
    if ((v->mon.status & 64u) && PbRand(b) < 63u) { PbMessage(b, s, PBRB_BM_PARALYZED); PbClearMulti(b, s); return 0; }
    return 1;
}
static void PbResidual(PbrbBattle* b, int s) {
    PbrbBattleSide* v = &b->side[s]; if (!v->mon.hp)return;
    if (v->mon.status & 24u) {
        unsigned damage = PbMax(1, v->mon.maxHP / 16u);
        if (v->flags & PBF_TOXIC)damage *= ++v->toxicCounter;
        PbMessage(b, s, (v->mon.status & 8u) ? PBRB_BM_POISON : PBRB_BM_BURN); PbAnimation(b, s, 186, 0, 3);
        PbHP(b, s, v->mon.hp - PbMin(v->mon.hp, damage));
    }
    if (v->mon.hp && (v->flags & PBF_SEED)) {
        unsigned damage = PbMax(1, v->mon.maxHP / 16u);
        if (v->flags & PBF_TOXIC)damage *= ++v->toxicCounter; damage = PbMin(v->mon.hp, damage);
        PbAnimation(b, 1 - s, PBMOVE_ABSORB, 0, 3); PbHP(b, s, v->mon.hp - damage);
        PbHP(b, 1 - s, b->side[1 - s].mon.hp + damage); PbMessage(b, s, PBRB_BM_LEECH_SEED_HURT);
    }
}
static void PbInflict(PbrbBattle* b, int target, unsigned status, int cause) {
    PbStatus(b, target, status); if (b->eventCount)b->events[b->eventCount - 1].message = cause;
}
static void PbPrimaryStatus(PbrbBattle* b, int s, const PbrbGameMove* m) {
    PbrbBattleSide* d = &b->side[1 - s]; unsigned effect = m->effect;
    if (effect == PBFX_SLEEP_EFFECT || effect == PBFX_EFFECT_01) {
        int recharge = (d->flags & PBF_RECHARGE) != 0; d->flags &= ~PBF_RECHARGE;
        if (!recharge && (d->mon.status || !PbAccuracy(b, s, m))) { PbFailed(b, s); return; }
        unsigned turns; do { turns = PbRand(b) & 7u; } while (!turns); PbInflict(b, 1 - s, turns, PBRB_BM_ASLEEP); return;
    }
    if (effect == PBFX_POISON_EFFECT) {
        if (d->mon.status || (d->flags & PBF_SUBSTITUTE) || PbType(&d->mon, PBTYPE_POISON) || !PbAccuracy(b, s, m)) { PbFailed(b, s); return; }
        int toxic = m->id == PBMOVE_TOXIC; if (toxic) { d->flags |= PBF_TOXIC; d->toxicCounter = 0; }
        PbInflict(b, 1 - s, 8, toxic ? PBRB_BM_TOXIC : PBRB_BM_POISON); return;
    }
    if (effect == PBFX_PARALYZE_EFFECT) {
        if (d->mon.status || (m->type == PBTYPE_ELECTRIC && PbType(&d->mon, PBTYPE_GROUND)) || !PbAccuracy(b, s, m)) { PbFailed(b, s); return; }
        PbInflict(b, 1 - s, 64, PBRB_BM_PARALYZED); return;
    }
}
static void PbSecondary(PbrbBattle* b, int s, const PbrbGameMove* m) {
    PbrbBattleSide* d = &b->side[1 - s]; unsigned e = m->effect;
    if (e == PBFX_POISON_SIDE_EFFECT1 || e == PBFX_POISON_SIDE_EFFECT2 || e == PBFX_TWINEEDLE_EFFECT) {
        if (d->mon.status || (d->flags & PBF_SUBSTITUTE) || PbType(&d->mon, PBTYPE_POISON))return;
        if (PbRand(b) < (e == PBFX_POISON_SIDE_EFFECT2 ? 103u : 52u))PbInflict(b, 1 - s, 8, PBRB_BM_POISON); return;
    }
    if ((e >= PBFX_BURN_SIDE_EFFECT1 && e <= PBFX_PARALYZE_SIDE_EFFECT1) ||
        (e >= PBFX_BURN_SIDE_EFFECT2 && e <= PBFX_PARALYZE_SIDE_EFFECT2)) {
        if (d->flags & PBF_SUBSTITUTE)return;
        if (d->mon.status) { if ((d->mon.status & 32u) && m->type == PBTYPE_FIRE)PbInflict(b, 1 - s, 0, PBRB_BM_NONE); return; }
        if (PbType(&d->mon, m->type))return; unsigned offset = e >= PBFX_BURN_SIDE_EFFECT2 ? e - PBFX_BURN_SIDE_EFFECT2 : e - PBFX_BURN_SIDE_EFFECT1;
        if (PbRand(b) >= (e >= PBFX_BURN_SIDE_EFFECT2 ? 77u : 26u))return;
        if (offset == 0)PbInflict(b, 1 - s, 16, PBRB_BM_BURN);
        else if (offset == 1) { PbInflict(b, 1 - s, 32, PBRB_BM_FROZEN); if (!s)d->flags &= ~PBF_RECHARGE; }
        else PbInflict(b, 1 - s, 64, PBRB_BM_PARALYZED); return;
    }
    if (e == PBFX_FLINCH_SIDE_EFFECT1 || e == PBFX_FLINCH_SIDE_EFFECT2) {
        if (!(d->flags & PBF_SUBSTITUTE) && PbRand(b) < (e == PBFX_FLINCH_SIDE_EFFECT1 ? 26u : 77u)) {
            d->flags |= PBF_FLINCH; d->flags &= ~PBF_RECHARGE;
        }return;
    }
    if (e == PBFX_CONFUSION_SIDE_EFFECT) { if (PbRand(b) < 25)PbConfusion(b, 1 - s); return; }
    if (e >= PBFX_ATTACK_DOWN_SIDE_EFFECT && e <= PBFX_SPECIAL_DOWN_SIDE_EFFECT) {
        if ((d->flags & (PBF_SUBSTITUTE | PBF_MIST | PBF_INVULNERABLE)) || (s && PbRand(b) < 64u) || PbRand(b) >= 85)return;
        PbStat(b, s, 1 - s, e - PBFX_ATTACK_DOWN_SIDE_EFFECT, -1, 1); return;
    }
}
static unsigned PbMultiCount(PbrbBattle* b) { unsigned n = PbRand(b) & 3u; if (n >= 2)n = PbRand(b) & 3u; return n + 2u; }
static void PbUseMove(PbrbBattle* b, int s, unsigned id, unsigned slot, unsigned selector, int continuation, unsigned depth) {
    if (depth > 4) { PbFailed(b, s); return; }PbrbGameMove m; if (!PbrbGameMoveData(&b->rom, id, &m)) { b->error = 1; return; }
    PbrbBattleSide* a = &b->side[s]; PbrbBattleSide* d = &b->side[1 - s]; unsigned effect = m.effect;
    b->currentMove[s] = id; if (id == PBMOVE_RAGE && !(a->flags & PBF_RAGE))a->rageAccuracy = m.accuracy;
    PbMoveEvent(b, s, id);
    if (effect == PBFX_EFFECT_01 || effect == PBFX_SLEEP_EFFECT || effect == PBFX_BIDE_EFFECT || effect == PBFX_DISABLE_EFFECT ||
        (effect >= PBFX_ATTACK_UP1_EFFECT && effect <= PBFX_EVASION_UP1_EFFECT) ||
        (effect >= PBFX_ATTACK_DOWN1_EFFECT && effect <= PBFX_EVASION_DOWN1_EFFECT) ||
        (effect >= PBFX_ATTACK_UP2_EFFECT && effect <= PBFX_EVASION_UP2_EFFECT) ||
        (effect >= PBFX_ATTACK_DOWN2_EFFECT && effect <= PBFX_EVASION_DOWN2_EFFECT))b->lastDamage = 0;
    if (effect == PBFX_MIRROR_MOVE_EFFECT || effect == PBFX_METRONOME_EFFECT) {
        unsigned move = d->lastMove; b->lastDamage = 0;
        if (effect == PBFX_METRONOME_EFFECT) { do { move = PbRand(b); } while (!move || move >= PBMOVE_STRUGGLE || move == PBMOVE_METRONOME); }
        else if (!move || move == PBMOVE_MIRROR_MOVE) { PbFailed(b, s); return; }
        /* ReloadMoveData refunds the first PP, then the copied action spends
           it. A copied charging move spends it on the following turn. */
        PbrbGameMove copied; if (!PbrbGameMoveData(&b->rom, move, &copied)) { PbFailed(b, s); return; }
        if (!s && slot < 4 && (copied.effect == PBFX_CHARGE_EFFECT || copied.effect == PBFX_FLY_EFFECT || move == PBMOVE_STRUGGLE)) { ++a->mon.pp[slot]; PbSync(b, s); }
        PbUseMove(b, s, move, slot, selector, 0, depth + 1); return;
    }
    if ((effect == PBFX_CHARGE_EFFECT || effect == PBFX_FLY_EFFECT) && !continuation) {
        a->chargeMove = id; a->lockSlot = slot; if (id == PBMOVE_FLY || id == PBMOVE_DIG)a->flags |= PBF_INVULNERABLE;
        PbMessage(b, s, PBRB_BM_CHARGING, id); return;
    }
    if ((effect >= PBFX_ATTACK_UP1_EFFECT && effect <= PBFX_EVASION_UP1_EFFECT) ||
        (effect >= PBFX_ATTACK_UP2_EFFECT && effect <= PBFX_EVASION_UP2_EFFECT)) {
        unsigned first = effect >= PBFX_ATTACK_UP2_EFFECT ? PBFX_ATTACK_UP2_EFFECT : PBFX_ATTACK_UP1_EFFECT;
        /* Original Minimize changes the picture only after a successful
           evasion raise. A capped attempt must not create the mini sprite. */
        if (id == PBMOVE_MINIMIZE && a->stages[5] < 13)a->flags |= PBF_MINIMIZE;
        PbStat(b, s, s, effect - first, first == PBFX_ATTACK_UP2_EFFECT ? 2 : 1); return;
    }
    if ((effect >= PBFX_ATTACK_DOWN1_EFFECT && effect <= PBFX_EVASION_DOWN1_EFFECT) ||
        (effect >= PBFX_ATTACK_DOWN2_EFFECT && effect <= PBFX_EVASION_DOWN2_EFFECT)) {
        if ((d->flags & PBF_SUBSTITUTE) || (s && PbRand(b) < 64u) || !PbAccuracy(b, s, &m)) { PbFailed(b, s); return; }
        unsigned first = effect >= PBFX_ATTACK_DOWN2_EFFECT ? PBFX_ATTACK_DOWN2_EFFECT : PBFX_ATTACK_DOWN1_EFFECT;
        PbStat(b, s, 1 - s, effect - first, first == PBFX_ATTACK_DOWN2_EFFECT ? -2 : -1); return;
    }
    if (effect == PBFX_SLEEP_EFFECT || effect == PBFX_EFFECT_01 || effect == PBFX_POISON_EFFECT || effect == PBFX_PARALYZE_EFFECT) { PbPrimaryStatus(b, s, &m); return; }
    switch (effect) {
    case PBFX_CONVERSION_EFFECT:
        if (d->flags & PBF_INVULNERABLE) { PbFailed(b, s); return; }memcpy(a->mon.types, d->mon.types, 2); PbMessage(b, s, PBRB_BM_CONVERSION); return;
    case PBFX_HAZE_EFFECT: {
        for (int i = 0; i < 2; ++i) {
            PbrbBattleSide* v = &b->side[i]; memset(v->stages, 7, 6);
            for (unsigned j = 0; j < 4; ++j)v->stats[j] = PbBaseStat(&v->mon, j);
            v->flags &= ~(PBF_MIST | PBF_FOCUS | PBF_REFLECT | PBF_SCREEN | PBF_SEED | PBF_TOXIC | PBF_XACCURACY);
            v->confusion = v->disabledMove = v->disabledTurns = 0; v->disabledSlot = 4;
        }
        /* Haze invalidates only this turn's selection. If the cured target
           already acted, next turn's normal selection must replace it. */
        if (d->mon.status & 39u)b->currentMove[1 - s] = 255; d->mon.status = 0; PbSync(b, 1 - s);
        PbMessage(b, s, PBRB_BM_HAZE); return;
    }
    case PBFX_BIDE_EFFECT:
        a->bideTurns = (PbRand(b) & 1u) + 2u; a->bideDamage = 0; a->lockSlot = slot; PbMessage(b, s, PBRB_BM_BIDE); return;
    case PBFX_SWITCH_AND_TELEPORT_EFFECT:
        if (b->setup.trainerClass) { PbFailed(b, s); return; }
        if (a->mon.level < d->mon.level && PbBound(b, a->mon.level + d->mon.level + 1u) < d->mon.level / 4u) { PbFailed(b, s); return; }
        PbFinish(b, s ? PBRB_BATTLE_ENEMY_FLED : PBRB_BATTLE_FLEE); return;
    case PBFX_MIST_EFFECT: case PBFX_FOCUS_ENERGY_EFFECT: case PBFX_LIGHT_SCREEN_EFFECT: case PBFX_REFLECT_EFFECT: {
        unsigned flag = effect == PBFX_MIST_EFFECT ? PBF_MIST : effect == PBFX_FOCUS_ENERGY_EFFECT ? PBF_FOCUS : effect == PBFX_LIGHT_SCREEN_EFFECT ? PBF_SCREEN : PBF_REFLECT;
        int message = effect == PBFX_MIST_EFFECT ? PBRB_BM_MIST : effect == PBFX_FOCUS_ENERGY_EFFECT ? PBRB_BM_FOCUS : effect == PBFX_LIGHT_SCREEN_EFFECT ? PBRB_BM_LIGHT_SCREEN : PBRB_BM_REFLECT;
        if (a->flags & flag) { PbFailed(b, s); return; }a->flags |= flag; PbMessage(b, s, message); return;
    }
    case PBFX_CONFUSION_EFFECT:
        if (d->confusion || (d->flags & PBF_SUBSTITUTE) || !PbAccuracy(b, s, &m)) { PbFailed(b, s); return; }PbConfusion(b, 1 - s); return;
    case PBFX_HEAL_EFFECT: {
        unsigned hp = a->mon.hp, max = a->mon.maxHP, borrow = (hp >> 8) < (max >> 8) ? 1u : 0u;
        if ((((hp & 255u) - (max & 255u) - borrow) & 255u) == 0) { PbFailed(b, s); return; }
        if (id == PBMOVE_REST) { a->mon.status = 2; PbSync(b, s); PbEvent(b, PBRB_BE_STATUS, s, 2, 0); PbHP(b, s, max); }
        else PbHP(b, s, hp + max / 2u); return;
    }
    case PBFX_TRANSFORM_EFFECT:
        if (!s && (a->flags & PBF_INVULNERABLE)) { PbFailed(b, s); return; }
        a->flags |= PBF_TRANSFORM; a->mon.species = d->mon.species; a->mon.catchRate = d->mon.catchRate;
        memcpy(a->mon.types, d->mon.types, 2); memcpy(a->mon.dvs, d->mon.dvs, 2); memcpy(a->mon.moves, d->mon.moves, 4);
        for (unsigned i = 0; i < 4; ++i)a->mon.pp[i] = a->mon.moves[i] ? 5 : 0;
        a->mon.attack = d->mon.attack; a->mon.defense = d->mon.defense; a->mon.speed = d->mon.speed; a->mon.special = d->mon.special;
        memcpy(a->stats, d->stats, sizeof(a->stats)); memcpy(a->stages, d->stages, 6); PbMessage(b, s, PBRB_BM_TRANSFORM, d->mon.species); return;
    case PBFX_SUBSTITUTE_EFFECT: {
        unsigned cost = (a->mon.maxHP / 4u) & 255u;
        if ((a->flags & PBF_SUBSTITUTE) || a->mon.hp < cost) { PbFailed(b, s); return; }
        a->flags |= PBF_SUBSTITUTE; a->substituteHP = cost; PbHP(b, s, a->mon.hp - cost, PBRB_BM_SUBSTITUTE); return;
    }
    case PBFX_MIMIC_EFFECT: {
        if (!PbAccuracy(b, s, &m) || slot >= 4) { PbFailed(b, s); return; }
        if (!s) {
            b->mimicPendingSlot = slot; b->state = PBRB_BATTLE_MIMIC;
            PbEvent(b, PBRB_BE_MIMIC_OFFER, 0, slot, 0); return;
        }
        unsigned selected = selector;
        if (s) {
            if (!(d->mon.moves[0] | d->mon.moves[1] | d->mon.moves[2] | d->mon.moves[3])) { PbFailed(b, s); return; }
            do { selected = PbRand(b) & 3u; } while (!d->mon.moves[selected]);
        }
        if (selected >= 4 || !d->mon.moves[selected]) { PbFailed(b, s); return; }
        a->mon.moves[slot] = d->mon.moves[selected]; a->mimicSlot = slot;
        PbAnimation(b, s, PBMOVE_MIMIC);
        PbrbBattleEvent* event = PbEvent(b, PBRB_BE_LEARNED, s, a->mon.moves[slot], slot);
        if (event) { event->move = a->mon.moves[slot]; event->message = PBRB_BM_MIMIC_LEARNED; }return;
    }
    case PBFX_LEECH_SEED_EFFECT:
        if ((d->flags & PBF_SEED) || PbType(&d->mon, PBTYPE_GRASS) || !PbAccuracy(b, s, &m)) { PbFailed(b, s); return; }
        d->flags |= PBF_SEED; PbMessage(b, 1 - s, PBRB_BM_SEEDED); return;
    case PBFX_SPLASH_EFFECT:PbMessage(b, s, PBRB_BM_NOTHING_HAPPENED); return;
    case PBFX_DISABLE_EFFECT: {
        if (d->flags & PBF_RAGE)PbStat(b, 1 - s, 1 - s, 0, 1, 1);
        if (d->disabledMove || !PbAccuracy(b, s, &m)) { PbFailed(b, s); return; }
        unsigned n = 0; for (unsigned i = 0; i < 4; ++i)if (d->mon.moves[i] && (s ? (d->mon.pp[i] & 63u) != 0 : 1))++n;
        if (!n) { PbFailed(b, s); return; }unsigned selected;
        do { selected = PbRand(b) & 3u; } while (!d->mon.moves[selected] || (s && !(d->mon.pp[selected] & 63u)));
        d->disabledMove = d->mon.moves[selected]; d->disabledSlot = selected; d->disabledTurns = (PbRand(b) & 7u) + 1u;
        PbMessage(b, 1 - s, PBRB_BM_DISABLED, d->disabledMove); return;
    }
    default:break;
    }
    if (effect == PBFX_THRASH_PETAL_DANCE_EFFECT && !continuation) { a->lockMove = id; a->lockTurns = (PbRand(b) & 1u) + 2u; a->lockSlot = slot; }
    unsigned hits = 1, damage = 0; int critical = 0, hit = 1;
    if (effect == PBFX_TRAPPING_EFFECT) { d->flags &= ~PBF_RECHARGE; a->trapTurns = PbMultiCount(b) - 1u; a->lockMove = id; a->lockSlot = slot; }
    if (effect == PBFX_TWO_TO_FIVE_ATTACKS_EFFECT || effect == PBFX_EFFECT_1E)hits = PbMultiCount(b);
    if (effect == PBFX_ATTACK_TWICE_EFFECT || effect == PBFX_TWINEEDLE_EFFECT)hits = 2;
    if (id == PBMOVE_COUNTER) {
        PbrbGameMove previous;
        hit = PbrbGameMoveData(&b->rom, b->currentMove[1 - s], &previous) && previous.id != PBMOVE_COUNTER && previous.power &&
            (previous.type == PBTYPE_NORMAL || previous.type == PBTYPE_FIGHTING) && b->lastDamage;
        if (hit) { damage = PbMin(65535, b->lastDamage * 2u); hit = PbAccuracy(b, s, &m); }
    }
    else if (effect == PBFX_SUPER_FANG_EFFECT) { damage = PbMax(1, d->mon.hp / 2u); hit = PbAccuracy(b, s, &m); }
    else if (effect == PBFX_SPECIAL_DAMAGE_EFFECT) {
        damage = (id == PBMOVE_SONICBOOM) ? 20u : (id == PBMOVE_DRAGON_RAGE) ? 40u : a->mon.level;
        if (id == PBMOVE_PSYWAVE) {
            unsigned limit = a->mon.level + a->mon.level / 2u;
            if (limit <= 1) { b->error = 2; damage = 1; }
            else { do { damage = PbBound(b, limit); } while (!damage && !s); }
        }
        hit = PbAccuracy(b, s, &m);
    }
    else if (effect == PBFX_OHKO_EFFECT) { hit = a->stats[2] >= d->stats[2] && PbEffectiveness(b, m.type, 1 - s, 0) != 0 && PbAccuracy(b, s, &m); damage = 65535; }
    else { critical = PbCritical(b, s, &m); damage = PbDamage(b, s, &m, critical); hit = damage != 0 && PbAccuracy(b, s, &m); }
    if (!hit) {
        b->lastDamage = 0;
        if (m.power && id != PBMOVE_COUNTER && effect != PBFX_SPECIAL_DAMAGE_EFFECT && effect != PBFX_SUPER_FANG_EFFECT && PbEffectivenessFeedback(b, m.type, 1 - s) == 0)
            PbEvent(b, PBRB_BE_EFFECTIVE, s, 0, 0);
        else PbEvent(b, PBRB_BE_MISS, s, 0, 0); a->trapTurns = 0;
        if (effect == PBFX_JUMP_KICK_EFFECT) {
            PbMessage(b, s, PBRB_BM_RECOIL); if (a->flags & PBF_SUBSTITUTE) {
                unsigned hp = d->substituteHP; PbMessage(b, 1 - s, PBRB_BM_SUBSTITUTE_DAMAGED, hp);
                d->substituteHP = hp ? hp - 1 : 0; if (!hp)d->flags &= ~PBF_SUBSTITUTE;
                if (!hp)PbMessage(b, 1 - s, PBRB_BM_SUBSTITUTE_BROKE, d->substituteHP);
            }
            else PbHit(b, s, 1, id, 1);
        }
        if (effect == PBFX_EXPLODE_EFFECT)PbHP(b, s, 0); return;
    }
    unsigned dealt = 0, actualHits = 0; int subBroke = 0;
    for (unsigned h = 0; h < hits && d->mon.hp; ++h) {
        int sub = (d->flags & PBF_SUBSTITUTE) != 0; dealt = PbHit(b, 1 - s, damage, id); ++actualHits;
        if (critical)PbEvent(b, PBRB_BE_CRITICAL, s, 0, 0);
        if (effect == PBFX_OHKO_EFFECT)PbEvent(b, PBRB_BE_CRITICAL, s, 1, 0);
        if (effect != PBFX_SPECIAL_DAMAGE_EFFECT && effect != PBFX_SUPER_FANG_EFFECT && id != PBMOVE_COUNTER) { unsigned factor = PbEffectivenessFeedback(b, m.type, 1 - s); if (factor != 10)PbEvent(b, PBRB_BE_EFFECTIVE, s, factor, 0); }
        if (d->mon.hp && (d->flags & PBF_RAGE))PbStat(b, 1 - s, 1 - s, 0, 1, 1);
        if (sub && !(d->flags & PBF_SUBSTITUTE)) { subBroke = 1; if (!h)break; }
    }
    if (hits > 1 && d->mon.hp)PbMessage(b, s, PBRB_BM_HIT_COUNT, actualHits);
    if (effect == PBFX_TRAPPING_EFFECT) { a->trapDamage = dealt; d->flags |= PBF_TRAPPED_THIS_TURN; }
    if (!subBroke) {
        if (effect == PBFX_DRAIN_HP_EFFECT || effect == PBFX_DREAM_EATER_EFFECT) { b->lastDamage = PbMax(1, dealt / 2u); PbHP(b, s, a->mon.hp + b->lastDamage, PBRB_BM_DRAIN); }
        if (effect == PBFX_RECOIL_EFFECT) { unsigned recoil = PbMax(1, dealt / (id == PBMOVE_STRUGGLE ? 2u : 4u)); PbHP(b, s, a->mon.hp - PbMin(a->mon.hp, recoil), PBRB_BM_RECOIL); }
        if (effect == PBFX_EXPLODE_EFFECT)PbHP(b, s, 0);
        if (effect == PBFX_PAY_DAY_EFFECT) { unsigned amount = (a->mon.level * 2u) & 255u; b->payday = PbMin(999999, b->payday + amount); PbMessage(b, s, PBRB_BM_PAY_DAY, amount); }
        if (effect == PBFX_RAGE_EFFECT) { a->flags |= PBF_RAGE; a->lockSlot = slot; }
        if (d->mon.hp) {
            if (effect == PBFX_HYPER_BEAM_EFFECT)a->flags |= PBF_RECHARGE;
            PbSecondary(b, s, &m);
        }
    }
}
static unsigned PbAvailableMoves(const PbrbBattleSide* v, int player) {
    unsigned mask = 0; for (unsigned i = 0; i < 4; ++i)if (v->mon.moves[i] &&
        (!v->disabledMove || (v->disabledSlot < 4 ? i != v->disabledSlot : v->mon.moves[i] != v->disabledMove)) &&
        (!player || (v->mon.pp[i] & 63u)))mask |= 1u << i; return mask;
}
static unsigned PbPlannedMove(const PbrbBattleSide* v, unsigned slot) {
    if (v->chargeMove)return v->chargeMove; if (v->bideTurns)return PBMOVE_BIDE;
    if (v->lockTurns || v->trapTurns)return v->lockMove; if (v->flags & PBF_RAGE)return PBMOVE_RAGE;
    return slot < 4 ? (unsigned)v->mon.moves[slot] : (unsigned)PBMOVE_STRUGGLE;
}
static void PbAct(PbrbBattle* b, int s, unsigned slot, unsigned selector) {
    PbrbBattleSide* v = &b->side[s];
    if (b->currentMove[s] == 255)return; /* CANNOT_MOVE precedes status counters. */
    unsigned move = b->currentMove[s] ? b->currentMove[s] : PbPlannedMove(v, slot);
    ++v->turnsOut;
    if (!PbCanAct(b, s, move))return; if (b->setup.ghost) { PbMessage(b, s, PBRB_BM_NOTHING_HAPPENED); return; }
    if (v->bideTurns) {
        v->bideDamage = (v->bideDamage + b->lastDamage) & 65535u; if (!--v->bideTurns) {
            unsigned damage = (v->bideDamage * 2u) & 65535u; v->bideDamage = 0;
            PbMoveEvent(b, s, PBMOVE_BIDE); PbMessage(b, s, PBRB_BM_BIDE_RELEASE); if (damage)PbHit(b, 1 - s, damage, PBMOVE_BIDE); else PbFailed(b, s);
        }
        else PbMessage(b, s, PBRB_BM_BIDE); return;
    }
    if (v->trapTurns) {
        --v->trapTurns; b->side[1 - s].flags |= PBF_TRAPPED_THIS_TURN;
        PbMoveEvent(b, s, v->lockMove); PbHit(b, 1 - s, v->trapDamage, v->lockMove); return;
    }
    int continuation = 0, consume = 1;
    if (v->lockTurns) { --v->lockTurns; continuation = 1; consume = 0; if (!v->lockTurns) { v->lockMove = 0; PbConfusion(b, s); } }
    else if (v->chargeMove) { continuation = 1; slot = v->lockSlot; v->chargeMove = 0; v->flags &= ~PBF_INVULNERABLE; }
    else if (v->flags & PBF_RAGE) { continuation = 1; consume = 0; }
    else if (!s) { if (!PbDisobedience(b, &slot))return; move = PbPlannedMove(v, slot); }
    PbrbGameMove data; if (!PbrbGameMoveData(&b->rom, move, &data)) { b->error = 1; return; }
    if (!continuation && (data.effect == PBFX_CHARGE_EFFECT || data.effect == PBFX_FLY_EFFECT))consume = 0;
    if (!s && consume && slot < 4 && move != PBMOVE_STRUGGLE) { --v->mon.pp[slot]; PbSync(b, s); }
    PbUseMove(b, s, move, slot, selector, continuation, 0);
}
static unsigned PbEnemySlot(PbrbBattle* b) {
    PbrbBattleSide* v = &b->side[1]; if (v->chargeMove || v->bideTurns || v->lockTurns || v->trapTurns || (v->flags & PBF_RAGE))return v->lockSlot;
    unsigned mask = PbAvailableMoves(v, 0); if (!mask)return 4; int score[4] = { 10,10,10,10 };
    for (unsigned i = 0; i < 4; ++i)if (!(mask & (1u << i)))score[i] = 80;
    unsigned mods[4] = { 0,0,0,0 };
    if (b->setup.trainerClass) {
        unsigned at = PbTable(&b->rom, PBD_TrainerClassMoveChoiceModifications), n = 1;
        while (n < b->setup.trainerClass) { const unsigned char* p = PbBytes(&b->rom, at++, 1); if (!p) { b->error = 1; return 4; }if (!*p)++n; }
        for (unsigned j = 0; j < 4; ++j) { const unsigned char* p = PbBytes(&b->rom, at++, 1); if (!p) { b->error = 1; return 4; }if (!*p)break; if (*p < 4)mods[*p] = 1; }
    }
    for (unsigned i = 0; i < 4; ++i)if (v->mon.moves[i]) {
        PbrbGameMove m; PbrbGameMoveData(&b->rom, v->mon.moves[i], &m); unsigned e = m.effect;
        if (mods[1] && b->side[0].mon.status && !m.power && (e == PBFX_EFFECT_01 || e == PBFX_SLEEP_EFFECT || e == PBFX_POISON_EFFECT || e == PBFX_PARALYZE_EFFECT))score[i] += 5;
        if (mods[2] && v->turnsOut == 1 && ((e >= PBFX_ATTACK_UP1_EFFECT && e < PBFX_BIDE_EFFECT) || (e >= PBFX_ATTACK_UP2_EFFECT && e < PBFX_POISON_EFFECT)))--score[i];
        if (mods[3]) {
            unsigned type = PbEffectiveness(b, m.type, 0, 1);
            if (type > 16)--score[i]; else if (type < 16) {
                for (unsigned j = 0; j < 4; ++j)if (v->mon.moves[j]) {
                    PbrbGameMove other; PbrbGameMoveData(&b->rom, v->mon.moves[j], &other);
                    if (other.effect == PBFX_SUPER_FANG_EFFECT || other.effect == PBFX_SPECIAL_DAMAGE_EFFECT || other.effect == PBFX_FLY_EFFECT ||
                        (other.power && other.type != m.type)) {
                        ++score[i]; break;
                    }
                }
            }
        }
    }
    int best = 100; for (unsigned i = 0; i < 4; ++i)if ((mask & (1u << i)) && score[i] < best)best = score[i];
    /* SelectEnemyMove uses 25 percent, 50 percent and 75 percent - 1,
       then retries disabled, absent or AI-filtered slots. */
    unsigned choice; do {
        unsigned r = PbRand(b); choice = r < 63u ? 0u : r < 127u ? 1u : r < 190u ? 2u : 3u;
    } while (!(mask & (1u << choice)) || score[choice] != best); return choice;
}
static PbrbBattleEvent* PbItemEvent(PbrbBattle* b, int s, unsigned item, int consumed, unsigned target) {
    PbrbBattleEvent* e = PbEvent(b, PBRB_BE_ITEM, s, (unsigned)consumed, target); if (e) { e->item = item; e->party = (int)target; }return e;
}
static int PbTrainerAction(PbrbBattle* b) {
    PbrbBattleSide* v = &b->side[1]; if (!b->setup.trainerClass || !v->aiUses)return 0;
    unsigned r = PbRand(b), item = 0, next = 6; int change = 0;
    switch (b->setup.trainerClass) {
    case PBTRAINER_UNUSED_JUGGLER:case PBTRAINER_JUGGLER:if (r < 64u)change = 1; break;
    case PBTRAINER_BLACKBELT:if (r < 32)item = PBITEM_X_ATTACK; break;
    case PBTRAINER_GIOVANNI:if (r < 64u)item = PBITEM_GUARD_SPEC; break;
    case PBTRAINER_COOLTRAINER_M:if (r < 64u)item = PBITEM_X_ATTACK; break;
    case PBTRAINER_COOLTRAINER_F:if (v->mon.hp < v->mon.maxHP / 10u)item = PBITEM_HYPER_POTION; else if (v->mon.hp < v->mon.maxHP / 5u)change = 1; break;
    case PBTRAINER_BROCK:if (v->mon.status)item = PBITEM_FULL_HEAL; break;
    case PBTRAINER_MISTY:if (r < 64u)item = PBITEM_X_DEFEND; break;
    case PBTRAINER_LT_SURGE:if (r < 64u)item = PBITEM_X_SPEED; break;
    case PBTRAINER_ERIKA:if (r < 128u && v->mon.hp < v->mon.maxHP / 10u)item = PBITEM_SUPER_POTION; break;
    case PBTRAINER_KOGA:if (r < 64u)item = PBITEM_X_ATTACK; break;
    case PBTRAINER_BLAINE:if (r < 64u)item = PBITEM_SUPER_POTION; break;
    case PBTRAINER_SABRINA:if (r < 64u && v->mon.hp < v->mon.maxHP / 10u)item = PBITEM_HYPER_POTION; break;
    case PBTRAINER_RIVAL2:if (r < 32 && v->mon.hp < v->mon.maxHP / 5u)item = PBITEM_POTION; break;
    case PBTRAINER_RIVAL3:if (r < 32 && v->mon.hp < v->mon.maxHP / 5u)item = PBITEM_FULL_RESTORE; break;
    case PBTRAINER_LORELEI:if (r < 128u && v->mon.hp < v->mon.maxHP / 5u)item = PBITEM_SUPER_POTION; break;
    case PBTRAINER_BRUNO:if (r < 64u)item = PBITEM_X_DEFEND; break;
    case PBTRAINER_AGATHA:if (r < 20)change = 1; else if (r < 128u && v->mon.hp < v->mon.maxHP / 4u)item = PBITEM_SUPER_POTION; break;
    case PBTRAINER_LANCE:if (r < 128u && v->mon.hp < v->mon.maxHP / 5u)item = PBITEM_HYPER_POTION; break;
    default:break;
    }
    if (change && (next = PbNextAlive(b, 1)) < 6) { PbSync(b, 1); PbSend(b, 1, next, 1); return 1; }
    if (!item)return 0; --v->aiUses; PbItemEvent(b, 1, item, 1, b->active[1]);
    if (item >= PBITEM_X_ATTACK && item <= PBITEM_X_SPECIAL) { PbStat(b, 1, 1, item - PBITEM_X_ATTACK, 1); return 1; }
    if (item == PBITEM_GUARD_SPEC) { v->flags |= PBF_MIST; PbMessage(b, 1, PBRB_BM_MIST); return 1; }
    if (item == PBITEM_FULL_HEAL || item == PBITEM_FULL_RESTORE) { v->mon.status = 0; v->flags &= ~PBF_TOXIC; PbSync(b, 1); PbEvent(b, PBRB_BE_STATUS, 1, 0, 0); }
    if (item == PBITEM_FULL_RESTORE)PbHP(b, 1, v->mon.maxHP);
    else if (item == PBITEM_POTION || item == PBITEM_SUPER_POTION || item == PBITEM_HYPER_POTION)
        PbHP(b, 1, v->mon.hp + (item == PBITEM_POTION ? 20u : item == PBITEM_SUPER_POTION ? 50u : 200u));
    return 1;
}
static int PbRun(PbrbBattle* b, int forced) {
    if (b->setup.trainerClass) { PbMessage(b, 0, PBRB_BM_CANNOT_RUN); return 0; }
    ++b->runAttempts; unsigned speed = b->side[0].stats[2], enemy = b->side[1].stats[2];
    if (forced || b->setup.safari || b->setup.ghost || speed >= enemy) { PbFinish(b, PBRB_BATTLE_FLEE); return 1; }
    unsigned divisor = (enemy / 4u) & 255u; if (!divisor) { PbFinish(b, PBRB_BATTLE_FLEE); return 1; }
    unsigned factor = ((speed * 32u) & 65535u) / divisor + 30u * (b->runAttempts - 1u);
    if (factor > 255 || PbRand(b) <= factor) { PbFinish(b, PBRB_BATTLE_FLEE); return 1; }
    PbMessage(b, 0, PBRB_BM_RUN_FAILED); return 1;
}
static int PbBall(PbrbBattle* b, unsigned item) {
    PbItemEvent(b, 0, item, 1, b->active[1]);
    if (b->setup.trainerClass) { PbMessage(b, 1, PBRB_BM_BALL_BLOCKED); return 1; }
    if (b->setup.ghost || b->setup.noCatch) { PbMessage(b, 1, PBRB_BM_BALL_MISSED); return 1; }
    PbrbPlayMon* m = &b->side[1].mon; unsigned rate = b->setup.safari ? b->safariRate : m->catchRate;
    unsigned status = (m->status & 39u) ? 25u : m->status ? 12u : 0u;
    unsigned r = PbBound(b, item == PBITEM_GREAT_BALL ? 201u : (item == PBITEM_ULTRA_BALL || item == PBITEM_SAFARI_BALL) ? 151u : 256u);
    unsigned w = (m->maxHP * 255u / (item == PBITEM_GREAT_BALL ? 8u : 12u)) / PbMax(1, m->hp / 4u);
    int caught = item == PBITEM_MASTER_BALL || r < status || ((r - status) <= rate && (w > 255 || PbRand(b) <= w));
    unsigned shakes = 4;
    if (!caught) {
        unsigned ball = item == PBITEM_POKE_BALL ? 255u : item == PBITEM_GREAT_BALL ? 200u : 150u;
        unsigned z = (PbMin(w, 255) * (rate * 100u / ball)) / 255u + (status == 25 ? 10u : status ? 5u : 0u);
        shakes = z < 10 ? 0u : z < 30 ? 1u : z < 70 ? 2u : 3u;
    }
    PbrbBattleEvent* e = PbEvent(b, PBRB_BE_CAPTURE, 1, (unsigned)caught, shakes); if (e)e->item = item;
    if (caught) {
        const PbrbPlayMon* original = &b->party[1][b->active[1]]; unsigned species = (b->side[1].flags & PBF_TRANSFORM) ? PbrbGameSpecies(&b->rom, 132) : original->species;
        unsigned rng = b->rng; if (!PbrbGameMakeMon(&b->rom, &b->captured, species, original->level, &rng, b->setup.trainerID, 0, 0)) { b->error = 1; return 1; }
        memcpy(b->captured.dvs, original->dvs, 2); PbrbGameRecalculate(&b->rom, &b->captured, 0);
        b->captured.hp = PbMin(m->hp, b->captured.maxHP); b->captured.status = m->status; PbrbGameMonName(&b->rom, species, b->captured.nickname);
        if (e)e->species = species; PbFinish(b, PBRB_BATTLE_CAPTURE);
    }
    else PbMessage(b, 1, shakes ? PBRB_BM_BALL_BROKE : PBRB_BM_BALL_MISSED, shakes);
    return 1;
}
static int PbMedicineItem(unsigned item) {
    return (item >= PBITEM_ANTIDOTE && item <= PBITEM_POTION) || item == PBITEM_FULL_HEAL || item == PBITEM_REVIVE || item == PBITEM_MAX_REVIVE ||
        (item >= PBITEM_FRESH_WATER && item <= PBITEM_LEMONADE) || (item >= PBITEM_ETHER && item <= PBITEM_MAX_ELIXER);
}
static int PbPlayerItem(PbrbBattle* b, const PbrbBattleCommand* command) {
    unsigned item = command->item, target = command->target; PbrbBattleSide* v = &b->side[0];
    if (item >= PBITEM_MASTER_BALL && item <= PBITEM_POKE_BALL)return PbBall(b, item);
    if (item == PBITEM_POKE_DOLL) { if (b->setup.trainerClass) { PbFailed(b, 0); return 0; }PbItemEvent(b, 0, item, 1, b->active[0]); PbFinish(b, PBRB_BATTLE_FLEE); return 1; }
    if (item == PBITEM_POKE_FLUTE) {
        PbItemEvent(b, 0, item, 0, b->active[0]); int used = 0;
        for (unsigned i = 0; i < b->count[0]; ++i)if (b->party[0][i].status & 7u) { b->party[0][i].status = 0; used = 1; }
        for (int s = 0; s < 2; ++s)if (b->side[s].mon.status & 7u) { b->side[s].mon.status = 0; PbSync(b, s); PbEvent(b, PBRB_BE_STATUS, s, 0, 0); used = 1; }
        if (!used)PbMessage(b, 0, PBRB_BM_NOTHING_HAPPENED); return 1;
    }
    if (item == PBITEM_X_ACCURACY || item == PBITEM_GUARD_SPEC || item == PBITEM_DIRE_HIT) {
        unsigned flag = item == PBITEM_X_ACCURACY ? PBF_XACCURACY : item == PBITEM_GUARD_SPEC ? PBF_MIST : PBF_FOCUS;
        PbItemEvent(b, 0, item, 1, b->active[0]); v->flags |= flag;
        if (item != PBITEM_X_ACCURACY)PbMessage(b, 0, item == PBITEM_GUARD_SPEC ? PBRB_BM_MIST : PBRB_BM_FOCUS); return 1;
    }
    if (item >= PBITEM_X_ATTACK && item <= PBITEM_X_SPECIAL) { PbItemEvent(b, 0, item, 1, b->active[0]); PbStat(b, 0, 0, item - PBITEM_X_ATTACK, 1); return 1; }
    if (!PbMedicineItem(item) || target >= b->count[0])return 0;
    PbSync(b, 0); PbrbPlayMon next = b->party[0][target];
    if (!PbrbGameUseMedicine(&b->rom, &next, item, command->moveSlot)) { PbItemEvent(b, 0, item, 0, target); PbFailed(b, 0); return 0; }
    unsigned oldHP = b->party[0][target].hp, oldStatus = b->party[0][target].status;
    PbItemEvent(b, 0, item, 1, target); b->party[0][target] = next;
    if (!oldHP && next.hp && (b->fought & (1u << target)))b->participants |= 1u << target;
    if (target == b->active[0]) {
        v->mon.status = next.status; memcpy(v->mon.pp, next.pp, 4); PbHP(b, 0, next.hp);
        if (oldStatus != next.status) {
            /* A Full Restore used below full HP retains the original penalties
               and Toxic flag; status-only medicine copies the party stats. */
            if (item != PBITEM_FULL_RESTORE || oldHP == next.maxHP) {
                v->flags &= ~PBF_TOXIC;
                v->mon.maxHP = next.maxHP; for (unsigned i = 0; i < 4; ++i)v->stats[i] = PbBaseStat(&next, i);
            }
            PbEvent(b, PBRB_BE_STATUS, 0, next.status, oldStatus);
        }
    }
    else {
        PbrbBattleEvent* e = PbEvent(b, PBRB_BE_HP, 0, next.hp, oldHP); if (e) { e->party = (int)target; e->species = next.species; }
        if (oldStatus != next.status) { e = PbEvent(b, PBRB_BE_STATUS, 0, next.status, oldStatus); if (e) { e->party = (int)target; e->species = next.species; } }
    }
    return 1;
}
static void PbSafariTurn(PbrbBattle* b, const PbrbBattleCommand* command) {
    if (command->type == PBRB_BATTLE_RUN) { PbRun(b, 1); return; }
    if (command->type == PBRB_BATTLE_SAFARI_BALL) { --b->safariBalls; PbBall(b, PBITEM_SAFARI_BALL); if (b->state == PBRB_BATTLE_FINISHED)return; }
    else if (command->type == PBRB_BATTLE_SAFARI_BAIT) {
        b->safariAnger = 0; b->safariRate /= 2u;
        b->safariBait = PbMin(255, b->safariBait + PbBound(b, 5) + 1u); PbMessage(b, 1, PBRB_BM_SAFARI_BAIT, b->safariBait); PbAnimation(b, 0, 202, 70);
    }
    else {
        b->safariBait = 0; b->safariRate = PbMin(255, b->safariRate * 2u);
        b->safariAnger = PbMin(255, b->safariAnger + PbBound(b, 5) + 1u); PbMessage(b, 1, PBRB_BM_SAFARI_ROCK, b->safariAnger); PbAnimation(b, 0, 201, 70);
    }
    if (!b->safariBalls) { PbMessage(b, 1, PBRB_BM_SAFARI_NO_BALLS); PbFinish(b, PBRB_BATTLE_ENEMY_FLED); return; }
    int eating = b->safariBait != 0, angry = !eating && b->safariAnger != 0;
    if (b->safariBait)--b->safariBait; else if (b->safariAnger && !--b->safariAnger)b->safariRate = b->side[1].mon.catchRate;
    /* PrintSafariZoneBattleText runs before the escape roll, including the
       final eating/angry turn. Neutral turns do not print another message. */
    if (eating)PbMessage(b, 1, PBRB_BM_SAFARI_EATING);
    else if (angry)PbMessage(b, 1, PBRB_BM_SAFARI_ANGRY);
    unsigned speed = (b->side[1].stats[2] & 255u) * 2u;
    if (speed > 255) { PbFinish(b, PBRB_BATTLE_ENEMY_FLED); return; }
    if (b->safariBait)speed /= 4u; else if (b->safariAnger)speed = PbMin(255, speed * 2u);
    if (PbRand(b) < speed) { PbFinish(b, PBRB_BATTLE_ENEMY_FLED); return; }
}
static void PbResolveFaint(PbrbBattle* b);
static void PbExperienceContinue(PbrbBattle* b);
static void PbLearn(PbrbBattle* b, unsigned party, unsigned slot, unsigned move) {
    PbrbPlayMon* m = &b->party[0][party]; PbrbGameMove d; if (slot >= 4 || !PbrbGameMoveData(&b->rom, move, &d))return;
    unsigned old = m->moves[slot]; m->moves[slot] = (unsigned char)move; m->pp[slot] = d.pp;
    if (party == b->active[0] && !(b->side[0].flags & PBF_TRANSFORM)) { b->side[0].mon.moves[slot] = (unsigned char)move; b->side[0].mon.pp[slot] = d.pp; }
    PbrbBattleEvent* e = PbEvent(b, PBRB_BE_LEARNED, 0, move, old); if (e) { e->party = (int)party; e->species = m->species; e->move = move; }
}
static int PbOfferLearning(PbrbBattle* b) {
    PbrbPlayMon* m = &b->party[0][b->expParty];
    while (b->learnIndex < b->learnCount) {
        unsigned move = b->learnMoves[b->learnIndex], slot;
        for (slot = 0; slot < 4; ++slot)if (m->moves[slot] == move)break;
        if (slot < 4) { ++b->learnIndex; continue; }
        for (slot = 0; slot < 4 && m->moves[slot]; ++slot) {}
        if (slot < 4) { PbLearn(b, b->expParty, slot, move); ++b->learnIndex; continue; }
        b->state = PBRB_BATTLE_LEARN_MOVE; PbrbBattleEvent* e = PbEvent(b, PBRB_BE_LEARN_OFFER, 0, move, 0);
        if (e) { e->party = (int)b->expParty; e->species = m->species; e->move = move; }return 1;
    }return 0;
}
static unsigned PbBitCount(unsigned v) { unsigned n = 0; while (v) { n += v & 1u; v >>= 1; }return n; }
static void PbDivideExperience(PbrbBattle* b) {
    unsigned count = PbBitCount(b->expMask); if (count < 2)return;
    for (unsigned i = 0; i < 5; ++i)b->expStats[i] = (unsigned char)(b->expStats[i] / count); b->expBase /= count;
}
static void PbExperienceContinue(PbrbBattle* b) {
    b->state = PBRB_BATTLE_COMMAND;
    for (;;) {
        for (; b->expParty < b->count[0]; ++b->expParty) {
            unsigned i = b->expParty; PbrbPlayMon* m = &b->party[0][i];
            if (!(b->expMask & (1u << i)) || !m->hp)continue;
            for (unsigned j = 0; j < 5; ++j)m->statExp[j] = (unsigned short)PbMin(65535, m->statExp[j] + b->expStats[j]);
            unsigned amount = b->expBase * b->side[1].mon.level / 7u;
            if (m->ot != b->setup.trainerID)amount += amount / 2u; if (b->setup.trainerClass)amount += amount / 2u;
            m->experience = PbMin(PbrbGameExperience(&b->rom, m->species, 100), m->experience + amount);
            PbrbBattleEvent* e = PbEvent(b, PBRB_BE_EXP, 0, amount, m->experience); if (e) { e->party = (int)i; e->species = m->species; e->message = b->expPass ? 2 : m->ot != b->setup.trainerID ? 1 : 0; }
            unsigned old = m->level, level = old; while (level < 100 && m->experience >= PbrbGameExperience(&b->rom, m->species, level + 1u))++level;
            if (level == old)continue; m->level = (unsigned char)level; PbrbGameRecalculate(&b->rom, m, 1);
            if (i == b->active[0]) {
                PbrbBattleSide* v = &b->side[0]; v->mon.level = m->level; v->mon.hp = m->hp; v->mon.maxHP = m->maxHP; v->mon.experience = m->experience;
                if (!(v->flags & PBF_TRANSFORM)) { v->mon.attack = m->attack; v->mon.defense = m->defense; v->mon.speed = m->speed; v->mon.special = m->special; }
                memcpy(v->mon.statExp, m->statExp, sizeof(m->statExp)); PbInitialStats(b, 0);
            }
            e = PbEvent(b, PBRB_BE_LEVEL, 0, level, old); if (e) {
                e->party = (int)i; e->species = m->species;
                for (unsigned stat = 0; stat < 4; ++stat)e->statValues[stat] = PbBaseStat(m, stat);
            }
            b->learnCount = PbrbGameMovesAtLevel(&b->rom, m->species, level, b->learnMoves); b->learnIndex = 0;
            if (PbOfferLearning(b))return;
        }
        if (b->setup.expAll && !b->expPass) { b->expPass = 1; b->expParty = 0; b->expMask = (1u << b->count[0]) - 1u; PbDivideExperience(b); continue; }
        break;
    }
    b->participants = b->fought = 1u << b->active[0]; PbResolveFaint(b);
}
static void PbStartExperience(PbrbBattle* b) {
    PbrbGameBase base; if (!PbrbGameBaseData(&b->rom, b->party[1][b->active[1]].species, &base)) { b->error = 1; return; }
    b->expStats[0] = base.hp; b->expStats[1] = base.attack; b->expStats[2] = base.defense; b->expStats[3] = base.speed; b->expStats[4] = base.special;
    b->expBase = base.experience; b->expParty = b->expPass = 0; b->expMask = b->participants;
    if (b->setup.expAll) { for (unsigned i = 0; i < 5; ++i)b->expStats[i] /= 2; b->expBase /= 2; }
    PbDivideExperience(b); PbExperienceContinue(b);
}
static void PbSendReplacement(PbrbBattle* b) {
    if (b->pendingEnemy < 6) { unsigned slot = b->pendingEnemy; b->pendingEnemy = 6; PbSend(b, 1, slot, 1); }
    b->state = PBRB_BATTLE_COMMAND;
}
static void PbResolveFaint(PbrbBattle* b) {
    if (!PbAlive(b, 0)) { PbFinish(b, PBRB_BATTLE_LOSE); return; }
    if (!PbAlive(b, 1)) { PbFinish(b, PBRB_BATTLE_WIN); return; }
    if (!b->side[1].mon.hp)b->pendingEnemy = PbNextAlive(b, 1);
    if (!b->side[0].mon.hp) { b->state = PBRB_BATTLE_FORCE_SWITCH; PbEvent(b, PBRB_BE_FORCE_SWITCH, 0, 0, 0); return; }
    if (b->pendingEnemy < 6 && !b->setup.setMode && b->setup.trainerClass && b->count[0]>1) {
        b->state = PBRB_BATTLE_SWITCH_OFFER; PbrbBattleEvent* e = PbEvent(b, PBRB_BE_SWITCH_OFFER, 1, b->pendingEnemy, 0);
        if (e) { e->party = (int)b->pendingEnemy; e->species = b->party[1][b->pendingEnemy].species; }return;
    }
    PbSendReplacement(b);
}
static int PbCheckFaints(PbrbBattle* b) {
    if (b->side[0].mon.hp && b->side[1].mon.hp)return 0;
    for (int s = 1; s >= 0; --s)if (!b->side[s].mon.hp && !(b->faintMask & (1u << s))) {
        b->faintMask |= 1u << s; b->side[s].mon.status = 0; PbSync(b, s); PbEvent(b, PBRB_BE_FAINT, s, 0, 0);
        b->side[1 - s].trapTurns = 0; b->side[1 - s].bideDamage = 0;
        if (!s)b->participants &= ~(1u << b->active[0]);
    }
    if (!PbAlive(b, 0)) { PbFinish(b, PBRB_BATTLE_LOSE); return 1; }
    if (!b->side[1].mon.hp && !(b->awarded & (1u << b->active[1]))) { b->awarded |= 1u << b->active[1]; PbStartExperience(b); return 1; }
    PbResolveFaint(b); return 1;
}
static int PbAfterAction(PbrbBattle* b, int s) {
    if (b->state == PBRB_BATTLE_FINISHED || b->error)return 1;
    if (PbCheckFaints(b))return 1; PbResidual(b, s); return PbCheckFaints(b);
}
void PbrbBattleDefaultSetup(PbrbBattleSetup* setup) {
    if (!setup)return; memset(setup, 0, sizeof(*setup)); setup->seed = 0x50425242u; setup->safariBalls = 30;
}
int PbrbBattleBegin(PbrbBattle* b, const PbrbGameRom* r, const PbrbPlayMon* player, unsigned playerCount, unsigned active,
    const PbrbPlayMon* enemy, unsigned enemyCount, const PbrbBattleSetup* setup) {
    if (!b || !r || !r->data || !player || !enemy || !playerCount || playerCount > 6 || !enemyCount || enemyCount > 6 || active >= playerCount || !player[active].hp)return 0;
    for (unsigned i = 0; i < playerCount; ++i)if (!PbrbGameValidateMon(r, &player[i]))return 0;
    for (unsigned i = 0; i < enemyCount; ++i)if (!PbrbGameValidateMon(r, &enemy[i]))return 0;
    if (setup && (setup->trainerClass > 47 || setup->badges > 255 || setup->safariBalls > 30))return 0;
    memset(b, 0, sizeof(*b)); b->rom = *r; if (setup)b->setup = *setup; else PbrbBattleDefaultSetup(&b->setup);
    b->rng = b->setup.seed; b->count[0] = playerCount; b->count[1] = enemyCount; b->pendingEnemy = 6;
    memcpy(b->party[0], player, playerCount * sizeof(*player)); memcpy(b->party[1], enemy, enemyCount * sizeof(*enemy));
    unsigned foe = 0; while (foe < enemyCount && !enemy[foe].hp)++foe; if (foe == enemyCount)return 0;
    PbSend(b, 1, foe, 0); PbSend(b, 0, active, 0); b->safariRate = b->side[1].mon.catchRate; b->safariBalls = b->setup.safariBalls;
    PbEvent(b, PBRB_BE_SEND_OUT, 1, foe, 0); if (!b->setup.safari)PbEvent(b, PBRB_BE_SEND_OUT, 0, active, 0);
    b->state = PBRB_BATTLE_COMMAND; return !b->error;
}
int PbrbBattleNextEvent(PbrbBattle* b, PbrbBattleEvent* event) {
    if (!b || !event || b->eventRead >= b->eventCount)return 0; *event = b->events[b->eventRead++]; return 1;
}
const PbrbPlayMon* PbrbBattlePlayerParty(const PbrbBattle* b) { return b ? b->party[0] : 0; }
int PbrbBattleForcedMove(const PbrbBattle* b) {
    return b && b->state == PBRB_BATTLE_COMMAND && !b->setup.safari &&
        (b->side[0].chargeMove || b->side[0].lockTurns || (b->side[0].flags & (PBF_RAGE | PBF_RECHARGE))) ? 1 : 0;
}
int PbrbBattleSkipMoveMenu(const PbrbBattle* b) {
    if (!b || b->state != PBRB_BATTLE_COMMAND || b->setup.safari)return 0;
    const PbrbBattleSide* p = &b->side[0];
    return (p->mon.status & 39u) || p->bideTurns || p->trapTurns || b->side[1].trapTurns;
}
static unsigned PbSwapSlot(unsigned slot, unsigned first, unsigned second) {
    return slot == first ? second : slot == second ? first : slot;
}
static void PbSwapMovePair(PbrbPlayMon* m, unsigned first, unsigned second) {
    unsigned char x = m->moves[first]; m->moves[first] = m->moves[second]; m->moves[second] = x;
    x = m->pp[first]; m->pp[first] = m->pp[second]; m->pp[second] = x;
}
int PbrbBattleSwapMoves(PbrbBattle* b, unsigned first, unsigned second) {
    if (!b || b->error || b->state != PBRB_BATTLE_COMMAND || b->eventRead < b->eventCount ||
        first >= 4 || second >= 4 || !b->side[0].mon.moves[first] || !b->side[0].mon.moves[second])return 0;
    PbrbBattleSide* v = &b->side[0]; PbSwapMovePair(&v->mon, first, second);
    /* SwapMovesInMenu also mutates the permanent party while transformed;
       its copied battle moves/PP stay separate from those permanent bytes. */
    PbSwapMovePair(&b->party[0][b->active[0]], first, second);
    v->disabledSlot = PbSwapSlot(v->disabledSlot, first, second);
    v->mimicSlot = PbSwapSlot(v->mimicSlot, first, second);
    v->lockSlot = PbSwapSlot(v->lockSlot, first, second);
    b->currentSlot[0] = PbSwapSlot(b->currentSlot[0], first, second); return 1;
}
static int PbValidSwitch(const PbrbBattle* b, unsigned slot) { return slot < b->count[0] && slot != b->active[0] && b->party[0][slot].hp; }
static unsigned PbSelectEnemy(PbrbBattle* b) {
    const PbrbBattleSide* e = &b->side[1];
    if ((e->flags & (PBF_RECHARGE | PBF_RAGE)) || e->chargeMove || e->lockTurns ||
        (e->mon.status & 39u) || e->trapTurns || e->bideTurns)return b->currentSlot[1];
    if (b->side[0].trapTurns) { b->currentMove[1] = 255; return b->currentSlot[1]; }
    unsigned slot = PbEnemySlot(b); b->currentSlot[1] = slot; b->currentMove[1] = PbPlannedMove(e, slot); return slot;
}
int PbrbBattleSubmit(PbrbBattle* b, const PbrbBattleCommand* command) {
    if (!b || !command || b->error || b->state == PBRB_BATTLE_FINISHED || b->eventRead < b->eventCount)return 0;
    b->eventRead = b->eventCount = 0;
    if (b->state == PBRB_BATTLE_MIMIC) {
        if (command->type != PBRB_BATTLE_CHOOSE_MIMIC || command->slot >= 4 ||
            !b->side[1].mon.moves[command->slot] || b->mimicPendingSlot >= 4)return 0;
        PbrbBattleSide* p = &b->side[0]; unsigned move = b->side[1].mon.moves[command->slot];
        p->mon.moves[b->mimicPendingSlot] = (unsigned char)move; p->mimicSlot = b->mimicPendingSlot;
        b->state = PBRB_BATTLE_COMMAND; PbAnimation(b, 0, PBMOVE_MIMIC);
        PbrbBattleEvent* e = PbEvent(b, PBRB_BE_LEARNED, 0, move, b->mimicPendingSlot);
        if (e) { e->move = move; e->message = PBRB_BM_MIMIC_LEARNED; }
        if (PbAfterAction(b, 0))return 1;
        if (b->mimicEnemyPending) {
            b->mimicEnemyPending = 0;
            if (!PbTrainerAction(b))PbAct(b, 1, b->mimicEnemySlot, 0); PbAfterAction(b, 1);
        }
        return 1;
    }
    if (b->state == PBRB_BATTLE_LEARN_MOVE) {
        if (command->type != PBRB_BATTLE_LEARN || command->slot > 4)return 0;
        if (command->slot < 4)PbLearn(b, b->expParty, command->slot, b->learnMoves[b->learnIndex]);
        ++b->learnIndex; if (PbOfferLearning(b))return 1; ++b->expParty; PbExperienceContinue(b); return 1;
    }
    if (b->state == PBRB_BATTLE_FORCE_SWITCH || b->state == PBRB_BATTLE_SWITCH_OFFER) {
        if (b->state == PBRB_BATTLE_FORCE_SWITCH && command->type == PBRB_BATTLE_RUN && !b->setup.trainerClass)return PbRun(b, 0);
        if (command->type == PBRB_BATTLE_SWITCH && PbValidSwitch(b, command->slot)) {
            int shift = b->state == PBRB_BATTLE_SWITCH_OFFER; PbSync(b, 0);
            /* EnemySendOut shows the new opponent before SwitchPlayerMon
               handles an accepted Shift offer. Forced replacements retain
               their original player-first sequence. */
            if (shift) { PbSendReplacement(b); b->participants = b->fought = 0; }
            PbSend(b, 0, command->slot, 1); if (!shift)PbSendReplacement(b); return 1;
        }
        if (b->state == PBRB_BATTLE_SWITCH_OFFER && command->type == PBRB_BATTLE_DECLINE) { PbSendReplacement(b); return 1; }return 0;
    }
    if (b->setup.safari) {
        if (command->type != PBRB_BATTLE_RUN && command->type != PBRB_BATTLE_SAFARI_BAIT && command->type != PBRB_BATTLE_SAFARI_ROCK &&
            !(command->type == PBRB_BATTLE_SAFARI_BALL && b->safariBalls))return 0;
        ++b->turn; PbSafariTurn(b, command); return 1;
    }
    PbrbBattleSide* player = &b->side[0]; unsigned slot = command->slot;
    int locked = player->chargeMove || player->lockTurns || (player->flags & (PBF_RAGE | PBF_RECHARGE));
    int skipSelection = PbrbBattleSkipMoveMenu(b);
    if (locked && command->type != PBRB_BATTLE_MOVE)return 0;
    if (command->type == PBRB_BATTLE_MOVE) {
        if (locked || skipSelection)slot = b->currentSlot[0];
        else {
            unsigned mask = PbAvailableMoves(player, 1);
            if (!mask)slot = 4; else if (slot >= 4 || !(mask & (1u << slot))) { PbMessage(b, 0, slot < 4 && player->mon.moves[slot] == player->disabledMove ? PBRB_BM_DISABLED : PBRB_BM_NO_PP); return 0; }
        }
    }
    else if (command->type == PBRB_BATTLE_SWITCH) { if (!PbValidSwitch(b, slot))return 0; }
    else if (command->type != PBRB_BATTLE_ITEM && command->type != PBRB_BATTLE_RUN)return 0;
    if (command->type == PBRB_BATTLE_RUN && b->setup.trainerClass) { PbMessage(b, 0, PBRB_BM_CANNOT_RUN); return 0; }
    if (!(player->flags & (PBF_RAGE | PBF_RECHARGE))) { b->side[0].flags &= ~PBF_FLINCH; b->side[1].flags &= ~PBF_FLINCH; }
    b->side[0].flags &= ~PBF_TRAPPED_THIS_TURN; b->side[1].flags &= ~PBF_TRAPPED_THIS_TURN;
    unsigned enemySlot = 4;
    if (command->type == PBRB_BATTLE_ITEM) {
        if (!PbPlayerItem(b, command))return 0; ++b->turn;
        if (b->state == PBRB_BATTLE_FINISHED)return 1;
        enemySlot = PbSelectEnemy(b);
        if (!PbTrainerAction(b))PbAct(b, 1, enemySlot, 0); PbAfterAction(b, 1); return 1;
    }
    ++b->turn;
    if (command->type == PBRB_BATTLE_RUN) {
        PbRun(b, 0); if (b->state == PBRB_BATTLE_FINISHED)return 1;
        enemySlot = PbSelectEnemy(b);
        if (!PbTrainerAction(b))PbAct(b, 1, enemySlot, 0); PbAfterAction(b, 1); return 1;
    }
    if (command->type == PBRB_BATTLE_SWITCH) {
        PbSync(b, 0); PbSend(b, 0, slot, 1);
        enemySlot = PbSelectEnemy(b);
        if (!PbTrainerAction(b))PbAct(b, 1, enemySlot, 0); PbAfterAction(b, 1); return 1;
    }
    if (!locked && !skipSelection) { b->currentSlot[0] = slot; b->currentMove[0] = PbPlannedMove(player, slot); }
    else if (!locked && !(player->mon.status & 39u) && !player->bideTurns && !player->trapTurns && b->side[1].trapTurns)b->currentMove[0] = 255;
    enemySlot = PbSelectEnemy(b);
    int order[2] = { 0,1 }; int priority[2] = { 0,0 };
    for (int s = 0; s < 2; ++s)priority[s] = b->currentMove[s] == PBMOVE_QUICK_ATTACK ? 1 : b->currentMove[s] == PBMOVE_COUNTER ? -1 : 0;
    if (priority[1] > priority[0] || (priority[0] == priority[1] && (b->side[1].stats[2] > player->stats[2] ||
        (b->side[1].stats[2] == player->stats[2] && PbRand(b) >= 128u)))) {
        order[0] = 1; order[1] = 0;
    }
    for (int n = 0; n < 2; ++n) {
        int s = order[n]; if (s) { if (!PbTrainerAction(b))PbAct(b, 1, enemySlot, 0); }
        else PbAct(b, 0, slot, command->moveSlot);
        if (b->state == PBRB_BATTLE_MIMIC) { b->mimicEnemySlot = enemySlot; b->mimicEnemyPending = n == 0; return 1; }
        if (PbAfterAction(b, s))break;
    }
    return 1;
}
#endif
