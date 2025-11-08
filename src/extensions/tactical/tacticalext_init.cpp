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
EXPORT_FUNC(_Tactical_Constructor_Patch)
{
    GET(Tactical *, this_ptr, ESI); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    TacticalMapExtension = Extension::Singleton::Make<Tactical, TacticalExtension>(this_ptr);

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
EXPORT_FUNC(_Tactical_Destructor_Patch)
{
    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<Tactical, TacticalExtension>(TacticalMapExtension);

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
EXPORT_FUNC(_Tactical_Scalar_Destructor_Patch)
{
    GET(Tactical *, this_ptr, ESI);

    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<Tactical, TacticalExtension>(TacticalMapExtension);

    /**
     *  Stolen bytes here.
     */
original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void TacticalExtension_Init()
{

}

declhook(0x0060F08A, _Tactical_Constructor_Patch, 0x5);
declhook(0x0060F0DD, _Tactical_Destructor_Patch, 0xA);
declhook(0x00618020, _Tactical_Scalar_Destructor_Patch, 0xA);
