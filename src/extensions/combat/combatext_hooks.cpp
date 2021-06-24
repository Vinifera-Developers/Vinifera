/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          COMBATEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended combat functions.
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
#include "buildingext_hooks.h"
#include "combat.h"
#include "cell.h"
#include "overlaytype.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-410
 * 
 *  Implements IsWallAbsoluteDestroyer for WarheadTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Explosion_Damage_IsWallAbsoluteDestroyer_Patch)
{
    GET_REGISTER_STATIC(OverlayTypeClass *, overlay, esi);
    GET_REGISTER_STATIC(const WarheadTypeClass *, warhead, ebx);
    GET_REGISTER_STATIC(CellClass *, cellptr, edi);
    GET_STACK_STATIC(int, strength, esp, 0x54);
    static const WarheadTypeClassExtension *warheadtypeext;

    /**
     *  Check to make sure that the warhead is of the kind that can destroy walls.
     */
    if (overlay->IsWall) {

        /**
         *  Is this warhead capable of instantly destroying the wall regardless
         *  of damage? If so, then pass -1 into Reduce_Wall to remove the wall
         *  section from the cell.
         */
        warheadtypeext = WarheadTypeClassExtensions.find(warhead);
        if (warheadtypeext && warheadtypeext->IsWallAbsoluteDestroyer) {
            cellptr->Reduce_Wall(-1);

        /**
         *  Original check.
         */
        } else if (warhead->IsWallDestroyer || warhead->IsWoodDestroyer || overlay->Armor == ARMOR_WOOD) {
            cellptr->Reduce_Wall(strength);
        }

    }

    JMP_REG(ecx, 0x0045FAD0);
}


/**
 *  Main function for patching the hooks.
 */
void CombatExtension_Hooks()
{
    Patch_Jump(0x0045FAA0, &_Explosion_Damage_IsWallAbsoluteDestroyer_Patch);
}
