/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RULESEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended RulesClass.
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
#include "rulesext_hooks.h"
#include "rulesext.h"
#include "rules.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  "new" operations must be done within a new function for patched code.
 * 
 *  @author: CCHyper
 */
static void New_Rules_Extension(RulesClass *this_ptr)
{
    /**
     *  Delete existing instance (should never be the case).
     */
    delete RulesExtension;

    RulesExtension = new RulesClassExtension(this_ptr);
}


/**
 *  "delete" operations must be done within a new function for patched code.
 * 
 *  @author: CCHyper
 */
static void Delete_Rules_Extension()
{
    delete RulesExtension;
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_RulesClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(RulesClass *, this_ptr, esi); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    New_Rules_Extension(this_ptr);
    if (!RulesExtension) {
        DEBUG_ERROR("Failed to create RulesExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create RulesExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create RulesExtension instance!\n");
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
    _asm { ret }
}


/**
 *  Patch for including the extended class members in the noinit creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_RulesClass_NoInit_Constructor_Patch)
{
    GET_REGISTER_STATIC(RulesClass *, this_ptr, esi); // "this" pointer.
    GET_STACK_STATIC(const NoInitClass *, noinit, esp, 0x4);

    /**
     *  Create the extended class instance.
     */
    New_Rules_Extension(this_ptr);
    if (!RulesExtension) {
        DEBUG_ERROR("Failed to create RulesExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create RulesExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create RulesExtension instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop esi }
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_RulesClass_Deconstructor_Patch)
{
    GET_REGISTER_STATIC(RulesClass *, this_ptr, esi);

    /**
     *  Remove the extended class instance.
     */
    Delete_Rules_Extension();

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { ret }
}


/**
 *  Patch for including the extended class members when initialsing rules data.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_RulesClass_Initialize_Patch)
{
    GET_STACK_STATIC(RulesClass *, this_ptr, esp, 0x10);
    GET_STACK_STATIC(CCINIClass *, ini, esp, 0x44);

    /**
     *  Find the extension instance.
     */
    if (!RulesExtension) {
        goto original_code;
    }

    RulesExtension->Initialize(*ini);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x130 }
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members when processing rules data.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_RulesClass_Process_Patch)
{
    GET_REGISTER_STATIC(RulesClass *, this_ptr, ebp);
    GET_REGISTER_STATIC(CCINIClass *, ini, esi);

    /**
     *  Find the extension instance.
     */
    if (!RulesExtension) {
        goto original_code;
    }

    RulesExtension->Process(*ini);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x20 }
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members to the base class detach process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_RulesClass_Detach_Patch)
{
    GET_REGISTER_STATIC(RulesClass *, this_ptr, esi);
    GET_STACK_STATIC(TARGET, target, esp, 0x4);
    GET_STACK_STATIC8(bool, all, esp, 0x8);

    /**
     *  Find the extension instance.
     */
    if (!RulesExtension) {
        goto original_code;
    }

    RulesExtension->Detach(target, all);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret 8 }
}


/**
 *  Main function for patching the hooks.
 */
void RulesClassExtension_Init()
{
    Patch_Jump(0x005C59A1, &_RulesClass_Constructor_Patch);
    Patch_Jump(0x005C4347, &_RulesClass_NoInit_Constructor_Patch);
    Patch_Jump(0x005C6120, &_RulesClass_Deconstructor_Patch);
    Patch_Jump(0x005C66FF, &_RulesClass_Initialize_Patch);
    Patch_Jump(0x005C6A4D, &_RulesClass_Process_Patch);
    Patch_Jump(0x005D17F5, &_RulesClass_Detach_Patch);
}
