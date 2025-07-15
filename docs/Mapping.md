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

## Script Actions

## Trigger Actions

### NeedCodes

- Every trigger action has a NeedCode associated with it, parsed from the 3rd field of the action (P1). The NeedCode dictates how some of the data used by the trigger action is parsed. Below is a table containing all valid NeedCodes.

|    *NeedCode*    | *Numeric Value* |   Meaning                                                          |
|-----------------:|:---------------:|:-------------------------------------------------------------------|
| NeedOther        | 0               | PARAM1 is parsed as a number                                       |
| NeedTeam         | 1               | PARAM1 is parsed as a team name                                    |
| NeedTrigger      | 2               | PARAM1 is parsed as a trigger name                                 |
| NeedTag          | 3               | PARAM1 is parsed as a tag name                                     |
| NeedTeamAndTime  | 4               | PARAM1 is parsed as a team name, PARAM6 (P7) is parsed as a number |

- A trigger action is parsed from the map as follows:

```ini
[Actions]
NAME = [Action Count], [TActionType], [NeedCode], [PARAM1], [PARAM2], [PARAM3], [PARAM4], [PARAM5], [PARAM6:OPTIONAL]
```

### New Trigger Actions

| **Code** | **Action**               | **NeedCode** | **PARAM1**       | **PARAM2** | **PARAM3** | **PARAM4** | **PARAM5** | **PARAM6** |
|----------|--------------------------|--------------|------------------|------------|------------|------------|------------|------------|
| 501      | Give Credits             |              |                  |            |            |            |            |            |
|          | Gives or removes credits from the specified house. A positive amount gives money, a negative amount subtracts it. | Other (0)   | House (#)        | Credits    | *unused*   | *unused*   | *unused*   | *unused*   |
| 502      | Enable Short Game        |              |                  |            |            |            |            |            |
|          | Enables the Short Game mode. Players will lose if all buildings are destroyed. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 503      | Disable Short Game       |              |                  |            |            |            |            |            |
|          | Disables the Short Game mode. Players can continue playing even after all buildings are destroyed. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 504      | Unused Action            |              |                  |            |            |            |            |            |
|          | This action does nothing. Originally used to display the difficulty in ts-patches. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 505      | Blow Up House            |              |                  |            |            |            |            |            |
|          | Instantly destroys all buildings and units of the specified house and marks them as defeated. | Other (0)   | House (#)        | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 506      | Make Elite               |              |                  |            |            |            |            |            |
|          | All units and buildings attached to this trigger will be promoted to elite status. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 507      | Enable Ally Reveal       |              |                  |            |            |            |            |            |
|          | Enables the Ally Reveal feature, allowing allied players to see each other's explored areas. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 508      | Disable Ally Reveal      |              |                  |            |            |            |            |            |
|          | Disables the Ally Reveal feature, hiding the fog of war even between allies. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 509      | Create Autosave          |              |                  |            |            |            |            |            |
|          | Schedules an autosave to be created on the next game frame. (Currently unimplemented, handled by ts-patches) | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 510      | Delete Attached Objects  |              |                  |            |            |            |            |            |
|          | Deletes all units and structures on the map that are linked to this trigger silently. | Other (0)   | *unused*         | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
| 511      | All Assign Mission       |              |                  |            |            |            |            |            |
|          | Forces all units owned by the trigger's house to begin the specified mission (e.g., hunt, move). | Other (0)   | Mission (#)   | *unused*   | *unused*   | *unused*   | *unused*   | *unused*   |
