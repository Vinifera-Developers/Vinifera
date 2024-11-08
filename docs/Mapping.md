# Mapping

This page describes all mapping-related additions and changes introduced by Vinifera.

## Bugfixes and Miscellaneous

- The game now supports reading and using up to 32767 waypoints in scenarios.
- Tutorial messages are now loaded from scenarios. This can be used to replace/update an existing entry from `TUTORIAL.INI`, or to add a new tutorial message index which can be used by trigger actions.
- Remove a hardcoded limitation where the remap color of `Neutral` and `Special` could not be overridden in multiplayer games. Due to the inconsistencies between the official maps, values of `Grey` and `LightGrey` will be forced to `LightGrey`.
- `[Basic]->SkipScore` is now considered when showing the multiplayer score screen. Setting to `SkipScore=yes` in the map file will now be all that is required for skip the score screen.

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

- The scenario file can now specify which loading screen to use.

In a scenario file:
```ini
[Basic]
LoadingScreen400=         ; string, the name of the loading screen to use with this resolution.
LoadingScreen480=         ; string, the name of the loading screen to use with this resolution.
LoadingScreen600=         ; string, the name of the loading screen to use with this resolution.
LoadingScreen400TextPos=  ; Point2D, a custom offset for the loading screen text and bars. 
LoadingScreen480TextPos=  ; Point2D, a custom offset for the loading screen text and bars. 
LoadingScreen600TextPos=  ; Point2D, a custom offset for the loading screen text and bars. 
```

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
ScorePlayerColor=253,181,28  ; color in R,G,B, color of the player's score bars
ScoreEnemyColor=250,28,28    ; color in R,G,B, color of the enemy's score bars
```

![Score screen colors in DTA:CR](https://github.com/user-attachments/assets/bc901430-abfc-4b8e-9648-107d07b7eafe)

## Script Actions

## Trigger Actions

### `106` Give Credits

- Give `P3` credits to House `P2`.

### `107` Enable Short Game

- Enable Short Game.

### `108` Disable Short Game

- Disable Short Game.

### `109` Reserved

- Does nothing.

### `110` Blow Up House

- Blow up all units and structures of House `P2`.

### `111` Make Elite

- Make all attached objects elite.

### `112` Enable AllyReveal

- Enable `AllyReveal`.

### `113` Disable AllyReveal

- Disable `AllyReveal`.

### `114` Create Auto-Save

- Schedule the creation of an auto-save at the end of this frame. Works in MP and SP.

### `115` Delete Object

- Silently delete all attached objects from the map.

### `116` Assign Mission to All

- Assign Mission `P2` to all attached objects.
