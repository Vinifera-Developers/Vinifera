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

## Harvesters

- In the original game, harvesters always prefer free refineries over occupied ones, even if the free refinery was much farther away than the occupied refinery. Vinifera fixes this so that harvesters now prefer queueing to occupied refineries if they are much closer than free refineries. The distance for this preference is customizable.

In `RULES.INI`:
```
[General]
; When looking for refineries, harvesters will prefer a distant free
; refinery over a closer occupied refinery if the refineries' distance
; difference in cells is less than this.
MaxFreeRefineryDistanceBias=16
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

In `ART.INI`:
```ini
[SOMEBULLET]  ; BulletType
SpawnDelay=3  ; unsigned integer, the number of frames between each of the spawned trailer animations.
```

### Torpedoes

- Vinifera ports the Torpedo logic from Red Alert 1. Torpedoes can only be fired at targets on water. Additionally, torpedoes explode when they collide with land or an enemy unit.

In `RULES.INI`:
```ini
[SOMEBULLET]
Torpedo=yes   ; boolean, is this projectile considered a torpedo?
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
RegularPowerPlant=   ; BuildingType, the regular power plant this side uses.
AdvancedPowerPlant=  ; BuildingType, the advanced power plant this side uses.
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

## Super Weapons

- Vinifera allows customizing the mouse cursor used for ranged super weapons (currently, the EMPulse Cannon).

In `RULES.INI`:
```ini
[SOMESUPERWEAPON]              ; SuperWeaponType
ActionOutOfRange=EMPulseRange  ; ActionType, the action used by this super weapon when it's out of range.
```

## Mouse Cursors and Actions

### Mouse Cursors

- Vinifera implements a new system to customize mouse cursor type properties, as well as add new mouse cursors.
- Mouse cursors are specified in a new `INI` file, `MOUSE.INI`. If the `INI` is not present, the game will default to the normal hardcoded mouse type properties.

```{note}
Vanilla cursors are always present implicitly, but their properties **can** be overridden in `MOUSE.INI`.
```

- <details>
    <summary>Basic `MOUSE.INI`</summary>

    ```ini
    ;============================================================================
    ; MOUSE.INI
    ;
    ; This control file is used to control the frames and rates of the various
    ; mouse pointers.
    ;
    ; $Author: $
    ; $Archive: $
    ; $Modtime: $
    ; $Revision: $
    ;============================================================================


    ; ******* Mouse List *******
    ; Lists the mouse types in this control file. Each mouse pointer is given a
    ; unique (internal only) identifier name, these can not be renamed or removed!
    ;
    ; FORMAT;
    ;   <StartFrame>,<FrameCount>,<FrameRate>,<SmallFrame>,<SmallFrameCount>,<SmallFrameRate>,<HotspotX>,<HotspotY>
    ;
    ;   A hotspot value can either be an integer value (ranged between the negative value of the width/height
    ;   of the shape frame, and the positive value of the width/height of the shape frame), or one of the
    ;   following hotspot types;
    ;      HotspotX = left, center, right
    ;      HotspotY = top, middle, bottom
    ;
    ; NOTE: 
    ;   A SmallFrame value of "-1" means it will use the normal pointer when
    ;   the mouse is over the radar panel.

    [MouseTypes]
    Normal=0,1,0,1,1,0,left,top
    ScrollN=2,1,0,-1,1,0,center,top
    ScrollNE=3,1,0,-1,1,0,right,top
    ScrollE=4,1,0,-1,1,0,right,middle
    ScrollSE=5,1,0,-1,1,0,right,bottom
    ScrollS=6,1,0,-1,1,0,center,bottom
    ScrollSW=7,1,0,-1,1,0,left,bottom
    ScrollW=8,1,0,-1,1,0,left,middle
    ScrollNW=9,1,0,-1,1,0,left,top
    NoScrollN=10,1,0,-1,1,0,center,top
    NoScrollNE=11,1,0,-1,1,0,right,top
    NoScrollE=12,1,0,-1,1,0,right,middle
    NoScrollSE=13,1,0,-1,1,0,right,bottom
    NoScrollS=14,1,0,-1,1,0,center,bottom
    NoScrollSW=15,1,0,-1,1,0,left,bottom
    NoScrollW=16,1,0,-1,1,0,left,middle
    NoScrollNW=17,1,0,-1,1,0,left,top
    CanSelect=18,13,4,-1,13,4,center,middle
    CanMove=31,10,4,42,10,4,center,middle
    NoMove=41,1,0,52,1,0,center,middle
    StayAttack=53,5,4,63,5,4,center,middle
    CanAttack=58,5,4,63,5,4,center,middle
    AreaGuard=68,5,4,73,5,4,center,middle
    Tote=78,10,4,-1,10,4,center,middle
    NoTote=88,1,0,-1,1,0,center,middle
    Enter=89,10,4,100,10,4,center,middle
    NoEnter=99,1,0,63,1,0,center,middle
    Deploy=110,9,4,-1,9,4,center,middle
    NoDeploy=119,1,0,-1,1,0,center,middle
    Undeploy=120,9,4,-1,9,4,center,middle
    Sell=129,10,4,-1,10,4,center,middle
    SellUnit=139,10,4,-1,10,4,center,middle
    NoSell=149,1,0,-1,1,0,center,middle
    GRepair=150,20,4,-1,20,4,center,middle
    Repair=170,20,4,-1,20,4,center,middle
    NoRepair=190,1,0,-1,1,0,center,middle
    Waypoint=191,10,4,-1,10,4,center,middle
    PlaceWaypoint=201,10,4,-1,10,4,center,middle
    NoPlaceWaypoint=211,1,0,-1,1,0,center,middle
    SelectWaypoint=212,7,4,-1,7,4,center,middle
    EnterWaypointMode=219,10,4,-1,10,4,center,middle
    FollowWaypoint=229,10,4,-1,10,4,center,middle
    ToteWaypoint=239,10,4,-1,10,4,center,middle
    RepairWaypoint=249,10,4,-1,10,4,center,middle
    AttackWaypoint=259,10,4,-1,10,4,center,middle
    EnterWaypoint=269,10,4,-1,10,4,center,middle
    LoopWaypointPath=356,1,0,-1,1,0,center,middle
    AirStrike=279,20,4,-1,20,4,center,middle               ; Ion Cannon
    ChemBomb=299,10,4,-1,10,4,center,middle
    Demolitions=309,10,4,-1,10,4,center,middle
    NuclearBomb=319,10,4,-1,10,4,center,middle
    TogglePower=329,16,2,-1,16,2,center,middle
    NoTogglePower=345,1,0,-1,1,0,center,middle
    Heal=346,10,4,42,10,4,center,middle
    EMPulse=357,20,3,-1,20,3,center,middle
    EMPulseRange=377,1,0,-1,1,0,center,middle
    ScrollCoast=378,1,0,-1,1,0,center,middle
    ScrollCoastN=379,1,0,-1,1,0,center,middle
    ScrollCoastNE=380,1,0,-1,1,0,center,middle
    ScrollCoastE=381,1,0,-1,1,0,center,middle
    ScrollCoastSE=382,1,0,-1,1,0,center,middle
    ScrollCoastS=383,1,0,-1,1,0,center,middle
    ScrollCoastSW=384,1,0,-1,1,0,center,middle
    ScrollCoastW=385,1,0,-1,1,0,center,middle
    ScrollCoastNW=386,1,0,-1,1,0,center,middle
    PatrolWaypoint=387,10,4,-1,10,4,center,middle

    ```
   </details>

### Actions

- Vinifera also implements a new system to customize action type properties, as well as add action types cursors.
- Actions are specified in a new `INI` file, `ACTION.INI`. If the `INI` is not present, the game will default to the normal hardcoded action type properties.

```{note}
Vanilla actions are always present implicitly, but their properties **can** be overridden in `ACTION.INI`.
```

- <details>
    <summary>Basic `ACTION.INI`</summary>
    
    ```ini
    ;============================================================================
    ; ACTION.INI
    ;
    ; This control file is used to control the mouse pointers used for various actions.
    ;
    ; $Author: $
    ; $Archive: $
    ; $Modtime: $
    ; $Revision: $
    ;============================================================================


    ; ******* Action List *******
    ; Lists the action types in this control file. Each action is given a
    ; unique (internal only) identifier name, these can not be renamed or removed!
    ;
    ; FORMAT;
    ;   <Mouse>,<ShadowMouse>
    ;
    ;   Mouse and ShadowMouse must be names of mouse types, listed in MOUSE.INI.
    ;   ShadowMouse is shown when the mouse pointer is over shroud, Mouse is shown otherwise.

    [ActionTypes]
    None=Normal,Normal
    Move=CanMove,CanMove
    NoMove=NoMove,NoMove
    Enter=Enter,Normal
    Self=Deploy,Normal
    Attack=CanAttack,CanMove
    Harvest=CanAttack,Normal
    Select=CanSelect,Normal
    ToggleSelect=CanSelect,Normal
    Capture=Enter,Normal
    Repair=Repair,NoRepair
    Sell=Sell,NoSell
    SellUnit=SellUnit,NoSell
    NoSell=NoSell,NoSell
    NoRepair=NoRepair,NoRepair
    Sabotage=Demolitions,Normal
    Tote=Tote,NoTote
    DontUse2=Normal,Normal
    DontUse3=Normal,Normal
    Nuke=NuclearBomb,NuclearBomb
    DontUse4=Normal,Normal
    DontUse5=Normal,Normal
    DontUse6=Normal,Normal
    DontUse7=Normal,Normal
    DontUse8=Normal,Normal
    GuardArea=AreaGuard,AreaGuard
    Heal=Heal,Heal
    Damage=Enter,Normal
    GRepair=GRepair,Normal
    NoDeploy=NoDeploy,NoDeploy
    NoEnter=NoEnter,NoEnter
    NoGRepair=NoRepair,NoRepair
    TogglePower=TogglePower,NoTogglePower
    NoTogglePower=NoTogglePower,NoTogglePower
    EnterTunnel=Enter,Normal
    NoEnterTunnel=Normal,NoEnter
    EMPulse=EMPulse,EMPulse
    IonCannon=AirStrike,AirStrike
    EMPulseRange=EMPulseRange,EMPulseRange
    ChemBomb=ChemBomb,ChemBomb
    PlaceWaypoint=PlaceWaypoint,PlaceWaypoint
    NoPlaceWaypoint=NoPlaceWaypoint,NoPlaceWaypoint
    EnterWaypointMode=EnterWaypointMode,EnterWaypointMode
    FollowWaypoint=FollowWaypoint,FollowWaypoint
    SelectWaypoint=SelectWaypoint,SelectWaypoint
    LoopWaypointPath=LoopWaypointPath,LoopWaypointPath
    DragWaypoint=Normal,Normal
    AttackWaypoint=AttackWaypoint,AttackWaypoint
    EnterWaypoint=EnterWaypoint,EnterWaypoint
    PatrolWaypoint=PatrolWaypoint,PatrolWaypoint
    DropPod=AirStrike,AirStrike
    RallyToPoint=CanMove,Normal
    AttackSupport=Normal,Normal

    ```
   </details>

## Technos

### Spawners

- Vinifera ports the spawn manager, responsible for AircraftType missiles and aircraft carriers from Red Alert 2.

In `RULES.INI`:
```ini
[SOMETECHNO]             ; TechnoType
Spawns=                  ; AircraftType, the type that this Techno spawns.
SpawnsNumber=0           ; integer, the number of aircraft that this Techno spawns.
SpawnRegenRate=0         ; integer, the time it takes for a spawn to regenerate after death.
SpawnReloadRate=0        ; integer, the time it takes for a spawn to reload its ammo and restore its strength.
SpawnSpawnRate=20        ; integer, the time between two consecutive spawns being created.
SpawnLogicRate=10        ; integer, the delay with which the spawn manager processes its logic.
MaxRandomSpawnOffset=0   ; integer, if greater than 0, the spawn offset will be randomly shifted in the horizontal plane by up to this many leptons.
```

- Additionally, it's possible to specify an alternative voxel model to use when the spawner has no spawns docked.

In `RULES.INI`:
```ini
[SOMETECHNO]        ; TechnoType
NoSpawnAlt=no       ; boolean, should this Techno use an alternative voxel model when it's out of spawns?
                    ; When true, the model named SOMETECHNOWO will be used when it's out of spawns.
```

```{note}
`NoSpawnAlt` is only available for Technos that use voxel models.
```

```{note}
Unlike in Red Alert 2, the voxel model used by `NoSpawnAlt` is loaded into a separate area of memory from the turret model. This means that Technos that have `NoSpawnAlt=yes` set **can** have turrets.
```

- For the unit to create spawns upon attack, a flag needs to be set on its weapon.

In `RULES.INI`:
```ini
[SOMEWEAPON]  ; WeaponType
Spawner=no    ; boolean, does this weapon spawn aircraft when firing?
```

- Additionally, spawned objects should have `Spawned=yes` to prevent the announcement of their death by EVA.

In `RULES.INI`:
```ini
[SOMETECHNO]  ; TechnoType
Spawned=no    ; boolean, is this object meant to be spawned, either by a spawner, or off-map?
```

#### Rockets

- For rocket launchers to function, RocketTypes need to be defined.

In `RULES.INI`:
```ini
[RocketTypes]
0=SOMEROCKET

[SOMEROCKET]            ; RocketType
PauseFrames=0           ; integer, how many frames the rocket pauses on the launcher before tilting.
TiltFrames=60           ; integer, how many frames it takes for the rocket to tilt to firing position.
PitchInitial=0.21       ; float, starting pitch of the rocket before tilting up (0 = horizontal, 1 = vertical).
PitchFinal=0.5          ; float, ending pitch of the rocket after tilting up before it fires.
TurnRate=0.05           ; float, pitch maneuverability of rocket in air.
RaiseRate=1             ; integer, how much the missile will raise each turn on the launcher (for Cruise Missiles only).
Acceleration=0.4        ; float, this much is added to the rocket's velocity each frame during launch.
Altitude=768            ; integer, cruising altitude in leptons (1/256 of a cell): at this height rocket begins leveling off.
Damage=200              ; integer, the rocker does this much damage when it explodes.
EliteDamage=400         ; integer, the rocker does this much damage when it explodes when the spawner is elite.
BodyLength=256          ; integer, the length of the body of the rocket in leptons (1/256 of a cell).
LazyCurve=yes           ; boolean, is the rocket's path a big, lazy curve, like the V3 is Red Alert 2.
CruiseMissile=no        ; boolean, is this rocket a Cruise Missile, like Boomer missiles in Yuri's Revenge.
CloseEnoughFactor=1.0   ; float, LazyCurve=no rockets begin turning toward their target after flying parallel to the ground when their horizontal distance from the target is a specified multiple of their vertical distance.
Type=                   ; AircraftType, the type this rocket is associated with.
Warhead=                ; WarheadType, the warhead that this rocket's explosion uses.
EliteWarhead=           ; WarheadType, the warhead that this rocket's explosion uses when the spawner is elite.
TakeoffAnim=            ; AnimType, the takeoff animation used by this rocket.
TrailAnim=              ; AnimType, the trail animation used by this rocket.
TrailSpawnDelay=3       ; integer, the delay after a trail anim is spawned until the next one spawns.
TrailAppearDelay=2      ; integer, the delay before a newly spawned trail anim appears.
Inaccuracy=0            ; integer, the maximum number of leptons that this rocket is able to miss its target by.
```

```{note}
The rocket object is an AircraftType, like in Red Alert 2. When determining its characteristics, the rocket will use the first RocketType whose `Type=` is equal to the type of the rocket itself.
```

- If `SpawnsNumber > 1`, missiles can be made to spawn at alternating offsets. For this, `Burst` on the `Weapon` needs to be set to a value greater than `1`. Up to `Burst` missiles will use alternating spawn offsets, and the rest will use the the odd missiles' spawn offset.
- Additionally, an additional offset for missiles spawned at even numbers can be defined.

In `ART.INI`:
```ini
[SOMETECHNO]             ; TechnoType
SecondSpawnOffset=0,0,0  ; 3 integers, an offset to be added to the firing FLH for the every second spawn's location.
```

- Additionally, missile aircraft should use rocket locomotion, as well as have `MissileSpawn=yes`.

In `RULES.INI`:
```ini
[SOMETECHNO]     ; TechnoType
Locomotor={B7B49766-E576-11d3-9BD9-00104B972FE8}  ; Rocket locomotion
MissileSpawn=no                                   ; boolean, is this object a missile meant to be spawned?
```

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
BaseArmor=          ; ArmorType, the armor that provides the default values for this armor (this includes the `Verses=` or `Modifier.*=` values defined for the armor in every warhead's section).
```

```{warning}
Make sure not to specify `BaseArmor` recursively (in a loop).
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
Modifier.myarmor=100%       ; % or float, the damage multiplier against a this armor type.
ForceFire.myarmor=yes       ; boolean, whether this warhead can be used to force-fire at a this armor type.
Retaliate.myarmor=yes       ; boolean, whether this warhead can be used to retaliate against a this armor type.
PassiveAcquire.myarmor=yes  ; boolean, whether this warhead can be used to passive acquire a this armor type.
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

### Alternative Water Image 

- `WaterAlt` can now be used to control whether a voxel unit uses a different model when in water, similar to the APC in vanilla.

In `RULES.INI`:
```ini
[SOMETECHNO]  ; TechnoType
WaterAlt=no   ; boolean, should this Techno use a different voxel model when in water. Defaults to yes for `[APC]`, no for other Technos.
              ; When true, the model named SOMETECHNOW will be used when it's in water.
```

```{note}
Unlike the APC in vanilla, the alternative model is loaded into a separate area of memory from the turret model. This means that Technos that have `WaterAlt=yes` set **can** have turrets.
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

### DontScore

- Vinifera ports the `DontScore` key from Red Alert 2. When `DontScore=yes` is set, killing the unit won't grant the killer experience, or give score to it's owner's house. It will, however, count towards your lost unit count.

In `RULES.INI`:
```ini
[SOMETECHNO]  ; TechnoType
DontScore=no  ; boolean, should this Techno not count towards promotion and multiplayer score?
```

### Build Time

- By default, the build time of an object is based on its cost. Vinifera allows customizing the build time of an object independently of its cost.

In `RULES.INI`:
```ini
[SOMETECHNO]
BuildTimeCost=300 ; integer, specifies the object's build time.
                  ; for example, setting this to 300 makes the object build as fast as a 300-cost object, regardless of its actual cost.
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
DamageToInfantry=  ; integer, the damage to infantry per tick, defaults to Power / 10, but a minimum of 1
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

- Similarly, the `Anim=` INI key for WeaponTypes now also supports 16, 32 and 64 entries.
- Because of the new extended facing support, it was observed that the buffer size was too small and has now been increased to allow a larger entry to accommodate a larger facing count. Mind that the maximum string length is 506 characters now, so be sure to use short names if you want to have 64 entries.

## Warheads

### MinDamage

- Vinifera allows customizing the minimum damage dealt using a specific warhead.

In `RULES.INI`:
```ini
[SOMEWARHEAD]  ; WarheadType
MinDamage=-1   ; integer, the minimum damage dealt using the warhead. A negative value means to use [CombatDamage]->MinDamage.
```

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

### `[Weapons]` Section

- Vinifera implements the reading of a new `RULES.INI` section, `[Weapons]`, to allow the definition of WeaponTypes.
This is to remove the need for the work-around known as the "Weed Guy" hack, and ensure all weapons are allocated in the WeaponsType heaps before any weapon lookup or loading is performed.

### Custom Attack Cursor

- Vinifera allows setting a custom attack cursor used by a weapon.

In `RULES.INI`:
```ini
[SOMEWEAPON]             ; WeaponType
CursorAttack=Attack      ; ActionType, the action whose cursor properties will be used for this weapon's attack cursor when the unit is not in range of the target.
CursorStayAttack=Attack  ; ActionType, the action whose cursor properties will be used for this weapon's attack cursor when the unit is in range of the target.
```

```{note}
While there is no vanilla action for `StayAttack`, when `CursorStayAttack=Attack`, the `StayAttack` mouse cursor will be used.
```

```{note}
Both `CursorAttack` and `CursorStayAttack` are only used when not hovering over shroud.
```

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
