/* xpd_reader.h - tiny ISO C compatible XPD1 reader for legacy RXDK projects.
   Runtime has no Python, Boost, SDL, zlib or stb dependency.
   Pair with xpd_reader.c; stdio wrappers can be swapped if preferred. */
#ifndef XPD_READER_H
#define XPD_READER_H
#include <stdio.h>
#include <stddef.h>
#if defined(_MSC_VER) && _MSC_VER < 1600
   /* Visual Studio versions predating stdint.h (older RXDK build hosts). */
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif
#ifdef __cplusplus
extern "C" {
#endif

#define XPD_GAME_RED 1u
#define XPD_GAME_BLUE 2u
#define XPD_TYPE(a,b,c,d) ((uint32_t)(uint8_t)(a) | ((uint32_t)(uint8_t)(b)<<8) | ((uint32_t)(uint8_t)(c)<<16) | ((uint32_t)(uint8_t)(d)<<24))

    typedef struct XpdEntry {
        uint32_t name_offset, type, flags, offset, packed_size, raw_size, crc32;
    } XpdEntry;
    typedef struct XpdArchive {
        FILE* stream;
        uint32_t file_size, game, count, names_length, data_offset;
        char* names;
        XpdEntry* entries;
    } XpdArchive;
    typedef struct XpdResource {
        uint32_t type, flags, size, crc32;
    } XpdResource;

    /* Return 1 on success, 0 on failure. Call xpd_close for any open archive.
       Initialize XpdArchive with {0} before first use. */
    int xpd_open(XpdArchive* a, const char* filename);
    void xpd_close(XpdArchive* a);
    int xpd_find(const XpdArchive* a, const char* name, XpdResource* info);
    int xpd_read(XpdArchive* a, const char* name, void* dst, uint32_t dst_capacity, uint32_t* got);
    /* Allocates through malloc; caller frees *dst with free() */
    int xpd_read_alloc(XpdArchive* a, const char* name, void** dst, uint32_t* got);
#ifdef __cplusplus
}
#endif
#endif
