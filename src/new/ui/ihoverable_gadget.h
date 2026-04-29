/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Interface for gadgets that respond to mouse hover events.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
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
