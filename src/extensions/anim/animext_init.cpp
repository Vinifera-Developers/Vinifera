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
#include "asserthandler.h"
#include "debughandler.h"


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
    static AnimClassExtension *exttype_ptr;
    static AnimTypeClassExtension *animtypeext;

    /**
     *  Find existing or create an extended class instance.
     */
    exttype_ptr = AnimClassExtensions.find_or_create(this_ptr);
    if (!exttype_ptr) {
        DEBUG_ERROR("Failed to create AnimClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create AnimClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create AnimClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
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
     *  #issue-561
     * 
     *  Implements ZAdjust override for Anims. This will only have an effect
     *  if the anim is created with a z-adjustment value of "0" (default value).
     * 
     *  @author: CCHyper
     */
    if (!this_ptr->ZAdjust) {
        animtypeext = AnimTypeClassExtensions.find(this_ptr->Class);
        if (animtypeext) {
            this_ptr->ZAdjust = animtypeext->ZAdjust;
        }
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
    this_ptr->entry_E4();
    
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
    static AnimClassExtension *exttype_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    exttype_ptr = AnimClassExtensions.find_or_create(this_ptr);
    if (!exttype_ptr) {
        DEBUG_ERROR("Failed to create AnimClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create AnimClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create AnimClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret 0x10 }
}


/**
 *  Patch for including the extended class members in the noinit creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_NoInit_Constructor_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    GET_STACK_STATIC(const NoInitClass *, noinit, esp, 0x10);
    static AnimClassExtension *ext_ptr;

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov dword ptr [esi], 0x006CB92C } // this->vftable = const AnimClass::`vftable'{for `IRTTITypeInfo'};
    JMP(0x004164DD);
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
     *  Remove the extended class from the global index.
     */
    AnimClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop esi }
    _asm { pop ebx }
    _asm { add esp, 0x10 }
    _asm { ret }
}


/**
 *  Patch for including the extended class members to the base class detach process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Detach_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    GET_STACK_STATIC(TARGET, target, esp, 0x10);
    GET_STACK_STATIC8(bool, all, esp, 0x8);
    static AnimClassExtension *ext_ptr;

    /**
     *  Find the extension instance.
     */
    ext_ptr = AnimClassExtensions.find(this_ptr);
    if (!ext_ptr) {
        goto original_code;
    }

    ext_ptr->Detach(target, all);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { ret 8 }
}


/**
 *  Patch for including the extended class members to the base class crc calculation.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Compute_CRC_Patch)
{
    GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
    GET_STACK_STATIC(WWCRCEngine *, crc, esp, 0xC);
    static AnimClassExtension *ext_ptr;

    /**
     *  Find the extension instance.
     */
    ext_ptr = AnimClassExtensions.find(this_ptr);
    if (!ext_ptr) {
        goto original_code;
    }

    ext_ptr->Compute_CRC(*crc);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { ret 4 }
}


/**
 *  Main function for patching the hooks.
 */
void AnimClassExtension_Init()
{
    Patch_Jump(0x00413C79, &_AnimClass_Constructor_Patch);
    Patch_Jump(0x004142A6, &_AnimClass_Default_Constructor_Patch);
    Patch_Jump(0x004164D7, &_AnimClass_NoInit_Constructor_Patch);
    Patch_Jump(0x0041441F, 0x00414475); // This jump goes from duplicate code in the destructor to our patch, removing the need for two hooks.
    Patch_Jump(0x0041447C, &_AnimClass_Destructor_Patch);
    Patch_Jump(0x004163D9, &_AnimClass_Detach_Patch);
    Patch_Jump(0x00416626, &_AnimClass_Compute_CRC_Patch);
}
