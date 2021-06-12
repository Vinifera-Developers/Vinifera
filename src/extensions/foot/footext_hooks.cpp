/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FOOTEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended FootClass.
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
#include "footext_hooks.h"
#include "foot.h"
#include "technotype.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-192
 * 
 *  IsInsignificant is not checked on FootClass objects.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_FootClass_Death_Announcement_IsInsignifcant_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, ecx);
    static const TechnoTypeClass *technotype;

    /**
     *  Stolen bytes/code here.
     */
    _asm { sub esp, 0x10 }

    /**
     *  Don't announce the death of objects we don't own.
     */
    if (!this_ptr->IsOwnedByPlayer) {
        goto function_return;
    }

    /**
     *  If this object is marked as "Insignificant", then the user
     *  should not hear any EVA notification when it is killed.
     */
    technotype = this_ptr->Techno_Type_Class();
    if (technotype->IsInsignificant) {
        goto function_return;
    }

    /**
     *  Continues to the Speak() call.
     */
continue_function:
    _asm { mov ecx, this_ptr }
    JMP(0x004A4D6D);

    /**
     *  Return from function.
     */
function_return:
    JMP(0x004A4DB5);
}


/**
 *  Main function for patching the hooks.
 */
void FootClassExtension_Hooks()
{
    /**
     *  #issue-192
     * 
     *  IsInsignificant is not checked on FootClass objects.
     */
    Patch_Jump(0x004A4D60, &_FootClass_Death_Announcement_IsInsignifcant_Patch);
}
