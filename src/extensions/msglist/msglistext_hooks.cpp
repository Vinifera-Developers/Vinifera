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
#include "vinifera_util.h"
#include "tibsun_globals.h"
#include "session.h"
#include "msglist.h"
#include "command.h"
#include "house.h"
#include "housetype.h"
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
 *  Related to #issue-525
 * 
 *  If the player reassigns the chat-to-all command to a ascii key, it will
 *  end up repeating into the edit buffer. This patch makes sure this does
 *  not happen in all cases (hopefully).
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MessageListClass_Skip_Assigned_Key_Echo_Patch)
{
    GET_REGISTER_STATIC(MessageListClass *, this_ptr, esi);
    GET_REGISTER_STATIC(KeyNumType *, input, ebp);
    GET_REGISTER_STATIC(KeyASCIIType, ascii, eax);
    static CommandClass *cmd;
    static KeyNumType cmd_key;
    static KeyASCIIType cmd_key_ascii;

    /**
     *  Mask off any irrelevant bits.
     */
    ascii = KeyASCIIType(ascii & 0x00FF);

    /**
     *  Is this a simple work-around/lock for handling the echo of the
     *  hotkey being passed into the message buffer. 
     */
    static bool _initial_flag = false;
    if (!_initial_flag) {

        /**
         *  Make sure the buffer is empty of any user input, future instances
         *  of this key could be legitimate input we need to process.
         */
        if (this_ptr->EditInitPos == this_ptr->EditCurPos) {

            cmd_key = Get_Command_Key_From_Name("ChatToAll");
            cmd_key_ascii = KeyASCIIType(WWKeyboard->To_ASCII(cmd_key));

            _initial_flag = true;

            /**
             *  Invalidate the input character if it matches the trigger key.
             */
            if (cmd_key_ascii > KA_SPACE && cmd_key_ascii == ascii) {
                ascii = KA_NONE;
            }

        }

    }

    /**
     *  Check if the user has entered any text, and reset the lock flag.
     */
    if (this_ptr->EditBuf[this_ptr->EditInitPos] != '\0') {
        _initial_flag = false;
    }

    /**
     *  Stolen bytes/code.
     */
    _asm { mov ebx, ascii }

    _asm { mov eax, [input+0] } // Restore EAX just to be sure.
    _asm { mov eax, [eax] }

    JMP_REG(ecx, 0x00573B0A);
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
 *  Main function for patching the hooks.
 */
void MessageListClassExtension_Hooks()
{
    Patch_Jump(0x00509D16, &_MessageListClass_Echo_Sent_Messages_Patch);
    Patch_Jump(0x00573B05, &_MessageListClass_Skip_Assigned_Key_Echo_Patch);
}
