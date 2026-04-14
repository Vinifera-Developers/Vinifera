/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          POWER_COMPONENT.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Power bar component implementation.
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

#include "power_component.h"



/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
PowerComponent::PowerComponent()
{
    View.Set_Model(&Model);
}


/**
 *  One-time initialization. Called once at game startup.
 *
 *  @author: ZivDero
 */
void PowerComponent::One_Time()
{
    View.One_Time();
}


/**
 *  Clears all state for a new scenario.
 *
 *  @author: ZivDero
 */
void PowerComponent::Init_Clear()
{
    Model.Init_Clear();
    View.Init_Clear();
}


/**
 *  Initializes IO gadgets.
 *
 *  @author: ZivDero
 */
void PowerComponent::Init_IO()
{
}


/**
 *  Initializes for the current player house.
 *
 *  @author: ZivDero
 */
void PowerComponent::Init_For_House()
{
    View.Init_For_House();
}


/**
 *  Per-frame update. Updates the model and view.
 *
 *  @author: ZivDero
 */
void PowerComponent::AI(KeyNumType &key, Point2D &mouse)
{
    Model.AI();
    View.AI();
}


/**
 *  Renders the power bar.
 *
 *  @author: ZivDero
 */
void PowerComponent::Draw()
{
    View.Draw();
}


/**
 *  Blits the power bar to the screen.
 *
 *  @author: ZivDero
 */
void PowerComponent::Blit(bool complete)
{
}


/**
 *  Reflows the power bar layout.
 *
 *  @author: ZivDero
 */
void PowerComponent::Shift_Sidebar()
{
    View.Shift_Sidebar();
}


void PowerComponent::Set_Visible_Buttons_Per_Column(int count)
{
    View.Set_Visible_Buttons_Per_Column(count);
}


void PowerComponent::Set_Sidebar_View_Type(SidebarViewType view_type)
{
    View.Set_Sidebar_View_Type(view_type);
}


/**
 *  Shuts down the power component.
 *
 *  @author: ZivDero
 */
void PowerComponent::Shutdown()
{
}


/**
 *  Returns help text for the power bar tooltip.
 *
 *  @author: ZivDero
 */
const char *PowerComponent::Help_Text(int gadget_id)
{
    return View.Help_Text(gadget_id);
}


/**
 *  Triggers the power bar flash effect.
 *
 *  @author: ZivDero
 */
void PowerComponent::Flash_Power()
{
    View.Flash_Power();
}
