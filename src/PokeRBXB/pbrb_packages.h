#ifndef PBRB_PACKAGES_H_INCLUDED
#define PBRB_PACKAGES_H_INCLUDED

enum PbrbPackStatus { PBRB_MISSING = 0, PBRB_INVALID = 1, PBRB_READY = 2 };
PbrbPackStatus PbrbInspectPack(const char* path, unsigned expectedGame);

/* Xbox preferences live with both versions' progress in one native save
   container. They never enter the original game's simulation or RNG. */
struct PbrbPreferences {
    int smooth, rumble, color; /* rumble: 0 Off, 1 Low, 2 Normal */
    int launcherMusic, music, effects, volume; /* volume: 0..10 */
    int dpadOnly, screenScale, screenX, screenY; /* scale: 80..100; offsets: -5..5 */
};
void PbrbDefaultPreferences(PbrbPreferences* preferences);
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
/* Read-only launcher preview; no ROM, live game or disk write is needed.
   1 = supported saved summary, 0 = no save, -1 = unavailable/invalid.
   The game still performs its full semantic validation on CONTINUE. */
struct PbrbSaveSummary { char trainer[16]; unsigned badges, hours, minutes; };
int PbrbReadSaveSummary(unsigned game, PbrbSaveSummary* summary);
const char* PbrbSaveError();

#endif
