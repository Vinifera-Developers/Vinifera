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
- Implement `DebugOnly` for Campaigns (by CCHyper/tomsons26)
- Implement developer commands for instant superweapon recharge (by CCHyper/tomsons26)
- Add support for PNG images as an alternative to PCX images (by CCHyper/tomsons26)
- Implement option to display the super weapon recharge timer on the tactical view (by CCHyper/tomsons26)
- Implement `CanPassiveAcquire` for TechnoTypes (by CCHyper/tomsons26)
- Implement `CanRetaliate` for TechnoTypes (by CCHyper/tomsons26)
- Add loading of `MPLAYER.INI` and `MPLAYERFS.INI` to override Rules data for multiplayer games (by CCHyper/tomsons26)
- Allow the game's Window title, Cursor and Icon to be overridden (by CCHyper/tomsons26)
- Implement `IdleRate`, `StartIdleFrame` and `IdleFrames` for TechnoTypes (by CCHyper/tomsons26)
- Add loading of `GENERIC.MIX` and `ISOGEN.MIX` mixfiles (by CCHyper/tomsons26)
- The game will no longer fail to start if the startup mix files are not found (by CCHyper/tomsons26)
- Implement support for new custom theater types (by CCHyper/tomsons26)
- Adds loading of `ELOCAL(00-99).MIX` expansion mixfiles (by CCHyper/tomsons26)
- Add Rules INI selection dialog for Developer Mode (by CCHyper/tomsons26)
- Implement the "Build Off Ally" feature from Red Alert 2 (by CCHyper/tomsons26)
- Add background fill behind in-game user typed messages (by CCHyper/tomsons26)
- Implement JumpCamera (North, South, East and West) commands (by CCHyper/tomsons26)
- Implement `EnterTransportSound` and `LeaveTransportSound` for TechnoTypes (by CCHyper/tomsons26)
- Hardcode shroud and fog graphics to circumvent cheating in multiplayer games (by CCHyper/tomsons26)
- Implement `Mechanic` and `OmniHealer` for InfantryTypes.

Vanilla fixes:
- Fix HouseType `Nod` having the `Prefix=B` and `Side=GDI` in vanilla `rules.ini` by setting them to `N` and `Nod`, respectively (by CCHyper/tomsons26)
- Fix a bug where VQA files could not be loaded from the root directory or local search paths (by CCHyper/tomsons26)
- Fix a bug where the player could issue a harvester to begin harvesting Tiberium that is below a bridge while the mouse is over the bridge itself (by CCHyper/tomsons26)
- Fixes a bug where the values of `RandomRate` were not stored correctly (by CCHyper/tomsons26)
- Fix a bug where the `FSMENU` theme would incorrectly play instead of the `INTRO` theme when in Tiberian Sun mode after returning to the main menu (by CCHyper/tomsons26)
- Fix a bug where the game would crash when a trigger with the action "Wakeup group..." is executed and the requested Group was not found (by CCHyper/tomsons26)
- Fix a bug where animations with a `DetailLevel` value greater than 2 would not show in-game (by CCHyper/tomsons26)
- Fix a bug where `EngineerDamage` was incorrectly loaded with `EngineerCaptureLevel` (by CCHyper/tomsons26)
- Fix a bug where `EngineerDamage` was not used to calculate the engineer damage (by CCHyper/tomsons26)
- Fix a bug where `EngineerCaptureLevel` was not considered when checking the target building (by CCHyper/tomsons26)
- Fix a bug where air transports are unable to land when given a move order (by CCHyper/tomsons26)

Fixes / interactions with TS Patches:
- Fixed something (by someone)

</details>

