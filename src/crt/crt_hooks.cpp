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
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


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
#ifdef _DEBUG
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);
#endif

    /**
     *  Call the games fpmath to make sure we init 
     */
    Patch_Jump(0x005FFDAB, &_set_fp_mode);

    /**
     *  Standard functions.
     */
    Hook_Function(0x006B602A, &std::strtok);
    Hook_Function(0x006BE766, &strdup);

    /**
     *  C memory functions.
     */
    Hook_Function(0x006B72CC, &std::malloc);
    Hook_Function(0x006BCA26, &std::calloc);
    Hook_Function(0x006B7F72, &std::realloc);
    Hook_Function(0x006B67E4, &std::free);
    Hook_Function(0x006B80AA, &_msize);

    /**
     *  C++ new and delete.
     */
    Hook_Function(0x006B51D7, &std::malloc);
    Hook_Function(0x006B51CC, &std::free);
}
