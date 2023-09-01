/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MSGLISTEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended message input function.
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
#include "msglistext_hooks.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "session.h"
#include "msglist.h"
#include "house.h"
#include "housetype.h"
#include "uicontrol.h"
#include "rules.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


static int Get_Message_Delay()
{
    return Rule->MessageDelay * TICKS_PER_MINUTE;
}


/**
 *  #issue-37
 * 
 *  Echo the users sent messages back to them (as a confirmation that it was sent).
 * 
 *  @author: CCHyper (based on research by Iran, back ported from Red Alert 2)
 */
DECLARE_PATCH(_MessageListClass_Echo_Sent_Messages_Patch)
{
    GET_REGISTER_STATIC(MessageListClass *, this_ptr, esi);
    GET_ADDRESS_STATIC(char *, Session_GPacket_Message_Buf, 0x007E3AD0);
    static char echobuff[MAX_MESSAGE_LENGTH]; 

    /**
     *  Original code:
     * 
     *  Store this message in our LastMessage buffer.
     */
    std::strcpy(Session.LastMessage, Session_GPacket_Message_Buf);

    /**
     *  Echo the last sent message back to the user.
     */
    std::snprintf(echobuff, sizeof(echobuff), TEXT_S_S, PlayerPtr->IniName, Session.LastMessage);
  	Session.Messages.Add_Message(
        nullptr, 0, echobuff, PlayerPtr->RemapColor,
        TPF_6PT_GRAD|TPF_USE_GRAD_PAL|TPF_FULLSHADOW,
        Get_Message_Delay());

    /**
     *  Flag screen to redraw.
     */
    JMP_REG(ecx, 0x00509D36);
}


/**
 *  Shrinks the width of the message list to accommodate for its moved position.
 *
 *  Author: Rampastring
 */
DECLARE_PATCH(_MessageListClass_Init_Modify_Width_Patch)
{
    GET_REGISTER_STATIC(MessageListClass *, this_ptr, esi);
    GET_REGISTER_STATIC(int, width, eax);
    static int posx;

    posx = 0;
    if (UIControls != nullptr) {
        posx = UIControls->MessageListPositionX;
    }

    width -= posx;

    DEBUG_INFO("MessageListClass::Init(Width: %d)", width);
    this_ptr->Width = width;

    _asm { xor ebx, ebx }
    _asm { mov edi, [esi] }
    JMP(0x00572EC4);
}


/**
 *  Main function for patching the hooks.
 */
void MessageListClassExtension_Hooks()
{
    Patch_Jump(0x00509D16, &_MessageListClass_Echo_Sent_Messages_Patch);
    Patch_Jump(0x00572EAC, &_MessageListClass_Init_Modify_Width_Patch);
}
