/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          THEMEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended ThemeClass.
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
#include "themeext_hooks.h"
#include "themeext.h"
#include "theme.h"
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
DECLARE_PATCH(_ThemeClass_ThemeControl_Constructor_Patch)
{
    GET_REGISTER_STATIC(ThemeClass::ThemeControl *, this_ptr, eax); // "this" pointer.
    static ThemeControlExtension *ext_ptr;

    /**
     *  Create an extended class instance.
     */
    Extension::List::Make<ThemeClass::ThemeControl, ThemeControlExtension>(this_ptr, ThemeControlExtensions);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { ret }
}


/**
 *  This patch replaces a inlined copy of the constructor with a call to the constructor.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ThemeClass_ThemeControl_Inlined_Constructor_Patch)
{
    _asm { mov ecx, eax }
    _asm { mov eax, 0x006439B0 } // ThemeClass::ThemeControl::ThemeControl()
    _asm { call eax }

    _asm { mov ebp, eax } // ebp = "this".

    JMP(0x00643B7C);
}


/**
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ThemeClass_ThemeControl_Read_INI_Patch)
{
    GET_REGISTER_STATIC(ThemeClass::ThemeControl *, this_ptr, esi);
    GET_REGISTER_STATIC(CCINIClass *, ini, edi);
    static ThemeControlExtension *exttype_ptr;

    /**
     *  Find the extension instance.
     */
    exttype_ptr = Extension::List::Fetch<ThemeClass::ThemeControl, ThemeControlExtension>(this_ptr, ThemeControlExtensions);

    /**
     *  Read type class ini.
     */
    exttype_ptr->Read_INI(*ini);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov al, 1 }
    _asm { pop edi }
    _asm { pop esi }
    _asm { ret 4 }
}


/**
 *  Main function for patching the hooks.
 */
void ThemeClassExtension_Init()
{
    Patch_Jump(0x006439E5, &_ThemeClass_ThemeControl_Constructor_Patch);
    Patch_Jump(0x00643B45, &_ThemeClass_ThemeControl_Inlined_Constructor_Patch);
    //Patch_Jump(0x, &_ThemeClass_ThemeControl_Destructor_Patch); // No destructor to hook, extension should clean up on exit.
    Patch_Jump(0x00643AAB, &_ThemeClass_ThemeControl_Read_INI_Patch);
}
