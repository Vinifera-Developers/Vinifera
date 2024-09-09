/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOUSEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended HouseClass.
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
#include "houseext_hooks.h"
#include "houseext.h"
#include "house.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "vinifera_globals.h"
#include "extension.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "storageext.h"


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_HouseClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(HouseClass *, this_ptr, ebp); // "this" pointer.
    GET_STACK_STATIC(const char *, ini_name, esp, 0xC); // ini name.

    /**
     *  If we are performing a load operation, the Windows API will invoke the
     *  constructors for us as part of the operation, so we can skip our hook here.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }

    /**
     *  Create an extended class instance.
     */
    Extension::Make<HouseClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x2C }
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_HouseClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(HouseClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<HouseClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov edx, ds:0x007E1558 } // Houses.vtble
    JMP_REG(eax, 0x004BB9BD);
}


static void Put_Storage_Pointers(HouseClass* house)
{
    new ((StorageClassExt*)&(house->Tiberium)) StorageClassExt(&Extension::Fetch<HouseClassExtension>(house)->TiberiumStorage);
    new ((StorageClassExt*)&(house->Weed)) StorageClassExt(&Extension::Fetch<HouseClassExtension>(house)->WeedStorage);
}


DECLARE_PATCH(_HouseClass_Load_StorageExtPtr)
{
    GET_REGISTER_STATIC(int, result, eax);
    GET_REGISTER_STATIC(HouseClass*, this_ptr, esi);

    Put_Storage_Pointers(this_ptr);

    if (result >= 0)
    {
        JMP(0x004C4AD9);
    }

    JMP(0x004C503B);
}


/**
 *  Main function for patching the hooks.
 */
void HouseClassExtension_Init()
{
    Patch_Jump(0x004BAEBE, &_HouseClass_Constructor_Patch);
    Patch_Jump(0x004BB9B7, &_HouseClass_Destructor_Patch);
    Patch_Jump(0x004C4AD1, &_HouseClass_Load_StorageExtPtr);
}
