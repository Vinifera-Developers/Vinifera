/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_CONFIG.H
 *
 *  @author        ZivDero
 *
 *  @brief         Sidebar and battle UI configuration, loaded from UI.INI.
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

#pragma once


/**
 *  Sidebar layout type.
 */
enum SidebarViewType {
    SIDEBAR_CLASSIC,
    SIDEBAR_TABBED,

    SIDEBAR_COUNT
};


/**
 *  Configuration for the sidebar subsystem.
 */
struct SidebarConfig
{
    SidebarConfig() : ViewType(SIDEBAR_CLASSIC) {}

    SidebarViewType ViewType;
};


/**
 *  Top-level configuration container for the entire battle UI.
 *  Future components (radar, credits) add their config here.
 */
struct BattleUIConfig
{
    SidebarConfig Sidebar;
};


/**
 *  Global battle UI configuration instance.
 */
extern BattleUIConfig UIConfig;


/**
 *  Reads the battle UI configuration from UI.INI.
 */
bool Read_UI_INI();
