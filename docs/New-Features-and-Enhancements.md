# New Features and Enhancements

This page describes all the engine features that are either new and introduced by Vinifera or are otherwise enhanced.

## Aircraft

### CurleyShuffle

- `[General]->CurleyShuffle`, which controls if the aircraft will shuffle its position between firing at its target, can now be overridden on a per-type basis.

In `RULES.INI`:
```ini
[SOMEAIRCRAFT]  ; AircraftType
CurleyShuffle=  ; boolean, should this aircraft shuffle its position between firing at its target?
```

### ReloadRate

- `[General]->ReloadRate`, which controls the rate that aircraft will reload its ammo when docked with a helipad, can now be overridden on a per-type basis.

In `RULES.INI`:
```ini
[SOMEAIRCRAFT]  ; AircraftType
ReloadRate=     ; float, the rate that this aircraft will reload its ammo when docked with a helipad. Defaults to [General]->ReloadRate.
```

## Animations

### Various Keys Ported from Red Alert 2

- Vinifera implements various AnimType keys from Red Alert 2.

In `ART.INI`:
```ini
[SOMEANIM]             ; AnimType
HideIfNoTiberium=no    ; boolean, should this animation be hidden if the holding cell does not contain Tiberium?
ForceBigCraters=no     ; boolean, are the craters spawned by this animation when it ends much larger than normal?
ZAdjust=0              ; integer, fudge to this animation's Z-axis (depth). Positive values move the animation "away from the screen"/"closer to the ground", negative values do the opposite.
Layer=<none>           ; LayerType, the map layer this animation is in when attached to an object.
                       ; Available Options: underground, surface, ground, air, and top.
                       ; NOTE: This will override the value of Surface= which forces a layer of ground.
SpawnsParticle=<none>  ; ParticleType, the particle to spawn at the mid-point of this animation.
                       ; This accepts any entry from the [Particles] list from RULES.INI.
NumParticles=0         ; integer, the number of particles to spawn (as defined by SpawnsParticle=).
```

## Buildings

### Gate Sounds

- Vinifera implements overrides for the gate rising and lowering sounds on BuildingTypes.

In `RULES.INI`:
```ini
[SOMEBUILDING]  ; BuildingType
GateUpSound=    ; VocType, sound effect to play when the gate is rising. Defaults to [AudioVisual]->GateUp.
GateDownSound=  ; VocType, sound effect to play when the gate is lowering. Defaults to [AudioVisual]->GateDown.
```

### ProduceCash

- Vinifera implements the Produce Cash logic from Red Alert 2. The system works exactly as it does in Red Alert 2, but with the following differences:
    - Ability to set a total budget available to the building.
    - The logic is sensitive to `Powered=yes`, meaning it will stop when the house has low power.

In `RULES.INI`:
```ini
[SOMEBUILDING]                ; BuildingType
ProduceCashStartup=0          ; integer, credits when captured from a house that has MultiplayPassive=yes set.
ProduceCashAmount=0           ; integer, amount every give to/take from the house every delay. This value supports negative values which will deduct money from the house which owns this building.
ProduceCashDelay=0            ; integer, frame delay between amounts.
ProduceCashBudget=0           ; integer, the total cash budget for this building.
ProduceCashResetOnCapture=no  ; boolean, reset the buildings available budget when captured.
ProduceCashStartupOneTime=no  ; boolean, is the bonus on capture a "one one" special (further captures will not get the bonus)?
```

### Crew

- Vinifera allows customizing the chance that an engineer will exit a building upon its deconstruction.

In `RULES.INI`:
```ini
[SOMEBUILDING]    ; BuildingType
EngineerChance=0  ; integer (%), what is the chance that an engineer will exit this building as its crew. Defaults to 25 for `Factory=BuildingType`, 0 otherwise.
```

```{warning}
It is not recommended to set `EngineerChance=100`, as this may put the game into an infinite loop when it insists an infantry other than an engineer exits the building.
```

## Ice

- Ice strength can now be customized.

In `RULES.INI`:
```ini
[CombatDamage]
IceStrength=0   ; integer, the strength of ice. Higher values make ice less likely to break from a shot.
                ; 0 makes ice break from any shot.
```

- Ice destruction can now be enabled or disabled per-scenario.

In the map file:
```ini
[Basic]
IceDestructionEnabled=no
```

## Infantry

### Mechanic and OmniHealer

- Vinifera reimplements the legacy `Mechanic` logic from Red Alert. In addition to this, a new key to allow the healing of both infantry and units has been added.

```{note}
Both these systems require the warhead to deal negative damage.
```

In `RULES.INI`:
```ini
[SOMEINFANTRY]  ; InfantryType
Mechanic=no     ; boolean, should this infantry only consider unit and aircraft as valid targets?
OmniHealer=no   ; boolean, should this infantry consider other infantry, unit, and aircraft as valid targets?
```

```{note}
When an infantry with `Mechanic=yes` and `OmniHealer=yes` is selected and the mouse is over a transport unit or aircraft, holding down the `Alt` key (Force Move) will allow you to enter the transport instead of healing it.
```

## Projectiles

### SpawnDelay

- Vinifera adds the `SpawnDelay` key from Red Alert 2.

In `RULES.INI`:
```ini
[SOMEBULLET]  ; BulletType
SpawnDelay=3  ; unsigned integer, the number of frames between each of the spawned trailer animations.
```

## Sides

### Crew

- Vinifera adds the option to customize the crew a side uses.

In `RULES.INI`:
```ini
[SOMESIDE]        ; Side
Crew=             ; InfantryType, this side's crew. Defaults to `[General]->Crew`
Engineer=         ; InfantryType, this side's engineer. Defaults to `[General]->Engineer`
Technician=       ; InfantryType, this side's technician. Defaults to `[General]->Technician`
Disguise=         ; InfantryType, the type this side will see other players' spies as. Defaults to `[General]->Disguise`
SurvivorDivisor=  ; integer, this side's survivor divisor. Defaults to `[General]->SurvivorDivisor`
```

### Colors

- Vinifera adds the option to customize what colors are used in the user interface per-side.

In `RULES.INI`:
```ini
[SOMESIDE]          ; Side
UIColor=LightGold   ; ColorScheme, the color to be used when drawing UI elements.
ToolTipColor=Green  ; ColorScheme, the color to be used when drawing tooltips.
```

![image](https://github.com/user-attachments/assets/f4219655-2d28-49d2-9537-25f2fe4ae102)

### Power Plants

- Vinifera allows customizing what power plants the AI will build per side.

In `RULES.INI`:
```ini
[SOMESIDE]           ; Side
RegularPowerPlant=   ; BuildingType, the advanced power plant this side uses.
AdvancedPowerPlant=  ; BuildingType, the regular power plant this side uses.
PowerTurbine=        ; BuildingType, the power turbine this side uses.
```

- The AI will prioritize upgrading its existing regular power plants with turbines. If possible, it will then construct advanced power plants; otherwise, it will build additional regular power plants.
- If the side's name contains `GDI`, the defaults are as follows:
```ini
RegularPowerPlant=[General]->GDIPowerPlant
AdvancedPowerPlant=<none>
PowerTurbine=[General]->GDIPowerTurbine
```
- Otherwise, the defaults are as follows:
```ini
RegularPowerPlant=[General]->NodRegularPower
AdvancedPowerPlant=[General]->NodAdvancedPower
PowerTurbine=<none>
```

```{note}
If you want to remove a key's default value, you can set its value to `<none>`.
```

### Hunter-Seeker

- Vinifera adds the option to customize what unit serves as the Hunter-Seeker per side.
- The default Hunter-Seeker is `[General]->GDIHunterSeeker` if the side's name contains `GDI`; otherwise, it defaults to `[General]->NodHunterSeeker`.

In `RULES.INI`:
```ini
[SOMESIDE]     ; Side
HunterSeeker=  ; UnitType, the unit that is this side's Hunter-Seeker.
```

## Technos

### New ArmorTypes

- Vinifera allows adding new armor types, as well as customizing the ability of warheads to target them.

In `RULES.INI`:
```ini
[ArmorTypes]
5=myarmor  ; string, armor name, recommended to be lowercase

[myarmor]
Modifier=100%       ; % or float, default Verses value for this armor.
ForceFire=yes       ; boolean, whether warheads can by default force-fire at this armor type.
Retaliate=yes       ; boolean, whether warheads can by default retaliate against this armor type.
PassiveAcquire=yes  ; boolean, whether warheads can by default passive acquire this armor type.
```

```{note}
Vanilla ArmorTypes are present implicitly, redeclaring them has no effect. However, it is possible to override default values for them in their respective sections.
```

- The new armor can be assigned to a Techno as any default armor.

```{note}
Armor names are case-sensitive, including when part of `INI` keys.
```

In `RULES.INI`:
```ini
[SOMETECHNO]   ; TechnoType
Armor=myarmor  ; ArmorType
```

- Verses for the new armor are appended to the end of the vanilla Verses key.
- Additionally, it is possible to customize whether a unit using a certain warhead can force-fire, retaliate or passively acquire units with a certain armor type. This mimics the special behavior of RA2's 0% and 1%, as well as Ares's 2%.
- It is also possible to override the game's default hardcoded behavior for infantry using warheads with 0% verses against armor "heavy".

In `RULES.INI`:
```ini
[SOMEWARHEAD]                           ; WarheadType
Verses=100%,100%,100%,100%,100%,100%    ; list of % or floats, the damage multiplier against a specific armor type. 1 entry per ArmorType.
ForceFire=yes,yes,yes,yes,yes,yes       ; list of booleans, whether this warhead can be used to force-fire at a specific armor type. 1 entry per ArmorType.
Retaliate=yes,yes,yes,yes,yes,yes       ; list of booleans, whether this warhead can be used to retaliate against a specific armor type. 1 entry per ArmorType.
PassiveAcquire=yes,yes,yes,yes,yes,yes  ; list of booleans, whether this warhead can be used to passive acquire a specific armor type. 1 entry per ArmorType.
Organic=no                              ; boolean, whether an infantry using this warhead can passively acquire or retaliate against vehicles, aircraft or buildings. This overrides the RA/TS 0% behavior.
```

- Alternatively, separate keys may be used, achieving higher clarify and allowing for setting values out of order. These keys take priority over the list keys.

In `RULES.INI`:
```ini
[SOMEWARHEAD]               ; WarheadType
Versus.myarmor=100%         ; % or float, the damage multiplier against a this armor type.
ForceFire.myarmor=yes       ; boolean, whether this warhead can be used to force-fire at a this armor type.
Retaliate.myarmor=yes       ; boolean, whether this warhead can be used to retaliate against a this armor type.
PassiveAcquire.myarmor=yes  ; boolean, whether this warhead can be used to passive acquire a this armor type.
Organic=no                  ; boolean, whether an infantry using this warhead can passively acquire or retaliate against vehicles, aircraft or buildings. This overrides the RA/TS 0% behavior.
```

```{warning}
It is recommended to set both `Retaliate.X` and `PassiveAcquire.X` to `no` if `ForceFire.X` is disabled. Otherwise, units may lock onto targets they are not permitted to fire at and continue to target them until they receive another order.
```

### Crew

- Vinifera allows customizing how many crew will exit a unit upon its death. Crew will only exit if `Crewed=yes`, even if `CrewCount` is set to a number greater than 0.

In `RULES.INI`:
```ini
[SOMETECHNO]  ; TechnoType
CrewCount=1   ; integer, how many crew will exit this unit.
```

```{note}
This tag does not apply to buildings.
```

### AILegalTarget

- `AILegalTarget` can be used with TechnoTypes to forbid the AI from performing a targeting evaluation on this object. It is subject to LegalTarget=yes.

In `RULES.INI`:
```ini
[SOMETECHNO]       ; TechnoType
AILegalTarget=yes  ; boolean, can this object be the target of an attack or move command by the computer?
```

### CanPassiveAcquire

- The `CanPassiveAcquire` key has been backported from Red Alert 2, which controls whether or not the object may acquire targets (within range) and attack them automatically, without player input.

In `RULES.INI`:
```ini
[SOMETECHNO]           ; TechnoType
CanPassiveAcquire=yes  ; boolean, can this object acquire targets that are within its weapons range and attack them automatically?
```
```{note}
In Red Alert 2, this key has a spelling error for "Acquire", spelling it as "Aquire". This has been fixed in Vinifera.
```

### CanRetaliate

- The `CanRetaliate` key has been backported from Red Alert 2, which controls if the object may retaliate (if other conditions are met) when hit by enemy fire.

In `RULES.INI`:
```ini
[SOMETECHNO]      ; TechnoType
CanRetaliate=yes  ; boolean, can this unit retaliate (if general conditions are met) when hit by enemy fire?
```

### Idle Animation Improvements

- The `IdleRate` key has been backported from Red Alert 2. This allows units with shape graphics to animate with their walk frames while idle (standing still).
```{note}
This key can be defined on either the `RULES.INI` section or the `ART.INI` image section, but the latter will take priority.
```

In `ART.INI`:
```ini
[SOMETECHNO]  ; TechnoType
IdleRate=0    ; unsigned integer, the rate at which this unit animates when it is standing idle (not moving). Defaults to 0.
```

- In addition to this, to help define custom idle animations, `StartIdleFrame` and `IdleFrames` has been added for UnitTypes. These will only be used if the UnitType has an `IdleRate` greater than 0.

In `ART.INI`:
```ini
[SOMETECHNO]     ; TechnoType
StartIdleFrame=  ; unsigned integer, the starting frame for the idle animation in the units shape file. Defaults to StartWalkFrame.
IdleFrames=      ; unsigned integer, the number of image frames for each of the idle animation sequences. Defaults to WalkFrames.
```

### Transport Sounds

- Vinifera implements `EnterTransportSound` and `LeaveTransportSound` from Red Alert 2 for TechnoTypes.

In `RULES.INI`:
```ini
[SOMETECHNO]                ; TechnoType
EnterTransportSound=<none>  ; VocType, the sound effect to play when a passenger enters this unit.
LeaveTransportSound=<none>  ; VocType, the sound effect to play when a passenger leaves this unit.
```

### Soylent

- Vinifera adds the `Soylent` key from Red Alert 2.

In `RULES.INI`:
```ini
[SOMETECHNO]  ; TechnoType
Soylent=0     ; unsigned integer, the refund value for the unit when it is sold at a Service Depot, or a building when sold by the player. 0 uses normal refund amount logic.
```

### New Voice Responses

- Vinifera implements various TechnoTypes keys from Red Alert 2 for adding new voice responses.

In `RULES.INI`:
```ini
[SOMETECHNO]         ; TechnoType
VoiceCapture=<none>  ; VocType list, list of voices to use when giving this object a capture order.
VoiceEnter=<none>    ; VocType list, list of voices to use when giving this object an enter order.
VoiceDeploy=<none>   ; VocType list, list of voices to use when giving this object a unload order.
VoiceHarvest=<none>  ; VocType list, list of voices to use when giving this object a harvest order.
```

### Customizable Cloaking Sounds

- Vinifera implements Cloaking and Uncloaking sound overrides to TechnoTypes.

In `RULES.INI`:
```ini
[SOMETECHNO]   ; TechnoType
CloakSound=    ; sound, the sound effect to play when the object is cloaking. Defaults to [AudioVisual]->CloakSound.
UncloakSound=  ; sound, the sound effect to play when the object is decloaking. Defaults to [AudioVisual]->CloakSound.
```

### Screen Shake on Destruction

- Vinifera restores the screen shake when a strong unit or building is destroyed. In addition to this, it also implements new options to control the amount the screen moves.

In `RULES.INI`:
```ini
[SOMETECHNO]       ; TechnoType
CanShakeScreen=no  ; boolean, can this unit or building cause the screen to shake the screen when it dies?
```

```{note}
The object must meet the rules as specified by `[AudioVisual]->ShakeScreen`.
```

- Shake Screen Controls
These values are used to shake the screen when the unit or building is destroyed.
In `RULES.INI`:
```ini
[SOMETECHNO]  ; TechnoType
ShakeYhi=0    ; unsigned integer, the maximum pixel Y value.
ShakeYlo=0    ; unsigned integer, the minimum pixel Y value.
ShakeXhi=0    ; unsigned integer, the maximum pixel X value.
ShakeXlo=0    ; unsigned integer, the minimum pixel X value.
```

### WalkRate

- Vinifera allows `WalkRate` to be optionally loaded from ART.INI image entries, overriding any value defined in RULES.INI.

### ImmuneToEMP

- Vinifera allows specific TechnoTypes to be immune to EMP effects.

In `RULES.INI`:
```ini
[SOMETECHNO]    ; TechnoType
ImmuneToEMP=no  ; boolean, is this Techno immune to EMP effects?
```

### Custom Special Pip

- TechnoTypes can have a custom pip be drawn in the same place as the medic pip using. Its location is the same as the medic pip's.

In `RULES.INI`:
```ini
[SOMETECHNO]        ; TechnoType
SpecialPipIndex=-1  ; integer, index of the pip to draw in place of the medic pip.
```

### PipWrap

- Vinifera ports PipWrap from Red Alert 2. If PipWrap is set to a positive integer greater than 0, that number of ammo pips will be rendered, incrementing the frame number for each time the pip count overflows PipWrap.
- For usage notes, please see [the ModEnc article](https://modenc.renegadeprojects.com/PipWrap).

In `RULES.INI`:
```ini
[SOMETECHNO]        ; TechnoType
PipWrap=0           ; integer, the number of ammo pips to draw using pip wrap.
```

```{note}
For `PipWrap` to function, new pips need to be added to `pips2.shp`. The pip at index 7 (1-based) is still used by ammo when `PipWrap=0`, pips starting from index 8 are used by `PipWrap`.
```

### Selection Filtering

- Vinifera adds the ability to exclude some TechnoTypes from band selection.

In `RULES.INI`:
```ini
[SOMETECHNO]                   ; TechnoType
FilterFromBandBoxSelection=no  ; boolean, should this Techno be excluded from band box selection when it contains units without this flag?
```

- Technos with `FilterFromBandBoxSelection=yes` will only be selected if the current selection contains any units with `FilterFromBandBoxSelection=yes`, or the player is making a new selection and only Technos with `FilterFromBandBoxSelection=yes` are in the selection box.
- By holding `ALT` it is possible to temporarily ignore this logic and select all types of objects.

- It is also possible to disable this behavior in `SUN.INI`.

In `SUN.INI`:
```ini
[Options]
FilterBandBoxSelection=yes  ; boolean, should the band box selection be filtered?
```

## Terrain

### Light Sources

- Vinifera implements light sources for TerrainTypes.

In `RULES.INI`:
```ini
[SOMETERRAIN]         ; TerrainType
IsLightEnabled=no     ; boolean, does this terrain object emit light?
LightVisibility=5000  ; integer, this terrain object radiates this amount of light.
LightIntensity=0      ; float, the distance that this light is visible from.
LightRedTint=1        ; float, the red tint of this terrain objects light.
LightGreenTint=1      ; float, the green tint of this terrain objects light.
LightBlueTint=1       ; float, the blue tint of this terrain objects light.
```

## Theaters

- Vinifera allow the creation of new custom theater types. A new INI has been added to define these TheaterTypes, if the INI is not present, the game will default to the normal `TEMPERATE` and `SNOW` TheaterTypes.
```{warning}
The random map generator does not currently support new theater types.
```
- <details>
    <summary>Basic `THEATERS.INI`</summary>

    ```ini
    ;============================================================================
    ; THEATERS.INI
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
[SOMETHEME]      ; ThemeType
RequiredAddon=0  ; AddonType, the addon required to be active for this theme to be available. Currently, only 0 (none) and 1 (Firestorm) are supported. 
```

## Tiberiums

### New Tiberiums

- Vinifera allows mods to create new Tiberiums beyond the vanilla 4.

- A Tiberium's Image can be customized manually.

In `RULES.INI`:
```ini
[SOMETIBERIUM]   ; Tiberium
Overlay=         ; OverlayType, the first overlay that the Tiberium uses, defaults to the value usually used by the Image=, or overlay at index 102 if not specified
UseSlopes=false  ; boolean, does this Tiberium have graphics for slopes?
```

```{note}
The new graphics keys override defaults set according to Image=, please refer to [ModEnc](https://modenc.renegadeprojects.com/Image) about its vanilla behavior. It is not required to set Image= if you specify the graphics using new keys.
```

```{note}
`Overlay` specifies the first overlay the Tiberium uses. There must be 12 overlays, located one after another sequentially. Additionally, is `UseSlopes` is set to yes, another 8 overlays are required after the previous 12.
```

```{warning}
All `OverlayTypes` used by a `Tiberium` must have `Tiberium=yes`, and no other `OverlayTypes` may have `Tiberium=yes`, or this will lead to severe lags/crashes.
```

### Tiberium Damage to Infantry

- The damage Tiberium deals to infantry is now customizable separately from Power.

In `RULES.INI`:
```ini
[SOMETIBERIUM]     ; Tiberium
DamageToInfantry=  ; integer, the damage to infantry per tick, defaults to Power / 10
```

### Pips

- Vinifera allows customizing the pips used for Tiberiums in unit storage, as well as their draw order.

In `RULES.INI`:
```ini
[SOMETIBERIUM]  ; Tiberium
PipIndex=1      ; integer, pip index to use.
PipDrawOrder=1  ; integer, the order the pips are drawn in. Less is earlier.
```

- Additionally, buildings now show their storage with the proper pips, instead of showing pip 1 for all tiberiums
- The pip used to diplay weeds can now also be customized.

In `RULES.INI`:
```ini
[AudioVisual]
WeedPipIndex=1  ; integer, the pip index used for Weeds.
```

## Vehicles

### Unit Transform

- Vinifera adds a new flag that allows a unit to transform into another type of unit upon deploying instead of transforming into a building.

In `RULES.INI`:
```ini
[SOMEUNIT]
TransformsInto=OTHERUNIT
```

- Additionally, the unit can be configured to require full charge to be able to transform, reusing the charge mechanic from the vanilla Mobile EMP Cannon. To do this, give the unit `TransformRequiresFullCharge=yes`.

### Totable

- Vinifera adds a new flag which can prevent a vehicle from being picked up by a Carryall.

In `RULES.INI`:
```ini
[SOMEUNIT]   ; UnitType
Totable=yes  ; boolean, can this unit be picked up by a Carryall?
```

### UnloadingClass

- Vinifera adds support for a custom unloading class when a harvester is unloading at a refinery. In addition to this working for regular harvesters, this will now work on harvesters with `Weeder=yes` for when they dock at a building that has `Weeder=yes`.

In `RULES.INI`:
```ini
[SOMEUNIT]       ; UnitType
UnloadingClass=  ; UnitType, UnitType whose image will be used when this harvester is docked.
```

### More Graphic Facings

- The engine now supports 16, 32 and 64 graphic facings for UnitTypes.

In `RULES.INI`:
```ini
[SOMEUNIT]           ; UnitType
StartTurretFrame=-1  ; integer, the starting turret frame index, allowing them to be adjusted manually if required.
TurretFacings=32     ; integer, the turret facing count.
```

- Additionally, the `Anim=` INI key for WeaponTypes will now read the number of entries that matches the firing object's Facings= entry.
- Because of the new extended facing support, it was observed that the buffer size was too small and has now been increased to allow a larger entry to accommodate a larger facing count.

## Warheads

### `[Weapons]` Section

- Vinifera implements the reading of a new `RULES.INI` section, `[Weapons]`, to allow the definition of WeaponTypes.
This is to fix the issue known as the "Weed Guy" hack, and ensure all weapons are allocated in the WeaponsType heaps before any weapon lookup or loading is performed.

### Various Keys Ported from Red Alert 2

- Vinifera implements various WarheadType keys from Red Alert 2.

In `RULES.INI`:
```ini
[SOMEWARHEAD]             ; WarheadType
WallAbsoluteDestroyer=no  ; boolean, does this warhead instantly destroy walls regardless of the warhead damage value?
AffectsAllies=yes         ; boolean, can this warhead damage friendly units?
CombatLightSize=0         ; integer, this is used to override the size of the combat light flash at the point of impact for Warheads with Bright=yes set (Bright=yes must also be set on the Weapon using this warhead).
```

- Shake Screen Controls
These values are used to shake the screen when the projectile impacts.
In `RULES.INI`:
```ini
[SOMEWARHEAD]   ; WarheadType
ShakeYhi=0      ; unsigned integer, the maximum pixel Y value.
ShakeYlo=0      ; unsigned integer, the minimum pixel Y value.
ShakeXhi=0      ; unsigned integer, the maximum pixel X value.
ShakeXlo=0      ; unsigned integer, the minimum pixel X value.
```

## Weapons

### Electric Bolts

- Vinifera implements the Electric Bolt (aka. "Tesla Bolts") weapon effect from Red Alert 2, with additional controls.

In `RULES.INI`:
```ini
[SOMEWEAPON]             ; WeaponType
IsElectricBolt=no        ; boolean, is this weapon an electric bolt? This is required to enable the drawing feature.
EBoltSegmentCount=8      ; integer, how many segment blocks should the electric bolt be made up from. A larger number will give a more "wild" effect.
EBoltLifetime=17         ; integer, the lifetime of the electric bolt graphic in game frames.
EBoltIterations=1        ; integer, how many draw iterations should the system perform?
EBoltDeviation=1         ; float, the maximum deviation from a straight line the electric bolts can be. A value of 0.0 will draw straight lines.
                         ; Electric bolts are made up of 3 lines, these values define the colours for each of the lines.
EBoltColor1=255,255,255  ; RGB color.
EBoltColor2=82,81,255    ; RGB color.
EBoltColor3=82,81,255    ; RGB color.
```

![GIF 08-09-2021 19-17-13](https://user-images.githubusercontent.com/73803386/132563132-7ebb771f-8acf-4ee2-ba4b-8dfa8a01de8f.gif)

### Suicide

- Vinifera adds the `Suicide` key or WeaponTypes from Red Alert 2, and adds an additional control `DeleteOnSuicide` for alternative behaviour.

In `RULES.INI`:
```ini
[SOMEWEAPON]        ; WeaponType
Suicide=no          ; boolean, will the firing unit commit suicide when this weapon is fired?
DeleteOnSuicide=no  ; boolean, logical option for Suicide=yes which will instantly remove the unit from the game world instead of dealing full damage.
```

```{note}
`DeleteOnSuicide=yes` mimics Red Alert 2 behavior.
```