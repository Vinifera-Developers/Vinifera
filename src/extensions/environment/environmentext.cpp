/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the extended EnvironmentClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
 
#include "environmentext.h"

#include "debughandler.h"
#include "environment.h"
#include "extension_globals.h"
#include "hooker.h"
#include "house.h"
#include "scenario.h"
#include "scenarioext.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
 
/**
*  Re-implementation of EnvironmentClass::Snapshot_Game_State.
*
*  @author: tomsons26, ZivDero, Rampastring
*/
ExtEnvironmentClass::ExtEnvironmentClass() :
    CarryOverMoney(0),
    MissionTimer(0),
    Difficulty(DIFF_NORMAL),
    Stage(0)
{
    /**
     *  The EnvironmentClass constructor initialises Difficulty to NORMAL.
     *  It now uses the ScenarioClass Difficulty if set at this point.
     *  Fixes bug where the game difficulty gets reset, but not reassigned
     *  after restarting a mission.
     *
     *  @author: CCHyper
     */
    if (Scen) {
        Difficulty = Scen->Difficulty;
        CDifficulty = Scen->CDifficulty;
    }

    DEBUG_INFO("ExtEnvironmentClass CTOR: Initialized human difficulty to {} and computer difficulty to {}\n", (int)Difficulty, (int)CDifficulty);

    for (int i = 0; i < std::size(EnvironmentGlobals); i++) {
        EnvironmentGlobals[i] = 0;
    }
}


/**
 *  Re-implementation of EnvironmentClass::Snapshot_Game_State.
 *
 *  @author: tomsons26, ZivDero, Rampastring
 */
void ExtEnvironmentClass::Snapshot_Game_State()
{
    for (int i = 0; i < std::size(EnvironmentGlobals); i++) {
        EnvironmentGlobals[i] = ScenExtension->GlobalFlags[i].Value;
    }

    CarryOverMoney = PlayerPtr->Available_Money();
    MissionTimer = Scen->MissionTimer;

    // Bugfix: Record difficulty according to scenario difficulty instead of player house difficulty.
    // Also, record CDifficulty.
    Difficulty = Scen->Difficulty;
    CDifficulty = Scen->CDifficulty;
    Stage = Scen->Stage;

    DEBUG_INFO("Recording environment information...\n");
    DEBUG_INFO("  CarryOverMoney: {}\n", CarryOverMoney);
    DEBUG_INFO("  MissionTimer: {}\n", (int)MissionTimer);
    DEBUG_INFO("  Difficulty: {}\n", (int)Difficulty);
    DEBUG_INFO("  CDifficulty: {}\n", (int)CDifficulty);
    DEBUG_INFO("  Stage: {}\n", Stage);
}


/**
 *  Re-implementation of EnvironmentClass::Apply_To_Game_State.
 *
 *  @author: tomsons26, ZivDero
 */
void ExtEnvironmentClass::Apply_To_Game_State()
{
    Apply_Globals();

    int cap = Scen->CarryOverCap;
    double money = CarryOverMoney * Scen->CarryOverPercent;

    if (cap != -1) {
        money = std::min(money, static_cast<double>(cap));
    }

    PlayerPtr->Refund_Money(money);
    PlayerPtr->Control.InitialCredits += money;

    // Already assigned by HouseClass::Read_All through Read_Scenario_INI,
    // as long as Scen->Difficulty and Scen->CDifficulty have been set correctly.
    // PlayerPtr->Assign_Handicap(Difficulty);

    if (Scen->IsInheritTimer) {
        if (MissionTimer > 0) {
            Scen->MissionTimer = MissionTimer;
            Scen->MissionTimer.Start();
        }
    }

    Scen->Stage = Stage;

    DEBUG_INFO("Applying environment information...\n");
    DEBUG_INFO("  CarryOverMoney: {}\n", CarryOverMoney);
    DEBUG_INFO("  MissionTimer: {}\n", (int)MissionTimer);
    DEBUG_INFO("  Difficulty: {}\n", (int)Difficulty);
    DEBUG_INFO("  CDifficulty: {}\n", (int)CDifficulty);
    DEBUG_INFO("  Stage: {}\n", Stage);
}


/**
 *  Applies saved difficulty settings to the scenario.
 *
 *  @author: Rampastring
 */
void ExtEnvironmentClass::Apply_Difficulty() const
{
    DEBUG_INFO("Applying environment difficulty information...\n");
    DEBUG_INFO("  Difficulty: {}\n", (int)Difficulty);
    DEBUG_INFO("  CDifficulty: {}\n", (int)CDifficulty);

    if (Scen) {
        Scen->Difficulty = Difficulty;
        Scen->CDifficulty = CDifficulty;
    }
}


/**
 *  Applies saved global variable settings to the scenario.
 *
 *  @author: Rampastring
 */
void ExtEnvironmentClass::Apply_Globals()
{
    DEBUG_INFO("Applying environment global flag information...\n");
    for (int i = 0; i < std::size(EnvironmentGlobals); i++) {
        ScenExtension->Set_Global_To(i, EnvironmentGlobals[i]);
    }
}


/**
 *  Re-implementation of EnvironmentClass::Load.
 *
 *  @author: tomsons26, ZivDero
 */
HRESULT ExtEnvironmentClass::Load(IStream* stream)
{
    HRESULT hr = stream->Read(this, sizeof(*this), nullptr);
    if (FAILED(hr)) return hr;

    hr = stream->Read(&EnvironmentGlobals, sizeof(EnvironmentGlobals), nullptr);

    DEBUG_INFO("Loaded environment information...\n");
    DEBUG_INFO("  CarryOverMoney: {}\n", CarryOverMoney);
    DEBUG_INFO("  MissionTimer: {}\n", (int)MissionTimer);
    DEBUG_INFO("  Difficulty: {}\n", (int)Difficulty);
    DEBUG_INFO("  CDifficulty: {}\n", (int)CDifficulty);
    DEBUG_INFO("  Stage: {}\n", Stage);

    return hr;
}


/**
 *  Re-implementation of EnvironmentClass::Save.
 *
 *  @author: tomsons26, ZivDero
 */
HRESULT ExtEnvironmentClass::Save(IStream* stream)
{
    HRESULT hr = stream->Write(this, sizeof(*this), nullptr);
    if (FAILED(hr)) return hr;

    hr = stream->Write(&EnvironmentGlobals, sizeof(EnvironmentGlobals), nullptr);
    return hr;
}


/**
 *  Replacement static initializer for Environment.
 *
 *  @author: ZivDero
 */
int __cdecl ExtEnvironmentClass::Static_Init()
{
    new (reinterpret_cast<ExtEnvironmentClass*>(&Environment)) ExtEnvironmentClass;
    return atexit(Static_Deinit);
};


/**
 *  Replacement static de-initializer for Environment.
 *
 *  @author: ZivDero
 */
void __cdecl ExtEnvironmentClass::Static_Deinit()
{
    reinterpret_cast<ExtEnvironmentClass*>(&Environment)->~ExtEnvironmentClass();
};