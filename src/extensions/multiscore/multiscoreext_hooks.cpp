/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended MultiScore class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "multiscoreext_hooks.h"

#include "hooker.h"
#include "house.h"
#include "scenario.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "vector.h"
#include "vinifera_globals.h"

#include <algorithm>


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
DEFINE_HOOK(0x005687A9, _MultiScore_Tally_Score_Fetch_Largest_CreditsSpent_Score, 6)
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
DEFINE_HOOK(0x005689D5, _MultiScore_Tally_Score_Calculate_Economy_Score, 0)
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
DEFINE_HOOK(0x00568D10, _MultiScore_568BE0_ElapsedTime_Patch, 0)
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
