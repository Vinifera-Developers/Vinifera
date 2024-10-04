/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUTOSURRENDER_HOOKS.H
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for auto-surrender in multiplayer.
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

#include "autosurrender_hooks.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "spawner.h"
#include "session.h"
#include "tibsun_globals.h"


static bool PlayerHasSurrendered = false;


/**
 *  Force surrender on abort.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_Standard_Options_Dialog_HANDLER_AutoSurrender)
{
    if (Spawner::Active && Spawner::Get_Config()->AutoSurrender)
    {
        if (Session.Type == GAME_IPX && !PlayerHasSurrendered)
        {
            SpecialDialog = SDLG_SURRENDER;
            JMP(0x004B6D2A);
        }
    }

    // Stolen bytes
    if (Session.Type != GAME_INTERNET)
    {
        JMP(0x004B6D20);
    }

    JMP(0x004B6D0D);
}


/**
 *  Force surrender on disconnection.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_EventClass_Execute_REMOVE_PLAYER_AutoSurrender)
{
    if (Spawner::Active && Spawner::Get_Config()->AutoSurrender)
    {
        if (Session.Type == GAME_IPX && Spawner::Get_Config()->AutoSurrender)
        {
            JMP(0x00494F16);
        }
    }

    // Stolen bytes
    if (Session.Type != GAME_INTERNET)
    {
        JMP(0x00494F28);
    }

    JMP(0x00494F0D);
}


/**
 *  Two patches to save whether the player has surrendered.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_Special_Dialog_Surrender1)
{
    PlayerHasSurrendered = true;
    _asm mov eax, PlayerPtr
    _asm mov eax, [eax]
    JMP_REG(ecx, 0x004627F8);
}


DECLARE_PATCH(_Special_Dialog_Surrender2)
{
    GET_REGISTER_STATIC(bool, has_surrendered, al);

    PlayerHasSurrendered = has_surrendered;

    // Stolen bytes
    if (has_surrendered)
    {
        JMP(0x00462842);
    }

    JMP(0x004628E3)
}


/**
 *  Main function for patching the hooks.
 */
void AutoSurrender_Hooks()
{
    Patch_Jump(0x004B6D04, &_Standard_Options_Dialog_HANDLER_AutoSurrender);
    Patch_Jump(0x00494F08, &_EventClass_Execute_REMOVE_PLAYER_AutoSurrender);
    Patch_Jump(0x004627F3, &_Special_Dialog_Surrender1);
    Patch_Jump(0x0046283A, &_Special_Dialog_Surrender2);
}
