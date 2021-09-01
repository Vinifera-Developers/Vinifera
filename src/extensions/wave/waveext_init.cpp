/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WAVEEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended WaveClass.
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
#include "waveext.h"
#include "wave.h"
#include "techno.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "vinifera_util.h"


#if 0
/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WaveClass_Default_Constructor_Patch)
{
    GET_REGISTER_STATIC(WaveClass *, this_ptr, esi); // Current "this" pointer.
    static WaveClassExtension *ext_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    ext_ptr = WaveClassExtensions.find_or_create(this_ptr);
    if (!ext_ptr) {
        DEBUG_ERROR("Failed to create WaveClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create WaveClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create WaveClassExtensions instance!\n");
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
    _asm { mov esp, ebp }
    _asm { pop ebp }
    _asm { ret }
}
#endif


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  We need do this before the wave values are initialised otherwise finding
 *  extension data will fail.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WaveClass_Default_Constructor_Before_Init_Patch)
{
    GET_REGISTER_STATIC(WaveClass *, this_ptr, esi); // Current "this" pointer.
    static WaveClassExtension *ext_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    ext_ptr = WaveClassExtensions.find_or_create(this_ptr);
    if (!ext_ptr) {
        DEBUG_ERROR("Failed to create WaveClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create WaveClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create WaveClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    static int waves_vector_max;
    waves_vector_max = Waves.Length();
    _asm { mov eax, waves_vector_max }
    JMP_REG(ecx, 0x0067018E);
}


#if 0
/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WaveClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(WaveClass *, this_ptr, esi); // Current "this" pointer.
    static WaveClassExtension *ext_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    ext_ptr = WaveClassExtensions.find_or_create(this_ptr);
    if (!ext_ptr) {
        DEBUG_ERROR("Failed to create WaveClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create WaveClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create WaveClassExtensions instance!\n");
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
    _asm { mov esp, ebp }
    _asm { pop ebp }
    _asm { ret 0x14 }
}
#endif


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  We need do this before the wave values are initialised otherwise finding
 *  extension data will fail.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WaveClass_Constructor_Before_Init_Patch)
{
    GET_REGISTER_STATIC(WaveClass *, this_ptr, esi); // Current "this" pointer.
    static WaveClassExtension *ext_ptr;

    /**
     *  Find existing or create an extended class instance.
     */
    ext_ptr = WaveClassExtensions.find_or_create(this_ptr);
    if (!ext_ptr) {
        DEBUG_ERROR("Failed to create WaveClassExtension instance for 0x%08X!\n", (uintptr_t)this_ptr);
        ShowCursor(TRUE);
        MessageBoxA(MainWindow, "Failed to create WaveClassExtensions instance!\n", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
        Vinifera_Generate_Mini_Dump();
        Fatal("Failed to create WaveClassExtensions instance!\n");
        goto original_code; // Keep this for clean code analysis.
    }

    /**
     *  Stolen bytes here.
     */
original_code:
    static int waves_vector_max;
    waves_vector_max = Waves.Length();
    _asm { mov eax, waves_vector_max }
    JMP_REG(ecx, 0x0066FED4);
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WaveClass_Deconstructor_Patch)
{
    GET_REGISTER_STATIC(WaveClass *, this_ptr, esi);

    /**
     *  Remove the extended class from the global index.
     */
    WaveClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { pop edi }
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
DECLARE_PATCH(_WaveClass_Scalar_Destructor_Patch)
{
    GET_REGISTER_STATIC(WaveClass *, this_ptr, edi);

    /**
     *  Remove the extended class from the global index.
     */
    WaveClassExtensions.remove(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop ecx }
    _asm { ret 4 }
}


/**
 *  Patch for including the extended class members to the base class detach process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_WaveClass_Detach_Patch)
{
    GET_REGISTER_STATIC(WaveClass *, this_ptr, esi);
    GET_STACK_STATIC(TARGET, target, esp, 0x10);
    GET_STACK_STATIC8(bool, all, esp, 0x8);
    static WaveClassExtension *ext_ptr;

    /**
     *  Find the extension instance.
     */
    ext_ptr = WaveClassExtensions.find(this_ptr);
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
 *  Main function for patching the hooks.
 */
void WaveClassExtension_Init()
{
    //Patch_Jump(0x006702B2, &_WaveClass_Default_Constructor_Patch);
    Patch_Jump(0x00670189, &_WaveClass_Default_Constructor_Before_Init_Patch);
    //Patch_Jump(0x006700A2, &_WaveClass_Constructor_Patch);
    Patch_Jump(0x0066FECF, &_WaveClass_Constructor_Before_Init_Patch);
    Patch_Jump(0x00670369, &_WaveClass_Deconstructor_Patch); // Destructor is actually inlined in scalar destructor!
    Patch_Jump(0x00672F1B, &_WaveClass_Scalar_Destructor_Patch);
    Patch_Jump(0x00670B3D, &_WaveClass_Detach_Patch);
}
