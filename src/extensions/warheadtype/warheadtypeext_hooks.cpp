/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended WarheadTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "warheadtypeext_hooks.h"

#include "debughandler.h"
#include "hooker.h"
#include "unittype.h"
#include "verses.h"
#include "warheadtypeext.h"
#include "warheadtypeext_init.h"


static const WarheadTypeClass* _Find_Or_Make(const char* name)
{
    const WarheadType warhead = WarheadTypeClass::From_Name(name);

    if (warhead == WARHEAD_NONE)
    {
        const WarheadTypeClass* wptr = WarheadTypeClass::Find_Or_Make(name);

        if (wptr != nullptr)
        {
            DEBUG_WARNING("Requested Warhead {} that is not listed under [Warheads]! To increase loading speed, please consider listing it under [Warheads]!\n", name);
            Verses::Resize();
        }

        return wptr;
    }

    return Warheads[warhead];
}


/**
 *  Main function for patching the hooks.
 */
void WarheadTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    WarheadTypeClassExtension_Init();

    /**
     *  Skip reading verses in the vanilla function to prevent crashes when there are not enough specified.
     */
    Patch_Jump(0x0066F41B, 0x0066F4A4);

    /**
     * Patch calls to WarheadTypeClass::Find_Or_Make to ensure our Verses vectors are resized properly.
     */
    Patch_Call(0x00419526, &_Find_Or_Make);
    Patch_Call(0x005AF345, &_Find_Or_Make);
    Patch_Call(0x005C6BA8, &_Find_Or_Make);
    Patch_Call(0x005C6C64, &_Find_Or_Make);
    Patch_Call(0x005C8F65, &_Find_Or_Make);
    Patch_Call(0x005C8FA4, &_Find_Or_Make);
    Patch_Call(0x005C8FE2, &_Find_Or_Make);
    Patch_Call(0x005C9021, &_Find_Or_Make);
    Patch_Call(0x005C9060, &_Find_Or_Make);
    Patch_Call(0x005C909E, &_Find_Or_Make);
    Patch_Call(0x005D3A5B, &_Find_Or_Make);
    Patch_Call(0x0065FA83, &_Find_Or_Make);
    Patch_Call(0x00681211, &_Find_Or_Make);
}
