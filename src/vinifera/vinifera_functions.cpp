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
#include "theatertype.h"
#include "debughandler.h"
#include <string>


/**
 *  Load any Vinifera settings that provide overrides.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Load_INI()
{
    CCFileClass file("VINIFERA.INI");
    INIClass ini;

    if (!file.Is_Available()) {
        return false;
    }

    ini.Load(file);

    ini.Get_String("General", "ProjectName", Vinifera_ProjectName, sizeof(Vinifera_ProjectName));
    ini.Get_String("General", "ProjectVersion", "Not Set", Vinifera_ProjectVersion, sizeof(Vinifera_ProjectVersion));
    ini.Get_String("General", "IconFile", Vinifera_IconName, sizeof(Vinifera_IconName));
    ini.Get_String("General", "CursorFile", Vinifera_CursorName, sizeof(Vinifera_CursorName));

    Vinifera_ProjectName[sizeof(Vinifera_ProjectName)-1] = '\0';
    Vinifera_ProjectVersion[sizeof(Vinifera_ProjectVersion)-1] = '\0';
    Vinifera_IconName[sizeof(Vinifera_IconName)-1] = '\0';
    Vinifera_CursorName[sizeof(Vinifera_CursorName)-1] = '\0';

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
            DEBUG_INFO("  - Skipping to Firestorm menu.\n");
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
    /**
     *  #issue-514:
     * 
     *  Adds various search paths for loading files locally for the TS-Client builds only.
     * 
     *  @author: CCHyper
     */
#if defined(TS_CLIENT)
    DWORD rc;
    DynamicVectorClass<Wstring> search_paths;

    /**
     *  If -CD has been defined, set the root directory as highest priority.
     */
    if (CD::IsFilesLocal) {
        search_paths.Add(".");
    }

    /**
     *  Add various local search drives to loading of files locally.
     */
    search_paths.Add("INI");
    search_paths.Add("MIX");
    search_paths.Add("MOVIES");
    search_paths.Add("MUSIC");
    search_paths.Add("SOUNDS");
    search_paths.Add("MAPS");
    search_paths.Add("MAPS\\MULTIPLAYER");
    search_paths.Add("MAPS\\MISSION");

    /**
     *  Current path (perhaps set set with -CD) should go next.
     */
    if (CCFileClass::RawPath[0] != '\0' && std::strlen(CCFileClass::RawPath) > 1) {
        search_paths.Add(CCFileClass::RawPath);
    }

    char *new_path = new char [_MAX_PATH * search_paths.Count()+1];
    new_path[0] = '\0';

    /**
     *  Build the search path string.
     */
    for (int i = 0; i < search_paths.Count(); ++i) {
        if (i != 0) std::strcat(new_path, ";");
        std::strcat(new_path, search_paths[i].Peek_Buffer());
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

    delete [] new_path;

    DEBUG_INFO("SearchPath: %s\n", CCFileClass::RawPath);
#endif

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
    EBoltClass::Clear_All();
    TheaterTypes.Clear();

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
    RulesClassExtension::Init_UI_Controls();

    if (!RulesClassExtension::Read_UI_INI()) {
        DEBUG_WARNING("Failed to read UI.INI!\n");
        //return EXIT_FAILURE;
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
            //return -1;
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

    DEBUG_INFO("  TestLocomotionClass\n");
    REGISTER_CLASS(TestLocomotionClass);

    DEBUG_INFO("  ...OK!\n");

    return true;
}
