/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended CellClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "cellext.h"

#include "cell.h"
#include "extension.h"
#include "tibsun_inline.h"


/**
 *  Replacement for CellClass::Spread_Tiberium that supports a custom Tiberium richness.
 *
 *  @author: CCHyper
 */
bool CellClassExtension::Spread_Tiberium(CellClass* this_ptr, bool forced, int richness)
{
    if (!forced) {
        if (!this_ptr->Can_Tiberium_Spread()) return (false);
    }

    TiberiumType tibtype = this_ptr->Tiberium_Type_Here();
    tibtype = forced && tibtype == TIBERIUM_NONE ? TIBERIUM_FIRST : tibtype;

    if (tibtype != TIBERIUM_NONE) {
        TiberiumClass* tiberium = Tiberiums[tibtype];
        FacingType offset = Random_Pick(FACING_N, FACING_NW);
        for (FacingType index = FACING_N; index < FACING_COUNT; index++) {
            CellClass* newcell = &this_ptr->Adjacent_Cell(FacingType((index + offset) & 7));

            if (newcell != NULL && newcell->Can_Tiberium_Germinate(tiberium)) {
                return (newcell->Place_Tiberium(tibtype, richness));
            }
        }
    }
    return (false);
}
