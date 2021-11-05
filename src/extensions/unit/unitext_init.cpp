/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UNITEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended UnitClass.
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
#include "unitext.h"
#include "unittypeext.h"
#include "unit.h"
#include "unittype.h"
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
DECLARE_PATCH(_UnitClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi); // Current "this" pointer.
    static UnitClassExtension *exttype_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    exttype_ptr = UnitClassExtensions.find_or_create(this_ptr);
    if (!exttype_ptr) {
        DEBUG_ERROR("Failed to create UnitClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create UnitClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create UnitClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x0C }
    _asm { ret 8 }
}


/**
 *  Patch for including the extended class members in the noinit creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_NoInit_Constructor_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    GET_STACK_STATIC(const NoInitClass *, noinit, esp, 0x18);
    static UnitClassExtension *ext_ptr;

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov dword ptr [esi], 0x006D8B6C } // this->vftable = const UnitClass::`vftable';
    JMP(0x00659680);
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Deconstructor_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    UnitClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
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
DECLARE_PATCH(_UnitClass_Detach_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    GET_STACK_STATIC(TARGET, target, esp, 0x10);
    GET_STACK_STATIC8(bool, all, esp, 0x8);
    static UnitClassExtension *ext_ptr;

    /**
     *  Find the extension instance.
     */
    ext_ptr = UnitClassExtensions.find(this_ptr);
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
DECLARE_PATCH(_UnitClass_Compute_CRC_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    GET_STACK_STATIC(WWCRCEngine *, crc, esp, 0xC);
    static UnitClassExtension *ext_ptr;

    /**
     *  Find the extension instance.
     */
    ext_ptr = UnitClassExtensions.find(this_ptr);
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
void UnitClassExtension_Init()
{
    Patch_Jump(0x0064D7B4, &_UnitClass_Constructor_Patch);
    Patch_Jump(0x0065967A, &_UnitClass_NoInit_Constructor_Patch);
    Patch_Jump(0x0064D9C0, &_UnitClass_Deconstructor_Patch);
    Patch_Jump(0x00659863, &_UnitClass_Detach_Patch);
    Patch_Jump(0x00659825, &_UnitClass_Compute_CRC_Patch);
}
