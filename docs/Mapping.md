# Mapping

This page describes all mapping-related additions and changes introduced by Vinifera.

## Bugfixes and Miscellaneous

- The game now supports reading and using up to 32767 waypoints in scenarios.
- Tutorial messages are now loaded from scenarios. This can be used to replace/update an existing entry from `TUTORIAL.INI`, or to add a new tutorial message index which can be used by trigger actions.
- Remove a hardcoded limitation where the remap color of `Neutral` and `Special` could not be overridden in multiplayer games. Due to the inconsistencies between the official maps, values of `Grey` and `LightGrey` will be forced to `LightGrey`.
- `[Basic]->SkipScore` is now considered when showing the multiplayer score screen. Setting to `SkipScore=yes` in the map file will now be all that is required for skip the score screen.

## Increased Overlay Limit

- Maps can now contain OverlayTypes with indices up to 65535.

- To enable this, set `[Basic]->NewINIFormat=5` in the scenario file.

```{note}
Maps using this feature cannot be loaded by the vanilla game.
```

```{warning}
Not all tools properly support this feature yet, and may crash or corrupt the map. We recommend using the [World-Altering Editor](https://github.com/CnCNet/WorldAlteringEditor) map editor when using this feature.
```

## Local/Global Variabes

- The game now supports up to 500 local and global variables each.

- Additionally, variables are now signed 32-bit integer numbers, allowing for greater flexibility during scripting. To make use of this feature, use new actions/events. Vanilla actions/events will treat 0 as false, and any other number as true.

## Campaign Settings

### Intro Movie

- `IntroMovie` can now be set for campaigns, allowing the customisation of the intro movie that plays before the campaign path starts.

In `BATTLE.INI`:
```ini
[Campaign]
IntroMovie=<none>  ; string, the intro movie name (without the .VQA extension) to play at the start of the campaign.
```

### DebugOnly

- `DebugOnly` can now be set for campaigns, which adds the prefix of "[Debug]" to the campaign description. In addition to this, it also makes the campaign only available Developer mode.

In `BATTLE.INI`:
```ini
[Campaign]
DebugOnly=no  ; boolean, is this campaign only available in Developer mode?
```
For testing/debugging versions of the Tiberian Sun and Firestorm campaigns, download [BATTLE_DEBUG_CAMPAIGN.INI](https://github.com/Vinifera-Developers/Vinifera-Files/blob/master/files/BATTLE_DEBUG_CAMPAIGN.INI) and place it in your game install directory.

## Scenario Settings

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

## Pre-placed units

- Pre-placed units could not have missions in multiplayer maps, regardless of who they belonged to. Vinifera lifts this limitation.

## Script Actions

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

- A trigger action is parsed from the map as follows:

```ini
[Actions]
NAME = [Action Count], [TActionType], [NeedCode], [PARAM1], [PARAM2], [PARAM3], [PARAM4], [PARAM5], [PARAM6:OPTIONAL]
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
| 13       | Bitwise AND        | x &= y         |
| 14       | Maximum            | x = max(x, y)  |
| 15       | Minimum            | x = min(x, y)  |

### New Trigger Actions

|  **ID**  | **Action**               | **NeedCode** | **PARAM1**       | **PARAM2** | **PARAM3** | **PARAM4** | **PARAM5** | **PARAM6** |
|----------|--------------------------|--------------|------------------|------------|------------|------------|------------|------------|
| 106      | Give Credits             |
|          | Gives or removes credits from the specified house. A positive amount gives money, a negative amount subtracts it. | Other (0)   | House (#)        | Credits    | *unused*   | *unused*   | *unused*   | *unused*   |
| 107      | Enable Short Game        |
|          | Enables Short Game. Players will lose if all buildings are destroyed. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 108      | Disable Short Game       |
|          | Disables Short Game. Players can continue playing even after all buildings are destroyed. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 109      | Unused Action            |
|          | This action does nothing. Originally used to display the difficulty in ts-patches. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 110      | Destroy all of...       |
|          | Kills everything of the specified house and marks them as defeated. | Other (0)   | House (#)        | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 111      | Make Elite               |
|          | All utechnos attached to this trigger will be promoted to elite status. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 112      | Enable Ally Reveal       |
|          | Enables Ally Reveal, allowing allied players to see each other's explored areas. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 113      | Disable Ally Reveal      |
|          | Disables Ally Reveal, stopping allied players from seeing each other's explored areas. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 114      | Create Autosave          |
|          | Schedules an autosave to be created on the next game frame. (Currently not implemented, handled by ts-patches) | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
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
| NeedFourArgs   | 5                 | Four arguments: PARAM1, PARAM2, PARAM3, PARAM4 parsed as numbers|
| NeedFiveArgs   | 6                 | Five arguments: PARAM1, PARAM2, PARAM3, PARAM4, PARAM5 parsed as numbers|

```{note}
Do not specigy extra arguments for trigger actions that don't require them!
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


### New Trigger Events

| **Code** | **Action**                | **NeedCode** | **PARAM1**       | **PARAM2**        | **PARAM3**              | **PARAM4**           | **PARAM5**                |
|----------|---------------------------|--------------|------------------|-------------------|-------------------------|----------------------|---------------------------|
| 56       | Compare variable (constant) | 
|          | Compares the value of a variable with a constant value. | NeedFourArgs (5) | Variable (#) | Is Global (0/1)          | Number     | Comparison Type            |
| 57       | Compare variable (variable) | 
|          | Compares the value of a variable with the value of another variable. | NeedFiveArgs (6) | Variable (#) | Is Global (0/1)          | Second Variable (#) | Is Second Global (0/1)  | Comparison Type    |
