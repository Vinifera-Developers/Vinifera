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
DECLARE_PATCH(_IsometricTileTypeClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(IsometricTileTypeClass *, this_ptr, ebp); // "this" pointer.
    GET_STACK_STATIC(const char *, ini_name, esp, 0x50); // ini name.

    // IsoTileTypes's are not saved to file, so this case is not required.
#if 0
    /**
     *  If we are performing a load operation, the Windows API will invoke the
     *  constructors for us as part of the operation, so we can skip our hook here.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }
#endif

    /**
     *  Create an extended class instance.
     */
    Extension::Make<IsometricTileTypeClassExtension>(this_ptr);

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
    Extension::Destroy<IsometricTileTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov edx, ds:0x0080F588 } // Neuron vector vtble
    JMP_REG(eax, 0x004F33CC);
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
    LEA_STACK_STATIC(/*const*/ char *, tileset_name, esp, 0x1F8);
    LEA_STACK_STATIC(/*const*/ char *, set_name, esp, 0x29C);
    static IsometricTileTypeClassExtension *exttype_ptr;

    /**
     *  Stolen bytes here.
     */
    _asm { mov [esp+0x84], ebp } // These must be before to retain stack.
    _asm { mov [esp+0x20], ebp }

    /**
     *  Fetch the extension instance.
     */
    exttype_ptr = Extension::Fetch(this_ptr);

    /**
     *  Read type class ini.
     */
    std::strncpy(exttype_ptr->TileSetName, tileset_name, sizeof(exttype_ptr->TileSetName) - 1);
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
    LEA_STACK_STATIC(/*const*/ char *, tileset_name, esp, 0x1F8);
    LEA_STACK_STATIC(/*const*/ char *, set_name, esp, 0x29C);
    static IsometricTileTypeClassExtension *exttype_ptr;

    /**
     *  Stolen bytes here.
     */
    _asm { mov [esp+0x20], ebp } // Must be before to retain stack.

    /**
     *  Fetch the extension instance.
     */
    exttype_ptr = Extension::Fetch(this_ptr);

    /**
     *  Read type class ini.
     */
    std::strncpy(exttype_ptr->TileSetName, tileset_name, sizeof(exttype_ptr->TileSetName) - 1);
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
    Patch_Jump(0x004F33C6, &_IsometricTileTypeClass_Destructor_Patch);
    Patch_Jump(0x004F55F2, &_IsometricTileTypeClass_Init_Patch);
    Patch_Jump(0x004F50AE, &_IsometricTileTypeClass_Read_INI_Patch_1);
    Patch_Jump(0x004F53E9, &_IsometricTileTypeClass_Read_INI_Patch_2);
}
