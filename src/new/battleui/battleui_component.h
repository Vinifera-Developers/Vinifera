/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BATTLEUI_COMPONENT.H
 *
 *  @author        ZivDero
 *
 *  @brief         Battle UI component framework. Defines the lifecycle
 *                 interface for UI components and the system that drives them.
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

#include "power_component.h"
#include "sidebar_component.h"

#include <objidl.h>


/**
 *  Top-level system that owns and drives all battle UI components.
 */
class BattleUISystem
{
public:
    BattleUISystem() = default;
    ~BattleUISystem();

    void One_Time();
    void Init_Clear();
    void Init_IO();
    void Init_For_House();
    void AI(KeyNumType &key, Point2D &mouse);
    void Draw(bool complete);
    void Blit(bool complete);
    void Set_Dimensions();
    void Shutdown();

    const char *Help_Text(int gadget_id);

    HRESULT Save(IStream *pStm) const;
    HRESULT Load(IStream *pStm);

    SidebarComponent &Get_Sidebar() { return Sidebar; }
    const SidebarComponent &Get_Sidebar() const { return Sidebar; }
    PowerComponent &Get_Power() { return Power; }

private:
    SidebarComponent Sidebar;
    PowerComponent Power;
};


extern BattleUISystem BattleUI;
