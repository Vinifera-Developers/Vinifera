/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for AircraftTrackerClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
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
