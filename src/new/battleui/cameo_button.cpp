/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CAMEO_BUTTON.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Button class for sidebar cameo slots.
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

#include "cameo_button.h"

#include "battleui_component.h"
#include "sidebar_strip_view.h"


CameoButtonClass::CameoButtonClass() :
    ControlClass(0, 0, 0, SidebarStripView::OBJECT_WIDTH - 1, SidebarStripView::OBJECT_HEIGHT,
                 LEFTPRESS | RIGHTPRESS | LEFTUP),
    Strip(nullptr),
    Index(0),
    MousedOver(false)
{
}


CameoButtonClass::CameoButtonClass(const NoInitClass& x) :
    ControlClass(x),
    Strip(nullptr),
    Index(0),
    MousedOver(false)
{
}


/**
 *  Handles button input for cameo clicks: left-click to produce/place,
 *  right-click to suspend/cancel.
 *
 *  @author: ZivDero
 */
bool CameoButtonClass::Action(unsigned flags, KeyNumType& key)
{
    if (!Strip) {
        return true;
    }

    BattleUI.Get_Sidebar().Handle_Cameo_Action(*Strip, Index, flags);
    ControlClass::Action(flags, key);
    return true;
}


void CameoButtonClass::On_Mouse_Enter()
{
    MousedOver = true;
}


void CameoButtonClass::On_Mouse_Leave()
{
    MousedOver = false;
}


void CameoButtonClass::Set_Owner(SidebarStripView& strip, int index)
{
    Strip = &strip;
    Index = index;
}
