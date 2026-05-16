/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the main function that sets up all hooks.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "setup_hooks.h"

/**
 *  Include the hook headers here.
 */
#include "crt_hooks.h"
#include "debug_hooks.h"
#include "extension_hooks.h"
#include "movieplayback_hooks.h"
#include "newswizzle_hooks.h"
#include "sidebarext_hooks.h"
#include "vinifera_hooks.h"


void Setup_Hooks()
{
    CRT_Hooks();
    Debug_Hooks();
    Vinifera_Hooks();
    NewSwizzle_Hooks();
    Extension_Hooks();
    MoviePlayback_Hooks();
}
