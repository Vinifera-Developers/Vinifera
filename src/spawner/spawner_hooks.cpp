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

#include "autosurrender_hooks.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "nethack.h"
#include "session.h"
#include "spawner.h"
#include "house.h"
#include "housetype.h"
#include "multiscore.h"
#include "protocolzero_hooks.h"
#include "quickmatch_hooks.h"
#include "spectator_hooks.h"
#include "statistics_hooks.h"
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


void SessionClassExt::_Read_Scenario_Descriptions()
{
    if (Spawner::Active)
        return;

    SessionClass::Read_Scenario_Descriptions();
}


/**
  *  A fake class for implementing new member functions which allow
  *  access to the "this" pointer of the intended class.
  *
  *  @note: This must not contain a constructor or destructor.
  *
  *  @note: All functions must not be virtual and must also be prefixed
  *         with "_" to prevent accidental virtualization.
  */
class HouseClassExt : public HouseClass
{
public:
    void _Computer_Paranoid() {}
};


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


static void MultiScore_Wrapper()
{
    if (!Spawner::Get_Config()->SkipScoreScreen)
        MultiScore::Presentation();
}


void Spawner_Hooks()
{
    Patch_Call(0x004629D1, &Spawner::Start_Game);   // Main_Game
    Patch_Call(0x00462B8B, &Spawner::Start_Game);   // Main_Game
    Patch_Call(0x006A2525, &NetHack::SendTo);       // NetHack
    Patch_Call(0x006A25F9, &NetHack::RecvFrom);     // NetHack

    /**
     *  The spawner allows player to jump right into a game, so no need to
     *  show the startup movies.
     */
    Vinifera_SkipLogoMovies = true;
    Vinifera_SkipStartupMovies = true;

    Patch_Dword(0x005DB794 + 1, Spawner::Get_Config()->ConnTimeout); // Set ConnTimeout

    /**
     *  Skip some checks inside HouseClass::MPlayer_Defeated to make the game continue
     *  seven if there are only allied AI players left in Skirmish.
     */
    Patch_Jump(0x004BF7B6, 0x004BF7BF);
    Patch_Jump(0x004BF7F0, 0x004BF7F9);

    /**
     *  Remove calls to SessionClass::Read_Scenario_Descriptions() when the
     *  spawner is active. This will speed up the initialisation and loading
     *  process, as the reason of PKT and MPR files are not required when using
     *  the spawner.
     */
    Patch_Call(0x004E8910, &SessionClassExt::_Read_Scenario_Descriptions); // New_Main_Menu
    Patch_Call(0x00564BAE, &SessionClassExt::_Read_Scenario_Descriptions); // Select_MPlayer_Game
    Patch_Call(0x0057FE2A, &SessionClassExt::_Read_Scenario_Descriptions); // NewMenuClass::Process_Game_Select
    Patch_Call(0x0058037C, &SessionClassExt::_Read_Scenario_Descriptions); // NewMenuClass::
    Patch_Call(0x005ED477, &SessionClassExt::_Read_Scenario_Descriptions); // SessionClass::One_Time

    Patch_Jump(0x004C06EF, &_HouseClass_Expert_AI_Check_Allies);
    Patch_Jump(0x004C3630, &HouseClassExt::_Computer_Paranoid);  // Disable paranoid computer behavior

    // SkipScoreScreen feature
    Patch_Call(0x005DC9DA, &MultiScore_Wrapper);
    Patch_Call(0x005DCD98, &MultiScore_Wrapper);

    ProtocolZero_Hooks();
    Spectator_Hooks();
    QuickMatch_Hooks();
    AutoSurrender_Hooks();
    Statistics_Hooks();
}
