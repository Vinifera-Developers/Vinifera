/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended SideClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sideext_hooks.h"

#include "sideext_init.h"


/**
 *  Main function for patching the hooks.
 */
void SideClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    SideClassExtension_Init();
}
