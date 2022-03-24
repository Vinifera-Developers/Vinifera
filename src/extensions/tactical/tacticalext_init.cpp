/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTICALEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended Tactical class.
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
#include "tacticalext_hooks.h"
#include "tacticalext.h"
#include "tactical.h"
#include "fatal.h"
#include "vinifera_util.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "hooker.h"
#include "hooker_macros.h"


/**
 *  "new" operations must be done within a new function for patched code.
 * 
 *  @author: CCHyper
 */
static void New_Tactical_Extension(Tactical *this_ptr)
{
    /**
     *  Delete existing instance (should never be the case).
     */
    delete TacticalExtension;

    TacticalExtension = new TacticalMapExtension(this_ptr);
}


/**
 *  "delete" operations must be done within a new function for patched code.
 * 
 *  @author: CCHyper
 */
static void Delete_Tactical_Extension()
{
    delete TacticalExtension;
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Tactical_Constructor_Patch)
{
    GET_REGISTER_STATIC(Tactical *, this_ptr, esi); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    New_Tactical_Extension(this_ptr);
    if (!TacticalExtension) {
        DEBUG_ERROR("Failed to create TacticalExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create TacticalExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create TacticalExtension instance!\n");
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
    _asm { ret }
}


/**
 *  Patch for including the extended class members in the noinit creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Tactical_NoInit_Constructor_Patch)
{
    GET_REGISTER_STATIC(Tactical *, this_ptr, esi);
    GET_STACK_STATIC(const NoInitClass *, noinit, esp, 0x4);

    /**
     *  Create the extended class instance.
     */
    New_Tactical_Extension(this_ptr);
    if (!TacticalExtension) {
        DEBUG_ERROR("Failed to create TacticalExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create TacticalExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create TacticalExtension instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
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
DECLARE_PATCH(_Tactical_Destructor_Patch)
{
    GET_REGISTER_STATIC(Tactical *, this_ptr, esi);

    /**
     *  Remove the extended class instance.
     */
    Delete_Tactical_Extension();

    /**
     *  Stolen bytes here.
     */
original_code:
    this_ptr->AbstractClass::~AbstractClass();
    _asm { ret }
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor.
 * 
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
static class FakeTacticalClass final : public Tactical
{
    public:
        void _Detach(TARGET target, bool all);
        void _Compute_CRC(WWCRCEngine &crc);
};


/**
 *  Patch for including the extended class members to the base class detach process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
void FakeTacticalClass::_Detach(TARGET target, bool all)
{
    Tactical::Detach(target, all);

    if (TacticalExtension) {
        TacticalExtension->Detach(target, all);
    }
}


/**
 *  Patch for including the extended class members to the base class crc calculation.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
void FakeTacticalClass::_Compute_CRC(WWCRCEngine &crc)
{
    AbstractClass::Compute_CRC(crc);

    if (TacticalExtension) {
        TacticalExtension->Compute_CRC(crc);
    }
}


/**
 *  Main function for patching the hooks.
 */
void TacticalExtension_Init()
{
    Patch_Jump(0x0060F08A, &_Tactical_Constructor_Patch);
    Patch_Jump(0x0060F0C5, &_Tactical_NoInit_Constructor_Patch);
    Patch_Jump(0x0060F0E7, &_Tactical_Destructor_Patch);
    Change_Virtual_Address(0x006D7720, Get_Func_Address(&FakeTacticalClass::_Detach));
    Change_Virtual_Address(0x006D7730, Get_Func_Address(&FakeTacticalClass::_Compute_CRC));
}
