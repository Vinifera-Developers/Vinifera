/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PARTICLESYSTYPEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended ParticleSystemTypeClass.
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
#include "particlesystypeext_hooks.h"
#include "particlesystypeext.h"
#include "particlesystype.h"
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
DECLARE_PATCH(_ParticleSystemTypeClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(ParticleSystemTypeClass *, this_ptr, esi); // "this" pointer.
    GET_STACK_STATIC(const char *, ini_name, esp, 0xC); // ini name.
    static ParticleSystemTypeClassExtension *exttype_ptr;

    //DEV_DEBUG_WARNING("Creating ParticleSystemTypeClassExtension instance for \"%s\".\n", ini_name);

    /**
     *  Find existing or create an extended class instance.
     */
    exttype_ptr = ParticleSystemTypeClassExtensions.find_or_create(this_ptr);
    if (!exttype_ptr) {
        DEBUG_ERROR("Failed to create ParticleSystemTypeClassExtensions instance for \"%s\"!\n", ini_name);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create ParticleSystemTypeClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create ParticleSystemTypeClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members in the noinit creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ParticleSystemTypeClass_NoInit_Constructor_Patch)
{
    GET_REGISTER_STATIC(ParticleSystemTypeClass *, this_ptr, esi);
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
DECLARE_PATCH(_ParticleSystemTypeClass_Destructor_Patch)
{
    GET_REGISTER_STATIC(ParticleSystemTypeClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    ParticleSystemTypeClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop esi }
    _asm { pop ecx }
    _asm { ret }
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ParticleSystemTypeClass_Scalar_Destructor_Patch)
{
    GET_REGISTER_STATIC(ParticleSystemTypeClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    ParticleSystemTypeClassExtensions.remove(this_ptr);

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
DECLARE_PATCH(_ParticleSystemTypeClass_Compute_CRC_Patch)
{
    GET_REGISTER_STATIC(ParticleSystemTypeClass *, this_ptr, esi);
    GET_STACK_STATIC(WWCRCEngine *, crc, esp, 0xC);
    static ParticleSystemTypeClassExtension *exttype_ptr;

    /**
     *  Find the extension instance.
     */
    exttype_ptr = ParticleSystemTypeClassExtensions.find(this_ptr);
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
DECLARE_PATCH(_ParticleSystemTypeClass_Read_INI_Patch)
{
    GET_REGISTER_STATIC(ParticleSystemTypeClass *, this_ptr, esi);
    GET_STACK_STATIC(CCINIClass *, ini, esp, 0x90);
    static ParticleSystemTypeClassExtension *exttype_ptr;

    /**
     *  Find the extension instance.
     */
    exttype_ptr = ParticleSystemTypeClassExtensions.find(this_ptr);
    if (!exttype_ptr) {
        goto original_code;
    }

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
    _asm { pop ebx }
    _asm { add esp, ebp }
    _asm { pop ebp }
    _asm { ret 4 }
}


/**
 *  Main function for patching the hooks.
 */
void ParticleSystemTypeClassExtension_Init()
{
    Patch_Jump(0x005AE537, &_ParticleSystemTypeClass_Constructor_Patch);
    Patch_Jump(0x005AE55A, &_ParticleSystemTypeClass_NoInit_Constructor_Patch);
    //Patch_Jump(0x005AE5C8, &_ParticleSystemTypeClass_Destructor_Patch); // Destructor is actually inlined in scalar destructor!
    Patch_Jump(0x005AECC8, &_ParticleSystemTypeClass_Scalar_Destructor_Patch);
    //Patch_Jump(0x005AEA9A, &_ParticleSystemTypeClass_Compute_CRC_Patch);
    //Patch_Jump(0x005AE90C, &_ParticleSystemTypeClass_Read_INI_Patch);
}
