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
| 6        | Shift Left         | x <<= y        |
| 7        | Shift Right        | x >>= y        |
| 8        | Bitwise NOT        | x = ~x         |
| 9        | Bitwise XOR        | x ^= y         |
| 10       | Bitwise OR         | x \|= y         |
| 11       | Bitwise AND        | x &= y         |
| 12       | Negate             | x = -x         |
| 13       | Maximum            | x = max(x, y)  |
| 14       | Minimum            | x = min(x, y)  |

### New Trigger Actions

| **Code** | **Action**               | **NeedCode** | **PARAM1**       | **PARAM2** | **PARAM3** | **PARAM4** | **PARAM5** | **PARAM6** |
|----------|--------------------------|--------------|------------------|------------|------------|------------|------------|------------|
| 501      | Give Credits             |
|          | Gives or removes credits from the specified house. A positive amount gives money, a negative amount subtracts it. | Other (0)   | House (#)        | Credits    | *unused*   | *unused*   | *unused*   | *unused*   |
| 502      | Enable Short Game        |
|          | Enables Short Game. Players will lose if all buildings are destroyed. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 503      | Disable Short Game       |
|          | Disables Short Game. Players can continue playing even after all buildings are destroyed. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 504      | Unused Action            |
|          | This action does nothing. Originally used to display the difficulty in ts-patches. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 505      | Destroy all of...       |
|          | Kills everything of the specified house and marks them as defeated. | Other (0)   | House (#)        | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 506      | Make Elite               |
|          | All utechnos attached to this trigger will be promoted to elite status. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 507      | Enable Ally Reveal       |
|          | Enables Ally Reveal, allowing allied players to see each other's explored areas. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 508      | Disable Ally Reveal      |
|          | Disables Ally Reveal, stopping allied players from seeing each other's explored areas. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 509      | Create Autosave          |
|          | Schedules an autosave to be created on the next game frame. (Currently not implemented, handled by ts-patches) | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 510      | Delete Attached Objects  |
|          | Deletes all units and structures on the map that are linked to this trigger silently. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 511      | All Assign Mission       |
|          | Forces all units owned by the trigger's house to begin the specified mission (e.g., hunt, move). | Other (0)   | Mission (#)   | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 512      | Make Ally (One-Way)      |
|          | Cause this trigger's house to make a one-sided alliance with the specified house. | Other (0)   | House (#)        | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 513      | Make Enemy (One-Way)     |
|          | Cause this trigger's house to unilaterally declare war on the specified house. | Other (0)   | House (#)        | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 514      | Modify Variable (Constant)  |
|          | Modifies a variable using a constant and a specified operation. | Other (0) | Variable (#)   | Operation Type   | Number   | Is Global (0/1)  | *unused*         | *unused* |
| 515      | Modify Variable (Variable)  |
|          | Modifies a variable using another variable and a specified operation. | Other (0) | Variable (#)   | Operation Type   | Second Variable (#) | Is Global (0/1)  | Is 2nd Global (0/1) | *unused* |
| 516      | Random Number to Variable   |
|          | Stores a random number between Min and Max into the variable. | Other (0) | Variable (#)   | Min Value        | Max Value        | Is Global (0/1)  | *unused*         | *unused* |
| 517      | Print Variable              |
|          | Displays the current value of a variable as a message. | Other (0) | Variable (#)   | Is Global (0/1)  | *unused*         | *unused*         | *unused*         | *unused* |

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


### New Trigger Events

| **Code** | **Action**                | **NeedCode** | **PARAM1**       | **PARAM2**        | **PARAM3**              | **PARAM4**           | **PARAM5**                |
|----------|---------------------------|--------------|------------------|-------------------|-------------------------|----------------------|---------------------------|
| 56       | Compare variable (constant) | 
|          | Compares the value of a variable with a constant value. | NeedFourArgs (5) | Variable (#) | Is Global (0/1)          | Number     | Comparison Type            |
| 57       | Compare variable (variable) | 
|          | Compares the value of a variable with the value of another variable. | NeedFiveArgs (6) | Variable (#) | Is Global (0/1)          | Second Variable (#) | Is Second Global (0/1)  | Comparison Type    |
