/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SPAWNER_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the multiplayer spawner class.
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

#include "spawner_hooks.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "session.h"
#include "spawner.h"
#include "house.h"
#include "housetype.h"
#include "multiscore.h"
#include "protocolzero_hooks.h"
#include "quickmatch_hooks.h"
#include "observer_hooks.h"
#include "statistics_hooks.h"
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
    if (Vinifera_SpawnerActive)
        return;

    SessionClass::Read_Scenario_Descriptions();
}


/**
 *  Patches Expert AI not the consider allies as enemies.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Expert_AI_Check_Allies)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, edi);
    GET_REGISTER_STATIC(HouseClass*, house, esi);

    if (house != this_ptr && !house->Class->IsMultiplayPassive && !house->IsDefeated && this_ptr->Is_Ally(house))
    {
        JMP(0x004C06F7);
    }

    JMP(0x004C0777);
}


/**
 *  Players skipping movies in multiplayer leads to disconnects.
 *  Prevent players from skipping movies in MP.
 *
 *  @author: ZivDero, Rampastring
 */
DECLARE_PATCH(_Play_VQA_Forbid_Skipping_In_MP_Patch)
{
    GET_STACK_STATIC8(bool, cant_break_out, esp, 0x40);

    /**
     *  Don't skip the movie.
     */
    if (cant_break_out || (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH))
    {
        JMP(0x0066BA30);
    }

    /**
     *  Check if we want to skip the movie.
     */
    JMP(0x0066BB61);
}


/**
 *  Hack VQA playback loop to do some network communication in MP
 *  so the tunnel server doesn't forget about us.
 *
 *  @author: ZivDero, Rampastring
 */
DECLARE_PATCH(_Play_VQA_Network_Callback_Patch)
{
    if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
        static unsigned NextNetworkRefreshTime = UINT_MAX;

        if (timeGetTime() >= NextNetworkRefreshTime) {
            Session.Loading_Callback(100);
            Call_Back();
        }

        NextNetworkRefreshTime = timeGetTime() + 1000;
    }

    // Stolen instructions
    _asm
    {
        push ebx
        push ebx
        push ebx
        lea  edx, [esp + 0x28]
    }

    JMP(0x0066BA5D);
}


/**
 *  Prevents AI Takeover if autosurrender is turned on.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_Destroy_Connection_AutoSurrender_Patch)
{
    GET_REGISTER_STATIC(HouseClass*, hptr, ebp);

    if ((Session.Type == GAME_INTERNET && PlanetWestwoodTournament) || (Vinifera_SpawnerActive && Vinifera_SpawnerConfig->AutoSurrender))
    {
        hptr->Flag_To_Die();
    }
    else
    {
        hptr->AI_Takeover();
    }

    JMP(0x0057526B);
}


/**
 *  Main function for patching the hooks.
 */
void Spawner_Hooks()
{
    Patch_Call(0x004629D1, &Spawner::Start_Game);   // Main_Game
    Patch_Call(0x00462B8B, &Spawner::Start_Game);   // Main_Game
    
    /**
     *  The spawner allows player to jump right into a game, so no need to
     *  show the startup movies.
     */
    Vinifera_SkipLogoMovies = true;
    Vinifera_SkipStartupMovies = true;

    Patch_Dword(0x005DB794 + 1, Vinifera_SpawnerConfig->ConnTimeout); // Set ConnTimeout

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

    Patch_Jump(0x004C06EF, &_HouseClass_Expert_AI_Check_Allies);

    /**
     *  PlayMoviesInMultiplayer feature.
     */
    Patch_Jump(0x0066BB57, &_Play_VQA_Forbid_Skipping_In_MP_Patch);
    Patch_Jump(0x0066BA56, &_Play_VQA_Network_Callback_Patch);

    /**
     *  AutoSurrender feature.
     */
    Patch_Jump(0x0057524A, &_Destroy_Connection_AutoSurrender_Patch);

    /**
     *  Hooks for various sub-modules.
     */
    ProtocolZero_Hooks();
    Observer_Hooks();
    QuickMatch_Hooks();
    Statistics_Hooks();
}
