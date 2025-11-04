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
#include "findmake.h"

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
EXPORT_FUNC(_AircraftTypeClass_Constructor_Patch)
{
    GET(AircraftTypeClass *, this_ptr, ESI); // "this" pointer.

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

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
EXPORT_FUNC(_AircraftTypeClass_Scalar_Destructor_Patch)
{
    GET(AircraftTypeClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<AircraftTypeClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class AircraftTypeClassExt : public AircraftTypeClass
{
public:
    static AircraftTypeClass* _Find_Or_Make(char const* name);
};


/**
 *  Replacement AircraftTypeClass::Find_Or_Make as it has the constructor inlined.
 *
 *  @author: ZivDero
 */
AircraftTypeClass* AircraftTypeClassExt::_Find_Or_Make(char const* name)
{
    return TFind_Or_Make<AircraftTypeClass>(name, AircraftTypes);
}


/**
 *  Main function for patching the hooks.
 */
void AircraftTypeClassExtension_Init()
{
    Patch_Jump(0x00410020, &AircraftTypeClassExt::_Find_Or_Make);
}

declhook(0x0040FC8F, _AircraftTypeClass_Constructor_Patch, 0x7);
declhook(0x00410228, _AircraftTypeClass_Scalar_Destructor_Patch, 0x6);
