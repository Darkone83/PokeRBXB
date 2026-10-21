/* Included once by the native play translation unit. C++98, no CRT formatters. */
#include "pbrb_game.h"
#include <string.h>
enum PgameTable {
    PG_BASE, PG_MEW, PG_DEX, PG_EVOS, PG_MON_NAMES, PG_MOVE_NAMES,
    PG_MOVES, PG_ITEM_NAMES, PG_PRICES, PG_TM, PG_TM_PRICES, PG_GROWTH, PG_KEY_FLAGS
};
static const unsigned kPgameTables[] = { 0x383deu,0x425bu,0x41024u,0x3b05cu,
    0x1c21eu,0xb0000u,0x38000u,0x472bu,0x4608u,0x13773u,0x7bfa7u,0x5901du,0xe799u };
static const unsigned char* PgameData(const PbrbGameRom* r, unsigned at, unsigned n) {
    return r && r->data && at <= r->bytes && n <= r->bytes - at ? r->data + at : 0;
}
static unsigned PgameRandom(unsigned* seed) {
    unsigned x = *seed; if (!x)x = 0x6d2b79f5u; x ^= x << 13; x ^= x >> 17; x ^= x << 5; *seed = x; return x;
}
unsigned PbrbGameDex(const PbrbGameRom* r, unsigned species) {
    const unsigned char* d = species && species <= 190 ? PgameData(r, kPgameTables[PG_DEX] + species - 1u, 1) : 0;
    return d && *d <= 151 ? *d : 0;
}
int PbrbGameValidSpecies(const PbrbGameRom* r, unsigned species) { return PbrbGameDex(r, species) != 0; }
unsigned PbrbGameSpecies(const PbrbGameRom* r, unsigned dex) {
    if (!dex || dex > 151)return 0;
    for (unsigned s = 1; s <= 190; ++s)if (PbrbGameDex(r, s) == dex)return s;
    return 0;
}
static const unsigned char* PgameBase(const PbrbGameRom* r, unsigned species) {
    unsigned d = PbrbGameDex(r, species); if (!d)return 0;
    return PgameData(r, d == 151 ? kPgameTables[PG_MEW] : kPgameTables[PG_BASE] + (d - 1u) * 28u, 28);
}
int PbrbGameBaseData(const PbrbGameRom* r, unsigned species, PbrbGameBase* out) {
    const unsigned char* d = PgameBase(r, species); if (!d || !out)return 0;
    out->dex = d[0]; out->hp = d[1]; out->attack = d[2]; out->defense = d[3]; out->speed = d[4]; out->special = d[5];
    out->types[0] = d[6]; out->types[1] = d[7]; out->catchRate = d[8]; out->experience = d[9];
    out->growth = d[19]; memcpy(out->moves, d + 15, 4); return 1;
}
int PbrbGameMoveData(const PbrbGameRom* r, unsigned move, PbrbGameMove* out) {
    const unsigned char* d = move && move <= 165 ? PgameData(r, kPgameTables[PG_MOVES] + (move - 1u) * 6u, 6) : 0;
    if (!d || !out)return 0; out->id = d[0]; out->effect = d[1]; out->power = d[2];
    out->type = d[3]; out->accuracy = d[4]; out->pp = d[5]; return 1;
}
void PbrbGameMonName(const PbrbGameRom* r, unsigned species, unsigned char out[11]) {
    memset(out, 0x50, 11); if (!PbrbGameValidSpecies(r, species))return;
    const unsigned char* d = PgameData(r, kPgameTables[PG_MON_NAMES] + (species - 1u) * 10u, 10);
    if (d)memcpy(out, d, 10);
}
static void PgameName(const PbrbGameRom* r, unsigned at, unsigned n, unsigned char out[16]) {
    memset(out, 0x50, 16); if (!n)return;
    while (n > 1 && at < r->bytes) { if (r->data[at++] == 0x50)--n; }
    for (unsigned i = 0; i < 15 && at < r->bytes && r->data[at] != 0x50; ++i, ++at)out[i] = r->data[at];
}
void PbrbGameMoveName(const PbrbGameRom* r, unsigned move, unsigned char out[16]) {
    PgameName(r, kPgameTables[PG_MOVE_NAMES], move <= 165 ? move : 0, out);
}
void PbrbGameItemName(const PbrbGameRom* r, unsigned item, unsigned char out[16]) {
    memset(out, 0x50, 16);
    if (item >= 0xc4 && item <= 0xfa) {
        unsigned n = item < 0xc9 ? item - 0xc3 : item - 0xc8;
        out[0] = (unsigned char)(item < 0xc9 ? 0x87 : 0x93); out[1] = 0x8c; out[2] = (unsigned char)(0xf6 + n / 10); out[3] = (unsigned char)(0xf6 + n % 10); return;
    }
    PgameName(r, kPgameTables[PG_ITEM_NAMES], item <= 0x61 ? item : 0, out);
}
unsigned PbrbGameExperience(const PbrbGameRom* r, unsigned species, unsigned level) {
    PbrbGameBase b; if (!PbrbGameBaseData(r, species, &b) || b.growth > 5 || level > 100)return 0;
    const unsigned char* d = PgameData(r, kPgameTables[PG_GROWTH] + b.growth * 4u, 4); if (!d)return 0;
    long v = (long)((level * level * level * (d[0] >> 4)) / (d[0] & 15));
    long q = (long)(level * level * (d[1] & 127)); v += (d[1] & 128) ? -q : q;
    v += (long)(level * d[2]); v -= (long)d[3]; return v > 0 ? (unsigned)v : 0;
}
void PbrbGameRecalculate(const PbrbGameRom* r, PbrbPlayMon* m, int preserveDamage) {
    PbrbGameBase b; if (!m || !PbrbGameBaseData(r, m->species, &b))return;
    unsigned dv[5], base[5]; dv[1] = m->dvs[0] >> 4; dv[2] = m->dvs[0] & 15;
    dv[3] = m->dvs[1] >> 4; dv[4] = m->dvs[1] & 15;
    dv[0] = ((dv[1] & 1) << 3) | ((dv[2] & 1) << 2) | ((dv[3] & 1) << 1) | (dv[4] & 1);
    base[0] = b.hp; base[1] = b.attack; base[2] = b.defense; base[3] = b.speed; base[4] = b.special;
    unsigned stats[5], old = m->maxHP, oldHP = m->hp;
    for (unsigned i = 0; i < 5; ++i) {
        unsigned root = 0; while (root < 255 && root * root < m->statExp[i])++root;
        stats[i] = ((2u * (base[i] + dv[i]) + root / 4u) * m->level) / 100u + (i ? 5u : m->level + 10u);
        if (stats[i] > 999)stats[i] = 999;
    }
    m->maxHP = stats[0]; m->attack = stats[1]; m->defense = stats[2]; m->speed = stats[3]; m->special = stats[4];
    m->types[0] = b.types[0]; m->types[1] = b.types[1]; m->catchRate = b.catchRate;
    if (!preserveDamage)m->hp = m->maxHP;
    else if (oldHP) { int hp = (int)oldHP + (int)m->maxHP - (int)old; m->hp = hp > 0 ? (unsigned)hp : 1u; if (m->hp > m->maxHP)m->hp = m->maxHP; }
}
static unsigned PgameEvos(const PbrbGameRom* r, unsigned species) {
    if (!PbrbGameValidSpecies(r, species))return 0;
    const unsigned char* p = PgameData(r, kPgameTables[PG_EVOS] + (species - 1u) * 2u, 2); if (!p)return 0;
    unsigned a = p[0] | ((unsigned)p[1] << 8); return a >= 0x4000 && a < 0x8000 ? 0x38000u + a - 0x4000u : 0;
}
static unsigned PgameLearnset(const PbrbGameRom* r, unsigned species) {
    unsigned at = PgameEvos(r, species); if (!at)return 0;
    for (unsigned n = 0; n < 16 && at < r->bytes; ++n) {
        unsigned kind = r->data[at++]; if (!kind)return at;
        if (kind > 3)return 0; at += kind == 2 ? 3u : 2u;
    }
    return 0;
}
unsigned PbrbGameMovesAtLevel(const PbrbGameRom* r, unsigned species, unsigned level, unsigned char out[8]) {
    memset(out, 0, 8); unsigned at = PgameLearnset(r, species), n = 0; if (!at)return 0;
    for (unsigned guard = 0; guard < 64 && at + 1 < r->bytes; ++guard, at += 2) {
        unsigned lv = r->data[at]; if (!lv || lv > level)break;
        if (lv == level && n < 8)out[n++] = r->data[at + 1];
    }
    return n;
}
unsigned PbrbGameEvolution(const PbrbGameRom* r, unsigned species, unsigned level, unsigned item, int trade) {
    unsigned at = PgameEvos(r, species); if (!at)return 0;
    for (unsigned n = 0; n < 16 && at + 3 < r->bytes; ++n) {
        unsigned kind = r->data[at++]; if (!kind)return 0;
        if (kind == 1) { unsigned min = r->data[at++], next = r->data[at++]; if (!trade && !item && level >= min)return next; }
        else if (kind == 2) { unsigned stone = r->data[at++], min = r->data[at++], next = r->data[at++]; if (!trade && item == stone && level >= min)return next; }
        else if (kind == 3) { unsigned min = r->data[at++], next = r->data[at++]; if (trade && level >= min)return next; }
        else return 0;
    }return 0;
}
unsigned PbrbGameMaxPP(const PbrbGameRom* r, unsigned move, unsigned ppByte) {
    PbrbGameMove d; if (!PbrbGameMoveData(r, move, &d))return 0;
    unsigned bonus = d.pp / 5u; if (bonus > 7)bonus = 7; return d.pp + bonus * (ppByte >> 6);
}
int PbrbGameMakeMon(const PbrbGameRom* r, PbrbPlayMon* m, unsigned species, unsigned level,
    unsigned* rng, unsigned ot, const unsigned char* trainerName, int trainer) {
    PbrbGameBase b; if (!m || !rng || !level || level > 100 || !PbrbGameBaseData(r, species, &b))return 0;
    memset(m, 0, sizeof(*m)); m->species = (unsigned char)species; m->level = (unsigned char)level; m->ot = ot & 65535u;
    m->dvs[0] = (unsigned char)(trainer ? 0x98 : PgameRandom(rng) & 255u); m->dvs[1] = (unsigned char)(trainer ? 0x88 : PgameRandom(rng) & 255u);
    memset(m->originalTrainer, 0x50, 11); if (trainerName)memcpy(m->originalTrainer, trainerName, 11);
    m->experience = PbrbGameExperience(r, species, level); memcpy(m->moves, b.moves, 4);
    unsigned at = PgameLearnset(r, species);
    for (unsigned guard = 0; at && guard < 64 && at + 1 < r->bytes; ++guard, at += 2) {
        unsigned lv = r->data[at], move = r->data[at + 1]; if (!lv || lv > level)break;
        int slot = 0; for (; slot < 4; ++slot)if (m->moves[slot] == move)break; if (slot < 4)continue;
        for (slot = 0; slot < 4 && m->moves[slot]; ++slot) {}
        if (slot == 4) { memmove(m->moves, m->moves + 1, 3); slot = 3; }m->moves[slot] = (unsigned char)move;
    }
    PbrbGameRecalculate(r, m, 0); PbrbGameHeal(r, m); PbrbGameMonName(r, species, m->nickname); return 1;
}
unsigned PbrbGameTM(const PbrbGameRom* r, unsigned tm) {
    if (tm < 1 || tm>55)return 0; const unsigned char* p = PgameData(r, kPgameTables[PG_TM] + tm - 1u, 1); return p ? *p : 0;
}
int PbrbGameCanLearnTM(const PbrbGameRom* r, unsigned species, unsigned tm) {
    const unsigned char* d = PgameBase(r, species); return d && tm >= 1 && tm <= 55 && (d[20 + (tm - 1) / 8] & (1u << ((tm - 1) % 8))) ? 1 : 0;
}
void PbrbGameHeal(const PbrbGameRom* r, PbrbPlayMon* m) {
    if (!m || !m->species)return; m->hp = m->maxHP; m->status = 0;
    for (unsigned i = 0; i < 4; ++i)m->pp[i] = (unsigned char)((m->pp[i] & 192u) | PbrbGameMaxPP(r, m->moves[i], m->pp[i]));
}
void PbrbGameInit(PbrbGameState* s) {
    memset(s, 0, sizeof(*s)); s->money = 3000; s->pcItems[0].id = 0x14; s->pcItems[0].count = 1; s->pcItemCount = 1;
    s->lastHealMap = 0; s->lastHealWarp = 0;
}
void PbrbGameMarkDex(const PbrbGameRom* r, PbrbGameState* s, unsigned species, int owned) {
    unsigned d = PbrbGameDex(r, species); if (!d)return; --d; s->seen[d / 8] |= (unsigned char)(1u << (d % 8));
    if (owned)s->owned[d / 8] |= (unsigned char)(1u << (d % 8));
}
unsigned PbrbGameDexCount(const unsigned char bits[19]) {
    unsigned n = 0; for (unsigned i = 0; i < 151; ++i)if (bits[i / 8] & (1u << (i % 8)))++n; return n;
}
int PbrbGameAddMon(const PbrbGameRom* r, PbrbGameState* s, const PbrbPlayMon* m) {
    if (!m || !PbrbGameValidSpecies(r, m->species))return 0;
    if (s->partyCount < 6)s->party[s->partyCount++] = *m;
    else if (s->currentBox < 12 && s->boxCount[s->currentBox] < 20) {
        PbrbPlayMon copy = *m; PbrbGameHeal(r, &copy);
        unsigned box = s->currentBox; memmove(s->boxes[box] + 1, s->boxes[box], s->boxCount[box] * sizeof(copy));
        s->boxes[box][0] = copy; ++s->boxCount[box];
    }
    else return 0;
    PbrbGameMarkDex(r, s, m->species, 1); return 1;
}
static int PgameValidItem(unsigned item) { return (item >= 1 && item <= 0x53) || (item >= 0xc4 && item <= 0xfa); }
int PbrbGameAddItem(PbrbGameItem* list, unsigned char* count, unsigned capacity, unsigned item, unsigned n) {
    if (!list || !count || *count > capacity || !PgameValidItem(item) || !n || n > 99)return 0;
    /* inventory.asm scans in order. Overflow fills a stack and continues
       to later stacks/new space. A full inventory rejects the first
       overflowing match, even if a later duplicate has room. */
    for (unsigned i = 0; i < *count; ++i)if (list[i].id == item) {
        unsigned total = list[i].count + n;
        if (total <= 99u) { list[i].count = (unsigned char)total; return 1; }
        if (*count == capacity)return 0;
        list[i].count = 99; n = total - 99u;
    }
    if (*count == capacity)return 0; list[*count].id = (unsigned char)item; list[(*count)++].count = (unsigned char)n; return 1;
}
int PbrbGameRemoveItemAt(PbrbGameItem* list, unsigned char* count, unsigned slot, unsigned n) {
    if (!list || !count || slot >= *count || !n || list[slot].count < n)return 0;
    list[slot].count = (unsigned char)(list[slot].count - n);
    if (!list[slot].count) { memmove(list + slot, list + slot + 1, (*count - slot - 1u) * sizeof(*list)); --*count; memset(list + *count, 0, sizeof(*list)); }
    return 1;
}
int PbrbGameRemoveItem(PbrbGameItem* list, unsigned char* count, unsigned item, unsigned n) {
    if (!list || !count || !n || PbrbGameCountItem(list, *count, item) < n)return 0;
    for (unsigned i = 0; i < *count && n;) {
        if (list[i].id != item) { ++i; continue; }
        unsigned take = list[i].count < n ? list[i].count : n, old = *count;
        if (!PbrbGameRemoveItemAt(list, count, i, take))return 0; n -= take;
        if (*count == old)++i;
    }return n == 0;
}
unsigned PbrbGameCountItem(const PbrbGameItem* list, unsigned count, unsigned item) {
    if (!list)return 0; unsigned n = 0; for (unsigned i = 0; i < count; ++i)if (list[i].id == item)n += list[i].count; return n;
}
int PbrbGameSwapItems(PbrbGameItem* list, unsigned char* count, unsigned first, unsigned second) {
    if (!list || !count || first >= *count || second >= *count || first == second)return 0;
    if (list[first].id != list[second].id) { PbrbGameItem item = list[first]; list[first] = list[second]; list[second] = item; return 1; }
    unsigned total = list[first].count + list[second].count;
    if (total > 99u) { list[first].count = (unsigned char)(total - 99u); list[second].count = 99; }
    else { list[second].count = (unsigned char)total; PbrbGameRemoveItemAt(list, count, first, list[first].count); }
    return 1;
}
int PbrbGameIsKeyItem(const PbrbGameRom* r, unsigned item) {
    if (item >= 0xc4u)return item < 0xc9u;
    if (!item || item > 0x53u)return 1;
    const unsigned char* flags = PgameData(r, kPgameTables[PG_KEY_FLAGS] + (item - 1u) / 8u, 1);
    return !flags || (*flags & (1u << ((item - 1u) & 7u))) != 0;
}
unsigned PbrbGameItemPrice(const PbrbGameRom* r, unsigned item) {
    if (item >= 0xc9 && item <= 0xfa) {
        unsigned n = item - 0xc9; const unsigned char* p = PgameData(r, kPgameTables[PG_TM_PRICES] + n / 2u, 1);
        return p ? ((n & 1u) ? *p & 15u : *p >> 4) * 1000u : 0;
    }
    if (!item || item > 0x53)return 0; const unsigned char* p = PgameData(r, kPgameTables[PG_PRICES] + (item - 1u) * 3u, 3);
    if (!p)return 0; unsigned v = 0; for (int i = 0; i < 3; ++i)v = v * 100u + (p[i] >> 4) * 10u + (p[i] & 15u); return v;
}
int PbrbGameUseMedicine(const PbrbGameRom* r, PbrbPlayMon* m, unsigned item, unsigned slot) {
    if (!m || !m->species)return 0;
    if (item == 0x35 || item == 0x36) { if (m->hp)return 0; m->hp = item == 0x36 ? m->maxHP : m->maxHP / 2u; if (!m->hp)m->hp = 1; m->status = 0; return 1; }
    if (item >= 0x23 && item <= 0x27) {
        unsigned i = item - 0x23; if (m->statExp[i] >= 25600u)return 0;
        unsigned hp = m->hp; m->statExp[i] = (unsigned short)(m->statExp[i] + 2560u); PbrbGameRecalculate(r, m, 1); m->hp = hp; return 1;
    }
    if (item == 0x28) {
        if (m->level >= 100)return 0; unsigned hp = m->hp, maximum = m->maxHP;
        ++m->level; m->experience = PbrbGameExperience(r, m->species, m->level); PbrbGameRecalculate(r, m, 1);
        m->hp = hp + m->maxHP - maximum; return 1;
    }
    if (item >= 0x4f && item <= 0x53) {
        int used = 0;
        for (unsigned i = 0; i < 4; ++i) {
            if (item < 0x52 && i != slot)continue; if (!m->moves[i])continue;
            unsigned old = m->pp[i], maximum = PbrbGameMaxPP(r, m->moves[i], old), current = old & 63u;
            if (item == 0x4f) {
                if ((old >> 6) >= 3)continue; unsigned next = old + 64u;
                unsigned gain = PbrbGameMaxPP(r, m->moves[i], next) - maximum; m->pp[i] = (unsigned char)(next + gain); used = 1;
            }
            else if (item == 0x51 || item == 0x53) { if (current != maximum) { m->pp[i] = (unsigned char)((old & 192u) | maximum); used = 1; } }
            else if (current < maximum) { current += 10; if (current > maximum)current = maximum; m->pp[i] = (unsigned char)((old & 192u) | current); used = 1; }
        }return used;
    }
    if (item >= 0x0b && item <= 0x0f) { static const unsigned mask[] = { 8,16,32,7,64 }; if (!(m->status & mask[item - 0x0b]))return 0; m->status = 0; return 1; }
    if (item == 0x34) { if (!m->status)return 0; m->status = 0; return 1; }
    if (!m->hp)return 0;
    unsigned restore = 0;
    switch (item) {
    case 0x10:case 0x11:restore = m->maxHP; break; case 0x12:restore = 200; break;
    case 0x13:restore = 50; break; case 0x14:restore = 20; break; case 0x3c:restore = 50; break;
    case 0x3d:restore = 60; break; case 0x3e:restore = 80; break; default:return 0;
    }
    if (m->hp == m->maxHP && !(item == 0x10 && m->status))return 0;
    m->hp += restore; if (m->hp > m->maxHP)m->hp = m->maxHP; if (item == 0x10)m->status = 0; return 1;
}
static int PgameValidName(const unsigned char* s) {
    int end = 0; for (int i = 0; i < 11; ++i) { if (s[i] == 0x50)end = 1; else if (end || i == 10 || s[i] < 0x60)return 0; }return end;
}
static int PgameValidOT(const unsigned char* s) {
    /* The original NPC trade records use the TRAINER text token as OT. */
    if (s[0] != 0x5d)return PgameValidName(s);
    for (unsigned i = 1; i < 11; ++i)if (s[i] != 0x50)return 0; return 1;
}
int PbrbGameValidateMon(const PbrbGameRom* r, const PbrbPlayMon* m) {
    if (!m || !PbrbGameValidSpecies(r, m->species) || !m->level || m->level > 100 || m->status > 127 || m->hp > m->maxHP ||
        !m->maxHP || m->maxHP > 999 || !m->attack || m->attack > 999 || !m->defense || m->defense > 999 ||
        !m->speed || m->speed > 999 || !m->special || m->special > 999 || m->ot > 65535 ||
        !PgameValidName(m->nickname) || !PgameValidOT(m->originalTrainer))return 0;
    if (m->experience<PbrbGameExperience(r, m->species, m->level) || m->experience>PbrbGameExperience(r, m->species, 100))return 0;
    for (unsigned i = 0; i < 4; ++i)if (m->moves[i] > 165 || (m->pp[i] & 63u) > PbrbGameMaxPP(r, m->moves[i], m->pp[i]) || (!m->moves[i] && m->pp[i]))return 0;
    return 1;
}
struct PgameCodec { unsigned char* out; const unsigned char* in; size_t size, at; int ok; };
static void PgamePut(PgameCodec* c, unsigned v, unsigned n) {
    if (!c->ok || n > c->size - c->at) { c->ok = 0; return; }for (unsigned i = 0; i < n; ++i)c->out[c->at++] = (unsigned char)(v >> (i * 8u));
}
static unsigned PgameGet(PgameCodec* c, unsigned n) {
    if (!c->ok || n > c->size - c->at) { c->ok = 0; return 0; }unsigned v = 0; for (unsigned i = 0; i < n; ++i)v |= (unsigned)c->in[c->at++] << (i * 8u); return v;
}
static void PgameWriteMon(PgameCodec* c, const PbrbPlayMon* m) {
    PgamePut(c, m->species, 1); PgamePut(c, m->level, 1); PgamePut(c, m->status, 1);
    for (int i = 0; i < 4; ++i) { PgamePut(c, m->moves[i], 1); PgamePut(c, m->pp[i], 1); }
    for (int i = 0; i < 2; ++i)PgamePut(c, m->dvs[i], 1);
    for (int i = 0; i < 11; ++i) { PgamePut(c, m->nickname[i], 1); PgamePut(c, m->originalTrainer[i], 1); }
    PgamePut(c, m->hp, 2); PgamePut(c, m->maxHP, 2); PgamePut(c, m->attack, 2); PgamePut(c, m->defense, 2); PgamePut(c, m->speed, 2); PgamePut(c, m->special, 2);
    PgamePut(c, m->experience, 3); PgamePut(c, m->ot, 2); for (int i = 0; i < 5; ++i)PgamePut(c, m->statExp[i], 2);
}
static void PgameReadMon(PgameCodec* c, const PbrbGameRom* r, PbrbPlayMon* m, int daycare = 0) {
    memset(m, 0, sizeof(*m)); m->species = (unsigned char)PgameGet(c, 1); m->level = (unsigned char)PgameGet(c, 1); m->status = (unsigned char)PgameGet(c, 1);
    for (int i = 0; i < 4; ++i) { m->moves[i] = (unsigned char)PgameGet(c, 1); m->pp[i] = (unsigned char)PgameGet(c, 1); }
    for (int i = 0; i < 2; ++i)m->dvs[i] = (unsigned char)PgameGet(c, 1);
    for (int i = 0; i < 11; ++i) { m->nickname[i] = (unsigned char)PgameGet(c, 1); m->originalTrainer[i] = (unsigned char)PgameGet(c, 1); }
    m->hp = PgameGet(c, 2); m->maxHP = PgameGet(c, 2); m->attack = PgameGet(c, 2); m->defense = PgameGet(c, 2); m->speed = PgameGet(c, 2); m->special = PgameGet(c, 2);
    m->experience = PgameGet(c, 3); m->ot = PgameGet(c, 2); for (int i = 0; i < 5; ++i)m->statExp[i] = (unsigned short)PgameGet(c, 2);
    PbrbGameBase b; if (!PbrbGameBaseData(r, m->species, &b)) { c->ok = 0; return; }
    m->types[0] = b.types[0]; m->types[1] = b.types[1]; m->catchRate = b.catchRate;
    unsigned storedExperience = m->experience;
    if (daycare) {
        if (storedExperience > 0x50ffffu)c->ok = 0;
        unsigned max = PbrbGameExperience(r, m->species, 100); if (m->experience > max)m->experience = max;
    }
    if (!PbrbGameValidateMon(r, m))c->ok = 0; m->experience = storedExperience;
}
size_t PbrbGameSerialize(const PbrbGameState* s, unsigned char* out, size_t bytes) {
    if (!s || !out || s->partyCount > 6 || s->currentBox >= 12 || s->bagCount > 20 || s->pcItemCount > 50 || s->daycareOccupied > 1 || s->hallOfFameCount > 50 || s->hallOfFameTotal > 255 || s->hallOfFameTotal < s->hallOfFameCount)return 0;
    PgameCodec c = { out,0,bytes,0,1 }; PgamePut(&c, 0x324d4750u, 4);
    PgamePut(&c, s->partyCount, 1); PgamePut(&c, s->currentBox, 1); PgamePut(&c, s->active, 1);
    for (unsigned i = 0; i < s->partyCount; ++i)PgameWriteMon(&c, s->party + i);
    for (unsigned b = 0; b < 12; ++b) { if (s->boxCount[b] > 20)return 0; PgamePut(&c, s->boxCount[b], 1); for (unsigned i = 0; i < s->boxCount[b]; ++i)PgameWriteMon(&c, s->boxes[b] + i); }
    PgamePut(&c, s->bagCount, 1); for (unsigned i = 0; i < s->bagCount; ++i) { PgamePut(&c, s->bag[i].id, 1); PgamePut(&c, s->bag[i].count, 1); }
    PgamePut(&c, s->pcItemCount, 1); for (unsigned i = 0; i < s->pcItemCount; ++i) { PgamePut(&c, s->pcItems[i].id, 1); PgamePut(&c, s->pcItems[i].count, 1); }
    for (int i = 0; i < 19; ++i) { PgamePut(&c, s->seen[i], 1); PgamePut(&c, s->owned[i], 1); }
    PgamePut(&c, s->badges, 1); PgamePut(&c, s->pokedex, 1); PgamePut(&c, s->money, 3); PgamePut(&c, s->coins, 2);
    PgamePut(&c, s->repelSteps, 2); PgamePut(&c, s->steps, 4); PgamePut(&c, s->safariBalls, 1); PgamePut(&c, s->safariSteps, 2); PgamePut(&c, s->hallOfFameCount, 2);
    PgamePut(&c, s->bicycle, 1); PgamePut(&c, s->surfing, 1); PgamePut(&c, s->lastHealMap, 1); PgamePut(&c, s->lastHealWarp, 1);
    PgamePut(&c, s->hallOfFameTotal, 1); PgamePut(&c, s->daycareOccupied, 1);
    if (s->daycareOccupied) { PgamePut(&c, s->daycareLevel, 1); PgamePut(&c, s->daycareSteps, 4); PgameWriteMon(&c, &s->daycare); }
    for (unsigned h = 0; h < s->hallOfFameCount; ++h) {
        const PbrbGameHallRecord* team = s->hallOfFame + h; if (!team->count || team->count > 6)return 0;
        PgamePut(&c, team->count, 1);
        for (unsigned m = 0; m < team->count; ++m) {
            PgamePut(&c, team->mons[m].species, 1); PgamePut(&c, team->mons[m].level, 1);
            for (unsigned i = 0; i < 11; ++i)PgamePut(&c, team->mons[m].nickname[i], 1);
        }
    }
    return c.ok ? c.at : 0;
}
static void PgameReadItems(PgameCodec* c, PbrbGameItem* items, unsigned char* count, unsigned max) {
    *count = (unsigned char)PgameGet(c, 1); if (*count > max) { c->ok = 0; return; }
    for (unsigned i = 0; i < *count; ++i) {
        items[i].id = (unsigned char)PgameGet(c, 1); items[i].count = (unsigned char)PgameGet(c, 1);
        if (!PgameValidItem(items[i].id) || !items[i].count || items[i].count > 99)c->ok = 0;
    }
}
int PbrbGameDeserialize(const PbrbGameRom* r, PbrbGameState* s, const unsigned char* data, size_t bytes) {
    if (!r || !s || !data)return 0; static PbrbGameState n; memset(&n, 0, sizeof(n)); PgameCodec c = { 0,data,bytes,0,1 };
    unsigned format = PgameGet(&c, 4); if (format != 0x314d4750u && format != 0x324d4750u)return 0;
    n.partyCount = (unsigned char)PgameGet(&c, 1); n.currentBox = (unsigned char)PgameGet(&c, 1); n.active = (unsigned char)PgameGet(&c, 1);
    if (n.partyCount > 6 || n.currentBox >= 12 || n.active > 5 || (n.partyCount && n.active >= n.partyCount))return 0;
    for (unsigned i = 0; i < n.partyCount; ++i)PgameReadMon(&c, r, n.party + i);
    for (unsigned b = 0; b < 12; ++b) { n.boxCount[b] = (unsigned char)PgameGet(&c, 1); if (n.boxCount[b] > 20)return 0; for (unsigned i = 0; i < n.boxCount[b]; ++i)PgameReadMon(&c, r, n.boxes[b] + i); }
    PgameReadItems(&c, n.bag, &n.bagCount, 20); PgameReadItems(&c, n.pcItems, &n.pcItemCount, 50);
    for (int i = 0; i < 19; ++i) { n.seen[i] = (unsigned char)PgameGet(&c, 1); n.owned[i] = (unsigned char)PgameGet(&c, 1); if (n.owned[i] & ~n.seen[i])c.ok = 0; }
    n.badges = PgameGet(&c, 1); n.pokedex = PgameGet(&c, 1); n.money = PgameGet(&c, 3); n.coins = PgameGet(&c, 2); n.repelSteps = PgameGet(&c, 2); n.steps = PgameGet(&c, 4);
    n.safariBalls = PgameGet(&c, 1); n.safariSteps = PgameGet(&c, 2); n.hallOfFameCount = PgameGet(&c, 2); n.bicycle = (unsigned char)PgameGet(&c, 1); n.surfing = (unsigned char)PgameGet(&c, 1);
    n.lastHealMap = (unsigned char)PgameGet(&c, 1); n.lastHealWarp = (unsigned char)PgameGet(&c, 1);
    if (n.hallOfFameCount > 50)return 0;
    if (format == 0x324d4750u) {
        n.hallOfFameTotal = PgameGet(&c, 1); if (n.hallOfFameTotal < n.hallOfFameCount)return 0;
        n.daycareOccupied = PgameGet(&c, 1); if (n.daycareOccupied > 1)return 0;
        if (n.daycareOccupied) {
            n.daycareLevel = PgameGet(&c, 1); n.daycareSteps = PgameGet(&c, 4); PgameReadMon(&c, r, &n.daycare, 1);
            if (!n.daycareLevel || n.daycareLevel > n.daycare.level)return 0;
        }
        for (unsigned h = 0; h < n.hallOfFameCount; ++h) {
            PbrbGameHallRecord* team = n.hallOfFame + h;
            team->count = (unsigned char)PgameGet(&c, 1); if (!team->count || team->count > 6)return 0;
            for (unsigned m = 0; m < team->count; ++m) {
                PbrbGameHallMon* mon = team->mons + m;
                mon->species = (unsigned char)PgameGet(&c, 1); mon->level = (unsigned char)PgameGet(&c, 1);
                for (unsigned i = 0; i < 11; ++i)mon->nickname[i] = (unsigned char)PgameGet(&c, 1);
                if (!PbrbGameValidSpecies(r, mon->species) || !mon->level || mon->level > 100 || !PgameValidName(mon->nickname))return 0;
            }
        }
    }
    else n.hallOfFameCount = 0;
    if (!c.ok || c.at != bytes || n.money > 999999 || n.coins > 9999 || n.pokedex > 1 || n.repelSteps > 250 || n.safariBalls > 30 || n.safariSteps > 500 || n.bicycle > 1 || n.surfing > 1 || (n.seen[18] & 128u) || (n.owned[18] & 128u))return 0;
    *s = n; return 1;
}
