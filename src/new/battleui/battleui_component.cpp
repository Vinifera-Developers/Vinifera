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


BattleUISystem BattleUI;


BattleUISystem::~BattleUISystem()
{
    Shutdown();
}


void BattleUISystem::One_Time()
{
    Sidebar.One_Time();
    Power.Set_Sidebar_View_Type(Sidebar.Get_View_Type());
    Power.One_Time();
}


void BattleUISystem::Init_Clear()
{
    Sidebar.Init_Clear();
    Power.Init_Clear();
}


void BattleUISystem::Init_IO()
{
    Sidebar.Init_IO();
    Power.Init_IO();
}


void BattleUISystem::Init_For_House()
{
    Sidebar.Init_For_House();
    Power.Init_For_House();
}


void BattleUISystem::AI(KeyNumType &key, Point2D &mouse)
{
    Sidebar.AI(key, mouse);
    Power.AI(key, mouse);
}


void BattleUISystem::Draw()
{
    Sidebar.Draw();
    Power.Draw();
}


void BattleUISystem::Blit(bool complete)
{
    Sidebar.Blit(complete);
    Power.Blit(complete);
}


void BattleUISystem::Set_Dimensions()
{
    Sidebar.Set_Dimensions();
    Power.Set_Visible_Buttons_Per_Column(Sidebar.Visible_Buttons_Per_Column());
    Power.Set_Dimensions();
}


void BattleUISystem::Shutdown()
{
    Sidebar.Shutdown();
    Power.Shutdown();
}


const char *BattleUISystem::Help_Text(int gadget_id)
{
    const char *text;

    text = Sidebar.Help_Text(gadget_id);
    if (text != nullptr) return text;

    text = Power.Help_Text(gadget_id);
    if (text != nullptr) return text;

    return nullptr;
}


HRESULT BattleUISystem::Save(IStream *pStm) const
{
    HRESULT hr;

    hr = Sidebar.Save(pStm);
    if (FAILED(hr)) return hr;

    hr = Power.Save(pStm);
    if (FAILED(hr)) return hr;

    return S_OK;
}


HRESULT BattleUISystem::Load(IStream *pStm)
{
    HRESULT hr;

    hr = Sidebar.Load(pStm);
    if (FAILED(hr)) return hr;

    hr = Power.Load(pStm);
    if (FAILED(hr)) return hr;

    return S_OK;
}
