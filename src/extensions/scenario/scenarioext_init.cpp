/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SCENARIOEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended ScenarioClass.
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
#include "scenarioext_hooks.h"
#include "scenarioext.h"
#include "scenario.h"
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
DEFINE_HOOK(0x005DADDE, _ScenarioClass_Constructor_Patch, 9)
{
    GET(ScenarioClass *, this_ptr, EBP); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    ScenExtension = Extension::Singleton::Make<ScenarioClass, ScenarioClassExtension>(this_ptr);

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
DEFINE_HOOK(0x006023CC, _ScenarioClass_Destructor_Patch, 6)
{
    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<ScenarioClass, ScenarioClassExtension>(ScenExtension);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members when initialsing the scenario data.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005DB166, _ScenarioClass_Init_Clear_Patch, 7)
{
    /**
     *  This is a odd case; ScenarioClass::Init_Clear is called within the class
     *  constructor, so the first time this patch is called, ScenExtension is NULL.
     *  The ScenarioClassExtension calls it's Init_Clear to mirror this behaviour
     *  so we can just check if the extension has be created first to catch this.
     */
    if (ScenExtension) {
        ScenExtension->Init_Clear();
    }

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
DEFINE_HOOK(0x005DD93B, _ScenarioClass_Read_INI_Patch, 6)
{
    GET(CCINIClass *, ini, EBP);

    bool retval = false;
    retval |= Scen->Read_INI(*ini);
    retval |= ScenExtension->Read_INI(*ini);

    R->AL(retval);
    return 0x005DD947;
}


/**
 *  Main function for patching the hooks.
 */
void ScenarioClassExtension_Init()
{

}
