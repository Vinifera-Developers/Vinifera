/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended SuperClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "superext_hooks.h"

#include "building.h"
#include "extension.h"
#include "hooker.h"
#include "house.h"
#include "housetype.h"
#include "sideext.h"
#include "superext.h"
#include "superext_init.h"
#include "syringe.h"
#include "unit.h"


/**
 *  Helper function that creates a hunter-seeker for the house's side.
 *
 *  @author: ZivDero
 */
static UnitClass* Make_HunterSeeker(HouseClass* house)
{
    const auto side_ext = Extension::Fetch(Sides[house->Class->Side]);

    if (side_ext->HunterSeeker) {
        return new UnitClass(const_cast<UnitTypeClass*>(side_ext->HunterSeeker), house);
    }

    return nullptr;
}


/**
 *  Patch to use the hunter-seeker for the house's side.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0060C5DE, _SuperClass_Place_HunterSeeker_Type_Patch, 0)
{
    GET(SuperClass*, this_ptr, ESI);

    /**
     *  Fetch the hunter-seeker for this house's side.
     */
    UnitClass*  hunter_seeker = Make_HunterSeeker(this_ptr->House);
    R->ESI(hunter_seeker);

    /**
     *  If we've successfully created a hunter-seeker, proceed to launching it.
     */
    if (hunter_seeker) {
        return 0x0060C642;
    }

    /**
     *  Otherwise, abort (return).
     */
    else {
        return 0x0060C68F;
    }
}


/**
 *  Patch to use the actual SW HeapID when launching a missile,
 *  instead of the Type= number.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0060C49E, _SuperClass_Place_NukeType, 0)
{
    GET(SuperClass*, this_ptr, EAX);
    GET(BuildingClass*, launchsite, ESI);

    launchsite->field_298 = this_ptr->Class->HeapID;

    return 0x0060C4AA;
}


/**
 *  Main function for patching the hooks.
 */
void SuperClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    SuperClassExtension_Init();
}
