/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_FUNCTIONS.CPP
 *
 *  @authors       CCHyper
 *
 *  @brief         General functions.
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
#include "vinifera_functions.h"
#include "vinifera_globals.h"
#include "vinifera_newdel.h"
#include "tibsun_globals.h"
#include "cncnet4.h"
#include "cncnet4_globals.h"
#include "cncnet5_globals.h"
#include "rulesext.h"
#include "ccfile.h"
#include "ccini.h"
#include "filestraw.h"
#include "readline.h"
#include "cd.h"
#include "ebolt.h"
#include "optionsext.h"
#include "rulesext.h"
#include "sessionext.h"
#include "scenarioext.h"
#include "tacticalext.h"
#include "tclassfactory.h"
#include "testlocomotion.h"
#include "kamikazetracker.h"
#include "spawnmanager.h"
#include "extension.h"
#include "theatertype.h"
#include "armortype.h"
#include "uicontrol.h"
#include "mousetype.h"
#include "actiontype.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <string>

#include "rocketlocomotion.h"
#include "setup_hooks.h"


static DynamicVectorClass<Wstring> ViniferaSearchPaths;


/**
 *  Load any Vinifera settings that provide overrides.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Load_INI()
{
    CCFileClass file;
    INIClass ini;

    if (CCFileClass("VINIFERA.INI").Is_Available()) {
        file.Set_Name("VINIFERA.INI");

    } else if (CCFileClass("INI\\VINIFERA.INI").Is_Available()) {
        file.Set_Name("INI\\VINIFERA.INI");
    }

    if (!file.Is_Available()) {
        return false;
    }

    ini.Load(file);

    ini.Get_String("General", "ProjectName", Vinifera_ProjectName, sizeof(Vinifera_ProjectName));
    ini.Get_String("General", "IconFile", Vinifera_IconName, sizeof(Vinifera_IconName));
    ini.Get_String("General", "CursorFile", Vinifera_CursorName, sizeof(Vinifera_CursorName));

#if defined(TS_CLIENT)
    /**
     *  TS Client uses a seperate "version" file, so its best we fetch the current
     *  version from there rather than have the user update the INI file each time
     *  they update the project.
     */
    RawFileClass ver_file("version");
    if (ver_file.Is_Available()) {
        INIClass ver_ini;
        ver_ini.Load(ver_file);
        ver_ini.Get_String("DTA", "Version", Vinifera_ProjectVersion, sizeof(Vinifera_ProjectVersion));
    } else {
        ini.Get_String("General", "ProjectVersion", Vinifera_ProjectVersion, sizeof(Vinifera_ProjectVersion));
    }
#else
    ini.Get_String("General", "ProjectVersion", Vinifera_ProjectVersion, sizeof(Vinifera_ProjectVersion));
#endif

    Vinifera_ProjectName[sizeof(Vinifera_ProjectName)-1] = '\0';
    Vinifera_ProjectVersion[sizeof(Vinifera_ProjectVersion)-1] = '\0';
    Vinifera_IconName[sizeof(Vinifera_IconName)-1] = '\0';
    Vinifera_CursorName[sizeof(Vinifera_CursorName)-1] = '\0';

    char buffer[1024];
    if (ini.Get_String("General", "SearchPaths", buffer, sizeof(buffer)) > 0) {
        char *path = std::strtok(buffer, ",");
        while (path) {
            if (!ViniferaSearchPaths.Is_Present(path)) {
                ViniferaSearchPaths.Add(path);
            }
            path = std::strtok(nullptr, ",");
        }
#if defined(TS_CLIENT)
    } else {
        DEBUG_ERROR("Failed to find SearchPaths in VINIFERA.INI!\n");
        MessageBox(MainWindow, "Failed to find SearchPaths in VINIFERA.INI, please reinstall Vinifera.", "Vinifera", MB_ICONEXCLAMATION|MB_OK);
        return false;
#endif
    }

    Vinifera_NewSidebar = ini.Get_Bool("Features", "NewSidebar", false);
    ini.Get_String("General", "SavedGamesDirectory", buffer, std::size(buffer));
    if (std::strlen(buffer) > 0) {
        std::strncpy(Vinifera_SavedGamesDirectory, buffer, std::size(Vinifera_SavedGamesDirectory) - 1);
    }

    return true;
}


/**
 *  Loads the exception database.
 * 
 *  @author: CCHyper
 */
static bool Vinifera_Load_Exception_Database(const char *filename)
{
    RawFileClass file(filename);
    if (!file.Is_Available()) {
        return false;
    }

    FileStraw fstraw(file);

    bool eof = false;
    bool found_first_line = false;
    char line_buffer[11 + 2 + 2 + 1024]; // address, bool, bool, desc
    ExceptionInfoDatabaseStruct einfo;

    while (true) {

        char *tok = nullptr;
           
        /**
         *  Read the line into the buffer.
         */
        int count = Read_Line(fstraw, line_buffer, sizeof(line_buffer), eof);

        /**
         *  Handle end of file and invalid line cases.
         */
        if (eof) {
            break;
        }
        if (!count) {
            continue;
        }
        if (count >= sizeof(line_buffer)) {
            break;
        }

        int index = 0; // cursor position.

        /**
         *  Step over any indenting.
         */
        while (std::isspace(line_buffer[index])) {
            ++index;
        }

        /**
         *  Handle commented out lines.
         */
        if (line_buffer[index] == ';') {
            continue;
        }

        /**
         *  Process the database line.
         */

        tok = std::strtok(&line_buffer[index], ",");
        ASSERT(tok != nullptr);
        if (tok[0] != '0' || tok[1] != 'x') {
            DEBUG_WARNING("Invalid address format in exception database!\n");
            return false;
        }
        einfo.Address = std::strtoul(tok+2, nullptr, 16);

        tok = std::strtok(nullptr, ",");
        ASSERT(tok != nullptr);
        einfo.CanContinue = std::strtoul(tok, nullptr, 10) ? true : false;

        tok = std::strtok(nullptr, ",");
        ASSERT(tok != nullptr);
        einfo.Ignore = std::strtoul(tok, nullptr, 10) ? true : false;
        
        tok = std::strtok(nullptr, ",");
        ASSERT(tok != nullptr);
        std::strncpy(einfo.Description, tok, std::strlen(tok));

        ExceptionInfoDatabase.Add(einfo);
    }

    if (!ExceptionInfoDatabase.Count()) {
        DEBUG_WARNING("Invalid format in exception database!\n");
        return false;
    }

#ifndef NDEBUG
    DEV_DEBUG_INFO("Exception database dump...\n");
    for (int i = 0; i < ExceptionInfoDatabase.Count(); ++i) {
        ExceptionInfoDatabaseStruct &e = ExceptionInfoDatabase[i];
        DEV_DEBUG_INFO("  0x%08X %s %s \"%.32s...\"\n",
                       e.Address, e.CanContinue ? "true " : "false",
                       e.Ignore ? "true " : "false",
                       e.Description);
    }
#endif

    return true;
}


/**
 *  Parses the command line parameters.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Parse_Command_Line(int argc, char *argv[])
{
    if (argc > 1) {
        DEBUG_INFO("Parsing command line arguments...\n");
    }

    bool menu_skip = false;

    /**
     *  Iterate over all command line params.
     */
    for (int index = 1; index < argc; index++) {

        char arg_string[512];

        char *src = argv[index];
        char *dest = arg_string; 
        for (int i= 0; i < std::strlen(argv[index]); ++i) {
            if (*src == '\"') {
                src++;
            } else {
                *dest++ = *src++;
            }
        }
        *dest++ = '\0';

        char *string = arg_string; // Pointer to current argument.
        strupr(string);

        /**
         *  Add all new command line params here.
         */

        /**
         *  Mod developer mode.
         */
        if (stricmp(string, "-DEVELOPER") == 0) {
            DEBUG_INFO("  - Developer mode enabled.\n");
            Vinifera_DeveloperMode = true;
            continue;
        }

        /**
         *  Skip the startup videos.
         */
        if (stricmp(string, "-NO_STARTUP_VIDEO") == 0) {
            DEBUG_INFO("  - Skipping startup videos.\n");
            Vinifera_SkipStartupMovies = true;
            continue;
        }

        /**
         *  Skip directly to Tiberian Sun menu.
         */
        if (stricmp(string, "-SKIP_TO_TS_MENU") == 0) {
            DEBUG_INFO("  - Skipping to Tiberian Sun menu.\n");
            Vinifera_SkipToTSMenu = true;
            menu_skip = true;
            continue;
        }

        /**
         *  Skip directly to Firestorm menu.
         */
        if (stricmp(string, "-SKIP_TO_FS_MENU") == 0) {
            DEBUG_INFO("  - Skipping to Firestorm menu.\n");
            Vinifera_SkipToFSMenu = true;
            menu_skip = true;
            continue;
        }

        /**
         *  Skip directly to a specific game mode dialog.
         */
        if (stricmp(string, "-SKIP_TO_LAN") == 0) {
            DEBUG_INFO("  - Skipping to LAN dialog.\n");
            Vinifera_SkipToLAN = true;
            menu_skip = true;
            continue;
        }

        if (stricmp(string, "-SKIP_TO_CAMPAIGN") == 0) {
            DEBUG_INFO("  - Skipping to campaign dialog.\n");
            Vinifera_SkipToCampaign = true;
            menu_skip = true;
            continue;
        }

        if (stricmp(string, "-SKIP_TO_SKIRMISH") == 0) {
            DEBUG_INFO("  - Skipping to skirmish dialog.\n");
            Vinifera_SkipToSkirmish = true;
            menu_skip = true;
            continue;
        }

        if (stricmp(string, "-SKIP_TO_INTERNET") == 0) {
            DEBUG_INFO("  - Skipping to internet dialog.\n");
            Vinifera_SkipToInternet = true;
            menu_skip = true;
            continue;
        }

        /**
         *  Exit the game after the dialog we skipped to has been canceled?
         */
        if (stricmp(string, "-EXIT_AFTER_SKIP") == 0) {
            DEBUG_INFO("  - Forcing game exit after return from menu skip.\n");
            Vinifera_ExitAfterSkip = true;
            menu_skip = true;
            continue;
        }

        /**
         *  #issue-513
         * 
         *  Re-implements the file search path override logic of "-CD" from Red Alert.
         */
        if (std::strstr(string, "-CD")) {
            DEBUG_INFO("  - \"-CD\" argument detected.\n");

            if (std::isspace(string[3]) || !std::isgraph(string[3])) {
                DEBUG_ERROR("Invalid search path defined!");
                MessageBox(MainWindow, "Invalid search path defined with -CD command line argument!", "Vinifera", MB_ICONEXCLAMATION|MB_OK);
                return false;
            }

            CCFileClass::Set_Search_Drives(&string[3]);
            if (CCFileClass::Is_There_Search_Drives()) {
                DEBUG_INFO("  - Search path set to \"%s\".\n", &string[3]);

                /**
                 *  Flag the cd search system to search for files locally.
                 */
                CD::IsFilesLocal = true;
            }
            continue;
        }

        /**
         *  Should assertions only be printed to the debug log?
         */
        if (stricmp(string, "-SILENT_ASSERTS") == 0) {
            DEBUG_INFO("  - Assertions are silent.\n");
            SilentAsserts = true;
            continue;
        }

        /**
         *  Ignore all assertions?
         */
        if (stricmp(string, "-IGNORE_ASSERTS") == 0) {
            DEBUG_INFO("  - Ignore all assertions.\n");
            IgnoreAllAsserts = true;
            continue;
        }

        /**
         *  Are file io errors fatal?
         */
        if (stricmp(string, "-FILE_ERROR_FATAL") == 0) {
            DEBUG_INFO("  - File read/write errors are fatal.\n");
            Vinifera_FatalFileErrors = true;
            continue;
        }

        /**
         *  Trigger an assertion on file io errors?
         */
        if (stricmp(string, "-FILE_ERROR_ASSERT") == 0) {
            DEBUG_INFO("  - Assertions on file read/write error.\n");
            Vinifera_AssertFileErrors = true;
            continue;
        }

        /**
         *  Specify the random number seed (for debugging).
         */
        if (std::strstr(string, "-SEED")) {
            CustomSeed = (unsigned short)(std::atoi(string + std::strlen("SEED")));
            continue;
        }

#ifndef RELEASE
        /**
         *  Hide the version string from the ingame tactical view?
         */
        if (stricmp(string, "-NO_VERSION_STRING") == 0) {
            DEBUG_INFO("  - No version string on tactical view.\n");
            Vinifera_NoTacticalVersionString = true;
            continue;
        }
#endif

    }

    if (argc > 1) {
        DEBUG_INFO("Finished parsing command line arguments.\n");
    }

    /**
     *  Firestorm has priority over Tiberian Sun.
     */
    if (Vinifera_SkipToTSMenu && Vinifera_SkipToFSMenu) {
        Vinifera_SkipToTSMenu = false;
    }

    /**
     *  If any of the menu skip commands have been set then
     *  we also need to skip the startup movies.
     */
    if (menu_skip) {
        Vinifera_SkipStartupMovies = true;
    }

    return true;
}


/**
 *  This function will get called on application startup, allowing you to
 *  perform any action that would effect the game initialisation process.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Startup()
{
    DWORD rc;

    ViniferaSearchPaths.Clear();

    /**
     *  If -CD has been defined, set the root directory as highest priority.
     */
    if (CD::IsFilesLocal) {
        ViniferaSearchPaths.Add(".");
    }
    
#ifndef NDEBUG
    /**
     *  Debug paths for CD contents (folders must contain .DSK files of the same name).
     */
    ViniferaSearchPaths.Add("TS1");
    ViniferaSearchPaths.Add("TS2");
    ViniferaSearchPaths.Add("TS3");
#endif

    /**
     *  #issue-514:
     * 
     *  Adds various search paths for loading files locally for the TS-Client builds only.
     * 
     *  #NOTE: REMOVED: Additional paths must now be set via SearchPaths in VINIFERA.INI!
     * 
     *  @author: CCHyper
     */
#if 0 // #if defined(TS_CLIENT)

    // Only required for the TS Client builds as most projects will
    // put VINIFERA.INI in this directory.
    ViniferaSearchPaths.Add("INI");

    // Required for startup mix files to be found.
    ViniferaSearchPaths.Add("MIX");
#endif

#if !defined(TS_CLIENT)
    // Required for startup movies to be found.
    ViniferaSearchPaths.Add("MOVIES");
#endif

    // REMOVED: Paths are now set via SearchPaths in VINIFERA.INI
//#if defined(TS_CLIENT)
//    ViniferaSearchPaths.Add("MUSIC");
//    ViniferaSearchPaths.Add("SOUNDS");
//    ViniferaSearchPaths.Add("MAPS");
//    ViniferaSearchPaths.Add("MAPS\\MULTIPLAYER");
//    ViniferaSearchPaths.Add("MAPS\\MISSION");
//#endif

    /**
     *  Load Vinifera settings and overrides.
     */
    if (Vinifera_Load_INI()) {
        DEBUG_INFO("\n");
        DEBUG_INFO("Project information:\n");
        DEBUG_INFO("  Title: %s\n", Vinifera_ProjectName);
        DEBUG_INFO("  Version: %s\n", Vinifera_ProjectVersion);
        DEBUG_INFO("\n");
    } else {
        DEBUG_WARNING("Failed to load VINIFERA.INI!\n");
#if defined(TS_CLIENT)
        MessageBoxA(nullptr, "Failed to load VINIFERA.INI!", "Vinifera", MB_ICONERROR|MB_OK);
        return false;
#endif
    }

    DEBUG_INFO("Setting up conditional hooks.\n");
    Setup_Conditional_Hooks();

    /**
     *  Current path (perhaps set set with -CD) should go next.
     */
    if (CCFileClass::RawPath[0] != '\0' && std::strlen(CCFileClass::RawPath) > 1) {
        ViniferaSearchPaths.Add(CCFileClass::RawPath);
    }

    if (ViniferaSearchPaths.Count() > 0) {
        char *new_path = new char[_MAX_PATH * ViniferaSearchPaths.Count()+1];
        new_path[0] = '\0';

        /**
         *  Build the search path string.
         */
        for (int i = 0; i < ViniferaSearchPaths.Count(); ++i) {
            if (i != 0) std::strcat(new_path, ";");
            std::strcat(new_path, ViniferaSearchPaths[i].Peek_Buffer());
        }

        /**
         *  Clear the current path ready to be set.
         */
        CCFileClass::Clear_Search_Drives();
        CCFileClass::Reset_Raw_Path();

        /**
         *  Set the new search drive path.
         */
        CCFileClass::Set_Search_Drives(new_path);

        delete[] new_path;

        DEBUG_INFO("SearchPath: %s\n", CCFileClass::RawPath);
    }

    /**
     *  We are finished with the vector, clear it.
     */
    ViniferaSearchPaths.Clear();

    /**
     *  Check for the existence of the exception database.
     */
    RawFileClass dbfile(VINIFERA_TARGET_EDB);
    if (!dbfile.Is_Available()) {
        DEBUG_ERROR("Failed to find the exception database!\n");
        MessageBox(MainWindow, "Failed to find the exception database, please reinstall Vinifera.", "Vinifera", MB_OK);
        return false;
    }
    if (!dbfile.Size()) {
        DEBUG_ERROR("Invalid or corrupt exception database!\n");
        MessageBox(MainWindow, "Invalid or corrupt exception database, please reinstall Vinifera.", "Vinifera", MB_OK);
        return false;
    }
    if (!Vinifera_Load_Exception_Database(dbfile.File_Name())) {
        DEBUG_ERROR("Failed to load the exception database!\n");
        MessageBox(MainWindow, "Failed to load the exception database, please reinstall Vinifera.", "Vinifera", MB_OK);
        return false;
    }

#if !defined(TS_CLIENT)
    /**
     *  Initialise the CnCNet4 system.
     */
    if (!CnCNet4::Init()) {
        CnCNet4::IsEnabled = false;
        DEBUG_WARNING("Failed to initialise CnCNet4, continuing without CnCNet4 support!\n");
    }

    /**
     *  Disable CnCNet4 if CnCNet5 is active, they can not co-exist.
     */
    if (CnCNet4::IsEnabled && CnCNet5::IsActive) {
        CnCNet4::Shutdown();
        CnCNet4::IsEnabled = false;
    }
#else
    /**
     *  Client builds can only use CnCNet5.
     */
    CnCNet4::IsEnabled = false;
    //CnCNet5::IsActive = true; // Enable when new Client system is implemented.
#endif

    KamikazeTracker = new KamikazeTrackerClass;

    return true;
}


/**
 *  This function will get called on application shutdown, allowing you to
 *  perform any memory cleanup or shutdown of new systems.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Shutdown()
{
    /**
     *  Cleanup mixfiles.
     */
    delete GenericMix;
    GenericMix = nullptr;

    delete IsoGenericMix;
    IsoGenericMix = nullptr;

    ViniferaMapsMixes.Clear();
    ViniferaMoviesMixes.Clear();

    /**
     *  Cleanup global heaps/vectors.
     */
    TheaterTypes.Clear();

    /**
     *  Cleanup global extension instances.
     */
    delete OptionsExtension;
    OptionsExtension = nullptr;

    delete UIControls;
    UIControls = nullptr;

    /**
     *  Cleanup additional extension instances.
     */
    ThemeControlExtensions.Clear();

    delete KamikazeTracker;
    KamikazeTracker = nullptr;

    DEV_DEBUG_INFO("Shutdown - New Count: %d, Delete Count: %d\n", Vinifera_New_Count, Vinifera_Delete_Count);

    return true;
}


/**
 *  This function will get called "before" the games "Init_Game" function,
 *  allowing you to perform any action that would effect the game initialisation process.
 * 
 *  @author: CCHyper
 */
int Vinifera_Pre_Init_Game(int argc, char *argv[])
{
    /**
     *  Read the UI controls and overrides.
     */
    UIControls = new UIControlsClass;

    CCFileClass ui_file("UI.INI");
    CCINIClass ui_ini;

    if (ui_file.Is_Available()) {

        ui_ini.Load(ui_file, false);

        if (!UIControls->Read_INI(ui_ini)) {
            DEV_DEBUG_ERROR("Failed to read UI.INI!\n");
            //return EXIT_FAILURE;
        }

    } else {
        DEV_DEBUG_WARNING("UI.INI not found!\n");
    }

#if defined(TS_CLIENT)
    /**
     *  The TS Client allows player to jump right into a game, so no need to
     *  show the startup movies for these builds.
     */
    Vinifera_SkipStartupMovies = true;
#endif

    /**
     *  Read the mouse controls and overrides.
     * 
     *  This must be loaded before Init_Game as MouseClass::Override_Mouse_Shape
     *  is called as part of the games initialisation.
     */
    MouseTypeClass::One_Time();

#if 0 //#ifndef NDEBUG
    /**
     *  Write the default mouse control values to ini.
     */
    {
        CCFileClass mouse_write_file("MOUSE.DBG");
        CCINIClass mouse_write_ini;
        mouse_write_file.Delete();
        MouseTypeClass::Write_Default_INI(mouse_write_ini);
        mouse_write_ini.Save(mouse_write_file, false);
        mouse_write_file.Close();
    }
#endif

    CCFileClass mouse_file("MOUSE.INI");

    if (mouse_file.Is_Available()) {

        CCINIClass mouse_ini;
        mouse_ini.Load(mouse_file, false);

        MouseTypeClass::Read_INI(mouse_ini);
    }

    /**
     *  Read the actions controls and overrides.
     * 
     *  This must be loaded after MouseTypeClass::One_Time as actions reference MouseTypes!
     */
    ActionTypeClass::One_Time();

#if 0 //#ifndef NDEBUG
    /**
     *  Write the default mouse control values to ini.
     */
    {
        CCFileClass action_write_file("ACTION.DBG");
        CCINIClass action_write_ini;
        action_write_file.Delete();
        ActionTypeClass::Write_Default_INI(action_write_ini);
        action_write_ini.Save(action_write_file, false);
        action_write_file.Close();
    }
#endif

    CCFileClass action_file("ACTION.INI");

    if (action_file.Is_Available()) {

        CCINIClass action_ini;
        action_ini.Load(action_file, false);

        ActionTypeClass::Read_INI(action_ini);
    }

    return EXIT_SUCCESS;
}


/**
 *  This function will get called "after" the games "Init_Game" function,
 *  allowing you to perform any action that would effect the game initialisation process.
 * 
 *  @author: CCHyper
 */
int Vinifera_Post_Init_Game(int argc, char *argv[])
{
    TheaterTypeClass::One_Time();

    CCFileClass theater_file("THEATERS.INI");
    CCINIClass theater_ini;

    if (theater_file.Is_Available()) {

        theater_ini.Load(theater_file, false);

        if (!TheaterTypeClass::Read_Theaters_INI(theater_ini)) {
            DEV_DEBUG_ERROR("Failed to read THEATERS.INI!\n");
            //return EXIT_FAILURE;
        }

    } else {
        DEV_DEBUG_WARNING("THEATERS.INI not found!\n");
    }

    return EXIT_SUCCESS;
}


/**
 *  This function registers any com objects required by the DLL.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Register_Com_Objects()
{
    DEBUG_INFO("Registering new com objects...\n");

    /**
     *  New locomotors.
     */
    REGISTER_CLASS(TestLocomotionClass);
    REGISTER_CLASS(RocketLocomotionClass);

    /**
     *  New types.
     */
    REGISTER_CLASS(ArmorTypeClass);
    REGISTER_CLASS(RocketTypeClass);

    /**
     *  Other new entities.
     */
    REGISTER_CLASS(SpawnManagerClass);
    
    Extension::Register_Class_Factories();

    DEBUG_INFO("  ...OK!\n");

    return true;
}
