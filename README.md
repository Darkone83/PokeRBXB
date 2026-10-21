# PokeRBXB

<div align=center>

<img src="https://github.com/Darkone83/PokeRBXB/blob/main/img/logo.png" width=300><img src="https://github.com/Darkone83/PokeRBXB/blob/main/img/Darkone83.png" width=400>

</div>

**Pokémon Red and Blue for the Original Xbox.**

PokeRBXB is a native Original Xbox implementation of Pokémon Red and Blue by [Darkone83](https://github.com/Darkone83). It brings both versions together in one launcher with Xbox controller support, native saves, a game soundtrack player, and shared display, audio, and controller settings.

The goal is to preserve the original games' gameplay, graphics, music, and presentation while making them comfortable to play on an Xbox.

<div align=center>

<img src="https://github.com/Darkone83/PokeRBXB/blob/main/img/launcher.png" width=375><img src="https://github.com/Darkone83/PokeRBXB/blob/main/img/game_red.jpg" width=375><img src="https://github.com/Darkone83/PokeRBXB/blob/main/img/game_blue.jpg" width=375>

</div>

## Features

- Pokémon Red and Blue in one application, with separate progress for each version.
- Native gameplay, battles, Pokémon catching, party management, PC storage, shops, and field moves.
- Original graphics, music, and sound effects read from your own game data.
- Native Xbox saves for game progress and shared launcher preferences.
- Saved-game details on the launcher, including trainer name, badges, and play time.
- Randomly selected game music in the launcher, with a **Now Playing** song title and a control to skip tracks.
- Sharp or smooth pixel filtering, original or GBC palette coloring, and screen size/position calibration.
- Separate launcher music, game music, and sound-effect controls, plus master volume.
- Adjustable controller rumble for battle hits, catches, tall grass, and selected field interactions.
- Controller-disconnect pause and a safe **Save and Return** option.
- Version-specific screen borders and automatic 480i, 480p, 720p, or PAL 576i output according to the console's video settings.

## Requirements

- A homebrew-capable Original Xbox and compatible controller.
- The PokeRBXB application files, including `default.xbe` and the supplied `data` folder.
- XPD Packer on your computer.
- Your own unmodified English **USA/Europe Pokémon Red and/or Pokémon Blue ROM** supported by the packer.

You only need the ROM for the version you want to play. ROM hacks, translations, Pokémon Yellow, and other Pokémon games are not supported.

## Installation

1. Copy the PokeRBXB application folder to your Xbox hard drive, keeping its existing folder structure.
2. Open **XPD Packer** on your computer.
3. Use **Browse…** to select your **Red ROM**, **Blue ROM**, or both, then select **Build Packs**.
4. Once the build completes, copy `red.xpd` and/or `blue.xpd` from the packer's `data` folder into PokeRBXB's `data` folder on the Xbox. Keep the supplied `img` and `font` folders in place.
5. Launch `default.xbe`, select the game you packed, and press **A**.

The files are arranged relative to the PokeRBXB installation folder:

| Location | Contents |
|---|---|
| `default.xbe` | Xbox application |
| `data/red.xpd` | Your packed Pokémon Red data, if installed |
| `data/blue.xpd` | Your packed Pokémon Blue data, if installed |
| `data/img/` | Supplied launcher artwork and borders |
| `data/font/` | Supplied launcher font |
| `data/xmv/intro.xmv` | Optional opening video, if installed |

The packer creates its output beside the packer application. See the README supplied with XPD Packer for its startup instructions and supported ROM details.

If a selected version's data is missing or invalid, PokeRBXB displays an error with the required filename. Copy the correct pack into the `data` folder beside `default.xbe`, then select **Retry**.
## Controls

### Launcher

| Control | Action |
|---|---|
| D-pad or left stick, left/right | Select Red or Blue |
| A | Start the selected game |
| X | Open launcher options |
| Y | Play another random launcher song |
| B | Exit to the Xbox dashboard |

The launcher selects game music at random and shows its name under **Now Playing**. The current song keeps playing while you browse the launcher. Use **Y** to change it without starting a game.

### Launcher options

| Control | Action |
|---|---|
| D-pad up/down | Choose an option or category |
| D-pad left/right or A | Change the selected value, where applicable |
| A | Open a settings category or controls guide |
| B | Return to the previous page |

From the main **Options** page, press **B** to save your shared preferences and return to game selection.

### In game

| Control | Action |
|---|---|
| D-pad (or left stick, when enabled) | Move and navigate menus |
| A | Interact, confirm, or advance dialogue |
| B | Cancel or go back |
| Start | Open the game's menu |
| Back | Game Boy SELECT; reorder items or battle moves where supported |
| Back + Start | Open the return-to-launcher prompt |

The **Back + Start** prompt provides **Save and Return**, **Return Without Saving**, and **Cancel**. **Save and Return** is available only when the game permits saving, such as during normal exploration. It is unavailable during battles and certain scripted events. If saving fails, the game stays open rather than exiting. Returning without saving discards progress made since your last successful save.

At the game's title screen, **B** returns directly to the launcher.

If the active controller disconnects during gameplay, the game and audio pause. Reconnect the controller, then press **A** to resume. Buttons held during reconnection are not passed into gameplay.

## Options

Press **X** at the launcher to open the shared settings. They apply to both Red and Blue and are kept separate from the original game's **OPTION** menu.

### Display

| Setting | Choices / behavior |
|---|---|
| Pixel Filtering | **Sharp** for crisp pixels, or **Smooth** for softened scaling |
| Color Graphics | **Original** or **GBC Palette** |
| Screen Calibration | Adjust screen size and horizontal/vertical position; reset to the default alignment |

Screen calibration helps compensate for displays that crop the edges. GBC Palette changes the coloring of the original graphics, not the game content.

### Audio

| Setting | Choices / behavior |
|---|---|
| Launcher Music | **On** or **Off** |
| Game Music | **On** or **Off** |
| Sound Effects | **On** or **Off** |
| Master Volume | Adjust overall audio level |

Turning off game music or sound effects does not intentionally shorten gameplay animation or event timing.

### Controller

| Setting | Choices / behavior |
|---|---|
| Controller Rumble | **Off**, **Low**, or **Normal** |
| Movement Input | **D-pad Only** or **D-pad + Stick** |

The **Controls** page in the launcher shows the Xbox button layout, including Game Boy SELECT and the return-to-launcher shortcut.

The game's own **OPTION** menu still provides **text speed**, **battle animation**, and **battle style** settings. These original gameplay choices remain in-game rather than being moved into the launcher.

## Save Data

Use **Start → SAVE** during normal gameplay, or choose **Save and Return** from the **Back + Start** prompt when saving is available. Select **CONTINUE** from that version's main menu to resume a saved game.

- Red and Blue keep separate game progress.
- Game saves and launcher preferences are stored in the native **PokeRBXB** Xbox save container.
- The launcher displays the saved trainer name, badge count, and play time for each available game; a version without a save shows **New Adventure**.
- The generated `.xpd` files contain game data; they are not your save files.
- Cartridge and emulator `.sav` import/export is not currently supported.

Back up the PokeRBXB save using your Xbox dashboard's save manager before removing it. Deleting the entire PokeRBXB save container removes both versions' progress and the launcher preferences.

## Current Limitations

- **Cable Club is disabled for now.** Player-to-player trades, link battles, and trade evolutions through a link session are unavailable. In-game NPC trades remain available.
- Some animation and frame-timing differences remain, and random outcomes do not reproduce the original hardware's exact sequence.
- Compatibility and full-game testing are ongoing; complete 1:1 parity is still a goal.

When reporting an issue in this repository, include the game version, build used, location or battle, steps to reproduce it, and your Xbox/video setup. A screenshot or short recording is helpful for graphics, audio, or timing problems.

## Credits & Attribution

### PokeRBXB

Original Xbox implementation and XPD Packer by **[Darkone83](https://github.com/Darkone83)**.

### pokered

**[pokered](https://github.com/pret/pokered) — pret and contributors**

The Pokémon Red and Blue disassembly is the primary reference for the original game behavior, scripts, maps, graphics, and audio. Its documented source and symbols make faithful native implementation and comparison possible.

### CppRed

**[CppRed](https://github.com/Helios-vmg/cppred) — Helios-vmg and contributors**

CppRed is an earlier C++ implementation of Pokémon Red and Blue. It provided an additional implementation and audio reference for this project.

### Supporting Projects

- **[stb_image](https://github.com/nothings/stb) — Sean Barrett and contributors:** image loading for the Xbox launcher and borders.
- **[RXDK](https://github.com/Team-Resurgent/RXDK) — Team Resurgent and contributors:** Original Xbox development tools.
- Thanks to the Xbox homebrew community and everyone testing and reporting issues.

### Original Games

Pokémon Red and Blue were developed by **GAME FREAK** and published by **Nintendo**. Pokémon, its characters, and the original game content remain the property of their respective rights holders.

PokeRBXB is an unofficial homebrew project and is not affiliated with or endorsed by Nintendo, GAME FREAK, Creatures, The Pokémon Company, or Microsoft. Original ROMs are not included. Generate your own game-data packs from supported ROMs.
