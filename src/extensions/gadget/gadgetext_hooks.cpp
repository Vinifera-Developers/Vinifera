/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended GadgetClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "gadgetext_hooks.h"

#include "gadget.h"
#include "ihoverable_gadget.h"
#include "syringe.h"


/**
 *  Patch in GadgetClass::Input to handle hover effects for IHoverableGadget.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004A9F0F, _GadgetClass_Input_Mouse_Enter_Leave, 0)
{
    GET(int, key, EAX);
    GET(int, mousex, EBP);
    GET(int, mousey, EBX);
    GET(GadgetClass*, this_ptr, ESI);

    GadgetClass* to_enter = this_ptr->Extract_Gadget_At_Mouse(mousex, mousey);
    IHoverableGadget::Handle_Mouse_Moved(to_enter);

    // Stolen code

    /**
     *  Set the mouse button state flags. These will be passed to the individual
     *  buttons so that they can determine what action to perform (if any).
     */
    unsigned flags = 0;
    if (key) {
        if (key == KN_LMOUSE) {
            flags |= GadgetClass::LEFTPRESS;
        }

        if (key == KN_RMOUSE) {
            flags |= GadgetClass::RIGHTPRESS;
        }

        if (key == (KN_LMOUSE | KN_RLSE_BIT)) {
            flags |= GadgetClass::LEFTRELEASE;
        }

        if (key == (KN_RMOUSE | KN_RLSE_BIT)) {
            flags |= GadgetClass::RIGHTRELEASE;
        }

        /**
         *  If the mouse wasn't responsible for this key code, then it must be from
         *  the keyboard. Flag this fact.
         */
        if (!flags) {
            flags |= GadgetClass::KEYBOARD;
        }

        R->EDI(flags);
        return 0x004A9F7F;
    }

    R->EDI(flags);
    return 0x004A9F4D;
}


/**
 *  Main function for patching the hooks.
 */
void GadgetClassExtension_Hooks()
{
}
