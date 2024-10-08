# What's New

This page lists the history of changes across stable Vinifera releases and also all the stuff that requires modders to change something in their mods to accommodate.

## Migrating

% ```{hint}
% You can use the migration utility (can be found on [Vinifera supplementaries repo](https://github.com/Vinifera-Developers/ViniferaSupplementaries)) to apply most of the changes automatically using a corresponding sed script file.
% ```

### From vanilla

- Tiberium `[Vinifera]->Power`, previously hardcoded to `17`, has been de-hardcoded. As such, a proper value needs to be set in `RULES.INI`.

% ### When updating Vinifera

### From TS Patches

- [place_building_hotkey](https://github.com/CnCNet/ts-patches/blob/master/src/place_building_hotkey.c) and [repeat_last_building_hotkey](https://github.com/CnCNet/ts-patches/blob/master/src/repeat_last_building_hotkey.c) should be disabled to avoid conflict with the analogous Vinifera keyboard commands.
- Removal of ts-patches [tiberium_stuff](https://github.com/CnCNet/ts-patches/blob/master/src/tiberium_stuff.asm) and [tiberium_damage](https://github.com/CnCNet/ts-patches/blob/master/src/tiberium_damage.asm) is required for this to work properly. Please keep in mind that Power once again behaves like in vanilla in regards to chain explosions and should be set to reasonable values, while `DamageToInfantry` should be used to customize Tiberium's damage to infantry.

% ### New user settings in `SUN.ini`
% 
% - These are new user setting keys added by various features in Vinifera. Most of them can be found in either in [user inteface](User-Interface.md) or [miscellaneous](Miscellaneous.md) sections. Search functionality can be used to find them quickly if % needed.
% 
% ```ini
% 
% ```
% 
% ### For Map Editor (Final Sun)
% 
% <details>
%   <summary>Click to show</summary>
% 
%   In `FAData.ini`:
%   ```ini
% 
%   ```
% </details>

## Changelog

### 0.1

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
- Implement `Mechanic` and `OmniHealer` for InfantryTypes (by CCHyper/tomsons26)
- Add support for 8-bit PCX and PNG cameos (by CCHyper/tomsons26)
- Implement `Soylent` for TechnoTypes (by CCHyper/tomsons26)
- Implement `SpawnDelay` for BulletTypes (by CCHyper/tomsons26)
- Implement `Suicide` and `DeleteOnSuicide` for WeaponTypes (by CCHyper/tomsons26)
- Implement `VoiceHarvest`, `VoiceDeploy`, `VoiceEnter`, and `VoiceCapture` for TechnoTypes (by CCHyper/tomsons26)
- Implement various Red Alert 2 AnimType features (by CCHyper/tomsons26)
- Add game options to allow MCV's to auto-deploy on game start and to pre-place construction yards instead of spawning an MCV (by CCHyper/tomsons26)
- Add developer commands for placing and removing Tiberium from a cell (by CCHyper/tomsons26)
- Implement Electric Bolts for WeaponTypes (by CCHyper/tomsons26)
- Make the position of the health bar graphic on unit selection boxes customizable (by CCHyper/tomsons26)
- Output screenshots to their own sub-directory (by CCHyper/tomsons26)
- Echo the user's sent messages back to them as confirmation they were sent (by CCHyper/tomsons26)
- Reimplement the command line argument `-CD` from Red Alert to allow file search path override logic (by CCHyper/tomsons26)
- Allow the score screen to be skipped at the end of a multiplayer game (by CCHyper/tomsons26)
- Add warning notification if a NULL house instance is detected during the game loading screen (by CCHyper/tomsons26)
- Implement light sources for TerrainTypes (by CCHyper/tomsons26)
- Add command line options to skip to specific game modes and to skip startup movies (by CCHyper/tomsons26)
- Implement diagonal scroll commands (by CCHyper/tomsons26)
- Add keyboard commands for playing previous and next music tracks in the jukebox (by CCHyper/tomsons26)
- Implement CloakSound and UncloakSound for TechnoTypes (by CCHyper/tomsons26)
- Restore the screen shake when a strong unit or building is destroyed (by CCHyper/tomsons26)
- Implement various Red Alert 2 WarheadType features (by CCHyper/tomsons26)
- Add reading of Weapons list from RULES.INI (by CCHyper/tomsons26)
- Allow WalkRate to be optionally loaded from ART.INI image entries (by CCHyper/tomsons26)
- Add gate rising and lowering sound overrides for buildings (by CCHyper/tomsons26)
- Add UnitType flag to prevent a vehicle from being picked up by a Carryall (by CCHyper/tomsons26)
- Add support for a custom unloading class when a harvester is unloading at a refinery (by CCHyper/tomsons26)
- Implement ToggleAIControlCommandClass (by CCHyper/tomsons26)
- Add support for more graphic facings for UnitTypes and various associated items (by CCHyper/tomsons26)
- Add `ImmuneToEMP` to TechnoTypes (by Rampastring)
- Implement ToggleFrameStepCommandClass (by CCHyper/tomsons26)
- Thicken the waypoint and rally point lines and adds stroke/outline to the waypoint number (by CCHyper/tomsons26)
- Implement hotkey command to enter the manual placement mode (by CCHyper/tomsons26)
- Allow harvesters to be considered when executing the "Guard" command (by CCHyper/tomsons26)
- Harvesters now auto harvest when built from the war factory (by CCHyper/tomsons26)
- Patch to allow Skirmish games to be started with no AI house(s) (by CCHyper/tomsons26)
- Implement the Produce Cash logic for BuildingTypes (by CCHyper/tomsons26)
- Add various developer mode hotkey commands (by CCHyper/tomsons26)
- Add a "Load Game" button to the retry dialog on mission failure (by CCHyper/tomsons26)
- Save screenshots as a PNG file instead of PCX file (by CCHyper/tomsons26)
- Add support for playing the renamed intro movies from The First Decade and Freeware TS installations (by CCHyper/tomsons26)
- Implement the Blowfish algorithm and removes the requirement for BLOWFISH.DLL (by CCHyper/tomsons26)
- Allow the game to continue if the side specific mix files are not found (by CCHyper/tomsons26)
- Change the default value of AllowHiResModes to true (by CCHyper/tomsons26)
- Implement CnCNet4 support (by CCHyper/tomsons26)
- Implement CnCNet5 support (by CCHyper/tomsons26)
- Adds keyboard commands to reproduce the last items that were built (by CCHyper/tomsons26)
- Allow customizing the pips used for Tiberiums in unit storage, as well as their draw order (by ZivDero)
- Buildings now show their storage with the proper pips, instead of showing pip 1 for all tiberiums (by ZivDero)
- The pip used to diplay weeds can now be customized via `[AudioVisual]->WeedPipIndex` (by ZivDero)
- Technos can have a custom pip be drawn in the same place as the medic pip using `[TechnoType]->SpecialPipIndex` (by ZivDero)
- The location of the control group number and veterancy pips can now be customized in `UI.ini` (by ZivDero)
- MaxPips can now we customized (by ZivDero)
- Change starting unit placement to be the same as Red Alert 2 (by CCHyper/tomsons26)
- Make it possible to assign rally points to service depots (by Rampastring)
- Allow adding new Tiberiums and customizing their Image (by ZivDero)
- Tooltips for objects on the sidebar will now show their name, as well as a custom description (by Rampastring)
- Implement a new sidebar with tabs (by ZivDero)
- Add the ability to queue/dequeue 5 units at a time, or dequeue all units instantly (by ZivDero)
- Make OverlayTypes 27 to 38 (fourth Tiberium images) passable by infantry (by AlexB)
- Implement the support for new ArmorTypes and allow forbidding force-fire, passive-acquire and retaliation versus specific armor types (by ZivDero/CCHyper)
- Add a developer command to dump all heaps to the log (by ZivDero)
- Make harvesters drop the Tiberium type they're carrying on death, instead of Tiberium Riparius (by ZivDero)
- Make it so that it is no longer required to list all Tiberiums in a map to override some Tiberium's properties (by ZivDero)
- Add `TransformsInto` and `TransformRequiresFullCharge` to UnitTypes (by Rampastring)
- Fix a bug where players were only able to queue up to `(BuildLimit - 1)` objects when an object has `BuildLimit > 0` (by Rampastring)

Vanilla fixes:
- Fix HouseType `Nod` having the `Prefix=B` and `Side=GDI` in vanilla `rules.ini` by setting them to `N` and `Nod`, respectively (by CCHyper/tomsons26)
- Fix a bug where VQA files could not be loaded from the root directory or local search paths (by CCHyper/tomsons26)
- Fix a bug where the player could issue a harvester to begin harvesting Tiberium that is below a bridge while the mouse is over the bridge itself (by CCHyper/tomsons26)
- Fix a bug where the values of `RandomRate` were not stored correctly (by CCHyper/tomsons26)
- Fix a bug where the `FSMENU` theme would incorrectly play instead of the `INTRO` theme when in Tiberian Sun mode after returning to the main menu (by CCHyper/tomsons26)
- Fix a bug where the game would crash when a trigger with the action "Wakeup group..." is executed and the requested Group was not found (by CCHyper/tomsons26)
- Fix a bug where animations with a `DetailLevel` value greater than 2 would not show in-game (by CCHyper/tomsons26)
- Fix a bug where `EngineerDamage` was incorrectly loaded with `EngineerCaptureLevel` (by CCHyper/tomsons26)
- Fix a bug where `EngineerDamage` was not used to calculate the engineer damage (by CCHyper/tomsons26)
- Fix a bug where `EngineerCaptureLevel` was not considered when checking the target building (by CCHyper/tomsons26)
- Fix a bug where air transports are unable to land when given a move order (by CCHyper/tomsons26)
- Fix the position of the health bar graphic on unit selection boxes (by CCHyper/tomsons26)
- Fix a bug where the game would crash when attempting to generate a random map if the `Neutral` or `Special` HouseTypes are not found (by CCHyper/tomsons26)
- Fix a bug where the game would crash when attempting to generate a random map if there are fewer than 4 HouseTypes defined (by CCHyper/tomsons26)
- Fix a limitation where the game could only choose between the first two HouseTypes for the AI players (by CCHyper/tomsons26)
- Fix a bug where the `Cloakable=yes` had no effect on AircraftTypes (by CCHyper/tomsons26)
- Fix a bug where `CloakStop` had no effect on the cloaking behaviour (by CCHyper/tomsons26)
- Fix a bug where pre-placed crates and crates spawned by a destroyed truck will trigger a respawn when they are picked up (by CCHyper/tomsons26)
- Increase the string buffer size from 128 to 2048 characters for when reading and writing Owners from INI (by CCHyper/tomsons26)
- Fix bugs where the Jumpjet uses the wrong animation sequence when firing and in the air (by CCHyper/tomsons26)
- Fix a bug where the wrong palette is used to draw the cameo above an enemy spied factory building (by CCHyper/tomsons26)
- Fix the animation speed of Waypoint and Rally Point lines so they are normalised and no longer subjected to the game speed setting (by CCHyper/tomsons26)
- Fix a limitation where returning to the Skirmish dialog after a game clamps the chosen side between 0 (GDI) and 1 (Nod) (by CCHyper/tomsons26)
- Fix a bug where the user is able to place a building anywhere on the map by taking advantage of the sidebar (by CCHyper/tomsons26)
- Fix division by zero crashes when ShakeScreen is set to 0 (by CCHyper/tomsons26)
- Increase the IsoMapPack5 buffer size when decoding a map (by CCHyper/tomsons26)
- Fix a bug where looping animations incorrectly use the unsynchronized RNG (by CCHyper/tomsons26)
- Fix a bug where `IsTrainable` is not checked when an object picks up a veteracy crate (by CCHyper/tomsons26)
- Fix a bug where `IsInsignificant` was not checked when a unit dies (by CCHyper/tomsons26)
- Fix the incorrect (RA legacy) cell calculation for the "move to cell" team script (by CCHyper/tomsons26)
- Fix a bug where the sidebar mouse wheel scrolling "error" sound can be heard at the main menu (by CCHyper/tomsons26)
- Fix a bug with TriggerTypes not setting difficulty flags correctly when reading from INI (by CCHyper/tomsons26)
- Fix a bug with triggers enabled via other triggers ignoring difficulty settings (by CCHyper/tomsons26)
- Fix MultiMission `MaxPlayers` incorrectly loaded with `MinPlayers` (by CCHyper/tomsons26)
- Fix to maintain aspect ratio when scaling movies/videos (by CCHyper/tomsons26)
- Fix incorrect spelling of "Loser" on the multiplayer score screen debug output (by CCHyper/tomsons26)
- Fix incorrect stretching of the main menu transition movies (by CCHyper/tomsons26)
- Bugfixes and improvements for the Dropship Loadout menu (by CCHyper/tomsons26)
- Fix an issue where losers were not marked as defeated in multiplayer when using TACTION_WIN or TACTION_LOSE to end the game (by Rampastring)
- Fix a bug where under some circumstances, the player could hear "New Construction Options", even though no new construction options were available (by ZivDero)
- Fix a bug where attempting to start construction when low funds would put the queue on hold (by ZivDero)
- Port the fix for the (Whiteboy bug)[https://modenc.renegadeprojects.com/Whiteboy-Bug] (by ZivDero)
- Fix a bug where the objects would sometimes receive a minimum of 1 damage even if MinDamage was set to 0 (by ZivDero)

</details>

