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
    hIcon = LoadIcon((HINSTANCE)DLLInstance, MAKEINTRESOURCE(VINIFERA_MAINICON));
    if (!hIcon) {
        hIcon = LoadIcon((HINSTANCE)hInstance, MAKEINTRESOURCE(TS_MAINICON));
    }
    hCursor = LoadCursor(nullptr, VINIFERA_MAINCURSOR); // IDC_ARROW is a system resource, does not require module.
    if (!hCursor) {
        hCursor = LoadCursor((HINSTANCE)hInstance, MAKEINTRESOURCE(TS_MAINCURSOR));
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
    wc.hIconSm        = hIcon;

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
