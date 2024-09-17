# New Features and Enhancements

This page describes all the engine features that are either new and introduced by Vinifera or are otherwise enhanced.

## Aircraft

### CurleyShuffle

- `[General]->CurleyShuffle`, which controls if the aircraft will shuffle its position between firing at its target, can now be overridden on a per-type basis.

In `RULES.INI`:
```ini
[AircraftType]
CurleyShuffle=<boolean>  ; Should this aircraft shuffle its position between firing at its target? Defaults to [General]->CurleyShuffle.
```

### ReloadRate

- `[General]->ReloadRate`, which controls the rate that aircraft will reload its ammo when docked with a helipad, can now be overridden on a per-type basis.

In `RULES.INI`:
```ini
[AircraftType]
ReloadRate=<floating point>  ; The rate that this aircraft will reload its ammo when docked with a helipad. Defaults to [General]->ReloadRate.
```

## Animations

## Buildings

## Crates

## Ice

- Ice strength can now be customized.

In `RULES.INI`:
```ini
[CombatDamage]
IceStrength=<integer>  ; The strength of ice. Higher values make ice less likely to break from a shot.
                       ; 0 makes ice break from any shot. Defaults to 0.
```

## Infantry

### Mechanic and OmniHealer

- Vinifera reimplements the legacy `Mechanic` logic from Red Alert. In addition to this, a new key to allow the healing of both infantry and units has been added.

```{note}
Both these systems require the warhead to deal negative damage.
```

In `RULES.INI`:
```ini
[InfantryType]
Mechanic=<boolean>  ; Should this infantry only consider unit and aircraft as valid targets? Defaults to no.
OmniHealer=<boolean>  ; Should this infantry consider other infantry, unit, and aircraft as valid targets? Defaults to no.
```

```{note}
When an infantry with `Mechanic=yes` and `OmniHealer=yes` is selected and the mouse is over a transport unit or aircraft, holding down the `Alt` key (Force Move) will allow you to enter the transport instead of healing it.
```

## Particle Systems

## Particles

## Projectiles

## Super Weapons

## Technos

### AILegalTarget

- `AILegalTarget` can be used with TechnoTypes to forbid the AI from performing a targeting evaluation on this object. It is subject to LegalTarget=yes.

In `RULES.INI`:
```ini
[TechnoType]
AILegalTarget=<boolean>  ; Can this object be the target of an attack or move command by the computer? Defaults to yes.
```

### CanPassiveAcquire

- The `CanPassiveAcquire` key has been backported from Red Alert 2, which controls whether or not the object may acquire targets (within range) and attack them automatically, without player input.

In `RULES.INI`:
```ini
[TechnoType]
CanPassiveAcquire=<boolean>  ; Can this object acquire targets that are within its weapons range and attack them automatically? Defaults to yes.
```
```{note}
This key has a spelling error for "Acquire" and should be `CanPassiveAcquire`.
```

### CanRetaliate

- The `CanRetaliate` key has been backported from Red Alert 2, which controls if the object may retaliate (if other conditions are met) when hit by enemy fire.

In `RULES.INI`:
```ini
[TechnoType]
CanRetaliate=<boolean>  ; Can this unit retaliate (if general conditions are met) when hit by enemy fire? Defaults to yes.
```

### Idle Animation Improvements

- The `IdleRate` key has been backported from Red Alert 2. This allows units with shape graphics to animate with their walk frames while idle (standing still).
```{note}
This key can be defined on either the `RULES.INI` section or the `ART.INI` image section, but the latter will take priority.
```

In `ART.INI`:
```ini
[TechnoType]
IdleRate=<unsigned integer>  ; The rate at which this unit animates when it is standing idle (not moving). Defaults to 0.
```

- In addition to this, to help define custom idle animations, `StartIdleFrame` and `IdleFrames` has been added for UnitTypes. These will only be used if the UnitType has an `IdleRate` greater than 0.

In `ART.INI`:
```ini
[TechnoType]
StartIdleFrame=<unsigned integer>  ; The starting frame for the idle animation in the units shape file. Defaults to the value of StartWalkFrame.
IdleFrames=<unsigned integer>  ; The number of image frames for each of the idle animation sequences. Defaults to the value of WalkFrames.
```

### EnterTransportSound/LeaveTransportSound

- Vinifera implements EnterTransportSound and LeaveTransportSound from Red Alert 2 for TechnoTypes.

In `RULES.INI`:
```ini
[TechnoType]
EnterTransportSound=<VocType>  ; The sound effect to play when a passenger enters this unit. Defaults to <none>.
LeaveTransportSound=<VocType>  ; The sound effect to play when a passenger leaves this unit. Defaults to <none>.
```

## Terrain

## Theaters

- Vinifera allow the creation of new custom theater types. A new INI has been added to define these TheaterTypes, if the INI is not present, the game will default to the normal `TEMPERATE` and `SNOW` TheaterTypes.
```{warning}
The random map generator does not currently support new theater types.
```
- <details>
    <summary>Basic `THEATERS.INI`</summary>
    ```ini
    ;============================================================================
    ; THEATER.INI
    ;
    ; This control file specifies the theater types that are in the game.
    ;
    ; $Author: $
    ; $Archive: $
    ; $Modtime: $
    ; $Revision: $
    ;============================================================================


    ; ******* Theater List *******
    ; Lists the theater types in this control file. Each
    ; theater is given a unique (internal only) identifier name.
    [TheaterTypes]
    1=TEMPERATE
    2=SNOW


    ; ******* Individual Theater Data *******
    ; Each theater data lists its information in a section that
    ; corresponds to its identifier theater name (see above).
    ;
    ;  -- All fields are required! --

    ; Root = The root name for the theater data and control INI. [9 characters max]
    ; IsoRoot = The root name for the theater tileset data. [9 characters max]
    ; Suffix = The file suffix for loading the theaters tilesets. [3 characters max]
    ; MMSuffix = The suffix for the "marble madness" tiles. [3 characters max]
    ; ImageLetter = The theater image letter, used to fixup graphics. [single characters only]
    ; BiomeName = The name of this theater as it appears in the map generator. [32 characters max]
    ;             This only applies to new theater types, TEMPERATE and SNOW are ignored.
    ; IsArctic = Is this theater the "arctic" theater set [used for deciding which occupy bits are used]? (def = false)
    ; IsIceGrowthEnabled = Is the ice growth logic enabled for this theater? (def = false)
    ; IsVeinGrowthEnabled = Is the vein growth logic enabled for this theater? (def = false)
    ; IsAllowedInRMG = Is this theater allowed to be used in the map generator? (def = false)
    ; IsGenerateVeinholesInRMG = Should the map generator produce veinholes for this theater? (def = false)
    ; LowRadarBrightness = The brightness of the lowest height level cells when drawn on the radar. (def = 1.0)
    ; HighRadarBrightness = The brightness of the highest height level cells when drawn on the radar. (def = 1.0)

    [TEMPERATE]
    Root=TEMPERAT
    IsoRoot=ISOTEMP
    Suffix=TEM
    MMSuffix=MMT
    ImageLetter=T
    IsArctic=false
    IsIceGrowthEnabled=false
    IsVeinGrowthEnabled=true
    IsAllowedInRMG=true
    IsGenerateVeinholesInRMG=true
    LowRadarBrightness=1.0
    HighRadarBrightness=1.6

    [SNOW]
    Root=SNOW
    IsoRoot=ISOSNOW
    Suffix=SNO
    MMSuffix=MMS
    ImageLetter=A
    IsArctic=true
    IsIceGrowthEnabled=true
    IsVeinGrowthEnabled=true
    IsAllowedInRMG=true
    IsGenerateVeinholesInRMG=true
    LowRadarBrightness=0.8
    HighRadarBrightness=1.1
    ```
   </details>

   - <details>
    <summary>Sample new theater</summary>
    ```ini
    [TheaterTypes]
    3=DESERT

    [DESERT]
    Root=DESERT
    IsoRoot=ISODES
    Suffix=DES
    MMSuffix=MMD
    ImageLetter=D
    BiomeName=New Desert
    IsArctic=false
    IsIceGrowthEnabled=false
    IsVeinGrowthEnabled=false
    IsAllowedInRMG=false
    IsGenerateVeinholesInRMG=false
    LowRadarBrightness=1.0
    HighRadarBrightness=1.6
    ```

    Files following this format must exist otherwise the game could crash at any moment during gameplay.
    `DESERT.MIX`, `ISODES.MIX`, `DES.MIX`, `DESERT.INI`, `ISODES.PAL`, `DESERT.PAL`, `UNITDES.PAL`, and `SLOP#Z.DES` (where # is 1 to 4).
   </details>

## Themes

- `RequiredAddon` can be set to be limit new and existing themes to a specific addon (i. e., Firestorm).
In `THEME.INI`:
```ini
[ThemeType]
RequiredAddon=<AddonType>  ; The addon required to be active for this theme to be available. Currently, only 1 (Firestorm) is supported. Defaults to 0 (none).
```

## Tiberiums

## Vehicles

## Warheads

## Weapons