/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended WaveClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "waveext_hooks.h"

#include "waveext_init.h"


/**
 *  Main function for patching the hooks.
 */
void WaveClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    WaveClassExtension_Init();
}
