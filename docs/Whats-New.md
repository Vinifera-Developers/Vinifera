# What's New

This page lists the history of changes across stable Vinifera releases and also all the stuff that requires modders to change something in their mods to accommodate.

## Migrating

% ```{hint}
% You can use the migration utility (can be found on [Vinifera supplementaries repo](https://github.com/Vinifera-Developers/ViniferaSupplementaries)) to apply most of the changes automatically using a corresponding sed script file.
% ```

### From vanilla

### When updating Vinifera

### From TS Patches

### New user settings in `SUN.ini`

- These are new user setting keys added by various features in Vinifera. Most of them can be found in either in [user inteface](User-Interface.md) or [miscellaneous](Miscellanous.md) sections. Search functionality can be used to find them quickly if needed.

```ini

```

### For Map Editor (Final Sun)

<details>
  <summary>Click to show</summary>

  In `FAData.ini`:
  ```ini

  ```
</details>

## Changelog

### 0.0

<details open>
  <summary>Click to show</summary>

New:
- Implement `CurleyShuffle` for AircraftTypes (by CCHyper/tomsons26)
- Implement `ReloadRate` for AircraftTypes (by CCHyper/tomsons26)
- Implement `AILegalTarget` for TechnoTypes (by CCHyper/tomsons26)
- Add support for up to 32767 waypoints to be used in scenarios (by CCHyper, tomsons26, ZivDero, secsome)
- Implement the loading of Tutorial messages from scenarios (by CCHyper/tomsons26)
- Add `IceStrength` to Rules, and `IceDestructionEnabled` scenario option (by Rampastring)
- Allow the remap color of `Neutral` and `Special` houses to be overridden in multiplayer games (by CCHyper/tomsons26)
- Add `RequiredAddon` to Theme control types (by CCHyper/tomsons26)
- Implement `IntroMovie` for Campaigns (by CCHyper/tomsons26)
- Implements DebugOnly for Campaigns (by CCHyper/tomsons26)

Vanilla fixes:
- Fix HouseType `Nod` having the `Prefix=B` and `Side=GDI` in vanilla `rules.ini` by setting them to `N` and `Nod`, respectively (by CCHyper/tomsons26)
- Fix a bug where VQA files could not be loaded from the root directory or local search paths (by CCHyper/tomsons26)
- Fix a bug where the player could issue a harvester to begin harvesting Tiberium that is below a bridge while the mouse is over the bridge itself (by CCHyper/tomsons26)
- Fixes a bug where the values of `RandomRate` were not stored correctly (by CCHyper/tomsons26)

Fixes / interactions with TS Patches:
- Fixed something (by someone)

</details>

