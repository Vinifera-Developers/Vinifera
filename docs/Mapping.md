# Mapping

This page describes all mapping-related additions and changes introduced by Vinifera.

## Bugfixes and Miscellaneous

- The game now supports reading and using up to 32767 waypoints in scenarios.
- Tutorial messages are now loaded from scenarios. This can be used to replace/update an existing entry from `TUTORIAL.INI`, or to add a new tutorial message index which can be used by trigger actions.
- Remove a hardcoded limitation where the remap color of `Neutral` and `Special` could not be overridden in multiplayer games. Due to the inconsistencies between the official maps, values of `Grey` and `LightGrey` will be forced to `LightGrey`.
- `[Basic]->SkipScore` is now considered when showing the multiplayer score screen. Setting to `SkipScore=yes` in the map file will now be all that is required for skip the score screen.
- The game now supports using a negative value (such as `-1`) in the Center Camera At Waypoint trigger action in order to snap the camera position to the waypoint instead of scrolling to it over time.

## Increased Overlay Limit

- Maps can now contain OverlayTypes with indices up to 65535.

- To enable this, set `[Basic]->NewINIFormat=5` in the scenario file.

```{note}
Maps using this feature cannot be loaded by the vanilla game.
```

```{warning}
Not all tools properly support this feature yet, and may crash or corrupt the map. We recommend using the [World-Altering Editor](https://github.com/CnCNet/WorldAlteringEditor) map editor when using this feature.
```

## Local/Global Variables

- The game now supports up to 500 local and global variables each.

- Additionally, variables are now signed 32-bit integer numbers, allowing for greater flexibility during scripting. To make use of this feature, use new actions/events. Vanilla actions/events will treat 0 as false, and any other number as true.

## Campaign Settings

### Campaign Side

- `Side` can now be set for campaigns, allowing the customisation of which **HOUSE**'s loading screens this campaign should use.

In `BATTLE.INI`:
```ini
[SOMECAMPAIGN]  ; Campaign
Side=0          ; integer, the index of the house whose loading screens will be used for this campaign.
```

```{note}
To preserve compatibility, the campaign's `Side` defaults to `0` if its scenario names contains `GDI`, to `1` if it contains `NOD`, and to 0 otherwise.
```

```{note}
This setting only affects the loading screen graphics used.
```

### Intro Movie

- `IntroMovie` can now be set for campaigns, allowing the customisation of the intro movie that plays before the campaign path starts.

In `BATTLE.INI`:
```ini
[SOMECAMPAIGN]  ; Campaign
IntroMovie=     ; string, the intro movie name (without the .VQA extension) to play at the start of the campaign.
```

### DebugOnly

- `DebugOnly` can now be set for campaigns, which adds the prefix of "[Debug]" to the campaign description. In addition to this, it also makes the campaign only available Developer mode.

In `BATTLE.INI`:
```ini
[SOMECAMPAIGN]  ; Campaign
DebugOnly=no    ; boolean, is this campaign only available in Developer mode?
```
For testing/debugging versions of the Tiberian Sun and Firestorm campaigns, download [BATTLE_DEBUG_CAMPAIGN.INI](https://github.com/Vinifera-Developers/Vinifera-Files/blob/master/files/BATTLE_DEBUG_CAMPAIGN.INI) and place it in your game install directory.

## Scenario Settings

### AI Base Nodes in Skirmish/Multiplayer

- Vinifera allows enabling base nodes for the AI outside of campaigns.

In a scenario file:
```ini
[Basic]
UseMPAIBaseNodes=no         ; boolean, should the AI use base nodes for base construction, like in campaign?
```

### Custom Loading Screen

- A campaign scenario can provide house-specific loading-screen overrides using the same list format as `UI.INI`.
- Each value has the format `<HouseType index>,<filename without .PCX>,<layout width>,<layout height>[,<singleplayer X>,<singleplayer Y>,<multiplayer X>,<multiplayer Y>]`. The layout dimensions are the image-fit threshold and the reference canvas used to center it.
- For the active house, Vinifera chooses randomly among entries with the greatest pixel area that fit the current resolution.

In a scenario file:
```ini
[LoadingScreens]
0=0,MYGDI400,640,400
1=0,MYGDI600,800,600,546,233,711,227
2=1,MYNOD400,640,400
```

The numeric keys only establish list order and must be unique. Position fields are optional; omitting them uses the built-in positions for 640x400, 640x480, or 800x600. Both coordinates for a mode must be positive; if either is zero or negative, that mode uses the built-in position. Loading-screen names in this section must not include the `.PCX` extension.

### Ice Destruction

- Ice destruction can now be disabled.

In a scenario file:
```ini
[Basic]
IceDestructionEnabled=yes  ; boolean, can ice tiles be destroyed in the scenario?
```

### Score Screen Bar Color Customization

- You can now customize colors of the score screen casualty bars.

In a scenario file:
```ini
[Basic]
ScorePlayerColor=253,181,28  ; color in R,G,B, color of the player's score bars.
ScoreEnemyColor=250,28,28    ; color in R,G,B, color of the enemy's score bars.
```

![Score screen colors in DTA:CR](https://github.com/user-attachments/assets/bc901430-abfc-4b8e-9648-107d07b7eafe)

### Free Radar
- Free Radar can now be made to work even while the player's house is low power. The scenario must enable Free Radar (`FreeRadar=yes`) for this to take effect.
- Note that Ion Storms will still cause radars to be turned off for their duration.

In `RULES.INI` or a scenario file:
```ini
[General]
FreeRadarOnLowPower=no ; boolean, can free radar stay active when player is in low power?
```

### Tag Persistence on AI-controlled units
- AI-controlled units can now be set to persist their tags when they deploy into a building. This allows them to keep the tag with their original team members.
- Note that Human-controlled units always persist their tags, regardless of this setting.

In `RULES.INI`:
```ini
[General]
PersistTagsOnAIDeploy=no  ; boolean, do tags persist on AI units that deploy?
```

## Pre-placed units

- Pre-placed units could not have missions in multiplayer maps, regardless of who they belonged to. Vinifera lifts this limitation.

## Script Actions

### New Quarry Types

- Script actions accepting a quarry as a parameter now take new quarry values.

| Index | Name | Description |
|-|-|-|
|10|Only Harvesters|The team will target only harvesters (not refineries)|

## Trigger Actions

### NeedCodes

- Every trigger action has a NeedCode associated with it. The NeedCode dictates how some of the data used by the trigger action is parsed. Below is a table containing all valid NeedCodes.

|   **NeedCode**   | **Numeric Value** |   **Meaning**                                                      |
|-----------------:|:-----------------:|:-------------------------------------------------------------------|
| NeedOther        | 0                 | PARAM1 is parsed as a number, PARAM6 is parsed as a waypoint       |
| NeedTeam         | 1                 | PARAM1 is parsed as a team name, PARAM6 is parsed as a waypoint    |
| NeedTrigger      | 2                 | PARAM1 is parsed as a trigger name, PARAM6 is parsed as a waypoint |
| NeedTag          | 3                 | PARAM1 is parsed as a tag name, PARAM6 is parsed as a waypoint     |
| NeedTeamAndTime  | 4                 | PARAM1 is parsed as a team name, PARAM6 is parsed as a number      |
| NeedSpeech       | 5                 | PARAM1 is parsed as a speech name, PARAM6 is parsed as a waypoint  |
| NeedSound        | 6                 | PARAM1 is parsed as a sound name, PARAM6 is parsed as a waypoint   |
| NeedTheme        | 7                 | PARAM1 is parsed as a theme name, PARAM6 is parsed as a waypoint   |

- A trigger action is parsed from the map as follows:

```ini
[Actions]
NAME = [Action Count], [TActionType], [NeedCode], [PARAM1], [PARAM2], [PARAM3], [PARAM4], [PARAM5], [PARAM6:OPTIONAL]
```

```{note}
Any action that takes a `VocType` (sound), `VoxType` (speech/EVA), or `ThemeType` (music), including vanilla actions, can be used with need code 0 (`NeedOther`), specifying the ordinal number of the relevant type as `PARAM1`, or with need codes 5, 6, or 7 (`NeedSpeech`, `NeedSound`, `NeedTheme` respectively), specifying the INI name of the type.
```

### Operation Types

- Actions that operate on variables use the following operation types:

| **Code** | **Operation Name** | **Example**    |
|----------|--------------------|----------------|
| 0        | Assign             | x = y          |
| 1        | Add                | x += y         |
| 2        | Subtract           | x -= y         |
| 3        | Multiply           | x *= y         |
| 4        | Divide             | x /= y         |
| 5        | Modulo             | x %= y         |
| 6        | Negate             | x = -x         |
| 7        | Shift Left         | x <<= y        |
| 8        | Shift Right        | x >>= y        |
| 9        | Bitwise NOT        | x = ~x         |
| 10       | Bitwise XOR        | x ^= y         |
| 11       | Bitwise OR         | x \|= y        |
| 12       | Bitwise AND        | x &= y         |
| 13       | Maximum            | x = max(x, y)  |
| 14       | Minimum            | x = min(x, y)  |

### Enhanced Trigger Actions

Vinifera enhances a few existing trigger actions to allow them to be customizable for mappers.

|  **ID**  | **Action**               | **NeedCode** | **PARAM1**       | **PARAM2** | **PARAM3** | **PARAM4** | **PARAM5** | **PARAM6** | **PARAM7** |
|----------|--------------------------|--------------|------------------|------------|------------|------------|------------|------------|------------|
| 11       | Text Trigger     |
|          | Displays a text message with optional color and duration. Supports templated text substitution: placeholders like `{{g_variableName}}` or `{{l_variableName}}` are replaced with the corresponding global or local variable values. Duration is in real time seconds (0 means like in vanilla). | Other (0) | Text ID (string)     | Color (#) | Duration  | *unused*   | *unused*   | *unused*   | *unused* |
| 17       | Reveal Around Waypoint     |
|          | Reveals a region of the map to the player around the waypoint specified. | Other (0) | *unused*     | Waypoint (#) | Reveal Radius (number)  | Ignore Elevation (boolean)   | *unused* | *unused* | *unused*
| 45       | Center Camera on Waypoint     |
|          | Moves the tactical view to a specified waypoint with the given speed (-1 to 4). | Other (0) | *unused*     | Number (-1 - 4) | Reveal Radius (number)  | Ignore Elevation (boolean)   | *unused* | *unused* | Waypoint (#)

#### [11] Text Trigger

```{warning}
Trigger action 11 `Text Trigger` now takes a string key for the tutorial text entry, not an integer!
```

Text Trigger trigger action now comes with 2 new parameters that can be set by mappers:
* Text: the text index of `[Tutorial]` in `Tutorial.ini` that should be shown. If the text includes `{{g_variableName}}`, its value will be replaced with the value of the provided global variable. If the text includes `{{l_variableName}}`, its value will be replaced with the value of the provided local variable.
* Color: the color that the text should appear with. The color is picked based on the index of `[Colors]` in `Rules.ini`. If set to 0, uses the default message color.
* Duration: the duration, in real-time seconds, that the text should stay on the screen. If set to 0, uses the default message duration.


#### [17] Reveal Around Waypoint

Reveal Around Waypoint trigger action now comes with 2 new parameters that can be set by mappers: 
* Reveal Radius: specify the radius to reveal in this instance of the trigger action. If set to 0 or a negative value, falls back to the value specified in `RevealTriggerRadius` in `Rules.ini`.
* Ignore Elevation: specifies whether the reveal should ignore elevation. When elevation is taken into account, cells that are higher than 3 elevations will not be revealed. Possible values: 0 = No (default), any other value = Yes. 
  * Requires `RevealByHeight=yes` (or omitted) for elevation to be taken into account.

#### [45] Center Camera at Waypoint

Center Camera at Waypoint action now supports the -1 for the camera scroll rate, which snaps the camera instantly to the target position.

### New Trigger Actions


|  **ID**  | **Action**               | **NeedCode** | **PARAM1**       | **PARAM2** | **PARAM3** | **PARAM4** | **PARAM5** | **PARAM6** |
|----------|--------------------------|--------------|------------------|------------|------------|------------|------------|------------|
| 11       | Text Trigger (Enhanced)     |
|          | Displays a text message with optional color and duration. Supports templated text substitution: placeholders like `{{g_variableName}}` or `{{l_variableName}}` are replaced with the corresponding global or local variable values. Duration is in real time seconds (0 means like in vanilla). | Other (0) | Text ID (string)     | Color (#) | Duration  | *unused*   | *unused*   | *unused*   |
| 17       | Reveal Around Waypoint (Enhanced)     |
|          | Reveals a region of the map to the player around the waypoint specified. | Other (0) | *unused*     | Waypoint (#) | Reveal Radius (number)  | Ignore Elevation (boolean)   | *unused*   | *unused*   |
| 106      | Give Credits             |
|          | Gives or removes credits from the specified house. A positive amount gives money, a negative amount subtracts it. | Other (0)   | House (#)        | Credits    | *unused*   | *unused*   | *unused*   | *unused*   |
| 107      | Enable Short Game        |
|          | Enables Short Game. Players will lose if all buildings are destroyed. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 108      | Disable Short Game       |
|          | Disables Short Game. Players can continue playing even after all buildings are destroyed. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 109      | Create Building At       |
|          | Places a building at given waypoint position. | Other (0)   | HouseType (#)  | BuildingType (#)   | Boolean (force placement) | *unused*   | *unused*   | Waypoint   |
| 110      | Destroy all of...       |
|          | Kills everything of the specified house and marks them as defeated. | Other (0)   | House (#)        | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 111      | Make Elite               |
|          | All utechnos attached to this trigger will be promoted to elite status. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 112      | Enable Ally Reveal       |
|          | Enables Ally Reveal, allowing allied players to see each other's explored areas. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 113      | Disable Ally Reveal      |
|          | Disables Ally Reveal, stopping allied players from seeing each other's explored areas. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 114      | Create Autosave          |
|          | Schedules an autosave to be created on the next game frame. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 115      | Delete Attached Objects  |
|          | Deletes all units and structures on the map that are linked to this trigger silently. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 116      | All Assign Mission       |
|          | Forces all units owned by the trigger's house to begin the specified mission (e.g., hunt, move). | Other (0)   | Mission (#)   | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 117      | Make Ally (One-Way)      |
|          | Cause this trigger's house to make a one-sided alliance with the specified house. | Other (0)   | House (#)        | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 118      | Make Enemy (One-Way)     |
|          | Cause this trigger's house to unilaterally declare war on the specified house. | Other (0)   | House (#)        | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 119      | Modify Global (Constant)        |
|          | Modifies a global variable using a constant and a specified operation. | Other (0) | Global Variable (#) | Operation Type   | Number     | unused         | unused         | unused         |
| 120      | Modify Global (Global)          |
|          | Modifies a global variable using another global variable and a specified operation. | Other (0) | Global Variable (#) | Operation Type   | Second Global (#)  | *unused*         | *unused*         | *unused*         |
| 121      | Modify Global (Local)           |
|          | Modifies a global variable using a local variable and a specified operation. | Other (0) | Global Variable (#) | Operation Type   | Local Variable (#) | *unused*         | *unused*         | *unused*         |
| 122      | Increment Global                |
|          | Increments a global variable by 1. | Other (0) | Global Variable (#) | *unused*          | *unused*         | *unused*         | *unused*         | *unused*         |
| 123      | Decrement Global                |
|          | Decrements a global variable by 1. | Other (0) | Global Variable (#) | *unused*          | *unused*         | *unused*         | *unused*         | *unused*         |
| 124      | Modify Local (Constant)         |
|          | Modifies a local variable using a constant and a specified operation. | Other (0) | Local Variable (#)  | Operation Type   | Number     | *unused*         | *unused*         | *unused*         |
| 125      | Modify Local (Global)           |
|          | Modifies a local variable using a global variable and a specified operation. | Other (0) | Local Variable (#)  | Operation Type   | Global Variable (#) | *unused*        | *unused*         | *unused*         |
| 126      | Modify Local (Local)            |
|          | Modifies a local variable using another local variable and a specified operation. | Other (0) | Local Variable (#)  | Operation Type   | Second Local (#)   | *unused*         | *unused*         | *unused*         |
| 127      | Increment Local                 |
|          | Increments a local variable by 1. | Other (0) | Local Variable (#)  | *unused*          | *unused*         | *unused*         | *unused*         | *unused*         |
| 128      | Decrement Local                 |
|          | Decrements a local variable by 1. | Other (0) | Local Variable (#)  | *unused*          | *unused*         | *unused*         | *unused*         | *unused*         |
| 129      | Random Number to Global         |
|          | Stores a random number between Min and Max into a global variable. | Other (0) | Global Variable (#) | Min Value        | Max Value        | *unused*         | *unused*         | *unused*         |
| 130      | Random Number to Local          |
|          | Stores a random number between Min and Max into a local variable. | Other (0) | Local Variable (#)  | Min Value        | Max Value        | *unused*         | *unused*         | *unused*         |
| 131      | Print Global                    |
|          | Displays the current value of a global variable as a message. | Other (0) | Global Variable (#) | *unused*          | *unused*         | *unused*         | *unused*         | *unused*         |
| 132      | Print Local                     |
|          | Displays the current value of a local variable as a message. | Other (0) | Local Variable (#)  | *unused*          | *unused*         | *unused*         | *unused*         | *unused*         |
| 133      | Enable Templated Text     |
|          | Displays a line of text on the screen with variable substitution. The text may include placeholders like `{{g_variableName}}` or `{{l_variableName}}`, which are replaced with the corresponding global or local variable values. Color `-1` uses the color of the player's house. | Other (0) | Text Index (#)     | Color (#) | *unused*   | *unused*   | *unused*   | *unused*   |
| 134      | Disable Templated Text           |
|          | Removes the currently active templated text from the screen. | Other (0) | *unused*           | *unused*            | *unused*   | *unused*   | *unused*   | *unused*   |
| 135      | Adjust House Modifier           |
|          | Adjusts a house modifier by given percentage points. | Other (0) | Modifier (#)           | Amount (%)            | *unused*   | *unused*   | *unused*   | *unused*   |
| 136      | Apply Iron Curtain             |
|          | Applies Iron Curtain to attached objects. Can optionally bypass legality checks. | Other (0) | Boolean (skip legality check)           | *unused*            | *unused*   | *unused*   | *unused*   | *unused*   |
| 137      | Stop Sounds At             |
|          | Stops sounds started by Play Sound At at the specified waypoint. | Other (0) | *unused*           | *unused*            | *unused*   | *unused*   | *unused*   | Waypoint   |
| 138      | Attach Sound             |
|          | Attaches an ambient sound to all objects associated with the trigger. The VocType should have `Control=LOOP` for a continuous attachment; non-looping vocs play once and then go silent. | Sound (6) | VocType (name)           | *unused*            | *unused*   | *unused*   | *unused*   | Waypoint   |
| 139      | Detach Sound             |
|          | Detaches any ambient sound from all objects associated with the trigger. | Other (0) | *unused*           | *unused*            | *unused*   | *unused*   | *unused*   | Waypoint   |

### [135] Adjust House Modifier — Modifier Types

| Number | Modifier |
|---|--------|
| 0 | Firepower |
| 1 | Armor |
| 2 | Groundspeed |
| 3 | Airspeed |
| 4 | Rate of Fire |
| 5 | Cost |
| 6 | Build Time |

## Trigger Events

- Every trigger event has a NeedCode associated with it. The NeedCode dictates how some of the data used by the trigger event is parsed. Below is a table containing all valid NeedCodes.

### NeedCodes

| **NeedCode**   | **Numeric Value** | **Meaning / Parameters**                                         |
|----------------|-------------------|-----------------------------------------------------------------|
| NeedOther      | 0                 | Single argument: PARAM1 parsed as a number                      |
| NeedTeam       | 1                 | Single argument: PARAM1 parsed as a team name                   |
| NeedTechnoAndNumber    | 2                 | Two arguments: PARAM1 parsed as a number, PARAM2 parsed as an INI name                   |
| NeedTwoArgs    | 3                 | Two arguments: PARAM1, PARAM2 parsed as numbers               |
| NeedThreeArgs  | 4                 | Three arguments: PARAM1, PARAM2, PARAM3 parsed as numbers        |

```{note}
Do not specify extra arguments for trigger actions that don't require them!
```

### Comparison Types

- Conditions that compare values use the following comparison types:

| Code | Comparison Name     | Example         |
|------|---------------------|-----------------|
| 0    | Greater Than        | x > y           |
| 1    | Less Than           | x < y           |
| 2    | Equal To            | x == y          |
| 3    | Not Equal To        | x != y          |
| 4    | Greater or Equal    | x >= y          |
| 5    | Less or Equal       | x <= y          |
| 6    | Bitwise AND         | (x & y) != 0    |
| 7    | Bitwise OR          | (x \| y) != 0    |
| 8    | Bitwise XOR         | (x ^ y) != 0    |

### Comes Near Waypoint

- The distance for the "Comes Near Waypoint" trigger event was originally hardcoded to 5 cells. Vinifera allows customizing this distance.

In `RULES.INI`, or a scenario INI:
```ini
[General]
ComesNearWaypointDistance=1280   ; integer, defines how close, in leptons, an object needs to be to a waypoint for the "Comes Near Waypoint" event to be fired
```

### Entered By

- Now allows to conditionally make cloaked units be able to trigger Entered By trigger events by moving into cell tags. Disabled by default.

In `RULES.INI`, or a scenario INI:
```ini
[General]
CellTagsIgnoreStealth=yes ; boolean, whether cell tags ignore cloaked units moving into them. If set to no, cloaked units would trigger Entered By trigger events like regular units
```

### New Trigger Events

| **Code** | **Action**                                                                           | **NeedCode**      | **PARAM1**          | **PARAM2**          | **PARAM3**             |
| -------- | ------------------------------------------------------------------------------------ | ----------------- | ------------------- | ------------------- | ---------------------- |
| 56       | Compare Global with Constant                                                         |                   |                     |                     |                        |
|          | Compares a global variable with a constant using a specified operation.              | NeedThreeArgs (4) | Global Variable (#) | Comparison Type     | Constant (#)           |
| 57       | Compare Global with Global                                                           |                   |                     |                     |                        |
|          | Compares a global variable with another global variable using a specified operation. | NeedThreeArgs (4) | Global Variable (#) | Comparison Type     | Global Variable (#)    |
| 58       | Compare Global with Local                                                            |                   |                     |                     |                        |
|          | Compares a global variable with a local variable using a specified operation.        | NeedThreeArgs (4) | Global Variable (#) | Comparison Type     | Local Variable (#)     |
| 59       | Global Equals Constant                                                               |                   |                     |                     |                        |
|          | True if a global variable equals a constant.                                         | NeedTwoArgs (3)   | Global Variable (#) | Constant (#)        |                        |
| 60       | Global Equals Global                                                                 |                   |                     |                     |                        |
|          | True if two global variables are equal.                                              | NeedTwoArgs (3)   | Global Variable (#) | Global Variable (#) |                        |
| 61       | Global Equals Local                                                                  |                   |                     |                     |                        |
|          | True if a global variable equals a local variable.                                   | NeedTwoArgs (3)   | Global Variable (#) | Local Variable (#)  |                        |
| 62       | Global Greater Than Constant                                                         |                   |                     |                     |                        |
|          | True if a global variable is greater than a constant.                                | NeedTwoArgs (3)   | Global Variable (#) | Constant (#)        |                        |
| 63       | Global Greater Than Global                                                           |                   |                     |                     |                        |
|          | True if one global variable is greater than another.                                 | NeedTwoArgs (3)   | Global Variable (#) | Global Variable (#) |                        |
| 64       | Global Greater Than Local                                                            |                   |                     |                     |                        |
|          | True if a global variable is greater than a local variable.                          | NeedTwoArgs (3)   | Global Variable (#) | Local Variable (#)  |                        |
| 65       | Global Less Than Constant                                                            |                   |                     |                     |                        |
|          | True if a global variable is less than a constant.                                   | NeedTwoArgs (3)   | Global Variable (#) | Constant (#)        |                        |
| 66       | Global Less Than Global                                                              |                   |                     |                     |                        |
|          | True if one global variable is less than another.                                    | NeedTwoArgs (3)   | Global Variable (#) | Global Variable (#) |                        |
| 67       | Global Less Than Local                                                               |                   |                     |                     |                        |
|          | True if a global variable is less than a local variable.                             | NeedTwoArgs (3)   | Global Variable (#) | Local Variable (#)  |                        |
| 68       | Compare Local with Constant                                                          |                   |                     |                     |                        |
|          | Compares a local variable with a constant using a specified operation.               | NeedThreeArgs (4) | Local Variable (#)  | Comparison Type     | Constant (#)           |
| 69       | Compare Local with Global                                                            |                   |                     |                     |                        |
|          | Compares a local variable with a global variable using a specified operation.        | NeedThreeArgs (4) | Local Variable (#)  | Comparison Type     | Global Variable (#)    |
| 70       | Compare Local with Local                                                             |                   |                     |                     |                        |
|          | Compares a local variable with another local variable using a specified operation.   | NeedThreeArgs (4) | Local Variable (#)  | Comparison Type     | Local Variable (#)     |
| 71       | Local Equals Constant                                                                |                   |                     |                     |                        |
|          | True if a local variable equals a constant.                                          | NeedTwoArgs (3)   | Local Variable (#)  | Constant (#)        |                        |
| 72       | Local Equals Global                                                                  |                   |                     |                     |                        |
|          | True if a local variable equals a global variable.                                   | NeedTwoArgs (3)   | Local Variable (#)  | Global Variable (#) |                        |
| 73       | Local Equals Local                                                                   |                   |                     |                     |                        |
|          | True if two local variables are equal.                                               | NeedTwoArgs (3)   | Local Variable (#)  | Local Variable (#)  |                        |
| 74       | Local Greater Than Constant                                                          |                   |                     |                     |                        |
|          | True if a local variable is greater than a constant.                                 | NeedTwoArgs (3)   | Local Variable (#)  | Constant (#)        |                        |
| 75       | Local Greater Than Global                                                            |                   |                     |                     |                        |
|          | True if a local variable is greater than a global variable.                          | NeedTwoArgs (3)   | Local Variable (#)  | Global Variable (#) |                        |
| 76       | Local Greater Than Local                                                             |                   |                     |                     |                        |
|          | True if a local variable is greater than another local variable.                     | NeedTwoArgs (3)   | Local Variable (#)  | Local Variable (#)  |                        |
| 77       | Local Less Than Constant                                                             |                   |                     |                     |                        |
|          | True if a local variable is less than a constant.                                    | NeedTwoArgs (3)   | Local Variable (#)  | Constant (#)        |                        |
| 78       | Local Less Than Global                                                               |                   |                     |                     |                        |
|          | True if a local variable is less than a global variable.                             | NeedTwoArgs (3)   | Local Variable (#)  | Global Variable (#) |                        |
| 79       | Local Less Than Local                                                                |                   |                     |                     |                        |
|          | True if a local variable is less than another local variable.                        | NeedTwoArgs (3)   | Local Variable (#)  | Local Variable (#)  |                        |
| 80       | Buildings Does Not Exist                                                             |                   |                     |                     |                        |
|          | Triggers when a building owned by the trigger's house does not exist.                | NeedOther (0)     | BuildingType (#)    |                     |                        |
