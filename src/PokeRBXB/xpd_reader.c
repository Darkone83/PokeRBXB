/* xpd_reader.c : endian-safe, memory-bounded, ROM-view-capable XPD1 loader. */
#include "xpd_reader.h"
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#define XP_HEADER 96u
#define XP_ENTRY 28u
#define XP_LIMIT_ENTRIES 100000u
#define XP_LIMIT_NAMES (64u*1024u*1024u)
#define XP_ALIAS 1u
static uint16_t r16(const unsigned char* p) { return (uint16_t)(p[0] | ((uint16_t)p[1] << 8)); }
static uint32_t r32(const unsigned char* p) { return (uint32_t)p[0] | ((uint32_t)p[1] << 8) | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24); }
static uint32_t xpd_crc(const unsigned char* p, uint32_t n) {
    uint32_t c = 0xffffffffu, i; int k;
    for (i = 0; i < n; i++) { c ^= p[i]; for (k = 0; k < 8; k++) c = (c >> 1) ^ (0xedb88320u & (0u - (c & 1u))); }
    return c ^ 0xffffffffu;
}
void xpd_close(XpdArchive* a) {
    if (!a) return;
    if (a->stream) fclose(a->stream);
    free(a->names); free(a->entries);
    memset(a, 0, sizeof(*a));
}
int xpd_open(XpdArchive* a, const char* filename) {
    unsigned char hdr[XP_HEADER], row[XP_ENTRY];
    unsigned int i;
    uint32_t index_off, names_off;
    long file_len;
    if (!a || !filename) return 0;
    memset(a, 0, sizeof(*a));
    a->stream = fopen(filename, "rb");
    if (!a->stream) goto fail;
    if (fseek(a->stream, 0, SEEK_END) != 0) goto fail;
    file_len = ftell(a->stream);
    if (file_len < (long)XP_HEADER || (unsigned long)file_len>0xfffffffful) goto fail;
    a->file_size = (uint32_t)file_len;
    if (fseek(a->stream, 0, SEEK_SET) != 0 || fread(hdr, 1, XP_HEADER, a->stream) != XP_HEADER) goto fail;
    if (memcmp(hdr, "XPD1", 4) || r16(hdr + 4) != 1 || r16(hdr + 6) != XP_HEADER) goto fail;
    a->game = r32(hdr + 8);
    a->count = r32(hdr + 12);
    index_off = r32(hdr + 16);
    names_off = r32(hdr + 20);
    a->names_length = r32(hdr + 24);
    a->data_offset = r32(hdr + 28);
    if (a->game != XPD_GAME_RED && a->game != XPD_GAME_BLUE) goto fail;
    if (a->count == 0 || a->count > XP_LIMIT_ENTRIES || a->names_length > XP_LIMIT_NAMES || a->file_size > 0x7fffffffu) goto fail;
    if (index_off != XP_HEADER || names_off != index_off + a->count * XP_ENTRY) goto fail;
    if ((uint64_t)names_off + a->names_length > a->file_size ||
        a->data_offset < names_off + a->names_length || a->data_offset>a->file_size) goto fail;
    a->entries = (XpdEntry*)calloc(a->count, sizeof(XpdEntry));
    a->names = (char*)malloc((size_t)a->names_length + 1u);
    if (!a->entries || !a->names) goto fail;
    if (fseek(a->stream, (long)names_off, SEEK_SET) != 0 ||
        fread(a->names, 1, a->names_length, a->stream) != a->names_length) goto fail;
    a->names[a->names_length] = 0;
    if (fseek(a->stream, (long)index_off, SEEK_SET) != 0) goto fail;
    for (i = 0; i < a->count; i++) {
        XpdEntry* e = a->entries + i;
        if (fread(row, 1, XP_ENTRY, a->stream) != XP_ENTRY) goto fail;
        e->name_offset = r32(row + 0); e->type = r32(row + 4); e->flags = r32(row + 8);
        e->offset = r32(row + 12); e->packed_size = r32(row + 16); e->raw_size = r32(row + 20); e->crc32 = r32(row + 24);
        if (e->name_offset >= a->names_length ||
            memchr(a->names + e->name_offset, 0, a->names_length - e->name_offset) == NULL ||
            (e->flags & ~XP_ALIAS) ||
            e->packed_size != e->raw_size ||
            e->offset<a->data_offset ||
            (uint64_t)e->offset + e->packed_size>a->file_size) goto fail;
    }
    return 1;
fail:
    xpd_close(a); return 0;
}
static const XpdEntry* lookup(const XpdArchive* a, const char* name) {
    uint32_t i;
    if (!a || !a->stream || !name) return NULL;
    for (i = 0; i < a->count; i++)
        if (strcmp(a->names + a->entries[i].name_offset, name) == 0) return a->entries + i;
    return NULL;
}
int xpd_find(const XpdArchive* a, const char* name, XpdResource* info) {
    const XpdEntry* e = lookup(a, name);
    if (!e) return 0;
    if (info) { info->type = e->type; info->flags = e->flags; info->size = e->raw_size; info->crc32 = e->crc32; }
    return 1;
}
int xpd_read(XpdArchive* a, const char* name, void* dst, uint32_t cap, uint32_t* got) {
    const XpdEntry* e = lookup(a, name);
    if (got) *got = 0;
    if (!e || !dst || cap < e->raw_size) return 0;
    if (fseek(a->stream, (long)e->offset, SEEK_SET) != 0) return 0;
    if (fread(dst, 1, e->raw_size, a->stream) != e->raw_size) return 0;
    if (xpd_crc((const unsigned char*)dst, e->raw_size) != e->crc32) return 0;
    if (got) *got = e->raw_size;
    return 1;
}
int xpd_read_alloc(XpdArchive* a, const char* name, void** dst, uint32_t* got) {
    XpdResource info; void* buf;
    if (!dst) return 0;
    *dst = NULL;
    if (got) *got = 0;
    if (!xpd_find(a, name, &info)) return 0;
    buf = malloc(info.size ? info.size : 1);
    if (!buf) return 0;
    if (!xpd_read(a, name, buf, info.size, got)) { free(buf); return 0; }
    *dst = buf; return 1;
}
