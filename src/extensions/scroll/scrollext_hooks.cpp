/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera (Dawn of the Tiberium Age Build)
 *
 *  @file          SCROLLEXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended ScrollClass.
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
#include "asserthandler.h"
#include "building.h"
#include "debughandler.h"
#include "fatal.h"
#include "house.h"
#include "housetype.h"
#include "mouse.h"
#include "particlesys.h"
#include "particlesysext_hooks.h"
#include "rules.h"
#include "scenario.h"
#include "techno.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "unit.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "syringe.h"


bool Passes_Cloak_Check(TechnoClass* techno)
{
    if (PlayerPtr->Is_Ally(techno)) {
        return true;
    }

    Coord coord = techno->Center_Coord();
    if (Map[coord].Sensed_By(PlayerPtr->HeapID)) {
        return true;
    }

    return false;
}


/**
 *  Fixes a bug where mouse input is not handled on cloaked objects of allied players.
 *  Sadly, the bug appears to be caused by a function that is inlined or copy-pasted
 *  in multiple places, so we need multiple patches.
 *
 *  Author: Rampastring
 */
EXPORT_FUNC(_ScrollClass_Input_Allied_Cloaked_Object_Patch1)
{
    GET(TechnoClass*, techno, ESI);

    if (Passes_Cloak_Check(techno)) {
        // Target object is cloaked but visible to us (allied or sensed),
        // handle mouse input on it normally.
        return 0x005E886B;
    }

    // Target object is cloaked and not visible to us.
    return 0x005E887F;
}


/**
 *  Fixes a bug where mouse input is not handled on cloaked buildings of allied players.
 *
 *  Author: Rampastring
 */
EXPORT_FUNC(_ScrollClass_Input_Allied_Cloaked_Object_Patch2)
{
    GET(BuildingClass*, building, EDI);

    if (Passes_Cloak_Check(building)) {
        // Target object is visible to us (allied or sensed), handle mouse input on it normally.
        return 0x005E88D6;
    }

    // Target object is cloaked and not visible to us.
    return 0x005E88E6;
}


/**
 *  Fixes a bug where mouse input is not handled on cloaked objects of allied players.
 *  While this is in TacticalClass, the bug is identical to the bugs in ScrollClass
 *  and has an identical fix, which is why we include this hack here.
 *
 *  Author: Rampastring
 */
EXPORT_FUNC(_Tactical_Get_Object_At_Cell_Allied_Cloaked_Object_Patch)
{
    GET(TechnoClass*, techno, EAX);

    if (Passes_Cloak_Check(techno)) {
        // Target object is visible to us (allied or sensed), handle mouse input on it normally.
        return 0x0061680F;
    }

    // Target object is cloaked and not visible to us.
    return 0x00616811;
}


/**
 *  Main function for patching the hooks.
 */
void ScrollClassExtension_Hooks()
{

}

declhook(0x005E8840, _ScrollClass_Input_Allied_Cloaked_Object_Patch1, 0);
declhook(0x005E88AA, _ScrollClass_Input_Allied_Cloaked_Object_Patch2, 0);
declhook(0x006167E3, _Tactical_Get_Object_At_Cell_Allied_Cloaked_Object_Patch, 0);
