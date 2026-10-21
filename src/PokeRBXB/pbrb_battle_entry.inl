/* Original Red/Blue send-out graphics. Included by pbrb_play.inl, so the
   existing RXDK source list does not need another translation unit.
   Animation tables and 2bpp graphics are read from the attached ROM. */
#ifndef PBRB_BATTLE_ENTRY_INL_INCLUDED
#define PBRB_BATTLE_ENTRY_INL_INCLUDED

static int PbrbBattleEntryFits(const PbrbGb* g, unsigned at, unsigned bytes) {
    return g && g->rom && (size_t)at <= g->bytes &&
        (size_t)bytes <= g->bytes - (size_t)at;
}

static void PbrbBattleEntryPoof(PbrbGb* g, unsigned elapsedFrame) {
    /* POOF_ANIM uses Subanim_0BallPoofEnemy, including its HFLIP transform
       for the player's side. Each block lasts four frames plus the
       original AnimationCleanOAM wait: six blocks, thirty frames. */
    if (elapsedFrame >= 30u || !PbrbBattleEntryFits(g, 0x7a89au, 19u))return;
    const unsigned entry = 0x7a89bu + (elapsedFrame / 5u) * 3u;
    const unsigned block = g->rom[entry];
    const unsigned coord = g->rom[entry + 1u];
    const unsigned pointer = 0x7af74u + block * 2u;
    const unsigned coordAt = 0x7bc85u + coord * 2u;
    if (!PbrbBattleEntryFits(g, pointer, 2u) ||
        !PbrbBattleEntryFits(g, coordAt, 2u))return;
    const unsigned address = g->rom[pointer] | ((unsigned)g->rom[pointer + 1u] << 8);
    if (address < 0x4000u || address >= 0x8000u)return;
    const unsigned data = 0x74000u + address;
    if (!PbrbBattleEntryFits(g, data, 1u))return;
    const unsigned count = g->rom[data];
    if (count > 40u || !PbrbBattleEntryFits(g, data + 1u, count * 4u))return;
    PbrbGbOam oam[40];
    const unsigned baseY = g->rom[coordAt], baseX = g->rom[coordAt + 1u];
    for (unsigned i = 0; i < count; ++i) {
        const unsigned char* sprite = g->rom + data + 1u + i * 4u;
        oam[i].y = (unsigned char)(baseY + sprite[0] + 40u);
        oam[i].x = (unsigned char)(168u - baseX - sprite[1]);
        oam[i].attributes = (unsigned char)(sprite[3] ^ 0x20u);
        oam[i].offset = 0x781feu + (unsigned)sprite[2] * 16u;
        if (!PbrbBattleEntryFits(g, oam[i].offset, 16u))return;
    }
    PbrbGbDrawOam(g, oam, count, 0xe4u, 0x6cu);
}

static void PbrbBattleEntryBall(PbrbGb* g, const unsigned char* backPicture,
    int x, int y) {
    /* AnimateSendingOutMon writes BG tile $4E. It aliases the currently
       loaded back picture's tile 29 ($4E-$31), in column-major order.
       The enemy's entrance still has Red's back picture loaded; the
       player's entrance has its own back picture, even if this tile is
       blank. Replacing this alias with a generic ball changes the ROM. */
    if (!g || !backPicture)return;
    for (int yy = 0; yy < 8; ++yy)for (int xx = 0; xx < 8; ++xx)
        PbrbGbPixel(g, x + xx, y + yy, backPicture[(8 + yy) * 56 + 32 + xx], 0);
}

#endif /* PBRB_BATTLE_ENTRY_INL_INCLUDED */
