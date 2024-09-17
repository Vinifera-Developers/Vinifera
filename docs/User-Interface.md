# User Interface

This page lists all user interface additions, changes, fixes that are implemented in Vinifera.

## Bugfixes and miscellanous

- PNG images can be used as an alternative to PCX images. This new system scans for the requested filename with the .PNG extension and use that if found, otherwise it will fall back to scanning and load the .PCX file.
```{note}
This system only supports 8-bit PNG. All other formats such as Greyscale, Paletted, Alpha and 16-bit are not supported.
```
Attached is a set of the original loading screens with a minor edit and saved as PNG for testing;
[PNG_Loading_Screens.zip](https://github.com/Vinifera-Developers/Vinifera/files/7392707/PNG_Loading_Screens.zip)

## Audio

## Hotkey Commands

### `[ ]` Toggle Special Timers

- Toggles the visibility of Super Weapon timers.

### `[ ]` Jump Camera West

- Jump the tactical map camera to the west edge of the map. Defaults to `Ctrl` + `Left Arrow`.

### `[ ]` Jump Camera East

- Jump the tactical map camera to the east edge of the map. Defaults to `Ctrl` + `Right Arrow`.

### `[ ]` Jump Camera North

- Jump the tactical map camera to the north edge of the map. Defaults to `Ctrl` + `Up Arrow`.

### `[ ]` Jump Camera South

- Jump the tactical map camera to the north edge of the map. Defaults to `Ctrl` + `Down Arrow`.

## Loading screen

## Sidebar / Battle UI

- Super Weapon timers, similar to those found in Red Alert 2, can now be displayed on the tactical view. This is disabled by default and each relevant SuperWeaponType must have it enabled. Superweapons that are offline due to low power or are disabled via other purposes will not show.
In `RULES.INI`:
```ini
[SuperWeaponType]
ShowTimer=<boolean>  ; When this superweapon is active, does its recharge timer display on the tactical view? Defaults to no.
```

- Vinifera adds a background behind the user typed messages that appear in-game to provide better readability.
![image](https://user-images.githubusercontent.com/73803386/137031682-3f265d48-7f28-410f-bf0d-3260e24f1748.png)

In `UI.INI`:
```ini
[Ingame]
TextLabelOutline=<boolean>  ; Should the text be drawn with a black outline? Defaults to yes.
TextLabelBackgroundTransparency=<unsigned integer>  ; The transparency of the text background fill. Ranged between 0 and 100. Defaults to 50.
```

## Tooltips

## Miscellanous

