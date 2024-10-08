/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MULTISCOREEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended MultiScore class.
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
#include "multiscoreext_hooks.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "tibsun_globals.h"
#include "house.h"
#include "vector.h";

#include "hooker.h"
#include "hooker_macros.h"


static int MostCreditsSpent;

static void _MultiScore_Tally_Score_Get_Largest_CreditsSpent_Score()
{
    MostCreditsSpent = 0;

    for (int i = 0; i < Houses.Count(); i++) {
        if (Houses[i]->CreditsSpent > MostCreditsSpent) {
            MostCreditsSpent = Houses[i]->CreditsSpent;
        }
    }
}


/**
 *  #issue-544
 *
 *  Fixes the nonsensical economy stat in the score screen.
 *  Records the highest "credits spent" score from all players for
 *  later use in comparing players' economy scores.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_MultiScore_Tally_Score_Fetch_Largest_CreditsSpent_Score)
{
    _MultiScore_Tally_Score_Get_Largest_CreditsSpent_Score();

    /**
     *  Stolen bytes / code.
     */
    _asm { mov ecx, dword ptr ds:0x007E1568 }
    JMP(0x005687AF);
}


/**
 *  #issue-544
 *
 *  Fixes the nonsensical economy stat in the score screen.
 *  Calculates a player's economy score based on their amount of
 *  credits spent.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_MultiScore_Tally_Score_Calculate_Economy_Score)
{
    GET_REGISTER_STATIC(HouseClass *, house, ebx);
    static int economy_score;

    /*
     * Calculate a percentage of how many credits this house has
     * spent compared to the house that spent the highest
     * amount of credits during the match.
     */
    if (MostCreditsSpent > 0) {
        if (house->CreditsSpent >= MostCreditsSpent) {
            /**
             *  For some reason the score screen presentation seems to
             *  lower this by some 1-2%, so we take that into account.
             *  TODO investigate and fix
             */
            economy_score = 102;
        }
        else {
            economy_score = (house->CreditsSpent * 100) / MostCreditsSpent;
        }
    } else {
        economy_score = 0;
    }

    _asm { mov eax, [economy_score] };

    /**
     *  Assign economy score and continue score processing.
     */
    JMP_REG(ecx, 0x005689E0);
}

/**
 *  Main function for patching the hooks.
 */
void MultiScoreExtension_Hooks()
{
    Patch_Jump(0x005687A9, &_MultiScore_Tally_Score_Fetch_Largest_CreditsSpent_Score);
    Patch_Jump(0x005689D5, &_MultiScore_Tally_Score_Calculate_Economy_Score);

    /**
     *  #issue-187
     *  
     *  Fixes incorrect spelling of "Loser" on the multiplayer score screen debug output.
     * 
     *  @author: CCHyper
     */
    static const char *TEXT_LOSER = "Loser";
    Patch_Dword(0x00568A05+1, (uintptr_t)TEXT_LOSER); // +1 skips "mov eax," opcode
}
