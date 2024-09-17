# Miscellanous

This page describes every change in Vinifera that wasn't categorized into a proper category yet.

- Vinifera hardcodes the shroud and fog graphics to circumvent cheating in multiplayer games.
- Vinifera redirects saved screenshots using the keyboard command to a new sub-directory in the games folders, `Screenshots`.

## MCV Auto-deploy

- Vinifera allow you to start the game with the MCV deployed, or have the MCV auto-deploy on start.

In `RULES.INI`:
```ini
[MultiplayerDefaults]
AutoDeployMCV=<boolean>  ; Player MCV's will auto-deploy on game start. (Defaults to no).
                         ; NOTE: This option only has an effect if the unit count is set to 1.
PrePlacedConYards=<boolean> ; Pre-place construction yards instead of spawning an MCV. (Defaults to no).
                            ; NOTE: This option has priority over AutoDeployMCV.
```

## Multi-Engineer

- Vinifera fixes `EngineerDamage` and `EngineerCaptureLevel` to be considered by the game, like they were in Tiberian Dawn and Red Alert.

In `RULES.INI`:
```ini
[General]
EngineerDamage=<float>  ; The engineer will damage a building by this percent of its full health each time it enters. Defaults to 0.0.
EngineerCaptureLevel=<float>  ; If the building’s health is equal to or below this percentage of its strength it can be captured by an engineer. Defaults to 1.0.
```
```{warning}
Upon observing the values used in `FIRESTRM.INI`, this could potentially cause an issue with the vanilla game. `FIRESTRM.INI` has the values `EngineerCaptureLevel=1.0` and `EngineerDamage=0.0`. Below are some values to help test these bug fixes and their potential impact on the vanilla game.

Red Alert Default values:
`EngineerDamage=0.33`
`EngineerCaptureLevel=0.25`

Red Alert Multiplayer (MPLAYER.INI) values:
`EngineerDamage=0.33`
`EngineerCaptureLevel=0.66`
```

## Build off Ally

- Vinifera implements the "Build Off Ally" feature from Red Alert 2 as a Quality of Life improvement. This is now the default behaviour for multiplayer games.

In `RULES.INI`:
```ini
[MultiplayerDefaults]
BuildOffAlly=<boolean>  ; Can players build their own structures adjacent to structures owned by their allies? (Defaults to yes).

[BuildingTypes]
EligibleForAllyBuilding=<boolean>  ; Is this building eligible for proximity checks by players who are its owner's allies?
                                   ; For buildings with `ConstructionYard=yes` this defaults to yes, otherwise it defaults to no.
```

## Window title, Cursor and Icon

- The game's Window title, Cursor and Icon can be overridden. These controls are loaded from a new INI file, `VINIFERA.INI`.
```{note}
While this is INI file is optional, it is recommended the `ProjectName` and `ProjectVersion` are set by the mod developer as this information is printed into debug logs and crash dumps to aid in the troubleshooting process.
```

In `VINIFERA.INI`:
```ini
[General]
ProjectName=<string>  ; The project's title name string. Limited to 64 characters.
ProjectVersion=<string>  ; The project's version string. Limited to 64 characters.
IconFile=<string>  ; The name of the icon file (including the .ICO extension) to use for the games window. Limited to 64 characters.
CursorFile=<string>  ; The name of the cursor file (including the .CUR extension) to use for the game's cursor. Limited to 64 characters.
```
```{note}
The filenames also support subdirectories.
```

## File System

- `GENERIC.MIX` and `ISOGEN.MIX` mixfiles can now be used to place common assets between theaters.
- The game now loads `ELOCAL(00-99).MIX` expansion mixfiles. These can be used to override files normally found in `LOCAL.MIX`.

- Vinifera reimplements the file search path override logic of `-CD` from Red Alert. This effectively allows the end-user to copy the CD contents to the game directory and run the game without any CD required to be inserted.
The argument supports multiple entries separated by the `;` character. Below are some examples:
`-CD.` - Sets the games root directory as the location to search for the CD contents.
`-CDcd_path` - Sets the `cd_path` sub-directory as the location to search for the CD contents.
`-CDcd1;cd2;cd3` - Sets the sub-directories `cd1`, `cd2`, and `cd3` as the search locations for the CD contents.

## Developer Features

- The game will no longer fail to start if the startup mix files are not found.
```{note}
It is not recommended that you modify or remove any of the original mix files, this could lead to undefined behaviour and the Vinifera developers will not be able to troubleshoot any issue related to these changes.
```

### Rule Selection

- Vinifera re-enables a feature from Tiberian Sun's development which allows you to select the Rules file if multiple are found in the game directory.

- At startup, the game will scan for files in the root directory with the format **`RULE*.INI`**, with `*` being a wildcard for any character or string (for example, `RULE_VERSION1.INI` or `RULE_TEST_WEAPON.INI`). While it has no impact on the loading, it is advised to use the format `RULE_*.INI` to help with file sorting in Windows Explorer.

- The name shown in the dialog is the value of the `Name=` _(with a limit of 128 characters)_ entry under `[General]` in the Rule INI. If the user presses Cancel on the dialog, the game will load the standard `RULES.INI` file.

```{note}
Due to the nature of its use, this feature is only available when Vinifera is running in Developer Mode
```

![image](https://user-images.githubusercontent.com/73803386/137135038-0a1e983f-d295-4723-86fb-1ab94ba8948b.png)

### NULL house warning

- Vinifera adds a warning to the debug log output when a null house pointer is detected during the game loading screen.
- Also, if the user has Developer Mode enabled, then a dialog will be shown to notify the user of the offending objects name.

![image](https://user-images.githubusercontent.com/73803386/129727642-8e929cc7-1b1c-4231-be44-abe4312ea265.png)

### Command Line Options

- Vinifera adds a number of command-line arguments allowing the user to skip the startup movies, or skip directly to a specific game mode and/or dialog.

- `-NO_STARTUP_VIDEO`
Skips all startup movies.

- `-SKIP_TO_TS_MENU`
Loads the game directly into the Tiberian Sun main menu (also skips startup movies).

- `-SKIP_TO_FS_MENU`
Loads the game directly into the Firestorm main menu (also skips startup movies). This option has priority over the TS main menu argument.

The following options will be affected by the choice of menu you skip to, otherwise, they default to the Tiberian Sun game mode.

- `-SKIP_TO_LAN`
Loads the game directly into the LAN dialog.

- `-SKIP_TO_CAMPAIGN`
Loads the game directly into the Campaign dialog.

- `-SKIP_TO_SKIRMISH`
Loads the game directly into the Skirmish dialog.

- `-SKIP_TO_INTERNET`
Loads the game directly into the Internet dialog.

- `-EXIT_AFTER_SKIP`
This option tells the game to exit when you press Cancel or Back from the dialog you skipped to.

### Developer Commands

#### `[ ]` Instant Special Recharge (Player)

- Toggles the instant recharge cheat for the player's superweapons.

#### `[ ]` Instant Special Recharge (AI)

- Toggles the instant recharge cheat for the AI player superweapons.

#### `[ ]` Place Tiberium

- Places Tiberium in a cell.

#### `[ ]` Reduce Tiberium

- TRemoves Tiberium from a cell.

## INI

- Add loading of `MPLAYER.INI` and `MPLAYERFS.INI` (Firestorm only) to override Rules data for multiplayer games (including Skirmish). Data contained in these INI's will not be loaded for the campaign and World Domination Tour games.