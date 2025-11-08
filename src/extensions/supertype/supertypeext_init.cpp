/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SUPERTYPEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended SuperWeaponTypeClass.
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
#include "supertypeext_hooks.h"
#include "supertypeext.h"
#include "supertype.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "vinifera_globals.h"
#include "extension.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "syringe.h"


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
EXPORT_FUNC(_SuperWeaponTypeClass_Constructor_Patch)
{
    GET(SuperWeaponTypeClass *, this_ptr, EBP); // "this" pointer.

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
    Extension::Make<SuperWeaponTypeClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
EXPORT_FUNC(_SuperWeaponTypeClass_Destructor_Patch)
{
    GET(SuperWeaponTypeClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<SuperWeaponTypeClassExtension>(this_ptr);

original_code:
    this_ptr->AbstractTypeClass::~AbstractTypeClass();
    return 0x0060D0F1;
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
EXPORT_FUNC(_SuperWeaponTypeClass_Scalar_Destructor_Patch)
{
    GET(SuperWeaponTypeClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<SuperWeaponTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    this_ptr->AbstractTypeClass::~AbstractTypeClass();
    return 0x0060D881;
}


/**
 *  Main function for patching the hooks.
 */
void SuperWeaponTypeClassExtension_Init()
{

}

declhook(0x0060D04A, _SuperWeaponTypeClass_Constructor_Patch, 0x5);
declhook(0x0060D0EA, _SuperWeaponTypeClass_Destructor_Patch, 0);
declhook(0x0060D87A, _SuperWeaponTypeClass_Scalar_Destructor_Patch, 0);
