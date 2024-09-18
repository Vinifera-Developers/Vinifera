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

### Various Keys Ported from Red Alert 2

- Vinifera implements various AnimType keys from Red Alert 2.

In `ART.INI`:
```ini
[AnimType]
HideIfNoTiberium=<boolean>  ; Should this animation be hidden if the holding cell does not contain Tiberium? Defaults to no.
ForceBigCraters=<boolean>  ; Are the craters spawned by this animation when it ends much larger than normal? Defaults to no.
ZAdjust=<integer>  ; Fudge to this animations Z-axis (depth). Positive values move the animation "away from the screen"/"closer to the ground", negative values do the opposite. Defaults to 0.
Layer=<LayerType>  ; The map layer this animation is in when attached to an object. Defaults to <none>.
                   ;Available Options: underground, surface, ground, air, and top.
                   ;NOTE: This will override the value of Surface= which forces a layer of ground.
SpawnsParticle=<ParticleType>  ; The particle to spawn at the mid-point of this animation. Defaults to <none>.
                               ; This accepts any entry from the [Particles] list from RULES.INI.
NumParticles=<integer>  ; The number of particles to spawn (as defined by SpawnsParticle=). Defaults to 0.
```

## Buildings

### Gate Sounds

- Vinifera implements overrides for the gate rising and lowering sounds on BuildingTypes.

In `RULES.INI`:
```ini
[BuildingType]
GateUpSound=<VocType>  ; Sound effect to play when the gate is rising.
GateDownSound=<VocType>  ; Sound effect to play when the gate is lowering.
```

### ProduceCash

- Vinifera implements the Produce Cash logic from Red Alert 2. The system works exactly as it does in Red Alert 2, but with the following differences:
    - Ability to set a total budget available to the building.
    - The logic is sensitive to `Powered=yes`, meaning it will stop when the house has low power.

In `RULES.INI`:
```ini
[BuildingType]
ProduceCashStartup=<integer>  ; Credits when captured from a house that has MultiplayPassive=yes set. Defaults to 0.
ProduceCashAmount=<integer>  ; Amount every give to/take from the house every delay. This value supports negative values which will deduct money from the house which owns this building. Defaults to 0.
ProduceCashDelay=<integer>  ; Frame delay between amounts. Defaults to 0 (instant).
ProduceCashBudget=<integer>  ; The total cash budget for this building. Defaults to 0 (infinite budget).
ProduceCashResetOnCapture=<boolean>  ; Reset the buildings available budget when captured. Defaults to false.
ProduceCashStartupOneTime=<boolean>  ; Is the bonus on capture a "one one" special (further captures will not get the bonus)? Defaults to false.
```

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

## Projectiles

### SpawnDelay

- Vinifera adds the `SpawnDelay` key from Red Alert 2.

In `RULES.INI`:
```ini
[BulletType]
SpawnDelay=<unsigned integer>  ; The number of frames between each of the spawned trailer animations. Defaults to 3.
```

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

- Vinifera implements `EnterTransportSound` and `LeaveTransportSound` from Red Alert 2 for TechnoTypes.

In `RULES.INI`:
```ini
[TechnoType]
EnterTransportSound=<VocType>  ; The sound effect to play when a passenger enters this unit. Defaults to <none>.
LeaveTransportSound=<VocType>  ; The sound effect to play when a passenger leaves this unit. Defaults to <none>.
```

### Soylent

- Vinifera adds the `Soylent` key from Red Alert 2.

In `RULES.INI`:
```ini
[TechnoType]
Soylent=<unsigned integer>  ; The refund value for the unit when it is sold at a Service Depot, or a building when sold by the player. Defaults to 0 (uses normal refund amount logic).
```

### New Voice Responses

- Vinifera implements various TechnoTypes keys from Red Alert 2 for adding new voice responses.

In `RULES.INI`:
```ini
[TechnoType]
VoiceCapture=<VocType list>  ; List of voices to use when giving this object a capture order. Defaults to <none>.
VoiceEnter=<VocType list>  ; List of voices to use when giving this object an enter order (ie, transport, infiltrate building). Defaults to <none>.
VoiceDeploy=<VocType list>  ; List of voices to use when giving this object a unload order. Defaults to <none>.
VoiceHarvest=<VocType list>  ; List of voices to use when giving this object a harvest order. Defaults to <none>.
```

### Customizable Cloaking Sounds

- Vinifera implements Cloaking and Uncloaking sound overrides to TechnoTypes.

In `RULES.INI`:
```ini
[TechnoType]
CloakSound=<Sound>  ; The sound effect to play when the object is cloaking. Defaults to [AudioVisual]->CloakSound.
UncloakSound=<Sound>  ; The sound effect to play when the object is decloaking. Defaults to [AudioVisual]->CloakSound.
```

### Screen Shake on Destruction

- Vinifera restores the screen shake when a strong unit or building is destroyed. In addition to this, it also implements new options to control the amount the screen moves.

In `RULES.INI`:
```ini
[TechnoType]
CanShakeScreen=<boolean>  ; Can this unit or building cause the screen to shake the screen when it dies? Defaults to no.
```

```{note}
The object must meet the rules as specified by `[AudioVisual]->ShakeScreen`.
```

- Shake Screen Controls
These values are used to shake the screen when the unit or building is destroyed. All of these values default to 0 and do not support negative values.
In `RULES.INI`:
```ini
[TechnoType]
ShakeYhi=<unsigned integer>  ; The maximum pixel Y value.
ShakeYlo=<unsigned integer>  ; The minimum pixel Y value.
ShakeXhi=<unsigned integer>  ; The maximum pixel X value.
ShakeXlo=<unsigned integer>  ; The minimum pixel X value.
```

### WalkRate

- Vinifera allows `WalkRate` to be optionally loaded from ART.INI image entries, overriding any value defined in RULES.INI.

### ImmuneToEMP

- Vinifera allows specific TechnoTypes to be immune to EMP effects.

In `RULES.INI`:
```ini
[TechnoType]
ImmuneToEMP=<boolean>  ; Is this Techno immune to EMP effects? Defaults to false.
```

## Terrain

### Light Sources

- Vinifera implements light sources for TerrainTypes.

In `RULES.INI`:
```ini
[TerrainType]
IsLightEnabled=<boolean>  ; Does this terrain object emit light? (default false).
LightVisibility=<integer>  ; This terrain object radiates this amount of light (default 5000).
LightIntensity=<float>  ; The distance that this light is visible from (default 0).
LightRedTint=<float>  ; The red tint of this terrain objects light (default 1.0).
LightGreenTint=<float>  ; The green tint of this terrain objects light (default 1.0).
LightBlueTint=<float>  ; The blue tint of this terrain objects light (default 1.0).
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
[ThemeType]
RequiredAddon=<AddonType>  ; The addon required to be active for this theme to be available. Currently, only 1 (Firestorm) is supported. Defaults to 0 (none).
```

## Vehicles

### Totable

- Vinifera adds a new flag which can prevent a vehicle from being picked up by a Carryall.

In `RULES.INI`:
```ini
[UnitType]
Totable=<boolean>  ; Can this unit be picked up by a Carryall? Defaults to yes.
```

### UnloadingClass

- Vinifera adds support for a custom unloading class when a harvester is unloading at a refinery. In addition to this working for regular harvesters, this will now work on harvesters with `Weeder=yes` for when they dock at a building that has `Weeder=yes`.

In `RULES.INI`:
```ini
[UnitType]
UnloadingClass=<UnitType>  ; UnitType whose image will be used when this harvester is docked. Defaults to [AudioVisual]->UnloadingClass
```

### More Graphic Facings

- The engine now supports 16, 32 and 64 graphic facings for UnitTypes.

In `RULES.INI`:
```ini
[UnitType]
StartTurretFrame=<integer>  ; The starting turret frame index, allowing them to be adjusted manually if required. Defaults to -1 (not used).
TurretFacings=<integer>  ; The turret facing count. Defaults to 32.
```

- Additionally, the `Anim=` INI key for WeaponTypes will now read the number of entries that matches the firing objects Facings= entry.
- Because of the new extended facing support, it was observed that the buffer size was too small and has now been increased to allow a larger entry to accommodate a larger facing count.

## Warheads

### `[Weapons]` Section

- Vinifera implements the reading of a new `RULES.INI` section, `[Weapons]`, to allow the definition of WeaponTypes.
This is to fix the issue known as the "Weed Guy" hack, and ensure all weapons are allocated in the WeaponsType heaps before any weapon lookup or loading is performed.

### Various Keys Ported from Red Alert 2

- Vinifera implements various WarheadType keys from Red Alert 2.

In `RULES.INI`:
```ini
[WarheadType]
WallAbsoluteDestroyer=<boolean>  ; Does this warhead instantly destroy walls regardless of the warhead damage value? Defaults to no.
AffectsAllies=<boolean>  ; Can this warhead damage friendly units? Defaults to yes.
CombatLightSize=<boolean>  ; This is used to override the size of the combat light flash at the point of impact for Warheads with Bright=yes set (Bright=yes must also be set on the Weapon using this warhead). Defaults to 0.0. Ranges between 0.0 (uses the default behaviour seen with Bright=yes) and 1.0 (full size).
```

- Shake Screen Controls
These values are used to shake the screen when the projectile impacts. All of these values default to 0 and do not support negative values.
In `RULES.INI`:
```ini
[WarheadType]
ShakeXhi=<unsigned integer>  ; Maxiumum Y pixel movement.
ShakeXlo=<unsigned integer>  ; Minimum Y pixel movement.
ShakeXhi=<unsigned integer>  ; Maxiumum X pixel movement.
ShakeXlo=<unsigned integer>  ; Minimum X pixel movement.
```

## Weapons

### Electric Bolts

- Vinifera implements the Electric Bolt (aka. "Tesla Bolts") weapon effect from Red Alert 2, with additional controls.

In `RULES.INI`:
```ini
[WeaponType]
IsElectricBolt=<boolean>  ; Is this weapon an electric bolt? This is required to enable the drawing feature. Defaults to no.
                     ; Electric bolts are made up of 3 lines, these values define the colours for each of the lines.
EBoltColor1=<r,g,b>  ; Defaults to 255,255,255.
EBoltColor2=<r,g,b>  ; Defaults to 82,81,255.
EBoltColor3=<r,g,b>  ; Defaults to 82,81,255.
EBoltSegmentCount=<integer>  ; How many segment blocks should the electric bolt be made up from. A larger number will give a more "wild" effect. Defaults to 8.
EBoltLifetime=<integer>  ; The lifetime of the electric bolt graphic in game frames. Defaults to 17.
EBoltIterations=<integer>  ; How many draw iterations should the system perform? Defaults to 1.
EBoltDeviation=<float>  ; The maximum deviation from a straight line the electric bolts can be. A value of 0.0 will draw straight lines. Defaults to 1.0.
```

```{warning}
Electric bolts are currently known to potentially cause issues on save/load.
```

![GIF 08-09-2021 19-17-13](https://user-images.githubusercontent.com/73803386/132563132-7ebb771f-8acf-4ee2-ba4b-8dfa8a01de8f.gif)

### Suicide

- Vinifera adds the `Suicide` key or WeaponTypes from Red Alert 2, and adds an additional control `DeleteOnSuicide` for alternative behaviour.

In `RULES.INI`:
```ini
[WeaponType]
Suicide=<boolean>  ; Will the firing unit commit suicide when this weapon is fired? Defaults to no.
DeleteOnSuicide=<boolean>  ; Logical option for Suicide=yes which will instantly remove the unit from the game world instead of dealing full damage. Defaults to no.
```

```{note}
`DeleteOnSuicide=yes` mimics Red Alert 2 behavior.
```