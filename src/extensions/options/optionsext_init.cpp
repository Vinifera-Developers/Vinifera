/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OPTIONSEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended OptionsClass.
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
#include "optionsext_hooks.h"
#include "optionsext.h"
#include "vinifera_util.h"
#include "options.h"
#include "techno.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  "new" operations must be done within a new function for patched code.
 * 
 *  @author: CCHyper
 */
static void New_Options_Extension(OptionsClass *this_ptr)
{
    /**
     *  Delete existing instance (should never be the case).
     */
    delete OptionsExtension;

    OptionsExtension = new OptionsClassExtension(this_ptr);
}


/**
 *  "delete" operations must be done within a new function for patched code.
 * 
 *  @author: CCHyper
 */
static void Delete_Options_Extension()
{
    delete OptionsExtension;
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OptionsClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(OptionsClass *, this_ptr, eax); // "this" pointer.

    /**
     *  The OptionsClass constructor is actually called twice as there are
     *  are two instances; The Options and another temporary one for the
     *  display options. So we handle this by skipping that second call.
     */
    if (OptionsExtension) {
        goto original_code;
    }

    /**
     *  Create the extended class instance.
     */
    New_Options_Extension(this_ptr);
    if (!OptionsExtension) {
        DEBUG_ERROR("Failed to create OptionsExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create OptionsExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create OptionsExtension instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:

    /**
     *  #issue-244
     * 
     *  Changes the default value of "AllowHiResModes" to "true".
     * 
     *  @author: CCHyper
     */
    this_ptr->AllowHiResModes = true;

    /**
     *  #issue-212
     * 
     *  Changes the default value of "IsScoreShuffle" to "true".
     * 
     *  @author: CCHyper
     */
    this_ptr->IsScoreShuffle = true;

    _asm { ret }
}


/**
 *  OptionsClass has no destructor! 
 */
#if 0
/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OptionsClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(OptionsClass *, this_ptr, esi);

    /**
     *  Create the extended class instance.
     */
    New_Options_Extension(this_ptr);
    if (!OptionsExtension) {
        DEBUG_ERROR("Failed to create OptionsExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create OptionsExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create OptionsExtension instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { ret 4 }
}
#endif


/**
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OptionsClass_Load_Settings_Patch)
{
    GET_REGISTER_STATIC(OptionsClass *, this_ptr, esi);

    /**
     *  Load ini.
     */
    if (OptionsExtension) {
        OptionsExtension->Load_Settings();
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop esi }
    _asm { ret }
}


/**
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WinMain_Load_Init_Options_Settings_Patch)
{
    GET_REGISTER_STATIC(OptionsClass *, this_ptr, esi);

    /**
     *  Load ini.
     */
    if (OptionsExtension) {
        OptionsExtension->Load_Init_Settings();
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { push 0x31C /* sizeof(WWKeyboardClass) */ }
    JMP(0x00601283);
}


/**
 *  Patch for saving the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OptionsClass_Save_Settings_Patch)
{
    GET_REGISTER_STATIC(OptionsClass *, this_ptr, esi);

    /**
     *  Save ini.
     */
    if (OptionsExtension) {
        OptionsExtension->Save_Settings();
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop esi }
    _asm { add esp, 0x64 }
    _asm { ret }
}


/**
 *  Patch for saving the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OptionsClass_Set_Patch)
{
    GET_REGISTER_STATIC(OptionsClass *, this_ptr, esi);

    /**
     *  Set options.
     */
    if (OptionsExtension) {
        OptionsExtension->Set();
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    TechnoClass::Set_Action_Lines(this_ptr->ActionLines);

    _asm { pop esi }
    _asm { ret }
}


/**
 *  Main function for patching the hooks.
 */
void OptionsClassExtension_Init()
{
    Patch_Jump(0x00589A12, &_OptionsClass_Constructor_Patch);
    //Patch_Jump(0x, &_OptionsClass_Destructor_Patch);
    Patch_Jump(0x0058A132, &_OptionsClass_Load_Settings_Patch);
    Patch_Jump(0x0060127E, &_WinMain_Load_Init_Options_Settings_Patch);
    Patch_Jump(0x0058A3C3, &_OptionsClass_Save_Settings_Patch);
    Patch_Jump(0x0058A5F7, &_OptionsClass_Set_Patch);
}
