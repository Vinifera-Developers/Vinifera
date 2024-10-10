/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ANIMGEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended AnimClass.
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
#include "animext.h"
#include "animtypeext.h"
#include "anim.h"
#include "animtype.h"
#include "fatal.h"
#include "vinifera_util.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi); // Current "this" pointer.
    static AnimTypeClassExtension *animtypeext;

    /**
     *  If we are performing a load operation, the Windows API will invoke the
     *  constructors for us as part of the operation, so we can skip our hook here.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }

    /**
     *  This following code was moved here due to a patching address conflict
     *  in animext_hooks.cpp and is not the normal approach when extending
     *  a game class. - CCHyper
     */

    /**
     *  #BUGFIX:
     * 
     *  This check was observed in Red Alert 2, so there must be an edge case
     *  where anims are created with a null type instance. So lets do that
     *  here and also report a warning to the debug log.
     */
    if (!this_ptr->Class) {
        goto destroy_anim;
    }

    /**
     *  Create an extended class instance.
     */
    Extension::Make<AnimClassExtension>(this_ptr);

    /**
     *  #issue-561
     * 
     *  Implements ZAdjust override for Anims. This will only have an effect
     *  if the anim is created with a z-adjustment value of "0" (default value).
     * 
     *  @author: CCHyper
     */
    if (!this_ptr->ZAdjust) {
        animtypeext = Extension::Fetch<AnimTypeClassExtension>(this_ptr->Class);
        this_ptr->ZAdjust = animtypeext->ZAdjust;
    }

original_code:
    /**
     *  Stolen bytes/code.
     */
    this_ptr->IsActive = true;

    /**
     *  Restore some registers.
     */
    _asm { mov ecx, this_ptr }
    _asm { mov edx, [ecx+0x64] } // this->Class
    _asm { mov ecx, edx }

    JMP_REG(edx, 0x00413C80);

    /**
     *  Report that the anim type instance was invalid.
     */
destroy_anim:
    DEBUG_WARNING("Anim: Invalid anim type instance!\n");

    /**
     *  Remove the anim from the game world.
     */
    this_ptr->Remove_This();
    
    _asm { mov esi, this_ptr }
    JMP_REG(edx, 0x00414157);
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Default_Constructor_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi); // Current "this" pointer.

    /**
     *  If we are performing a load operation, the Windows API will invoke the
     *  constructors for us as part of the operation, so we can skip our hook here.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }

    /**
     *  Create an extended class instance.
     */
    Extension::Make<AnimClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret }
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);

    /**
     *  If this anim instance was destoryed because it has a NULL class type, then
     *  it would not have created an extension instance, so we can skip the destroy
     *  call here.
     */
    if (!this_ptr->Class) {
        goto original_code;
    }

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<AnimClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, ds:0x007E4580 } // GameActive
    JMP_REG(ebx, 0x004142D0);
}


/**
 *  Main function for patching the hooks.
 */
void AnimClassExtension_Init()
{
    Patch_Jump(0x00413C79, &_AnimClass_Constructor_Patch);
    Patch_Jump(0x004142A6, &_AnimClass_Default_Constructor_Patch);
    Patch_Jump(0x0041441F, 0x00414475); // This jump goes from duplicate code in the destructor to our patch, removing the need for two hooks.
    Patch_Jump(0x004142CB, &_AnimClass_Destructor_Patch);
}
