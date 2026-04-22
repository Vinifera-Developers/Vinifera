/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended TechnoTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "technotypeext_hooks.h"

#include "asserthandler.h"
#include "extension.h"
#include "extension_globals.h"
#include "hooker.h"
#include "house.h"
#include "rulesext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_defines.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(TechnoTypeClass)
{
public:
    int _Max_Pips() const;
};


/**
 *  Reimplements TechnoTypeClass::Max_Pips.
 *
 *  @author: ZivDero
 */
int TechnoTypeClassExt::_Max_Pips() const
{
    int max_pips = 0;
    if (PipScale - 1 < RuleExtension->MaxPips.Count())
        max_pips = RuleExtension->MaxPips[PipScale - 1];

    // Negative values are not allowed
    if (max_pips < 0)
        return 0;

    switch (PipScale)
    {
    case PIPSCALE_AMMO:
        return std::clamp(MaxAmmo, 0, max_pips);

    case PIPSCALE_PASSENGERS:
        return std::clamp(MaxPassengers, 0, max_pips);

    case PIPSCALE_TIBERIUM:
    case PIPSCALE_POWER:
    case PIPSCALE_CHARGE:
    default:
        return max_pips;
    }
}


/**
 *  Main function for patching the hooks.
 */
void TechnoTypeClassExtension_Hooks()
{
    Patch_Jump(0x0063D460, &TechnoTypeClassExt::_Max_Pips);
}
