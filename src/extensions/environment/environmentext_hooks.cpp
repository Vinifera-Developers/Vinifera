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
#include "house.h"
#include "scenarioext.h"
#include "vinifera_globals.h"


/**
 *  Replacement class for EnvironmentClass.
 *
 *  @author: ZivDero, tomsons26
 */
class ExtEnvironmentClass
{
    friend void EnvironmentExtension_Hooks();

public:
    ExtEnvironmentClass();
    ~ExtEnvironmentClass() = default;

    void Snapshot_Game_State();
    void Apply_To_Game_State();

    HRESULT Load(IStream* stream);
    HRESULT Save(IStream* stream);

private:
    ExtEnvironmentClass* Hook_Ctor() { return new (this) ExtEnvironmentClass; }
    void Hook_Dtor() { this->~ExtEnvironmentClass(); }

    static int __cdecl Static_Init();
    static void __cdecl Static_Deinit();

private:
    char __Padding[50]; // Used to be Globals[50], available for re-use.

public:
    int CarryOverMoney;
    int MissionTimer;
    DiffType Difficulty;
    unsigned short Stage;
};


/**
 *  Since we're not making a new instance, ensure that the size is the same
 */
static_assert(sizeof(ExtEnvironmentClass) == sizeof(EnvironmentClass), "sizeof(ExtEnvironmentClass) != sizeof(EnvironmentClass)!");


/**
 *  Re-implementation of EnvironmentClass::Snapshot_Game_State.
 *
 *  @author: tomsons26, ZivDero
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
    if (Scen && Scen->Difficulty != -1) {
        Difficulty = Scen->Difficulty;
    }

    for (int i = 0; i < std::size(EnvironmentGlobals); i++) {
        EnvironmentGlobals[i] = 0;
    }
}


/**
 *  Re-implementation of EnvironmentClass::Snapshot_Game_State.
 *
 *  @author: tomsons26, ZivDero
 */
void ExtEnvironmentClass::Snapshot_Game_State()
{
    for (int i = 0; i < std::size(EnvironmentGlobals); i++) {
        EnvironmentGlobals[i] = ScenExtension->GlobalFlags[i].Value;
    }

    CarryOverMoney = PlayerPtr->Available_Money();
    MissionTimer = Scen->MissionTimer;
    Difficulty = PlayerPtr->Difficulty;
    Stage = Scen->Stage;

    DEBUG_INFO("Recording environment information...\n");
    DEBUG_INFO("  CarryOverMoney: %d\n", CarryOverMoney);
    DEBUG_INFO("  MissionTimer: %d\n", MissionTimer);
    DEBUG_INFO("  Difficulty: %d\n", Difficulty);
    DEBUG_INFO("  Stage: %d\n", Stage);
}


/**
 *  Re-implementation of EnvironmentClass::Apply_To_Game_State.
 *
 *  @author: tomsons26, ZivDero
 */
void ExtEnvironmentClass::Apply_To_Game_State()
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
    PlayerPtr->Assign_Handicap(Difficulty);

    if (Scen->IsInheritTimer) {
        if (MissionTimer > 0) {
            Scen->MissionTimer = MissionTimer;
            Scen->MissionTimer.Start();
        }
    }

    Scen->Stage = Stage;

    DEBUG_INFO("Applying environment information...\n");
    DEBUG_INFO("  CarryOverMoney: %d\n", CarryOverMoney);
    DEBUG_INFO("  MissionTimer: %d\n", MissionTimer);
    DEBUG_INFO("  Difficulty: %d\n", Difficulty);
    DEBUG_INFO("  Stage: %d\n", Stage);
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


/**
 *  Main function for patching the hooks.
 */
void EnvironmentExtension_Hooks()
{
    Patch_Jump(0x004938A0, &ExtEnvironmentClass::Snapshot_Game_State);
    Patch_Jump(0x00493920, &ExtEnvironmentClass::Apply_To_Game_State);
    Patch_Jump(0x00493A30, &ExtEnvironmentClass::Load);
    Patch_Jump(0x00493A50, &ExtEnvironmentClass::Save);
    Patch_Jump(0x00493860, &ExtEnvironmentClass::Hook_Ctor);
    Patch_Jump(0x00493890, &ExtEnvironmentClass::Hook_Dtor);
    Patch_Jump(0x00493810, &ExtEnvironmentClass::Static_Init);
    Patch_Jump(0x00493850, &ExtEnvironmentClass::Static_Deinit);
}
