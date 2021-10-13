/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INFANTRYEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended InfantryClass.
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
#include "infantryext.h"
#include "infantrytypeext.h"
#include "infantry.h"
#include "infantrytype.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "vinifera_util.h"


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, esi); // Current "this" pointer.
    static InfantryClassExtension *exttype_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    exttype_ptr = InfantryClassExtensions.find_or_create(this_ptr);
    if (!exttype_ptr) {
        DEBUG_ERROR("Failed to create InfantryClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create InfantryClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create InfantryClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret 8 }
}


/**
 *  Patch for including the extended class members in the noinit creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_NoInit_Constructor_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, esi);
    GET_STACK_STATIC(const NoInitClass *, noinit, esp, 0x20);
    static InfantryClassExtension *ext_ptr;

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov dword ptr [ebp+0], 0x006D2100 } // this->vftable = const InfantryClass::`vftable';
    JMP(0x004D9415);
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_Deconstructor_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    InfantryClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { add esp, 0x8 }
    _asm { ret }
}


/**
 *  Patch for including the extended class members to the base class detach process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_Detach_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, esi);
    GET_STACK_STATIC(TARGET, target, esp, 0x10);
    GET_STACK_STATIC8(bool, all, esp, 0x8);
    static InfantryClassExtension *ext_ptr;

    /**
     *  Find the extension instance.
     */
    ext_ptr = InfantryClassExtensions.find(this_ptr);
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
DECLARE_PATCH(_InfantryClass_Compute_CRC_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, esi);
    GET_STACK_STATIC(WWCRCEngine *, crc, esp, 0xC);
    static InfantryClassExtension *ext_ptr;

    /**
     *  Find the extension instance.
     */
    ext_ptr = InfantryClassExtensions.find(this_ptr);
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
void InfantryClassExtension_Init()
{
    Patch_Jump(0x004D21E1, &_InfantryClass_Constructor_Patch);
    Patch_Jump(0x004D940F, &_InfantryClass_NoInit_Constructor_Patch);
    Patch_Jump(0x004D23F2, &_InfantryClass_Deconstructor_Patch);
    Patch_Jump(0x004D40E5, &_InfantryClass_Detach_Patch);
    Patch_Jump(0x004D96DB, &_InfantryClass_Compute_CRC_Patch);
}
