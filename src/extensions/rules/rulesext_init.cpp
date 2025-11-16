/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RULESEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended RulesClass.
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
#include "rulesext_hooks.h"
#include "rulesext.h"
#include "rules.h"
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
DEFINE_HOOK(0x005C59A1, _RulesClass_Constructor_Patch, 5)
{
    GET(RulesClass *, this_ptr, ESI); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    RuleExtension = Extension::Singleton::Make<RulesClass, RulesClassExtension>(this_ptr);

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
DEFINE_HOOK(0x005C6120, _RulesClass_Destructor_Patch, 5)
{
    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<RulesClass, RulesClassExtension>(RuleExtension);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members when processing rules data.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005C6A4D, _RulesClass_Process_Patch, 7)
{
    GET(CCINIClass*, ini, ESI);

    RuleExtension->Process(*ini);

    /**
     *  Stolen bytes here.
     */
original_code:
    return 0;
}


/**
 *  Patch for including the extended class members when processing the MPlayer section.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005CC3BF, _RulesClass_MPlayer_Patch, 7)
{
    GET(CCINIClass*, ini, EDI);

    RuleExtension->MPlayer(*ini);

    /**
     *  Stolen bytes here.
     */
original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void RulesClassExtension_Init()
{

}

