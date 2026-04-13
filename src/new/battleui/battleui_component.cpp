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


void BattleUISystem::Register(IBattleUIComponent *component)
{
    if (component && !Components.Is_Present(component)) {
        Components.Add(component);
    }
}


void BattleUISystem::Unregister(IBattleUIComponent *component)
{
    Components.Delete(component);
}


void BattleUISystem::One_Time()
{
    for (auto *component : Components) {
        component->One_Time();
    }
}


void BattleUISystem::Init_Clear()
{
    for (auto *component : Components) {
        component->Init_Clear();
    }
}


void BattleUISystem::Init_IO()
{
    for (auto *component : Components) {
        component->Init_IO();
    }
}


void BattleUISystem::Init_For_House()
{
    for (auto *component : Components) {
        component->Init_For_House();
    }
}


void BattleUISystem::AI(KeyNumType &key, Point2D &mouse)
{
    for (auto *component : Components) {
        component->AI(key, mouse);
    }
}


void BattleUISystem::Draw(bool complete)
{
    for (auto *component : Components) {
        component->Draw(complete);
    }
}


void BattleUISystem::Blit(bool complete)
{
    for (auto *component : Components) {
        component->Blit(complete);
    }
}


void BattleUISystem::Shutdown()
{
    for (auto *component : Components) {
        component->Shutdown();
    }

    Components.Clear();
}
