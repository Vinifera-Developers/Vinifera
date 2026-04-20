/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CELLEXT.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Extended CellClass class.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
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
