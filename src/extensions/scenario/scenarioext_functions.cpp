/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SCENARIOEXT_FUNCTIONS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the supporting functions for the extended ScenarioClass.
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
#include "scenarioext_functions.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "session.h"
#include "scenario.h"
#include "rules.h"
#include "iomap.h"
#include "house.h"
#include "housetype.h"
#include "object.h"
#include "techno.h"
#include "technotype.h"
#include "infantrytype.h"
#include "unit.h"
#include "unittype.h"
#include "language.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


#define HOUSE_GDI HousesType(0)
#define HOUSE_NOD HousesType(1)


/**
 *  Assigns multiplayer houses to various players.
 * 
 *  @author: 06/09/1995 BRR - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 */
void Vinifera_Assign_Houses()
{
    bool assigned[MAX_PLAYERS];     // true = this house slot is in use.
    bool color_used[MAX_PLAYERS];   // true = this color is in use.

    HouseClass *housep;
    HousesType house;
    int lowest_color;
    int index;
    HousesType pref_house;
    int color;

    DEBUG_INFO("Assign_Houses(enter)\n");

    /**
     *  Initialize
     */
    std::memset(assigned, 0, MAX_PLAYERS * sizeof(bool));
    std::memset(color_used, 0, MAX_PLAYERS * sizeof(bool));
    
    if (Session.Players.Count() > 0) {
        DEBUG_INFO("  Assigning players (%d)...\n", Session.Players.Count());
    }

    /**
     *  Assign each player in 'Players' to a multiplayer house. Players will
     *  be sorted by their chosen color value (this value must be unique among
     *  all the players).
     */
    for (int i = 0; i < Session.Players.Count(); i++) {

        /**
         *  Find the player with the lowest color index.
         */
        index = 0;
        lowest_color = -1;
        for (int j = 0; j < Session.Players.Count(); j++) {

            /**
             *  If we've already assigned this house, skip it.
             */
            if (assigned[j]) {
                continue;
            }
            if (lowest_color == -1 || Session.Players[j]->Player.Color < lowest_color) {
                lowest_color = Session.Players[j]->Player.Color;
                index = j;
            }
        }

        NodeNameTag &node = *Session.Players[index];

        /**
         *  Mark this player as having been assigned.
         */
        assigned[index] = true;
        color_used[node.Player.Color] = true;

        /**
         *  Assign the lowest-color'd player to the next available slot
         *  in the HouseClass array.
         */
        housep = new HouseClass(HouseTypes[node.Player.House]);

        std::memset((char *)housep->IniName, 0, MPLAYER_NAME_MAX);
        std::strncpy((char *)housep->IniName, node.Name, MPLAYER_NAME_MAX-1);

        /**
         *  Set the house's IsHuman, Credits, ActLike, and RemapTable.
         */
        housep->IsHuman = true;

        housep->Control.TechLevel = BuildLevel;
        housep->Init_Data((PlayerColorType)node.Player.Color,
            node.Player.House, Session.Options.Credits);
        housep->RemapColor = Session.Player_Color_To_Scheme_Color((PlayerColorType)node.Player.Color);
        housep->Init_Remap_Color();

        /**
         *  If this ID is for myself, set up PlayerPtr.
         */
        if (index == 0) {
            PlayerPtr = housep;
            housep->IsPlayerControl = true;
        }

        housep->Assign_Handicap(DIFF_NORMAL);

        /**
         *  Record where we placed this player.
         */
        node.Player.ID = HousesType(housep->ID);

        DEBUG_INFO("    Assigned player \"%s\" (House: \"%s\", ID: %d, Color: \"%s\") to slot %d.\n",
            node.Name, housep->Class->Name(), node.Player.ID, ColorSchemes[housep->RemapColor]->Name, i);
    }

    if (Session.Options.AIPlayers > 0) {
        DEBUG_INFO("  Assigning computer players (%d)...\n", Session.Options.AIPlayers);
    }

    /**
     *  Now assign computer players to the remaining houses.
     */
    for (int i = Session.Players.Count(); i < Session.Players.Count() + Session.Options.AIPlayers; ++i) {

        if (Percent_Chance(50)) {
            pref_house = HOUSE_GDI;
        } else {
            pref_house = HOUSE_NOD;
        }

        /**
         *  Pick a color for this house; keep looping until we find one.
         */
        while (true) {
            color = Random_Pick(0, (MAX_PLAYERS-1));
            if (color_used[color] == false) {
                break;
            }
        }
        color_used[color] = true;

        housep = new HouseClass(HouseTypes[pref_house]);

        /**
         *  Set the house's IsHuman, Credits, ActLike, and RemapTable.
         */
        housep->IsHuman = false;
        //housep->IsStarted = true;

        housep->Control.TechLevel = BuildLevel;
        housep->Init_Data((PlayerColorType)color, pref_house, Session.Options.Credits);
        housep->RemapColor = Session.Player_Color_To_Scheme_Color((PlayerColorType)color);
        housep->Init_Remap_Color();

        std::strcpy(housep->IniName, Text_String(TXT_COMPUTER));

        if (Session.Type != GAME_NORMAL) {
            housep->IQ = Rule->MaxIQ;
        }

        DiffType difficulty = Scen->CDifficulty;

        if (Session.Players.Count() > 1 && Rule->IsCompEasyBonus && difficulty > DIFF_EASY) {
            difficulty = (DiffType)(difficulty - 1);
        }
        housep->Assign_Handicap(difficulty);

        DEBUG_INFO("    Assigned computer house \"%s\" (ID: %d, Color: \"%s\") to slot %d.\n",
            housep->Class->Name(), housep->ID, ColorSchemes[housep->RemapColor]->Name, i);
    }

    /**
     *  Create Neutral and Special houses as they must exist!
     * 
     *  #BUGFIX:
     *  Added checks to make sure the houses exist before blindly
     *  attempting to create a instance of them.
     */
    
    house = HouseTypeClass::From_Name("Neutral");
    if (house != HOUSE_NONE) {
        DEBUG_INFO("  Creating Neutral house...\n");

        housep = new HouseClass(HouseTypes[house]);
        housep->RemapColor = ColorScheme::From_Name("LightGrey");
        housep->Init_Remap_Color();
    }

    house = HouseTypeClass::From_Name("Special");
    if (house != HOUSE_NONE) {
        DEBUG_INFO("  Creating Special house...\n");
        
        housep = new HouseClass(HouseTypes[house]);
        housep->RemapColor = ColorScheme::From_Name("LightGrey");
        housep->Init_Remap_Color();

    }

    DEBUG_INFO("Assign_Houses(exit)\n");
}
