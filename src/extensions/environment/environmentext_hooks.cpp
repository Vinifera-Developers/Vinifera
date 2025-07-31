/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ENVIRONMENTEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended EnvironmentClass.
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
#include "environmentext_hooks.h"
#include "tibsun_globals.h"
#include "environment.h"
#include "scenario.h"
#include "theme.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-46
 * 
 *  Fixes bug where the game difficulty gets reset, but not reassigned
 *  after restarting a mission.
 * 
 *  This also handles the case where the Environment instance is re-initialised.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_EnvironmentClass_Constructor_Set_Difficulty_Patch)
{
    GET_REGISTER_STATIC(EnvironmentClass *, this_ptr, edx);

    /**
     *  The EnvironmentClass constructor initialises Difficulty to NORMAL.
     *  This patch uses the ScenarioClass Difficulty if set at this point.
     */
    if (Scen && Scen->Difficulty != -1) {
        this_ptr->Difficulty = Scen->Difficulty;
    }

    //DEBUG_INFO("EnvironmentClass constructor.\n");

    /**
     *  Stolen bytes/code.
     */
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { ret }
}

DECLARE_PATCH(_Select_Game_Set_EnvironmentClass_Difficulty_Patch)
{
    DEBUG_INFO("Scen->Difficulty = %d\n", Scen->Difficulty);
    DEBUG_INFO("Scen->CDifficulty = %d\n", Scen->CDifficulty);

    /**
     *  Assign the ScenarioClass Difficulty. This is done to ensure
     *  the difficulty is restored after game restart.
     */
    DEBUG_INFO("Setting Environment difficulty to %d.\n", Scen->Difficulty);
    Environment.Difficulty = Scen->Difficulty;

    /**
     *  Stolen bytes/code.
     */
    Theme.Stop(true);

    JMP(0x004E2AE3);
}

DECLARE_PATCH(_EnvironmentClass_Record_Debug_Patch)
{
    GET_REGISTER_STATIC(EnvironmentClass *, this_ptr, esi);

    /**
     *  Stolen bytes/code.
     */
    this_ptr->Stage = Scen->Stage;

    DEBUG_INFO("Recording end game information...\n");
    DEBUG_INFO("  CarryOverMoney: %d\n", this_ptr->CarryOverMoney);
    DEBUG_INFO("  MissionTimer: %d\n", this_ptr->MissionTimer);
    DEBUG_INFO("  Difficulty: %d\n", this_ptr->Difficulty);
    DEBUG_INFO("  Stage: %d\n", this_ptr->Stage);

    /**
     *  Stolen bytes/code.
     */
    _asm { pop esi }
    _asm { ret }
}

DECLARE_PATCH(_EnvironmentClass_Apply_Debug_Patch)
{
    GET_REGISTER_STATIC(EnvironmentClass *, this_ptr, edi);

    DEBUG_INFO("Applying end game information...\n");
    DEBUG_INFO("  CarryOverMoney: %d\n", this_ptr->CarryOverMoney);
    DEBUG_INFO("  MissionTimer: %d\n", this_ptr->MissionTimer);
    DEBUG_INFO("  Difficulty: %d\n", this_ptr->Difficulty);
    DEBUG_INFO("  Stage: %d\n", this_ptr->Stage);

    /**
     *  Stolen bytes/code.
     */
    Scen->Stage = this_ptr->Stage;

    _asm { pop edi }
    _asm { pop esi }
    _asm { add esp, 0x0C }
    _asm { ret }
}


/**
 *  Main function for patching the hooks.
 */
void EnvironmentExtension_Hooks()
{
    Patch_Jump(0x00493881, &_EnvironmentClass_Constructor_Set_Difficulty_Patch);
    Patch_Jump(0x004E2AD7, &_Select_Game_Set_EnvironmentClass_Difficulty_Patch);

    /**
     *  Patches to log the current state of EnvironmentClass.
     */
    Patch_Jump(0x00493919, &_EnvironmentClass_Record_Debug_Patch);
    Patch_Jump(0x004939F1, &_EnvironmentClass_Apply_Debug_Patch);
    Patch_Jump(0x00493A07, 0x004939F1);
    Patch_Jump(0x00493A18, 0x004939F1);
}
