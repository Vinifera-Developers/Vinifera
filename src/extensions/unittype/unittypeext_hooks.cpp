/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended UnitTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "unittypeext_hooks.h"

#include "unittypeext_init.h"


/**
 *  Main function for patching the hooks.
 */
void UnitTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    UnitTypeClassExtension_Init();
}
