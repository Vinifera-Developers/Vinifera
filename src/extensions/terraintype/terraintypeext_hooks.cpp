/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended TerrainTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "terraintypeext_hooks.h"

#include "terraintypeext_init.h"


/**
 *  Main function for patching the hooks.
 */
void TerrainTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    TerrainTypeClassExtension_Init();
}
