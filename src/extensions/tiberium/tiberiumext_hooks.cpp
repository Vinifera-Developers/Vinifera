/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TIBERIUMEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TiberiumClass.
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
#include "tiberiumext_hooks.h"
#include "tiberiumext_init.h"
#include "tiberiumext.h"
#include "tiberium.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "cell.h"
#include "extension.h"
#include "hooker.h"
#include "hooker_macros.h"


 /**
  *  Uses a new extension value as the damage Tiberium deals when exploding.
  *
  *  @author: ZivDero
  */
DECLARE_PATCH(_Chain_Reaction_Damage_Patch)
{
    GET_REGISTER_STATIC(CellClass*, cell, esi);
    GET_REGISTER_STATIC(TiberiumClass*, tib, ebx);
    static bool reduce_tib;
    static int damage;

    reduce_tib = false;
    damage = (cell->OverlayData / 2) * Extension::Fetch<TiberiumClassExtension>(tib)->ChainReactionDamage;

    if (cell->OverlayData >= 11)
        reduce_tib = true;

    cell->OverlayData -= cell->OverlayData / 2;

    _asm
    {
        movzx eax, reduce_tib
        mov [esp + 0xF], al
        mov edi, damage
    }

    JMP_REG(ecx, 0x0045ED29);
}


/**
 *  Main function for patching the hooks.
 */
void TiberiumClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    TiberiumClassExtension_Init();

    Patch_Jump(0x00644DB8, 0x00644DD4); // De-hardcode Power for Tiberium Vinifera
    Patch_Jump(0x0045ED02, _Chain_Reaction_Damage_Patch);
}
