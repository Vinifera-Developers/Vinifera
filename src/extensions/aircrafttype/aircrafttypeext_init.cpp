/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AIRCRAFTTYPEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended AircraftTypeClass.
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
#include "aircrafttypeext_hooks.h"
#include "aircrafttypeext.h"
#include "aircrafttype.h"
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
DECLARE_PATCH(_AircraftTypeClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(AircraftTypeClass *, this_ptr, esi); // "this" pointer.
    GET_STACK_STATIC(const char *, ini_name, esp, 0x0C); // ini name.

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
    Extension::Make<AircraftTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret 4 }
}


/**
 *  Patch for removing the inlined constructor and replacing it with a direct call.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AircraftTypeClass_Find_Or_Make_Patch)
{
    static uintptr_t constructor_addr = 0x0040FBC0;

    _asm { push edi } // edi == ini_name
    _asm { mov ecx, eax } // eax == memory pointer
    _asm { call constructor_addr } // AircraftTypeClass::AircraftTypeClass()

    JMP(0x00410165);
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AircraftTypeClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(AircraftTypeClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<AircraftTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov edx, ds:0x007E2420 } // AircraftTypes.vtble
    JMP_REG(eax, 0x0040FCDE);
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AircraftTypeClass_Scalar_Destructor_Patch)
{
    GET_REGISTER_STATIC(AircraftTypeClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<AircraftTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov edx, ds:0x007E2420 } // AircraftTypes.vtble
    JMP_REG(eax, 0x0041022E);
}


/**
 *  Main function for patching the hooks.
 */
void AircraftTypeClassExtension_Init()
{
    Patch_Jump(0x0040FC8F, &_AircraftTypeClass_Constructor_Patch);
    Patch_Jump(0x0041009C, &_AircraftTypeClass_Find_Or_Make_Patch); // Constructor is also inlined in AircraftTypeClass::Find_Or_Make!
    //Patch_Jump(0x0040FCD8, &_AircraftTypeClass_Destructor_Patch); // Destructor is actually inlined in scalar destructor!
    Patch_Jump(0x00410228, &_AircraftTypeClass_Scalar_Destructor_Patch);
}
