/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          COMMANDEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended hotkey command class.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once

#include "extension.h"
#include "command.h"


class BuildingClass;
class HouseClass;


/**
 *  Based class for all new command classes.
 */
class ViniferaCommandClass : public CommandClass
{
    public:
        ViniferaCommandClass() : CommandClass(), IsDeveloper(false) {}
        virtual ~ViniferaCommandClass() {}

        virtual KeyNumType Default_Key() const = 0;

    public:
        /**
         *  Is this command only available in developer mode?
         */
        bool IsDeveloper;
};


/**
 *  
 */
class PNGScreenCaptureCommandClass : public ViniferaCommandClass
{
    public:
        PNGScreenCaptureCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
        virtual ~PNGScreenCaptureCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Enter the manual placement mode when a building is complete
 *  and pending placement on the sidebar.
 */
class ManualPlaceCommandClass : public ViniferaCommandClass
{
    public:
        ManualPlaceCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
        virtual ~ManualPlaceCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_Z); }
};


/**
 *  Skip to the previous playable music track.
 */
class PrevThemeCommandClass : public ViniferaCommandClass
{
    public:
        PrevThemeCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
        virtual ~PrevThemeCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_LBRACKET); }
};


/**
 *  Skip to the next playable music track.
 */
class NextThemeCommandClass : public ViniferaCommandClass
{
    public:
        NextThemeCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
        virtual ~NextThemeCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_RBRACKET); }
};


/**
 *  Scroll tactical map to the north-east.
 */
class ScrollNECommandClass : public ViniferaCommandClass
{
    public:
        ScrollNECommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
        virtual ~ScrollNECommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Scroll tactical map to the south-east.
 */
class ScrollSECommandClass : public ViniferaCommandClass
{
    public:
        ScrollSECommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
        virtual ~ScrollSECommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Scroll tactical map to the south-west.
 */
class ScrollSWCommandClass : public ViniferaCommandClass
{
    public:
        ScrollSWCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
        virtual ~ScrollSWCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};


/**
 *  Scroll tactical map to the north-west.
 */
class ScrollNWCommandClass : public ViniferaCommandClass
{
    public:
        ScrollNWCommandClass() : ViniferaCommandClass() { IsDeveloper = false; }
        virtual ~ScrollNWCommandClass() {}

        virtual const char *Get_Name() const override;
        virtual const char *Get_UI_Name() const override;
        virtual const char *Get_Category() const override;
        virtual const char *Get_Description() const override;
        virtual bool Process() override;

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }

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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
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

        virtual KeyNumType Default_Key() const override { return KeyNumType(KN_NONE); }
};

#ifndef DEBUG
/**
 *  Based class for all new developer/debug command classes.
 */
class ViniferaDebugCommandClass : public ViniferaCommandClass
{
    public:
        ViniferaDebugCommandClass() : ViniferaCommandClass() {}
        virtual ~ViniferaDebugCommandClass() {}
};
#endif
