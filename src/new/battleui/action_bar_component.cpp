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

#include "house.h"
#include "mouse.h"
#include "options.h"
#include "radar.h"
#include "sidebar.h"
#include "tibsun_globals.h"


ActionBarComponent ActionBar;


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
ActionBarComponent::ActionBarComponent() :
    IsActive(false)
{
}


/**
 *  One-time initialization. Registers with BattleUI.
 *
 *  @author: ZivDero
 */
void ActionBarComponent::One_Time()
{
    BattleUI.Register(this);
}


void ActionBarComponent::Init_Clear()
{
    IsActive = false;
}


void ActionBarComponent::Init_IO()
{
}


void ActionBarComponent::Init_For_House()
{
}


/**
 *  Per-frame update. Handles button clicks and synchronizes button
 *  visual state with the current mode flags.
 *
 *  @author: ZivDero
 */
void ActionBarComponent::AI(KeyNumType& key, Point2D& mouse)
{
    if (!IsActive) {
        return;
    }

    /**
     *  Enable or disable the repair button based on whether
     *  the player owns any buildings.
     */
    if (PlayerPtr->CurBuildings > 0) {
        Map.Activate_Repair(true);
    } else {
        Map.Activate_Repair(false);
    }

    /**
     *  Handle button clicks — toggle the corresponding mode.
     */
    if (key == (SidebarClass::BUTTON_REPAIR | KN_BUTTON)) {
        Map.Repair_Mode_Control(-1);
    }

    if (key == (SidebarClass::BUTTON_POWER | KN_BUTTON)) {
        Map.Power_Mode_Control(-1);
    }

    if (key == (SidebarClass::BUTTON_WAYPOINT | KN_BUTTON)) {
        Map.Waypoint_Mode_Control(-1, false);
    }

    if (key == (SidebarClass::BUTTON_SELL | KN_BUTTON)) {
        Map.Sell_Mode_Control(-1);
    }

    /**
     *  Synchronize button visual state with the mode flags.
     */
    if (!Map.IsRepairMode && SidebarClass::Repair.IsOn) {
        SidebarClass::Repair.Turn_Off();
    }

    if (!Map.IsSellMode && SidebarClass::Sell.IsOn) {
        SidebarClass::Sell.Turn_Off();
    }

    if (!Map.IsPowerMode && SidebarClass::Power.IsOn) {
        SidebarClass::Power.Turn_Off();
    }

    if (!Map.IsWaypointMode && SidebarClass::Waypoint.IsOn) {
        SidebarClass::Waypoint.Turn_Off();
    }
}


void ActionBarComponent::Draw(bool complete)
{
}


void ActionBarComponent::Blit(bool complete)
{
}


/**
 *  Positions the background gadget to fill the sidebar area
 *  below the radar panel.
 *
 *  @author: ZivDero
 */
void ActionBarComponent::Set_Dimensions()
{
    SidebarClass::Background.Set_Position(
        Options.SidebarSide ? TacticalRect.X + TacticalRect.Width : 0,
        RadarClass::RadarButton.Height + RadarClass::RadarButton.Y);
    SidebarClass::Background.Set_Size(
        SidebarSurface->Get_Width(),
        SidebarSurface->Get_Height() - RadarClass::RadarButton.Height + RadarClass::RadarButton.Y);
}


void ActionBarComponent::Shutdown()
{
    Deactivate();
}


const char* ActionBarComponent::Help_Text(int gadget_id)
{
    return nullptr;
}


/**
 *  Adds the action bar buttons to the input gadget chain.
 *
 *  @author: ZivDero
 */
void ActionBarComponent::Activate(int control)
{
    if (IsActive) {
        return;
    }

    SidebarClass::Repair.Zap();
    Map.Add_A_Button(SidebarClass::Repair);
    SidebarClass::Sell.Zap();
    Map.Add_A_Button(SidebarClass::Sell);
    SidebarClass::Power.Zap();
    Map.Add_A_Button(SidebarClass::Power);
    SidebarClass::Waypoint.Zap();
    Map.Add_A_Button(SidebarClass::Waypoint);
    SidebarClass::Background.Zap();
    Map.Add_A_Button(SidebarClass::Background);

    IsActive = true;
}


/**
 *  Removes the action bar buttons from the input gadget chain.
 *
 *  @author: ZivDero
 */
void ActionBarComponent::Deactivate()
{
    if (!IsActive) {
        return;
    }

    Map.Remove_A_Button(SidebarClass::Repair);
    Map.Remove_A_Button(SidebarClass::Sell);
    Map.Remove_A_Button(SidebarClass::Power);
    Map.Remove_A_Button(SidebarClass::Waypoint);
    Map.Remove_A_Button(SidebarClass::Background);

    IsActive = false;
}
