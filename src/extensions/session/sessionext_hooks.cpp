/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended SessionClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sessionext_hooks.h"

#include "sessionext_init.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Main function for patching the hooks.
 */
void SessionClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    SessionClassExtension_Init();

    /**
     *  #issue-218
     *
     *  Changes the default value of SessionClass 0x1D91 (IsGDI) from "1" to "0".. This is
     *  because we now use it as a HouseType index, and need it to default to the first index.
     */
    Patch_Byte(0x005ED06B+1, 0x85); // changes "dl" (1) to "al" (0)
}
