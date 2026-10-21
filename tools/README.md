# XPD Packer

Create the Pokémon Red and Blue game-data files used by **PokeRBXB** on the Original Xbox.

Choose Red, Blue, or both, then select **Build Packs**. The packer checks your ROMs, builds the files, and verifies them automatically.

## What You Need

- Your own unmodified English **USA/Europe Pokémon Red and/or Pokémon Blue ROM**, extracted as an individual file. ROM hacks, translations, ZIP archives, and other Pokémon games are not supported.
- The packer files together in a writable folder: `XPD_pack.py`, `xpd_gui.py`, and `xpd.py`.
- **Python 3.10 or newer** to run the Python version.

The Python launcher installs the required PySide6 interface package automatically if it is missing. That first-time installation needs an internet connection.

## Open the Packer

Run **`XPD_pack.py`**. If double-clicking it does not open the application, open a terminal in the packer folder and run:

```sh
python XPD_pack.py
```

If you are using a packaged application instead of the Python files, open that application directly.

## Build Your Packs

1. Click **Browse…** beside **Red ROM** or **Blue ROM** and select the matching game. Fill both slots if you want both versions.
2. Leave the unused slot empty. Clear any remembered path you do not want to use.
3. Click **Build Packs** and wait for the progress bar to reach **100%** and the completion message to appear.

The output is saved in a `data` folder beside the packer:

| Selected ROM | Output |
|---|---|
| Pokémon Red | `data/red.xpd` |
| Pokémon Blue | `data/blue.xpd` |

Building again replaces the selected version's existing pack after verification. A pack for a version you did not select is left in place. Your original ROM files are not changed.

## Copy to Your Xbox

Copy `red.xpd` and/or `blue.xpd` into PokeRBXB's **`data` folder beside `default.xbe`**. Keep the existing `img` and `font` folders. Launch PokeRBXB, select the version you packed, and press **A**.

The packer creates game-data packs. Use them with the PokeRBXB application and its supplied artwork and font files.

## Troubleshooting

| Problem | What to check |
|---|---|
| **Build Packs** is unavailable | Select at least one ROM. |
| The ROM is rejected | Check the version, region, and matching slot. Use an unmodified supported ROM. |
| A ROM cannot be read | Browse to the file again, or clear an unused remembered path. |
| The packer cannot save its output | Move the packer to a writable folder and check available space. |
| The Xbox reports missing or invalid game data | Copy the completed `.xpd` file into the `data` folder beside `default.xbe`, keep its original filename, and select **Retry**. |

## Credits

- **[Darkone83](https://github.com/Darkone83):** XPD Packer and PokeRBXB.
- **[pokered](https://github.com/pret/pokered), by pret and contributors:** the Pokémon Red/Blue disassembly used as the main original-game reference for PokeRBXB.
- **[CppRed](https://github.com/Helios-vmg/cppred), by Helios-vmg and contributors:** an additional C++ implementation and audio reference for PokeRBXB.

Pokémon and the original game content belong to their respective rights holders. The packer is an unofficial tool and does not include ROMs. Supply your own supported game files.
