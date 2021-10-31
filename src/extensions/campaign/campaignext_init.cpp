/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CAMPAIGNEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended CampaignClass.
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
#include "campaignext_hooks.h"
#include "campaignext.h"
#include "campaign.h"
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
DECLARE_PATCH(_CampaignClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(CampaignClass *, this_ptr, ebp); // "this" pointer.
    GET_STACK_STATIC(const char *, ini_name, esp, 0x10); // ini name.
    static CampaignClassExtension *exttype_ptr;

    //EXT_DEBUG_WARNING("Creating CampaignClassExtension instance for \"%s\".\n", ini_name);

    /**
     *  Find existing or create an extended class instance.
     */
    exttype_ptr = CampaignClassExtensions.find_or_create(this_ptr);
    if (!exttype_ptr) {
        DEBUG_ERROR("Failed to create CampaignClassExtensions instance for \"%s\"!\n", ini_name);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create CampaignClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create CampaignClassExtensions instance!\n");
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
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CampaignClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(CampaignClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    CampaignClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop esi }
    _asm { pop ecx }
    _asm { ret }
}


/**
 *  Patch for removing the inlined constructor and replacing it with a direct call.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CampaignClass_Process_Patch)
{
    static uintptr_t constructor_addr = 0x00448A10;

    _asm { lea ecx, [esp+0x18] } // ini_name
    _asm { push ecx }
    _asm { mov ecx, ebp } // ebp == memory pointer
    _asm { call constructor_addr } // CampaignClass::CampaignClass()

    _asm { mov ecx, eax }

    JMP(0x00448D86);
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CampaignClass_Scalar_Destructor_Patch)
{
    GET_REGISTER_STATIC(CampaignClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    CampaignClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { pop ecx }
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members when computing a unique crc value for this instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CampaignClass_Compute_CRC_Patch)
{
    GET_REGISTER_STATIC(CampaignClass *, this_ptr, esi);
    GET_STACK_STATIC(WWCRCEngine *, crc, esp, 0xC);
    static CampaignClassExtension *exttype_ptr;

    /**
     *  Find the extension instance.
     */
    exttype_ptr = CampaignClassExtensions.find(this_ptr);
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
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CampaignClass_Read_INI_Patch)
{
    GET_REGISTER_STATIC(int, required_addon, eax);  // Return from ini.Get_Int("RequiredAddon")
    GET_REGISTER_STATIC(CampaignClass *, this_ptr, esi);
    GET_REGISTER_STATIC(CCINIClass *, ini, ebx);
    static CampaignClassExtension *exttype_ptr;

    /**
     *  Stolen bytes here.
     */
    this_ptr->RequiredAddon = (AddonType)required_addon;

    /**
     *  Find the extension instance.
     */
    exttype_ptr = CampaignClassExtensions.find(this_ptr);
    if (!exttype_ptr) {
        goto original_code;
    }

    /**
     *  Read type class ini.
     */
    exttype_ptr->Read_INI(*ini);

original_code:
    _asm { mov al, 1 }
    _asm { pop edi }
    _asm { pop ebp }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret 4 }
}


/**
 *  Main function for patching the hooks.
 */
void CampaignClassExtension_Init()
{
    Patch_Jump(0x00448AC4, &_CampaignClass_Constructor_Patch);
    //Patch_Jump(0x00448B38, &_CampaignClass_Destructor_Patch); // Destructor is actually inlined in scalar destructor!
    Patch_Jump(0x00448CD0, &_CampaignClass_Process_Patch); // Constructor is also inlined in CampaignClass::Process!
    Patch_Jump(0x00448F58, &_CampaignClass_Scalar_Destructor_Patch);
    Patch_Jump(0x00448E4E, &_CampaignClass_Compute_CRC_Patch);
    Patch_Jump(0x00448C17, &_CampaignClass_Read_INI_Patch);
}
