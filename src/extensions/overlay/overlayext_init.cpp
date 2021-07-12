/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OVERLAYEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended OverlayClass.
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
#include "overlayext.h"
#include "overlay.h"
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
DECLARE_PATCH(_OverlayClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(OverlayClass *, this_ptr, esi); // Current "this" pointer.
    static OverlayClassExtension *exttype_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    exttype_ptr = OverlayClassExtensions.find_or_create(this_ptr);
    if (!exttype_ptr) {
        DEBUG_ERROR("Failed to create OverlayClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create OverlayClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create OverlayClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { add esp, 0x0C }
    _asm { ret 0x0C }
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  We need do this before the unlimbo otherwise finding extension data
 *  will fail if you patch Unlimbo.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OverlayClass_Constructor_Before_Unlimbo_Patch)
{
    GET_REGISTER_STATIC(OverlayClass *, this_ptr, esi); // Current "this" pointer.
    GET_STACK_STATIC(Cell *, cell, esp, 0x18);
    static OverlayClassExtension *ext_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    ext_ptr = OverlayClassExtensions.find_or_create(this_ptr);
    if (!ext_ptr) {
        DEBUG_ERROR("Failed to create OverlayClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create OverlayClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create OverlayClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov esi, this_ptr }
    _asm { mov eax, cell }

    JMP_REG(edx, 0x0058B4E4);
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  This patch is required because an instance of the class constructor was
 *  inlined in Read_INI.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OverlayClass_Read_INI_Inlined_Constructor_Patch)
{
    GET_REGISTER_STATIC(OverlayClass *, this_ptr, esi); // Current "this" pointer.
    LEA_STACK_STATIC(Cell *, cell, esp, 0x14);
    static OverlayClassExtension *ext_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    ext_ptr = OverlayClassExtensions.find_or_create(this_ptr);
    if (!ext_ptr) {
        DEBUG_ERROR("Failed to create OverlayClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create OverlayClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create OverlayClassExtensions instance!\n");
        goto unlimbo; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
    if (cell->X <= 0 && cell->Y <= 0) {
        goto constructor_return;
    }

unlimbo:
    JMP_REG(edx, 0x0058C079);

constructor_return:
    JMP_REG(edx, 0x0058C0A5);
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OverlayClass_Deconstructor_Patch)
{
    GET_REGISTER_STATIC(OverlayClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    OverlayClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop esi }
    _asm { pop ecx }
    _asm { ret }
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OverlayClass_Scalar_Destructor_Patch)
{
    GET_REGISTER_STATIC(OverlayClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    OverlayClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { pop ecx }
    _asm { ret 4 }
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class OverlayClassFake final : public OverlayClass
{
    public:
        void _Detach(TARGET target, bool all = true);
        void _Compute_CRC(WWCRCEngine &crc) const;

        bool _Unlimbo(Coordinate &coord, DirType dir = DIR_N);
};


/**
 *  Implementation of Detach() for OverlayClass.
 */
void OverlayClassFake::_Detach(TARGET target, bool all)
{
    OverlayClassExtension *ext_ptr;

    ObjectClass::Detach(target, all);

    /**
     *  Find the extension instance and call Detach on it.
     */
    ext_ptr = OverlayClassExtensions.find(this);
    if (ext_ptr) {
        ext_ptr->Detach(target, all);
    }
}


/**
 *  Implementation of Compute_CRC() for OverlayClass.
 */
void OverlayClassFake::_Compute_CRC(WWCRCEngine &crc) const
{
    OverlayClassExtension *ext_ptr;

    ObjectClass::Compute_CRC(crc);

    /**
     *  Find the extension instance and call Compute_CRC on it.
     */
    ext_ptr = OverlayClassExtensions.find(this);
    if (ext_ptr) {
        ext_ptr->Compute_CRC(crc);
    }
}


/**
 *  Implementation of Compute_CRC() for OverlayClass.
 */
bool OverlayClassFake::_Unlimbo(Coordinate &coord, DirType dir)
{
    OverlayClassExtension *ext_ptr;

    if (!ObjectClass::Unlimbo(coord, dir)) {
        return false;
    }

    /**
     *  Find the extension instance and call Unlimbo on it.
     */
    ext_ptr = OverlayClassExtensions.find(this);
    if (ext_ptr) {
        return ext_ptr->Unlimbo(coord, dir);
    }

    return true;
}


/**
 *  Main function for patching the hooks.
 */
void OverlayClassExtension_Init()
{
    //Patch_Jump(0x0058B545, _OverlayClass_Constructor_Patch);
    Patch_Jump(0x0058B4DD, _OverlayClass_Constructor_Before_Unlimbo_Patch);
    Patch_Jump(0x0058C05D, _OverlayClass_Read_INI_Inlined_Constructor_Patch);
    //Patch_Jump(0x0058B5CF, _OverlayClass_Deconstructor_Patch); // Destructor is actually inlined in scalar destructor!
    Patch_Jump(0x0058CB8F, &_OverlayClass_Scalar_Destructor_Patch);
    Change_Virtual_Address(0x006D4FC4, Get_Func_Address(&OverlayClassFake::_Detach));
    Change_Virtual_Address(0x006D4FD4, Get_Func_Address(&OverlayClassFake::_Compute_CRC));
    Patch_Call(0x0058B536, &OverlayClassFake::_Unlimbo);
    Patch_Call(0x0058C09A, &OverlayClassFake::_Unlimbo);
}
