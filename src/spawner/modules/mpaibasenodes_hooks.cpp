/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MPAIBASENODES_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks making the AI use base nodes in MP.
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

#include "mpaibasenodes_hooks.h"

#include "hooker.h"
#include "session.h"
#include "tibsun_globals.h"

#include "hooker_macros.h"
#include "spawner.h"


/**
 *  Patches the AI to use base nodes in MP Co-op.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Building_MP_AI_BaseNodes_Patch)
{
    _asm pushad

    /**
     *  Use base nodes in Campaign.
     */
    if (Session.Type == GAME_NORMAL)
    {
        _asm popad
        JMP_REG(ecx, 0x004C1554);
    }

    /**
     *  Also use base nodes if it was requiested by the client.
     */
    if (Spawner::Active && Spawner::Get_Config()->UseMPAIBaseNodes)
    {
        _asm popad
        JMP_REG(ecx, 0x004C1554);
    }

    /**
     *  Continue checks.
     */
    _asm popad
    JMP_REG(ecx, 0x004C129D);
}


/**
 *  Patches the AI to ignore base spacing when using base nodes in MP.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Can_Build_Here_MP_AI_BaseNodes_Patch)
{
    // Stolen instructions
    _asm push edi
    _asm mov edi, ecx

    /**
     *  Ignore AIBaseSpacing in Campaign.
     */
    if (Session.Type == GAME_NORMAL)
    {
        // return 1;
        JMP(0x004CB9D2);
    }

    /**
     *  Also ignore AIBaseSpacing if it was requiested by the client.
     */
    if (Spawner::Active && Spawner::Get_Config()->UseMPAIBaseNodes)
    {
        // return 1;
        JMP(0x004CB9D2);
    }

    /**
     *  Continue with AIBaseSpacing.
     */
    JMP_REG(ecx, 0x004CB9DE);
}


/**
 *  Patches the AI not to try raising money when using base nodes in MP.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Expert_AI_MP_AI_BaseNodes_Patch)
{
    _asm push eax

    if (Session.Type == GAME_NORMAL && !(Spawner::Active && Spawner::Get_Config()->UseMPAIBaseNodes))
    {
        // Potentially try to raise money
        _asm pop eax
        JMP_REG(ecx, 0x004C08D1);
    }

    /**
     *  Skip trying to raise money.
     */
    _asm pop eax
    JMP_REG(ecx, 0x004C09AF);
}


/**
 *  Main function for patching the hooks.
 */
void MPAIBaseNodes_Hooks()
{
    Patch_Jump(0x004C128F, &_HouseClass_AI_Building_MP_AI_BaseNodes_Patch);
    Patch_Jump(0x004CB9CD, &_HouseClass_Can_Build_Here_MP_AI_BaseNodes_Patch);
    Patch_Jump(0x004C08C5, &_HouseClass_Expert_AI_MP_AI_BaseNodes_Patch);
}
