/* Native field presentation from pokered engine/overworld/cut{,2}.asm and
   player_animations.asm. These are addresses in the verified Red/Blue ROMs,
   not copied graphics. The relevant labels have identical layouts in both.
   Timings below are the original explicit DelayFrames/Delay3 intervals. */
static const unsigned FIELD_BIRD = 0x14d80u, FIELD_RED = 0x14180u;
static const unsigned FIELD_FISHING_TILES = 0x70866u, FIELD_FISHING_ROD_OAM = 0x70856u;
static const unsigned FIELD_FLY_OUT1 = 0x7064fu, FIELD_FLY_OUT2 = 0x70667u, FIELD_FLY_IN = 0x70592u;
static const unsigned FIELD_TREE = 0x64000u, FIELD_LEAF = 0x7874eu, FIELD_FADE_WHITE = 0x2119u;
enum {
    FIELD_FLY_LEAVE = 157, FIELD_FLY_ENTER = 184, FIELD_FLY_END = 220,
    FIELD_ESCAPE_LEAVE = 166, FIELD_ESCAPE_ENTER = 193, FIELD_ESCAPE_LAND = 205, FIELD_ESCAPE_END = 233
};
static void FieldAnimationStart(PbrbWorld* w, int kind) {
    PbrbWorldFieldAnimation* a = &w->fieldAnimation;
    memset(a, 0, sizeof(*a)); a->kind = kind; a->facing = w->facing; a->travel = w->travelMode;
    if (kind == WFA_FLY) {
        unsigned raw = (unsigned)w->battleReturn, p = 0x647cu + raw * 6u;
        a->targetMap = PbrbWorldMapFromRom(raw); a->targetX = w->rom[p + 3]; a->targetY = w->rom[p + 2];
    }
    else if (kind == WFA_ESCAPE) {
        a->targetMap = w->respawnMap; a->targetX = w->respawnX; a->targetY = w->respawnY;
        Emit(w, PBRB_WORLD_SFX, 0, PBRB_AUDIO_RAW(2, 0xa1)); /* TELEPORT_EXIT_2 */
    }
    if (kind == WFA_FLY || kind == WFA_ESCAPE) {
        w->travelMode = 0; w->fieldActive = 0; w->serviceFlags &= ~64u;
    }
}
static int FieldAnimationTick(PbrbWorld* w) {
    PbrbWorldFieldAnimation* a = &w->fieldAnimation;
    if (!a->kind)return 0;
    int end = a->kind == WFA_CUT_TREE ? 8 : a->kind == WFA_CUT_GRASS ? 32 :
        a->kind == WFA_FISHING ? (w->scriptObject > 0 ? 281 : 190) :
        a->kind == WFA_FLY ? FIELD_FLY_END : FIELD_ESCAPE_END;
    /* Keep the fishing pose/rod while its result text is being read. */
    if (a->kind == WFA_FISHING && a->frame >= end)return 0;
    ++a->frame;
    if (a->frame >= end) { if (a->kind != WFA_FISHING)a->kind = WFA_NONE; return 0; }
    int sound = -1, warp = 0;
    if (a->kind == WFA_FLY) {
        if (a->frame == 24 || a->frame == FIELD_FLY_ENTER)sound = PBRB_SFX_FLY;
        warp = a->frame == FIELD_FLY_LEAVE;
    }
    else if (a->kind == WFA_ESCAPE) {
        if (a->frame == 54 || a->frame == 92 || a->frame == 114)sound = PBRB_AUDIO_RAW(2, 0xa1);
        if (a->frame == 120)sound = PBRB_AUDIO_RAW(2, 0x9f); /* TELEPORT_EXIT_1 */
        if (a->frame == FIELD_ESCAPE_ENTER)sound = PBRB_AUDIO_RAW(2, 0xa0); /* ENTER_1 */
        if (a->frame == FIELD_ESCAPE_LAND)sound = PBRB_AUDIO_RAW(2, 0xa3); /* ENTER_2 */
        warp = a->frame == FIELD_ESCAPE_LEAVE;
    }
    if (warp) {
        w->lastMapRom = PbrbWorldMapToRom(a->targetMap); w->encounterCooldown = 3;
        if (!LoadMap(w, a->targetMap, a->targetX, a->targetY, 0)) {
            a->kind = WFA_NONE; NativeEnd(w); Emit(w, PBRB_WORLD_PLAYTEST_END, 0, -1); return 1;
        }
    }
    if (sound >= 0)Emit(w, PBRB_WORLD_SFX, 0, sound);
    return 1;
}
static void FieldAnimationPalettes(const PbrbWorld* w, unsigned* bg, unsigned* obj0, unsigned* obj1) {
    const PbrbWorldFieldAnimation* a = &w->fieldAnimation;
    if (a->kind == WFA_CUT_TREE || a->kind == WFA_CUT_GRASS) {
        *obj1 = ((a->frame + 1) & 1) ? 0x80u : 0xe4u; return;
    }
    if (a->kind != WFA_FLY && a->kind != WFA_ESCAPE)return;
    int leave = a->kind == WFA_FLY ? FIELD_FLY_LEAVE : FIELD_ESCAPE_LEAVE;
    int level = 0;
    if (a->frame >= leave - 24 && a->frame < leave)level = 1 + (a->frame - (leave - 24)) / 8;
    else if (a->frame >= leave && a->frame < leave + 3)level = 3;
    else if (a->frame >= leave + 3 && a->frame < leave + 27)level = 2 - (a->frame - (leave + 3)) / 8;
    unsigned p = FIELD_FADE_WHITE + (unsigned)level * 3u;
    if (InRom(w, p, 3)) { *bg = w->rom[p]; *obj0 = w->rom[p + 1]; *obj1 = w->rom[p + 2]; }
}
static void FieldOamTile(PbrbGbOam* out, unsigned* count, unsigned gfx, int x, int y, unsigned attributes) {
    if (*count >= 40 || x <= -8 || x >= 160 || y <= -8 || y >= 144)return;
    PbrbGbOam* o = out + (*count)++; o->offset = gfx; o->x = (unsigned char)(x + 8);
    o->y = (unsigned char)(y + 16); o->attributes = (unsigned char)attributes;
}
static int FieldSpinFacing(int facing, int steps) {
    static const int order[4] = { 0,2,1,3 }; int start = 0;
    while (start < 3 && order[start] != facing)++start;
    return order[(start + steps) & 3];
}
static void FieldBird(const PbrbWorld* w, PbrbGbOam* out, unsigned* count,
    int frame, unsigned coords, int length, int x, int y, int right) {
    int step = frame / 3;
    /* DoFlyAnimation toggles the image, waits three frames, then copies the
       next ROM coordinate pair. The first interval stays at the old point. */
    if (coords && step > 0) {
        int index = step - 1; if (index >= length)index = length - 1;
        unsigned p = coords + (unsigned)index * 2u;
        if (InRom(w, p, 2)) { y = (int)(signed char)w->rom[p]; x = w->rom[p + 1]; }
    }
    unsigned gfx = FIELD_BIRD + 128u + ((step & 1) ? 0u : 192u);
    if (InRom(w, gfx, 64))AddOam(out, count, gfx, x, y, right, 0);
}
static int FieldAnimationPlayer(const PbrbWorld* w, PbrbGbOam* out, unsigned* count) {
    const PbrbWorldFieldAnimation* a = &w->fieldAnimation; int t = a->frame;
    if (a->kind == WFA_FISHING && t >= 90) {
        int shake = w->scriptObject > 0 && t >= 190 && t < 220 ? 1 - ((t - 190) / 3 & 1) : 0;
        if (w->scriptObject > 0 && t >= 220 && t < 281)AddOam(out, count, WORLD_SHOCK[GameIndex(w)], 64, 44, 0, 0);
        int dir = a->facing < 2 ? a->facing : 2, flip = a->facing == 3;
        unsigned p = FIELD_FISHING_TILES + (unsigned)dir * 6u;
        unsigned lower = InRom(w, p, 6) ? Address(w->rom[p + 3], Read16(w->rom + p)) : 0;
        unsigned upper = FIELD_RED + (unsigned)dir * 64u;
        if (InRom(w, lower, 32) && InRom(w, upper, 32))for (int x = 0; x < 2; ++x) {
            int px = 64 + (flip ? 1 - x : x) * 8;
            FieldOamTile(out, count, upper + (unsigned)x * 16u, px, 60 + shake, flip ? 0x20u : 0u);
            FieldOamTile(out, count, lower + (unsigned)x * 16u, px, 68 + shake, flip ? 0x20u : 0u);
        }
        return 1;
    }
    if (a->kind == WFA_FLY) {
        if (t < 24)FieldBird(w, out, count, t, 0, 0, 64, 60, 1);
        else if (t < 100)FieldBird(w, out, count, t < 60 ? t - 24 : 36, FIELD_FLY_OUT1, 12, 64, 60, 1);
        else if (t < FIELD_FLY_LEAVE)FieldBird(w, out, count, t < 133 ? t - 100 : 33, FIELD_FLY_OUT2, 11, 160, 39, 0);
        else if (t >= FIELD_FLY_ENTER)FieldBird(w, out, count, t - FIELD_FLY_ENTER, FIELD_FLY_IN, 12, 64, -20, 0);
        return 1;
    }
    if (a->kind == WFA_ESCAPE) {
        int y = 60, turns = 0, facing = a->facing;
        if (t < 120) {
            int at = 0; for (int delay = 15; delay > 0 && t >= at + delay; --delay) { at += delay; ++turns; }
        }
        else if (t < 132) { turns = 16 + (t - 120) / 3; y = 44 - 16 * ((t - 120) / 3); }
        else if (t < FIELD_ESCAPE_ENTER) { return 1; }
        else if (t < FIELD_ESCAPE_LAND) { facing = 0; turns = (t - FIELD_ESCAPE_ENTER) / 3; y = -4 + 16 * turns; }
        else {
            facing = 0; turns = 5; int at = 0, elapsed = t - FIELD_ESCAPE_LAND;
            for (int delay = 1; delay < 8 && elapsed >= at + delay; ++delay) { at += delay; ++turns; }
        }
        SpriteOam(w, out, count, 1, 64, y, FieldSpinFacing(facing, turns), 0, 0); return 1;
    }
    return 0;
}
static void FieldAnimationOverlay(const PbrbWorld* w, PbrbGbOam* out, unsigned* count) {
    const PbrbWorldFieldAnimation* a = &w->fieldAnimation;
    if (a->kind == WFA_FISHING && a->frame >= 90) {
        if (w->scriptObject > 0 && a->frame >= 220 && a->frame < 281 && a->facing == 1)return;
        unsigned p = FIELD_FISHING_ROD_OAM + (unsigned)a->facing * 4u;
        unsigned descriptor = FIELD_FISHING_TILES + 18u;
        if (!InRom(w, p, 4) || !InRom(w, descriptor, 6))return;
        unsigned gfx = Address(w->rom[descriptor + 3], Read16(w->rom + descriptor));
        gfx += (unsigned)(w->rom[p + 2] - 0xfdu) * 16u;
        int shake = w->scriptObject > 0 && a->frame >= 190 && a->frame < 220 ? 1 - ((a->frame - 190) / 3 & 1) : 0;
        if (*count > 39)*count = 39;
        if (InRom(w, gfx, 16))FieldOamTile(out, count, gfx, (int)w->rom[p + 1] - 8, ((int)w->rom[p] ^ shake) - 16, w->rom[p + 3]);
        return;
    }
    if (a->kind != WFA_CUT_TREE && a->kind != WFA_CUT_GRASS)return;
    static const int xOffset[4] = { 0,0,-16,16 }, yOffset[4] = { 20,-12,4,4 };
    int x[4], y[4]; unsigned attr[4], gfx[4];
    for (int i = 0; i < 4; ++i) {
        x[i] = 64 + xOffset[a->facing] + (i & 1) * 8; y[i] = 60 + yOffset[a->facing] + (i / 2) * 8;
        attr[i] = a->kind == WFA_CUT_GRASS ? (i & 1 ? 0x50u : 0x30u) : 0x10u;
        gfx[i] = a->kind == WFA_CUT_GRASS ? FIELD_LEAF : FIELD_TREE + (unsigned)((i < 2 ? 0x2d : 0x3d) + (i & 1)) * 16u;
    }
    int frames = a->frame + 1;
    for (int f = 0; f < frames; ++f) {
        if (a->kind == WFA_CUT_TREE) { x[0]++; x[1]++; x[2]--; x[3]--; }
        else {
            x[0]++; x[1] += 2; x[2] -= 2; x[3]--;
            if ((f + 1) % 8 == 0)for (int i = 0; i < 2; ++i) {
                int v = x[i]; x[i] = x[i + 2]; x[i + 2] = v; v = y[i]; y[i] = y[i + 2]; y[i + 2] = v;
                unsigned u = attr[i]; attr[i] = attr[i + 2]; attr[i + 2] = u;
            }
            if ((f + 1) % 16 == 0 && f + 1 < frames)for (int i = 0; i < 4; ++i)y[i] += 2;
        }
    }
    if (*count > 36)*count = 36;
    for (int i = 0; i < 4; ++i)if (InRom(w, gfx[i], 16))FieldOamTile(out, count, gfx[i], x[i], y[i], attr[i]);
}
