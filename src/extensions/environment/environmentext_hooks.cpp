/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ENVIRONMENTEXT_HOOKS.CPP
 *
 *  @author        CCHyper, ZivDero
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
#include "extension_globals.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "scenarioext.h"
#include "vinifera_globals.h"


class EnvironmentClassExt final : public EnvironmentClass
{
public:
    void _Snapshot_Game_State();
    void _Apply_To_Game_State();
};

/**
 *  Re-implementation of EnvironmentClass::Snapshot_Game_State.
 *
 *  @author: tomsons26, ZivDero
 */
void EnvironmentClassExt::_Snapshot_Game_State()
{
    for (int i = 0; i < std::size(EnvironmentGlobals); i++) {
        EnvironmentGlobals[i] = ScenExtension->GlobalFlags[i].Value;
    }

    CarryOverMoney = PlayerPtr->Available_Money();
    MissionTimer = Scen->MissionTimer;
    Difficulty = PlayerPtr->Difficulty;
    Stage = Scen->Stage;

    DEBUG_INFO("Recording environment information...\n");
    DEBUG_INFO("  Credits: %d\n", CarryOverMoney);
    DEBUG_INFO("  MissionTimer: %d\n", MissionTimer);
    DEBUG_INFO("  Difficulty: %d\n", Difficulty);
    DEBUG_INFO("  Stage: %d\n", Stage);
}


/**
 *  Re-implementation of EnvironmentClass::Apply_To_Game_State.
 *
 *  @author: tomsons26, ZivDero
 */
void EnvironmentClassExt::_Apply_To_Game_State()
{
    for (int i = 0; i < std::size(EnvironmentGlobals); i++) {
        ScenExtension->Set_Global_To(i, EnvironmentGlobals[i]);
    }

    int cap = Scen->CarryOverCap;
    double money = CarryOverMoney * Scen->CarryOverPercent;

    if (cap != -1) {
        money = std::min(money, static_cast<double>(cap));
    }

    PlayerPtr->Refund_Money(money);
    PlayerPtr->Control.InitialCredits += money;
    PlayerPtr->Assign_Handicap(static_cast<DiffType>(Difficulty));

    if (Scen->IsInheritTimer) {
        if (MissionTimer > 0) {
            Scen->MissionTimer = MissionTimer;
            Scen->MissionTimer.Start();
        }
    }

    Scen->Stage = Stage;

    DEBUG_INFO("Applying environment information...\n");
    DEBUG_INFO("  Credits: %d\n", CarryOverMoney);
    DEBUG_INFO("  MissionTimer: %d\n", MissionTimer);
    DEBUG_INFO("  Difficulty: %d\n", Difficulty);
    DEBUG_INFO("  Stage: %d\n", Stage);
}


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
DECLARE_PATCH(_EnvironmentClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(EnvironmentClass *, this_ptr, edx);

    /**
     *  The EnvironmentClass constructor initialises Difficulty to NORMAL.
     *  This patch uses the ScenarioClass Difficulty if set at this point.
     */
    if (Scen && Scen->Difficulty != -1) {
        this_ptr->Difficulty = Scen->Difficulty;
    }

    /**
     *  Initialize the new globals array.
     */
    static int i;
    for (i = 0; i < 50; i++) {
        EnvironmentGlobals[i] = 0;
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


/**
 *  Main function for patching the hooks.
 */
void EnvironmentExtension_Hooks()
{
    Patch_Jump(0x00493881, &_EnvironmentClass_Constructor_Patch);
    Patch_Jump(0x004E2AD7, &_Select_Game_Set_EnvironmentClass_Difficulty_Patch);
    Patch_Jump(0x004938A0, &EnvironmentClassExt::_Snapshot_Game_State);
    Patch_Jump(0x00493920, &EnvironmentClassExt::_Apply_To_Game_State);
}
