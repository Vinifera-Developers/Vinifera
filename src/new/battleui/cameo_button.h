/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CAMEO_BUTTON.H
 *
 *  @author        ZivDero
 *
 *  @brief         Button class for sidebar cameo slots.
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

#include "gcntrl.h"


class SidebarStripView;


class CameoButtonClass : public ControlClass
{
public:
    CameoButtonClass();
    CameoButtonClass(const NoInitClass& x);

    virtual bool Action(unsigned flags, KeyNumType& key) override;

    virtual void On_Mouse_Enter();
    virtual void On_Mouse_Leave();

    void Set_Owner(SidebarStripView& strip, int index);

public:
    SidebarStripView* Strip;
    int Index;
    bool MousedOver;
};
