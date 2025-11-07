/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SUPEREXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended SuperClass.
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
#include "superext_hooks.h"
#include "superext_init.h"
#include "superext.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "extension.h"
#include "unit.h"
#include "house.h"
#include "housetype.h"
#include "sideext.h"
#include "building.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "syringe.h"


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
EXPORT_FUNC(_SuperClass_Place_HunterSeeker_Type_Patch)
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
EXPORT_FUNC(_SuperClass_Place_NukeType)
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

declhook(0x0060C5DE, _SuperClass_Place_HunterSeeker_Type_Patch, 0);
declhook(0x0060C49E, _SuperClass_Place_NukeType, 0);
