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

#include "always.h"

#include "aircrafttracker_hooks.h"

#include "aircrafttracker.h"
#include "extension.h"
#include "flylocomotion.h"
#include "footext.h"
#include "hooker.h"
#include "jumpjetlocomotion.h"
#include "levitatelocomotion.h"
#include "spawnmanager.h"
#include "syringe.h"
#include "techno.h"
#include "technotype.h"
#include "tibsun_globals.h"
#include "veinholemonster.h"
#include "vinifera_globals.h"
#include "voc.h"


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
        FlightLevel = LinkedTo->TClass->Flight_Level();

        const auto extension = Extension::Fetch(LinkedTo);
        if (extension->Get_Last_Flight_Cell() == CELL_NONE) {
            AircraftTracker->Track(LinkedTo);
        }

        if (LinkedTo->HeightAGL == 0) {
            LinkedTo->PrimaryFacing.Set(LinkedTo->SecondaryFacing.Desired());
        }

        Static_Sound(LinkedTo->TClass->AuxSound1, LinkedTo->PositionCoord);
    }
}


DEFINE_HOOK(0x00499F51, _FlyLocomotionClass_Movement_AI_AircraftTracker_Patch1, 6)
{
    GET(FlyLocomotionClass*, loco, EDI);

    auto linked_ext = Extension::Fetch(loco->LinkedTo);

    Cell oldcell = linked_ext->Get_Last_Flight_Cell();
    Cell newcell = loco->LinkedTo->Get_Cell();

    if (newcell != oldcell && loco->Is_Moving_Now()) {
        AircraftTracker->Update_Position(loco->LinkedTo, oldcell, newcell);
    }

    return 0;
}


DEFINE_HOOK(0x0049A07D, _FlyLocomotionClass_Movement_AI_AircraftTracker_Patch2, 10)
{
    GET(FootClass*, linked_to, ECX);

    AircraftTracker->Untrack(linked_to);

    // Stolen instruction
    return 0;
}


DEFINE_HOOK(0x0049B92C, _FlyLocomotionClass_Process_Landing_AircraftTracker_Patch, 6)
{
    GET(FlyLocomotionClass*, loco, ESI);

    AircraftTracker->Untrack(loco->LinkedTo);

    return 0;
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class LevitateLocomotionClassExt : public LevitateLocomotionClass
{
public:
    void _func_4FDF80();
};


void LevitateLocomotionClassExt::_func_4FDF80()
{
    func_4FDF80();

    FootClassExtension* linked_ext = Extension::Fetch(LinkedTo);

    Cell oldcell = linked_ext->Get_Last_Flight_Cell();
    Cell newcell = LinkedTo->Get_Cell();

    if (newcell != oldcell) {
        AircraftTracker->Update_Position(LinkedTo, oldcell, newcell);
    }
}


/**
 *  Main function for patching the hooks.
 */
void AircraftTracker_Hooks()
{
    Patch_Jump(0x0049CB00, &FlyLocomotionClassExt::_Take_Off);
    Patch_Call(0x00500BF5, &LevitateLocomotionClassExt::_func_4FDF80);
}
