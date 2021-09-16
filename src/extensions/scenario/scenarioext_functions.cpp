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
#include "building.h"
#include "buildingtype.h"
#include "language.h"
#include "sessionext.h"
#include "session.h"
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
    HouseTypeClass *housetype;
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

#if 0
        if (Percent_Chance(50)) {
            pref_house = HOUSE_GDI;
        } else {
            pref_house = HOUSE_NOD;
        }
#endif

        /**
         *  #issue-7
         * 
         *  Replaces code from above.
         * 
         *  Fixes a limitation where the AI would only be able to choose
         *  between the houses GDI (0) and NOD (1). Now, all houses that
         *  have "IsMultiplay" true will be considered for sellection.
         */
        while (true) {
            pref_house = (HousesType)Random_Pick(0, HouseTypes.Count()-1);
            if (HouseTypes[pref_house]->IsMultiplay) {
                break;
            }
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


/**
 *  Randomly scatters from given cell; won't fall off map.
 * 
 *  @author: 07/30/1995 BRR - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 */
static Cell Clip_Scatter(Cell cell, int maxdist)
{
    /**
     *  Get X & Y coords of given starting cell.
     */
    int x = cell.X;
    int y = cell.Y;

    /**
     *  Compute our x & y limits
     */
    int xmin = Map.MapCellX;
    int xmax = xmin + Map.MapCellWidth - 1;
    int ymin = Map.MapCellY;
    int ymax = ymin + Map.MapCellHeight - 1;

    /**
     *  Adjust the x-coordinate.
     */
    int xdist = Random_Pick(0, maxdist);
    if (Percent_Chance(50)) {
        x += xdist;
        if (x > xmax) {
            x = xmax;
        }
    } else {
        x -= xdist;
        if (x < xmin) {
            x = xmin;
        }
    }

    /**
     *  Adjust the y-coordinate.
     */
    int ydist = Random_Pick(0, maxdist);
    if (Percent_Chance(50)) {
        y += ydist;
        if (y > ymax) {
            y = ymax;
        }
    } else {
        y -= ydist;
        if (y < ymin) {
            y = ymin;
        }
    }

    return XY_Cell(x, y);
}


/**
 *  Moves in given direction from given cell; clips to map.
 * 
 *  @author: 07/30/1995 BRR - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 */
static Cell Clip_Move(Cell cell, FacingType facing, int dist)
{
    /**
     *  Get X & Y coords of given starting cell.
     */
    int x = cell.X;
    int y = cell.Y;

    /**
     *  Compute our x & y limits.
     */
    int xmin = Map.MapCellX;
    int xmax = xmin + Map.MapCellWidth - 1;
    int ymin = Map.MapCellY;
    int ymax = ymin + Map.MapCellHeight - 1;

    /**
     *  Adjust the x-coordinate.
     */
    switch (facing) {
        case FACING_N:
            y -= dist;
            break;

        case FACING_NE:
            x += dist;
            y -= dist;
            break;

        case FACING_E:
            x += dist;
            break;

        case FACING_SE:
            x += dist;
            y += dist;
            break;

        case FACING_S:
            y += dist;
            break;

        case FACING_SW:
            x -= dist;
            y += dist;
            break;

        case FACING_W:
            x -= dist;
            break;

        case FACING_NW:
            x -= dist;
            y -= dist;
            break;
    }

    /**
     *  Clip to the map
     */
    if (x > xmax) x = xmax;
    if (x < xmin) x = xmin;

    if (y > ymax) y = ymax;
    if (y < ymin) y = ymin;

    return XY_Cell(x, y);
}


/**
 *  Places an object >near< the given cell.
 * 
 *  @author: 06/09/1995 BRR - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 */
static int Scan_Place_Object(ObjectClass *obj, Cell cell)
{
    int dist;               // for object placement
    FacingType rot;         // for object placement
    FacingType fcounter;    // for object placement
    int tryval;
    Cell newcell;
    TechnoClass *techno;
    bool skipit;

    /**
     *  First try to unlimbo the object in the given cell.
     */
    if (Map.In_Radar(cell)) {
        techno = Map[cell].Cell_Techno();
        if (!techno || (techno->What_Am_I() == RTTI_INFANTRY &&
            obj->What_Am_I() == RTTI_INFANTRY)) {
            Coordinate coord = Cell_Coord(newcell, true);
            coord.Z = Map.Get_Cell_Height(coord);
            if (obj->Unlimbo(coord, DIR_N)) {
                return true;
            }
        }
    }

    /**
     *  Loop through distances from the given center cell; skip the center cell.
     *  For each distance, try placing the object along each rotational direction;
     *  if none are available, try each direction with a random scatter value.
     *  If that fails, go to the next distance.
     *  This ensures that the closest coordinates are filled first.
     */
    for (dist = 1; dist < 32; dist++) {

        /**
         *  Pick a random starting direction
         */
        rot = Random_Pick(FACING_N, FACING_NW);

        /**
         *  Try all directions twice
         */
        for (tryval = 0 ; tryval < 2; tryval++) {

            /**
             *  Loop through all directions, at this distance.
             */
            for (fcounter = FACING_N; fcounter <= FACING_NW; fcounter++) {

                skipit = false;

                /**
                 *  Pick a coordinate along this directional axis
                 */
                newcell = Clip_Move(cell, rot, dist);

                /**
                 *  If this is our second try at this distance, add a random scatter
                 *  to the desired cell, so our units aren't all aligned along spokes.
                 */
                if (tryval > 0) {
                    newcell = Clip_Scatter(newcell, 1);
                }

                /**
                 *  If, by randomly scattering, we've chosen the exact center, skip
                 *  it & try another direction.
                 */
                if (newcell == cell) {
                    skipit = true;
                }

                if (Map.In_Radar(newcell) && !skipit) {

                    /**
                     *  Only attempt to Unlimbo the object if:
                     *  - there is no techno in the cell
                     *  - the techno in the cell & the object are both infantry
                     */
                    techno = Map[newcell].Cell_Techno();
                    if (!techno || (techno->What_Am_I() == RTTI_INFANTRY &&
                        obj->What_Am_I() == RTTI_INFANTRY)) {
                        Coordinate coord = Cell_Coord(newcell, true);
                        coord.Z = Map.Get_Cell_Height(coord);
                        if (obj->Unlimbo(coord, DIR_N)) {
                            return true;
                        }
                    }
                }

                rot++;
                if (rot > FACING_NW) {
                    rot = FACING_N;
                }
            }
        }
    }

    return false;
}


/**
 *  Build a list of valid multiplayer starting waypoints.
 * 
 *  @author: CCHyper
 */
static DynamicVectorClass<Cell> Build_Starting_Waypoint_List(bool official)
{
    DynamicVectorClass<Cell> waypts;

    /**
     *  Find first valid?
     */
    int min_waypts = 0;
    for (int i = 0; i < WAYPT_COUNT; ++i) {
        if (Scen->Waypoint[min_waypts]) {
            break;
        }
    }

    /**
     *  Calculate the number of waypoints (as a minimum) that will be lifted from the
     *  mission file. Bias this number so that only the first 4 waypoints are used
     *  if there are 4 or fewer players. Unofficial maps will pick from all the
     *  available waypoints.
     */
    int look_for = std::max(min_waypts, Session.Players.Count()+Session.Options.AIPlayers);
    if (!official) {
        look_for = MAX_PLAYERS;
    }

    for (int waycount = 0; waycount < look_for; ++waycount) {
        if (Scen->Is_Valid_Waypoint(waycount)) {
            Cell waycell = Scen->Get_Waypoint_Location(waycount);
            waypts.Add(waycell);
            DEBUG_INFO("Multiplayer start waypoint found at cell %d,%d.\n", waycell.X, waycell.Y);
        }
    }

    /**
     *  If there are insufficient waypoints to account for all players, then randomly
     *  assign starting points until there is enough.
     */
    int deficiency = look_for - waypts.Count();
    if (deficiency > 0) {
        DEBUG_WARNING("Multiplayer start waypoint deficiency - looking for more start positions.\n");
        for (int index = 0; index < deficiency; ++index) {

            Cell trycell = XY_Cell(Map.MapCellX + Random_Pick(10, Map.MapCellWidth-10),
                                   Map.MapCellY + Random_Pick(0, Map.MapCellHeight-10) + 10);

            trycell = Map.Nearby_Location(trycell, SPEED_TRACK, -1, MZONE_NORMAL, false, 8, 8);
            if (trycell) {
                waypts.Add(trycell);
                DEBUG_INFO("Random multiplayer start waypoint added at cell %d,%d.\n", trycell.X, trycell.Y);
            }
        }
    }

    return waypts;
}


/**
 *  New implementation of Create_Units()
 * 
 *  @author: CCHyper (assistance from tomsons26).
 */
void Vinifera_Create_Units(bool official)
{
    DynamicVectorClass<TechnoClass *> deployed_objects;

    int tot_units = Session.Options.UnitCount;
    if (Session.Options.Bases) {
        --tot_units;
    }

    DEBUG_INFO("NumPlayers = %d\n", Session.NumPlayers);
    DEBUG_INFO("AIPlayers = %d\n", Session.Options.AIPlayers);
    DEBUG_INFO("Creating %d starting units per house - Random seed is %08x\n", tot_units, Scen->RandomNumber);
    DEBUG_INFO("UniqueID is %08x\n", Scen->UniqueID);

    Cell centroid;          // centroid of this house's stuff.
    TechnoClass *obj;       // newly-created object.

    /**
     *  Generate lists of all the available starting units (regardless of owner).
     */
    int tot_count = 0;
    int tot_cost = 0;
    int budget = 0;

    for (int i = 0; i < UnitTypes.Count(); ++i) {
        UnitTypeClass *unittype = UnitTypes[i];
        if (unittype && unittype->IsAllowedToStartInMultiplayer) {
            if (Rule->BaseUnit->Fetch_ID() != unittype->Fetch_ID()) {
                tot_cost += unittype->Raw_Cost();
                ++tot_count;
            }
        }
    }

    for (int i = 0; i < InfantryTypes.Count(); ++i) {
        InfantryTypeClass *infantrytype = InfantryTypes[i];
        if (infantrytype && infantrytype->IsAllowedToStartInMultiplayer) {
            tot_cost += infantrytype->Raw_Cost();
            ++tot_count;
        }
    }

    if (!tot_count) {
        DEBUG_WARNING("No starting units available!");
    }

    /**
     *  #BUGFIX:
     *  Check to prevent division by zero crash.
     */
    if (tot_cost != 0 && tot_count != 0) {
        budget = tot_units * (tot_cost / tot_count);
    }

    /**
     *  Build a list of the valid waypoints. This normally shouldn't be
     *  necessary because the scenario level designer should have assigned
     *  valid locations to the first N waypoints, but just in case, this
     *  loop verifies that.
     */
    const unsigned int MAX_STORED_WAYPOINTS = 26;

    bool taken[MAX_STORED_WAYPOINTS];
    std::memset(taken, '\0', sizeof(taken));

    DynamicVectorClass<Cell> waypts;
    waypts = Build_Starting_Waypoint_List(official);

    /**
     *  Loop through all houses.  Computer-controlled houses, with Session.Options.Bases
     *  ON, are treated as though bases are OFF (since we have no base-building AI logic.)
     */
    int numtaken = 0;
    for (HousesType house = HOUSE_FIRST; house < Houses.Count(); ++house) {

        /**
         *  Get a pointer to this house; if there is none, go to the next house.
         */
        HouseClass *hptr = Houses[house];
        if (hptr == nullptr) {
            DEV_DEBUG_INFO("Invalid house %d!\n", house);
            continue;
        }

        DynamicVectorClass<InfantryTypeClass *> available_infantry;
        DynamicVectorClass<UnitTypeClass *> available_units;

        /**
         *  Skip passive houses.
         */
        if (hptr->Class->IsMultiplayPassive) {
            DEV_DEBUG_INFO("House %d (%s - \"%s\") is passive, skipping.\n", house, hptr->Class->Name(), hptr->IniName);
            continue;
        }

        int owner_id = 1 << hptr->Class->ID;

        DEBUG_INFO("Generating units for house %d (Name: %s - \"%s\", Color: %s)...\n",
            house, hptr->Class->Name(), hptr->IniName, ColorSchemes[hptr->RemapColor]->Name);

        /**
         *  Generate list of starting units for this house.
         */
        DEBUG_INFO("  Creating list of available UnitTypes...\n");
        for (int i = 0; i < UnitTypes.Count(); ++i) {
            UnitTypeClass *unittype = UnitTypes[i];
            if (unittype) {

                /**
                 *  Is this unit allowed to be placed in multiplayer?
                 */
                if (!unittype->IsAllowedToStartInMultiplayer) {
                    continue;
                }

                /**
                 *  Check tech level and ownership.
                 */
                if (unittype->TechLevel <= hptr->Control.TechLevel && (owner_id & unittype->Ownable) != 0) {

                    if (Rule->BaseUnit->Fetch_ID() != unittype->Fetch_ID()) {
                        DEBUG_INFO("    Added %s\n", unittype->Name());
                        available_units.Add(unittype);
                    }
                }
            }
        }

        /**
         *  Generate list of starting infantry for this house.
         */
        DEBUG_INFO("  Creating list of available InfantryTypes...\n");
        for (int i = 0; i < InfantryTypes.Count(); ++i) {
            InfantryTypeClass *infantrytype = InfantryTypes[i];
            if (infantrytype) {

                /**
                 *  Is this unit allowed to be placed in multiplayer?
                 */
                if (!infantrytype->IsAllowedToStartInMultiplayer) {
                    continue;
                }

                /**
                 *  Check tech level and ownership.
                 */
                if (infantrytype->TechLevel <= hptr->Control.TechLevel && (owner_id & infantrytype->Ownable) != 0) {
                    available_infantry.Add(infantrytype);
                    DEBUG_INFO("    Added %s\n", infantrytype->Name());
                }
            }
        }

        /**
         *  Pick the starting location for this house. The first house just picks
         *  one of the valid locations at random. The other houses pick the furthest
         *  waypoint from the existing houses.
         */        
        if (numtaken == 0) {
            int pick = Random_Pick(0, waypts.Count()-1);
            centroid = waypts[pick];
            taken[pick] = true;
            numtaken++;

        } else {

            /**
             *  Set all waypoints to have a score of zero in preparation for giving
             *  a distance score to all waypoints.
             */
            int score[MAX_STORED_WAYPOINTS];
            std::memset(score, '\0', sizeof(score));

            /**
             *  Scan through all waypoints and give a score as a value of the sum
             *  of the distances from this waypoint to all taken waypoints.
             */
            for (int index = 0; index < waypts.Count(); index++) {

                /**
                 *  If this waypoint has not already been taken, then accumulate the
                 *  sum of the distance between this waypoint and all other taken
                 *  waypoints.
                 */
                if (!taken[index]) {
                    for (int trypoint = 0; trypoint < waypts.Count(); trypoint++) {

                        if (taken[trypoint]) {
                            score[index] += Distance(waypts[index], waypts[trypoint]);
                        }
                    }
                }
            }

            /**
             *  Now find the waypoint with the largest score. This waypoint is the one
             *  that is furthest from all other taken waypoints.
             */
            int best = 0;
            int bestvalue = 0;
            for (int searchindex = 0; searchindex < waypts.Count(); searchindex++) {
                if (score[searchindex] > bestvalue || bestvalue == 0) {
                    bestvalue = score[searchindex];
                    best = searchindex;
                }
            }

            /**
             *  Assign this best position to the house.
             */
            centroid = waypts[best];
            taken[best] = true;
            numtaken++;
        }

        /**
         *  Assign the center of this house to the waypoint location.
         */
        hptr->Center = Cell_Coord(centroid, true);
        DEBUG_INFO("  Setting house center to %d,%d\n", centroid.X, centroid.Y);

        /**
         *  If Bases are ON, place a base unit (MCV).
         */
        if (Session.Options.Bases) {

            /**
             *  #issue-206
             * 
             *  Adds game option to allow construction yards to be placed on the
             *  map at game start instead of an MCV.
             * 
             *  @author: CCHyper
             */
            if (SessionExtension && SessionExtension->ExtOptions.IsPrePlacedConYards) {

                /**
                 *  Create a construction yard (decided from the base unit).
                 */
                obj = new BuildingClass(Rule->BaseUnit->DeploysInto, hptr);
                if (obj->Unlimbo(Cell_Coord(centroid, true), DIR_N) || Scan_Place_Object(obj, centroid)) {
                    if (obj != nullptr) {
                        DEBUG_INFO("  Construction yard %s placed at %d,%d.\n",
                            obj->Class_Of()->Name(), obj->Get_Cell().X, obj->Get_Cell().Y);

                        BuildingClass *building = reinterpret_cast<BuildingClass *>(obj);

                        /**
                         *  Always reveal the construction yard to the player
                         *  that owns it.
                         */
                        building->Revealed(obj->House);
                        building->IsReadyToCommence = true;

                        /**
                         *  Always consider production to have started for the
                         *  owning house. This ensures that in multiplay, computer
                         *  opponents will begin construction as soon as they start
                         *  their base.
                         */
                        if (Session.Type != GAME_NORMAL) {

                            if (!building->House->Is_Player_Control()) {

                                building->IsToRebuild = true;
                                building->IsToRepair = true;

                                if (building->Class->IsConstructionYard) {

                                    Cell cell = Coord_Cell(building->Coord);

                                    building->House->Begin_Construction();

                                    building->House->Base.Nodes[0].Where = cell;
                                    building->House->Base.field_50 = cell;

                                    building->House->IsStarted = true;
                                    building->House->field_C8 = true;
                                    building->House->IsBaseBuilding = true;
                                }
                            }
                        }
                    }
                    hptr->FlagHome = Cell(0,0);
                    hptr->FlagLocation = nullptr;
                }

            } else {

                /**
                 *  For a human-controlled house:
                 *    - Create an MCV
                 *    - Attach a flag to it for capture-the-flag mode.
                 */
                obj = new UnitClass(Rule->BaseUnit, hptr);
                if (obj->Unlimbo(Cell_Coord(centroid, true), DIR_N) || Scan_Place_Object(obj, centroid)) {
                    if (obj != nullptr) {
                        DEBUG_INFO("  Base unit %s placed at %d,%d.\n",
                            obj->Class_Of()->Name(), obj->Get_Cell().X, obj->Get_Cell().Y);
                        hptr->FlagHome = Cell(0,0);
                        hptr->FlagLocation = nullptr;
                        if (Special.IsCaptureTheFlag) {
                            hptr->Flag_Attach((UnitClass *)obj, true);
                        }

                        /**
                         *  #issue-206
                         * 
                         *  Adds game option to allow MCV's to auto-deploy on game start.
                         * 
                         *  @author: CCHyper
                         */
                        if (Session.Options.UnitCount == 1) {
                            if (SessionExtension && SessionExtension->ExtOptions.IsAutoDeployMCV) {
                                if (hptr->Is_Human_Control()) {
                                    obj->Set_Mission(MISSION_UNLOAD);
                                }
                            }
                        }
                    }

                } else if (obj) {
                    delete obj;
                    obj = nullptr;
                }

            }
        }

        /**
         *  Clear the previous house's deployed list.
         */
        deployed_objects.Clear();

        //DEBUG_INFO("  budget == %d\n", budget);

        if (budget > 0) {

            /**
             *  Calculate the cost cap for units.
             */
            int unit_cost_cap = 2 * budget / 3;

            /**
             *  Place starting units for this house.
             */
            int i = 0;
            while (i < budget) {

                TechnoTypeClass *technotype = nullptr;

                if (i < unit_cost_cap && available_units.Count() > 0) {
                    technotype = available_units[Random_Pick(0, available_units.Count()-1)];
                } else if (available_infantry.Count() > 0) {
                    technotype = available_infantry[Random_Pick(0, available_infantry.Count()-1)];
                }

                if (!technotype) {
                    DEBUG_WARNING("  Invalid techno pointer!\n");
                    continue;
                }

                /**
                 *  Create an instance of the unit.
                 */
                obj = reinterpret_cast<TechnoClass *>(technotype->Create_One_Of(hptr));
                if (obj) {

                    if (Scan_Place_Object(obj, centroid)) {

                        DEBUG_INFO("  House %s deployed object %s at %d,%d\n",
                            hptr->Class->Name(), obj->Name(), obj->Get_Cell().X, obj->Get_Cell().Y);

                        i += technotype->Raw_Cost();

                        if (Scen->SpecialFlags.InitialVeteran) {
                            obj->Veterancy.Set_Elite(true);
                        }

                        if (hptr->Is_Human_Control()) {
                            obj->Set_Mission(MISSION_GUARD);
                        } else {
                            obj->Set_Mission(MISSION_GUARD_AREA);
                        }

                        /**
                         *  Add to the list of objects successfully deployed.
                         */
                        deployed_objects.Add(obj);

                    } else if (obj) {
                        delete obj;
                    }

                }

                /**
                 *  Scatter all the human placed objects to create
                 *  some space around the base unit.
                 */
                if (hptr->Is_Human_Control()) {
                    for (int i = 0; i < deployed_objects.Count(); ++i) {
                        TechnoClass *techno = deployed_objects[i];
                        if (techno) {
                            techno->Scatter();
                        }
                    }
                }

            }

        }
    }

    DEBUG_INFO("Finished unit generation. Random number is %d\n", Scen->RandomNumber);
}

