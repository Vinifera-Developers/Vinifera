/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended StorageClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "storageext_hooks.h"

#include "hooker.h"
#include "storageext.h"


/**
 *  Main function for patching the hooks.
 */
void StorageClassExtension_Hooks()
{
    /**
     *  Patch all the methods of StorageClass to our new extension class.
     *  Operators '+' and '-' are not patched because they are not used in the game,
     *  and require us to instantiate a new class, which we cannot do
     *  (because we now store the amounts in a DVC that belongs to the owner class)
     */
    Patch_Jump(0x0060AD80, &StorageClassExt::Get_Total_Value);
    Patch_Jump(0x0060ADB0, &StorageClassExt::Get_Total_Amount);
    Patch_Jump(0x0060ADD0, &StorageClassExt::Get_Amount);
    Patch_Jump(0x0060ADE0, &StorageClassExt::Increase_Amount);
    Patch_Jump(0x0060AE00, &StorageClassExt::Decrease_Amount);
    Patch_Jump(0x0060AFA0, &StorageClassExt::First_Used_Slot);
    Patch_Jump(0x0060AE90, &StorageClassExt::operator+=);
    Patch_Jump(0x0060AF50, &StorageClassExt::operator-=);
}
