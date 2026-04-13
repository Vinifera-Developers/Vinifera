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

#include "point.h"
#include "vector.h"
#include "wwkeyboard.h"


/**
 *  Lifecycle interface for battle UI components (sidebar, radar, etc.).
 */
class IBattleUIComponent
{
public:
    virtual ~IBattleUIComponent() = default;

    virtual void One_Time() = 0;
    virtual void Init_Clear() = 0;
    virtual void Init_IO() = 0;
    virtual void Init_For_House() = 0;
    virtual void AI(KeyNumType &key, Point2D &mouse) = 0;
    virtual void Draw(bool complete) = 0;
    virtual void Blit(bool complete) = 0;
    virtual void Shutdown() = 0;
};


/**
 *  Top-level system that owns and drives all battle UI components.
 */
class BattleUISystem
{
public:
    BattleUISystem() = default;
    ~BattleUISystem();

    void Register(IBattleUIComponent *component);
    void Unregister(IBattleUIComponent *component);

    void One_Time();
    void Init_Clear();
    void Init_IO();
    void Init_For_House();
    void AI(KeyNumType &key, Point2D &mouse);
    void Draw(bool complete);
    void Blit(bool complete);
    void Shutdown();

private:
    DynamicVectorClass<IBattleUIComponent *> Components;
};


extern BattleUISystem BattleUI;
