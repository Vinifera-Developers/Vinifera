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
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  "new" operations must be done within a new function for patched code.
 * 
 *  @author: CCHyper
 */
static void New_Scenario_Extension(ScenarioClass *this_ptr)
{
    /**
     *  Delete existing instance (should never be the case).
     */
    delete ScenarioExtension;

    ScenarioExtension = new ScenarioClassExtension(this_ptr);
}


/**
 *  "delete" operations must be done within a new function for patched code.
 * 
 *  @author: CCHyper
 */
static void Delete_Scenario_Extension()
{
    delete ScenarioExtension;
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ScenarioClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(ScenarioClass *, this_ptr, ebp); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    New_Scenario_Extension(this_ptr);
    if (!ScenarioExtension) {
        DEBUG_ERROR("Failed to create ScenarioExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create ScenarioExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create ScenarioExtension instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:

    /**
     *  We can't assign to Views directly without trashing the stack, so clear the whole array.
     */
    std::memset(&this_ptr->Views, 0, sizeof(this_ptr->Views));

    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { ret }
}


/**
 *  Patch for including the extended class members in the noinit creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ScenarioClass_NoInit_Constructor_Patch)
{
    GET_REGISTER_STATIC(ScenarioClass *, this_ptr, esi); // "this" pointer.
    GET_STACK_STATIC(const NoInitClass *, noinit, esp, 0x4);

    /**
     *  Create the extended class instance.
     */
    New_Scenario_Extension(this_ptr);
    if (!ScenarioExtension) {
        DEBUG_ERROR("Failed to create ScenarioExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create ScenarioExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create ScenarioExtension instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ScenarioClass_Deconstructor_Patch)
{
    GET_REGISTER_STATIC(ScenarioClass *, this_ptr, esi);

    /**
     *  Remove the extended class instance.
     */
    Delete_Scenario_Extension();

    /**
     *  Stolen bytes here.
     */
original_code:
    Scen = nullptr;

    JMP(0x006023D2);
}


/**
 *  Patch for including the extended class members when initialsing the scenario data.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ScenarioClass_Init_Clear_Patch)
{
    GET_REGISTER_STATIC(ScenarioClass *, this_ptr, esi);

    /**
     *  Find the extension instance.
     */
    if (!ScenarioExtension) {
        goto original_code;
    }

    ScenarioExtension->Init_Clear();

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x0C }
    _asm { ret }
}


/**
 *  Patch for including the extended class members when computing a unique crc value for this instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ScenarioClass_Compute_CRC_Patch)
{
    GET_REGISTER_STATIC(OverlayTypeClass *, this_ptr, esi);
    GET_STACK_STATIC(WWCRCEngine *, crc, esp, 0xC);

    /**
     *  Find the extension instance.
     */
    if (!ScenarioExtension) {
        goto original_code;
    }

    /**
     *  Read type class compute crc.
     */
    ScenarioExtension->Compute_CRC(*crc);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { ret 4 }
}


/**
 *  Main function for patching the hooks.
 */
void ScenarioClassExtension_Init()
{
    Patch_Jump(0x005DADDE, &_ScenarioClass_Constructor_Patch);
    Patch_Jump(0x005DAE87, &_ScenarioClass_NoInit_Constructor_Patch);
    Patch_Jump(0x006023CC, &_ScenarioClass_Deconstructor_Patch); // Inlined in game shutdown.
    Patch_Jump(0x005DB166, &_ScenarioClass_Init_Clear_Patch);
    Patch_Jump(0x005E1440, &_ScenarioClass_Compute_CRC_Patch);
}
