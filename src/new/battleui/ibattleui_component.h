/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          IBATTLEUI_COMPONENT.H
 *
 *  @author        ZivDero
 *
 *  @brief         Lifecycle interface for battle UI components.
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
#include "wwkeyboard.h"

#include <objidl.h>


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
    virtual void Draw() = 0;
    virtual void Blit(bool complete) = 0;
    virtual void Set_Dimensions() = 0;
    virtual void Shutdown() = 0;

    virtual const char *Help_Text(int gadget_id) { return nullptr; }

    virtual HRESULT Save(IStream *pStm) const { return S_OK; }
    virtual HRESULT Load(IStream *pStm) { return S_OK; }
};
