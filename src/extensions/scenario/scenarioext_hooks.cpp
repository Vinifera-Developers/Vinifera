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
#include "reinf.h"
#include "spawner.h"
#include "teamtype.h"
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


#define SPAWN_HOUSE_OFFSET 50

/**
 *  Returns a house from a spawn house name.
 *
 *  @author: ZivDero
 */
HousesType Spawn_House_From_Name(const char* name)
{
    ASSERT(name != nullptr);

    int spawn_number;

    /**
     *  Try to read the house name as a spawn house name and extract its number.
     */
    if (std::sscanf(name, "Spawn%d", &spawn_number) == 1) {

        /**
         *  If we're successful, return a spawn house number.
         */
        return static_cast<HousesType>(spawn_number - 1 + SPAWN_HOUSE_OFFSET);
    }

    /**
     *  Fetch the house the normal way.
     */
    return HouseTypeClass::From_Name(name);
}


bool Is_Spawn_House(HousesType house)
{
    return house >= SPAWN_HOUSE_OFFSET && house < SPAWN_HOUSE_OFFSET + MAX_PLAYERS;
}

/**
 *  Returns a house from a spawn house name or a normal house name.
 *
 *  @author: ZivDero
 */
HousesType House_Or_Spawn_House_From_Name(const char* name)
{
    /**
     *  In campaigns, proceed as usual.
     */
    if (Session.Type == GAME_NORMAL) {
        return HouseTypeClass::From_Name(name);
    }

    /**
     *  In skirmish/multiplayer, try to fetch a spawn house instead.
     */
    return Spawn_House_From_Name(name);
}


/**
 *  Special unit version of House_Or_Spawn_House_From_Name that adds a
 *  null pointer to the unit vector if the house is not found.
 *
 *  @author: ZivDero
 */
HousesType House_Or_Spawn_House_From_Name_Unit(const char* name)
{
    /**
     *  In campaigns, proceed as usual.
     */
    if (Session.Type == GAME_NORMAL) {
        return HouseTypeClass::From_Name(name);
    }

    /**
     *  In skirmish/multiplayer, try to fetch a spawn house instead.
     *  If we couldn't find the spawn house, add a null pointer to the unit vector
     *  so that the "LinkedTo" numbers don't break. We'll remove these null pointers
     *  at the end.
     */
    HousesType house = Spawn_House_From_Name(name);
    if (house == HOUSE_NONE) {
        Units.Add(nullptr);
    }

    return house;
}


/**
 *  Returns a house pointer from a house type.
 *
 *  @author: ZivDero
 */
HouseClass* HouseClass_As_Pointer(HousesType house)
{
    /**
     *  In campaigns, or if this isn't a spawn house, proceed as usual.
     */
    if (Session.Type == GAME_NORMAL || !Is_Spawn_House(house)) {

        for (int i = 0; i < Houses.Count(); i++) {
            if (Houses[i]->Class->House == house) {
                return Houses[i];
            }
        }

        return nullptr;
    }

    /**
     *  For spawn houses, iterate all assigned starting positions and check if the one we want is present.
     */
    for (int i = 0; i < Session.Players.Count() + Session.Options.AIPlayers; i++) {

        /**
         *  If it is, that's our desired house.
         */
        if (ScenExtension->StartingPositions[i] == house - SPAWN_HOUSE_OFFSET) {
            return Houses[i];
        }
    }

    return nullptr;
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

    house = House_Or_Spawn_House_From_Name(house_name);

    if (house != HOUSE_NONE)
    {
        hptr = HouseClass_As_Pointer(house);

        _asm mov edi, hptr
        JMP(0x004D7BD5);
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
    for (int i = 0; i < Units.Count(); ++i)
    {
        int follower_id = link_vector[i];
        UnitClass* unit = Units[i];

        if (unit) {

            if (follower_id != -1 && follower_id < Units.Count() && Units[follower_id]) {
                UnitClass* follower = Units[follower_id];
                unit->FollowingMe = follower;
                follower->IsFollowing = true;
            }
            else {
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
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class CCINIClassExt final : public CCINIClass
{
public:
    HousesType _Get_HousesType(const char* section, const char* entry, const HousesType defvalue);
};


/**
 *  A wrapper for CCINIClass::Get_HousesType to read SpawnX houses.
 *
 *  @author: ZivDero
 */
HousesType CCINIClassExt::_Get_HousesType(const char* section, const char* entry, const HousesType defvalue)
{
    char buffer[128];

    /**
     *  In campaigns, proceed as usual.
     */
    if (Session.Type == GAME_NORMAL) {
        return Get_HousesType(section, entry, defvalue);
    }

    Get_String(section, entry, "", buffer, sizeof(buffer));

    /**
     *  Try to fetch the spawn houses's index.
     */
    return Spawn_House_From_Name(buffer);
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

    /**
     *  Hooks for SpawnX houses.
     */

    /**
     *  Patch HouseClass::As_Pointer to return houses based on spawn positions for IDs 50-57.
     */
    Patch_Jump(0x004C4730, &HouseClass_As_Pointer);

    /**
     *  Patch Unit, Building, Aircraft, Infatry and Team creation from the map to
     *  fetch Spawn houses by names correctly.
     */
    Patch_Call(0x00658658, &House_Or_Spawn_House_From_Name_Unit); // UnitClass
    Patch_Call(0x00434843, &House_Or_Spawn_House_From_Name); // BuildingClass
    Patch_Call(0x0040E806, &House_Or_Spawn_House_From_Name); // AircraftClass
    Patch_Jump(0x004D7B98, &_InfantryClass_Read_INI_SpawnHouses_Patch); // InfantryClass has As_Pointer inlined, so we have to do this instead
    Patch_Call(0x00628600, &CCINIClassExt::_Get_HousesType); // TeamTypeClass

    /**
     *  Units have the followed mechanic, so we need to fix that up to account for potentially missing units.
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
