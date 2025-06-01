# Mapping

This page describes all mapping-related additions and changes introduced by Vinifera.

## Bugfixes and Miscellaneous

- The game now supports reading and using up to 32767 waypoints in scenarios.
- Tutorial messages are now loaded from scenarios. This can be used to replace/update an existing entry from `TUTORIAL.INI`, or to add a new tutorial message index which can be used by trigger actions.
- Remove a hardcoded limitation where the remap color of `Neutral` and `Special` could not be overridden in multiplayer games. Due to the inconsistencies between the official maps, values of `Grey` and `LightGrey` will be forced to `LightGrey`.
- `[Basic]->SkipScore` is now considered when showing the multiplayer score screen. Setting to `SkipScore=yes` in the map file will now be all that is required for skip the score screen.
- Maps can now contain OverlayTypes with indices up to 65535. To enable this, set `[Basic]->NewINIFormat=5` in the scenario file. Note that such maps are not backwards compatible with vanilla, and loading them 

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
