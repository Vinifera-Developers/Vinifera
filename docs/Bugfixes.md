# Fixed / Improved Logics

This page lists all vanilla bugs fixed by Vinifera.

## Bugfixes and miscellaneous

- Fix HouseType `Nod` having the `Prefix=B` and `Side=GDI` in vanilla `rules.ini` by setting them to `N` and `Nod`, respectively.
- Fix a bug where VQA files could not be loaded from the root directory or local search paths.
- Fix a bug where the player could issue a harvester to begin harvesting Tiberium that is below a bridge while the mouse is over the bridge itself.
- Fix a bug where the values of `RandomRate` were not stored correctly. In addition to this, negative values will also be converted to absolute numbers.
- Fix a bug where the `FSMENU` theme would incorrectly play instead of the `INTRO` theme when in Tiberian Sun mode after returning to the main menu.
- Fix a bug where the game would crash when a trigger with the action "Wakeup group..." is executed and the requested Group was not found.
- Fix a bug where animations with a `DetailLevel` value greater than 2 would not show in-game.
- Fix a bug where `EngineerDamage` was incorrectly loaded with `EngineerCaptureLevel`.
- Fix a bug where `EngineerDamage` was not used to calculate the engineer damage.
- Fix a bug where `EngineerCaptureLevel` was not considered when checking the target building.
- Fix a bug where air transports are unable to land when given a move order.
