/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UNITTYPEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended UnitTypeClass.
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
#include "unittypeext_hooks.h"
#include "unittypeext.h"
#include "unittype.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "vinifera_globals.h"
#include "extension.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitTypeClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(UnitTypeClass *, this_ptr, esi); // "this" pointer.
    GET_STACK_STATIC(const char *, ini_name, esp, 0x10); // ini name.

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
    Extension::Make<UnitTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitTypeClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(UnitTypeClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<UnitTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov edx, ds:0x007E2218 } // UnitTypes.vtble
    JMP_REG(eax, 0x0065BADE);
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitTypeClass_Scalar_Destructor_Patch)
{
    GET_REGISTER_STATIC(UnitTypeClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<UnitTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov edx, ds:0x007E2218 } // UnitTypes.vtble
    JMP_REG(eax, 0x0065C79E);
}


/**
 *  Main function for patching the hooks.
 */
void UnitTypeClassExtension_Init()
{
    Patch_Jump(0x0065BA96, &_UnitTypeClass_Constructor_Patch);
    //Patch_Jump(0x0065BAD8, &_UnitTypeClass_Destructor_Patch); // Destructor is actually inlined in scalar destructor!
    Patch_Jump(0x0065C798, &_UnitTypeClass_Scalar_Destructor_Patch);
}
