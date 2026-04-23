/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended DriveLocomotorClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "drivelocomotionext_hooks.h"

#include "anim.h"
#include "cell.h"
#include "drivelocomotion.h"
#include "extension.h"
#include "foot.h"
#include "footext.h"
#include "hooker.h"
#include "rules.h"
#include "syringe.h"
#include "technotype.h"
#include "technotypeext.h"


/**
 *  Re-implements the section of code that spawns the wake animation as a
 *  unit moves across water.
 *
 *  @author: CCHyper
 */
static void DriveLocomotionClass_Process_Create_WakeAnim(DriveLocomotionClass *this_ptr)
{
    FootClass *linked_foot = this_ptr->LinkedTo;
    FootClassExtension *linked_footext = Extension::Fetch(linked_foot);
    TechnoTypeClassExtension *technotype_ext = Extension::Fetch(linked_foot->TClass);

    /**
     *  Wakes are only created if the unit is on water and not on a bridge.
     */
    if (linked_foot->IsOnBridge || linked_foot->Get_Cell_Ptr()->Land_Type() != LAND_WATER) {
        return;
    }

    /**
     *  If this unit shouldn't show wakes when cloaked, hide the idle wake and return.
     */
    if (technotype_ext->IsHideWakeWhenCloaked && linked_foot->Cloak == CLOAKED) {
        if (linked_footext->IdleWakeAnim) {
            linked_footext->IdleWakeAnim->Make_Invisible();
        }
        return;
    }

    if (technotype_ext->IdleWakeAnim) {

        /**
         *  If the idle wake animation already exists, toggle its visibility
         */
        if (linked_footext->IdleWakeAnim) {

            if (this_ptr->Is_Moving()) {
                linked_footext->IdleWakeAnim->Make_Invisible();
            } else {
                linked_footext->IdleWakeAnim->Make_Visible();
            }

        } else {

            /**
             *  Otherwise, create the wake animation at the current object's coordinate and attach it to follow the object.
             */
            linked_footext->IdleWakeAnim = new AnimClass(technotype_ext->IdleWakeAnim, linked_foot->PositionCoord);
            linked_footext->IdleWakeAnim->Attach_To(linked_foot);

            if (this_ptr->Is_Moving()) {
                linked_footext->IdleWakeAnim->Make_Invisible();
            }
        }
    }

    /**
     *  If the unit is moving, spawn the wake animation.
     */
    if (this_ptr->Is_Moving()) {

        /**
         *  #issue-944
         * 
         *  Only spawn the wake animation every 'X' frames, as set by the objects properties.
         */
        if (!(Frame % technotype_ext->WakeAnimRate)) {

            /**
             *  #issue-944
             *
             *  Fetch the wake animation from the object attached to this
             *  locomotor, and fall-back to the Rules wake animation if
             *  one is not defined.
             */
            const AnimTypeClass *wake_anim = technotype_ext->WakeAnim != nullptr ? technotype_ext->WakeAnim : Rule->Wake;

            /**
             *  Create the wake animation at the current object's coordinate.
             */
            if (wake_anim) {
                new AnimClass(wake_anim, linked_foot->PositionCoord);
            }
        }
    }
}


/**
 *  #issue-944
 *
 *  Implement support for custom wake animations as a unit moves across water..
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0047DFDB, _DriveLocomotionClass_Process_WakeAnim_Patch, 0)
{
    GET(ILocomotion *, this_ptr, ESI);

    DriveLocomotionClass_Process_Create_WakeAnim(static_cast<DriveLocomotionClass*>(this_ptr));

    return 0x0047E05D;
}


/**
 *  Main function for patching the hooks.
 */
void DriveLocomotionClassExtension_Hooks()
{

}
