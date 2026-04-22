/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended EMPulseClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "buildingtypeext.h"
#include "extension.h"
#include "foot.h"
#include "hooker.h"
#include "syringe.h"
#include "techno.h"
#include "technotype.h"

/**
 *  #issue-181
 *
 *  Implements optional EMP (electromagnetic pulse) immunity for buildings.
 *  Typically buildings are disabled by EMP. This patch allows
 *  making specific building types immune to the effect.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00492C45, _EMPulseClass_Create_Building_EMPImmune_Patch, 0)
{
    GET(BuildingTypeClass *, buildingtype, EAX);

    auto exttype_ptr = Extension::Fetch(buildingtype);

    /**
     *  Is this building immune to EMP weapons?
     */
    if (exttype_ptr->IsImmuneToEMP) {
        goto loop_continue;
    }

    /**
     *  Stolen bytes / code.
     */
original_code:
    if (buildingtype->IsInvisibleInGame) {
        goto loop_continue;
    }

    return 0x00492C53;

    /**
     *  Continue looping through affected cells.
     */
loop_continue:
    return 0x00492F93;
}

/**
 *  #issue-181
 *
 *  Implements optional EMP (electromagnetic pulse) immunity for mobile TechnoTypes 
 *  (Technos that derive from FootClass).
 *  Typically vehicles and cyborgs are disabled by EMP. This patch
 *  allows making specific object types immune to the effect.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00492E84, _EMPulseClass_Create_Foot_EMPImmune_Patch, 0)
{
    GET(FootClass *, foot, ESI);

    auto exttype_ptr = Extension::Fetch(foot->TClass);

    /**
     *  Is this object immune to EMP weapons?
     */
    if (exttype_ptr->IsImmuneToEMP) {
        goto loop_continue;
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    foot->Locomotion->Power_Off();

    return 0x00492EB8;

    /**
     *  Continue looping through the cell occupiers.
     */
loop_continue:
    return 0x00492F78;
}


/**
 *  Main function for patching the hooks.
 */
void EMPulseClassExtension_Hooks()
{
}
