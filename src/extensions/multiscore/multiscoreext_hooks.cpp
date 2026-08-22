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

#include "debughandler.h"
#include "extension.h"
#include "hooker.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "multiscore.h"
#include "scenario.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "vector.h"
#include "vinifera_globals.h"

#include <algorithm>

class HouseClassExtension;
/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class MultiScoreExt final : public MultiScore
{
public:
    void _Tally_Score();
};


void MultiScoreExt::_Tally_Score()
{
    /**
     *  Reset the score entry count.
     */
    Session.NumScores = 0;

    /**
     *  Find which player has spent the most credits.
     */
    int most_credits_spent = 0;

    for (HousesType house = HOUSE_FIRST; house < Houses.Count(); house++) {
        HouseClass* hptr = Houses[house];
        const HouseClassExtension* hext = hptr ? Extension::Fetch(hptr) : nullptr;

        /**
         *  Skip this house if it's multiplay passive, or is an observer.
         */
        if (!hptr || hptr->Class->IsMultiplayPassive || hext->IsObserver) {
            continue;
        }

        most_credits_spent = std::max<unsigned int>(Houses[house]->CreditsSpent, most_credits_spent);
    }

    /**
     *  Loop through all houses, tallying up each player's score.
     */
    for (HousesType house = HOUSE_FIRST; house < Houses.Count(); house++) {
        HouseClass* hptr = Houses[house];
        const HouseClassExtension* hext = hptr ? Extension::Fetch(hptr) : nullptr;

        /**
         *  Skip this house if it's multiplay passive, or is an observer.
         */
        if (!hptr || hptr->Class->IsMultiplayPassive || hext->IsObserver) {
            continue;
        }

        /**
         *  Now find out where this player is in the score array.
         */
        const int score_index = Session.NumScores++;

        /**
         *  Initialize this score entry.
         */
        Session.Score[score_index].Wins = 0;
        std::strncpy(Session.Score[score_index].Name, hptr->IniName.c_str(), std::size(Session.Score[score_index].Name) - 1);

        /**
         *  Init this player's statistics to 0 (-1 means he didn't play this round;
         *  0 means he played but did nothing).
         */
        Session.Score[score_index].Lost[0] = 0;
        Session.Score[score_index].Kills[0] = 0;
        Session.Score[score_index].Economy[0] = 0;
        Session.Score[score_index].Score[0] = 0;

        /**
         *  Init this player's color to his last-used color index
         */
        Session.Score[score_index].Color = static_cast<PlayerColorType>(hptr->Scheme);

        /**
         *  If this house was undefeated, it must have been the winner.
         *  (If no human houses are undefeated, the computer won.)
         */
        if (!hptr->IsDefeated) {
            Session.Score[score_index].Wins++;
            Session.Winner = score_index;

            /**
             *  Calculate the average score for all other houses and use it as a basseline, I guess? Score inflation.
             */
            int score = 0;
            int count = 0;

            for (HousesType house2 = HOUSE_FIRST; house2 < Houses.Count(); house2++) {
                const HouseClass* hptr2 = Houses[house2];
                const HouseClassExtension* hext2 = hptr2 ? Extension::Fetch(hptr2) : nullptr;

                /**
                 *  Skip this house if it's the same house, is multiplay passive, or is an observer.
                 */
                if (!hptr2 || hptr2->Class->IsMultiplayPassive || hptr == hptr2 || hext2->IsObserver) {
                    continue;
                }

                score += hptr->PointTotal;
                count++;
            }

            /**
             *  Average the scores.
             */
            if (count > 0) {
                score /= count;
            }

            score = std::max(200, score);
            Session.Score[score_index].Score[0] = score / 2;
        }

        /**
         *  Tally up all kills for this player.
         */
        unsigned total_kills = 0;
        for (int i = 0; i < std::size(hptr->UnitsKilled); i++) {
            total_kills += hptr->UnitsKilled[i];
        }

        for (int i = 0; i < std::size(hptr->BuildingsKilled); i++) {
            total_kills += hptr->BuildingsKilled[i];
        }

        Session.Score[score_index].Kills[0] = total_kills;

        /**
         *  Tally up the losses for this player.
         */
        const int total_losses = hptr->UnitsLost + hptr->BuildingsLost;
        Session.Score[score_index].Lost[0] = total_losses;

        /**
         *  Calculate the kill to loss ratio.
         */
        double kill_ratio = 0.0;
        if (total_losses > 0) {
            kill_ratio = static_cast<double>(total_kills) / total_losses;
        }

        /*
         *  Calculate a percentage of how many credits this house has
         *  spent compared to the house that spent the highest
         *  amount of credits during the match.
         *  
         *  @author: Rampastring
         */
        double build_economy;
        if (most_credits_spent > 0) {
            build_economy = static_cast<double>(hptr->CreditsSpent) / most_credits_spent;
        }
        else {
            build_economy = 0;
        }
        Session.Score[score_index].Economy[0] = (build_economy * 100.0);

        /**
         *  A score of 100 prints as 99 for some reason, so we do this to make it print 100.
         */
        if (Session.Score[score_index].Economy[0] == 100) {
            Session.Score[score_index].Economy[0] = 102;
        }

        /**
         *  Set the player's score.
         */
        if (hptr->PointTotal > 0) {
            Session.Score[score_index].Score[0] += hptr->PointTotal;
        }

        /**
         *  Print if this player is a winner or a loser.
         */
        const char* win_string = Session.Score[score_index].Wins > 0 ? "Winner" : "Loser";

        DEBUG_INFO(
            "{}: {}\n Scheme: {}\n Lost = {}\n Kills = {}\n Economy = {}\n Score = {}\n",
            Session.Score[score_index].Name,
            win_string,
            (int)Session.Score[score_index].Color,
            Session.Score[score_index].Lost[0],
            Session.Score[score_index].Kills[0],
            Session.Score[score_index].Economy[0],
            Session.Score[score_index].Score[0]);

        DEBUG_INFO(" KillRatio = {}\n BuildEconomy = {}\n", kill_ratio, build_economy);
    }
}


/**
 *  Main function for patching the hooks.
 */
void MultiScoreExtension_Hooks()
{
    Patch_Jump(0x005687A0, &MultiScoreExt::_Tally_Score);
    
    /**
     *  #issue-187
     *  
     *  Fixes incorrect spelling of "Loser" on the multiplayer score screen debug output.
     * 
     *  @author: CCHyper
     */
    Patch_Dword(0x00568A05 + 1, (uintptr_t)&"Loser"); // +1 skips "mov eax," opcode
}
