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
#include "overlaytype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "cell.h"
#include "extension.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "debughandler.h"
#include "tibsun_inline.h"


/**
 *  Replace the log comment with a more descriptive one.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_Get_Tiberium_Type_Debug_Info_Patch)
{
    GET_REGISTER_STATIC(OverlayTypeClass*, overlaytype, eax);

    DEBUG_FATAL("Overlay %s [%d] is not really Tiberium!\nAll overlays with Tiberium=yes must be used by a Tiberium!\n", overlaytype->Full_Name(), overlaytype->Fetch_Heap_ID());

    JMP(0x0058C951);
}


/**
 *  Randomize the overlay index when placing Tiberium within the Tiberium's variety,
 *  not from 0 to 11.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_CellClass_Place_Tiberium_Variety_Patch)
{
    GET_REGISTER_STATIC(TiberiumClass*, tiberium, ebp);
    static int frame;

    frame = Random_Pick(0, tiberium->Variety - 1);

    _asm mov eax, frame
    JMP_REG(edx, 0x0045CEC8);
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

    /**
     *  De-hardcode Power for Tiberium Vinifera.
     */
    Patch_Jump(0x00644DB8, 0x00644DD4); // 

    /**
     *  OverlayTypes indexes 27 to 38 (fourth Tiberium images) are hardcoded to be
     *  impassable by infantry. This hack removes this.
     */
    Patch_Jump(0x004D54E7, 0x004D5507);

    Patch_Jump(0x0058C934, _Get_Tiberium_Type_Debug_Info_Patch);
    Patch_Jump(0x0045CEB9, _CellClass_Place_Tiberium_Variety_Patch);
}
