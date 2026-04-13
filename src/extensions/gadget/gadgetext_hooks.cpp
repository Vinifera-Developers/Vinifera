/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          GADGETEXT_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the extended GadgetClass.
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

#include "gadgetext_hooks.h"

#include "gadget.h"
#include "hooker_macros.h"
#include "ihoverable_gadget.h"
#include "keyboard.h"


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
