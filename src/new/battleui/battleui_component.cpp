/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BATTLEUI_COMPONENT.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Battle UI component framework implementation.
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

#include "battleui_component.h"

#include "tibsun_globals.h"
#include "tooltip.h"


BattleUISystem BattleUI;


/***************************************************************************
**  Lifecycle and orchestration
***************************************************************************/


/**
 *  Destroys the battle UI system and releases owned components.
 *
 *  @author: ZivDero
 */
BattleUISystem::~BattleUISystem()
{
    Shutdown();
}


/**
 *  Performs one-time startup for all battle UI components.
 *
 *  @author: ZivDero
 */
void BattleUISystem::One_Time()
{
    Sidebar.One_Time();
    Power.Set_Sidebar_View_Type(Sidebar.Get_View_Type());
    Power.One_Time();
}


/**
 *  Resets all battle UI components for a new scenario.
 *
 *  @author: ZivDero
 */
void BattleUISystem::Init_Clear()
{
    Sidebar.Init_Clear();
    Power.Init_Clear();
}


/**
 *  Initializes battle UI IO gadgets.
 *
 *  @author: ZivDero
 */
void BattleUISystem::Init_IO()
{
    Sidebar.Init_IO();
    Power.Init_IO();
}


/**
 *  Loads house-specific battle UI resources.
 *
 *  @author: ZivDero
 */
void BattleUISystem::Init_For_House()
{
    Sidebar.Init_For_House();
    Power.Init_For_House();
}


/**
 *  Advances all battle UI components for the current frame.
 *
 *  @author: ZivDero
 */
void BattleUISystem::AI(KeyNumType &key, Point2D &mouse)
{
    Sidebar.AI(key, mouse);
    Power.AI(key, mouse);
}


/**
 *  Draws all battle UI components.
 *
 *  @author: ZivDero
 */
void BattleUISystem::Draw()
{
    Sidebar.Draw();
    Power.Draw();

    if (ToolTips != nullptr) {
        ToolTips->Force_Redraw(true);
    }
}


/**
 *  Blits all battle UI components to the screen.
 *
 *  @author: ZivDero
 */
void BattleUISystem::Blit(bool complete)
{
    Sidebar.Blit(complete);
    Power.Blit(complete);
}


/**
 *  Reflows sidebar-driven battle UI layout.
 *
 *  @author: ZivDero
 */
void BattleUISystem::Shift_Sidebar()
{
    Sidebar.Shift_Sidebar();
    Power.Set_Visible_Buttons_Per_Column(Sidebar.Visible_Buttons_Per_Column());
    Power.Shift_Sidebar();
}


/**
 *  Shuts down all battle UI components.
 *
 *  @author: ZivDero
 */
void BattleUISystem::Shutdown()
{
    Sidebar.Shutdown();
    Power.Shutdown();
}


/***************************************************************************
**  Queries and persistence
***************************************************************************/


/**
 *  Returns help text for the specified battle UI gadget, if any.
 *
 *  @author: ZivDero
 */
const char *BattleUISystem::Help_Text(int gadget_id)
{
    const char *text;

    text = Sidebar.Help_Text(gadget_id);
    if (text != nullptr) return text;

    text = Power.Help_Text(gadget_id);
    if (text != nullptr) return text;

    return nullptr;
}


/**
 *  Saves all battle UI component state to the provided stream.
 *
 *  @author: ZivDero
 */
HRESULT BattleUISystem::Save(IStream *pStm) const
{
    HRESULT hr;

    hr = Sidebar.Save(pStm);
    if (FAILED(hr)) return hr;

    hr = Power.Save(pStm);
    if (FAILED(hr)) return hr;

    return S_OK;
}


/**
 *  Loads all battle UI component state from the provided stream.
 *
 *  @author: ZivDero
 */
HRESULT BattleUISystem::Load(IStream *pStm)
{
    HRESULT hr;

    hr = Sidebar.Load(pStm);
    if (FAILED(hr)) return hr;

    hr = Power.Load(pStm);
    if (FAILED(hr)) return hr;

    return S_OK;
}
