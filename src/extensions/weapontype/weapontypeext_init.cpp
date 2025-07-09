/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WEAPONTYPEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended WeaponTypeClass.
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
#include "weapontypeext_hooks.h"
#include "weapontypeext.h"
#include "weapontype.h"
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
DECLARE_PATCH(_WeaponTypeClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(WeaponTypeClass *, this_ptr, esi); // "this" pointer.
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
    Extension::Make<WeaponTypeClassExtension>(this_ptr);

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
DECLARE_PATCH(_WeaponTypeClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(WeaponTypeClass *, this_ptr, esi);

    /**
     *  Stolen bytes here.
     */
    _asm { mov [esi+0x0A5], bl }
    _asm { mov [esi+0x0A0], ebx }

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<WeaponTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    this_ptr->AbstractTypeClass::~AbstractTypeClass();
    JMP_REG(ecx, 0x00680D1F);
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WeaponTypeClass_Scalar_Destructor_Patch)
{
    GET_REGISTER_STATIC(WeaponTypeClass *, this_ptr, esi);

    /**
     *  Stolen bytes here.
     */
    _asm { mov [esi+0x0A5], bl }
    _asm { mov [esi+0x0A0], ebx }

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<WeaponTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    this_ptr->AbstractTypeClass::~AbstractTypeClass();
    JMP_REG(ecx, 0x006819BF);
}


/**
 *  Main function for patching the hooks.
 */
void WeaponTypeClassExtension_Init()
{
    Patch_Jump(0x00680BEF, &_WeaponTypeClass_Constructor_Patch);
    //Patch_Jump(0x00680D0C, &_WeaponTypeClass_Destructor_Patch); // Destructor is actually inlined in scalar destructor!
    Patch_Jump(0x006819AC, &_WeaponTypeClass_Scalar_Destructor_Patch);
}
