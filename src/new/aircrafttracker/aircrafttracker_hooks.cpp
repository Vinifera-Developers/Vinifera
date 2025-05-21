/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AIRCRAFTTRACKER_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for AircraftTrackerClass.
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

#include "aircrafttracker_hooks.h"

#include "aircrafttracker.h"
#include "extension.h"
#include "flylocomotion.h"
#include "footext.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "jumpjetlocomotion.h"
#include "spawnmanager.h"
#include "techno.h"
#include "technotype.h"
#include "kamikazetracker.h"
#include "levitatelocomotion.h"
#include "tibsun_globals.h"
#include "veinholemonster.h"
#include "vinifera_globals.h"
#include "voc.h"


void JJ_Track_Helper(JumpjetLocomotionClass* loco)
{
    FootClassExtension* extension;
    Cell last_cell;

    extension = Extension::Fetch<FootClassExtension>(loco->LinkedTo);
    last_cell = extension->Get_Last_Flight_Cell();

    if (last_cell == CELL_NONE) {
        AircraftTracker->Track(loco->LinkedTo);
    }
}


DECLARE_PATCH(_JumpjetLocomotionClass_State0_AircraftTrackerPatch)
{
    GET_REGISTER_STATIC(JumpjetLocomotionClass*, loco, edi);
    static FootClassExtension* extension;
    static Cell last_cell;

    JJ_Track_Helper(loco);

    // Stolen instruction
    loco->CurrentState = JumpjetLocomotionClass::ASCENDING;

    JMP(0x004F977E);
}


void JJ_Update_Position_Helper(JumpjetLocomotionClass* loco)
{
    FootClassExtension* linked_ext;
    Cell oldcell, newcell;

    linked_ext = Extension::Fetch<FootClassExtension>(loco->LinkedTo);

    oldcell = linked_ext->Get_Last_Flight_Cell();
    newcell = loco->LinkedTo->Get_Cell();

    if (newcell != oldcell) {
        AircraftTracker->Update_Position(loco->LinkedTo, oldcell, newcell);
    }
}


DECLARE_PATCH(_JumpjetLocomotionClass_State1_AircraftTrackerPatch)
{
    GET_REGISTER_STATIC(JumpjetLocomotionClass*, loco, esi);
    
    JJ_Update_Position_Helper(loco);

    // Stolen instructions (function epilogue)
    _asm pop edi
    _asm pop esi
    _asm add esp, 0x14

    JMP(0x004F9EBF);
}


DECLARE_PATCH(_JumpjetLocomotionClass_State3_AircraftTrackerPatch)
{
    GET_REGISTER_STATIC(JumpjetLocomotionClass*, loco, ecx);

    _asm pushad

    JJ_Update_Position_Helper(loco);

    _asm popad

    // Stolen instructions
    _asm mov esi, ecx
    _asm lea edx, [esp+0x10]

    JMP(0x004FA071);
}


DECLARE_PATCH(_JumpjetLocomotionClass_State4_AircraftTrackerPatch)
{
    GET_REGISTER_STATIC(JumpjetLocomotionClass*, loco, esi);

    AircraftTracker->Untrack(loco->LinkedTo);

    // Stolen instruction
    loco->CurrentState = JumpjetLocomotionClass::GROUNDED;

    JMP(0x004FA42E);
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class FlyLocomotionClassExt : public FlyLocomotionClass
{
public:
    void _Take_Off();
};


void FlyLocomotionClassExt::_Take_Off()
{
    if (LinkedTo->EMPFramesRemaining <= 0) {
        IsLanding = false;
        IsTakingOff = true;
        FlightLevel = LinkedTo->Techno_Type_Class()->Flight_Level();

        const auto extension = Extension::Fetch<FootClassExtension>(LinkedTo);
        if (extension->Get_Last_Flight_Cell() == CELL_NONE) {
            AircraftTracker->Track(LinkedTo);
        }

        if (LinkedTo->HeightAGL == 0) {
            LinkedTo->PrimaryFacing.Set(LinkedTo->SecondaryFacing.Desired());
        }

        Static_Sound(LinkedTo->Techno_Type_Class()->AuxSound1, LinkedTo->Get_Coord());
    }
}


DECLARE_PATCH(_FlyLocomotionClass_Movement_AI_AircraftTracker_Patch1)
{
    GET_REGISTER_STATIC(FlyLocomotionClass*, loco, edi);
    static FootClassExtension* linked_ext;
    static Cell oldcell, newcell;

    linked_ext = Extension::Fetch<FootClassExtension>(loco->LinkedTo);

    oldcell = linked_ext->Get_Last_Flight_Cell();
    newcell = loco->LinkedTo->Get_Cell();

    if (newcell != oldcell && loco->Is_Moving_Now()) {
        AircraftTracker->Update_Position(loco->LinkedTo, oldcell, newcell);
    }

    // Stolen instructions
    _asm mov ecx, [edi+4]
    _asm lea ebp, [edi+4]

    JMP(0x00499F57);
}


DECLARE_PATCH(_FlyLocomotionClass_Movement_AI_AircraftTracker_Patch2)
{
    GET_REGISTER_STATIC(FootClass*, linked_to, ecx);

    AircraftTracker->Untrack(linked_to);

    // Stolen instruction
    linked_to->HeightAGL = 0;

    JMP(0x0049A087);
}


DECLARE_PATCH(_FlyLocomotionClass_Process_Landing_AircraftTracker_Patch)
{
    GET_REGISTER_STATIC(FlyLocomotionClass*, loco, esi);

    AircraftTracker->Untrack(loco->LinkedTo);

    // Stolen instructions
    loco->CurrentSpeed = 0;
    loco->TargetSpeed = 0;

    _asm mov bp, 0
    JMP(0x0049B938);
}


void Levitate_Update_Position_Helper(LevitateLocomotionClass* loco)
{
    FootClassExtension* linked_ext;
    Cell oldcell, newcell;

    linked_ext = Extension::Fetch<FootClassExtension>(loco->LinkedTo);

    oldcell = linked_ext->Get_Last_Flight_Cell();
    newcell = loco->LinkedTo->Get_Cell();

    if (newcell != oldcell) {
        AircraftTracker->Update_Position(loco->LinkedTo, oldcell, newcell);
    }
}


DECLARE_PATCH(_LevitateLocomotionClass_State_AI_AircraftTracker_Patch)
{
    GET_REGISTER_STATIC(ILocomotion*, loco, ebp);
    static int result;

    Levitate_Update_Position_Helper(static_cast<LevitateLocomotionClass*>(loco));

    result = loco->Is_Moving();

    _asm mov eax, result
    JMP_REG(edi, 0x00500C01);
}


/**
 *  Main function for patching the hooks.
 */
void AircraftTracker_Hooks()
{
    Patch_Jump(0x004F9777, &_JumpjetLocomotionClass_State0_AircraftTrackerPatch);
    Patch_Jump(0x004F9EBA, &_JumpjetLocomotionClass_State1_AircraftTrackerPatch);
    Patch_Jump(0x004FA06B, &_JumpjetLocomotionClass_State3_AircraftTrackerPatch);
    Patch_Jump(0x004FA427, &_JumpjetLocomotionClass_State4_AircraftTrackerPatch);
    Patch_Jump(0x0049CB00, &FlyLocomotionClassExt::_Take_Off);
    Patch_Jump(0x00499F51, &_FlyLocomotionClass_Movement_AI_AircraftTracker_Patch1);
    Patch_Jump(0x0049A07D, &_FlyLocomotionClass_Movement_AI_AircraftTracker_Patch2);
    Patch_Jump(0x0049B92C, &_FlyLocomotionClass_Process_Landing_AircraftTracker_Patch);
    Patch_Jump(0x00500BFA, &_LevitateLocomotionClass_State_AI_AircraftTracker_Patch);
}
