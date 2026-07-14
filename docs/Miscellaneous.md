# Miscellaneous

This page describes every change in Vinifera that wasn't categorized into a proper category yet.

- Vinifera hardcodes the shroud and fog graphics to circumvent cheating in multiplayer games.
- Vinifera redirects saved screenshots using the keyboard command to a new sub-directory in the games folders, `Screenshots`.
- Vinifera allows Skirmish games to be started with no AI house(s).
- Vinifera implements the Blowfish algorithm into the Vinifera DLL itself, removing the requirement for the external BLOWFISH.DLL library. As a result, this allows the game will run without BLOWFISH.DLL registered on the target system or present in the installation directory. The game can still load encrypted mix files to be loaded without any issues.
- Vinifera allows players to set a rally point for their service depot, similar to the functionality already available for factories.
- OverlayTypes 27 to 38 (fourth Tiberium images) were hardcoded to be impassable by infantry. This limitation is removed.
- Harvesters used to drop their cargo as Tiberium Riparius on death. They will now drop the Tiberium types they are carrying, instead.
- It is no longer required to list all Tiberiums in a map to override some Tiberium's properties.
- `FreeUnit` or `PadAircraft` would in some cases affect the cost of a building. This functionality has been removed.
- Pre-placed units can now have missions in multiplayer.
- Parachute animations with `AltPalette=yes` now remap to the parachuted unit owner's color.
- Improve alternative factory selection when the primary factory is blocked.
- When revealing shroud via a unit, structure, or triggers, TS had a maximum allowed sight radius of 10. This meant units could not have a `Sight=` value above 10, and Reveal Around Waypoint trigger actions could not reveal in radius higher than 10 even if specified in `RevealTriggerRadius`. Vinifera now allows and handles revealing shroud in any desired range, with no limit.
- Aircraft can now click on Helipad that are occupied or about to be occupied by other aircraft, which reassigns them to a different free Helipad, or near the existing Helipad if no free Helipads exist. 
- Players can now click on a Service Depot with units and aircraft even if it is occupied or about to be occupied by other units. Doing so will add these units to the list of units waiting to be repaired.
- Vinifera allows aircraft to use Q-Move, similarly to other types of units in the game. Q-Moving aircraft will stay in the air as they move on to their next destination. Unlike ground units, aircraft cannot target enemies while Q-Moving. Ordering queue-moves to an aircraft currently targetting an enemy will remove the attack order. Carryalls get extended handling while Q-Moving, allowing it to pick up units along the way and carry them until the end of their path.
- Healing units now apply area-guard on a nearby combatant unit when attacking enemy targets, rather than area-guarding on themselves.
- Make `SOUND01.INI` load additively with `SOUND.INI`, reload sounds after loading side `MIX` files.
- Allow pre-placed units to have missions in multiplayer.
- `BaseUnit` now accepts a list of units. Players will be granted the first unit in the list that has their house listed under `Owners=`.
- The AI now correctly considers all entries of `BuildConst`, `BuildRefinery`, `BuildWeapons` and `HarvesterUnit`.

## SIMD Blitters

Vinifera replaces the engine's software pixel blitters - the per-scanline routines that draw every sprite, unit, building, shadow and translucent effect - with hand-written SIMD implementations. The fastest variant supported by the host CPU is selected automatically at startup (AVX2 or SSE2); systems without SSE2 keep the original routines. This only affects the 16-bit (hicolor) renderer and requires no configuration.

The replacements produce the same output as the originals, with one intentional improvement: the 25%/50%/75% translucency blend now uses exact per-channel rounding, removing the slight progressive darkening the original blend caused when translucent sprites were layered.

## Modern Video Playback

Vinifera adds support for modern video formats as replacements for the original VQA movies. If a file with the same basename as a VQA is found with one of the supported extensions (`.MP4`, `.WMV`, `.MPG`, `.AVI`), it will be used in place of the VQA.

This applies to both fullscreen cutscene movies and in-game movies played in the radar area.

```{note}
Modern movies respect the `[Video]->StretchMovies` key in `SUN.INI`. If `StretchMovies=no`, movies will be played at 640x400 (preserving the aspect ratio).
```

```{note}
While in-game movies are scaled to play at the size of the radar, it is recommended that they be `140x100` in size. Due to technical reasons they have to be processed in software, and excessively large movies may decrease performance at no benefit.
```

## INI

### INI Inclusion and Inheritance

INI files now support modularity through the `[$Include]` and `[$Inherit]` sections. These allow files to be merged or used as templates, each with distinct override behaviors.

#### `[$Include]`
* **Behavior:** The current file pulls in external data as if "pasting" it into the logic.
* **Priority:** Included files override the current file. Values read later in the inclusion chain take precedence.
* **Use Case:** Splitting a massive configuration into smaller, organized sub-modules.

#### `[$Inherit]`
* **Behavior:** The current file uses external files as a base template or background layer.
* **Priority:** The current file overrides inherited files. This allows the host file to act as a "patch" layer.
* **Use Case:** Creating map-specific overrides or mods that only define changes relative to a base file (e.g., `RULES.INI`).

### Technical Rules
* Supported in any INI file (`RULES.INI`, `ART.INI`, `SOUND.INI`, `AI.INI`, maps, etc.).
* Files must be listed with unique keys (e.g., `0=FILE1.INI`, `1=FILE2.INI`).
* Supported with files loaded from the game directory or from within any loaded `.MIX` archive.
* Both features perform **depth-first recursion** (nested includes are resolved before moving to the next file in the list).
* Entries are processed sequentially in the order in which they appear, regardless of their key (left of `=`).

In any `INI` file:
```ini
[$Inherit]
0=SOMEFILE1.INI  ; file name

[$Include]
0=SOMEFILE2.INI  ; file name
```

```{caution}
Avoid recursive includes. Vinifera does not provide circular reference protection; self-referencing files will cause the game to crash.
```

```{note}
Included/inherited files **MUST** be present. Failure to find such a file will cause the game to exit for security reasons.
```

### Section inheritance

Sections can now inherit entries from one or more "parent" sections using the `$Inherits` key. This allows for shared configuration templates and reduced redundancy within INI files.

### Technical Rules
* If a key is missing or has no value in the current section, the game looks up the value in the specified parent sections. If the value is still not found, it falls back to the hardcoded engine default.
* The current (child) section always takes precedence.
* You can list multiple parents separated by commas. The lookup follows a **left-to-right** priority:
  `$Inherits=ParentA, ParentB` (The engine checks ParentA first, then ParentB).
* Inheritance is **depth-first**. If ParentA inherits from ParentX, ParentA's hierarchy will be fully resolved before ParentB is checked.

In any `INI` file:
```ini
[TemplateA]
Armor=heavy
Speed=5

[TemplateB]
Speed=10
Weapon=Vulcan

[NewUnit]
$Inherits=TemplateA,TemplateB
Owner=GDI
; Result: Armor=heavy (from A), Weapon=Vulcan (from B), Speed=5 (from A), Owner=GDI (Own value)
```

```{caution}
There are no internal guards against recursive inheritance. If Section A inherits from Section B, and Section B inherits from Section A, the game will crash.
```

```{caution}
Section-level does not work in certain cases that *iterate* a section. Notably, it may not be used with type lists, `[Tutorial]`, map briefings, as well as with map object lists.
```

## Spawner

- Vinifera implements its own spawner, capable of starting a new singleplayer, skirmish or multiplayer game, as well as loading saved games.
- To start the game in spawner mode, the `-SPAWN` command line argument must be specified.
- The spawner's options can be configures in `SPAWN.INI`.

In `SPAWN.INI`:
```ini
[Settings]
; Game Mode Options
Bases=yes                   ; boolean, do players start with MCVs/Construction Yards?
Credits=10000               ; integer, starting amount of credits for the players.
BridgeDestroy=yes           ; boolean, can bridges be destroyed?
Crates=no                   ; boolean, are crates enabled?
ShortGame=no                ; boolean, is short game enabled?
BuildOffAlly=no             ; boolean, is building off ally bases allowed?
GameSpeed=0                 ; integer, starting game speed.
MultiEngineer=no            ; boolean, is multi-engineer enabled?
UnitCount=0                 ; integer, starting unit count.
AIPlayers=0                 ; integer, number of AI players.
AIDifficulty=1              ; integer, AI difficulty.
AlliesAllowed=no            ; boolean, can players form and break alliances in-game?
HarvesterTruce=no           ; boolean, are harvesters invulnerable?
FogOfWar=no                 ; boolean, is fog of war enabled?
MCVRedeploy=yes             ; boolean, can MCVs be redeployed?

; Savegame Options
LoadSaveGame=no             ; boolean, should the spawner load a saved game, as opposed to starting a new scenario?
SaveGameName=               ; string, name of the saved game to load.
AutoSaveInterval=7200       ; integer, interval in frames between auto-saves.
NextAutoSaveNumber=1        ; integer, the number of the next campaign auto-save to make.

; Scenario Options
Seed=0                      ; integer, random seed.
TechLevel=10                ; integer, maximum tech level.
IsCampaign=no               ; boolean, is the game that is about to start campaign, as opposed to skirmish?
CampaignID=-1               ; integer, ID of the campaign (from BATTLE.INI) to start
CampaignModeHuman=1         ; DiffType, difficulty used by the human player in Campaign.
CompaignModeComputer=1      ; DiffType, difficulty used by the AI players in Campaign.
Tournament=0                ; integer, WOL Tournament Type
WOLGameID=3735928559        ; unsigned integer, WOL Game ID
ScenarioName=spawnmap.ini   ; string, name of the scenario (map) to load.
MapHash=                    ; string, map hash, only used in statistics collection.
UIMapName=                  ; string, name of the map, only used in statistics collection.
PlayMoviesInMultiplayer=no  ; boolean, should movies be played in multiplayer.

; Network Options
Protocol=2                  ; integer, network protocol to use.
FrameSendRate=4             ; integer, starting FrameSendRate value.
ReconnectTimeout=2400       ; integer, player reconnection timeout.
ConnTimeout=3600            ; integer, player connection timeout.
MaxAhead=-1                 ; integer, starting MaxHead value.
PreCalcMaxAhead=0           ; integer, starting PrecalcMaxHead value.
MaxLatencyLevel=255         ; unsigned byte, maximum allowed Protocol 0 latency level.

; Tunnel Options
TunnelId=0                  ; integer, tunnel ID.
TunnelIp=0.0.0.0            ; string, tunnel IP.
TunnelPort=0                ; integer, tunnel port.
ListenPort=1234             ; integer, listen port.

; Extra Options
Firestorm=yes               ; boolean, should the game start with Firestorm enabled?
QuickMatch=no               ; boolean, should the game start in Quick Match mode?
SkipScoreScreen=no          ; boolean, should the score screen be skipped once the game is over?
WriteStatistics=no          ; boolean, should statistics be sent?
AINamesByDifficulty=no      ; boolean, should AI players have their difficulty in their name?
CoachMode=no                ; boolean, should defeated players that have allies not have the entire map revealed to them upon death?
AutoSurrender=yes           ; boolean, should players surrender on disconnection, as opposed to turning their base over to the AI?
AttackNeutralUnits=no       ; boolean, should neutral units be targeted by the player's army automatically?
ScrapMetal=no               ; boolean, should explosions use alternative animations from the `ScrapExplosion=` list?
ContinueWithoutHumans=yes   ; boolean, should the game not end even if the only players left alive are AI?
```

- Information about the local player is read from the `Settings` section, for all other players - from `OtherX` sections, where `X` ranges from `1` to `7`.

In `SPAWN.INI`:
```ini
[PLAYERSECTION]
IsHuman=no                  ; boolean, is this a human player?
Name=                       ; string, the player's name.
Color=-1                    ; integer, the player's color.
House=-1                    ; integer, the player's house.
Difficulty=-1               ; integer, the player's difficulty.
Ip=0.0.0.0                  ; string, the player's IP address.
Port=-1                     ; integer, the player's port.
```

- Additionally, AI players (always come after human players) have these options parsed from sections of the format `MultiX`, where `X` ranges from `1` to `8`.

In `SPAWN.INI`:
```ini
[MULTISECTION]
Color=-1                    ; integer, the player's color.
House=-1                    ; integer, the player's house.
Difficulty=-1               ; integer, the player's difficulty.
```

- Additionally, the spawner reads configuration for each house. Player houses come first, in the order of their color (increasing), then AI houses.
- Alliances are read from sections of the format `MultiX_Alliances`, where `X` ranges from `1` to `8`.

In `SPAWN.INI`:
```ini
[MULTISECTION]
IsSpectator=no              ; boolean, is this house a spectator (observer)?
SpawnLocations=-2           ; integer, spawn location of this house. 90 and -1 mean spectator, -2 means random.

[ALLIANCESSECTION]
HouseAllyOne=-1             ; integer, index of the house this house is allied to, -1 means none.
HouseAllyTwo=-1             ; integer, index of the house this house is allied to, -1 means none.
HouseAllyThree=-1           ; integer, index of the house this house is allied to, -1 means none.
HouseAllyFour=-1            ; integer, index of the house this house is allied to, -1 means none.
HouseAllyFive=-1            ; integer, index of the house this house is allied to, -1 means none.
HouseAllySix=-1             ; integer, index of the house this house is allied to, -1 means none.
HouseAllySeven=-1           ; integer, index of the house this house is allied to, -1 means none.
HouseAllyEight=-1           ; integer, index of the house this house is allied to, -1 means none.
```

## Quality of Life

- Harvesters are now considered when executing the "Guard" command. They have a special case when assigned with the Guard mission that tells them to find the nearest Tiberium patch and begin harvesting.
- Harvesters now auto harvest when built from the war factory.
- Vinifera changes the default value of `IsScoreShuffle` to true.
- Vinifera changes the default value of `AllowHiResModes` to true.
- Factories now hold their object if there is no war factory available for the unit to exit from instead of refuding construction.

### DirectDraw replacement

- Vinifera replaced the old DirectDraw (`ddraw.dll`) API with SDL. As a result, DirectDraw wrappers are no longer necessary for the game to run properly, and may even be harmful.
- Accordingly, some new video settings are available in `SUN.INI`.

In `SUN.INI`:
```ini
[Video]
Windowed=no         ; boolean, should the game start in a window
WindowWidth=-1      ; integer, if positive and Windowed=true, sets the window width override
WindowHeight=-1     ; integer, if positive and Windowed=true, sets the window height override
RendererDriver=Auto ; renderer backend, valid options are "Auto", "Direct3D", "Direct3D11", "Direct3D12", "OpenGL" and "Vulkan"
ScaleMode=PixelArt  ; scale mode, valid options are "Linear", "Nearest" and "PixelArt"
CursorScale=0       ; integer, cursor scale factor override
VSync=no            ; boolean, is vertical synchronization on?
```

`RendererDriver` supports SDL's Direct3D backends, OpenGL and Vulkan. If SDL cannot initialize the game with the select renderer, startup will fail instead of silently falling back.

```{note}
`CursorScale` options:
- `<0` - disable scaling
- `0` - scale automatically
- `>0` - explicit scale value
```

```{warning}
Fullscreen mode uses a borderless window; exclusive fullscreen is not supported. To disable the windowed mode entirely, set `Windowed` to `false`.
```

### Starting Unit Placement

- Vinifera changes starting units to be placed in the same way as they are in Red Alert 2.
- The starting unit placement in Tiberian Sun is awkward and requires the player to micro-manage their units before they can deploy the MCV. Now, with this change, the starting units are placed exactly like Red Alert 2, allowing the player _(and the AI)_ to instantly deploy the MCV at the starting spawn location;

![image](https://user-images.githubusercontent.com/73803386/123679907-f501b080-d83f-11eb-8de8-a790fc2f6815.png)

### MCV Auto-deploy

- Vinifera allow you to start the game with the MCV deployed, or have the MCV auto-deploy on start.

In `RULES.INI`:
```ini
[MultiplayerDefaults]
AutoDeployMCV=no      ; boolean, should player MCV's auto-deploy on game start?
PrePlacedConYards=no  ; boolean, should pre-place construction yards instead of spawning an MCV?
                      ; NOTE: This option only has an effect if the unit count is set to 1.
                      ; NOTE: This option has priority over AutoDeployMCV.
```

### Bridge Strength
- Vinifera adds the ability to replace the random chance to break bridges with strength trackers for all bridge types. When enabled, bridges will sustain damage over time and break when they accumulate enough damage.
- Bridge strength is determined via the existing `BridgeStrength=` key.
- Remaining strength is tracked separately for each 3x1 (1x3) bridge tile.
- Repairing the bridge by entering a bridge repair hut with an engineer fully replenishes all of that bridge's strength trackers.
- When bridge strength trackers are enabled, bridges can now have an armor type associated with them in order to apply the appropriate Verses of all warheads to them. If no armor type is assigned to the bridge, then bridges will take 100% of all weapon damage.

in `RULES.INI`:
```ini
[CombatDamage]
UseBridgeHealth=no   	; boolean, should bridges track their health instead of having random chance to break
BridgeStrength=1000 	; integer, the health each bridge tile can sustain before breaking. Existing key from vanilla.
BridgeArmor= 			; Armor Type, the armor type of associated with the bridge for damage calculations. Requires UseBridgeHealth=yes for this to take effect.
```

## Prerequisites

### Multi-MCV

- Vinifera allows turning off the check for the house that built the MCV to allow giving each faction their own MCV (instead of a shared MCV).

In `RULES.INI`:
```ini
[General]
MultiMCV=no  ; boolean, should MCVs allow the construction of buildings of any house, not only the house that built them?
```

### Sticky Technologies

- In vanilla, technologies are "sticky", that is, for example, if you lose a tech center, you will not lose access to objects that require a tech center until you lose all factories of the type. Vinifera allows turning off this behavior.

In `RULES.INI`:
```ini
[General]
RecheckPrerequisites=no  ; boolean, should prerequisites be rechecked, and unavailable items removed from the sidebar, when buildings are lost?
```

## Auto-Saves

- When playing campaigns, Vinifera will now make auto-saves for the player at equal intervals. The number of auto-saves to keep, as well as the interval, can be customized.

In `SUN.INI`:
```ini
[Options]
AutoSaveCount=5        ; integer, the number of auto-saves to keep simultaneously. Setting to 0 will disable auto-saves.
AutoSaveInterval=7200  ; integer, the interval between auto-saves, in frames.
```

## Multi-Engineer

- Vinifera fixes `EngineerDamage` and `EngineerCaptureLevel` to be considered by the game, like they were in Tiberian Dawn and Red Alert.

In `RULES.INI`:
```ini
[General]
EngineerDamage=0.0        ; float, the engineer will damage a building by this percent of its full health each time it enters.
EngineerCaptureLevel=1.0  ; float, if the building’s health is equal to or below this percentage of its strength it can be captured by an engineer.
```
```{warning}
Upon observing the values used in `FIRESTRM.INI`, this could potentially cause an issue with the vanilla game. `FIRESTRM.INI` has the values `EngineerCaptureLevel=1.0` and `EngineerDamage=0.0`. Below are some values to help test these bug fixes and their potential impact on the vanilla game.

Red Alert Default values:
`EngineerDamage=0.33`
`EngineerCaptureLevel=0.25`

Red Alert Multiplayer (`MPLAYER.INI`) values:
`EngineerDamage=0.33`
`EngineerCaptureLevel=0.66`
```

## Build off Ally

- Vinifera implements the "Build Off Ally" feature from Red Alert 2 as a Quality of Life improvement. This is now the default behaviour for multiplayer games.

In `RULES.INI`:
```ini
[MultiplayerDefaults]
BuildOffAlly=yes                   ; boolean, can players build their own structures adjacent to structures owned by their allies?

[SOMEBUILDING]                     ; BuildingType
EligibleForAllyBuilding=<boolean>  ; Is this building eligible for proximity checks by players who are its owner's allies?
                                   ; For buildings with ConstructionYard=yes this defaults to yes, otherwise it defaults to no.
```

## AI Repair Base Nodes

- You can now customize whether the AI can repair structures created as base nodes. 
- Applies globally to all AI houses, and only affects non-skirmish games. 
- Can be set and overridden at either game (Rules.ini) or map level.

In a scenario file:
```ini
[AI]
AIRepairBaseNodes=no   ; boolean, can the AI can repair structures created as base nodes?
```

## Armory and Hospital Improvements
- Hospitals and armories can now set rally points, similarly to production buildings and service depots.
- Hospitals and armories can now accept multiple infantry, which will form a queue around them. Units will go in one at a time.
- If charges (ammo) deplete while units are still waiting in the queue, remaining units will be dismissed and be ordered to go to the respective rally point instead.

## Veteran and Elite Sights
- Technos can now be given specific values to use as their sight range when they are Veteran or Elite.
- When a key is not specified, technos will fall back to the lower level sight. For example, if `EliteSight` is not specified but `VeteranSight` is, a techno that is Elite would use `VeteranSight`. If it's also not specified, it will fall back to `Sight`.

In `RULES.INI`:
```ini
[SOMETECHNO]
VeteranSight=-1  ; integer, the sight range to use when a techno is Veteran. Falls back to Sight when not provided.
EliteSight=-1    ; integer, the sight range to use when a techno is Elite. Falls back to VeteranSight when not provided.
```

## Window Title, Cursor and Icon

- The game's Window title, Cursor and Icon can be overridden. These controls are loaded from a new INI file, `VINIFERA.INI`.
```{note}
While this is INI file is optional, it is recommended the `ProjectName` and `ProjectVersion` are set by the mod developer as this information is printed into debug logs and crash dumps to aid in the troubleshooting process.
```

In `VINIFERA.INI`:
```ini
[General]
ProjectName=     ; string, the project's title name string. Limited to 64 characters.
ProjectVersion=  ; string, the project's version string. Limited to 64 characters.
IconFile=        ; string, the name of the icon file (including the .ICO extension) to use for the games window. Limited to 64 characters.
CursorFile=      ; string, the name of the cursor file (including the .CUR extension) to use for the game's cursor. Limited to 64 characters.
```

```{note}
The filenames also support subdirectories.
```

## Custom Saved Games Directory

- By default Vinifera changes the game to save games into a subdirectly called `Saved Games`. This can be customized.

In `VINIFERA.INI`:
```ini
[General]
SavedGamesDirectory=Saved Games  ; string, the name of the directory in which to save games.
```

```{note}
Subdirectories are also supported, e. g. `Tiberian Sun\Saved Games`.
```

## Voxel Light Customization

- In vanilla, voxels are lit directly from the South at a 45 degree angle. With Vinifera, this can be customized.

In `RULES.INI`:
```ini
[AudioVisual]
VoxelLightAzimuth=0     ; float, the horizontal direction of the light source,
                        ; rotating from the South (bottom-left) counter-clockwise, in degrees.
VoxelLightElevation=45  ; float, the vertical angle of the light source (how high up is the Sun),
                        ; rotating from the South (bottom-left) counter-clockwise (towards North, passing above the unit), in degrees.
VoxelShadowOffset=6     ; float, how much the shadow is offset from the unit.
```

## Tiberium Storage

- By default, you need storage (in the form of buildings with `Storage=`) to collect Tiberium. Vinifera allows optionally disabling it, depositing Tiberium directly as credits (like in Red Alert 2).

In `RULES.INI`:
```ini
[General]
TiberiumStorage=yes  ; boolean, does the player need storage (silos) to collect Tiberium?
```

## Water Movement Zone

- Red Alert 2 adds a few new movement zones, among them is the `Water` movement zone, used for ships.

- Unfortunately, it is not trivial to add new movement zones to Tiberian Sun. However, Vinifera allows overriding any of the existing movement zones with a copy of the RA2 `Water` movement zone.

```ini
[General]
WaterMovementZoneOverride=  ; MZoneType, the name of the movement zone which will be replaced with Water.
```

```{note}
Movement zone `Normal` cannot be overridden this way.
```

```{note}
Once a movement zone is replaced with water, it cannot be reverted.
```

- Introducing a `Water` movement zone comes with a nuance. By default, lands `Water` and `Beach` are considered water-passable. This allows amphibious units to path through water and beaches, while preventing land units from doing so. However, introducing a `Water` movement zone makes ships attempt to path over `Beach`, where they get stuck because their `Speed` typically disallows moving onto beaches.

- Unfortunately, it is not currently possible to fix this neatly. However, as a workaround, Vinifera allows marking beaches as "requiring crushing" for passability purposes. This will allow movement zones `AmphibiousDestroyer` and `AmphibiousCrusher` to move over beaches, while preventing `Amphibious`, as well as the `Water` override from doing so.

```ini
[General]
BeachIsCrush=  ; boolean, are beaches considered as requiring crushing for pathfinding purposes?
```

## Building Catching on Fire Timeout

- When a building enters a damaged state, it spawns flame animations that may deal damage. If the building rapidly switches between damage states, it may end up spawning many instance of flame animations, taking large amount of damage. You can now specify a timeout during which buildings do not get flames spawned on them on damage state change after once catching fire.

```ini
[CombatDamage]
BuildingFlameSpawnBlockFrames=  ; integer, for how many frames buildings do not get flames spawned on them on damage state change after once catching fire.
```

## Pause Building Repairs
- Vinifera changes building repairs to pause rather than outright stop when the player doesn't have enough funds to continue repairs. This behaviour can optionally be reverted to the vanilla stopping behaviour by each player depending on their preferences.

In `SUN.INI`:
```ini
[Options]
PauseRepairs=yes  ; boolean, whether buildings pause repairs when the player doesn't have enough funds to complete the repairs.
```

- While repairs are paused, the game draws a specific frame of the wrench shape (`WRENCH.SHP`) on the building. This can be customized in order to draw a different frame.

In `RULES.INI`:
```ini
[General]
PausedRepairsFrame=6  ; integer, the frame index on the wrench shape to show while building repairs are paused.
```

## Area Guard Escort Logic Improvements
- Vinifera allows modders to specify the range in which an area-guarding unit that is assigned to guard another unit will follow it, as well as the range where it will abandon targets it is currently attacking (or healing) and go back to their assigned unit.
- This only applies on Area Guards on a unit; Area Guards on a cell does not count.
- Can be specified globally or for each unit individually. When both are applied, unit-specific values are used over the global values. 
- When those values are not stated or are non-positive, Vinifera falls back to the original behavior, which causes area-guarding units to follow their assigned unit once it leaves 2x the area-guarding unit's Guard Range (up to a maximum of 12 cells).

in `RULES.INI`:
```ini
[General]
EscortRange=-1  			 ; integer, the range in cells that an area guarding unit assigned to guard a unit will wait before closing the distance to its assigned unit while not engaging another unit.
AbandonTargetEscortRange=-1  ; integer, the range in cells that an area guarding unit assigned to guard a unit will keep engaging targets before abandoning the targets and go back to their assigned unit.

[SOMETECHNO]
EscortRange=-1  			 ; integer, the range in cells that an area guarding unit assigned to guard a unit will wait before closing the distance to its assigned unit while not engaging another unit.
AbandonTargetEscortRange=-1  ; integer, the range in cells that an area guarding unit assigned to guard a unit will keep engaging targets before abandoning the targets and go back to their assigned unit.
```

```{note}
To achieve good results with `AbandonTargetEscortRange`, it is recommended to set a value that is higher than 2x its `GuardRange`. Otherwise, the unit will keep re-acquiring and abdndoning it repeatedly as long as it is in range.
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

- You can enable the developer mode by running Vinifera (`LaunchVinifera.exe`) with the command line argument `-DEVELOPER`. You can also explicitly enable the debug console with `-CONSOLE`.

- `-NO_VERSION_STRING` can be used to hide the build version number from the in-game view.

- `-AUDIO_DEBUG` enables extensive logging for the new audio engine. In debug builds, it also opens the audio debug window.

```{note}
If you are using Vinifera with the TS Client, you can add these to the `ExtraCommandLineParams=` in `Resources\ClientDefinitions.ini`

**Do not release your project/mod with either of these arguments defined, they will break multiplayer!**
```

- The game will no longer fail to start if the startup mix files are not found.
```{note}
It is not recommended that you modify or remove any of the original mix files, this could lead to undefined behaviour and the Vinifera developers will not be able to troubleshoot any issue related to these changes.
```

- The game will no longer fail if the side specific mix files are not found.

### Rule Selection

- Vinifera re-enables a feature from Tiberian Sun's development which allows you to select the Rules file if multiple are found in the game directory.

- At startup, the game will scan for files in the root directory with the format **`RULE*.INI`**, with `*` being a wildcard for any character or string (for example, `RULE_VERSION1.INI` or `RULE_TEST_WEAPON.INI`). While it has no impact on the loading, it is advised to use the format `RULE_*.INI` to help with file sorting in Windows Explorer.

- The name shown in the dialog is the value of the `Name=` _(with a limit of 128 characters)_ entry under `[General]` in the Rule INI. If the user presses Cancel on the dialog, the game will load the standard `RULES.INI` file.

```{note}
Due to the nature of its use, this feature is only available when Vinifera is running in Developer Mode.
```

![image](https://user-images.githubusercontent.com/73803386/137135038-0a1e983f-d295-4723-86fb-1ab94ba8948b.png)

### NULL house warning

- Vinifera adds a warning to the debug log output when a null house pointer is detected during the game loading screen.
- Also, if the user has Developer Mode enabled, then a dialog will be shown to notify the user of the offending objects name.

![image](https://user-images.githubusercontent.com/73803386/129727642-8e929cc7-1b1c-4231-be44-abe4312ea265.png)

### Command Line Options

- Vinifera adds a number of command-line arguments.

- `-SPAWN`
Launch the game in spawner mode.

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

#### `[ ]` Memory Dump

- Produces a mini-dump of the memory for analysis.

#### `[ ]` Dump Heap CRCs

- Dumps all the current game objects as CRCs to the log output.

#### `[ ]` Dump Heaps

- Dumps all the type heaps to an output log.

#### `[ ]` Dump Trigger Info

- Dumps all existing triggers, tags, and local and global variables to the log output.

#### `[ ]` Reload Rules

- Reloads the Rules and Art INI files.

```{warning}
This could very well crash the game, please use it with caution and make small incremental changes only!
```

#### `[ ]` Instant Build (Player)

- Toggles the instant build cheat for the player.

#### `[ ]` Instant Build (AI)

- Toggles the instant build cheat for the AI.

#### `[ ]` Instant Special Recharge (Player)

- Toggles the instant recharge cheat for the players super weapons.

#### `[ ]` Instant Special Recharge (AI)

- Toggles the instant recharge cheat for the AI player super weapons.

#### `[ ]` Place Infantry

- Places a random infantry at the mouse cell.

#### `[ ]` Place Unit

- Places a random unit at the mouse cell.

#### `[ ]` Place Tiberium

- Places tiberium at the mouse cell.

#### `[ ]` Reduce Tiberium

- Reduces tiberium at the mouse cell.

#### `[ ]` Place Fully Grown Tiberium

- Places fully grown tiberium at the mouse cell.

#### `[ ]` Remove Tiberium

- Removes tiberium at the mouse cell.

#### `[ ]` Toggle AI Control

- Toggles AI control of the player house.

#### `[ ]` Toggle Frame Step

- Toggle frame step mode to step through the game frame-by-frame (for inspection).

#### `[ ]` Step 1 Frame

- Frame Step Only: Step forward 1 frame.

#### `[ ]` Step 5 Frame

- Frame Step Only: Step forward 5 frames.

#### `[ ]` Step 10 Frame

- Frame Step Only: Step forward 10 frame.

#### `[ ]` Build Cheat

- Unlocks all available build options for the player house.

#### `[ ]` Toggle Elite

- Toggle the elite status of the selected objects.

#### `[ ]` Damage

- Apply damage to all selected objects.

#### `[ ]` Spawn All

- Spawn all buildable units and structures at mouse cursor location.

#### `[ ]` Delete Object

- Removes the selected object(s) from the game world.

#### `[ ]` Map Snapshot

- Saves a snapshot of the current scenario state (Saved as 'SCEN_<date-time>.MAP.).

#### `[ ]` Ion Storm

- Toggles the ion storm on/off.

#### `[ ]` Bail Out

- Exits the game to the desktop.

```{note}
`Explosion` and `Super Explosion` are currently disabled due to a possible engine bug.
```

#### `[ ]` Explosion

- Spawns an explosion at the mouse cursor location.

#### `[ ]` Super Explosion

- Spawns a large explosion at the mouse cursor location.

#### `[ ]` Ion Blast

- Fires an ion blast bolt at the current mouse cursor location.

#### `[ ]` Lightning Bolt

- Fires a lightning bolt at the current mouse cursor location.

#### `[ ]` Free Money

- Hands out free money to the player.

#### `[ ]` Special Weapons

- Grants all available special weapons to the player.

#### `[ ]` Capture Object

- Take ownership of any selected objects.

#### `[ ]` Force Win

- Forces the player to win the current game session.

#### `[ ]` Force Lose

- Forces the player to lose the current game session.

#### `[ ]` Force Die

- Forces all of the player's units and structures to explode, losing the current game session.

#### `[ ]` Instant Build

- Toggles the instant build cheat for the player.

#### `[ ]` Instant Build (AI)

- Toggles the instant build cheat for the AI.

#### `[ ]` Toggle Shroud

- Toggles the visibility of the map shroud.

#### `[ ]` Heal

- Heal the selected objects by 50 hit points.

#### `[ ]` Toggle Inert

- Toggles if weapons are inert or not.

#### `[ ]` Dump AI Base Nodes

- Dumps all the current AI house base node info to the log output.

#### `[ ]` Toggle Alliance

- Toggles alliance with the selected objects house.

#### `[ ]` Encroach Fog

- Increase the fog of war by one step (cell).

#### `[ ]` Encroach Shadow

- Increase the shroud darkness by one step (cell).

#### `[ ]` Toggle Berzerk

- Toggles the berzerk state of the selected infantry.

#### `[ ]` Place Crate

- Places a random crate at the mouse location.

#### `[ ]` Add Power

- Adds 2000 power units to the player.

#### `[ ]` Cursor Position

- Displays cell coordinates of the mouse cursor.

#### `[ ]` Cycle Starting Waypoints

- Cycle the camera between the starting waypoints on the map.

#### `[ ]` Meteor Impact

- Sends a meteor at the current mouse cell.

#### `[ ]` Meteor Shower

- Creates a meteor shower around the current mouse cell.

## INI

- Add loading of `MPLAYER.INI` and `MPLAYERFS.INI` (Firestorm only) to override Rules data for multiplayer games (including Skirmish). Data contained in these INI's will not be loaded for the campaign and World Domination Tour games.
