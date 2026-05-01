/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended WeaponTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "weapontypeext_hooks.h"

#include "animtype.h"
#include "findmake.h"
#include "syringe.h"
#include "weapontype.h"
#include "weapontypeext_init.h"


/**
 *  #issue-391
 *
 *  Expands the buffer size used to read the AnimType list.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00680F07, _WeaponTypeClass_Read_INI_Get_AnimTypes_Patch, 0)
{
    GET(WeaponTypeClass*, this_ptr, ESI);
    GET(CCINIClass*, ini, EBX);
    GET(const char*, ini_name, EDI);

    /**
     *  Load the AnimType list.
     */
    this_ptr->Anim = TGet_TypeList(*ini, ini_name, "Anim", this_ptr->Anim);

    return 0x00681004;
}


/**
 *  Main function for patching the hooks.
 */
void WeaponTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    WeaponTypeClassExtension_Init();
}
