# Mapping

This page describes all mapping-related additions and changes introduced by Vinifera.

## Bugfixes and Miscellaneous

- The game now supports reading and using up to 32767 waypoints in scenarios.
- Tutorial messages are now loaded from scenarios. This can be used to replace/update an existing entry from `TUTORIAL.INI`, or to add a new tutorial message index which can be used by trigger actions.

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
