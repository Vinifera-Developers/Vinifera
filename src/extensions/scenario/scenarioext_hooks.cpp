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
#include "addon.h"
#include "aircrafttracker.h"
#include "asserthandler.h"
#include "buildingtype.h"
#include "campaign.h"
#include "campaignext.h"
#include "ccfile.h"
#include "ccini.h"
#include "debughandler.h"
#include "environment.h"
#include "environmentext_hooks.h"
#include "fatal.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "housetype.h"
#include "housetypeext.h"
#include "kamikazetracker.h"
#include "language.h"
#include "mouse.h"
#include "multiscore.h"
#include "progressscreen.h"
#include "reinf.h"
#include "rules.h"
#include "rulesext.h"
#include "scenario.h"
#include "scenarioext.h"
#include "scenarioext_init.h"
#include "session.h"
#include "sessionext.h"
#include "spawner.h"
#include "teamtype.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "uicontrol.h"
#include "unit.h"
#include "vinifera_globals.h"
#include "wsproto.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class ScenarioClassExt : public ScenarioClass
{
public:
    Cell _Waypoint_Cell(WAYPOINT wp) const { return ScenExtension->Waypoint_Cell(wp); }
    CellClass *_Waypoint_CellClass(WAYPOINT wp) const { return ScenExtension->Waypoint_CellClass(wp); }
    Coord _Waypoint_Coord(WAYPOINT wp) const { return ScenExtension->Waypoint_Coord(wp); }

    void _Set_Waypoint_Cell(WAYPOINT wp, Cell cell) { ScenExtension->Set_Waypoint_Cell(wp, cell); }
    void _Set_Waypoint_Coord(WAYPOINT wp, Coord &coord) { ScenExtension->Set_Waypoint_Coord(wp, coord); }

    bool _Is_Waypoint_Valid(WAYPOINT wp) const { return ScenExtension->Is_Waypoint_Valid(wp); }
    void _Clear_Waypoint(WAYPOINT wp) { ScenExtension->Clear_Waypoint(wp); }

    void _Clear_All_Waypoints() { ScenExtension->Clear_All_Waypoints(); }

    void _Read_Waypoint_INI(CCINIClass &ini) { ScenExtension->Read_Waypoint_INI(ini); }
    void _Write_Waypoint_INI(CCINIClass &ini) { ScenExtension->Write_Waypoint_INI(ini); }

    const char *_Waypoint_As_String(WAYPOINT wp) const { return ScenExtension->Waypoint_As_String(wp); }

    void _Read_Global_INI(INIClass& ini) { ScenExtension->Read_Global_INI(ini); }
    void _Read_Local_INI(INIClass& ini) { ScenExtension->Read_Local_INI(ini); }
    void _Write_Local_INI(INIClass& ini) { ScenExtension->Write_Local_INI(ini); }

    bool _Set_Global_To(int global, bool value) { return ScenExtension->Set_Global_To(global, value ? 1 : 0) != 0; }
    bool _Set_Global_To(const char* name, bool value) { return ScenExtension->Set_Global_To(name, value ? 1 : 0) != 0; }

    bool _Get_Global_Value(int global, bool& value)
    {
        int raw;
        if (ScenExtension->Get_Global_Value(global, raw)) {
            value = (raw != 0);
            return true;
        }
        return false;
    }

    bool _Get_Global_Value(const char* name, bool& value)
    {
        int raw;
        if (ScenExtension->Get_Global_Value(name, raw)) {
            value = (raw != 0);
            return true;
        }
        return false;
    }

    bool _Set_Local_To(int local, bool value) { return ScenExtension->Set_Local_To(local, value ? 1 : 0) != 0; }
    bool _Set_Local_To(const char* name, bool value) { return ScenExtension->Set_Local_To(name, value ? 1 : 0) != 0; }

    bool _Get_Local_Value(int local, bool& value)
    {
        int raw;
        if (ScenExtension->Get_Local_Value(local, raw)) {
            value = (raw != 0);
            return true;
        }
        return false;
    }

    bool _Get_Local_Value(const char* name, bool& value)
    {
        int raw;
        if (ScenExtension->Get_Local_Value(name, raw)) {
            value = (raw != 0);
            return true;
        }
        return false;
    }

    int _Find_Global_Variable_Index(const char* name) { return ScenExtension->Find_Global_Variable_Index(name); }
    int _Find_Local_Variable_Index(const char* name) { return ScenExtension->Find_Local_Variable_Index(name); }

    int _Find_Free_Local() { return ScenExtension->Find_Free_Local(); }
    int _Num_Locals() { return ScenExtension->Num_Locals(); }
};


/**
 *  #issue-71
 * 
 *  Clear things in preparation for loading the scenario data.
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_Clear_Scenario_Patch)
{
    /**
     *  Stolen bytes/code.
     */
    _asm { add esp, 0x4 } // Fixes up the stack from the WWDebugPrintf call.

    //DEBUG_INFO("Clearing waypoints...\n");
    ScenExtension->Clear_All_Waypoints();

    KamikazeTracker->Clear();
    AircraftTracker->Clear();

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
        int home_cell_number = EnvironmentGlobals[0] ? Scen->AltHome : Scen->Home;
        Cell home_cell = ScenExtension->Waypoint[home_cell_number];

        Scen->Views[0] = home_cell;
        Scen->Views[1] = Scen->Views[0];
        Scen->Views[2] = Scen->Views[1];
        Scen->Views[3] = Scen->Views[2];

        Coord home_coord = home_cell.As_Coord();
        home_coord.Z = Map.Get_Height_GL(home_coord);

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
 *  Reimplements the loading screen setup routine.
 * 
 *  @author: CCHyper, ZivDero
 */
static void Init_Loading_Screen(const char* filename)
{
    /**
     *  For the campaign, we check to see if the scenario name contains either
     *  "GDI" or "NOD", and then set the side to those respectively.
     */
    HousesType house = HOUSE_GDI;
    if (Session.Type == GAME_NORMAL) {
        if (Scen->Campaign != CAMPAIGN_NONE) {
            const auto campaign_ext = Extension::Fetch(Campaigns[Scen->Campaign]);
            house = campaign_ext->House;
        }
    } else {

        /**
         *  The first player in the player array is always the local player, so
         *  fetch our player info and the house we are assigned as.
         */

        HouseTypeClass* housetype = HouseTypes[Session.Players.Fetch_Head()->Player.House];
        house = housetype->House;

        /**
         *  Set the player's side. This would happen in Select_Game, but we
         *  do it here for the spawner, and to take advantage of fixups.
         */
        SessionExtension->House = housetype->HeapID;
    }

    /**
     *  Sanity check the side type.
     */
    if (house == HOUSE_NONE || house >= HouseTypes.Count()) {
        house = HOUSE_GDI;
    }

    /**
     *  Fetch the loading screen properties for the user's screen size.
     */
    auto loading_screen = UIControls->Pick_Loading_Screen(house);

    /**
     *  Fetch the loading screen override from the scenario.
     */
    const auto ls_override = ScenExtension->Pick_Loading_Screen_Override(house);
    if (ls_override != nullptr) {
        loading_screen = *ls_override;
    }

    /**
     *  
     */
    const char* loadname = loading_screen.Filename.c_str();
    Point2D bar_pos = Session.Singleplayer_Game() ? loading_screen.Size.SPPosition : loading_screen.Size.MPPosition;

    char loadfilename[PATH_MAX];
    std::snprintf(loadfilename, sizeof(loadfilename), "%s.PCX", loadname);

    /**
     *  The spawner can forcibly override the loading screen, and it already includes .PCX.
     */
    if (Vinifera_SpawnerConfig != nullptr) {

        if (Wstring(Vinifera_SpawnerConfig->CustomLoadScreen).Is_Not_Empty()) {
            std::snprintf(loadfilename, sizeof(loadfilename), "%s", Vinifera_SpawnerConfig->CustomLoadScreen);

            if (Vinifera_SpawnerConfig->CustomLoadScreenPos != Point2D(0, 0)) {
                bar_pos = Vinifera_SpawnerConfig->CustomLoadScreenPos;
            }
        }
    }

    /**
     *  Adjust the position of the text and bars so it is correct for widescreen resolutions.
     */
    bar_pos.X += (VisibleRect.Width - loading_screen.Size.Size.X) / 2;
    bar_pos.Y += (VisibleRect.Height - loading_screen.Size.Size.Y) / 2;

    DEV_DEBUG_INFO("Loading Screen: \"%s\"\n", loadfilename);

    /**
     *  If this is a tournament game, format the game id.
     */
    char gamenamebuffer[128];
    const char* prog_msg = nullptr;

    if (Session.Type == GAME_INTERNET && PlanetWestwoodTournament == WOL::TOURNAMENT_0) {
        std::snprintf(gamenamebuffer, sizeof(gamenamebuffer), Text_String(TXT_GAME_ID), PlanetWestwoodGameID);
        prog_msg = gamenamebuffer;
    }

    /**
     *  Select the progress bar graphic depending on the game mode.
     */
    const int player_count = Session.Singleplayer_Game() ? 1 : Session.Players.Count();
    const char* progress_name = player_count <= 1 ? "PROGBAR.SHP" : "PROGBARM.SHP";

    /**
     *  Initialise the loading screen.
     */
    Progress.Initialize(100, player_count);

    /**
     *  Forces the initial draw, Call_Back calls will update the redraw from here on.
     */
    Progress.Set_Graphic_Data(progress_name, loadfilename, prog_msg, bar_pos);
    Progress.Display_Progress();
}


/**
 *  Patch to intercept and replace the loading screen setup.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Read_Scenario_Loading_Screen_Patch)
{
    LEA_STACK_STATIC(const char *, filename, esp, 0x50);

    ScenExtension->Read_Loading_Screen_INI(filename);

    Init_Loading_Screen(filename);

    /**
     *  Jump to setting broadcast addresses.
     */
    JMP(0x005DBD4A);
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
 *  Patch the check for if ALtScenario should be started
 *  to use the extended global variables.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_ScenarioClass_Do_Win_GlobalFlags_Patch)
{
    _asm pushad

    if (ScenExtension->GlobalFlags[1].Value) {

        /**
         *  Proceed to AltNextScenario.
         */
        _asm popad
        JMP_REG(edx, 0x005DCB63);
    } else {

        /**
         *  Proceed to NextScenario.
         */
        _asm popad
        JMP_REG(edx, 0x005DCB72);
    }
}


/**
 *  Replace a loop resetting all globals in Clear_Scenario.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_Clear_Scenario_Clear_Globals_Patch)
{
    static int i;
    for (i = 0; i < std::size(ScenExtension->GlobalFlags); i++) {
        ScenExtension->Set_Global_To(i, 0);
    }
    JMP(0x005DC688);
}


/**
 *  Special unit version of House_Or_Spawn_House_From_Name that adds a
 *  null pointer to the unit vector if the house is not found.
 *
 *  @author: ZivDero
 */
HousesType House_From_Name_Unit(const char* name)
{
    /**
     *  If we couldn't find the spawn house, add a null pointer to the unit vector
     *  so that the "LinkedTo" numbers don't break. We'll remove these null pointers
     *  at the end.
     */
    HousesType house = HouseTypeClass::From_Name(name);
    if (house == HOUSE_NONE) {
        Units.Add(nullptr);
    }

    return house;
}


/**
 *  Patch to fetch the spawn house for infantry during initial placement.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_InfantryClass_Read_INI_SpawnHouses_Patch)
{
    GET_REGISTER_STATIC(char*, house_name, eax);

    static HousesType house;
    static HouseClass* hptr;

    house = HouseTypeClass::From_Name(house_name);

    if (house != HOUSE_NONE) {
        hptr = House_From_HousesType(house);

        if (hptr) {
            _asm mov edi, hptr
            JMP(0x004D7BD5);
        }
    }

    JMP(0x004D7F30);
}


/**
 *  Link units to their followers.
 *
 *  @author: ZivDero
 */
static void Link_Units(DynamicVectorClass<int>& link_vector)
{
    /**
     *  Links the followed and followed units, checking to make sure both actually exist.
     */
    for (int i = 0; i < Units.Count(); ++i) {
        int follower_id = link_vector[i];
        UnitClass* unit = Units[i];

        if (unit) {

            if (follower_id != -1 && follower_id < Units.Count() && Units[follower_id]) {
                UnitClass* follower = Units[follower_id];
                unit->FollowingMe = follower;
                follower->IsFollowing = true;
            } else {
                unit->FollowingMe = nullptr;
            }
        }
    }

    /**
     *  We need to remove the null pointers we added from the unit vector.
     */
    for (int i = 0; i < Units.Count(); i++) {
        if (!Units[i]) {
            Units.Delete(i--);
        }
    }
}


/**
 *  Patch to link follower and followed units.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_UnitClass_Read_INI_Link_Units)
{
    LEA_STACK_STATIC(DynamicVectorClass<int>*, link_vector, esp, 0xC);

    Link_Units(*link_vector);

    JMP(0x00658A10);
}


/**
 *  A wrapper for Do_Reinforcements that checks if the team has a house.
 *
 *  @author: ZivDero
 */
bool Do_Reinforcements_Wrapper(const TeamTypeClass* team, WaypointType wp = WAYPOINT_NONE)
{
    /**
     *  Since not all spawn houses are present, some teams may have null houses. Don't spawn these teams.
     */
    if (team->House) {
        return Do_Reinforcements(team, wp);
    }

    return false;
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
     *  Hooks for new scanario-related functions.
     *
     *  @author: CCHyper, ZivDero
     */
    Patch_Jump(0x005DB170, &ScenarioClassExtension::Start_Scenario);
    Patch_Jump(0x005DD4C0, &ScenarioClassExtension::Read_Scenario_INI);
    Patch_Jump(0x005DE210, &ScenarioClassExtension::Assign_Houses);
    Patch_Jump(0x005DE580, &ScenarioClassExtension::Create_Units);

    Patch_Jump(0x005DC9D4, &_Do_Win_Skip_MPlayer_Score_Screen_Patch);
    Patch_Jump(0x005DCD92, &_Do_Lose_Skip_MPlayer_Score_Screen_Patch);

    /**
     *  #issue-71
     *
     *  Increases the amount of available waypoints (see ScenarioClassExtension for implementation).
     *
     *  @author: CCHyper, ZivDero
     */
    Patch_Jump(0x005E1460, &ScenarioClassExt::_Waypoint_Cell);
    Patch_Jump(0x005E1480, &ScenarioClassExt::_Waypoint_CellClass);
    Patch_Jump(0x005E14A0, &ScenarioClassExt::_Waypoint_Coord);
    Patch_Jump(0x005E1500, &ScenarioClassExt::_Clear_All_Waypoints);
    Patch_Jump(0x005E1520, &ScenarioClassExt::_Is_Waypoint_Valid);
    Patch_Jump(0x005E1560, &ScenarioClassExt::_Read_Waypoint_INI);
    Patch_Jump(0x005E1630, &ScenarioClassExt::_Write_Waypoint_INI);
    Patch_Jump(0x005E16C0, &ScenarioClassExt::_Clear_Waypoint);
    Patch_Jump(0x005E16E0, &ScenarioClassExt::_Set_Waypoint_Cell);
    Patch_Jump(0x005E1700, &ScenarioClassExt::_Waypoint_CellClass);
    Patch_Jump(0x005E1720, &ScenarioClassExt::_Waypoint_As_String);
    Patch_Jump(0x005DC852, &_Clear_Scenario_Patch);
    Patch_Jump(0x005DC0A0, &_Fill_In_Data_Home_Cell_Patch);
    Patch_Jump(0x00673330, &_Waypoint_From_Name);
    Patch_Jump(0x006732B0, &_Waypoint_To_Name);
    Patch_Jump(0x005DF930, &ScenarioClassExt::_Read_Global_INI);
    Patch_Jump(0x005DFBD0, &ScenarioClassExt::_Read_Local_INI);
    Patch_Jump(0x005DFD10, &ScenarioClassExt::_Write_Local_INI);
    Patch_Jump(0x005DF720, static_cast<bool (ScenarioClassExt::*)(int, bool)>(&ScenarioClassExt::_Set_Global_To));
    Patch_Jump(0x005DF770, static_cast<bool (ScenarioClassExt::*)(const char*, bool)>(&ScenarioClassExt::_Set_Global_To));
    Patch_Jump(0x005DF810, static_cast<bool (ScenarioClassExt::*)(int, bool&)>(&ScenarioClassExt::_Get_Global_Value));
    Patch_Jump(0x005DF840, static_cast<bool (ScenarioClassExt::*)(const char*, bool&)>(&ScenarioClassExt::_Get_Global_Value));
    Patch_Jump(0x005DF9C0, static_cast<bool (ScenarioClassExt::*)(int, bool)>(&ScenarioClassExt::_Set_Local_To));
    Patch_Jump(0x005DFA10, static_cast<bool (ScenarioClassExt::*)(const char*, bool)>(&ScenarioClassExt::_Set_Local_To));
    Patch_Jump(0x005DFAB0, static_cast<bool (ScenarioClassExt::*)(int, bool&)>(&ScenarioClassExt::_Get_Local_Value));
    Patch_Jump(0x005DFAE0, static_cast<bool (ScenarioClassExt::*)(const char*, bool&)>(&ScenarioClassExt::_Get_Local_Value));
    Patch_Jump(0x005DF8D0, &ScenarioClassExt::_Find_Global_Variable_Index);
    Patch_Jump(0x005DFB70, &ScenarioClassExt::_Find_Local_Variable_Index);
    Patch_Jump(0x005DFDC0, &ScenarioClassExt::_Find_Free_Local);
    Patch_Jump(0x005DFDA0, &ScenarioClassExt::_Num_Locals);

    Patch_Jump(0x005DCB59, &_ScenarioClass_Do_Win_GlobalFlags_Patch);
    Patch_Jump(0x005DC64D, &_Clear_Scenario_Clear_Globals_Patch);

    /**
     *  #issue-218
     * 
     *  Changes the default value of ScenarioClass 0x1D91 (IsGDI) from "1" to "0". This is
     *  because we now use it as a HouseType index, and need it to default to the first index.
     */
    Patch_Byte(0x005DAFD0+6, 0x00); // +6 skips the opcode.

    Patch_Jump(0x005DBA8B, &_Read_Scenario_Loading_Screen_Patch);

    /**
     *  Patch Unit creation to take possible missing houses into account for linked units.
     *  Patch Infantry to read spawn houses, as it doesn't house House_From_HousesType.
     */
    Patch_Call(0x00658658, &House_From_Name_Unit); // UnitClass
    Patch_Jump(0x004D7B98, &_InfantryClass_Read_INI_SpawnHouses_Patch); // InfantryClass

    /**
     *  Units have the follower mechanic, so we need to fix that up to account for potentially missing units.
     */
    Patch_Jump(0x006589C8, &_UnitClass_Read_INI_Link_Units);

    /**
     *  Jump past check in BuildingClass::Read_INI() preventing multiplayer building spawning for players.
     */
    Patch_Jump(0x0043485F, 0x00434874);

    /**
     *  Skip doing reinforcements if their receiver is non-existent.
     */
    Patch_Call(0x0061C39A, &Do_Reinforcements_Wrapper);
    Patch_Call(0x0061C3C1, &Do_Reinforcements_Wrapper);
}
