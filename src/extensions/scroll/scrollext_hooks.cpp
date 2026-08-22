/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended ScrollClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "building.h"
#include "hooker.h"
#include "house.h"
#include "mouse.h"
#include "particlesys.h"
#include "syringe.h"
#include "techno.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "windialog.h"


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
DEFINE_HOOK(0x005E8840, _ScrollClass_Input_Allied_Cloaked_Object_Patch1, 0)
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
DEFINE_HOOK(0x005E88AA, _ScrollClass_Input_Allied_Cloaked_Object_Patch2, 0)
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
DEFINE_HOOK(0x006167E3, _Tactical_Get_Object_At_Cell_Allied_Cloaked_Object_Patch, 0)
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
 *  Blocks the map from receiving inputs while any Windows dialogs are open.
 *
 *  Author: ZivDero
 */
DEFINE_HOOK(0x005E934E, _ScrollClass_Message_Handler_Block_Inputs_In_Dialogs_Patch, 6)
{
    if (WSDialogCount != 0) {
        return 0x005E961E;
    }
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void ScrollClassExtension_Hooks()
{

}
