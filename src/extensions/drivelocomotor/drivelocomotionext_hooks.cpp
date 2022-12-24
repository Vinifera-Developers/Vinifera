/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DRIVELOCOMOTOREXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended DriveLocomotorClass.
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
#include "drivelocomotionext_hooks.h"
#include "drivelocomotion.h"
#include "foot.h"
#include "technotype.h"
#include "technotypeext.h"
#include "cell.h"
#include "rules.h"
#include "anim.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Re-implements the section of code that spawns the wake animation as a
 *  unit moves across water.
 * 
 *  @author: CCHyper
 */
static void DriveLocomotionClass_Process_Create_WakeAnim(DriveLocomotionClass *this_ptr)
{
    /**
     *  Only spawn the wake animation every 10 frames.
     */
    if (!(Frame % 10)) {

        FootClass *linked_foot = this_ptr->Linked_To();

        if (!linked_foot->IsOnBridge && linked_foot->Get_Cell_Ptr()->Land_Type() == LAND_WATER) {

            /**
             *  #issue-944
             * 
             *  Fetch the wake animation from the object attached to this
             *  locomotor, and fall-back to the Rules wake animation if
             *  one is not defined.
             */
            TechnoTypeClassExtension *technotype_ext = Extension::Fetch<TechnoTypeClassExtension>(linked_foot->Techno_Type_Class());
            const AnimTypeClass *wake_anim = technotype_ext->WakeAnim != nullptr ? technotype_ext->WakeAnim : Rule->Wake;

            /**
             *  Create the wake animation at the current objects coordinate.
             */
            if (wake_anim) {
                AnimClass *animptr = new AnimClass(wake_anim, linked_foot->Get_Coord());
                ASSERT(animptr != nullptr);
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
DECLARE_PATCH(_DriveLocomotionClass_Process_WakeAnim_Patch)
{
    GET_REGISTER_STATIC(DriveLocomotionClass *, this_ptr, esi);

    DriveLocomotionClass_Process_Create_WakeAnim(this_ptr);

    JMP(0x0047E05D);
}


/**
 *  Main function for patching the hooks.
 */
void DriveLocomotionClassExtension_Hooks()
{
    Patch_Jump(0x0047DFE8, &_DriveLocomotionClass_Process_WakeAnim_Patch);
}
