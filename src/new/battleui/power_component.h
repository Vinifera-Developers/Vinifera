/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          POWER_COMPONENT.H
 *
 *  @author        ZivDero
 *
 *  @brief         Power bar component. Owns the power data model and view,
 *                 and implements the IBattleUIComponent lifecycle interface.
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

#include "battleui_component.h"
#include "power_model.h"
#include "power_view.h"


/**
 *  Power bar component. Owns the data model and view for the power bar
 *  and registers with BattleUISystem for lifecycle management.
 */
class PowerComponent : public IBattleUIComponent
{
public:
    PowerComponent();
    ~PowerComponent() = default;

    /**
     *  IBattleUIComponent
     */
    virtual void One_Time() override;
    virtual void Init_Clear() override;
    virtual void Init_IO() override;
    virtual void Init_For_House() override;
    virtual void AI(KeyNumType &key, Point2D &mouse) override;
    virtual void Draw(bool complete) override;
    virtual void Blit(bool complete) override;
    virtual void Set_Dimensions() override;
    virtual void Shutdown() override;

    virtual const char *Help_Text(int gadget_id) override;

    void Flash_Power();

    PowerModel &Get_Model() { return Model; }
    const PowerModel &Get_Model() const { return Model; }

private:
    PowerModel Model;
    PowerView View;
};


extern PowerComponent PowerBar;
