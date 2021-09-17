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
#include "cd.h"
#include "debughandler.h"
#include <string>


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
    RulesClassExtension::Init_UI_Controls();

    /**
     *  Read the UI controls and overrides.
     */
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
    return EXIT_SUCCESS;
}
