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
#include "extension.h"

#include "tibsun_globals.h"
#include "house.h"
#include "vector.h";

#include "hooker.h"
#include "hooker_macros.h"
#include "houseext.h"
#include "housetype.h"
#include "multiscore.h"
#include "scenario.h"
#include "session.h"
#include "vinifera_globals.h"


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
        const HouseClassExtension* hext = hptr ? Extension::Fetch<HouseClassExtension>(hptr) : nullptr;

        /**
         *  Skip this house if it's multiplay passive, or is an observer.
         */
        if (!hptr || hptr->Class->IsMultiplayPassive || hext->IsObserver) {
            continue;
        }

        if (Houses[house]->CreditsSpent > most_credits_spent) {
            most_credits_spent = Houses[house]->CreditsSpent;
        }
    }

    /**
     *  Loop through all houses, tallying up each player's score.
     */
    for (HousesType house = HOUSE_FIRST; house < Houses.Count(); house++) {
        HouseClass* hptr = Houses[house];
        const HouseClassExtension* hext = hptr ? Extension::Fetch<HouseClassExtension>(hptr) : nullptr;

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
        std::strncpy(Session.Score[score_index].Name, hptr->IniName, std::size(Session.Score[score_index].Name) - 1);

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
        Session.Score[score_index].Color = static_cast<PlayerColorType>(hptr->RemapColor);

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
                const HouseClassExtension* hext2 = hptr2 ? Extension::Fetch<HouseClassExtension>(hptr2) : nullptr;

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

        /**
         *  Original economy score calculation. Ratio of currently owned objects to total built objects.
         */
#if 0
        int total_owned = hptr->ActiveBQuantity.Total();
        total_owned += hptr->ActiveUQuantity.Total();
        total_owned += hptr->ActiveIQuantity.Total();
        total_owned += hptr->ActiveAQuantity.Total();

        double build_economy;
        int total_built = total_owned + hptr->BuildingsLost + hptr->UnitsLost;
        if (total_built <= 0) {
            build_economy = 0.0;
        }
        else {
            build_economy = total_owned / total_built;
        }
        build_economy = std::max(0.0, build_economy);
        Session.Score[score_index].Economy[0] = (build_economy * 100.0);
#endif
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
            "%s: %s\n Scheme: %d\n Lost = %d\n Kills = %d\n Economy = %d\n Score = %d\n",
            Session.Score[score_index].Name,
            win_string,
            Session.Score[score_index].Color,
            Session.Score[score_index].Lost[0],
            Session.Score[score_index].Kills[0],
            Session.Score[score_index].Economy[0],
            Session.Score[score_index].Score[0]);

        DEBUG_INFO(" KillRatio = %f\n BuildEconomy = %f\n", kill_ratio, build_economy);
    }
}


/**
 *  Patches the score screen to show the total time since the scenario was started,
 *  not since the last time the game was loaded.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_MultiScore_568BE0_ElapsedTime_Patch)
{
    static unsigned elapsed_time;
    elapsed_time = Scen->ElapsedTimer.Value() + Vinifera_TotalPlayTime;

    _asm mov ebx, elapsed_time
    JMP(0x00568D38);
}


/**
 *  Main function for patching the hooks.
 */
void MultiScoreExtension_Hooks()
{
    Patch_Jump(0x00568D10, &_MultiScore_568BE0_ElapsedTime_Patch);
    Patch_Jump(0x005687A0, &MultiScoreExt::_Tally_Score);
}
