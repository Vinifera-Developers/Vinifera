/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Main-window Dear ImGui integration.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "vinifera_imgui.h"

#include "audio_manager.h"
#include "debug_overlay.h"
#include "debughandler.h"
#include "movieskip.h"
#include "scenario_overlay.h"
#include "vinifera_globals.h"
#include "zbuffer_window.h"

#include <imgui.h>
#include <imgui_impl_sdlrenderer3.h>
#include <imgui_impl_win32.h>

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif

/**
 *  Forward declare message handler from imgui_impl_win32.cpp.
 */
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace
{
    static bool IsInitialized = false;

    static bool Is_Mouse_Position_Message(UINT message)
    {
        switch (message) {
        case WM_MOUSEMOVE:
        case WM_MOUSELEAVE:
        case WM_NCMOUSEMOVE:
        case WM_NCMOUSELEAVE:
            return true;
        default:
            return false;
        }
    }

    static bool Is_Mouse_Button_Message(UINT message)
    {
        switch (message) {
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_XBUTTONDBLCLK:
            return true;
        default:
            return false;
        }
    }

    static bool Is_Mouse_Wheel_Message(UINT message)
    {
        switch (message) {
        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
            return true;
        default:
            return false;
        }
    }

    static bool Is_Keyboard_Message(UINT message)
    {
        switch (message) {
        case WM_KEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_CHAR:
        case WM_SYSCHAR:
            return true;
        default:
            return false;
        }
    }
}

/**
 *  Initializes the main-window ImGui context and backends.
 *
 *  @author: ZivDero
 */
bool ViniferaImGui::Initialize(HWND hwnd, SDL_Renderer* renderer)
{
    if (IsInitialized) {
        return true;
    }

    if (hwnd == nullptr || renderer == nullptr) {
        return false;
    }

    ImGui_ImplWin32_EnableDpiAwareness();

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    /**
     *  SDLMouseClass drives the cursor via SDL_SetCursor / SDL_HideCursor.
     *  Stop the Win32 backend from calling SetCursor() each NewFrame, which
     *  races SDL's WM_SETCURSOR handler and produces a flicker on mouse motion.
     */
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    io.IniFilename = nullptr;

    ImGui::StyleColorsDark();

    if (!ImGui_ImplWin32_Init(hwnd)) {
        ImGui::DestroyContext();
        return false;
    }

    if (!ImGui_ImplSDLRenderer3_Init(renderer)) {
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        return false;
    }

    IsInitialized = true;

    return true;
}

/**
 *  Shuts down the main-window ImGui context and backends.
 *
 *  @author: ZivDero
 */
void ViniferaImGui::Shutdown()
{
    if (!IsInitialized) {
        return;
    }

    ZBufferDebugWindow::Shutdown();

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    IsInitialized = false;
}

/**
 *  Forwards a Win32 message to ImGui and returns whether ImGui consumed it.
 *
 *  @author: ZivDero
 */
bool ViniferaImGui::Process_Window_Message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    if (!IsInitialized) {
        return false;
    }

    ImGuiIO& io = ImGui::GetIO();

    /*
    **  The Win32 backend mutates global mouse capture on button messages.
    **  Only feed those messages when ImGui already wants mouse input; mouse
    **  movement is still always fed so hover state can become true.
    */
    if (Is_Mouse_Position_Message(msg)) {
        ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
        return false;
    }

    if (Is_Mouse_Button_Message(msg) || Is_Mouse_Wheel_Message(msg)) {
        if (!io.WantCaptureMouse) {
            return false;
        }

        ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
        return true;
    }

    if (Is_Keyboard_Message(msg)) {
        /*
        **  Always forward to ImGui so its key state stays consistent, but only
        **  *consume* the message when a text widget is actually focused.
        **  `WantCaptureKeyboard` would stay sticky under `NavEnableKeyboard`
        **  and steal every game hotkey whenever any ImGui window is open.
        */
        ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);
        return io.WantTextInput;
    }

    ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam);

    return false;
}

namespace
{
    static void Render_Frame(bool include_game_overlays)
    {
        if (!IsInitialized || SDLWindowRenderer == nullptr) {
            return;
        }

        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

#ifndef NDEBUG
        if (include_game_overlays && Vinifera_AudioDebug) {
            AudioManager.Draw_Debug_UI();
        }
        //ZBufferDebugWindow::Draw();
#endif

        if (include_game_overlays) {
            DebugOverlay::Draw();
            ScenarioOverlay::Draw();
        }

        MovieSkip::Draw_Overlay();

        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), SDLWindowRenderer);
    }
}


/**
 *  Renders the main-window ImGui frame through the active SDL renderer.
 *
 *  @author: ZivDero
 */
void ViniferaImGui::Render()
{
    Render_Frame(true);
}


/**
 *  Renders only overlays intended for fullscreen modern movie playback.
 */
void ViniferaImGui::Render_Movie_Overlay()
{
    Render_Frame(false);
}

/**
 *  Returns whether the main-window ImGui context is initialized.
 *
 *  @author: ZivDero
 */
bool ViniferaImGui::Is_Initialized()
{
    return IsInitialized;
}
