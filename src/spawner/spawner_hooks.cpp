/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the multiplayer spawner class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "spawner_hooks.h"

#include "hooker.h"
#include "house.h"
#include "housetype.h"
#include "movieplayback.h"
#include "movieskip.h"
#include "observer_hooks.h"
#include "protocolzero_hooks.h"
#include "quickmatch_hooks.h"
#include "session.h"
#include "sessionext.h"
#include "spawner.h"
#include "statistics_hooks.h"
#include "syringe.h"
#include "tibsun_functions.h"
#include "vinifera_globals.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class SessionClassExt : public SessionClass
{
public:
    void _Read_Scenario_Descriptions();
};


/**
 *  Patches Read_Scenario_Descriptions to do nothing when the spawner is active.
 *
 *  @author: ZivDero
 */
void SessionClassExt::_Read_Scenario_Descriptions()
{
    if (SessionExtension->IsSpawnerSession) {
        return;
    }

    SessionClass::Read_Scenario_Descriptions();
}


/**
 *  Patches Expert AI not the consider allies as enemies.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004C06EF, _HouseClass_Expert_AI_Check_Allies, 0)
{
    GET(HouseClass*, this_ptr, EDI);
    GET(HouseClass*, house, ESI);

    if (house != this_ptr && !house->Class->IsMultiplayPassive && !house->IsDefeated && this_ptr->Is_Ally(house)) {
        return 0x004C06F7;
    }

    return 0x004C0777;
}


/**
 *  In multiplayer, convert ESC into a skip vote and only let the original
 *  VQA breakout path run once every connected player has voted.
 *
 *  @author: ZivDero, Rampastring
 */
DEFINE_HOOK(0x0066BB57, _Play_VQA_Forbid_Skipping_In_MP_Patch, 0)
{
    GET_STACK(bool, cant_break_out, 0x40);

    if (cant_break_out) {
        return 0x0066BA30;
    }

    if (!Session.Singleplayer_Game() && !MovieSkip::Is_Local_Skip_Allowed()) {
        MovieSkip::Update_Input();

        if (!MovieSkip::Should_Skip()) {
            return 0x0066BA30;
        }

        MovieSkip::Prepare_Legacy_Breakout();
    }

    /**
     *  Let the original VQA code consume ESC and perform its normal cleanup.
     */
    return 0x0066BB61;
}


/**
 *  Hack VQA playback loop to do some network communication in MP
 *  so the tunnel server doesn't forget about us.
 *
 *  @author: ZivDero, Rampastring
 */
DEFINE_HOOK(0x0066BA56, _Play_VQA_Network_Callback_Patch, 7)
{
    MoviePlayback_Update_Networking();

    return 0;
}


/**
 *  Prevents AI Takeover if autosurrender is turned on.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0057524A, _Destroy_Connection_AutoSurrender_Patch, 0)
{
    GET(HouseClass*, hptr, EBP);

    if ((Session.Type == GAME_INTERNET && WestwoodOnline_Tournament) || SessionExtension->ExtOptions.IsAutoSurrender) {
        hptr->Flag_To_Die();
    } else {
        hptr->AI_Takeover();
    }

    return 0x0057526B;
}


/**
 *  Changes the waiting for players timeout.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005DB794, _Wait_For_Load_Timeout_Patch, 5)
{
    if (SessionExtension->ConnTimeout > 0) {
        R->ECX(SessionExtension->ConnTimeout);
        return 0x005DB799;
    }

    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void Spawner_Hooks()
{
    Patch_Call(0x004629D1, &Spawner::Start_Game); // Select_Game in Main_Game
    Patch_Call(0x00462B8B, &Spawner::Start_Game); // Select_Game in Main_Game

    /**
     *  The spawner allows player to jump right into a game, so no need to
     *  show the startup movies.
     */
    Vinifera_SkipLogoMovies = true;
    Vinifera_SkipStartupMovies = true;

    /**
     *  Remove calls to SessionClass::Read_Scenario_Descriptions() when the
     *  spawner is active. This will speed up the initialisation and loading
     *  process, as PKT and MPR files are not required when using the spawner.
     */
    Patch_Call(0x004E8910, &SessionClassExt::_Read_Scenario_Descriptions); // New_Main_Menu
    Patch_Call(0x00564BAE, &SessionClassExt::_Read_Scenario_Descriptions); // Select_MPlayer_Game
    Patch_Call(0x0057FE2A, &SessionClassExt::_Read_Scenario_Descriptions); // NewMenuClass::Process_Game_Select
    Patch_Call(0x0058037C, &SessionClassExt::_Read_Scenario_Descriptions); // NewMenuClass::
    Patch_Call(0x005ED477, &SessionClassExt::_Read_Scenario_Descriptions); // SessionClass::One_Time

    /**
     *  Hooks for various sub-modules.
     */
    ProtocolZero_Hooks();
    Observer_Hooks();
    QuickMatch_Hooks();
    Statistics_Hooks();
}
