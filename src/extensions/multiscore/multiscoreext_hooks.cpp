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

#include <algorithm>
#include "debughandler.h"
#include "asserthandler.h"

#include "tibsun_globals.h"
#include "house.h"
#include "vector.h"

#include "hooker.h"
#include "scenario.h"
#include "syringe.h"
#include "vinifera_globals.h"


/**
 *  #issue-544
 *
 *  Fixes the nonsensical economy stat in the score screen.
 *  Records the highest "credits spent" score from all players for
 *  later use in comparing players' economy scores.
 *
 *  @author: Rampastring
 */
static int MostCreditsSpent;
EXPORT_FUNC(_MultiScore_Tally_Score_Fetch_Largest_CreditsSpent_Score)
{
    MostCreditsSpent = 0;
    for (int i = 0; i < Houses.Count(); i++) {
        MostCreditsSpent = std::max<unsigned int>(Houses[i]->CreditsSpent, MostCreditsSpent);
    }
    return 0;
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
EXPORT_FUNC(_MultiScore_Tally_Score_Calculate_Economy_Score)
{
    GET(HouseClass *, house, EBX);
    int economy_score;

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

    R->EAX(economy_score);

    /**
     *  Assign economy score and continue score processing.
     */
    return 0x005689E0;
}


/**
 *  Patches the score screen to show the total time since the scenario was started,
 *  not since the last time the game was loaded.
 *
 *  @author: ZivDero
 */
EXPORT_FUNC(_MultiScore_568BE0_ElapsedTime_Patch)
{
    unsigned elapsed_time = Scen->ElapsedTimer.Value() + Vinifera_TotalPlayTime;
    R->EBX(elapsed_time);
    return 0x00568D38;
}


/**
 *  Main function for patching the hooks.
 */
void MultiScoreExtension_Hooks()
{
    /**
     *  #issue-187
     *  
     *  Fixes incorrect spelling of "Loser" on the multiplayer score screen debug output.
     * 
     *  @author: CCHyper
     */
    Patch_Dword(0x00568A05 + 1, (uintptr_t)&"Loser"); // +1 skips "mov eax," opcode
}

declhook(0x005687A9, _MultiScore_Tally_Score_Fetch_Largest_CreditsSpent_Score, 0x6);
declhook(0x005689D5, _MultiScore_Tally_Score_Calculate_Economy_Score, 0);
declhook(0x00568D10, _MultiScore_568BE0_ElapsedTime_Patch, 0);
