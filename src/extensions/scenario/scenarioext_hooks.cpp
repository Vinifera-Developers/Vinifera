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
#include "tibsun_inline.h"
#include "house.h"
#include "housetype.h"
#include "unit.h"
#include "unittype.h"
#include "unittypeext.h"
#include "rules.h"
#include "rulesext.h"
#include "campaign.h"
#include "multiscore.h"
#include "scenario.h"
#include "scenarioext.h"
#include "session.h"
#include "rules.h"
#include "ccfile.h"
#include "ccini.h"
#include "endgame.h"
#include "addon.h"
#include "side.h"
#include "infantrytype.h"
#include "aircrafttype.h"
#include "buildingtype.h"
#include "progressscreen.h"
#include "language.h"
#include "wsproto.h"
#include "ownrdraw.h"
#include "spritecollection.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "campaignext.h"
#include "housetypeext.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "kamikazetracker.h"
#include "mouse.h"
#include "spawner.h"
#include "vinifera_globals.h"


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

    Vinifera_ObserverPtr = nullptr;

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


enum {
    LS_400,
    LS_480,
    LS_600,
    LS_SIZE_COUNT,
    LS_SIZE_FIRST = LS_400
};


struct LoadingScreenConfig
{
    int SizeIndex;
    TPoint2D<int> Size;
    TPoint2D<int> SPPosition;
    TPoint2D<int> MPPosition;
};


/**
 *  This array contains the loading screen properties for different screen sizes.
 */
static LoadingScreenConfig LoadingScreenConfigs[]
{
    { LS_400, { 640, 400 }, { 436, 155 }, { 566, 152 } },
    { LS_480, { 640, 480 }, { 436, 186 }, { 566, 177 } },
    { LS_600, { 800, 600 }, { 546, 233 }, { 711, 227 } }
};


/**
 *  Gets the loading screen roperties for the user's screen size.
 *
 *  @author: ZivDero
 */
static LoadingScreenConfig& Get_Loading_Screen_Config()
{
    for (int i = LS_SIZE_COUNT - 1; i > LS_SIZE_FIRST; i--) {
        if (ScreenRect.Width >= LoadingScreenConfigs[i].Size.X && ScreenRect.Height >= LoadingScreenConfigs[i].Size.Y) {
            return LoadingScreenConfigs[i];
        }
    }

    return LoadingScreenConfigs[LS_SIZE_FIRST];
}


/**
 *  Reimplements the loading screen setup routine.
 * 
 *  @author: CCHyper, ZivDero
 */
static void Init_Loading_Screen(const char* filename)
{
    /**
     *  We need to read sides and houses now, because we need them to determine the player's
     *  side and loading screens.
     */
    Rule->Sides(*RuleINI);
    Rule->Houses(*RuleINI);

    for (int i = 0; i < HouseTypes.Count(); i++)
        HouseTypes[i]->Read_INI(*RuleINI);

    for (int i = 0; i < HouseTypeExtensions.Count(); i++)
        HouseTypeExtensions[i]->Read_INI(*RuleINI);

    /**
     *  #EDGE-CASE/#BUGFIX:
     *
     *  We need to do the fixup even earlier now as we need to use the Side
     *  value from the players HouseType.
     */
    {
        CCFileClass file(filename);
        CCINIClass ini(file);
        RuleExtension->Fixups(ini);
    }

    /**
     *  For the campaign, we check to see if the scenario name contains either
     *  "GDI" or "NOD", and then set the side to those respectively.
     */
    HousesType house = HOUSE_GDI;
    if (Session.Type == GAME_NORMAL) {
        if (Scen->CampaignID != CAMPAIGN_NONE) {

            const auto campaign_ext = Extension::Fetch<CampaignClassExtension>(Campaigns[Scen->CampaignID]);
            house = campaign_ext->House;
        }
    }
    else {

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
        reinterpret_cast<unsigned char&>(Session.IsGDI) = static_cast<unsigned char>(housetype->Side) & 0xFF;
    }

    /**
     *  Sanity check the side type.
     */
    if (house == HOUSE_NONE || house >= HouseTypes.Count()) {
        house = HOUSE_GDI;
    }

    const char* loadname;
    TPoint2D<int> textpos;

    /**
     *  Fetch the loading screen properties for the user's screen size.
     */
    LoadingScreenConfig& ls_config = Get_Loading_Screen_Config();

    /**
     *  Fetch the according loading screen from the user's house's data.
     */
    const auto housetype_ext = Extension::Fetch<HouseTypeClassExtension>(HouseTypes[house]);
    const auto& house_ls = housetype_ext->LoadingScreens[ls_config.SizeIndex];

    loadname = house_ls[Sim_Random_Pick(0, house_ls.Count() - 1)].Peek_Buffer();
    textpos = Session.Singleplayer_Game() ? ls_config.SPPosition : ls_config.MPPosition;

    /**
     *  Adjust the text position for Nod.
     */
    if (house == HOUSE_NOD) {
        textpos.Y += 7;
    }

    /**
     *  Fetch the loading screen override from the scenario.
     */
    const auto& ls_override = ScenExtension->LoadingScreens[ls_config.SizeIndex];
    if (ls_override.Filename.Is_Not_Empty()) {
        loadname = ls_override.Filename.Peek_Buffer();

        if (ls_override.Position.Is_Valid()) {
            textpos = ls_override.Position;
        }
    }

    /**
     *  Adjust the position of the text so it is correct for widescreen resolutions.
     */
    textpos.X += (ScreenRect.Width - ls_config.Size.X) / 2;
    textpos.Y += (ScreenRect.Height - ls_config.Size.Y) / 2;

    char loadfilename[PATH_MAX];
    std::snprintf(loadfilename, sizeof(loadfilename), "%s.PCX", loadname);

    /**
     *  The spawner can forcibly override the loading screen, and it already includes .PCX.
     */
    if (Vinifera_SpawnerActive) {

        if (Wstring(Vinifera_SpawnerConfig->CustomLoadScreen).Is_Not_Empty()) {
            std::snprintf(loadfilename, sizeof(loadfilename), "%s", Vinifera_SpawnerConfig->CustomLoadScreen);

            if (Vinifera_SpawnerConfig->CustomLoadScreenPos.Is_Valid()) {
                textpos = Vinifera_SpawnerConfig->CustomLoadScreenPos;
            }
        }
    }

    DEV_DEBUG_INFO("Loading Screen: \"%s\"\n", loadfilename);

    /**
     *  If this is a tournament game, format the game id.
     */
    char gamenamebuffer[128];
    const char* gamename = nullptr;

    if (Session.Type == GAME_INTERNET && PlanetWestwoodTournament == WOL::TOURNAMENT_0) {
        std::snprintf(gamenamebuffer, sizeof(gamenamebuffer), Text_String(TXT_GAME_ID), PlanetWestwoodGameID);
        gamename = gamenamebuffer;
    }

    /**
     *  Select the progress bar graphic depending on the game mode.
     */
    const int player_count = Session.Singleplayer_Game() ? 1 : Session.Players.Count();
    const char* progress_name = player_count <= 1 ? "PROGBAR.SHP" : "PROGBARM.SHP";

    /**
     *  Initialise the loading screen.
     */
    ProgressScreen.Init(100.0f, player_count);

    /**
     *  Forces the initial draw, Call_Back calls will update the redraw from here on.
     */
    ProgressScreen.Draw_Graphics(progress_name, loadfilename, gamename, textpos.X, textpos.Y);
    ProgressScreen.Draw_Bars_And_Text();
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
    //Patch_Jump(0x005DD100, &ScenarioClassExtension::Read_Scenario_INI); // Identical to vanilla for now, but missing a timer
    Patch_Jump(0x005DD4C0, &ScenarioClassExtension::Load_Scenario);
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
    Patch_Jump(0x005DC852, &_Clear_Scenario_Patch);
    Patch_Jump(0x005DC0A0, &_Fill_In_Data_Home_Cell_Patch);
    Patch_Jump(0x00673330, &_Waypoint_From_Name);
    Patch_Jump(0x006732B0, &_Waypoint_To_Name);
    // 0047A96C

    /**
     *  #issue-218
     * 
     *  Changes the default value of ScenarioClass 0x1D91 (IsGDI) from "1" to "0". This is
     *  because we now use it as a HouseType index, and need it to default to the first index.
     */
    Patch_Byte(0x005DAFD0+6, 0x00); // +6 skips the opcode.

    Patch_Jump(0x005DBA8B, &_Read_Scenario_Loading_Screen_Patch);
}
