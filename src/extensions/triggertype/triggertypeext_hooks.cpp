/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TRIGGERTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TriggerTypeClass.
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
#include "triggertypeext_hooks.h"
#include "triggertype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-299
 * 
 *  Fixes the issue with the difficulty flags not being loaded correctly. The
 *  original code only set these values if they were "true", but they are already
 *  initialised to that in the TriggerTypeClass constructor...
 * 
 *  @see: TriggerClass and TActionClass for the other parts of this fix.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TriggerTypeClass_Read_INI_Load_Difficulty_Patch)
{
    GET_REGISTER_STATIC(TriggerTypeClass *, this_ptr, ebp);
    static char *tok;

    tok = std::strtok(nullptr, ",");
    this_ptr->IsEnabledEasy = tok && std::atoi(tok);

    tok = std::strtok(nullptr, ",");
    this_ptr->IsEnabledMedium = tok && std::atoi(tok);

    tok = std::strtok(nullptr, ",");
    this_ptr->IsEnabledHard = tok && std::atoi(tok);

    _asm { xor ebx, ebx } // Restore EBX state.
    JMP(0x0064A337);
}


/**
 *  Main function for patching the hooks.
 */
void TriggerTypeClassExtension_Hooks()
{
    //Patch_Jump(0x0064A2CE, &_TriggerTypeClass_Read_INI_Load_Difficulty_Patch);

    /**
     *  #issue-299
     *
     *  "_TriggerTypeClass_Read_INI_Load_Difficulty_Patch" done manually due
     *  to issues with jumping out of the game binary into the DLL, resulting
     *  in memory heap issues with strtok.
     */
    Patch_Byte_Range(0x0064A2DC, 0x90, 4); // 4 nops
    Patch_Byte_Range(0x0064A2E9, 0x90, 4); // 4 nops
    Patch_Dword(0x0064A2ED, 0x90654588); // mov [ebp+65h], al; nop;

    Patch_Byte_Range(0x0064A2FF, 0x90, 4); // 4 nops
    Patch_Byte_Range(0x0064A30C, 0x90, 4); // 4 nops
    Patch_Dword(0x0064A310, 0x90664588); // mov [ebp+66h], al; nop;

    Patch_Byte_Range(0x0064A322, 0x90, 4); // 4 nops
    Patch_Byte_Range(0x0064A32F, 0x90, 4); // 4 nops
    Patch_Dword(0x0064A333, 0x90674588); // mov [ebp+67h], al; nop;

    /**
     *  This patch skips the code for setting the enabled state of the
     *  trigger, we have moved this to the TriggerClass constructor now.
     */
    Patch_Jump(0x0064A35A, 0x0064A3A7);
}
