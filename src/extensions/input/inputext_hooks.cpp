/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INPUTEXT_HOOKS.CPP
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
#include "inputext_hooks.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "command.h"
#include "language.h"
#include "session.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  This patch allows custom keys to be checked at the entry
 *  point of Message_Input().
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Message_Input_Allow_Remap_Keys_Patch)
{
    GET_REGISTER_STATIC(KeyNumType *, input, ebp);
    static CommandClass *cmd;
    static KeyNumType cmd_key;
    static GameEnum session_type;
    static int session_max_players;

    /**
     *  New case to handle reassignment of the new chat-to-all command.
     */
    cmd_key = Get_Command_Key_From_Name("ChatToAll");
    if (cmd_key > KN_NONE && *input == cmd_key) {
        goto key_passes;
    }

    /**
     *	Default behaviour: Make sure the key is in the expected range.
     */
    if (*input < KN_F1) {
        goto key_failed_check;
    }

    /**
     *  Continue check for key range (F1 to F7).
     */
key_continue_check:
    session_type = Session.Type;
    _asm { mov eax, session_type }
    _asm { mov ecx, [input] }
    JMP_REG(edx, 0x005098FD);

    /**
     *  Key passes the check, continue.
     */
key_passes:
    session_type = Session.Type;
    session_max_players = Session.MaxPlayers;
    _asm { mov eax, session_type }
    _asm { mov edx, session_max_players }
    JMP_REG(ecx, 0x0050991B);

    /**
     *  Not an expected key.
     */
key_failed_check:
    JMP_REG(edx, 0x00509A6C);
}


/**
 *  #issue-525
 * 
 *  This patch allows the chat-to-all command to be remapped by the user
 *  by checking the key assigned to ChatToAllCommandClass. 
 * 
 *  @see: ChatToAllCommandClass
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Message_Input_Chat_To_All_Key_Patch)
{
    GET_REGISTER_STATIC(KeyNumType *, input, ebp);
    static CommandClass *cmd;
    static KeyNumType cmd_key;

    /**
     *  New case to handle reassignment of the new chat-to-all command.
     */
    cmd_key = Get_Command_Key_From_Name("ChatToAll");
    if (cmd_key > KN_NONE) {
        if (*input == cmd_key) {
            goto message_to_all;
        } else {
            goto process_next;
        }
    }

    /**
     *	Default value for a network/internet game is F8 = "To All:"
     */
    if (*input == (KN_F1 + Session.MaxPlayers - 1)) {
        goto message_to_all;
    }

    /**
     *  Next key range check (Next check is for specific player F1 to F7).
     */
process_next:
    JMP_REG(eax, 0x00509975);

    /**
     *  Enter the "send to all" routine.
     */
message_to_all:
    JMP_REG(ecx, 0x00509947);
}


/**
 *  Main function for patching the hooks.
 */
void InputExtension_Hooks()
{
    Patch_Jump(0x005098F1, &_Message_Input_Allow_Remap_Keys_Patch);
    Patch_Jump(0x00509940, &_Message_Input_Chat_To_All_Key_Patch);
}
