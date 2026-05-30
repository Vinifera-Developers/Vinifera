/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended OverlayTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "overlaytypeext_hooks.h"

#include "overlaytypeext_init.h"
#include "syringe.h"


/**
 *  Main function for patching the hooks.
 */
void OverlayTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    OverlayTypeClassExtension_Init();
}
