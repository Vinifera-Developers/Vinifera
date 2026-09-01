/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended ScenarioClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "scenarioext.h"

#include "addon.h"
#include "aircraft.h"
#include "aitrigtype.h"
#include "armortype.h"
#include "asserthandler.h"
#include "beacon.h"
#include "building.h"
#include "buildingtype.h"
#include "campaign.h"
#include "campaignext.h"
#include "ccini.h"
#include "cd.h"
#include "commandext.h"
#include "debughandler.h"
#include "environmentext.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "infantry.h"
#include "infantrytype.h"
#include "iomap.h"
#include "language.h"
#include "lightsource.h"
#include "multimiss.h"
#include "noinit.h"
#include "optionsext.h"
#include "overlay.h"
#include "ownrdraw.h"
#include "playmovie.h"
#include "radarevent.h"
#include "restate.h"
#include "rules.h"
#include "rulesext.h"
#include "scripttype.h"
#include "session.h"
#include "sessionext.h"
#include "sideext.h"
#include "smudge.h"
#include "spawner.h"
#include "swizzle.h"
#include "tactical.h"
#include "tacticalext.h"
#include "tag.h"
#include "tagtype.h"
#include "taskforce.h"
#include "teamtype.h"
#include "terrain.h"
#include "theme.h"
#include "tiberium.h"
#include "tibsun_defines.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "tracker.h"
#include "triggertype.h"
#include "tube.h"
#include "unit.h"
#include "unittype.h"
#include "unittypeext.h"
#include "veinholemonster.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"
#include "vinifera_util.h"
#include "waypoint.h"
#include "wwmouse.h"

#include <algorithm>
#include <regex>


namespace
{
    /**
     *  Scenario loading-screen overrides are only needed while setting up a
     *  scenario. Keep this non-trivial state outside the raw-serialized global
     *  extension object.
     */
    std::vector<UIControlsClass::LoadingScreen> ScenarioLoadingScreens;
}


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
ScenarioClassExtension::ScenarioClassExtension(const ScenarioClass *this_ptr) :
    GlobalExtensionClass(this_ptr),
    IsIceDestruction(true),
    SidebarSide(SIDE_NONE),
    IsUseMPAIBaseNodes(false)
{
    /**
     *  This copies the behavior of the games ScenarioClass.
     */
    Init_Clear();
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
ScenarioClassExtension::ScenarioClassExtension(const NoInitClass &noinit) :
    GlobalExtensionClass(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
ScenarioClassExtension::~ScenarioClassExtension()
{
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT ScenarioClassExtension::Load(IStream *pStm)
{
    HRESULT hr = GlobalExtensionClass::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) ScenarioClassExtension(NoInitClass());

    /**
     *  NewINIFormat is normally only set when a scenario INI is parsed, which
     *  does not happen when loading a saved game, so restore it from the save.
     */
    hr = pStm->Read(&NewINIFormat, sizeof(NewINIFormat), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT ScenarioClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = GlobalExtensionClass::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    hr = pStm->Write(&NewINIFormat, sizeof(NewINIFormat), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int ScenarioClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void ScenarioClassExtension::Object_CRC(CRCEngine &crc) const
{
    crc(IsIceDestruction);
}


/**
 *  Initialises any values for this instance.
 *  
 *  @author: CCHyper
 */
void ScenarioClassExtension::Init_Clear()
{
    IsIceDestruction = true;
    ScorePlayerColor = RGBStruct{ 253, 181, 28 }; // Default to TS GDI score color
    ScoreEnemyColor = RGBStruct{ 250, 28, 28 };   // Default to TS Nod score color

    ScenarioLoadingScreens.clear();

    {
        /**
         *  Clear the any previously loaded tutorial messages in preparation for
         *  reloading the TUTORIAL.INI as they might contain scenario overrides.
         */
        Vinifera_TutorialText.clear();

        /**
         *  Reload the main tutorial message data.
         */
        CCINIClass ini;
        CCFileClass tutorial_file("TUTORIAL.INI");
        ini.Load(tutorial_file, false);
        Read_Tutorial_INI(ini);
    }

    /**
     *  Clear all waypoint values, preparing for scenario loading.
     */
    Clear_All_Waypoints();

    for (int index = 0; index < std::size(GlobalFlags); index++) {
        GlobalFlags[index].VariableName[0] = '\0';
        Set_Global_To(index, 0);
    }

    for (int index = 0; index < std::size(LocalFlags); index++) {
        LocalFlags[index].VariableName[0] = '\0';
        Set_Local_To(index, 0);
    }

    /* 
     * Erase unit all filter hotkey states as their unit objects are invalid now 
     */
    UnitFilterLastFullSelectionByClassifiers.clear();
}


/**
 *  Initialises any values for this instance.
 *
 *  @author: CCHyper
 */
bool ScenarioClassExtension::Read_INI(CCINIClass &ini)
{
    static const char * const BASIC = "Basic";

    IsIceDestruction = ini.Get_Bool(BASIC, "IceDestructionEnabled", IsIceDestruction);
    ScorePlayerColor = ini.Get_RGBColor(BASIC, "ScorePlayerColor", ScorePlayerColor);
    ScoreEnemyColor = ini.Get_RGBColor(BASIC, "ScoreEnemyColor", ScoreEnemyColor);
    IsUseMPAIBaseNodes = ini.Get_Bool(BASIC, "UseMPAIBaseNodes", IsUseMPAIBaseNodes);

    /**
     *  #issue-123
     * 
     *  Fetch additional tutorial message data (if present) from the scenario.
     */
    Read_Tutorial_INI(ini);

    BeaconManager.Load_Art();

    return true;
}


/**
 *  Read the loading screen overrides from the scenario INI.
 *
 *  @author: CCHyper
 */
bool ScenarioClassExtension::Read_Loading_Screen_INI(const char *filename)
{
    CCFileClass file(filename);
    CCINIClass ini;
    ini.Load(file, false);

    if (!ini.Is_Loaded()) {
        return false;
    }

    if (Session.Type == GAME_NORMAL) {
        static char const* const LOADING_SCREENS = "LoadingScreens";
        for (int i = 0; i < ini.Entry_Count(LOADING_SCREENS); i++) {
            const char* key = ini.Get_Entry(LOADING_SCREENS, i);
            const std::string entry = ini.Get_String(LOADING_SCREENS, key, {});
            UIControlsClass::LoadingScreen screen(entry.c_str());
            if (screen.IsValid) {
                ScenarioLoadingScreens.emplace_back(screen);
            }
        }
    }

    return true;
}


UIControlsClass::LoadingScreen const* ScenarioClassExtension::Pick_Loading_Screen_Override(HousesType house) const
{
    std::vector<UIControlsClass::LoadingScreen const*> screens;

    int largest_size = 0;
    for (auto& screen : ScenarioLoadingScreens) {
        if (screen.House == house && VisibleRect.Width >= screen.Size.Size.X && VisibleRect.Height >= screen.Size.Size.Y) {
            int size = screen.Size.Size.X * screen.Size.Size.Y;
            if (size > largest_size) {
                screens.clear();
                screens.emplace_back(&screen);
                largest_size = size;
            } else if (size == largest_size) {
                screens.emplace_back(&screen);
            }
        }
    }

    if (!screens.empty()) {
        return screens[Sim_Random_Pick(0u, static_cast<unsigned int>(screens.size() - 1))];
    }

    return nullptr;
}


/**
 *  Load the tutorial messages section from the ini database.
 *
 *  @author: CCHyper
 */
bool ScenarioClassExtension::Read_Tutorial_INI(CCINIClass const& ini)
{
    char buffer[512];
    static char const * const TUTORIAL = "Tutorial";

    /**
     *  Fetch the additional tutorial message data (if present).
     */
    if (ini.Is_Present(TUTORIAL)) {
        int counter = ini.Entry_Count(TUTORIAL);

        for (int index = 0; index < counter; ++index) {
            const char *entry = ini.Get_Entry(TUTORIAL, index);

            /**
             *  Get a tutorial message entry.
             */
            if (ini.Get_String(TUTORIAL, entry, "", buffer, sizeof(buffer))) {
                Vinifera_TutorialText[entry] = buffer;
            }
        }
    }

    return true;
}


/**
 *  Get the cell value of a waypoint location.
 *
 *  @author: CCHyper
 */
Cell ScenarioClassExtension::Waypoint_Cell(WAYPOINT wp) const
{
    ASSERT_FATAL(wp < std::size(Waypoint));

    return Waypoint[wp];
}


/**
 *  Get the cell pointer of a waypoint location.
 *
 *  @author: CCHyper
 */
CellClass *ScenarioClassExtension::Waypoint_CellClass(WAYPOINT wp) const
{
    ASSERT_FATAL(wp < std::size(Waypoint));

    return &Map[Waypoint[wp]];
}


/**
 *  Get the coordinate of a waypoint location.
 *
 *  #NOTE: The coordinate is adjusted by the bridge height if the waypoint is on a bridge cell.
 *
 *  @author: CCHyper
 */
Coord ScenarioClassExtension::Waypoint_Coord(WAYPOINT wp) const
{
    ASSERT_FATAL(wp < std::size(Waypoint));

    CellClass *cell = &Map[Waypoint[wp]];
    Coord coord = cell->Center_Coord();
    if (cell->IsUnderBridge || cell->WasUnderBridge) {
        coord.Z += BRIDGE_LEPTON_HEIGHT;
    }
    return coord;
}


/**
 *  Set the waypoint location from the cell value.
 *
 *  @author: CCHyper
 */
void ScenarioClassExtension::Set_Waypoint_Cell(WAYPOINT wp, Cell &cell)
{
    ASSERT_FATAL(wp < std::size(Waypoint));

    Waypoint[wp] = cell;
}


/**
 *  Set the waypoint location from a coordinate value.
 *
 *  @author: CCHyper
 */
void ScenarioClassExtension::Set_Waypoint_Coord(WAYPOINT wp, Coord &coord)
{
    Waypoint[wp] = coord.As_Cell();
}


/**
 *  Is this waypoint a valid cell location?
 *
 *  @author: CCHyper
 */
bool ScenarioClassExtension::Is_Waypoint_Valid(WAYPOINT wp) const
{
    ASSERT_FATAL(wp < std::size(Waypoint));

    return (wp >= WAYPOINT_FIRST && wp < std::size(Waypoint)) ? (Waypoint[wp] != CELL_NONE) : false;
}


/**
 *  Clear the waypoint value.
 *
 *  @author: CCHyper
 */
void ScenarioClassExtension::Clear_Waypoint(WAYPOINT wp)
{
    ASSERT_FATAL(wp < std::size(Waypoint));

    Waypoint[wp] = Cell(0, 0);
}


/**
 *  Clear all the waypoints, emptying the list.
 *
 *  @author: CCHyper
 */
void ScenarioClassExtension::Clear_All_Waypoints()
{
    for (auto& waypoint : Waypoint) {
        waypoint = CELL_NONE;
    }
}


/**
 *  Read the waypoint locations from the ini database.
 *
 *  @author: CCHyper
 */
void ScenarioClassExtension::Read_Waypoint_INI(CCINIClass &ini)
{
    static const char * const WAYNAME = "Waypoints";

    char entry[32];
    int valid_count = 0;

    /**
     *  Read the Waypoint entries.
     */
    for (WAYPOINT wp = WAYPOINT_FIRST; wp < std::size(Waypoint); ++wp) {

        /**
         *  Get a waypoint entry.
         */
        std::snprintf(entry, sizeof(entry), "%d", wp);
        int value = ini.Get_Int(WAYNAME, entry, 0);

        /**
         *  Skip invalid entries.
         */
        if (!value) {
            continue;
        }

        ++valid_count;

        /**
         *  Convert this value to an actual map cell location.
         */
        Cell cell;
        cell.X = value % 1000;
        cell.Y = value / 1000;

        int wp_num = std::strtol(entry, nullptr, 10);

        switch (wp_num) {
            case WAYPOINT_HOME:
                DEV_DEBUG_INFO("Scenario: Read waypoint '{}' (HOME) ({},{}).\n", ::Waypoint_As_String(wp), cell.X, cell.Y);
                break;
            case WAYPOINT_REINF:
                DEV_DEBUG_INFO("Scenario: Read waypoint '{}' (REINF) ({},{}).\n", ::Waypoint_As_String(wp), cell.X, cell.Y);
                break;
            case WAYPOINT_SPECIAL:
                DEV_DEBUG_INFO("Scenario: Read waypoint '{}' (SPECIAL) ({},{}).\n", ::Waypoint_As_String(wp), cell.X, cell.Y);
                break;
            default:
                DEV_DEBUG_INFO("Scenario: Read waypoint '{}' ({},{}).\n", ::Waypoint_As_String(wp), cell.X, cell.Y);
                break;
        };

        /**
         *  Store the waypoint value.
         */
        Waypoint[wp_num] = cell;

        /**
         *  If the cell location is valid, flag the cell on the map as a waypoint holder.
         */
        if (wp_num >= 0 && cell != CELL_NONE) {
#ifndef NDEBUG
            //DEV_DEBUG_INFO("Scenario: Waypoint '{}', location '{},{}' -> IsWaypoint = true.\n", ::Waypoint_As_String(cell), cell.X, cell.Y);
#endif
            Map[cell].IsWaypoint = true;
        }

    }

    if (valid_count > 0) DEV_DEBUG_INFO("Scenario: Read a total of '{}' waypoints.\n", valid_count);
}


/**
 *  Write the waypoint locations to the ini database.
 *
 *  @author: CCHyper
 */
void ScenarioClassExtension::Write_Waypoint_INI(CCINIClass &ini)
{
    static char const * const WAYNAME = "Waypoints";

    char entry[32];
    int valid_count = 0;

    /**
     *  Clear any existing section from the ini database.
     */
    ini.Clear(WAYNAME);

    /**
     * Save the Waypoint entries.
     */
    for (WAYPOINT wp = WAYPOINT_FIRST; wp < std::size(Waypoint); ++wp) {
        if (Is_Waypoint_Valid(wp)) {
            std::snprintf(entry, sizeof(entry), "%d", wp);
            int value = Waypoint[wp].X + 1000 * Waypoint[wp].Y;
            ini.Put_Int(WAYNAME, entry, value);
            ++valid_count;
        }
    }

    if (valid_count > 0) DEV_DEBUG_INFO("Scenario: Wrote a total of '{}' waypoints.\n", valid_count);
}


/**
 *  Returns the waypoint number as a string.
 *
 *  @author: CCHyper
 */
const char * ScenarioClassExtension::Waypoint_As_String(WAYPOINT wp) const
{
    if (Is_Waypoint_Valid(wp)) {
        return ::Waypoint_As_String(wp);
    }

    return "";
}


/**
 *  Set scenario global to value specified.
 *
 *  @author: ZivDero
 */
int ScenarioClassExtension::Set_Global_To(int global, int value)
{
    if ((unsigned)global < std::size(GlobalFlags)) {

        int previous = GlobalFlags[global].Value;
        if (previous != value) {
            GlobalFlags[global].Value = value;
            This()->IsGlobalChanged = true;

            /*
            **  Special case to scan through all triggers and if any are found that depend on this
            **  global being set/cleared, then if there is an elapsed time event associated, it
            **  will be reset at this time.
            */
            TagClass::All_Timer_Global_Reset(global);

            /*
            **  Clear the templated text cache as it may contain this variable.
            */
            if (TacticalMapExtension) {
                TacticalMapExtension->Clear_Templated_Text_Cache();
            }
        }
        return previous;
    }
    return 0;
}


/**
 *  Set scenario global to value specified.
 *
 *  @author: ZivDero
 */
int ScenarioClassExtension::Set_Global_To(char const* name, int value)
{
    int global = Find_Global_Variable_Index(name);
    if (global != -1) {
        return Set_Global_To(global, value);
    }
    return 0;
}


/**
 *  Get scenario global value.
 *
 *  @author: ZivDero
 */
bool ScenarioClassExtension::Get_Global_Value(int global, int& value)
{
    if (global >= 0 && global < std::size(GlobalFlags)) {
        value = GlobalFlags[global].Value;
        return true;
    }
    return false;
}


/**
 *  Get scenario global value.
 *
 *  @author: ZivDero
 */
bool ScenarioClassExtension::Get_Global_Value(char const* name, int& value)
{
    int global = Find_Global_Variable_Index(name);
    if (global != -1) {
        return Get_Global_Value(global, value);
    }
    return false;
}


/**
 *  Get scenario global index by its name.
 *
 *  @author: ZivDero
 */
int ScenarioClassExtension::Find_Global_Variable_Index(char const* name)
{
    for (int i = 0; i < std::size(GlobalFlags); i++) {
        if (!strcmp(name, GlobalFlags[i].VariableName)) {
            return i;
        }
    }
    return -1;
}


/**
 *  Read scenario globals from an INI file.
 *
 *  @author: ZivDero
 */
bool ScenarioClassExtension::Read_Global_INI(INIClass& ini)
{
    int count = std::min(ini.Entry_Count("VariableNames"), static_cast<int>(std::size(GlobalFlags)));

    for (int i = 0; i < count; i++) {
        const char* entry = ini.Get_Entry("VariableNames", i);
        int idx = std::atoi(entry);
        ini.Get_String("VariableNames", entry, nullptr, GlobalFlags[idx].VariableName, sizeof(GlobalFlags[idx].VariableName));
    }

    return true;
}


/**
 *  Set scenario local to value specified.
 *
 *  @author: ZivDero
 */
int ScenarioClassExtension::Set_Local_To(int local, int value)
{
    if (static_cast<size_t>(local) < std::size(LocalFlags)) {

        int previous = LocalFlags[local].Value;
        if (previous != value) {
            LocalFlags[local].Value = value;
            This()->IsGlobalChanged = true;

            /*
            **  Special case to scan through all triggers and if any are found that depend on this
            **  local being set/cleared, then if there is an elapsed time event associated, it
            **  will be reset at this time.
            */
            TagClass::All_Timer_Local_Reset(local);

            /*
            **  Clear the templated text cache as it may contain this variable.
            */
            if (TacticalMapExtension) {
                TacticalMapExtension->Clear_Templated_Text_Cache();
            }
        }
        return previous;
    }
    return 0;
}


/**
 *  Set scenario local to value specified.
 *
 *  @author: ZivDero
 */
int ScenarioClassExtension::Set_Local_To(char const* name, int value)
{
    int local = Find_Local_Variable_Index(name);
    if (local != -1) {
        return Set_Local_To(local, value);
    }
    return 0;
}


/**
 *  Set scenario local to value specified.
 *
 *  @author: ZivDero
 */
bool ScenarioClassExtension::Get_Local_Value(int local, int& value)
{
    if (local >= 0 && local < std::size(LocalFlags)) {
        value = LocalFlags[local].Value;
        return true;
    }
    return false;
}


/**
 *  Get scenario local value.
 *
 *  @author: ZivDero
 */
bool ScenarioClassExtension::Get_Local_Value(char const* name, int& value)
{
    int local = Find_Local_Variable_Index(name);
    if (local != -1) {
        return Get_Local_Value(local, value);
    }
    return false;
}


/**
 *  Get scenario local index by its name.
 *
 *  @author: ZivDero
 */
int ScenarioClassExtension::Find_Local_Variable_Index(char const* name)
{
    for (int i = 0; i < std::size(LocalFlags); i++) {
        if (!strcmp(name, LocalFlags[i].VariableName)) {
            return i;
        }
    }
    return -1;
}


/**
 *  Read scenario locals from an INI file.
 *
 *  @author: ZivDero
 */
bool ScenarioClassExtension::Read_Local_INI(INIClass& ini)
{
    char buffer[128];

    for (int i = 0; i < std::size(LocalFlags); i++) {
        LocalFlags[i].VariableName[0] = 0;
    }

    int count = std::min(ini.Entry_Count("VariableNames"), static_cast<int>(std::size(LocalFlags)));

    for (int i = 0; i < count; i++) {
        const char* entry = ini.Get_Entry("VariableNames", i);
        int local = atoi(entry);
        ini.Get_String("VariableNames", entry, nullptr, buffer, sizeof(buffer));

        const char* tok = std::strtok(buffer, ",");
        strcpy(LocalFlags[local].VariableName, tok);

        tok = std::strtok(nullptr, ",");
        if (tok != nullptr) {
            LocalFlags[local].Value = std::atoi(tok);
        }
    }

    return true;
}


/**
 *  Write scenario locals to an INI file.
 *
 *  @author: ZivDero
 */
bool ScenarioClassExtension::Write_Local_INI(INIClass& ini)
{
    static char const* const VARIABLENAMES = "VariableNames";
    char buffer[128];
    char local[10];

    ini.Clear(VARIABLENAMES);

    int length = std::size(LocalFlags);
    for (int index = 0; index < length; index++) {
        if (LocalFlags[index].VariableName[0] != '\0') {
            std::snprintf(local, sizeof(local), "%d", index);
            std::snprintf(buffer, sizeof(buffer), "%s,%d", LocalFlags[index].VariableName, LocalFlags[index].Value);
            ini.Put_String(VARIABLENAMES, local, buffer);
        }
    }

    return true;
}


/**
 *  Count how many locals are in use in this scenario.
 *
 *  @author: ZivDero
 */
int ScenarioClassExtension::Num_Locals() const
{
    int count = 0;
    for (int i = 0; i < std::size(LocalFlags); i++) {
        if (LocalFlags[i].VariableName[0] != '\0') {
            count++;
        }
    }
    return count;
}


/**
 *  Dumps global variables to the game log file.
 *
 *  @author: Rampastring
 */
void ScenarioClassExtension::Dump_Globals() const
{
    char buffer[4096];
    int bufferindex = 0;

    for (int i = 0; i < std::size(GlobalFlags); i++)
    {
        char* ptr = &buffer[bufferindex];
        int numchars = sprintf_s(ptr, std::size(buffer) - bufferindex, "%d,", GlobalFlags[i].Value);

        if (numchars < 1) {
            DEBUG_ERROR("Dump_Globals: Failed to print globals! (sprintf returned -1)");
            return;
        }

        bufferindex += numchars;
    }

    // Erase last comma for cleanness
    buffer[bufferindex] = '\0';

    DEBUG_INFO("Global variables: {}\n", buffer);
}


/**
 *  Gets the value of a global as a string.
 *
 *  @author: ZivDero
 */
static std::string Resolve_Global(const std::string& name)
{
    int value;
    if (ScenExtension->Get_Global_Value(ScenExtension->Find_Global_Variable_Index(name.c_str()), value)) {
        return std::to_string(value);
    }
    return "";
}


/**
 *  Gets the value of a local as a string.
 *
 *  @author: ZivDero
 */
static std::string Resolve_Local(const std::string& name)
{
    int value;
    if (ScenExtension->Get_Local_Value(ScenExtension->Find_Local_Variable_Index(name.c_str()), value)) {
        return std::to_string(value);
    }
    return "";
}


/**
 *  Replaces variable placeholders in a text line with the variables' values.
 *
 *  @author: ZivDero
 */
std::string ScenarioClassExtension::Substitute_Variable_Placeholders(std::string input)
{
    static const std::regex placeholder_re(R"(\{\{([^{}]*)\}\})");

    std::string result;
    std::sregex_iterator begin(input.begin(), input.end(), placeholder_re), end;
    std::size_t last_pos = 0;

    for (auto it = begin; it != end; ++it) {
        result.append(input, last_pos, it->position() - last_pos);

        const std::string name = (*it)[1];
        if (name.compare(0, 2, "g_") == 0) {
            result.append(Resolve_Global(name.substr(2)));
        }
        else if (name.compare(0, 2, "l_") == 0) {
            result.append(Resolve_Local(name.substr(2)));
        }
        else {
            // Unknown prefix: skip (i.e. replace with "")
        }

        last_pos = it->position() + it->length();
    }

    result.append(input, last_pos, std::string::npos);
    return result;
}


/**
 *  Finds the first unused local variable.
 *
 *  @author: ZivDero
 */
int ScenarioClassExtension::Find_Free_Local() const
{
    for (int index = 0; index < std::size(LocalFlags); index++) {
        if (LocalFlags[index].VariableName[0] == '\0') {
            return index;
        }
    }
    return -1;
}


/**
 *  Starts the scenario.
 *
 *  @author: 07/04/1995 JLB : Red Alert Source Code
 *           01/11/2024 ZivDero : Adjustments for Tiberian Sun
 */
bool ScenarioClassExtension::Start_Scenario(char* name, bool briefing, CampaignType campaignid)
{
    /**
     *  If there is no scenario name supplied, but we got a campaign id, fetch the scenario name from the campaign.
     */
    if ((name == nullptr || std::strlen(name) == 0) && campaignid != CAMPAIGN_NONE) {
        name = Campaigns[campaignid]->Scenario;
    }

    /**
     *  Set the current campaign ID.
     */
    Scen->Campaign = campaignid;

    DEBUG_INFO("\n----- Starting scnenario: {} -----\n", name);
    DEBUG_INFO("Player Count: {}\n", Session.Players.Count());

    /**
     *  Set the scenario name.
     */
    std::strcpy(Scen->ScenarioName, name);
    _strupr(Scen->ScenarioName);

    Theme.Stop();

    /**
     *  Play the winning movie and then start the next scenario.
     */
    CD::SetRequiredDisk(DISK_ANY);

    if (Session.Type == GAME_NORMAL) {
        if (Scen->Campaign != CAMPAIGN_NONE) {
            CD::SetRequiredDisk(Campaigns[Scen->Campaign]->WhichCD);
        }
    } else if (Session.Options.ScenarioIndex != -1) {
        MultiMission* multi = Session.Scenarios[Session.Options.ScenarioIndex];
        if (!multi->Is_Available(CD::GetCurrentDisk())) {
            CD::SetRequiredDisk(multi->Get_Disk());
        }
    }

    Session.Suspended++;

    if (CD::ForceAvailable() == false) {
        Session.Suspended--;
        return false;
    }

    Session.Suspended--;

    if (briefing && campaignid != CAMPAIGN_NONE && Scen->Scenario == 1) {

        /**
         *  #issue-95
         *
         *  Patch for handling the campaign intro movies
         *  for "The First Decade" and "Freeware TS" installations.
         *
         *  @author: CCHyper
         */
        char movie_filename[32];

        /**
         *  Fetch the campaign disk id.
         */
        CampaignClass* campaign = Campaigns[campaignid];
        DiskID cd_num = campaign->WhichCD;

        /**
         *  Check if the current campaign is an original GDI or NOD campaign.
         */
        bool is_original_gdi = (cd_num == DISK_GDI && (campaign->IniName == "GDI1" || campaign->IniName == "GDI1A") && std::string_view(campaign->Scenario) == "GDI1A.MAP");
        bool is_original_nod = (cd_num == DISK_NOD && (campaign->IniName == "NOD1" || campaign->IniName == "NOD1A") && std::string_view(campaign->Scenario) == "NOD1A.MAP");

        /**
         *  #issue-762
         *
         *  Fetch the campaign extension (if available) and get the custom intro movie.
         *
         *  @author: CCHyper
         */
        CampaignClassExtension* campaignext = Extension::Fetch(campaign);
        if (campaignext->IntroMovie[0] != '\0') {
            std::snprintf(movie_filename, sizeof(movie_filename), "%s.VQA", campaignext->IntroMovie);
            DEBUG_INFO("About to play \"{}\".\n", movie_filename);
            Play_Movie(movie_filename);
        }
        /**
         *  If this is an original Tiberian Sun campaign, play the respective intro movie.
         */
        else if (is_original_gdi || is_original_nod) {

            /**
             *  "The First Decade" and "Freeware TS" installations reshuffle
             *  the movie files due to all mix files being local now and a
             *  primitive "no-cd" added;
             *
             *  MOVIES01.MIX -> INTRO.VQA (GDI) is now INTR0.VQA
             *  MOVIES02.MIX -> INTRO.VQA (NOD) is now INTR1.VQA
             *
             *  Build the movie filename based on the current campaign's desired CD (see DiskID enum).
             */
            std::snprintf(movie_filename, sizeof(movie_filename), "INTR%d.VQA", cd_num);

            /**
             *  Now play the movie if it is found, falling back to original behavior otherwise.
             */
            if (CCFileClass(movie_filename).Is_Available()) {
                DEBUG_INFO("About to play \"{}\".\n", movie_filename);
                Play_Movie(movie_filename);

            } else if (CCFileClass("INTRO.VQA").Is_Available()) {
                DEBUG_INFO("About to play \"INTRO.VQA\".\n");
                Play_Movie("INTRO.VQA");

            } else {
                DEBUG_WARNING("Failed to find Intro movie, continuing without it.\n");
            }

        } else {
            DEBUG_WARNING("No campaign intro movie defined.\n");
        }
    }

    DEBUG_INFO("Reading scenario: {}\n", name);

    if (!Read_Scenario(name)) {
        return false;
    }

    Theme.Stop();

    if (briefing) {
        Play_Movie(Scen->IntroMovie, THEME_NONE);
        Play_Movie(Scen->BriefMovie, THEME_NONE);
    }

    /**
     *  If there's no briefing movie, restate the mission at the beginning.
     */
    char buffer[32];
    if (Scen->BriefMovie != VQ_NONE) {
        std::snprintf(buffer, std::size(buffer), "%s.VQA", Movies[Scen->BriefMovie]);
    }

    bool transit_theme_played = false;

    if (Session.Type == GAME_NORMAL && (Scen->BriefMovie == VQ_NONE || !CCFileClass(buffer).Is_Available())) {

        /**
         *  Make sure the mouse is visible before showing the restatement.
         */
        MouseCursor->Release_Mouse();
        MouseCursor->Show_Mouse();

        /**
         *  Load images for OwnerDraw graphics or the game will crash trying to present them.
         */
        OwnerDraw::Cache_Images();

        if (Scen->TransitTheme != THEME_NONE) {
            transit_theme_played = true;
            Theme.Play_Song(Scen->TransitTheme);
        }

        /**
         *  Set color for the text of the Resume Mission button.
         */
        RGBClass* textcolor = &Extension::Fetch(Sides[PlayerPtr->Class->Side])->OptionsMenuTextColor;
        OwnerDraw::TextColor1 = RGB(textcolor->Get_Red(), textcolor->Get_Green(), textcolor->Get_Blue());

        Restate_Mission(Scen);

        MouseCursor->Hide_Mouse();
        MouseCursor->Capture_Mouse();
    }

    /**
     *  Show the dropship loadout screen if this mission has a dropship.
     */
    if (Scen->StartingDropships > 0) {

        /**
         *  If the transit theme is still playing, smoothly fade it out.
         */
        if (transit_theme_played && Theme.Still_Playing()) {
            Theme.Stop(true); // Smoothly fade out the track.
        }

        /**
         *  issue-284
         *
         *  Play a background theme during the loadout menu.
         *
         *  @author: CCHyper
         */
        if (!Theme.Still_Playing()) {

            /**
             *  If DSHPLOAD is defined in THEME.INI, play that, otherwise default
             *  to playing the TS Maps theme.
             */
            ThemeType theme = Theme.From_Name("DSHPLOAD");
            if (theme == THEME_NONE) {
                theme = Theme.From_Name("MAPS");
            }

            Theme.Play_Song(theme);
        }

        MouseCursor->Release_Mouse();
        MouseCursor->Show_Mouse();

        Dropship_Loadout();

        MouseCursor->Hide_Mouse();
        MouseCursor->Capture_Mouse();

        if (Theme.Still_Playing()) {
            Theme.Stop(true); // Smoothly fade out the track.
        }
    }

    if (briefing) {
        Play_Movie(Scen->ActionMovie, Scen->TransitTheme);
    }

    /*
    *  This seems unnecessary as ThemeClass::AI already sets a track if none is playing.
    *
    if (Scen->ActionMovie != VQ_NONE || Scen->TransitTheme == THEME_NONE) {
         Theme.Queue_Song(THEME_PICK_ANOTHER);
    } else {
        Theme.Queue_Song(Scen->TransitTheme);
    }*/

    /**
     *  Set the options values, since the palette has been initialized by Read_Scenario.
     */
    Options.Set();

    /**
     *  Black out the screen.
     */
    HiddenSurface->Fill(0);
    Update_Visible_Surface();

    /**
     *  Toggle the display mode if mode toggling is allowed.
     */
    if (Debug_AllowModeToggle && (VisibleRect.Width != Options.ScreenWidth || VisibleRect.Height != Options.ScreenHeight)) {
        DEBUG_INFO("Toggle display mode to {} X {}\n", Options.ScreenWidth, Options.ScreenHeight);
        Change_Video_Mode(Options.ScreenWidth, Options.ScreenHeight);
    }

    if (Session.Type == GAME_NORMAL) {

        /**
         *  Print a message stating the current difficulty level.
         */
        char diff_message[50];

        const char* diff_name = SessionExtension->SpawnerInfo.DifficultyName.empty()
            ? CDifficulty_Name(Scen->CDifficulty)
            : SessionExtension->SpawnerInfo.DifficultyName.c_str();

        sprintf_s(diff_message, std::size(diff_message), "Difficulty: %s", diff_name);

        Session.Messages.Add_Message(nullptr, 0, diff_message, Fetch_Scheme_Index_By_Name("DarkGold"), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, Rule->MessageDelay * TICKS_PER_MINUTE);
    }

    /**
     *  Mark the game as having started.
     */
    Scen->ElapsedTimer.Start();

    ScenarioActive = true;
    TacticalActive = true;

    Show_Mouse();

    return true;
}


/**
 *  Process additions to the Rules data from the input file.
 *
 *  @author: CCHyper
 */
static bool Rule_Addition(const char* fname, bool with_digest = false)
{
    CCFileClass file(fname);
    if (!file.Is_Available()) {
        return false;
    }

    CCINIClass ini;
    if (!ini.Load(file, with_digest)) {
        return false;
    }

    DEBUG_INFO("Calling Rule->Addition() with \"{}\" overrides.\n", fname);

    Rule->Addition(ini);

    return true;
}


/**
 *  Load the scenario from the specified INI file.
 *
 *  @author: 10/07/1992 JLB - Red Alert source code.
 *           ZivDero - Adjustments for Tiberian Sun.
 *           Rampastring - Adjustments for spawner logic.
 */
bool ScenarioClassExtension::Read_Scenario_INI(CCINIClass& ini, bool random)
{
    static const char* BASIC = "Basic";
    static const char* MAP = "Map";
    char buffer[32];

    ScenarioInit++;

    DEBUG_INFO("Clearing old scenario\n");
    Clear_Scenario();

    /**
     *  Set up difficulty and fog of war settings.
     */
    if (Session.Type == GAME_NORMAL) {
        reinterpret_cast<ExtEnvironmentClass&>(Environment).Apply_Difficulty();
        Scen->Special.IsFogOfWar = false;
        Special.IsFogOfWar = false;
    } else {
        Scen->Difficulty = static_cast<DiffType>(Session.Options.AIDifficulty);
        Scen->CDifficulty = static_cast<DiffType>(2 - Scen->Difficulty);
        Scen->Special.IsFogOfWar = Session.Options.FogOfWar;
        Special.IsFogOfWar = Session.Options.FogOfWar;
    }

    /**
     *  If using the spawner, set up global variables provided by the client.
     *  This is only necessary in the first scenario, because for the rest,
     *  when using the original game's mission progression logic,
     *  the environment is already applied by Do_Win, Do_Lose and Do_Restart.
     */
    if (SessionExtension->IsSpawnerSession && Scen->Scenario == 1) {
        reinterpret_cast<ExtEnvironmentClass&>(Environment).Apply_Globals();
    }

    Scen->InitTime = ini.Get_Int(BASIC, "InitTime", 10000);
    const bool official = ini.Get_Bool(BASIC, "Official", false);

    DEBUG_INFO("[Vinifera] Starting new scenario. Playthrough ID: {}.\n", Vinifera_PlaythroughID);

    /**
     *  Make sure we have, and then enable the required addon.
     */
    if (Session.Type == GAME_NORMAL) {
        Disable_Addon(ADDON_ANY);
        Scen->RequiredAddOn = static_cast<AddonType>(ini.Get_Bool(BASIC, "RequiredAddOn", ADDON_BASE_GAME));
        Set_Required_Addon(Scen->RequiredAddOn);
        if (!Addon_Installed(Scen->RequiredAddOn)) {
            ScenarioInit--;
            return false;
        }
        Enable_Addon(Scen->RequiredAddOn);
    } else {
        Scen->RequiredAddOn = Get_Required_Addon();
    }

    Session.Loading_Callback(3);

    /**
     *  Reset the swizzle manager.
     */
    SwizzleManager.Reset();

    /**
     *  Recreate the tactical map.
     */
    DEBUG_INFO("Creating new tactical map\n");
    delete TacticalMap;
    TacticalMap = new Tactical;
    TacticalMap->Set_View_Dimensions(TacticalRect);

    /**
     *  Initialize the theater.
     */
    DEBUG_INFO("Initializing Theater\n");
    Scen->Theater = ini.Get_TheaterType(MAP, "Theater", THEATER_FIRST);
    Init_Theater(Scen->Theater);
    Session.Loading_Callback(8);

    /**
     *  Load the main rules file.
     */
    DEBUG_INFO("Initializing Rules\n");
    RuleExtension->Initialize(*RuleINI);
    Rule->Initialize(*RuleINI);

    Session.Loading_Callback(15);
    Call_Back();

    /**
     *  Read the rules into ScenarioClass.
     */
    DEBUG_INFO("Calling Scen->Read_Global_INI(*RuleINI);\n");
    Scen->Read_Global_INI(*RuleINI);

    Call_Back();

    /**
     *  #issue-#671
     *
     *  Add loading of MPLAYER.INI to override Rules data for multiplayer games.
     *
     *  @author: CCHyper
     */
    if (Session.Type != GAME_NORMAL && Session.Type != GAME_WDT) {

        /**
         *  Process the multiplayer ini overrides.
         */
        Rule_Addition("MPLAYER.INI");
        if (Addon_Enabled(ADDON_FIRESTORM)) {
            Rule_Addition("MPLAYERFS.INI");
        }
    }

    Session.Loading_Callback(30);

    Call_Back();

    /**
     *  Read scenario overrides into our Rules.
     */
    DEBUG_INFO("Calling Rule->Addition() with scenario overrides\n");
    Rule->Addition(ini);
    DEBUG_INFO("Finished Rule->Addition() with scenario overrides\n");

    Session.Loading_Callback(45);

    /**
     *  Read in the specific information for each of the house types. This creates
     *  the houses of different types.
     */
    if (Session.Type == GAME_NORMAL) {
        DEBUG_INFO("Reading in scenario house types\n");
        HouseClass::Read_Scenario_INI(ini);
    }

    /**
     *  Init the Scenario CRC value
     */
    ScenarioCRC = 0;

    /**
     *  Read scenario data from the scenario INI.
     */
    if (!Scen->Read_INI(ini) || !ScenExtension->Read_INI(ini)) {
        ScenarioInit--;
        return false;
    }

    Session.Loading_Callback(50);

    /**
     *  Determine the player's side.
     */
    if (Session.Type == GAME_NORMAL) {
        ini.Get_String(BASIC, "Player", "GDI", buffer, 32);

        /**
         *  Fetch the house's side and use this to decide which assets to load.
         */
        const auto housetype = HouseTypes[HouseTypeClass::From_Name(buffer)];

        ScenExtension->House = housetype->HeapID;
        Scen->SpeechSide = housetype->Side;
        ScenExtension->SidebarSide = housetype->Side;
        Scen->Special.IsFogOfWar = false;
        Special.IsFogOfWar = false;

    } else {

        /**
         *  Fetch the house's side and use this to decide which assets to load.
         */
        const auto housetype = HouseTypes[SessionExtension->House];

        ScenExtension->House = housetype->HeapID;
        Scen->SpeechSide = housetype->Side;
        ScenExtension->SidebarSide = housetype->Side;
        Scen->Special.IsFogOfWar = Session.Options.FogOfWar;
        Special.IsFogOfWar = Session.Options.FogOfWar;
    }

    /**
     *  Init side-specific data.
     */
    DEBUG_INFO("Calling Prep_For_Side()\n");
    if (!Prep_For_Side(ScenExtension->SidebarSide)) {
        ScenarioInit--;
        return false;
    }

    Call_Back();

    /**
     *  In single player, the speech and sidebar side can be overridden by the scenario.
     */
    if (Session.Type == GAME_NORMAL) {
        Scen->SpeechSide = ini.Get_SideType("Basic", "SpeechSide", Scen->SpeechSide);
        ScenExtension->SidebarSide = ini.Get_SideType("Basic", "SidebarSide", ScenExtension->SidebarSide);
    }

    /**
     *  Init the speech for the side.
     */
    DEBUG_INFO("Calling Prep_Speech_For_Side()\n");
    if (!Prep_Speech_For_Side(Scen->SpeechSide)) {
        ScenarioInit--;
        return false;
    }

    Session.Loading_Callback(58);

    Scen->Read_Waypoints(ini);

    /**
     *  Outside of campaign, assign houses their starting positions.
     *  This used to happen in Create_Units(), but needs to happen earlier
     *  so that we can handle Spawn houses.
     */
    if (Session.Type != GAME_NORMAL) {
        ScenExtension->Assign_Starting_Positions(official);
    }

    /**
     *  Outside of campaign, whether the bridges are destructible can be controlled.
     */
    if (Session.Type != GAME_NORMAL && !Session.Options.BridgeDestruction) {
        Special.IsDestroyableBridges = false;
    }
    // Special.Apply_To_Game(); // does nothing
    Call_Back();

    /**
     *  Outside of campaign, the scenario may request that we read base nodes for
     *  Spawn houses. Do that if necessary.
     */
    if (Session.Type != GAME_NORMAL && ScenExtension->IsUseMPAIBaseNodes) {
        for (int i = 0; i < Session.Players.Count() + Session.Options.AIPlayers; i++) {

            /**
             *  Skip observers, they don't need base nodes.
             */
            const auto houseext = Extension::Fetch(Houses[i]);
            if (houseext->IsObserver) {
                continue;
            }

            /**
             *  Read base nodes for this house.
             */
            std::snprintf(buffer, std::size(buffer), "Spawn%d", houseext->SpawnWaypoint + 1);
            Houses[i]->Base.Read_INI(ini, buffer);
        }
    }

    /**
     *  Read in the team type data. The team types must be created before any
     *  triggers can be created.
     */
    TeamTypeClass::Read_Scenario_INI(AIINI, true);
    if (Addon_Enabled(ADDON_FIRESTORM)) {
        TeamTypeClass::Read_Scenario_INI(FSAIINI, true);
    }
    TeamTypeClass::Read_Scenario_INI(ini, false);

    /**
     *  Read in the script type data.
     */
    ScriptTypeClass::Read_Scenario_INI(AIINI, true);
    if (Addon_Enabled(ADDON_FIRESTORM)) {
        ScriptTypeClass::Read_Scenario_INI(FSAIINI, true);
    }
    ScriptTypeClass::Read_Scenario_INI(ini, false);

    /**
     *  Read in the task force data.
     */
    TaskForceClass::Read_Scenario_INI(AIINI, true);
    if (Addon_Enabled(ADDON_FIRESTORM)) {
        TaskForceClass::Read_Scenario_INI(FSAIINI, true);
    }
    TaskForceClass::Read_Scenario_INI(ini, false);

    /**
     *  Read in the trigger data. The triggers must be created before any other
     *  objects can be initialized.
     */
    TriggerTypeClass::Read_Scenario_INI(ini);

    /**
     *  Read in the trigger tag data.
     */
    TagTypeClass::Read_Scenario_INI(ini);

    /**
     *  Read in the AI trigger data.
     */
    AITriggerTypeClass::Read_Scenario_INI(AIINI, true);
    if (Addon_Enabled(ADDON_FIRESTORM)) {
        AITriggerTypeClass::Read_Scenario_INI(FSAIINI, true);
    }
    AITriggerTypeClass::Read_Scenario_INI(ini, 0);

    Session.Loading_Callback(60);

    /**
     *  Read in the map control values. This includes dimensions
     *  as well as theater information.
     */
    Map.Read_INI(ini);
    Call_Back();

    /**
     *  Read in the tunnel values.
     */
    TubeClass::Read_Scenario_INI(ini);

    /**
     *  Buildings that convert into isometric tiles need to have
     *  pointers to those tiles fetched now.
     */
    BuildingTypeClass::Fetch_ToTile_Types();

    Map.Flag_To_Redraw(2);

    Session.Loading_Callback(70);
    Call_Back();

    /**
     *  Read in any normal overlay objects.
     */
    OverlayClass::Read_INI(ini);
    Call_Back();

    /**
     *  Recalc the attributes of all cells of the map.
     */
    Map.Reset_Iterator();
    for (CellClass* cell = Map.Iterate(); cell; cell = Map.Iterate()) {
        cell->Recalc_Attributes(-1);
    }

    /**
     *  Place veins onto the map.
     */
    OverlayClass::Scenario_Load_Fixup_Veins();

    /**
     *  Read in and place the 3D terrain objects.
     */
    TerrainClass::Read_INI(ini);
    Call_Back();

    /**
     *  Place veinhole monsters onto the map.
     */
    VeinholeMonsterClass::Place_Monsters(true);

    /**
     *  Initialize Tiberium.
     */
    TiberiumClass::Initialize_Tiberium_Growth_System();
    TiberiumClass::Initialize_Tiberium_Spread_System();

    Session.Loading_Callback(72);

    /**
     *  Do something with the radar.
     */
    Map.Compute_Radar_Image();

    /**
     *  Read in and place the units (all sides).
     */
    UnitClass::Read_INI(ini);
    Call_Back();
    Session.Loading_Callback(74);

    /**
     *  Read in and place the aircraft units (all sides).
     */
    AircraftClass::Read_INI(ini);
    Call_Back();

    /**
     *  Read in and place the infantry units (all sides).
     */
    InfantryClass::Read_INI(ini);
    Call_Back();
    Session.Loading_Callback(76);

    /**
     *  Read in and place all the buildings on the map.
     */
    LightSourceClass::Recalc = false;
    BuildingClass::Read_INI(ini);
    Call_Back();
    Session.Loading_Callback(78);

    LightSourceClass::Recalc = true;
    Call_Back();

    /**
     *  Read in any smudge overlays.
     */
    SmudgeClass::Read_INI(ini);
    Call_Back();

    CCINIClass mini;
    CCFileClass file;

    if (Session.Type == GAME_NORMAL) {

        /**
         *  Reload the rules with out scenario file again? Not sure why.
         */
        _splitpath(Scen->ScenarioName, nullptr, nullptr, buffer, nullptr);
        std::strncat(buffer, ".INI", std::size(buffer) - 1);

        file.Set_Name(buffer);
        if (file.Is_Available(false)) {
            mini.Load(file, false);
            Rule->Addition(mini);
        }
        file.Close();

        /**
         *  Read the name and briefing of the mission from the MISSION.INI file.
         */
        if (Scen->RequiredAddOn > ADDON_BASE_GAME) {
            char fname[32];
            std::snprintf(fname, std::size(fname), "MISSION%1d.INI", Scen->RequiredAddOn);
            file.Set_Name(fname);
        } else {
            file.Set_Name("MISSION.INI");
        }


        if (file.Is_Available(false)) {
            mini.Load(file, false);

            if (mini.Is_Present("Name")) {
                mini.Get_String(Scen->ScenarioName, "Name", "", Scen->Description, std::size(Scen->Description));
            }

            if (mini.Is_Present("Briefing")) {
                mini.Get_String(Scen->ScenarioName, "Briefing", "", buffer, std::size(buffer));
                if (std::strlen(buffer) > 0) {
                    mini.Get_TextBlock(buffer, Scen->BriefingText, std::size(Scen->BriefingText));
                }
            }
        }
    }

    /**
     *  WW's "TheTeam" cheat.
     */
    if (Session.Type == GAME_SKIRMISH && Cheat_TheTeam) {
        file.Close();
        file.Set_Name("TMCJ4F.INI");

        if (file.Is_Available(false)) {
            mini.Load(file, false, false);
            Rule->Addition(mini);
        }
    }

    Session.Loading_Callback(82);
    Call_Back();

    /**
     *  Do some last passes on some map stuff.
     */
    Map.Overpass();
    Session.Loading_Callback(86);
    Call_Back();

    Session.Loading_Callback(90);
    Call_Back();

    /**
     *  Multi-player last-minute fixups
     */
    if (Session.Type != GAME_NORMAL && !random) {
        Last_Minute_Multiplayer_Fixups(official);
    }

    if (Session.Type != GAME_NORMAL) {
        Init_Forced_Alliances();
    }

    Call_Back();

    /**
     *  Reset the swizzle manager.
     */
    SwizzleManager.Reset();
    Session.Loading_Callback(96);
    Call_Back();

    /**
     *  Remove all inactive objects.
     */
    Delete_Marked();

    /**
     *  Outside of campaign, the scenario's special flags are not used.
     */
    if (Session.Type != GAME_NORMAL) {
        Scen->Special = Special;
    }

    ScenarioInit--;

    /**
     *  Set up laser fences.
     */
    int save_init = ScenarioInit;
    ScenarioInit = 0;
    BuildingClass::Init_Laser_Fences();
    ScenarioInit = save_init;

    Session.Loading_Callback(98);
    Call_Back();

    Map.Clear_Background_Update_Stack();

    /**
     *  If we have FoW turned on, fog the entire map.
     */
    if (Scen->Special.IsFogOfWar) {
        Map.Initialize_Fog_System();
    }

    /**
     *  Refresh the radar.
     */
    RadarEventClass::Clear();
    Map.Total_Radar_Refresh();

    /**
     *  Schedule the next autosave.
     */
    SessionExtension->Schedule_Next_Autosave();

    /**
     *  Apply the spawner's score-screen override after the scenario has been
     *  read, because Clear_Scenario resets ScenarioClass state.
     */
    if (SessionExtension->IsSpawnerSession) {
        Scen->IsSkipScore |= SessionExtension->ExtOptions.IsSkipScoreScreen;
    }

    /**
     *  Return with flag saying that the scenario file was read.
     */
    return true;
}


/**
 *  Creates alliances as dictated by the client.
 *
 *  @author: ZivDero
 */
void ScenarioClassExtension::Init_Forced_Alliances()
{
    /**
     *  Process the client's forced alliances.
     */
    if (SessionExtension->IsSpawnerSession) {
        for (int i = 0; i < Session.Players.Count() + Session.Options.AIPlayers; i++) {
            HouseClass* housep = Houses[i];

            /**
             *  Multiplay passive houses don't get allies.
             */
            if (housep->Class->IsMultiplayPassive) continue;

            const auto& slot_info = SessionExtension->SlotInfo[i];
            for (int j = 0; j < std::size(slot_info.Alliances); j++) {
                const int ally_index = slot_info.Alliances[j];
                if (ally_index != -1) housep->Make_Ally(static_cast<HousesType>(ally_index));
            }
        }
    }
}


/**
 *  Build a list of valid multiplayer starting waypoints.
 *
 *  @author: CCHyper
 */
static DynamicVectorClass<Cell> _Fetch_Starting_Points(bool official)
{
    DynamicVectorClass<Cell> list;

    /**
     *  Find first valid player spawn waypoint.
     */
    int avail_waypoints = 0;
    for (int i = 0; i < MAX_PLAYERS; i++) {
        if (Scen->Is_Waypoint_Valid(i)) {
            avail_waypoints++;
        } else {
            break;
        }
    }

    /**
     *  Calculate the number of waypoints (as a minimum) that will be lifted from the
     *  mission file. Bias this number so that only the first 4 waypoints are used
     *  if there are 4 or fewer players. Unofficial maps will pick from all the
     *  available waypoints.
     */
    int look_for = std::max(avail_waypoints, Session.Players.Count() + Session.Options.AIPlayers);
    if (!official) {
        look_for = MAX_PLAYERS;
    }

    if (SessionExtension->IsSpawnerSession) {
        for (int i = 0; i < Session.Players.Count() + Session.Options.AIPlayers; i++) {
            const auto houseext = Extension::Fetch(Houses[i]);
            if (houseext->IsObserver) {
                look_for--;
            }
        }
    }

    int missing_waypoints = 0;
    for (int waycount = 0; waycount < look_for; ++waycount) {
        if (Scen->Is_Waypoint_Valid(waycount)) {
            list.Add(Scen->Waypoint_Cell(waycount));
            DEBUG_INFO("Multiplayer start waypoint found at cell {},{}.\n", Scen->Waypoint_Cell(waycount).X, Scen->Waypoint_Cell(waycount).Y);
        } else {
            /**
             *  Preserve the waypoint ID as the vector index. Create_Units will
             *  replace this virtual location with a valid random cell.
             */
            list.Add(CELL_NONE);
            missing_waypoints++;
        }
    }

    if (missing_waypoints > 0) {
        DEBUG_WARNING("Multiplayer start waypoint deficiency - injected {} virtual start positions.\n", missing_waypoints);
    }

    return list;
}


/**
 *  Assigns starting positions to multiplayer houses.
 *  Split from Create_Units().
 *
 *  @author: ZivDero, CCHyper
 */
void ScenarioClassExtension::Assign_Starting_Positions(bool official)
{
    int numtaken = 0;

    /**
     *  Build a list of the valid waypoints. This normally shouldn't be
     *  necessary because the scenario level designer should have assigned
     *  valid locations to the first N waypoints, but just in case, this
     *  loop verifies that.
     */
    bool taken[26] = {};

    DynamicVectorClass<Cell> starting_points = _Fetch_Starting_Points(official);

    DEV_DEBUG_INFO("Assigning starting positions to houses.\n");

    /**
     *  If the spawner is active, assign the received starting positions to the houses.
     */
    if (SessionExtension->IsSpawnerSession) {
        for (int house = 0; house < Session.Players.Count() + Session.Options.AIPlayers; house++) {
            Extension::Fetch(Houses[house])->SpawnWaypoint = SessionExtension->SlotInfo[house].SpawnLocation;
        }
    }

    /**
     *  First pass - assign spawn waypoints given to use by the client.
     */
    if (SessionExtension->IsSpawnerSession) {
        for (int house = HOUSE_FIRST; house < Houses.Count(); house++) {

            /**
             *  Get a pointer to this house.
             */
            HouseClass* hptr = Houses[house];
            ASSERT(hptr != nullptr);

            /**
             *  Skip passive houses.
             */
            if (hptr->Class->IsMultiplayPassive) {
                continue;
            }

            /**
             *  Skip observers for now, we'll set them to observe another player later.
             */
            const auto houseext = Extension::Fetch(hptr);
            if (houseext->IsObserver) {
                houseext->SpawnWaypoint = WAYPOINT_NONE;
                continue;
            }

            const int chosen_spawn = houseext->SpawnWaypoint;
            if (chosen_spawn >= 0 && chosen_spawn < starting_points.Count() && !taken[chosen_spawn]) {
                taken[chosen_spawn] = true;
                numtaken++;
            } else {
                /**
                 *  Invalid and duplicate client choices are assigned normally
                 *  in the second pass.
                 */
                houseext->SpawnWaypoint = WAYPOINT_NONE;
            }
        }
    }

    /**
     *  Second pass - assign spawn waypoints to houses that don't have one yet.
     */
    for (int house = HOUSE_FIRST; house < Houses.Count(); house++) {

        /**
         *  Get a pointer to this house.
         */
        HouseClass* hptr = Houses[house];
        ASSERT(hptr != nullptr);
        auto houseext = Extension::Fetch(hptr);

        /**
         *  Skip passive houses.
         */
        if (hptr->Class->IsMultiplayPassive) {
            continue;
        }

        /**
         *  Skip observers for now, we'll set them to observe another player later.
         */
        if (houseext->IsObserver) {
            continue;
        }

        /**
         *  Skip houses that already have a waypoint.
         */
        if (houseext->SpawnWaypoint != WAYPOINT_NONE) {
            continue;
        }

        /**
         *  Pick the starting location for this house. The first house just picks
         *  one of the valid locations at random. The other houses pick the furthest
         *  waypoint from the existing houses.
         */
        if (numtaken == 0) {
            int pick = Random_Pick(0, starting_points.Count() - 1);
            while (taken[pick]) {
                pick = Random_Pick(0, starting_points.Count() - 1);
            }
            taken[pick] = true;
            numtaken++;
            houseext->SpawnWaypoint = pick;

        } else {

            /**
             *  Set all waypoints to have a score of zero in preparation for giving
             *  a distance score to all waypoints.
             */
            int score[std::size(taken)] = {};

            /**
             *  Scan through all waypoints and give a score as a value of the sum
             *  of the distances from this waypoint to all taken waypoints.
             */
            for (int index = 0; index < starting_points.Count(); index++) {

                /**
                 *  If this waypoint has not already been taken, then accumulate the
                 *  sum of the distance between this waypoint and all other taken
                 *  waypoints.
                 */
                if (!taken[index]) {
                    for (int trypoint = 0; trypoint < starting_points.Count(); trypoint++) {

                        if (taken[trypoint]) {
                            score[index] += Distance(starting_points[index], starting_points[trypoint]);
                        }
                    }
                }
            }

            /**
             *  Now find the waypoint with the largest score. This waypoint is the one
             *  that is furthest from all other taken waypoints.
             */
            int best = -1;
            int bestvalue = INT_MIN;
            for (int searchindex = 0; searchindex < starting_points.Count(); searchindex++) {
                if (!taken[searchindex] && score[searchindex] > bestvalue) {
                    bestvalue = score[searchindex];
                    best = searchindex;
                }
            }

            if (best < 0) {
                DEBUG_ERROR("Unable to assign a unique multiplayer start waypoint to house {}.\n", house);
                houseext->SpawnWaypoint = WAYPOINT_NONE;
                continue;
            }

            /**
             *  Assign this best position to the house.
             */
            taken[best] = true;
            numtaken++;
            houseext->SpawnWaypoint = best;
        }
    }

    
    /**
     *  Third pass - give observers someone to observe, and assign everyone their house centers.
     */
    for (int house = HOUSE_FIRST; house < Houses.Count(); house++) {
        Cell centroid(0, 0); // centroid of this house's stuff.

        /**
         *  Get a pointer to this house.
         */
        HouseClass* hptr = Houses[house];
        ASSERT(hptr != nullptr);

        auto houseext = Extension::Fetch(hptr);

        /**
         *  Skip passive houses.
         */
        if (hptr->Class->IsMultiplayPassive) {
            continue;
        }

        /**
         *  Observers now pick a random house to observe.
         */
        if (houseext->IsObserver) {

            /**
             *  No players - just plop the spectator in the map center.
             */
            if (numtaken == 0) {
                centroid = Cell(Map.MapRect.X + Map.MapRect.Width / 2, Map.MapRect.Y + Map.MapRect.Height / 2);
            }

            /**
             *  Pick a random house to observe.
             */
            else {
                std::vector<int> valid_taken_waypoints;
                for (int waypoint = 0; waypoint < starting_points.Count(); ++waypoint) {
                    if (taken[waypoint] && starting_points[waypoint] != CELL_NONE) {
                        valid_taken_waypoints.push_back(waypoint);
                    }
                }

                if (valid_taken_waypoints.empty()) {
                    centroid = Cell(Map.MapRect.X + Map.MapRect.Width / 2, Map.MapRect.Y + Map.MapRect.Height / 2);
                } else {
                    const int pick = valid_taken_waypoints[Random_Pick(0u, static_cast<unsigned int>(valid_taken_waypoints.size() - 1))];
                    centroid = starting_points[pick];
                }
            }

            /**
             *  Ensure that observers do not have a spawn waypoint.
             */
            houseext->SpawnWaypoint = WAYPOINT_NONE;

        } else {

            /**
             *  For normal players, the centroid is their starting waypoint.
             */
            if (houseext->SpawnWaypoint >= 0 && houseext->SpawnWaypoint < starting_points.Count() && starting_points[houseext->SpawnWaypoint] != CELL_NONE) {
                centroid = starting_points[houseext->SpawnWaypoint];
            } else {
                centroid = Cell(Map.MapRect.X + Map.MapRect.Width / 2, Map.MapRect.Y + Map.MapRect.Height / 2);
            }
        }

        /**
         *  Assign the center of this house to the waypoint location.
         */
        hptr->Center = centroid.As_Coord();
        DEBUG_INFO("  House {} ({}) starting at waypoint {} ({},{})\n", (int)house, hptr->IniName, houseext->SpawnWaypoint, centroid.X, centroid.Y);
    }
}


/**
 *  Assigns multiplayer houses to various players.
 *
 *  @author: 06/09/1995 BRR - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 */
void ScenarioClassExtension::Assign_Houses()
{
    bool assigned[MAX_PLAYERS] = {};   // true = this house slot is in use.
    std::vector<bool> color_used(ColorSchemes.Count()); // true = this color is in use.

    DEBUG_INFO("Assign_Houses(enter)\n");

    if (Session.Players.Count() > 0) {
        DEBUG_INFO("  Assigning players ({})...\n", Session.Players.Count());
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
        int index = 0;
        int lowest_color = -1;
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

        NodeNameTag& node = *Session.Players[index];

        /**
         *  Mark this player as having been assigned.
         */
        assigned[index] = true;
        color_used[node.Player.Color] = true;

        /**
         *  Assign the lowest-colored player to the next available slot
         *  in the HouseClass array.
         */
        HouseClass* housep = new HouseClass(HouseTypes[node.Player.House]);
        HouseClassExtension* houseext = Extension::Fetch(housep);
        
        housep->IniName = node.Name;

        housep->IsHuman = true;
        /**
         *  Set the house's properties.
         */
        housep->Init_Data(node.Player.Color, node.Player.House, Session.Options.Credits);
        housep->Scheme = Session.Scheme_From_Color_ID(node.Player.Color);
        housep->Initialize_Radar_Color();

        /**
         *  If this ID is for myself, set up PlayerPtr.
         */
        if (index == 0) {
            PlayerPtr = housep;
            housep->IsPlayerControl = true;
        }

        /**
         *  Convert the build level into an actual tech level to assign to the house.
         *  There isn't a one-to-one correspondence.
         */
        housep->Control.TechLevel = BuildLevel;

        housep->Assign_Handicap(DIFF_NORMAL);

        /**
         *  Process spawner overrides.
         */
        if (SessionExtension->IsSpawnerSession) {
            int house_index = Houses.Count() - 1;
            const auto& slot_info = SessionExtension->SlotInfo[house_index];

            /**
             *  Mark an observer accordingly.
             */
            if (slot_info.IsObserver) {
                houseext->IsObserver = true;

                /**
                 *  If the local player starts as an observer, mark him as "Obi Wan" -
                 *  prevents him from sending DMs to players (and removes fog).
                 */
                if (housep == PlayerPtr) {
                    Session.ObiWan = true;
                }
            }
        }

        /**
         *  Record where we placed this player.
         */
        node.Player.ID = housep->HeapID;

        DEBUG_INFO("    Assigned player \"{}\" (House: \"{}\", ID: {}, Color: \"{}\") to slot {}.\n",
            node.Name, housep->Class->Name(), (int)node.Player.ID, ColorSchemes[housep->Scheme]->Name, i);
    }

    if (Session.Options.AIPlayers > 0) {
        DEBUG_INFO("  Assigning computer players ({})...\n", Session.Options.AIPlayers);
    }

    /**
     *  Now assign computer players to the remaining houses.
     */
    for (int i = Session.Players.Count(); i < Session.Players.Count() + Session.Options.AIPlayers; ++i) {
        HousesType pref_house;
        int color;

        if (!SessionExtension->IsSpawnerSession || !SessionExtension->SlotInfo[i].IsConfigured) {

            /**
             *  #issue-7
             *
             *  Fixes a limitation where the AI would only be able to choose
             *  between the houses GDI (0) and Nod (1). Now, all houses that
             *  have "IsMultiplay" true will be considered for sellection.
             */
            while (true) {
                pref_house = static_cast<HousesType>(Random_Pick(0, HouseTypes.Count() - 1));
                if (HouseTypes[pref_house]->IsMultiplay) {
                    break;
                }
            }

            /**
             *  Pick a color for this house; keep looping until we find one.
             */
            while (true) {
                color = Random_Pick(0, (MAX_PLAYERS - 1));
                if (color_used[color] == false) {
                    break;
                }
            }
            color_used[color] = true;
        } else {
            const auto& slot_info = SessionExtension->SlotInfo[i];
            color = slot_info.Color;
            pref_house = static_cast<HousesType>(slot_info.House);
        }

        HouseClass* housep = new HouseClass(HouseTypes[pref_house]);

        /**
         *  Set up the house.
         */
        housep->Control.TechLevel = BuildLevel;
        housep->IsHuman = false;

        housep->Init_Data(static_cast<PlayerColorType>(color), pref_house, Session.Options.Credits);

        housep->Scheme = Session.Scheme_From_Color_ID(static_cast<PlayerColorType>(color));
        housep->Initialize_Radar_Color();

        housep->IniName = Text_String(TXT_COMPUTER);

        if (Session.Type != GAME_NORMAL) {
            housep->IQ = Rule->MaxIQ;
        }

        DiffType difficulty = Scen->CDifficulty;

        if (Session.Players.Count() > 1 && Rule->IsCompEasyBonus && difficulty > DIFF_EASY) {
            difficulty = static_cast<DiffType>(difficulty - 1);
        }
        housep->Assign_Handicap(difficulty);

        /**
         *  Process spawner overrides.
         */
        if (SessionExtension->IsSpawnerSession) {
            const auto& slot_info = SessionExtension->SlotInfo[i];

            /**
             *  Set the difficulty and name for the AI (for AIs, player index == house index)
             */
            if (slot_info.Difficulty >= DIFF_FIRST && slot_info.Difficulty < EXT_DIFF_COUNT) {
                housep->Assign_Handicap(static_cast<DiffType>(slot_info.Difficulty));
                if (SessionExtension->ExtOptions.IsAINamesByDifficulty && !housep->IsHuman) {
                    housep->IniName = std::string(CDifficulty_Name(static_cast<DiffType>(slot_info.Difficulty))) + " AI";
                }
            }

            if (slot_info.IsObserver) {
                Extension::Fetch(housep)->IsObserver = true;
            }
        }

        DEBUG_INFO("    Assigned computer house \"{}\" (ID: {}, Color: \"{}\") to slot {}.\n",
            housep->Class->Name(), (int)housep->HeapID, ColorSchemes[housep->Scheme]->Name, i);
    }

    /**
     *  Create Neutral and Special houses as they must exist!
     *
     *  #BUGFIX:
     *  Added checks to make sure the houses exist before blindly
     *  attempting to create a instance of them.
     */
    ColorSchemeType scheme_lgrey = Fetch_Scheme_Index_By_Name("LightGrey");
    ColorSchemeType scheme_grey = Fetch_Scheme_Index_By_Name("Grey");

    HousesType house = HouseTypeClass::From_Name("Neutral");
    if (house != HOUSE_NONE) {
        DEBUG_INFO("  Creating Neutral house...\n");

        HouseTypeClass* housetype = HouseTypes[house];
        HouseClass* housep = new HouseClass(housetype);

        /**
         *  #issue-773
         *
         *  Allow the remap colour of Neutral to be overriden. Due to the difference
         *  in the colours used between RULES.INI and scenarios for official maps, we
         *  need to check for both LightGrey and Grey, and only allow overrides
         *  if it does not match these colors.
         *
         *  @author: CCHyper
         */
        if (housetype->Scheme != scheme_lgrey && housetype->Scheme != scheme_grey) {
            housep->Scheme = housetype->Scheme;
        } else {
            housep->Scheme = scheme_lgrey;
        }

        housep->Initialize_Radar_Color();
    }

    house = HouseTypeClass::From_Name("Special");
    if (house != HOUSE_NONE) {
        DEBUG_INFO("  Creating Special house...\n");

        HouseTypeClass* housetype = HouseTypes[house];
        HouseClass* housep = new HouseClass(housetype);

        /**
         *  #issue-773
         *
         *  Allow the remap colour of Special to be overriden. Due to the difference
         *  in the colours used between RULES.INI and scenarios for official maps, we
         *  need to check for both LightGrey and Grey, and only allow overrides
         *  if it does not match these colors.
         *
         *  @author: CCHyper
         */
        if (housetype->Scheme != scheme_lgrey && housetype->Scheme != scheme_grey) {
            housep->Scheme = housetype->Scheme;
        } else {
            housep->Scheme = scheme_lgrey;
        }

        housep->Initialize_Radar_Color();
    }

    /**
     *  Process the spawner's forced alliances.
     */
    if (SessionExtension->IsSpawnerSession) {
        for (int i = 0; i < Session.Players.Count() + Session.Options.AIPlayers; i++) {
            HouseClass* housep = Houses[i];

            /**
             *  Multiplay passive houses don't get allies.
             */
            if (housep->Class->IsMultiplayPassive) continue;

            const auto& slot_info = SessionExtension->SlotInfo[i];
            for (int j = 0; j < std::size(slot_info.Alliances); ++j) {
                const int ally_index = slot_info.Alliances[j];
                if (ally_index != -1) {
                    housep->Allies |= 1 << ally_index;
                }
            }
        }
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
    int xmin = Map.MapRect.X;
    int xmax = xmin + Map.MapRect.Width - 1;
    int ymin = Map.MapRect.Y;
    int ymax = ymin + Map.MapRect.Height - 1;

    /**
     *  Adjust the x-coordinate.
     */
    int xdist = Random_Pick(0, maxdist);
    if (Percent_Chance(50)) {
        x += xdist;
        x = std::min(x, xmax);
    } else {
        x -= xdist;
        x = std::max(x, xmin);
    }

    /**
     *  Adjust the y-coordinate.
     */
    int ydist = Random_Pick(0, maxdist);
    if (Percent_Chance(50)) {
        y += ydist;
        y = std::min(y, ymax);
    } else {
        y -= ydist;
        y = std::max(y, ymin);
    }

    return Cell(x, y);
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
    int xmin = Map.MapRect.X;
    int xmax = xmin + Map.MapRect.Width - 1;
    int ymin = Map.MapRect.Y;
    int ymax = ymin + Map.MapRect.Height - 1;

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
    x = std::min(x, xmax);
    x = std::max(x, xmin);

    y = std::min(y, ymax);
    y = std::max(y, ymin);

    return Cell(x, y);
}


/**
 *  Places an object >near< the given cell.
 * 
 *  @author: 06/09/1995 BRR - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 * 
 *  #issue-338 - Adds "min_dist" argument.
 */
static int _Scan_Place_Object(ObjectClass *obj, Cell cell, int min_dist = 1, int max_dist = 31, bool no_scatter = false)
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
        if (!techno || (techno->RTTI == RTTI_INFANTRY &&
            obj->RTTI == RTTI_INFANTRY)) {
            Coord coord = cell.As_Coord();
            coord.Z = Map.Get_Height_GL(coord);
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
    for (dist = min_dist; dist <= max_dist; dist++) {

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
                if (!no_scatter && tryval > 0) {
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
                    if (!techno || (techno->RTTI == RTTI_INFANTRY &&
                        obj->RTTI == RTTI_INFANTRY)) {
                        Coord coord = newcell.As_Coord();
                        coord.Z = Map.Get_Height_GL(coord);
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
 *  Checks if the cell adjacent from the input cell is occupied.
 * 
 *  @author: CCHyper
 */
static bool Is_Adjacent_Cell_Empty(Cell cell, FacingType facing, int dist)
{
    Cell newcell;
    TechnoClass *techno;

    /**
     *  Pick a coordinate along this directional axis
     */
    newcell = Clip_Move(cell, facing, dist);

    /**
     *  Is there already an object on this cell?
     */
    techno = Map[newcell].Cell_Techno();
    if (!techno) {
        return true;
    }
    
    /**
     *  Is there any free infantry spots?
     */
    if (techno->RTTI == RTTI_INFANTRY
        && Map[newcell].Is_Any_Spot_Free()) {

        return true;
    }

    return false;
}


/**
 *  Finds a random starting waypoint position for a specific house.
 *
 *  @author: Rampastring
 */
bool ScenarioClassExtension::Assign_Random_Starting_Position(HouseClass* house)
{
    DEBUG_INFO("Looking for a random starting location for house {}.\n", (int)house->HeapID);

    HouseClassExtension* houseext = Extension::Fetch(house);

    Cell bestcell = CELL_NONE;
    int bestscore = INT_MIN;
    int maxtries = 20;

    /**
     *  Check a number of potential candidate cells depending on RNG.
     *  Calculate scores for them and pick the best one.
     */
    for (int tries = 0; tries < maxtries; tries++) {
        Cell trycell = Cell(Map.MapRect.X + Random_Pick(10, Map.MapRect.Width - 10), Map.MapRect.Y + Random_Pick(0, Map.MapRect.Height - 10) + 10);

        trycell = Map.Nearby_Location(trycell, SPEED_TRACK, -1, MZONE_NORMAL, false, Point2D(8, 8), true, false, false, false);
        if (trycell != CELL_NONE && Map[trycell].Cell_Terrain() == nullptr) {

            /**
             *  Calculate a score for this candidate cell. The farther away it is from any existing starting location, the better.
             */
            int lowestdistance = INT_MAX;

            for (int i = 0; i < MAX_PLAYERS; i++) {
                Cell wpcell = Scen->Waypoint_Cell((WAYPOINT)i);
                if (wpcell != CELL_NONE) {
                    int distance = ::Distance(trycell, wpcell);
                    if (distance < lowestdistance) {
                        lowestdistance = distance;
                    }
                }
            }

            if (lowestdistance > bestscore) {
                bestcell = trycell;
                bestscore = lowestdistance;
            }
        }
    }

    if (bestcell != CELL_NONE) {
        Scen->Set_Waypoint(houseext->SpawnWaypoint, bestcell);
        DEBUG_INFO("Random multiplayer start waypoint placed at cell {},{}.\n", bestcell.X, bestcell.Y);
        return true;
    }

    return false;
}


/**
 *  New implementation of Create_Units()
 *
 *  @author: CCHyper (assistance from tomsons26).
 */
void ScenarioClassExtension::Create_Units(bool official)
{
    /**
     *  #issue-338
     *
     *  Change the starting unit formation to be like Red Alert 2.
     *
     *  This sets the desired placement distance from the base center cell.
     *
     *  @author: CCHyper
     */
    const unsigned int MIN_PLACEMENT_DISTANCE = 3;
    const unsigned int MAX_PLACEMENT_DISTANCE = 32;

    Cell centroid; // centroid of this house's stuff
    DynamicVectorClass<TechnoClass*> just_deployed;
    int unit_count = Session.Options.UnitCount;

    if (Session.Options.Bases) {
        unit_count--;
    }

    DEBUG_INFO("NumPlayers = {}\n", Session.NumPlayers);
    DEBUG_INFO("AIPlayers = {}\n", Session.Options.AIPlayers);
    DEBUG_INFO("Creating {} starting units per house - Random seed is {:08x}\n", unit_count, *reinterpret_cast<const unsigned int *>(&Scen->RandomNumber));
    DEBUG_INFO("UniqueID is {:08x}\n", Scen->UniqueID);

    /**
     *  Generate lists of all the available starting units (regardless of owner).
     */
    int total_cost = 0;
    int total_objs = 0;

    for (int i = 0; i < UnitTypes.Count(); ++i) {
        UnitTypeClass* utype = UnitTypes[i];
        if (utype && utype->IsAllowedToStartInMultiplayer) {
            if (!RuleExtension->BaseUnit.Is_Present(utype)) {
                total_cost += utype->Raw_Cost();
                total_objs++;
            }
        }
    }

    for (int i = 0; i < InfantryTypes.Count(); ++i) {
        InfantryTypeClass* itype = InfantryTypes[i];
        if (itype && itype->IsAllowedToStartInMultiplayer) {
            total_cost += itype->Raw_Cost();
            total_objs++;
        }
    }

    if (total_objs == 0) {
        DEBUG_WARNING("No starting units available!");
    }

    int average_cost = total_objs ? (total_cost / total_objs) : 0;
    int allowed_unit_cost = unit_count * average_cost;

    /**
     *  Loop through all houses.  Computer-controlled houses, with Session.Options.Bases
     *  ON, are treated as though bases are OFF (since we have no base-building AI logic.)
     */
    for (HousesType house = HOUSE_FIRST; house < Houses.Count(); ++house) {

        /**
         *  Get a pointer to this house; if there is none, go to the next house.
         */
        HouseClass* hptr = Houses[house];
        if (hptr == nullptr) {
            DEV_DEBUG_INFO("Invalid house {}!\n", (int)house);
            continue;
        }

        HouseClassExtension* houseext = Extension::Fetch(hptr);
        if (houseext->IsObserver) {
            DEV_DEBUG_INFO("House {} is an Observer, skipping.\n", (int)house);
            continue;
        }

        /**
         *  Skip passive houses.
         */
        if (hptr->Class->IsMultiplayPassive) {
            DEV_DEBUG_INFO("House {} ({} - \"{}\") is passive, skipping.\n", (int)house, hptr->Class->Name(), hptr->IniName);
            continue;
        }

        /**
         *  If the spawn waypoint for this house is nonexistent, look for a proper place for it.
         */
        if (Scen->Waypoint_Cell(houseext->SpawnWaypoint) == CELL_NONE) {
            if (!Assign_Random_Starting_Position(hptr)) {
                DEBUG_WARNING("Failed to find a fitting random starting location for house {}.\n", (int)house);
                continue;
            }
        }

        /**
         *  Fetch the center cell for this house that we assigned earlier in Assign_Starting_Positions().
         */
        centroid = Scen->Waypoint_Cell(houseext->SpawnWaypoint);

        DEBUG_INFO("Generating units for house {} (Name: {} - \"{}\", Color: {})...\n",
            (int)house, hptr->Class->Name(), hptr->IniName, ColorSchemes[hptr->Scheme]->Name);

        DynamicVectorClass<InfantryTypeClass*> infantry;
        DynamicVectorClass<UnitTypeClass*> units;
        unsigned long mask = 1 << hptr->Class->HeapID;

        /**
         *  Generate list of starting units for this house.
         */
        DEBUG_INFO("  Creating list of available UnitTypes...\n");
        for (int i = 0; i < UnitTypes.Count(); ++i) {
            UnitTypeClass* unittype = UnitTypes[i];

            /**
             *  Is this unit allowed to be placed in multiplayer?
             */
            if (!unittype->IsAllowedToStartInMultiplayer) {
                continue;
            }

            /**
             *  Check tech level and ownership.
             */
            if (unittype->Level <= hptr->Control.TechLevel && (unittype->Ownable & mask) != 0 && Extension::Fetch(hptr)->Required_Forbidden_Houses_Check(unittype)) {
                if (!RuleExtension->BaseUnit.Is_Present(unittype)) {
                    DEBUG_INFO("    Added {}\n", unittype->Name());
                    units.Add(unittype);
                }
            }
        }

        /**
         *  Generate list of starting infantry for this house.
         */
        DEBUG_INFO("  Creating list of available InfantryTypes...\n");
        for (int i = 0; i < InfantryTypes.Count(); ++i) {
            InfantryTypeClass* infantrytype = InfantryTypes[i];

            /**
             *  Is this unit allowed to be placed in multiplayer?
             */
            if (!infantrytype->IsAllowedToStartInMultiplayer) {
                continue;
            }

            /**
             *  Check tech level and ownership.
             */
            if (infantrytype->Level <= hptr->Control.TechLevel && (infantrytype->Ownable & mask) != 0 && Extension::Fetch(hptr)->Required_Forbidden_Houses_Check(infantrytype)) {
                infantry.Add(infantrytype);
                DEBUG_INFO("    Added {}\n", infantrytype->Name());
            }
        }

        /**
         *  Assign the center of this house to the waypoint location.
         */
        hptr->Center = centroid.As_Coord();
        DEBUG_INFO("  Setting house center to {},{}\n", centroid.X, centroid.Y);

        /**
         *  If Bases are ON, place a base unit (MCV).
         */
        if (Session.Options.Bases) {

            UnitTypeClass* baseunittype = hptr->Get_First_Ownable(RuleExtension->BaseUnit);
            if (baseunittype == nullptr) {
                Vinifera_Log_And_Show_WWMessageBox("Failed to find a valid BaseUnit for house %s (%d) of HouseType %s (%d)!", hptr->IniName.c_str(), hptr->HeapID, hptr->Class->IniName.c_str(), hptr->Class->HeapID);
                continue;
            }

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

                if (baseunittype->DeploysInto == nullptr) {
                    Vinifera_Log_And_Show_WWMessageBox("BaseUnit %s (%d) of house %s (%d) of HouseType %s (%d) has DeploysInto=none!", 
                        baseunittype->IniName.c_str(), baseunittype->HeapID, hptr->IniName.c_str(), hptr->HeapID, hptr->Class->IniName.c_str(), hptr->Class->HeapID);
                    continue;
                }

                BuildingClass* building = new BuildingClass(baseunittype->DeploysInto, hptr);

                if (building->Unlimbo(centroid.As_Coord(), DIR_N) || _Scan_Place_Object(building, centroid)) {
                    if (building != nullptr) {
                        DEBUG_INFO("  Construction yard {} placed at {},{}.\n", building->Class_Of()->Name(), building->Get_Cell().X, building->Get_Cell().Y);

                        /**
                         *  Always reveal the construction yard to the player
                         *  that owns it.
                         */
                        building->Revealed(building->House);
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

                                    Cell cell = building->Position.As_Cell();

                                    building->House->Begin_Construction();

                                    building->House->Base.Nodes[0].CellID = cell;
                                    building->House->Base.field_50 = cell;

                                    building->House->IsStarted = true;
                                    building->House->IsAITriggersOn = true;
                                    building->House->IsBaseBuilding = true;
                                }
                            }
                        }
                    }
                    hptr->FlagHome = Cell(0, 0);
                    hptr->FlagLocation = nullptr;
                }

            } else {

                /**
                 *  For a human-controlled house:
                 *    - Create an MCV
                 *    - Attach a flag to it for capture-the-flag mode.
                 */
                UnitClass* unit = new UnitClass(baseunittype, hptr);
                if (unit->Unlimbo(centroid.As_Coord(), DIR_N) || _Scan_Place_Object(unit, centroid)) {
                    if (unit != nullptr) {
                        DEBUG_INFO("  Base unit {} placed at {},{}.\n", unit->Class_Of()->Name(), unit->Get_Cell().X, unit->Get_Cell().Y);
                        hptr->FlagHome = Cell(0, 0);
                        hptr->FlagLocation = nullptr;
                        if (Special.IsCaptureTheFlag) {
                            hptr->Flag_Attach(unit, true);
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
                                if (hptr->Is_Human_Player()) {
                                    unit->Set_Mission(MISSION_UNLOAD);
                                }
                            }
                        }
                    }

                } else if (unit) {
                    delete unit;
                    unit = nullptr;
                }
            }
        }

        /**
         *  #BUGFIX:
         *  Make sure there are units available to place before entering the loop.
         */
        if (total_objs) {

            TechnoTypeClass* technotype = nullptr;
            int deployed_so_far = 0;

            just_deployed.Clear();

            while (deployed_so_far < allowed_unit_cost) {

                technotype = nullptr;

                if (deployed_so_far < (allowed_unit_cost * 2) / 3 && units.Count() > 0) {
                    technotype = units[Random_Pick(0, units.Count() - 1)];
                } else if (infantry.Count() > 0) {
                    technotype = infantry[Random_Pick(0, infantry.Count() - 1)];
                }

                if (technotype == nullptr) {
                    DEBUG_WARNING("No technotype available to place for house {}, skipping further placement\n", hptr->Class->IniName);
                    break;
                }

                /**
                 *  Create units (Note: Unlimbo calls Enter_Idle_Mode(), which
                 *  assigns the unit to HUNT; we must use Set_Mission() to override
                 *  this state.)
                 */
                ObjectClass* obj = technotype->Create_One_Of(hptr);
                TechnoClass* tobj = As_Techno(obj);

                if (!_Scan_Place_Object(obj, centroid, MIN_PLACEMENT_DISTANCE, MAX_PLACEMENT_DISTANCE)) {
                    delete obj;
                } else {
                    DEBUG_INFO("House {} deployed object {}\n", hptr->Class->IniName, technotype->IniName);

                    deployed_so_far += technotype->Raw_Cost();
                    just_deployed.Add(tobj);

                    if (Scen->Special.IsInitialVeteran) {
                        tobj->Crew.Set_Elite(true);
                    }

                    if (!hptr->Is_Human_Player()) {
                        tobj->Set_Mission(MISSION_GUARD_AREA);
                    } else {
                        tobj->Set_Mission(MISSION_GUARD);
                    }
                }
            }

#if 0 // don't scatter anymore, we're deploying in a RA2-like formation
            if (hptr->Is_Human_Player()) {
                for (int t = 0; t < just_deployed.Count(); t++) {
                    just_deployed[t]->Scatter(COORD_NONE);
                }
            }
#endif
            just_deployed.Clear();
        }
    }

    DEBUG_INFO("Finished unit generation. Random number is {}\n", Scen->RandomNumber());
}
