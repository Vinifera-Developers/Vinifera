/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TERRAINEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended TerrainClass.
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
#include "terrainext.h"
#include "terrain.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "vinifera_util.h"


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TerrainClass_Default_Constructor_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, esi); // Current "this" pointer.
    GET_STACK_STATIC(const TerrainTypeClass *, classof, esp, 0x20);
    GET_STACK_STATIC(const Cell *, cell, esp, 0x24);
    static TerrainClassExtension *ext_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    ext_ptr = TerrainClassExtensions.find_or_create(this_ptr);
    if (!ext_ptr) {
        DEBUG_ERROR("Failed to create TerrainClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create TerrainClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create TerrainClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x8 }
    _asm { ret }
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TerrainClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, esi); // Current "this" pointer.
    static TerrainClassExtension *ext_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    ext_ptr = TerrainClassExtensions.find_or_create(this_ptr);
    if (!ext_ptr) {
        DEBUG_ERROR("Failed to create TerrainClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create TerrainClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create TerrainClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x0C }
    _asm { ret 8 }
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  We need do this before the unlimbo otherwise finding extension data
 *  will fail if you patch Unlimbo.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TerrainClass_Constructor_Before_Unlimbo_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, esi); // Current "this" pointer.
    GET_STACK_STATIC(Cell *, cell, esp, 0x24);
    static TerrainClassExtension *ext_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    ext_ptr = TerrainClassExtensions.find_or_create(this_ptr);
    if (!ext_ptr) {
        DEBUG_ERROR("Failed to create TerrainClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create TerrainClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create TerrainClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov esi, this_ptr }
    _asm { mov edx, [esi+0x64] } // this->Class
    _asm { mov ecx, cell }

    JMP(0x0063F55D);
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TerrainClass_Deconstructor_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    TerrainClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { add esp, 0x8 }
    _asm { ret }
}


/**
 *  Patch for including the extended class members to the base class detach process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TerrainClass_Detach_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, esi);
    GET_STACK_STATIC(TARGET, target, esp, 0x10);
    GET_STACK_STATIC8(bool, all, esp, 0x8);
    static TerrainClassExtension *ext_ptr;

    /**
     *  Find the extension instance.
     */
    ext_ptr = TerrainClassExtensions.find(this_ptr);
    if (!ext_ptr) {
        goto original_code;
    }

    ext_ptr->Detach(target, all);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { ret 8 }
}


/**
 *  Patch for including the extended class members to the base class crc calculation.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TerrainClass_Compute_CRC_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, esi);
    GET_STACK_STATIC(WWCRCEngine *, crc, esp, 0xC);
    static TerrainClassExtension *ext_ptr;

    /**
     *  Find the extension instance.
     */
    ext_ptr = TerrainClassExtensions.find(this_ptr);
    if (!ext_ptr) {
        goto original_code;
    }

    ext_ptr->Compute_CRC(*crc);

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
void TerrainClassExtension_Init()
{
    Patch_Jump(0x0063F88C, _TerrainClass_Default_Constructor_Patch);
    //Patch_Jump(0x0063F701, _TerrainClass_Constructor_Patch);
    Patch_Jump(0x0063F556, _TerrainClass_Constructor_Before_Unlimbo_Patch);
    Patch_Jump(0x0063F2BC, _TerrainClass_Deconstructor_Patch);
    Patch_Jump(0x0064089F, _TerrainClass_Detach_Patch);
    Patch_Jump(0x0064086E, _TerrainClass_Compute_CRC_Patch);
}
