/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SMUDGEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended SmudgeClass.
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
#include "smudgeext.h"
#include "smudgetypeext.h"
#include "smudge.h"
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
DECLARE_PATCH(_SmudgeClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(SmudgeClass *, this_ptr, esi); // Current "this" pointer.

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
    Extension::Make<SmudgeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { ret 0x0C }
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SmudgeClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(SmudgeClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<SmudgeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov dword ptr [esi+0x4C], 0 } // this->Class = nullptr;
    this_ptr->ObjectClass::~ObjectClass();
    JMP(0x005FAB3F);
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_SmudgeClass_Scalar_Destructor_Patch)
{
    GET_REGISTER_STATIC(SmudgeClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<SmudgeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov dword ptr [esi+0x4C], 0 } // this->Class = nullptr;
    this_ptr->ObjectClass::~ObjectClass();
    JMP(0x005FAF6F);
}


/**
 *  Main function for patching the hooks.
 */
void SmudgeClassExtension_Init()
{
    Patch_Jump(0x005FAAB3, &_SmudgeClass_Constructor_Patch);
    //Patch_Jump(0x005FAB3F, &_SmudgeClass_Destructor_Patch); // Destructor is actually inlined in scalar destructor!
    Patch_Jump(0x005FAF61, &_SmudgeClass_Scalar_Destructor_Patch);
}
