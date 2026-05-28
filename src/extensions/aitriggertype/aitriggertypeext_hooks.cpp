/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended AITriggerType class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "aitriggertypeext_hooks.h"

#include "aitrigtype.h"
#include "hooker.h"
#include "house.h"
#include "syringe.h"


/**
 *  Patch that AITriggerTypes no longer assume that not-GDI means Nod and vice versa.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004109EF, _AITriggerTypeClass_Process_MultiSide_Patch, 0)
{
    GET(AITriggerTypeClass*, trigtype, ESI);
    GET(HouseClass*, house, EBP);

    if (trigtype->MultiSide != 0 && trigtype->MultiSide != house->ActLike + 1) {
        return 0x00410A00;
    }

    return 0x00410A1F;
}


/**
 *  Main function for patching the hooks.
 */
void AITriggerTypeClassExtension_Hooks()
{
}
