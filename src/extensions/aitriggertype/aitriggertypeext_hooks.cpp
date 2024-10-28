/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AITRIGGERTYPEEXT_HOOKS.H
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the extended AITriggerType class.
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
#include "aitriggertypeext_hooks.h"

#include "aitrigtype.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "tibsun_globals.h"
#include "house.h"
#include "vector.h";

#include "hooker.h"
#include "hooker_macros.h"
#include "scenario.h"
#include "vinifera_globals.h"


/**
 *  Patch that AITriggerTypes no longer assume that not-GDI means Nod and vice versa.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_AITriggerTypeClass_Process_MultiSide_Patch)
{
    GET_REGISTER_STATIC(AITriggerTypeClass*, trigtype, esi);
    GET_REGISTER_STATIC(HouseClass*, house, ebp);

    if (trigtype->MultiSide != 0 && trigtype->MultiSide != house->ActLike + 1)
    {
        // return 0;
        JMP(0x00410A00);
    }

    JMP(0x00410A1F);
}


/**
 *  Main function for patching the hooks.
 */
void AITriggerTypeClassExtension_Hooks()
{
    Patch_Jump(0x004109EF, &_AITriggerTypeClass_Process_MultiSide_Patch);
}
