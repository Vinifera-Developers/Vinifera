/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended AircraftTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "aircrafttypeext_hooks.h"

#include "aircrafttypeext_init.h"


/**
 *  Main function for patching the hooks.
 */
void AircraftTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    AircraftTypeClassExtension_Init();
}
