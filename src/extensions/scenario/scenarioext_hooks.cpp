/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SCENARIOEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended ScenarioClass.
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
#include "scenarioext_hooks.h"
#include "scenarioext_init.h"
#include "scenarioext.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "house.h"
#include "unit.h"
#include "unittype.h"
#include "unittypeext.h"
#include "rules.h"
#include "rulesext.h"
#include "multiscore.h"
#include "scenario.h"
#include "session.h"
#include "rules.h"
#include "ccfile.h"
#include "ccini.h"
#include "endgame.h"
#include "addon.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "mouse.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class ScenarioClassExt final : public ScenarioClass
{
    public:
        Cell _Get_Waypoint_Cell(WaypointType wp) const { return ScenExtension->Get_Waypoint_Cell(wp); }
        CellClass *_Get_Waypoint_CellPtr(WaypointType wp) const { return ScenExtension->Get_Waypoint_CellPtr(wp); }
        Coordinate _Get_Waypoint_Coord(WaypointType wp) const { return ScenExtension->Get_Waypoint_Coord(wp); }
        Coordinate _Get_Waypoint_Coord_Height(WaypointType wp) const { return ScenExtension->Get_Waypoint_Coord_Height(wp); }

        void _Set_Waypoint_Cell(WaypointType wp, Cell cell) { ScenExtension->Set_Waypoint_Cell(wp, cell); }
        void _Set_Waypoint_Coord(WaypointType wp, Coordinate &coord) { ScenExtension->Set_Waypoint_Coord(wp, coord); }

        bool _Is_Valid_Waypoint(WaypointType wp) const { return ScenExtension->Is_Valid_Waypoint(wp); }
        void _Clear_Waypoint(WaypointType wp) { ScenExtension->Clear_Waypoint(wp); }

        void _Clear_All_Waypoints() { ScenExtension->Clear_All_Waypoints(); }

        void _Read_Waypoint_INI(CCINIClass &ini) { ScenExtension->Read_Waypoint_INI(ini); }
        void _Write_Waypoint_INI(CCINIClass &ini) { ScenExtension->Write_Waypoint_INI(ini); }

        const char *_Waypoint_As_String(WaypointType wp) const { return ScenExtension->Waypoint_As_String(wp); }
};


/**
 *  #issue-71
 * 
 *  Clear all waypoints in preperation for loading the scenario data.
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_Clear_Scenario_Clear_Waypoints_Patch)
{
    /**
     *  Stolen bytes/code.
     */
    _asm { add esp, 0x4 } // Fixes up the stack from the WWDebugPrintf call.

    //DEBUG_INFO("Clearing waypoints...\n");
    ScenExtension->Clear_All_Waypoints();

    JMP(0x005DC872);
}


/**
 *  #issue-71
 *
 *  Reimplements a part of the Fill_In_Data function to set the view to the HomeCell.
 *
 *  @author: ZivDero
 */
void Init_Home_Cell()
{
    Map.SidebarClass::Activate(1);
    if (Session.Type == GAME_NORMAL)
    {
        int home_cell_number = EndGame.Globals[0] ? Scen->AltHomeCell : Scen->HomeCell;
        Cell home_cell = ScenExtension->Waypoint[home_cell_number];

        Scen->Views[0] = home_cell;
        Scen->Views[1] = Scen->Views[0];
        Scen->Views[2] = Scen->Views[1];
        Scen->Views[3] = Scen->Views[2];

        Coordinate home_coord = Cell_Coord(home_cell);
        home_coord.Z = Map.Get_Cell_Height(home_coord);

        Map.RadarClass::Set_Tactical_Position(home_coord);
    }
}


/**
 *  #issue-71
 *
 *  Assign the home cell waypoint.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_Fill_In_Data_Home_Cell_Patch)
{
    Init_Home_Cell();

    JMP(0x005DC166);
}


/**
 *  #issue-71
 *
 *  Replace waypoint number to string conversion.
 *
 *  @author: secsome, ZivDero
 */
const char* _Waypoint_To_Name(int wp)
{
    enum { CHAR_COUNT = 26 };

    static char buffer[8]{ '\0' };

    if (wp < 0)
        return buffer;

    ++wp;
    int pos = 7;

    while (wp > 0)
    {
        --pos;
        char m = wp % CHAR_COUNT;
        if (m == 0) m = CHAR_COUNT;
        buffer[pos] = m + '@'; // '@' = 'A' - 1
        wp = (wp - m) / CHAR_COUNT;
    }

    return buffer + pos;
}


/**
 *  #issue-71
 *
 *  Replace waypoint string to number conversion.
 *
 *  @author: secsome, ZivDero
 */
int _Waypoint_From_Name(char* wp)
{
    enum { CHAR_COUNT = 26 };

    int n = 0;
    int len = strlen(wp);

    for (int i = len - 1, j = 1; i >= 0; i--, j *= CHAR_COUNT)
    {
        int c = toupper(wp[i]);
        if (c < 'A' || c > 'Z')
            return WAYPOINT_NONE;

        n += (c - '@') * j; // '@' = 'A' - 1
    }

    return n - 1;
}


/**
 *  Wrapper function for creating a UnitClass instance.
 * 
 *  @author: CCHyper
 */
static UnitClass *New_UnitClass(UnitTypeClass *classof, HouseClass *house)
{
    return new UnitClass(classof, house);
}


/**
 *  #issue-177
 * 
 *  Various patches to add support for more than one entry defined on BaseUnit.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Create_Units_Calculate_Total_Cost_Of_Units_Patch)
{
    GET_REGISTER_STATIC(UnitTypeClass *, current_unittype, esi);
    GET_REGISTER_STATIC(int, tot_cost, ebp);
    GET_REGISTER_STATIC(int, tot_count, ebx);
    static UnitTypeClass *unittype;
    static int i;

    unittype = nullptr;

    /**
     *  Fetch the extended rules class instance and make sure value
     *  entries have been loaded.
     */
    if (RuleExtension && RuleExtension->BaseUnit.Count() > 0) {

        /**
         *  Iterate over all defined BaseUnits, if any match the unit
         *  we are currently processing then skip it.
         */
        for (i = 0; i < RuleExtension->BaseUnit.Count(); ++i) {
            if (RuleExtension->BaseUnit[i] == current_unittype) {
                goto skip_unit;
            }
        }

    /**
     *  Fallback to the original code.
     */
    } else if (Rule->BaseUnit == current_unittype) {
        goto skip_unit;
    }

    DEBUG_INFO("Calc: %s\n", current_unittype->Name());

    /**
     *  Add the unit to the available units vector.
     */
add_cost:
    _asm { mov esi, current_unittype }
    _asm { mov ebp, tot_cost }
    _asm { mov ebx, tot_count }
    JMP_REG(edx, 0x005DE663);

    /**
     *  Don't add the unit to the vector.
     */
skip_unit:
    JMP(0x005DE670);
}

DECLARE_PATCH(_Create_Units_Populate_Available_Units_Vector_Patch)
{
    GET_REGISTER_STATIC(UnitTypeClass *, current_unittype, esi);
    static UnitTypeClass *unittype;
    static int i;

    unittype = nullptr;

    /**
     *  Fetch the extended rules class instance and make sure value
     *  entries have been loaded.
     */
    if (RuleExtension && RuleExtension->BaseUnit.Count() > 0) {

        /**
         *  Iterate over all defined BaseUnits, if any match the unit
         *  we are currently processing then skip it.
         */
        for (i = 0; i < RuleExtension->BaseUnit.Count(); ++i) {
            if (RuleExtension->BaseUnit[i] == current_unittype) {
                goto dont_add_to_vector;
            }
        }

    /**
     *  Fallback to the original code.
     */
    } else if (Rule->BaseUnit == current_unittype) {
        goto dont_add_to_vector;
    }

    DEBUG_INFO("Pop: %s\n", current_unittype->Name());

    /**
     *  Add the unit to the available units vector.
     */
add_to_vector:
    _asm { mov esi, current_unittype }
    JMP(0x005DE9DC);

    /**
     *  Don't add the unit to the vector.
     */
dont_add_to_vector:
    JMP(0x005DEA22);
}

DECLARE_PATCH(_Create_Units_Place_BaseUnit_Patch)
{
    GET_REGISTER_STATIC(HouseClass *, hptr, edi);
    static UnitClass *unit;
    static UnitTypeClass *unittype;

    unit = nullptr;

    /**
     *  Fetch the extended rules class instance and make sure value
     *  entries have been loaded.
     */
    if (RuleExtension && RuleExtension->BaseUnit.Count() > 0) {

        /**
         *  Fetch the first buildable base unit from the new base unit entry
         *  and get the current count of that unit that this can house own.
         */
        unittype = hptr->Get_First_Ownable(RuleExtension->BaseUnit);

    /**
     *  Fallback to the original code.
     */
    } else {
        unittype = Rule->BaseUnit;
    }

    if (unittype) {
        unit = New_UnitClass(unittype, hptr);
    } else {
        DEBUG_WARNING("Invalid or no available BaseUnit found in Create_Units()!\n");
        goto failed_to_create;
    }

    /**
     *  Unlimbo the base unit.
     */
unlimbo_baseunit:
    _asm { mov esi, unit }
    JMP(0x005DEC33);

    /**
     *  Failed to create the unit instance.
     */
failed_to_create:
    JMP(0x005DECC2);
}


/**
 *  Process additions to the Rules data from the input file.
 * 
 *  @author: CCHyper
 */
static bool Rule_Addition(const char *fname, bool with_digest = false)
{
    CCFileClass file(fname);
    if (!file.Is_Available()) {
        return false;
    }

    CCINIClass ini;
    if (!ini.Load(file, with_digest)) {
        return false;
    }

    DEBUG_INFO("Calling Rule->Addition() with \"%s\" overrides.\n", fname);

    Rule->Addition(ini);

    return true;
}


/**
 *  #issue-#671
 * 
 *  Add loading of MPLAYER.INI to override Rules data for multiplayer games.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Read_Scenario_INI_MPlayer_INI_Patch)
{
    if (Session.Type != GAME_NORMAL && Session.Type != GAME_WDT) {

        /**
         *  Process the multiplayer ini overrides.
         */
        Rule_Addition("MPLAYER.INI");
        if (Is_Addon_Enabled(ADDON_FIRESTORM)) { 
            Rule_Addition("MPLAYERFS.INI");
        }

    }

    /**
     *  Update the progress screen bars.
     */
    Session.Loading_Callback(42);

    /**
     *  Stolen bytes/code.
     */
    Call_Back();

    JMP(0x005DD8DA);
}


/**
 *  #issue-522
 * 
 *  These patches make the multiplayer score screen to honour the value of
 *  "IsSkipScore" from ScenarioClass.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Do_Win_Skip_MPlayer_Score_Screen_Patch)
{
    /**
     *  Stolen bytes/code.
     */
    ++Session.GamesPlayed;

    if (!Scen->IsSkipScore) {
        MultiScore::Presentation();
    }

    JMP(0x005DC9DF);
}

DECLARE_PATCH(_Do_Lose_Skip_MPlayer_Score_Screen_Patch)
{
    /**
     *  Stolen bytes/code.
     */
    ++Session.GamesPlayed;

    if (!Scen->IsSkipScore) {
        MultiScore::Presentation();
    }

    JMP(0x005DCD9D);
}


/**
 *  Main function for patching the hooks.
 */
void ScenarioClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    ScenarioClassExtension_Init();

    /**
     *  Hooks in the new Assign_Houses() function.
     * 
     *  @author: CCHyper
     */
    Patch_Call(0x005E08E3, &ScenarioClassExtension::Assign_Houses);

    /**
     *  #issue-338
     * 
     *  Hooks in the new Create_Units() function.
     * 
     *  @author: CCHyper
     */
    Patch_Call(0x005DD320, &ScenarioClassExtension::Create_Units);

    Patch_Jump(0x005DC9D4, &_Do_Win_Skip_MPlayer_Score_Screen_Patch);
    Patch_Jump(0x005DCD92, &_Do_Lose_Skip_MPlayer_Score_Screen_Patch);
    Patch_Jump(0x005DD8D5, &_Read_Scenario_INI_MPlayer_INI_Patch);

    /**
     *  #issue-71
     *
     *  Increases the amount of available waypoints (see ScenarioClassExtension for implementation).
     *
     *  @author: CCHyper, ZivDero
     */
    Patch_Jump(0x005E1460, &ScenarioClassExt::_Get_Waypoint_Cell);
    Patch_Jump(0x005E1480, &ScenarioClassExt::_Get_Waypoint_CellPtr);
    Patch_Jump(0x005E14A0, &ScenarioClassExt::_Get_Waypoint_Coord);
    Patch_Jump(0x005E1500, &ScenarioClassExt::_Clear_All_Waypoints);
    Patch_Jump(0x005E1520, &ScenarioClassExt::_Is_Valid_Waypoint);
    Patch_Jump(0x005E1560, &ScenarioClassExt::_Read_Waypoint_INI);
    Patch_Jump(0x005E1630, &ScenarioClassExt::_Write_Waypoint_INI);
    Patch_Jump(0x005E16C0, &ScenarioClassExt::_Clear_Waypoint);
    Patch_Jump(0x005E16E0, &ScenarioClassExt::_Set_Waypoint_Cell);
    Patch_Jump(0x005E1700, &ScenarioClassExt::_Get_Waypoint_CellPtr);
    Patch_Jump(0x005E1720, &ScenarioClassExt::_Waypoint_As_String);
    Patch_Jump(0x005DC852, &_Clear_Scenario_Clear_Waypoints_Patch);
    Patch_Jump(0x005DC0A0, &_Fill_In_Data_Home_Cell_Patch);
    Patch_Jump(0x00673330, &_Waypoint_From_Name);
    Patch_Jump(0x006732B0, &_Waypoint_To_Name);
    // 0047A96C

    Patch_Jump(0x005DE637, &_Create_Units_Calculate_Total_Cost_Of_Units_Patch);
    Patch_Jump(0x005DE9AF, &_Create_Units_Populate_Available_Units_Vector_Patch);
    Patch_Jump(0x005DEC07, &_Create_Units_Place_BaseUnit_Patch);
}
