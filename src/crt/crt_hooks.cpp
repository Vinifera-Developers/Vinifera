/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CRT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Setup all the hooks to take control of the basic CRT.
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
#include "crt_hooks.h"
#include <fenv.h>
#include "vinifera_newdel.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Redirect msize() to use HeapSize as we now control all memory allocations.
 */
static unsigned int __cdecl vinifera_msize(void *ptr)
{
    return HeapSize(GetProcessHeap(), 0, ptr);
}


/**
 *  Reimplementation of strdup() to use our allocator.
 */
static char * __cdecl vinifera_strdup(const char *string)
{
    char *str;
    char *p;
    int len = 0;

    while (string[len]) {
        len++;
    }
    str = (char *)vinifera_allocate(len + 1);
    p = str;
    while (*string) {
        *p++ = *string++;
    }
    *p = '\0';
    return str;
}


/**
 *  Set the FPU mode to match the game (rounding towards zero [chop mode]).
 */
DECLARE_PATCH(_set_fp_mode)
{
    // Call to "store_fpu_codeword"
    _asm { mov edx, 0x006B2314 };
    _asm { call edx };

    /**
     *  Set the FPU mode to match the game (rounding towards zero [chop mode]).
     */
    _set_controlfp(_RC_CHOP, _MCW_RC);

    /**
     *  And this is required for the std c++ lib.
     */
    fesetround(FE_TOWARDZERO);

    JMP(0x005FFDB0);
}


/**
 *  Main function for patching the hooks.
 */
void CRT_Hooks()
{
    /**
     *  Call the games fpmath to make sure we init 
     */
    Patch_Jump(0x005FFDAB, &_set_fp_mode);

    /**
     *  dynamic init functions call _msize indirectly.
     *  They call __onexit, so we need to patch this.
     */
    Hook_Function(0x006B80AA, &vinifera_msize);

    /**
     *  Standard functions.
     */
    Hook_Function(0x006BE766, &vinifera_strdup);

    /**
     *  C memory functions.
     */
    Hook_Function(0x006B72CC, &vinifera_allocate);
    Hook_Function(0x006BCA26, &vinifera_count_allocate);
    Hook_Function(0x006B7F72, &vinifera_reallocate);
    Hook_Function(0x006B67E4, &vinifera_free);

    /**
     *  C++ new and delete.
     */
    Hook_Function(0x006B51D7, &vinifera_allocate);
    Hook_Function(0x006B51CC, &vinifera_free);

    /**
     *  Redirect the games CRT functions to use use the DLL's CRT.
     */
    Hook_Function(0x006B602A, &std::strtok);
}
