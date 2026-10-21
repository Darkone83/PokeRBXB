#ifndef PBRB_PACKAGES_H_INCLUDED
#define PBRB_PACKAGES_H_INCLUDED

enum PbrbPackStatus { PBRB_MISSING = 0, PBRB_INVALID = 1, PBRB_READY = 2 };
PbrbPackStatus PbrbInspectPack(const char* path, unsigned expectedGame);

/* Xbox preferences live with both versions' progress in one native save
   container. They never enter the original game's simulation or RNG. */
struct PbrbPreferences { int smooth, rumble, color; };
const PbrbPreferences* PbrbGetPreferences();
int PbrbLoadPreferences();
/* Applies the preference for this session even if persistent storage fails.
   Return zero means the caller must report the save failure to the player. */
int PbrbSavePreferences(const PbrbPreferences* preferences);

/* The game owns its explicitly serialized payload. The storage layer checks
   its envelope/checksum and keeps the preceding valid snapshot intact.
   game is XPD_GAME_RED (1) or XPD_GAME_BLUE (2), never a shared save slot. */
#define PBRB_SAVE_MAX_BYTES 65536u
int PbrbSaveGame(unsigned game, const void* data, unsigned bytes);
int PbrbLoadGame(unsigned game, void* data, unsigned capacity, unsigned* bytes);
int PbrbHasSave(unsigned game);
const char* PbrbSaveError();

#endif
