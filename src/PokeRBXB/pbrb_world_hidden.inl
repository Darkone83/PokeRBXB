/* Original hidden-item tables are read directly from the attached ROM. */
static const unsigned WORLD_HIDDEN_COORDS[2] = { 0x766b8u,0x766b9u };
static const unsigned WORLD_HIDDEN_EVENT_MAPS[2] = { 0x46a40u,0x46a40u };
static const unsigned WORLD_HIDDEN_EVENT_POINTERS[2] = { 0x46a96u,0x46a96u };
static const unsigned WORLD_HIDDEN_ITEM_FUNCTION[2] = { 0x76688u,0x76689u };
static unsigned WorldHiddenItemId(const PbrbWorld* w, int x, int y) {
    unsigned maps = WORLD_HIDDEN_EVENT_MAPS[GameIndex(w)], ptrs = WORLD_HIDDEN_EVENT_POINTERS[GameIndex(w)];
    for (unsigned i = 0; i < 128 && InRom(w, maps + i, 1); ++i) {
        unsigned raw = w->rom[maps + i]; if (raw == 255)return 0; if (raw != (unsigned)w->romMapId)continue;
        if (!InRom(w, ptrs + i * 2, 2))return 0;
        unsigned p = Address(ptrs / 0x4000u, Read16(w->rom + ptrs + i * 2));
        for (unsigned e = 0; e < 128 && InRom(w, p, 6); ++e, p += 6) {
            if (w->rom[p] == 255)return 0;
            if (w->rom[p] == y && w->rom[p + 1] == x &&
                Address(w->rom[p + 3], Read16(w->rom + p + 4)) == WORLD_HIDDEN_ITEM_FUNCTION[GameIndex(w)])return w->rom[p + 2];
        }
        return 0;
    }
    return 0;
}
static int WorldHiddenItemInteraction(PbrbWorld* w, int x, int y) {
    unsigned p = WORLD_HIDDEN_COORDS[GameIndex(w)];
    for (unsigned i = 0; i < 64 && InRom(w, p, 3); ++i, p += 3) {
        if (w->rom[p] == 255)return 0;
        if (w->rom[p] != w->romMapId || w->rom[p + 1] != y || w->rom[p + 2] != x)continue;
        if (w->hiddenItems[i / 8] & (1u << (i & 7)))return 1;
        unsigned item = WorldHiddenItemId(w, x, y); if (!item)return 0;
        NativeStart(w, WG_HIDDEN_ITEM, (int)i); w->scriptValue = (int)item; return 1;
    }
    return 0;
}
static int WorldHiddenItemNear(const PbrbWorld* w) {
    unsigned p = WORLD_HIDDEN_COORDS[GameIndex(w)];
    int minY = w->heroY >= 5 ? w->heroY - 5 : 0, minX = w->heroX >= 5 ? w->heroX - 5 : 0;
    for (unsigned i = 0; i < 64 && InRom(w, p, 3); ++i, p += 3) {
        if (w->rom[p] == 255)return 0;
        if (w->rom[p] != w->romMapId || (w->hiddenItems[i / 8] & (1u << (i & 7))))continue;
        int y = w->rom[p + 1], x = w->rom[p + 2];
        /* Original strict lower bounds and asymmetric +4/+5 upper bounds. */
        if (y > minY && y <= w->heroY + 4 && x > minX && x <= w->heroX + 5)return 1;
    }
    return 0;
}
static int WorldHiddenItemScript(PbrbWorld* w) {
    int s = w->scriptStep;
    if (w->scriptId == WG_HIDDEN_ITEM) {
        if (s == 0)NativeItem(w, (unsigned)w->scriptValue, 1, 1);
        else if (s == 1) {
            if (w->scriptResult) {
                w->hiddenItems[w->scriptObject / 8] |= (unsigned char)(1u << (w->scriptObject & 7));
                NativeText(w, WX_FoundHiddenItemText, 2, PBRB_WORLD_AUTO_CLOSE | PBRB_WORLD_KEEP_TEXT);
            }
            else NativeText(w, WX_FoundHiddenItemText, 4, PBRB_WORLD_KEEP_TEXT);
            w->event.itemId = (unsigned)w->scriptValue;
        }
        else if (s == 2)NativeSound(w, PBRB_SFX_GET_ITEM2, 3);
        else if (s == 4)NativeText(w, WX_HiddenItemBagFullText, 3, PBRB_WORLD_CONTINUE_TEXT);
        else NativeEnd(w);
        return 1;
    }
    if (w->scriptId == WG_ITEMFINDER) {
        if (!w->scriptValue && s == 0)NativeText(w, WX_ItemfinderFoundNothingText, 9);
        else if (s < 8)NativeSound(w, (s & 1) ? PBRB_SFX_PURCHASE : PBRB_SFX_HEALING_MACHINE, s + 1);
        else if (s == 8)NativeText(w, WX_ItemfinderFoundItemText, 9);
        else NativeEnd(w);
        return 1;
    }
    return 0;
}
