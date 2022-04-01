/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INITEXT_FUNCTIONS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains supporting functions for game init process.
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
#include "initext_functions.h"
#include "vinifera_const.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "language.h"
#include "addon.h"
#include "cd.h"
#include "session.h"
#include "iomap.h"
#include "theme.h"
#include "dsaudio.h"
#include "vinifera_gitinfo.h"
#include "tspp_gitinfo.h"
#include "resource.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <Windows.h>
#include <commctrl.h>

#include "options.h"


extern HMODULE DLLInstance;


/**
 *  Tiberian Sun resource constants.
 */
#define TS_MAINICON		    93
#define TS_MAINCURSOR		104


/**
 *  Creates the main window for Tiberian Sun
 * 
 *  @author: CCHyper
 */
void Vinifera_Create_Main_Window(HINSTANCE hInstance, int nCmdShow, int width, int height)
{
    //DEV_DEBUG_INFO("Create_Main_Window(enter)\n");

    MainWindow = nullptr;

    HWND hWnd = nullptr;
    BOOL rc;
    WNDCLASSEX wc;
    tagRECT rect;
    HICON hIcon = nullptr;
    HICON hSmIcon = nullptr;
    HCURSOR hCursor = nullptr;

    //DEV_DEBUG_INFO("Create_Main_Window() - About to call InitCommonControls()\n");

    InitCommonControls();

    //DEV_DEBUG_INFO("Create_Main_Window() - Preparing window name (with version info).\n");

    DWORD dwPid = GetProcessId(GetCurrentProcess());
    if (!dwPid) {
        DEBUG_ERROR("Create_Main_Window() - Failed to get the process id!\n");
        return;
    }

    //DEV_DEBUG_INFO("Create_Main_Window() - Loading icon and cursor resources.\n");

    /**
     *  Load the Vinifera icon and cursor resources, falling back to the GAME.EXE
     *  resources if not available or failed to load.
     */
    if (Vinifera_IconName[0] != '\0') {
        DEBUG_INFO("Loading custom icon \"%s\"\n", Vinifera_IconName);
        hIcon = (HICON)LoadImage(
            nullptr,
            Vinifera_IconName,
            IMAGE_ICON,
            0,
            0,
            LR_LOADFROMFILE);
        DEBUG_INFO("Loading custom small icon \"%s\"\n", Vinifera_IconName);
        hSmIcon = (HICON)LoadImage(
            nullptr,
            Vinifera_IconName,
            IMAGE_ICON,
            GetSystemMetrics(SM_CXSMICON),
            GetSystemMetrics(SM_CXSMICON),
            LR_LOADFROMFILE);
    }
    if (!hIcon) {
        hIcon = LoadIcon((HINSTANCE)DLLInstance, MAKEINTRESOURCE(VINIFERA_MAINICON));
        if (!hIcon) {
            hIcon = LoadIcon((HINSTANCE)hInstance, MAKEINTRESOURCE(TS_MAINICON));
        }
    }
    if (Vinifera_CursorName[0] != '\0') {
        DEBUG_INFO("Loading custom cursor \"%s\"\n", Vinifera_CursorName);
        hCursor = LoadCursorFromFile(Vinifera_CursorName);
    }
    if (!hCursor) {
        hCursor = LoadCursor(nullptr, VINIFERA_MAINCURSOR); // IDC_ARROW is a system resource, does not require module.
        if (!hCursor) {
            hCursor = LoadCursor((HINSTANCE)hInstance, MAKEINTRESOURCE(TS_MAINCURSOR));
        }
    }

    //DEV_DEBUG_INFO("Create_Main_Window() - Setting up window class info.\n");

    /**
     *  Register the window class.
     */
    wc.cbSize         = sizeof(WNDCLASSEX);
    wc.style          = CS_HREDRAW|CS_VREDRAW;
    wc.lpfnWndProc    = Main_Window_Procedure;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = 0;
    wc.hInstance      = (HINSTANCE)hInstance;
    wc.hIcon          = hIcon;
    wc.hCursor        = hCursor;
    wc.hbrBackground  = nullptr;
    wc.lpszMenuName   = nullptr;
    wc.lpszClassName  = "Vinifera";
    wc.hIconSm        = (hSmIcon ? hSmIcon : hIcon);

    //DEV_DEBUG_INFO("Create_Main_Window() - About to call RegisterClass()\n");

    /**
     *  Register window class.
     */
    rc = RegisterClassEx(&wc);
    if (!rc) {
        DEBUG_INFO("Create_Main_Window() - Failed to register window class!\n");
        return;
    }

    /**
     *  Get the dimensions of the primary display.
     */
    int display_width = GetSystemMetrics(SM_CXSCREEN);
    int display_height = GetSystemMetrics(SM_CYSCREEN);

    //DEV_DEBUG_INFO("Create_Main_Window() - Desktop size %d x %d\n", display_width, display_height);

    /**
     *  Create our main window.
     */
    if (Debug_Windowed) {

        DEBUG_INFO("Create_Main_Window() - Creating desktop window (%d x %d).\n", width, height);

        hWnd = CreateWindowEx(
            WS_EX_LEFT|WS_EX_TOPMOST,
            "Vinifera",
            Vinifera_Get_Window_Title(dwPid),
            WS_SYSMENU|WS_MINIMIZEBOX|WS_CLIPCHILDREN|WS_CAPTION,
            0, 0, 0, 0,
            nullptr,
            nullptr,
            (HINSTANCE)hInstance,
            nullptr);

        SetRect(&rect, 0, 0, width, height);

        AdjustWindowRectEx(&rect,
            GetWindowLong(hWnd, GWL_STYLE),
            GetMenu(hWnd) != nullptr,
            GetWindowLong(hWnd, GWL_EXSTYLE));

        /**
         *  #BUGFIX:
         * 
         *  Fetch the desktop size, calculate the screen center position the window and move it.
         */
        RECT workarea;
        SystemParametersInfo(SPI_GETWORKAREA, 0, &workarea, 0);

        int x_pos = (display_width - width) / 2;
        int y_pos = (((display_height - height) / 2) - (display_height - workarea.bottom));
        
        DEBUG_INFO("Create_Main_Window() - Moving window (%d,%d,%d,%d).\n",
            x_pos, y_pos, (rect.right - rect.left), (rect.bottom - rect.top));

        MoveWindow(hWnd, x_pos, y_pos, (rect.right - rect.left), (rect.bottom - rect.top), TRUE);

    } else {

        DEBUG_INFO("Create_Main_Window() - Creating fullscreen window.\n");

        hWnd = CreateWindowEx(
            WS_EX_TOPMOST,
            "Vinifera",
            Vinifera_Get_Window_Title(dwPid),
            WS_POPUP|WS_CLIPCHILDREN,
            0, 0,
            display_width,
            display_height,
            nullptr,
            nullptr,
            (HINSTANCE)hInstance,
            nullptr);
    }

    if (!hWnd) {
        DEBUG_INFO("Create_Main_Window() - Failed to create window!\n");
        return;
    }

    //DEV_DEBUG_INFO("Create_Main_Window() - About to call ShowWindow()\n");

    ShowWindow(hWnd, SW_SHOWNORMAL);
    ShowCommand = nCmdShow;

    //DEV_DEBUG_INFO("Create_Main_Window() - About to call UpdateWindow()\n");

    UpdateWindow(hWnd);

    //DEV_DEBUG_INFO("Create_Main_Window() - About to call SetFocus()\n");

    SetFocus(hWnd);

    //DEV_DEBUG_INFO("Create_Main_Window() - About to call RegisterHotKey()\n");

    RegisterHotKey(hWnd, 1, MOD_ALT|MOD_CONTROL|MOD_SHIFT, VK_M);

    //DEV_DEBUG_INFO("Create_Main_Window() - About to call SetCursor()\n");

    SetCursor(hCursor);

    Audio.AudioFocusLossFunction = &Focus_Loss;

    /**
     *  Save the handle to our main window.
     */
    MainWindow = hWnd;
    ProgramInstance = hInstance;

    /**
     *  #NOTE:
     *  This had been added to resolved a issue where the game gets stuck in a
     *  focus checking loop, this could be because the DLL now creates the 
     *  window in its thread.
     */
    GameInFocus = true;

    //DEV_DEBUG_INFO("Create_Main_Window(exit)\n");
}


/**
 *  Reimplemention of Prep_For_Side()
 *  
 *  Prepare the mixfiles for the player side.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Prep_For_Side(SideType side)
{
    DEBUG_INFO("Preparing Mixfiles for Side %02d.\n", side);

    MFCC *mix = nullptr;
    char buffer[16];

    int sidenum = (side+1); // Logical side number.

    if (SideCachedMix) {
        DEBUG_INFO("  Releasing %s\n", SideCachedMix->Filename);
        delete SideCachedMix;
        SideCachedMix = nullptr;
    }
    if (SideNotCachedMix) {
        DEBUG_INFO("  Releasing %s\n", SideNotCachedMix->Filename);
        delete SideNotCachedMix;
        SideNotCachedMix = nullptr;
    }
    if (SideCDMix) {
        DEBUG_INFO("  Releasing %s\n", SideCDMix->Filename);
        delete SideCDMix;
        SideCDMix = nullptr;
    }

    for (int i = 0; i < SideMixFiles.Count(); ++i) {
        DEBUG_INFO("  Releasing %s\n", SideMixFiles[i]->Filename);
        delete SideMixFiles[i];
        SideMixFiles.Delete(i);
    }

    if (Addon_Enabled(ADDON_ANY) == ADDON_FIRESTORM) {

        for (int i = 99; i >= 0; --i) {
            std::snprintf(buffer, sizeof(buffer), "E%02dSC%02d.MIX", i, sidenum);
            if (CCFileClass(buffer).Is_Available()) {
                mix = new MFCC(buffer, &FastKey);
                ASSERT(mix);
                if (!mix) {
                    DEBUG_WARNING("  Failed to load %s!\n", buffer);
                    //return false; // #issue-193: Unable to load side mix files is no longer a fatal error.
                }
                if (!mix->Cache()) {
                    DEBUG_WARNING("  Failed to cache %s!\n", buffer);
                    return false;
                }
                ExpansionMixFiles.Add(mix);
                DEBUG_INFO(" %s\n", buffer);
            }
        }

    }

    std::snprintf(buffer, sizeof(buffer), "SIDEC%02d.MIX", sidenum);
    if (CCFileClass(buffer).Is_Available()) {
        SideCachedMix = new MFCC(buffer, &FastKey);
        ASSERT(SideCachedMix);
        if (!SideCachedMix) {
            DEBUG_WARNING("  Failed to load %s!\n", buffer);
            //return false; // #issue-193: Unable to load side mix files is no longer a fatal error.
        }
        if (!SideCachedMix->Cache()) {
            DEBUG_WARNING("  Failed to cache %s!\n", buffer);
            return false;
        }
        DEBUG_INFO(" %s\n", buffer);
    }

    std::snprintf(buffer, sizeof(buffer), "SIDENC%02d.MIX", sidenum);
    if (CCFileClass(buffer).Is_Available()) {
        SideNotCachedMix = new MFCC(buffer, &FastKey);
        ASSERT(SideNotCachedMix);
        if (!SideNotCachedMix) {
            DEBUG_WARNING("  Failed to load %s!\n", buffer);
            //return false; // #issue-193: Unable to load side mix files is no longer a fatal error.
        }
        DEBUG_INFO(" %s\n", buffer);
    }

    if (Session.Type == GAME_NORMAL) {
        if (Addon_Enabled(ADDON_ANY) == ADDON_FIRESTORM) {
            std::snprintf(buffer, sizeof(buffer), "E%02dSCD%02d.MIX", Get_Required_Addon(), sidenum);
        } else {
            std::snprintf(buffer, sizeof(buffer), "SIDECD%02d.MIX", sidenum);
        }
        if (CCFileClass(buffer).Is_Available()) {
            SideCDMix = new MFCC(buffer, &FastKey);
            ASSERT(SideCDMix);
            if (!SideCDMix) {
                DEBUG_WARNING("  Failed to load %s!\n", buffer);
                //return false; // #issue-193: Unable to load side mix files is no longer a fatal error.
            }
            DEBUG_INFO(" %s\n", buffer);
        }
    }

    Map.Init_For_House();

    return true;
}


/**
 *  Reimplemention of Init_Secondary_Mixfiles()
 *  
 *  Register and cache secondary mixfiles.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Init_Secondary_Mixfiles()
{
    MFCC *mix;
    char buffer[16];

    DEBUG_INFO("\n"); // Fixes missing new-line after "Init Secondary Mixfiles....." print.
    //DEBUG_INFO("Init secondary mixfiles...\n");

    /**
     *  #issue-653
     * 
     *  Adds support for loading GENERIC.MIX and ISOGEN.MIX mix files.
     * 
     *  @author: CCHyper
     */
    if (CCFileClass("GENERIC.MIX").Is_Available()) {
        GenericMix = new MFCC("GENERIC.MIX", &FastKey);
        ASSERT(GenericMix);
    }
    if (!GenericMix) {
        DEV_DEBUG_WARNING("Failed to load GENERIC.MIX!\n");
    } else {
        GenericMix->Cache();
        DEBUG_INFO(" GENERIC.MIX\n");
    }
    if (CCFileClass("ISOGEN.MIX").Is_Available()) {
        IsoGenericMix = new MFCC("ISOGEN.MIX", &FastKey);
        ASSERT(IsoGenericMix);
    }
    if (!IsoGenericMix) {
        DEV_DEBUG_WARNING("Failed to load ISOGEN.MIX!\n");
    } else {
        IsoGenericMix->Cache();
        DEBUG_INFO(" ISOGEN.MIX\n");
    }

    if (CCFileClass("CONQUER.MIX").Is_Available()) {
        ConquerMix = new MFCC("CONQUER.MIX", &FastKey);
        ASSERT(ConquerMix);
    }
    if (!ConquerMix) {
        DEBUG_WARNING("Failed to load CONQUER.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" CONQUER.MIX\n");
    }

    int cd = CD::Get_Volume_Index();

    /**
     *  Make sure we have a grounded volume index (invalid volumes will cause error).
     */
    if (CD::Get_Volume_Index() < 0) {
        cd = 0;
    }

    /**
     *  Mix file indices are 1 based.
     */
    cd += 1;

    /**
     *  #issue-513
     * 
     *  If the CD system has been flagged that the files are local, we
     *  just glob all the map mix files in the game directory.
     * 
     *  @author: CCHyper
     */
    if (CD::IsFilesLocal) {

        std::snprintf(buffer, sizeof(buffer), "MAPS*.MIX");
        if (CCFileClass::Find_First_File(buffer)) {
            DEBUG_INFO(" %s\n", buffer);
            MapsMix = new MFCC(buffer, &FastKey);
            ASSERT(MapsMix);
            while (CCFileClass::Find_Next_File(buffer)) {
                DEBUG_INFO(" %s\n", buffer);
                mix = new MFCC(buffer, &FastKey);
                ASSERT(mix);
                if (mix) {
                    ViniferaMapsMixes.Add(mix);
                }
            }
        }
        CCFileClass::Find_Close();

    } else {
        std::snprintf(buffer, sizeof(buffer), "MAPS%02d.MIX", cd);
        if (CCFileClass(buffer).Is_Available()) {
            MapsMix = new MFCC(buffer, &FastKey);
            ASSERT(MapsMix);
        }
    }
    if (!MapsMix) {
        DEBUG_WARNING("Failed to load %s!\n", buffer);
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        if (!CD::IsFilesLocal) DEBUG_INFO(" %s\n", buffer);
    }

    if (CCFileClass("MULTI.MIX").Is_Available()) {
        MultiMix = new MFCC("MULTI.MIX", &FastKey);
        ASSERT(MultiMix);
    }
    if (!MultiMix) {
        DEBUG_WARNING("Failed to load MULTI.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" MULTI.MIX\n", buffer);
    }

    if (Addon_Installed(ADDON_FIRESTORM)) {
        if (CCFileClass("SOUNDS01.MIX").Is_Available()) {
            FSSoundsMix = new MFCC("SOUNDS01.MIX", &FastKey);
            ASSERT(FSSoundsMix);
        }
        if (!FSSoundsMix) {
            DEBUG_WARNING("Failed to load SOUNDS01.MIX!\n");
            //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
        } else {
            DEBUG_INFO(" SOUNDS01.MIX\n", buffer);
        }
    }

    if (CCFileClass("SOUNDS.MIX").Is_Available()) {
        SoundsMix = new MFCC("SOUNDS.MIX", &FastKey);
        ASSERT(SoundsMix);
    }
    if (!SoundsMix) {
        DEBUG_WARNING("Failed to load SOUNDS.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" SOUNDS.MIX\n", buffer);
    }

    if (CCFileClass("SCORES01.MIX").Is_Available()) {
        FSScoresMix = new MFCC("SCORES01.MIX", &FastKey);
        ASSERT(FSScoresMix);
    }
    if (!FSScoresMix) {
        DEBUG_WARNING("Failed to load SCORES01.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" SCORES01.MIX\n", buffer);
    }

	/*
	**	Register the score mixfile.
	*/
    if (CCFileClass("SCORES.MIX").Is_Available()) {
        ScoreMix = new MFCC("SCORES.MIX", &FastKey);
        ASSERT(ScoreMix);
    }
    if (!ScoreMix) {
        DEBUG_WARNING("Failed to load SCORES.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" SCORES.MIX\n", buffer);
    }
	ScoresPresent = true;
	Theme.Scan();

    /**
     *  #issue-513
     * 
     *  If the CD system has been flagged that the files are local, we
     *  just glob all the movies mix files in the game directory.
     * 
     *  @author: CCHyper
     */
    if (CD::IsFilesLocal) {

        std::snprintf(buffer, sizeof(buffer), "MOVIES*.MIX");
        if (CCFileClass::Find_First_File(buffer)) {
            DEBUG_INFO(" %s\n", buffer);
            MoviesMix = new MFCC(buffer, &FastKey);
            ASSERT(MoviesMix);
            while (CCFileClass::Find_Next_File(buffer)) {
                DEBUG_INFO(" %s\n", buffer);
                mix = new MFCC(buffer, &FastKey);
                ASSERT(mix);
                if (mix) {
                    ViniferaMoviesMixes.Add(mix);
                }
            }
        }
        CCFileClass::Find_Close();

    } else {
        std::snprintf(buffer, sizeof(buffer), "MOVIES%02d.MIX", cd);
        if (CCFileClass(buffer).Is_Available()) {
            MoviesMix = new MFCC(buffer, &FastKey);
            ASSERT(MoviesMix);
        }
    }
    if (!MoviesMix) {
        DEBUG_WARNING("Failed to load %s!\n", buffer);
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        if (!CD::IsFilesLocal) DEBUG_INFO(" %s\n", buffer);
    }

    return true;
}


/**
 *  Register and cache expansion mixfiles.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Init_Expansion_Mixfiles()
{
    MFCC *mix;
    char buffer[16];

    for (int i = 99; i >= 0; --i) {
        std::snprintf(buffer, sizeof(buffer), "EXPAND%02d.MIX", i);
        if (CCFileClass(buffer).Is_Available()) {
            mix = new MFCC(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load %s!\n", buffer);
            } else {
                ExpansionMixFiles.Add(mix);
                DEBUG_INFO(" %s\n", buffer);
            }
        }
    }

    for (int i = 99; i >= 0; --i) {
        std::snprintf(buffer, sizeof(buffer), "ECACHE%02d.MIX", i);
        if (CCFileClass(buffer).Is_Available()) {
            mix = new MFCC(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load %s!\n", buffer);
            } else {
                mix->Cache();
                ExpansionMixFiles.Add(mix);
                DEBUG_INFO(" %s\n", buffer);
            }
        }
    }

    /**
     *  #issue-648
     * 
     *  Load ELOCAL*.MIX expansion mixfiles.
     * 
     *  #NOTE:
     *  Red Alert 2 uses the wild-card system to load these files, but to retain
     *  the file naming format Tiberian Sun uses, we now use 00-99.
     * 
     *  @author: CCHyper
     */
#if 0
    std::snprintf(buffer, sizeof(buffer), "ELOCAL*.MIX");
    if (CCFileClass::Find_First_File(buffer)) {
        DEBUG_INFO(" %s\n", buffer);
        mix = new MFCC(buffer, &FastKey);
        ASSERT(mix);
        while (CCFileClass::Find_Next_File(buffer)) {
            DEBUG_INFO(" %s\n", buffer);
            mix = new MFCC(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load %s!\n", buffer);
            } else {
                ExpansionMixFiles.Add(mix);
                DEBUG_INFO(" %s\n", buffer);
            }
        }
    }
    CCFileClass::Find_Close();
#else
    for (int i = 99; i >= 0; --i) {
        std::snprintf(buffer, sizeof(buffer), "ELOCAL%02d.MIX", i);
        if (CCFileClass(buffer).Is_Available()) {
            mix = new MFCC(buffer, &FastKey);
            ASSERT(mix);
            if (!mix) {
                DEBUG_WARNING("Failed to load %s!\n", buffer);
            } else {
                ExpansionMixFiles.Add(mix);
                DEBUG_INFO(" %s\n", buffer);
            }
        }
    }
#endif

    return true;
}


/**
 *  Reimplemention of Init_Bootstrap_Mixfiles()
 *  
 *  Registers and caches any mixfiles needed for bootstrapping.
 * 
 *  @author: CCHyper
 */
bool Vinifera_Init_Bootstrap_Mixfiles()
{
    bool ok;
    MFCC *mix;

    int temp = CD::RequiredCD;
    CD::Set_Required_CD(-2);

    DEBUG_INFO("\n"); // Fixes missing new-line after "Bootstrap..." print.
    //DEBUG_INFO("Init bootstrap mixfiles...\n");

    if (CCFileClass("PATCH.MIX").Is_Available()) {
        mix = new MFCC("PATCH.MIX", &FastKey);
        ASSERT(mix);
        if (mix) {
            DEBUG_INFO(" PATCH.MIX\n");
        }
    }

    if (CCFileClass("PCACHE.MIX").Is_Available()) {
        mix = new MFCC("PCACHE.MIX", &FastKey);
        ASSERT(mix);
        if (mix) {
            mix->Cache();
            DEBUG_INFO(" PCACHE.MIX\n");
        }
    }

    Vinifera_Init_Expansion_Mixfiles();

    Addon_Present();

    TibSunMix = new MFCC("TIBSUN.MIX", &FastKey);
    ASSERT(TibSunMix);
    if (!TibSunMix) {
        DEBUG_WARNING("Failed to load TIBSUN.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" TIBSUN.MIX\n");
    }

    /*
    **	Bootstrap enough of the system so that the error dialog
    *   box can successfully be displayed.
    */
    CacheMix = new MFCC("CACHE.MIX", &FastKey);
    ASSERT(CacheMix);
    if (!CacheMix) {
        DEBUG_WARNING("Failed to load CACHE.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        if (!CacheMix->Cache()) {
            DEBUG_WARNING("Failed to cache CACHE.MIX!\n");
            return false;
        }
        DEBUG_INFO(" CACHE.MIX\n");
    }

    LocalMix = new MFCC("LOCAL.MIX", &FastKey);
    ASSERT(LocalMix);
    if (!LocalMix) {
        DEBUG_WARNING("Failed to load LOCAL.MIX!\n");
        //return false; // #issue-110: Unable to load startup mix files is no longer a fatal error.
    } else {
        DEBUG_INFO(" LOCAL.MIX\n");
    }

    CD::Set_Required_CD(temp);

    return true;
}
