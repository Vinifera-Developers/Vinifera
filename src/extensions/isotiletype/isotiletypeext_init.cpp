/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ISOTILETYPEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended IsometricTileTypeClass.
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
#include "isotiletypeext_hooks.h"
#include "isotiletypeext.h"
#include "isotiletype.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(IsometricTileTypeClass *, this_ptr, ebp); // "this" pointer.
    GET_STACK_STATIC(const char *, ini_name, esp, 0x50); // ini name.
    static IsometricTileTypeClassExtension *exttype_ptr;

    //DEV_DEBUG_WARNING("Creating IsometricTileTypeClassExtension instance for \"%s\".\n", ini_name);

    /**
     *  Find existing or create an extended class instance.
     */
    exttype_ptr = IsometricTileTypeClassExtensions.find_or_create(this_ptr);
    if (!exttype_ptr) {
        DEBUG_ERROR("Failed to create IsometricTileTypeClassExtension instance for \"%s\"!\n", ini_name);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create IsometricTileTypeClassExtension instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create IsometricTileTypeClassExtension instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x34 }
    _asm { ret 0x14 }
}


/**
 *  Patch for including the extended class members in the noinit creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_NoInit_Constructor_Patch)
{
    GET_REGISTER_STATIC(IsometricTileTypeClass *, this_ptr, esi);
    GET_STACK_STATIC(const NoInitClass *, noinit_ptr, esp, 0x4);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(IsometricTileTypeClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    IsometricTileTypeClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop esi }
    _asm { pop ebx }
    _asm { pop ecx }
    _asm { ret }
}


/**
 *  Patch for including the extended class members to the base class detach process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_Detach_Patch)
{
    GET_REGISTER_STATIC(IsometricTileTypeClass *, this_ptr, esi);
    GET_STACK_STATIC(TARGET, target, esp, 0x4);
    GET_STACK_STATIC8(bool, all, esp, 0x8);
    static IsometricTileTypeClassExtension *exttype_ptr;

    /**
     *  Find the extension instance.
     */
    exttype_ptr = IsometricTileTypeClassExtensions.find(this_ptr, false);
    if (!exttype_ptr) {
        goto original_code;
    }

    /**
     *  Read type class detach.
     */
    exttype_ptr->Detach(target, all);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { ret 8 }
}


/**
 *  Patch for including the extended class members when computing a unique crc value for this instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_Compute_CRC_Patch)
{
    GET_REGISTER_STATIC(IsometricTileTypeClass *, this_ptr, esi);
    GET_STACK_STATIC(WWCRCEngine *, crc, esp, 0xC);
    static IsometricTileTypeClassExtension *exttype_ptr;

    /**
     *  Find the extension instance.
     */
    exttype_ptr = IsometricTileTypeClassExtensions.find(this_ptr);
    if (!exttype_ptr) {
        goto original_code;
    }

    /**
     *  Read type class compute crc.
     */
    exttype_ptr->Compute_CRC(*crc);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
    _asm { pop esi }
    _asm { ret 4 }
}


/**
 *  
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_Init_Patch)
{
    LEA_STACK_STATIC(CCINIClass *, ini, esp, 0x30);
    //GET_STACK_STATIC(const char *, theater_name, esp, 0x3C);

    /**
     *  Load static values.
     */
    IsometricTileTypeClassExtension::Init(*ini);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov ebx, 0x006CAD64 } // const CCFileClass::`vftable'
    JMP(0x004F55F7);
}


/**
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_Read_INI_Patch_1)
{
    GET_REGISTER_STATIC(IsometricTileTypeClass *, this_ptr, ebp);
    LEA_STACK_STATIC(CCINIClass *, ini, esp, 0x30);
    LEA_STACK_STATIC(/*const*/ char *, tilset_name, esp, 0x1F8);
    LEA_STACK_STATIC(/*const*/ char *, set_name, esp, 0x29C);
    static IsometricTileTypeClassExtension *exttype_ptr;

    /**
     *  Stolen bytes here.
     */
    _asm { mov [esp+0x84], ebp } // These must be before to retain stack.
    _asm { mov [esp+0x20], ebp }

    /**
     *  Find the extension instance.
     */
    exttype_ptr = IsometricTileTypeClassExtensions.find(this_ptr);
    if (!exttype_ptr) {
        goto original_code;
    }

    /**
     *  Read type class ini.
     */
    exttype_ptr->TileSetName = tilset_name;
    exttype_ptr->Read_INI(*ini);

    /**
     *  Stolen bytes here.
     */
original_code:
    //_asm { mov [esp+0x84], ebp }
    //_asm { mov [esp+0x20], ebp }
    JMP(0x004F50BD);
}


/**
 *  Patch for reading the extended class members from the ini instance.
 *  
 *  The game in fact uses this patched area for the tile sets that
 *  contain random variations (clear1a, clear1b etc). We need to
 *  read load the ini data here to make sure this linked variation
 *  has the same properties as the main tile set instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_Read_INI_Patch_2)
{
    GET_REGISTER_STATIC(IsometricTileTypeClass *, this_ptr, ebp);
    LEA_STACK_STATIC(CCINIClass *, ini, esp, 0x30);
    LEA_STACK_STATIC(/*const*/ char *, tilset_name, esp, 0x1F8);
    LEA_STACK_STATIC(/*const*/ char *, set_name, esp, 0x29C);
    static IsometricTileTypeClassExtension *exttype_ptr;

    /**
     *  Stolen bytes here.
     */
    _asm { mov [esp+0x20], ebp } // Must be before to retain stack.

    /**
     *  Find the extension instance.
     */
    exttype_ptr = IsometricTileTypeClassExtensions.find(this_ptr);
    if (!exttype_ptr) {
        goto original_code;
    }

    /**
     *  Read type class ini.
     */
    exttype_ptr->TileSetName = tilset_name;
    exttype_ptr->Read_INI(*ini);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { cmp esi, edi }
    JMP(0x004F53EF);
}


/**
 *  Main function for patching the hooks.
 */
void IsometricTileTypeClassExtension_Init()
{
    Patch_Jump(0x004F32C4, &_IsometricTileTypeClass_Constructor_Patch);
    Patch_Jump(0x004F331F, &_IsometricTileTypeClass_NoInit_Constructor_Patch);
    Patch_Jump(0x004F34A2, &_IsometricTileTypeClass_Destructor_Patch);
    //Patch_Jump(0x004F872E, &_IsometricTileTypeClass_Detach_Patch);
    Patch_Jump(0x004F85AA, &_IsometricTileTypeClass_Compute_CRC_Patch);
    Patch_Jump(0x004F55F2, &_IsometricTileTypeClass_Init_Patch);
    Patch_Jump(0x004F50AE, &_IsometricTileTypeClass_Read_INI_Patch_1);
    Patch_Jump(0x004F53E9, &_IsometricTileTypeClass_Read_INI_Patch_2);
}
