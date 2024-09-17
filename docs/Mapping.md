# Mapping

This page describes all mapping-related additions and changes introduced by Vinifera.

## Bugfixes and Miscellaneous

- The game now supports reading and using up to 32767 waypoints in scenarios.
- Tutorial messages are now loaded from scenarios. This can be used to replace/update an existing entry from `TUTORIAL.INI`, or to add a new tutorial message index which can be used by trigger actions.
- Remove a hardcoded limitation where the remap color of `Neutral` and `Special` could not be overridden in multiplayer games. Due to the inconsistencies between the official maps, values of `Grey` and `LightGrey` will be forced to `LightGrey`.
- `[Basic]->SkipScore` is now considered when showing the multiplayer score screen. Setting to `SkipScore=yes` in the map file will now be all that is required for skip the score screen.

## Campaign Settings

### Intro Movie

- `IntroMovie` can now be set for campaigns, allowing the customisation of the intro movie that plays before the campaign path starts.
In `BATTLE.INI`:
```ini
[Campaign]
IntroMovie=<string>  ; The intro movie name (without the .VQA extension) to play at the start of the campaign? Defaults to <none>.
```

### DebugOnly

- `DebugOnly` can now be set for campaigns, which adds the prefix of "[Debug]" to the campaign description. In addition to this, it also makes the campaign only available Developer mode.
In `BATTLE.INI`:
```ini
[Campaign]
DebugOnly=<boolean>  ; Is this campaign only available in Developer mode? Defaults to no.
```
For testing/debugging versions of the Tiberian Sun and Firestorm campaigns, download [BATTLE_DEBUG_CAMPAIGN.INI](https://github.com/Vinifera-Developers/Vinifera-Files/blob/master/files/BATTLE_DEBUG_CAMPAIGN.INI) and place it in your game install directory.

## Scenario Settings

### Ice Destruction

- Ice destruction can now be disabled.

In a scenario file:
```ini
[Basic]
IceDestructionEnabled=<boolean> ; Can ice tiles be destroyed in the scenario? Defaults to yes.
```

## Script Actions

## Trigger Actions
