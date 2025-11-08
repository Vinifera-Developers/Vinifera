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
#include "debughandler.h"
#include "syringe.h"
#include "tibsun_inline.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(TiberiumClass)
{
public:
    void _Spread_AI() { Extension::Fetch(this)->Spread_AI(); }
    void _Init_Spread() { Extension::Fetch(this)->Init_Spread(); }
    void _Recalc_Spread() { Extension::Fetch(this)->Recalc_Spread(); }
    void _Clear_Spread() { Extension::Fetch(this)->Clear_Spread(); }
    void _Queue_Spread(Cell const& cell) { Extension::Fetch(this)->Queue_Spread(cell); }
    void _Growth_AI() { Extension::Fetch(this)->Growth_AI(); }
    void _Init_Growth() { Extension::Fetch(this)->Init_Growth(); }
    void _Recalc_Growth() { Extension::Fetch(this)->Recalc_Growth(); }
    void _Clear_Growth() { Extension::Fetch(this)->Clear_Growth(); }
    void _Queue_Growth(Cell const& cell) { Extension::Fetch(this)->Queue_Growth(cell); }

    static void _Initialize_Tiberium_Spread_System();
    static void _Deinitialize_Tiberium_Spread_System();
    static void _Initialize_Tiberium_Growth_System();
    static void _Deinitialize_Tiberium_Growth_System();
    static void _Clear_Spread_State(Cell const& cell) { TiberiumClassExtension::Clear_Spread_State(cell); }
};


void TiberiumClassExt::_Initialize_Tiberium_Spread_System()
{
    for (int i = 0; i < Tiberiums.Count(); i++) {
        Tiberiums[i]->Init_Spread();
    }
}


void TiberiumClassExt::_Deinitialize_Tiberium_Spread_System()
{
    for (int i = 0; i < Tiberiums.Count(); i++) {
        Tiberiums[i]->Clear_Spread();
    }
}


void TiberiumClassExt::_Initialize_Tiberium_Growth_System()
{
    for (int i = 0; i < Tiberiums.Count(); i++) {
        Tiberiums[i]->Init_Growth();
    }
}


void TiberiumClassExt::_Deinitialize_Tiberium_Growth_System()
{
    for (int i = 0; i < Tiberiums.Count(); i++) {
        Tiberiums[i]->Clear_Growth();
    }
}


/**
 *  Replace the defective log (passes a TStringID instance as the argument) with a fixed and improved one.
 *
 *  @author: ZivDero
 */
EXPORT_FUNC(_Get_Tiberium_Type_Debug_Info_Patch)
{
    GET(OverlayTypeClass*, overlaytype, EAX);

    DEBUG_FATAL("Overlay %s [%d] is not really Tiberium!\nAll overlays with Tiberium=yes must be used by a Tiberium!\n", overlaytype->IniName.c_str(), overlaytype->HeapID);

    return 0x0058C951;
}


/**
 *  Randomize the overlay index when placing Tiberium within the Tiberium's variety,
 *  not from 0 to 11.
 *
 *  @author: ZivDero
 */
EXPORT_FUNC(_CellClass_Place_Tiberium_Variety_Patch)
{
    GET(TiberiumClass*, tiberium, EBP);

    int frame = Random_Pick(0, tiberium->Variety - 1);
    R->EAX(frame);

    return 0x0045CEC8;
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

    Patch_Jump(0x006455C0, &TiberiumClassExt::_Spread_AI);
    Patch_Jump(0x006458F0, &TiberiumClassExt::_Init_Spread);
    Patch_Jump(0x00645A30, &TiberiumClassExt::_Recalc_Spread);
    Patch_Jump(0x00645BA0, &TiberiumClassExt::_Clear_Spread);
    Patch_Jump(0x00645C70, &TiberiumClassExt::_Queue_Spread);
    Patch_Jump(0x00646080, &TiberiumClassExt::_Growth_AI);
    Patch_Jump(0x006463D0, &TiberiumClassExt::_Init_Growth);
    Patch_Jump(0x00646510, &TiberiumClassExt::_Recalc_Growth);
    Patch_Jump(0x00646680, &TiberiumClassExt::_Clear_Growth);
    Patch_Jump(0x00646710, &TiberiumClassExt::_Queue_Growth);
    Patch_Jump(0x006453C0, &TiberiumClassExt::_Initialize_Tiberium_Spread_System);
    Patch_Jump(0x00645510, &TiberiumClassExt::_Deinitialize_Tiberium_Spread_System);
    Patch_Jump(0x00645E80, &TiberiumClassExt::_Initialize_Tiberium_Growth_System);
    Patch_Jump(0x00645FD0, &TiberiumClassExt::_Deinitialize_Tiberium_Growth_System);
    Patch_Jump(0x00645C30, &TiberiumClassExt::_Clear_Spread_State);
}

declhook(0x0058C934, _Get_Tiberium_Type_Debug_Info_Patch, 0);
declhook(0x0045CEB9, _CellClass_Place_Tiberium_Variety_Patch, 0);
