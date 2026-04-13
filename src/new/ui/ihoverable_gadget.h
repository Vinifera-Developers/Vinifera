/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          IHOVERABLE_GADGET.H
 *
 *  @author        ZivDero
 *
 *  @brief         Interface for gadgets that respond to mouse hover events.
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

#include "gadget.h"


/**
 *  Interface for gadgets that respond to mouse enter/leave hover events.
 */
class IHoverableGadget
{
public:
    virtual ~IHoverableGadget() = default;

    virtual void On_Mouse_Enter() = 0;
    virtual void On_Mouse_Leave() = 0;

    /**
     *  Called each frame with the gadget currently under the mouse cursor
     *  (or nullptr when no gadget is hovered). Fires On_Mouse_Enter and
     *  On_Mouse_Leave on the appropriate objects and keeps track of which
     *  gadget was last hovered.
     */
    static void Handle_Mouse_Moved(GadgetClass* candidate)
    {
        IHoverableGadget* to_enter = dynamic_cast<IHoverableGadget*>(candidate);
        if (to_enter == LastHovered) {
            return;
        }

        if (LastHovered) {
            LastHovered->On_Mouse_Leave();
            LastHovered = nullptr;
        }

        if (to_enter) {
            LastHovered = to_enter;
            to_enter->On_Mouse_Enter();
        }
    }

private:
    inline static IHoverableGadget* LastHovered = nullptr;
};
