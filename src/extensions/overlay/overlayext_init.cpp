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
#include "overlaytypeext.h"
#include "overlay.h"
#include "vinifera_util.h"
#include "vinifera_globals.h"
#include "extension.h"
#include "fatal.h"
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
DECLARE_PATCH(_OverlayClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(OverlayClass *, this_ptr, esi); // Current "this" pointer.

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
    Extension::Make<OverlayClassExtension>(this_ptr);

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
 *  Patch for removing the inlined constructor and replacing it with a direct call.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_OverlayClass_Read_INI_Constructor_Patch)
{
    static uintptr_t constructor_addr = 0x0058B460;

    _asm { lea edx, [esp+0x14] } // cell
    _asm { push 0xFFFFFFFF } // house (default arg, HOUSES_NONE)
    _asm { push edx } // edx == cell
    _asm { push edi } // edi == overlay type
    _asm { mov ecx, esi } // esi == memory pointer
    _asm { call constructor_addr } // OverlayClass::OverlayClass()

    JMP(0x0058C0A5);
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OverlayClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(OverlayClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<OverlayClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov dword ptr [esi+0x4C], 0 } // this->Class = nullptr;
    this_ptr->ObjectClass::~ObjectClass();
    JMP(0x0058B5CF);
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
    Extension::Destroy<OverlayClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov dword ptr [esi+0x4C], 0 } // this->Class = nullptr;
    this_ptr->ObjectClass::~ObjectClass();
    JMP(0x0058CB7F);
}


/**
 *  Main function for patching the hooks.
 */
void OverlayClassExtension_Init()
{
    Patch_Jump(0x0058B545, &_OverlayClass_Constructor_Patch);
    Patch_Jump(0x0058C02B, &_OverlayClass_Read_INI_Constructor_Patch); // Constructor is also inlined in OverlayClass::Read_INI!
    //Patch_Jump(0x0058B5C1, &_OverlayClass_Destructor_Patch); // Destructor is actually inlined in scalar destructor!
    Patch_Jump(0x0058CB71, &_OverlayClass_Scalar_Destructor_Patch);
}
