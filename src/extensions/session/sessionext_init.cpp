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
#include "vinifera_util.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  "new" operations must be done within a new function for patched code.
 * 
 *  @author: CCHyper
 */
static void New_Session_Extension(SessionClass *this_ptr)
{
    /**
     *  Delete existing instance (should never be the case).
     */
    delete SessionExtension;

    SessionExtension = new SessionClassExtension(this_ptr);
}


/**
 *  "delete" operations must be done within a new function for patched code.
 * 
 *  @author: CCHyper
 */
static void Delete_Session_Extension()
{
    delete SessionExtension;
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SessionClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(SessionClass *, this_ptr, ebp); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    New_Session_Extension(this_ptr);
    if (!SessionExtension) {
        DEBUG_ERROR("Failed to create SessionExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create SessionExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create SessionExtension instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, ebp }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { pop ecx }
    _asm { ret }
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SessionClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(SessionClass *, this_ptr, esi);

    /**
     *  Create the extended class instance.
     */
    New_Session_Extension(this_ptr);
    if (!SessionExtension) {
        DEBUG_ERROR("Failed to create SessionExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create SessionExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create SessionExtension instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret }
}


/**
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SessionClass_Read_MultiPlayer_Settings_Patch)
{
    GET_REGISTER_STATIC(SessionClass *, this_ptr, ebp);

    /**
     *  Load ini.
     */
    if (SessionExtension) {
        //DEBUG_INFO("Reading extended session settings\n");
        SessionExtension->Read_MultiPlayer_Settings();
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop esi }
    _asm { pop ebp }
    _asm { add esp, 0x0F8 }
    _asm { ret }
}


/**
 *  Patch for saving the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SessionClass_Write_MultiPlayer_Settings_Patch)
{
    GET_REGISTER_STATIC(SessionClass *, this_ptr, esi);

    /**
     *  Save ini.
     */
    if (SessionExtension) {
        //DEBUG_INFO("Writing extended session settings\n");
        SessionExtension->Write_MultiPlayer_Settings();
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { add esp, 0x0AC }
    _asm { ret }
}


/**
 *  Main function for patching the hooks.
 */
void SessionClassExtension_Init()
{
    Patch_Jump(0x005ED1AA, &_SessionClass_Constructor_Patch);
    Patch_Jump(0x005ED465, &_SessionClass_Destructor_Patch);
    Patch_Jump(0x005EE17F, &_SessionClass_Read_MultiPlayer_Settings_Patch);
    Patch_Jump(0x005EE7BA, &_SessionClass_Write_MultiPlayer_Settings_Patch);
}
