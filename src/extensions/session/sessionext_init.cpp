/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SESSIONEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended SessionClass.
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
#include "sessionext_hooks.h"
#include "sessionext.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "extension.h"
#include "extension_globals.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "syringe.h"


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005ED1AA, _SessionClass_Constructor_Patch, 5)
{
    GET(SessionClass *, this_ptr, EBP); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    SessionExtension = Extension::Singleton::Make<SessionClass, SessionClassExtension>(this_ptr);

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
DEFINE_HOOK(0x005ED465, _SessionClass_Destructor_Patch, 3)
{
    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<SessionClass, SessionClassExtension>(SessionExtension);

original_code:
    return 0;
}


/**
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005EE17F, _SessionClass_Read_MultiPlayer_Settings_Patch, 8)
{
    /**
     *  Load ini.
     */
    //DEBUG_INFO("Reading extended session settings\n");
    SessionExtension->Read_MultiPlayer_Settings();

original_code:
    return 0;
}


/**
 *  Patch for saving the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005EE7BA, _SessionClass_Write_MultiPlayer_Settings_Patch, 9)
{
    /**
     *  Save ini.
     */
    //DEBUG_INFO("Writing extended session settings\n");
    SessionExtension->Write_MultiPlayer_Settings();

original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void SessionClassExtension_Init()
{

}

