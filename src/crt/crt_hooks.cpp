/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Setup all the hooks to take control of the basic CRT.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "crt_hooks.h"

#include "asserthandler.h"
#include "debughandler.h"
#include "hooker.h"
#include "hooker_macros.h"

#include <crtdbg.h>
#include <cstring>
#include <fenv.h>


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
