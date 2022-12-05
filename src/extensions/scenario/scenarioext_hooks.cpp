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
#include "session.h"
#include "rules.h"
#include "ccfile.h"
#include "ccini.h"
#include "endgame.h"
#include "addon.h"
#include "house.h"
#include "housetype.h"
#include "side.h"
#include "infantrytype.h"
#include "unittype.h"
#include "aircrafttype.h"
#include "buildingtype.h"
#include "progressscreen.h"
#include "language.h"
#include "wsproto.h"
#include "rulesext.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "kamikazetracker.h"
#include "mouse.h"
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
 *  Reimplements the loading screen setup routine.
 * 
 *  @author: CCHyper
 */
static void Init_Loading_Screen(const char *filename)
{
    int image_width = 640;
    int image_height = 400;
    int load_filename_height = 400;
    char prefix = 'A';

    bool solo = Session.Singleplayer_Game();
    int player_count = solo ? 1 : Session.Players.Count();

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
     *  For the campaign, we abuse the required CD to get the desired Side.
     */
    SideType side = SIDE_GDI;
    if (Session.Type == GAME_NORMAL) {

        if (Scen->CampaignID != CAMPAIGN_NONE) {
            side = SideType(Campaigns[Scen->CampaignID]->WhichCD);
        }

    /**
     *  The first player in the player array is always the local player, so
     *  fetch our player info and the house we are assigned as.
     */
    } else {
        HousesType house = Session.Players.Fetch_Head()->Player.House;
        HouseTypeClass *housetype = HouseTypes[house];

        side = housetype->Side;
    }

    /**
     *  Sanity check the side type.
     */
    if (side == SIDE_NONE || side >= Sides.Count()) {
        side = SIDE_GDI;
    }

    /**
     *  Get the loading screen variation prefix.
     * 
     *  #NOTE: Why does this use network random?
     */
    if (side == SIDE_GDI) {
        prefix = Percent_Chance(50) ? 'C' : 'D';

    } else if (side == SIDE_NOD) {
        prefix = Percent_Chance(50) ? 'A' : 'B';

    /**
     *  #issue-665
     *
     *  The remaining characters can be used in standard order for new sides.
     *  This gives us support for 11 new sides before this system breaks.
     */
    } else {
        prefix = Percent_Chance(50) ? 'A' : 'B';
        prefix += char(2*side); // Offset the character based on the side index.
        if (prefix > 'Z') {
            prefix = 'Z';
        }
    }

    Point2D textpos(0,0);

    /**
     *  Set the progress text draw positions (resolves #issue-294).
     */
    if (ScreenRect.Width >= 640 && ScreenRect.Height == 400) {

        /**
         *  #issue-294
         * 
         *  Centralises the position of the loading screen text.
         * 
         *  Original code retained below.
         */
#if 0
        if (solo) {
            textpos.X = 440;
            textpos.Y = 158;
        } else {
            textpos.X = 570;
            textpos.Y = 155;
        }
#endif

        /**
         *  The text box is in slightly different positions between GDI and NOD.
         */
        if (side == SIDE_GDI) {
            textpos.X = solo ? 435 : 435;
            textpos.Y = solo ? 157 : 157;

        } else if (side == SIDE_NOD) {
            textpos.X = solo ? 436 : 436;
            textpos.Y = solo ? 161 : 161;

        /**
         *  All other sides (uses the GDI offsets).
         */
        } else {
            textpos.X = solo ? 435 : 435;
            textpos.Y = solo ? 157 : 157;
        }

        image_width = 640;
        image_height = 400;

        load_filename_height = 400;

    } else if (ScreenRect.Width >= 640 && ScreenRect.Height == 480) {

        /**
         *  #issue-294
         *
         *  Centralises the position of the loading screen text.
         *
         *  Original code retained below.
         */
#if 0
        if (solo) {
            textpos.X = 440;
            textpos.Y = 189;
        } else {
            textpos.X = 570;
            textpos.Y = 180;
        }
#endif

        /**
         *  The text box is in slightly different positions between GDI and NOD.
         */
        if (side == SIDE_GDI) {
            textpos.X = solo ? 435 : 435;
            textpos.Y = solo ? 195 : 195;

        } else if (side == SIDE_NOD) {
            textpos.X = solo ? 436 : 436;
            textpos.Y = solo ? 200 : 200;

        /**
         *  All other sides (uses the GDI offsets).
         */
        } else {
            textpos.X = solo ? 435 : 435;
            textpos.Y = solo ? 195 : 195;
        }

        image_width = 640;
        image_height = 480;

        load_filename_height = 480;

    } else if (ScreenRect.Width >= 800 && ScreenRect.Height >= 600) {

        /**
         *  #issue-294
         *
         *  Centralises the position of the loading screen text.
         *
         *  Original code retained below.
         */
#if 0
        if (solo) {
            textpos.X = 550;
            textpos.Y = 236;
        } else {
            textpos.X = 715;
            textpos.Y = 230;
        }
#endif

        /**
         *  The text box is in slightly different positions between GDI and NOD.
         */
        if (side == SIDE_GDI) {
            textpos.X = solo ? 563 : 563;
            textpos.Y = solo ? 252 : 252;

        } else if (side == SIDE_NOD) {
            textpos.X = solo ? 565 : 565;
            textpos.Y = solo ? 258 : 258;

        /**
         *  All other sides (uses the GDI offsets).
         */
        } else {
            textpos.X = solo ? 563 : 563;
            textpos.Y = solo ? 252 : 252;
        }

        image_width = 800;
        image_height = 600;

        load_filename_height = 600;
    }

    /**
     *  Adjust the position of the text so it is correct for widescreen resolutions.
     */
    textpos.X += (ScreenRect.Width - image_width) / 2;
    textpos.Y += (ScreenRect.Height - image_height) / 2;

    /**
     *  Adjust the text positions for the Nod side graphics.
     */
    textpos.X -= 4;
    if (side == SIDE_NOD) {
        textpos.Y += 10;
    } else {
        textpos.Y += 3;
    }

    /**
     *  Build the loading screen filename. (Format: LOAD[screen width][side char].PCX)
     */
    char loadname[16];
    std::snprintf(loadname, sizeof(loadname), "LOAD%d%c.PCX", load_filename_height, prefix);

    /**
     *  Check to make sure the loading screen file can be found, if not, then
     *  default to the GDI loading screen image set.
     */
    if (!CCFileClass(loadname).Is_Available()) {
        std::snprintf(loadname, sizeof(loadname), "LOAD%d%c.PCX", load_filename_height, Sim_Percent_Chance(50) ? 'C' : 'D');
    }

    DEV_DEBUG_INFO("Loading Screen: \"%s\"\n", loadname);

    /**
     *  If this is a tournament game, format the game id.
     */
    char gamenamebuffer[128];
    const char *gamename = nullptr;

    if (Session.Type == GAME_INTERNET && TournamentGameType == WOL::TOURNAMENT_0) {
        std::snprintf(gamenamebuffer, sizeof(gamenamebuffer), Text_String(TXT_GAME_ID), GameID);
        gamename = gamenamebuffer;
    }

    /**
     *  Select the progress bar graphic depending on the game mode.
     */
    const char *progress_name = player_count <= 1 ? "PROGBAR.SHP" : "PROGBARM.SHP";

    /**
     *  Initialise the loading screen.
     */
    ProgressScreen.Init(100.0f, player_count);

    /**
     *  Forces the initial draw, Call_Back calls will update the redraw from here on.
     */
    ProgressScreen.Draw_Graphics(progress_name, loadname, gamename, textpos.X, textpos.Y);
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

    Init_Loading_Screen(filename);

    /**
     *  Jump to setting broadcast addresses.
     */
    JMP(0x005DBD4A);
}


/**
 *  This is a real kludge to work around the fact that in Tiberian Sun, each
 *  of the side asset mix files contain their own versions of cameo artwork
 *  for the opposing side.
 * 
 *  We call Call_Back() after each iteration to ensure the network connection
 *  does not drop between clients in multiplayer games.
 * 
 *  @author: CCHyper
 */
static bool Read_Scenario_INI_Reload_Cameo_For_Side()
{
    char buffer[32];

    DEBUG_INFO("Reloading TechnoType cameos.\n");

    Call_Back();

    for (int index = 0; index < TechnoTypes.Count(); ++index) {

        TechnoTypeClass *ttype = TechnoTypes[index];
        std::snprintf(buffer, sizeof(buffer), "%s.SHP", ttype->CameoFilename);

        const ShapeFileStruct *cameodata = MFCC::RetrieveT<const ShapeFileStruct>(buffer);

        if (cameodata == nullptr) continue;

        if (ttype->CameoData != nullptr && cameodata == ttype->CameoData) continue;

        DEBUG_INFO("  Reloaded cameo for %s \"%s\" (%s).\n", Name_From_RTTI(RTTIType(ttype->What_Am_I())), ttype->Name(), buffer);
        ttype->CameoData = cameodata;
    }

    DEBUG_INFO("Finished reloading TechnoType cameos.\n");

    Call_Back();

    return true;
}


/**
 *  Process the rules and scenario rules overrides.
 *
 *  @author: CCHyper
 */
static bool Read_Scenario_INI_Rules_Process(CCINIClass &ini)
{
    /**
     *  Initialise and process rules.
     */
    DEBUG_INFO("Initializeing Rules\n");
    Rule->Initialize(*RuleINI);

    Session.Loading_Callback(35);

    Call_Back();

    /**
     *  Read global variables from rules.
     */
    DEBUG_INFO("Calling Scen->Read_Global_INI(*RuleINI)...\n");
    Scen->Read_Global_INI(*RuleINI);

    Call_Back();

    /**
     *  Process rules scenario overrides.
     */
    DEBUG_INFO("Calling Rule->Addition() with scenario overrides.\n");
    Rule->Addition(ini);
    DEBUG_INFO("Finished Rule->Addition() with scenario overrides.\n");

    Session.Loading_Callback(45);

    /**
     *  Read in scenario house types.
     */
    if (Session.Type == GAME_NORMAL) {
        DEBUG_INFO("Calling HouseClass::Read_Scenario_INI()...\n");
        HouseClass::Read_Scenario_INI(ini);
    }

    Session.Loading_Callback(50);

    /**
     *  Read scenario basic.
     */
    DEBUG_INFO("Calling Scen->Read_INI()...\n");
    if (!Scen->Read_INI(ini)) {
        return false;
    }

    return true;
}


/**
 *  Initialise the player value which is later used to load the side assets.
 *
 *  @author: CCHyper
 */
static bool Read_Scenario_INI_Init_Side(CCINIClass &ini)
{
    HouseTypeClass *housetype = nullptr;

    /**
     *  If this is a campaign session, load the house from the "Player" value.
     */
    if (Session.Type == GAME_NORMAL) {

        char buffer[32];
        ini.Get_String("Basic", "Player", "GDI", buffer, sizeof(buffer));

#if 0
        /**
         *  Original game code.
         */
        bool is_gdi = strcmpi(buffer, "GDI") == 0;
        Scen->IsGDI = is_gdi;
        Scen->SpeechSide = is_gdi ? SIDE_GDI : SIDE_NOD;
#endif

        /**
         *  Fetch the houses side type and use this to decide which assets to load.
         */
        housetype = (HouseTypeClass *)HouseTypeClass::As_Pointer(buffer);

        Scen->IsGDI = (unsigned char)housetype->Side & 0xFF;
        Scen->SpeechSide = housetype->Side;

        ASSERT_FATAL_PRINT(housetype != nullptr, "Invalid \"Player\" value in [Basic] section!");

        /**
         *  Read speech side override.
         */
        Scen->SpeechSide = ini.Get_SideType("Basic", "SpeechSide", Scen->SpeechSide);

        ASSERT_FATAL_PRINT(Scen->SpeechSide != SIDE_NONE && Scen->SpeechSide < Sides.Count(), "Invalid \"SpeechSide\" value in [Basic] section!");

        /**
         *  #issue-309
         * 
         *  Read sidebar side override.
         */
        ScenExtension->SidebarSide = ini.Get_SideType("Basic", "SidebarSide", housetype->Side);

        ASSERT_FATAL_PRINT(ScenExtension->SidebarSide != SIDE_NONE && ScenExtension->SidebarSide < Sides.Count(), "Invalid \"SidebarSide\" value in [Basic] section!");

    } else {

#if 0
        /**
         *  Original game code.
         */
        Scen->IsGDI = Session.IsGDI;
        Scen->SpeechSide = Session.IsGDI ? SIDE_GDI : SIDE_NOD;
#endif

        /**
         *  Fetch the houses side type and use this to decide which assets to load.
         */
        housetype = (HouseTypeClass *)HouseTypeClass::As_Pointer(HousesType(PlayerPtr->Class->House));
        ASSERT_FATAL_PRINT(housetype != nullptr, "Invalid multiplayer house!");

        Scen->IsGDI = (unsigned char)housetype->Side & 0xFF;
        Scen->SpeechSide = housetype->Side;
        ScenExtension->SidebarSide = housetype->Side;

    }

    return true;
}


/**
 *  Initialise the player side assets (sidebar, speech, etc).
 *
 *  @author: CCHyper
 */
static bool Read_Scenario_INI_Prep_For_Side()
{
    /**
     *  Fetch the houses side type and use this to decide which assets to load.
     */
    HouseTypeClass *housetype = (HouseTypeClass *)HouseTypeClass::As_Pointer(HousesType(Scen->IsGDI));

#ifndef NDEBUG
    DEV_DEBUG_INFO("About to prepare for...\n");
    DEV_DEBUG_INFO("  House \"%s\" (%d) with Side \"%s\" (%d)\n",
        housetype->Name(), housetype->Get_Heap_ID(),
        Sides[housetype->Side]->Name(), Sides[housetype->Side]->Get_Heap_ID());

    DEV_DEBUG_INFO("Side info:\n");
    for (int i = 0; i < Sides.Count(); ++i) {
        SideClass *side = Sides[i];
        DEV_DEBUG_INFO("  Side \"%s\" (%d), Houses.Count %d\n", side->IniName, i, side->Houses.Count());
        for (int i = 0; i < side->Houses.Count(); ++i) {
            DEV_DEBUG_INFO("    Houses %d = %s (%d)\n", i, HouseTypes[side->Houses[i]]->Name(), side->Houses[i]);
        }
    }
#endif

    DEBUG_INFO("Calling Prep_For_Side()...\n");
    if (!Prep_For_Side(ScenExtension->SidebarSide)) {

        DEBUG_WARNING("Prep_For_Side(%d) failed! Trying with side 0...\n", ScenExtension->SidebarSide);

        /**
         *  Try once again but with the Side 0 (GDI) assets.
         */
        if (!Prep_For_Side(SIDE_GDI)) {
            DEBUG_ERROR("Prep_For_Side() failed!\n");
            return false;
        }
    }

    DEBUG_INFO("Calling Prep_Speech_For_Side()...\n");
    if (!Prep_Speech_For_Side(Scen->SpeechSide)) {

        DEBUG_WARNING("Prep_Speech_For_Side(%d) failed! Trying with side 0...\n", Scen->SpeechSide);

        /**
         *  Try once again but with the Side 0 (GDI) assets.
         */
        if (!Prep_Speech_For_Side(SIDE_GDI)) {
            DEBUG_ERROR("Prep_Speech_For_Side() failed!\n");
            return false;
        }
    }

    return true;
}


/**
 *  #issue-218
 *
 *  This patch replaces a fairly large chunk of code in Read_Scenario_INI that
 *  read/initialised the player house, loaded the side assets, then processed the
 *  rules data and scenario overrides.
 * 
 *  Because we now abuse ScenarioClass::IsGDI and SessionClass::IsGDI to store
 *  the player house, we need to reorder this chunk of code to we are reading
 *  the house types before we load the side assets, this allows us to use the
 *  Side member of HouseTypeClass to pick the correct assets.
 * 
 *  There is an additional chunk of code that has been added to reload the object
 *  cameos a second time, this is because the loading of the side assets is now
 *  before the rules initialisation, and as a result, the rules data is being
 *  processed before the side assets are declared to the mix file system.
 */
DECLARE_PATCH(_Read_Scenario_INI_Init_Side_Patch)
{
    GET_REGISTER_STATIC(CCINIClass *, ini, ebp);

    /**
     *  The original code flow was;
     *    Read [Basic] -> Player=
     *    Prep_For_Side
     *    Process Rules.
     * 
     *  To support assets for new sides, be now need to move
     *  the rules processing to before the side and asset loading.
     */

    if (!Read_Scenario_INI_Rules_Process(*ini)) {
        goto return_false;
    }

    if (!Read_Scenario_INI_Init_Side(*ini)) {
        goto return_false;
    }

    if (!Read_Scenario_INI_Prep_For_Side()) {
        goto return_false;
    }

    if (!Read_Scenario_INI_Reload_Cameo_For_Side()) {
        goto return_false;
    }

    JMP(0x005DD956);

return_false:
    _asm{ xor al, al }
    JMP_REG(ecx, 0x005DD7AB);
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
        if (Addon_Enabled(ADDON_FIRESTORM)) { 
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
    Patch_Jump(0x005DD6FB, &_Read_Scenario_INI_Init_Side_Patch);

    Patch_Jump(0x005DBA8B, &_Read_Scenario_Loading_Screen_Patch);
}
