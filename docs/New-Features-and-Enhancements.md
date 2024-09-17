# New Features and Enhancements

This page describes all the engine features that are either new and introduced by Vinifera or are otherwise enhanced.

## Aircraft

### CurleyShuffle

- `[General]->CurleyShuffle`, which controls if the aircraft will shuffle its position between firing at its target, can now be overridden on a per-type basis.

In `rulesmd.ini`:
```ini
[AircraftType]
CurleyShuffle=<boolean>  ; Should this aircraft shuffle its position between firing at its target? Defaults to [General]->CurleyShuffle.
```

### ReloadRate

- `[General]->ReloadRate`, which controls the rate that aircraft will reload its ammo when docked with a helipad, can now be overridden on a per-type basis.

In `rulesmd.ini`:
```ini
[AircraftType]
ReloadRate=<floating point>  ; The rate that this aircraft will reload its ammo when docked with a helipad. Defaults to [General]->ReloadRate.
```

## Animations

## Buildings

## Crates

## Ice

- Ice strength can now be customized.
In `rulesmd.ini`:
```ini
[CombatDamage]
IceStrength=<integer>  ; The strength of ice. Higher values make ice less likely to break from a shot.
                       ; 0 makes ice break from any shot. Defaults to 0.
```

## Infantry

## Particle Systems

## Particles

## Projectiles

## Super Weapons

## Technos

### AILegalTarget

- `AILegalTarget` can be used with TEchnoTypes to forbid the AI from performing a targeting evaluation on this object. It is subject to LegalTarget=yes.

In `rulesmd.ini`:
```ini
[TechnoType]
AILegalTarget=<boolean>  ; Can this object be the target of an attack or move command by the computer? Defaults to yes.
```

## Terrain

## Tiberiums

## Vehicles

## Warheads

## Weapons