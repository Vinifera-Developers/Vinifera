/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ACTION_BAR_COMPONENT.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Action bar component implementation.
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

#include "action_bar_component.h"

#include "battleui_component.h"
#include "sidebar_view.h"


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
ActionBarComponent::ActionBarComponent()
{
}


/**
 *  One-time initialization. Registers with BattleUI.
 *
 *  @author: ZivDero
 */
void ActionBarComponent::One_Time()
{
}


void ActionBarComponent::Init_Clear()
{
}


void ActionBarComponent::Init_IO()
{
}


void ActionBarComponent::Init_For_House()
{
}


/**
 *  Per-frame update. Delegates action bar logic to the active view.
 *
 *  @author: ZivDero
 */
void ActionBarComponent::AI(KeyNumType& key, Point2D& mouse)
{
    ISidebarView* view = BattleUI.Get_Sidebar().Get_View();
    if (view == nullptr) {
        return;
    }

    view->Action_Bar_AI(key);
}


void ActionBarComponent::Draw(bool complete)
{
}


void ActionBarComponent::Blit(bool complete)
{
}


void ActionBarComponent::Set_Dimensions()
{
}


void ActionBarComponent::Shutdown()
{
}


const char* ActionBarComponent::Help_Text(int gadget_id)
{
    return nullptr;
}
