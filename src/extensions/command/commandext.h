/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended hotkey command class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "command.h"
#include "tibsun_defines.h"
#include "vinifera_globals.h"

#include <map>
#include <unordered_set>


/**
 *  Base class for all new command classes.
 */
class ViniferaCommandClass : public CommandClass
{
public:
    ViniferaCommandClass() : CommandClass(), IsDeveloper(false), IsMultiplayerOnly(false) {}
    virtual ~ViniferaCommandClass() {}

    bool Developer_Only() const { return IsDeveloper; }
    bool Multiplayer_Only() const { return IsMultiplayerOnly; }

public:
    /**
     *  Is this command only available in developer mode?
     */
    bool IsDeveloper;

    /**
     *  Is this command only available in multiplayer games?
     */
    bool IsMultiplayerOnly;
};


/**
 *  Replacement for ScreenCaptureCommandClass.
 */
class PNGScreenCaptureCommandClass : public ViniferaCommandClass
{
public:
    PNGScreenCaptureCommandClass() : ViniferaCommandClass() {}
    virtual ~PNGScreenCaptureCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Replacement for DeleteWaypointCommandClass.
 */
class DeleteCommandClass : public ViniferaCommandClass
{
public:
    DeleteCommandClass() : ViniferaCommandClass() {}
    virtual ~DeleteCommandClass() {}

    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Enter the manual placement mode when a building is complete
 *  and pending placement on the sidebar.
 */
class ManualPlaceCommandClass : public ViniferaCommandClass
{
public:
    ManualPlaceCommandClass() : ViniferaCommandClass() {}
    virtual ~ManualPlaceCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Reproduces the last structure that was built.
 */
class RepeatLastBuildingCommandClass : public ViniferaCommandClass
{
public:
    RepeatLastBuildingCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
    virtual ~RepeatLastBuildingCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Reproduces the last infantry that was built.
 */
class RepeatLastInfantryCommandClass : public ViniferaCommandClass
{
public:
    RepeatLastInfantryCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
    virtual ~RepeatLastInfantryCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Reproduces the last unit that was built.
 */
class RepeatLastUnitCommandClass : public ViniferaCommandClass
{
public:
    RepeatLastUnitCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
    virtual ~RepeatLastUnitCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Reproduces the last aircraft that was built.
 */
class RepeatLastAircraftCommandClass : public ViniferaCommandClass
{
public:
    RepeatLastAircraftCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
    virtual ~RepeatLastAircraftCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Skip to the previous playable music track.
 */
class PrevThemeCommandClass : public ViniferaCommandClass
{
public:
    PrevThemeCommandClass() : ViniferaCommandClass() {}
    virtual ~PrevThemeCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Skip to the next playable music track.
 */
class NextThemeCommandClass : public ViniferaCommandClass
{
public:
    NextThemeCommandClass() : ViniferaCommandClass() {}
    virtual ~NextThemeCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Scroll tactical map to the north-east.
 */
class ScrollNECommandClass : public ViniferaCommandClass
{
public:
    ScrollNECommandClass() : ViniferaCommandClass() {}
    virtual ~ScrollNECommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Scroll tactical map to the south-east.
 */
class ScrollSECommandClass : public ViniferaCommandClass
{
public:
    ScrollSECommandClass() : ViniferaCommandClass() {}
    virtual ~ScrollSECommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Scroll tactical map to the south-west.
 */
class ScrollSWCommandClass : public ViniferaCommandClass
{
public:
    ScrollSWCommandClass() : ViniferaCommandClass() {}
    virtual ~ScrollSWCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Scroll tactical map to the north-west.
 */
class ScrollNWCommandClass : public ViniferaCommandClass
{
public:
    ScrollNWCommandClass() : ViniferaCommandClass() {}
    virtual ~ScrollNWCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Jump the tactical map camera to the west edge of the map.
 */
class JumpCameraWestCommandClass : public ViniferaCommandClass
{
public:
    JumpCameraWestCommandClass() : ViniferaCommandClass() {}
    virtual ~JumpCameraWestCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Jump the tactical map camera to the east edge of the map.
 */
class JumpCameraEastCommandClass : public ViniferaCommandClass
{
public:
    JumpCameraEastCommandClass() : ViniferaCommandClass() {}
    virtual ~JumpCameraEastCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Jump the tactical map camera to the north edge of the map.
 */
class JumpCameraNorthCommandClass : public ViniferaCommandClass
{
public:
    JumpCameraNorthCommandClass() : ViniferaCommandClass() {}
    virtual ~JumpCameraNorthCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Jump the tactical map camera to the south edge of the map.
 */
class JumpCameraSouthCommandClass : public ViniferaCommandClass
{
public:
    JumpCameraSouthCommandClass() : ViniferaCommandClass() {}
    virtual ~JumpCameraSouthCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggles the visibility of the super weapon timers on the tactical view.
 */
class ToggleSuperTimersCommandClass : public ViniferaCommandClass
{
public:
    ToggleSuperTimersCommandClass() : ViniferaCommandClass() {}
    virtual ~ToggleSuperTimersCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Switches the sidebar to the Building tab.
 */
class SetStructureTabCommandClass : public ViniferaCommandClass
{
public:
    SetStructureTabCommandClass() : ViniferaCommandClass() {}
    virtual ~SetStructureTabCommandClass() {}
    
    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Switches the sidebar to the Infantry tab.
 */
class SetInfantryTabCommandClass : public ViniferaCommandClass
{
public:
    SetInfantryTabCommandClass() : ViniferaCommandClass() {}
    virtual ~SetInfantryTabCommandClass() {}
    
    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Switches the sidebar to the Vehicles tab.
 */
class SetUnitTabCommandClass : public ViniferaCommandClass
{
public:
    SetUnitTabCommandClass() : ViniferaCommandClass() {}
    virtual ~SetUnitTabCommandClass() {}
    
    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Switches the sidebar to the Special tab.
 */
class SetSpecialTabCommandClass : public ViniferaCommandClass
{
public:
    SetSpecialTabCommandClass() : ViniferaCommandClass() {}
    virtual ~SetSpecialTabCommandClass() {}
    
    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Produces a memory dump on request.
 */
class MemoryDumpCommandClass : public ViniferaCommandClass
{
public:
    MemoryDumpCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~MemoryDumpCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Produces a log dump of all the game object CRC's
 */
class DumpHeapCRCCommandClass : public ViniferaCommandClass
{
public:
    DumpHeapCRCCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~DumpHeapCRCCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Produces a log dump of existing trigger instances
 */
class DumpTriggersCommandClass : public ViniferaCommandClass
{
public:
    DumpTriggersCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~DumpTriggersCommandClass() {}
    
    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggles the instant build cheat for the player.
 */
class InstantBuildCommandClass : public ViniferaCommandClass
{
public:
    InstantBuildCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~InstantBuildCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggles the instant build cheat for the AI.
 */
class AIInstantBuildCommandClass : public ViniferaCommandClass
{
public:
    AIInstantBuildCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~AIInstantBuildCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Forces the player to win the current game session.
 */
class ForceWinCommandClass : public ViniferaCommandClass
{
public:
    ForceWinCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ForceWinCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Forces the current multiplayer game to go out of sync (for testing).
 */
class ForceDesyncCommandClass : public ViniferaCommandClass
{
public:
    ForceDesyncCommandClass() : ViniferaCommandClass() {}
    virtual ~ForceDesyncCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Forces the player to lose the current game session.
 */
class ForceLoseCommandClass : public ViniferaCommandClass
{
public:
    ForceLoseCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ForceLoseCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Forces the player to blowup and lose the current game session.
 */
class ForceDieCommandClass : public ViniferaCommandClass
{
public:
    ForceDieCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ForceDieCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Take ownership of any selected objects.
 */
class CaptureObjectCommandClass : public ViniferaCommandClass
{
public:
    CaptureObjectCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~CaptureObjectCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Promote selected units.
 */
class VeterancyPromoteCommandClass : public ViniferaCommandClass
{
public:
    VeterancyPromoteCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~VeterancyPromoteCommandClass() {}

    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  A pointer to a function that classifies a TechnoClass by assigning it an integer tier from 0 to 2
 */
typedef int (*Classify_Function)(TechnoClass*);

extern std::map<Classify_Function, DynamicVectorClass<TechnoClass*>*> UnitFilterLastFullSelectionByClassifiers;


/**
 *  Cycles through green/veteran/elite units among the initially selected group.
 */
class VeterancyFilterCommandClass : public ViniferaCommandClass
{
public:
    VeterancyFilterCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
    virtual ~VeterancyFilterCommandClass() {}

    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Cycles through red/yellow/green health units among the initially selected group.
 */
class HealthFilterCommandClass : public ViniferaCommandClass
{
public:
    HealthFilterCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
    virtual ~HealthFilterCommandClass() {}

    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Adds lower-ranked units to already filtered veterans.
 */
class VeterancyFilterAddNextCommandClass : public ViniferaCommandClass
{
public:
    VeterancyFilterAddNextCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
    virtual ~VeterancyFilterAddNextCommandClass() {}

    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Adds units from the next health group (yellow, green) to already filtered veterans.
 */
class HealthFilterAddNextCommandClass : public ViniferaCommandClass
{
public:
    HealthFilterAddNextCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
    virtual ~HealthFilterAddNextCommandClass() {}

    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Enters beacon placement mode.
 */
class BeaconPlacementCommandClass : public ViniferaCommandClass
{
public:
    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Grants all available special weapons to the player.
 */
class SpecialWeaponsCommandClass : public ViniferaCommandClass
{
public:
    SpecialWeaponsCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~SpecialWeaponsCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Hands out free money to the player.
 */
class FreeMoneyCommandClass : public ViniferaCommandClass
{
public:
    FreeMoneyCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~FreeMoneyCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Fires a lightning bolt at the current mouse cursor location.
 */
class LightningBoltCommandClass : public ViniferaCommandClass
{
public:
    LightningBoltCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~LightningBoltCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Fires an ion blast bolt at the current mouse cursor location.
 */
class IonBlastCommandClass : public ViniferaCommandClass
{
public:
    IonBlastCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~IonBlastCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Spawns an explosion at the mouse cursor location.
 */
class ExplosionCommandClass : public ViniferaCommandClass
{
public:
    ExplosionCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ExplosionCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Spawns a large explosion at the mouse cursor location.
 */
class SuperExplosionCommandClass : public ViniferaCommandClass
{
public:
    SuperExplosionCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~SuperExplosionCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Exits the game completely.
 */
class BailOutCommandClass : public ViniferaCommandClass
{
public:
    BailOutCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~BailOutCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggles the ion storm on/off.
 */
class IonStormCommandClass : public ViniferaCommandClass
{
public:
    IonStormCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~IonStormCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Saves a snapshot of the current scenario state.
 */
class MapSnapshotCommandClass : public ViniferaCommandClass
{
public:
    MapSnapshotCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~MapSnapshotCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Removes the selected object(s) from the game world.
 */
class DeleteObjectCommandClass : public ViniferaCommandClass
{
public:
    DeleteObjectCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~DeleteObjectCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Spawn all buildable units and structures at mouse cursor location.
 */
class SpawnAllCommandClass : public ViniferaCommandClass
{
public:
    SpawnAllCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~SpawnAllCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;

private:
    bool Try_Unlimbo(TechnoClass *techno, Cell &cell);
};


/**
 *  Apply damage to all selected objects.
 */
class DamageCommandClass : public ViniferaCommandClass
{
public:
    DamageCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~DamageCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggle the elite status of the selected objects.
 */
class ToggleEliteCommandClass : public ViniferaCommandClass
{
public:
    ToggleEliteCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ToggleEliteCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Unlock all available build options for the player house.
 */
class BuildCheatCommandClass : public ViniferaCommandClass
{
public:
    BuildCheatCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~BuildCheatCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggles the visibility of the map shroud.
 */
class ToggleShroudCommandClass : public ViniferaCommandClass
{
public:
    ToggleShroudCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ToggleShroudCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Heal the selected objects by 50 hit points.
 */
class HealCommandClass : public ViniferaCommandClass
{
public:
    HealCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~HealCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggles if weapons do damage or not.
 */
class ToggleInertCommandClass : public ViniferaCommandClass
{
public:
    ToggleInertCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ToggleInertCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Dumps all the current AI house base node info to the log output.
 */
class DumpAIBaseNodesCommandClass : public ViniferaCommandClass
{
public:
    DumpAIBaseNodesCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~DumpAIBaseNodesCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggles the berzerk state of the selected infantry.
 */
class ToggleBerzerkCommandClass : public ViniferaCommandClass
{
public:
    ToggleBerzerkCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ToggleBerzerkCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Increase the shroud darkness by one step (cell).
 */
class EncroachShadowCommandClass : public ViniferaCommandClass
{
public:
    EncroachShadowCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~EncroachShadowCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Increase the fog of war by one step (cell).
 */
class EncroachFogCommandClass : public ViniferaCommandClass
{
public:
    EncroachFogCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~EncroachFogCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggles alliance with the selected objects house.
 */
class ToggleAllianceCommandClass : public ViniferaCommandClass
{
public:
    ToggleAllianceCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ToggleAllianceCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Adds 2000 power units to the player.
 */
class AddPowerCommandClass : public ViniferaCommandClass
{
public:
    AddPowerCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~AddPowerCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Places a random crate at the mouse location.
 */
class PlaceCrateCommandClass : public ViniferaCommandClass
{
public:
    PlaceCrateCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~PlaceCrateCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Displays cell coordinates of the mouse cursor.
 */
class CursorPositionCommandClass : public ViniferaCommandClass
{
public:
    CursorPositionCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~CursorPositionCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggle frame step mode to step through the game frame-by-frame (for inspection).
 */
class ToggleFrameStepCommandClass : public ViniferaCommandClass
{
public:
    ToggleFrameStepCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ToggleFrameStepCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Frame Step Only: Step forward 1 frame.
 */
class Step1FrameCommandClass : public ViniferaCommandClass
{
public:
    Step1FrameCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~Step1FrameCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Frame Step Only: Step forward 5 frames.
 */
class Step5FramesCommandClass : public ViniferaCommandClass
{
public:
    Step5FramesCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~Step5FramesCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Frame Step Only: Step forward 10 frames.
 */
class Step10FramesCommandClass : public ViniferaCommandClass
{
public:
    Step10FramesCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~Step10FramesCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggles AI control of the player house.
 */
class ToggleAIControlCommandClass : public ViniferaCommandClass
{
public:
    ToggleAIControlCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ToggleAIControlCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Cycle the camera between the starting waypoints on the map.
 */
class StartingWaypointsCommandClass : public ViniferaCommandClass
{
public:
    StartingWaypointsCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~StartingWaypointsCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Places a random infantry at the mouse cell.
 */
class PlaceInfantryCommandClass : public ViniferaCommandClass
{
public:
    PlaceInfantryCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~PlaceInfantryCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Places a random unit at the mouse cell.
 */
class PlaceUnitCommandClass : public ViniferaCommandClass
{
public:
    PlaceUnitCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~PlaceUnitCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Places tiberium at the mouse cell.
 */
class PlaceTiberiumCommandClass : public ViniferaCommandClass
{
public:
    PlaceTiberiumCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~PlaceTiberiumCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Reduce tiberium at the mouse cell.
 */
class ReduceTiberiumCommandClass : public ViniferaCommandClass
{
public:
    ReduceTiberiumCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ReduceTiberiumCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Places fully grown tiberium at the mouse cell.
 */
class PlaceFullTiberiumCommandClass : public ViniferaCommandClass
{
public:
    PlaceFullTiberiumCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~PlaceFullTiberiumCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Removes tiberium at the mouse cell.
 */
class RemoveTiberiumCommandClass : public ViniferaCommandClass
{
public:
    RemoveTiberiumCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~RemoveTiberiumCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Creates a meteor shower around the current mouse cell.
 */
class MeteorShowerCommandClass : public ViniferaCommandClass
{
    public:
        MeteorShowerCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~MeteorShowerCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;
};


/**
 *  Sends a meteor at the current mouse cell.
 */
class MeteorImpactCommandClass : public ViniferaCommandClass
{
    public:
        MeteorImpactCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
        virtual ~MeteorImpactCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;
};


/**
 *  Toggles the instant recharge cheat for the players super weapons.
 */
class InstantSuperRechargeCommandClass : public ViniferaCommandClass
{
public:
    InstantSuperRechargeCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~InstantSuperRechargeCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggles the instant recharge cheat for the AI player super weapons.
 */
class AIInstantSuperRechargeCommandClass : public ViniferaCommandClass
{
public:
    AIInstantSuperRechargeCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~AIInstantSuperRechargeCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Print CRC's
 */
class DumpNetworkCRCCommandClass : public ViniferaCommandClass
{
public:
    DumpNetworkCRCCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~DumpNetworkCRCCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Dumps all the type heaps to an output log.
 */
class DumpHeapsCommandClass : public ViniferaCommandClass
{
public:
    DumpHeapsCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~DumpHeapsCommandClass() {}
    
    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Reload Rules and Art.
 */
class ReloadRulesCommandClass : public ViniferaCommandClass
{
public:
    ReloadRulesCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ReloadRulesCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggle the in-game ImGui debug overlay window.
 */
class ToggleDebugOverlayCommandClass : public ViniferaCommandClass
{
public:
    ToggleDebugOverlayCommandClass() : ViniferaCommandClass() {}
    virtual ~ToggleDebugOverlayCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Toggle the developer-mode scenario debug window.
 */
class ToggleScenarioOverlayCommandClass : public ViniferaCommandClass
{
public:
    ToggleScenarioOverlayCommandClass() : ViniferaCommandClass() { IsDeveloper = true; }
    virtual ~ToggleScenarioOverlayCommandClass() {}

    virtual const char *Get_Name() const override;
    virtual const char *Get_UI_Name() const override;
    virtual const char *Get_Category() const override;
    virtual const char *Get_Description() const override;
    virtual bool Process() override;
};


/**
 *  Replacement for SelectSameTypeCommandClass.
 */
class SelectSameTypeImprovedCommandClass : public ViniferaCommandClass
{
public:
    SelectSameTypeImprovedCommandClass() : ViniferaCommandClass() {}
    virtual ~SelectSameTypeImprovedCommandClass() {}

    virtual const char* Get_Name() const override;
    virtual const char* Get_UI_Name() const override;
    virtual const char* Get_Category() const override;
    virtual const char* Get_Description() const override;
    virtual bool Process() override;
    static void Process_Callback(ObjectClass* object);
    inline static std::unordered_set<const TechnoTypeClass*> SelectionTypes;
    inline static DWORD LastExecutionTime = 0;
};
