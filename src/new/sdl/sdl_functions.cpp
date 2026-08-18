/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains functions for the SDL system.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sdl_functions.h"

#include "SDL3/SDL_hints.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include "cctooltip.h"
#include "cdctrl.h"
#include "command.h"
#include "convert.h"
#include "debughandler.h"
#include "mouse.h"
#include "optionsext.h"
#include "ownrdraw.h"
#include "playmovie.h"
#include "rect.h"
#include "sdl_movie.h"
#include "sdlmouse.h"
#include "sdlsurface.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "vinifera_imgui.h"
#include "vinifera_util.h"
#include "windialog.h"
#include "wsproto.h"
#include "wwmouse.h"

#include <algorithm>
#include <cmath>
#include <commctrl.h>
#include <unordered_map>
#include <windowsx.h>

#ifndef WM_MOUSEHWHEEL
#define WM_MOUSEHWHEEL 0x020E
#endif


namespace
{
    thread_local bool SDLForwardingMouseMessage = false;
    thread_local bool SDLCallingTranslatedMouseProc = false;

    std::unordered_map<HWND, WNDPROC> SDLChildWindowProcedures;

    /**
     *  The border thickness used by OwnerDraw controls, fixed at 1 by
     *  OwnerDraw's one-time initialization (vanilla ODBorderThickness).
     */
    constexpr int OD_BORDER_THICKNESS = 1;

    struct SDLTrackbarDragInfo
    {
        bool IsDragging = false;
        bool HasRange = false;
        int Minimum = 0;
        int Maximum = 0;

        /**
         *  Step set by OD_SETTRACKSTEP; 0 means never set, which the game's
         *  trackbar procedure treats as a step of 1.
         */
        int Step = 0;

        /**
         *  Set by OD_TRACKNUMBERS. Trackbars showing numbers reserve a
         *  50-pixel value-number gutter at the right side of the control.
         */
        bool ShowNumbers = true;
    };
    std::unordered_map<HWND, SDLTrackbarDragInfo> SDLTrackbarDragState;

    struct SDLScrollBarDragInfo
    {
        bool IsDragging = false;
        bool HasRange = false;
        int Range = 0;
    };
    std::unordered_map<HWND, SDLScrollBarDragInfo> SDLScrollBarDragState;

    LRESULT CALLBACK SDL_Child_Windows_Procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam);
    void SDL_Subclass_Child_Window(HWND window);

    /**
     *  Calls a child window's saved original procedure without re-entering the
     *  coordinate router.
     *
     *  @author: Rampastring
     */
    LRESULT SDL_Call_Stored_Child_Window_Procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
    {
        auto proc = SDLChildWindowProcedures.find(window);
        if (proc == SDLChildWindowProcedures.end() || proc->second == nullptr) {
            return DefWindowProc(window, message, wparam, lparam);
        }

        const bool was_calling_translated_mouse_proc = SDLCallingTranslatedMouseProc;
        SDLCallingTranslatedMouseProc = true;
        LRESULT result = CallWindowProc(proc->second, window, message, wparam, lparam);
        SDLCallingTranslatedMouseProc = was_calling_translated_mouse_proc;

        return result;
    }

    /**
     *  Returns true if this message carries mouse coordinates in lParam.
     *
     *  @author: Rampastring
     */
    bool SDL_Is_Mouse_Coordinate_Message(UINT message)
    {
        switch (message) {
        case WM_MOUSEMOVE:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONUP:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONUP:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONUP:
        case WM_MBUTTONDBLCLK:
        case WM_MOUSEWHEEL:
        case WM_MOUSEHWHEEL:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONUP:
        case WM_XBUTTONDBLCLK:
            return true;

        default:
            return false;
        }
    }

    /**
     *  Mouse wheel messages carry screen coordinates. Other mouse messages use
     *  coordinates relative to the receiving window's client area.
     *
     *  @author: Rampastring
     */
    bool SDL_Mouse_Message_Uses_Screen_Coordinates(UINT message)
    {
        return message == WM_MOUSEWHEEL || message == WM_MOUSEHWHEEL;
    }

    /**
     *  Packs signed mouse coordinates into an LPARAM.
     *
     *  @author: Rampastring
     */
    LPARAM SDL_Make_Mouse_LParam(int x, int y)
    {
        return MAKELPARAM(static_cast<SHORT>(x), static_cast<SHORT>(y));
    }

    /**
     *  Returns true if the window is a standard Win32 trackbar control.
     *
     *  @author: Rampastring
     */
    bool SDL_Is_Trackbar_Window(HWND window)
    {
        char class_name[64] = {};
        if (GetClassNameA(window, class_name, sizeof(class_name)) == 0) {
            return false;
        }

        return lstrcmpiA(class_name, "msctls_trackbar32") == 0;
    }

    /**
     *  Returns true if the window is the game's custom combo box drop-down
     *  child window.
     *
     *  @author: Rampastring
     */
    bool SDL_Is_Combo_Dropdown_Window(HWND window)
    {
        char class_name[64] = {};
        if (GetClassNameA(window, class_name, sizeof(class_name)) == 0) {
            return false;
        }

        return lstrcmpiA(class_name, "ComboDropWin") == 0;
    }

    /**
     *  Returns true if the window is a standard Win32 scrollbar control
     *  (the game's listboxes and combo drop-downs attach these).
     *
     *  @author: ZivDero
     */
    bool SDL_Is_ScrollBar_Window(HWND window)
    {
        char class_name[64] = {};
        if (GetClassNameA(window, class_name, sizeof(class_name)) == 0) {
            return false;
        }

        return lstrcmpiA(class_name, "ScrollBar") == 0;
    }

    /**
     *  Returns true for child controls whose capture should override normal
     *  logical hit testing.
     *
     *  @author: Rampastring
     */
    bool SDL_Should_Use_Captured_Child_Window(HWND window)
    {
        return SDL_Is_Trackbar_Window(window) || SDL_Is_Combo_Dropdown_Window(window) || SDL_Is_ScrollBar_Window(window);
    }

    /**
     *  Returns true for child controls whose captured mouse handling must pass
     *  through the SDL coordinate-transforming proxy.
     *
     *  @author: Rampastring
     */
    bool SDL_Should_Subclass_Child_Window(HWND window)
    {
        return SDL_Should_Use_Captured_Child_Window(window);
    }

    /**
     *  Converts a point in the source message's coordinate space to physical
     *  coordinates relative to MainWindow's client area.
     *
     *  @author: Rampastring
     */
    POINT SDL_Mouse_Message_Point_To_Main_Window(HWND source, UINT message, LPARAM lparam)
    {
        POINT point = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };

        if (SDL_Mouse_Message_Uses_Screen_Coordinates(message)) {
            ScreenToClient(MainWindow, &point);
        } else if (source != MainWindow) {
            MapWindowPoints(source, MainWindow, &point, 1);
        }

        return point;
    }

    /**
     *  Converts a physical MainWindow client point to the unscaled
     *  coordinates used by TS dialogs and surfaces based on the
     *  resolution the game thinks it is running at.
     *
     *  @author: Rampastring
     */
    POINT SDL_Physical_Point_To_Logical_Game_Point(POINT point)
    {
        point.x = static_cast<LONG>(point.x * SDL_XScale());
        point.y = static_cast<LONG>(point.y * SDL_YScale());

        return point;
    }

    /**
     *  Returns the logical range that was previously sent to a game-owned
     *  trackbar subclass.
     *
     *  @author: Rampastring
     */
    bool SDL_Get_Recorded_Trackbar_Range(HWND trackbar, int& min_pos, int& max_pos)
    {
        auto state = SDLTrackbarDragState.find(trackbar);
        if (state != SDLTrackbarDragState.end() && state->second.HasRange && state->second.Maximum > state->second.Minimum) {
            min_pos = state->second.Minimum;
            max_pos = state->second.Maximum;
            return true;
        }

        return false;
    }

    /**
     *  Computes and applies a trackbar position from a logical client point,
     *  mirroring the position math of the game's own trackbar procedure.
     *  The game's procedure sends the WM_HSCROLL parent notification itself
     *  when the position actually changes.
     *
     *  @author: Rampastring, ZivDero
     */
    int SDL_Set_Trackbar_Pos_From_Point(HWND trackbar, POINT point)
    {
        const int min_pos = static_cast<int>(SDL_Call_Stored_Child_Window_Procedure(trackbar, TBM_GETRANGEMIN, 0, 0));
        int max_pos = static_cast<int>(SDL_Call_Stored_Child_Window_Procedure(trackbar, TBM_GETRANGEMAX, 0, 0));
        int effective_min_pos = min_pos;
        int effective_max_pos = max_pos;

        if (effective_max_pos <= effective_min_pos) {
            SDL_Get_Recorded_Trackbar_Range(trackbar, effective_min_pos, effective_max_pos);
        }

        const int range = effective_max_pos - effective_min_pos;
        if (range <= 0) {
            return effective_min_pos;
        }

        RECT client_rect = {};
        GetClientRect(trackbar, &client_rect);

        SDLTrackbarDragInfo& state = SDLTrackbarDragState[trackbar];
        const int step = state.Step > 0 ? state.Step : 1;
        const int number_width = state.ShowNumbers ? 50 : 0;
        int slider_width = client_rect.right - client_rect.left - number_width - 13;
        if (slider_width <= 1) {
            slider_width = 1;
        }

        int xpos = point.x - 6;
        if (xpos < 1) {
            xpos = 1;
        }

        int max_x = client_rect.right - number_width - 12;
        xpos = std::min(xpos, max_x);

        if (max_x <= 1) {
            return effective_min_pos;
        }

        int idx = ((range + 1) * (xpos - 1)) / slider_width;
        if (idx >= range) {
            idx = range;
        }

        int new_pos = step * ((effective_min_pos + idx) / step);
        new_pos = std::clamp(new_pos, effective_min_pos, effective_max_pos);
        state.HasRange = true;
        state.Minimum = effective_min_pos;
        state.Maximum = effective_max_pos;
        SDL_Call_Stored_Child_Window_Procedure(trackbar, TBM_SETPOS, TRUE, new_pos);
        return new_pos;
    }

    /**
     *  Handles scaled dragging for standard Win32 trackbars without relying on
     *  the common control's internal mouse-to-position tracking.
     *
     *  @author: Rampastring
     */
    bool SDL_Handle_Trackbar_Mouse_Message(HWND trackbar, UINT message, LPARAM lparam)
    {
        if (!SDL_Is_Trackbar_Window(trackbar)) {
            return false;
        }

        switch (message) {
        case WM_LBUTTONDOWN:
        {
            SetFocus(trackbar);
            SetCapture(trackbar);
            SDLTrackbarDragInfo& state = SDLTrackbarDragState[trackbar];
            state.IsDragging = true;

            POINT point = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
            SDL_Set_Trackbar_Pos_From_Point(trackbar, point);
            return true;
        }

        case WM_MOUSEMOVE:
        {
            auto drag_state = SDLTrackbarDragState.find(trackbar);
            if (drag_state != SDLTrackbarDragState.end() && drag_state->second.IsDragging && GetCapture() == trackbar) {
                POINT point = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
                SDL_Set_Trackbar_Pos_From_Point(trackbar, point);
                return true;
            }
            return false;
        }

        case WM_LBUTTONUP:
        {
            auto drag_state = SDLTrackbarDragState.find(trackbar);
            if (drag_state != SDLTrackbarDragState.end() && drag_state->second.IsDragging) {
                POINT point = { GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam) };
                SDL_Set_Trackbar_Pos_From_Point(trackbar, point);
                drag_state->second.IsDragging = false;

                if (GetCapture() == trackbar) {
                    ReleaseCapture();
                }
                return true;
            }
            return false;
        }

        case WM_CAPTURECHANGED:
        {
            auto drag_state = SDLTrackbarDragState.find(trackbar);
            if (drag_state != SDLTrackbarDragState.end() && drag_state->second.IsDragging) {
                drag_state->second.IsDragging = false;
            }
            return false;
        }

        case WM_NCDESTROY:
            SDLTrackbarDragState.erase(trackbar);
            return false;

        default:
            return false;
        }
    }

    /**
     *  Computes the scrollbar grip geometry the same way the game's own
     *  scrollbar procedure does.
     *
     *  @author: ZivDero
     */
    struct SDLScrollBarMetrics
    {
        RECT ClientRect;
        int GripHeight;
        int Travel;
        int GripTop;
        int GripBottom;
    };

    void SDL_Get_ScrollBar_Metrics(HWND scrollbar, int range, SDLScrollBarMetrics& metrics)
    {
        GetClientRect(scrollbar, &metrics.ClientRect);
        metrics.ClientRect.right -= 2 * OD_BORDER_THICKNESS;
        metrics.ClientRect.bottom -= 2 * OD_BORDER_THICKNESS;

        const int travel_height = metrics.ClientRect.bottom - metrics.ClientRect.top - 44;
        metrics.GripHeight = static_cast<int>(travel_height - std::log(static_cast<double>(range + 1)) * travel_height * 0.2);
        if (metrics.GripHeight <= 14) {
            metrics.GripHeight = 14;
        }

        metrics.Travel = travel_height - metrics.GripHeight;
        if (metrics.Travel <= 1) {
            metrics.Travel = 1;
        }

        int position = static_cast<int>(SDL_Call_Stored_Child_Window_Procedure(scrollbar, SBM_GETPOS, 0, 0));
        position = std::clamp(position, 0, range);
        metrics.GripTop = position * metrics.Travel / range + metrics.ClientRect.top + 22;
        metrics.GripBottom = metrics.GripTop + metrics.GripHeight;
    }

    /**
     *  Applies a scrollbar position through the game's own scrollbar
     *  procedure, which notifies the owner window and repaints itself
     *  when the position actually changes.
     *
     *  @author: ZivDero
     */
    void SDL_Apply_ScrollBar_Position(HWND scrollbar, int range, int position)
    {
        SCROLLINFO info {};
        info.cbSize = sizeof(info);
        info.fMask = SIF_RANGE | SIF_POS;
        info.nMax = range;
        info.nPos = std::clamp(position, 0, range);
        SDL_Call_Stored_Child_Window_Procedure(scrollbar, SBM_SETSCROLLINFO, 0, reinterpret_cast<LPARAM>(&info));
    }

    /**
     *  Computes and applies a scrollbar position from a logical client point,
     *  mirroring the grip math of the game's own scrollbar procedure.
     *
     *  @author: ZivDero
     */
    void SDL_Set_ScrollBar_Pos_From_Point(HWND scrollbar, int range, const SDLScrollBarMetrics& metrics, int y)
    {
        int grip_top = y - metrics.GripHeight / 2;
        if (grip_top < 22) {
            grip_top = 22;
        }

        const int max_top = metrics.ClientRect.bottom - metrics.GripHeight - 22;
        if (grip_top > max_top) {
            grip_top = max_top;
        }

        SDL_Apply_ScrollBar_Position(scrollbar, range, range * (grip_top - 22) / metrics.Travel);
    }

    /**
     *  Returns the current physical cursor position converted to the given
     *  window's logical client coordinates.
     *
     *  @author: ZivDero
     */
    POINT SDL_Logical_Cursor_Point_For_Window(HWND window)
    {
        POINT point;
        GetCursorPos(&point);
        ScreenToClient(MainWindow, &point);
        point = SDL_Physical_Point_To_Logical_Game_Point(point);
        if (window != MainWindow) {
            MapWindowPoints(MainWindow, window, &point, 1);
        }
        return point;
    }

    /**
     *  Handles scaled grip dragging and arrow auto-repeat for the game's
     *  scrollbar controls. The game's own scrollbar procedure computes both
     *  from the physical cursor position (GetCursorPos), which does not map
     *  to the logical control layout when the display is scaled. Arrow and
     *  track clicks are left to the game's procedure, which handles them
     *  from the (already translated) message coordinates.
     *
     *  @author: ZivDero
     */
    bool SDL_Handle_ScrollBar_Mouse_Message(HWND scrollbar, UINT message, LPARAM lparam)
    {
        if (!SDL_Is_ScrollBar_Window(scrollbar)) {
            return false;
        }

        auto state = SDLScrollBarDragState.find(scrollbar);
        const bool has_range = state != SDLScrollBarDragState.end() && state->second.HasRange && state->second.Range > 0;

        switch (message) {
        case WM_LBUTTONDOWN:
        {
            if (!has_range) {
                return false;
            }

            SDLScrollBarMetrics metrics;
            SDL_Get_ScrollBar_Metrics(scrollbar, state->second.Range, metrics);

            const int y = GET_Y_LPARAM(lparam);
            if (y < metrics.GripTop || y >= metrics.GripBottom) {
                return false;
            }

            SetCapture(scrollbar);
            state->second.IsDragging = true;
            return true;
        }

        case WM_MOUSEMOVE:
        {
            if (has_range && state->second.IsDragging && GetCapture() == scrollbar) {
                SDLScrollBarMetrics metrics;
                SDL_Get_ScrollBar_Metrics(scrollbar, state->second.Range, metrics);
                SDL_Set_ScrollBar_Pos_From_Point(scrollbar, state->second.Range, metrics, GET_Y_LPARAM(lparam));
                return true;
            }
            return false;
        }

        case WM_LBUTTONUP:
        {
            if (has_range && state->second.IsDragging) {
                SDLScrollBarMetrics metrics;
                SDL_Get_ScrollBar_Metrics(scrollbar, state->second.Range, metrics);
                SDL_Set_ScrollBar_Pos_From_Point(scrollbar, state->second.Range, metrics, GET_Y_LPARAM(lparam));
                state->second.IsDragging = false;

                if (GetCapture() == scrollbar) {
                    ReleaseCapture();
                }
                return true;
            }
            return false;
        }

        case WM_TIMER:
        {
            /**
             *  The game's procedure runs the arrow auto-repeat from a timer
             *  it starts on (non-grip) button-down, while it holds capture.
             */
            if (!has_range || state->second.IsDragging || GetCapture() != scrollbar) {
                return false;
            }

            SDLScrollBarMetrics metrics;
            SDL_Get_ScrollBar_Metrics(scrollbar, state->second.Range, metrics);

            const POINT point = SDL_Logical_Cursor_Point_For_Window(scrollbar);
            if (point.x > metrics.ClientRect.left) {
                const int position = static_cast<int>(SDL_Call_Stored_Child_Window_Procedure(scrollbar, SBM_GETPOS, 0, 0));
                if (point.y < 22) {
                    SDL_Apply_ScrollBar_Position(scrollbar, state->second.Range, position - 1);
                } else if (point.y > metrics.ClientRect.bottom - 22) {
                    SDL_Apply_ScrollBar_Position(scrollbar, state->second.Range, position + 1);
                }
            }

            SetTimer(scrollbar, 0, 0x19, nullptr);
            return true;
        }

        case WM_CAPTURECHANGED:
        {
            if (state != SDLScrollBarDragState.end()) {
                state->second.IsDragging = false;
            }
            return false;
        }

        case WM_NCDESTROY:
            SDLScrollBarDragState.erase(scrollbar);
            return false;

        default:
            return false;
        }
    }

    /**
     *  Finds the deepest child window that would contain the logical point if
     *  the Win32 window tree existed at the game's render resolution.
     *
     *  @author: Rampastring
     */
    HWND SDL_Window_From_Logical_Point(POINT logical_point)
    {
        HWND capture = GetCapture();
        if (capture != nullptr && (capture == MainWindow || (IsChild(MainWindow, capture) && SDL_Should_Use_Captured_Child_Window(capture)))) {
            return capture;
        }

        HWND current = MainWindow;

        for (;;) {
            POINT child_point = logical_point;
            if (current != MainWindow) {
                MapWindowPoints(MainWindow, current, &child_point, 1);
            }

            HWND child = ChildWindowFromPointEx(current, child_point, CWP_SKIPINVISIBLE | CWP_SKIPDISABLED);
            if (child == nullptr || child == current) {
                return current;
            }

            current = child;
        }
    }

    /**
     *  Installs our coordinate-transforming proxy on native child controls
     *  that can receive captured mouse drags directly from Windows.
     *
     *  @author: Rampastring
     */
    void SDL_Subclass_Child_Window(HWND window)
    {
        if (window == nullptr || window == MainWindow || !IsChild(MainWindow, window)) {
            return;
        }

        if (!SDL_Should_Subclass_Child_Window(window)) {
            return;
        }

        if (SDLChildWindowProcedures.find(window) != SDLChildWindowProcedures.end()) {
            return;
        }

        WNDPROC previous_proc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(window, GWLP_WNDPROC));
        if (previous_proc == nullptr || previous_proc == SDL_Child_Windows_Procedure) {
            return;
        }

        WNDPROC replaced_proc = reinterpret_cast<WNDPROC>(SetWindowLongPtr(window, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(SDL_Child_Windows_Procedure)));
        if (replaced_proc != nullptr) {
            SDLChildWindowProcedures[window] = replaced_proc;
        }
    }

    /**
     *  Subclasses combo drop-down children after the game creates them and
     *  gives them mouse capture.
     *
     *  @author: Rampastring
     */
    BOOL CALLBACK SDL_Subclass_Combo_Dropdown_Window(HWND window, LPARAM)
    {
        if (SDL_Is_Combo_Dropdown_Window(window)) {
            SDL_Subclass_Child_Window(window);
        }

        return TRUE;
    }

    /**
     *  Installs the coordinate-transforming proxy on combo drop-down children
     *  below a dialog window.
     *
     *  @author: Rampastring
     */
    void SDL_Subclass_Combo_Dropdown_Windows_For_Parent(HWND parent)
    {
        if (parent == nullptr) {
            return;
        }

        EnumChildWindows(parent, SDL_Subclass_Combo_Dropdown_Window, 0);
    }

    /**
     *  Presents combo drop-down hover changes immediately after the custom
     *  control invalidates itself from WM_MOUSEMOVE.
     *
     *  @author: Rampastring
     */
    void SDL_Update_Combo_Dropdown_Window(HWND window, UINT message)
    {
        if (!SDL_Is_Combo_Dropdown_Window(window)) {
            return;
        }

        if (message == WM_MOUSEMOVE) {
            UpdateWindow(window);
        } else if (message == WM_PAINT) {
            SDL_Update_Screen(VisibleSurface);
        }
    }

    /**
     *  Calls a child window's original procedure while suppressing a nested
     *  routing pass if the original procedure is another Vinifera proxy.
     *
     *  @author: Rampastring
     */
    LRESULT SDL_Call_Child_Window_Procedure(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
    {
        LRESULT result = SDL_Call_Stored_Child_Window_Procedure(window, message, wparam, lparam);

        if (message == CB_SHOWDROPDOWN && wparam != 0) {
            SDL_Subclass_Combo_Dropdown_Windows_For_Parent(GetParent(window));
        }

        SDL_Update_Combo_Dropdown_Window(window, message);

        if (message == WM_NCDESTROY) {
            SDLChildWindowProcedures.erase(window);
        }

        return result;
    }

    /**
     *  Window procedure for native child controls that need logical mouse
     *  coordinates during captured drags.
     *
     *  @author: Rampastring
     */
    LRESULT CALLBACK SDL_Child_Windows_Procedure(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam)
    {
        LPARAM translated_lparam = lparam;
        if (SDL_Redirect_Mouse_Message(hwnd, message, wparam, lparam, &translated_lparam)) {
            return 0;
        }

        if (SDL_Handle_Trackbar_Mouse_Message(hwnd, message, translated_lparam)) {
            return 0;
        }

        if (SDL_Handle_ScrollBar_Mouse_Message(hwnd, message, translated_lparam)) {
            return 0;
        }

        return SDL_Call_Child_Window_Procedure(hwnd, message, wparam, translated_lparam);
    }

    /**
     *  Converts a logical MainWindow client point into the lParam coordinate
     *  space expected by the target window for the given mouse message.
     *
     *  @author: Rampastring
     */
    LPARAM SDL_Logical_Mouse_LParam_For_Target(HWND target, UINT message, POINT logical_point)
    {
        POINT target_point = logical_point;

        if (SDL_Mouse_Message_Uses_Screen_Coordinates(message)) {
            ClientToScreen(MainWindow, &target_point);
        } else if (target != MainWindow) {
            MapWindowPoints(MainWindow, target, &target_point, 1);
        }

        return SDL_Make_Mouse_LParam(target_point.x, target_point.y);
    }

    /**
     *  Applies the SDL renderer driver hint for the selected backend.
     *
     *  @author: ZivDero
     */
    void SDL_Apply_Renderer_Driver_Hint()
    {
        const char* requested_driver_name = OptionsClassExtension::Get_Renderer_Driver_SDL_Name(OptionsExtension->RendererDriver);
        const char* requested_driver_config_name = OptionsClassExtension::Get_Renderer_Driver_Config_Name(OptionsExtension->RendererDriver);

        DEBUG_INFO("Requested renderer driver: {}\n", requested_driver_config_name);

        if (requested_driver_name != nullptr) {
            SDL_SetHint(SDL_HINT_RENDER_DRIVER, requested_driver_name);
        } else {
            SDL_ResetHint(SDL_HINT_RENDER_DRIVER);
        }
    }

    /**
     *  Computes the tactical display rectangle for the given visible area.
     *
     *  @author: ZivDero
     */
    Rect SDL_Get_Display_View_Rect(const Rect& visible_rect)
    {
        Rect temp = visible_rect;
        temp.X = Options.SidebarSide || Debug_Map ? 0 : 168;
        temp.Y = 16;
        temp.Width -= 168;
        temp.Height -= 16;
        return temp;
    }

    /**
     *  Recalculates the SDL mouse cursor image if a cursor exists.
     *
     *  @author: ZivDero
     */
    void SDL_Recalc_Mouse_Cursor_Image()
    {
        if (MouseCursor != nullptr) {
            static_cast<SDLMouseClass*>(MouseCursor)->Recalc_Cursor_Image();
        }
    }

    /**
     *  Rebuilds the software surfaces and UI state for the current display mode.
     *
     *  @author: ZivDero
     */
    void SDL_Rebuild_Display_State(const Rect& visible_rect)
    {
        Rect temp = SDL_Get_Display_View_Rect(visible_rect);

        VisibleRect = visible_rect;
        VideoWidth = visible_rect.Width;
        VideoHeight = visible_rect.Height;

        VisibleSurface = SDLSurface::Create_Primary();

        Allocate_Surfaces(
            VisibleRect,
            Rect(0, 0, temp.Width, VisibleRect.Height),
            Rect(0, 0, temp.Width, VisibleRect.Height),
            Rect(0, 0, 168, VisibleRect.Height));
        LogicalSurface = HiddenSurface;

        Hide_Mouse();
        SDL_Recalc_Mouse_Cursor_Image();
        Show_Mouse();

        Map.Set_View_Dimensions(temp);
        Map.Init_IO();
        Map.Activate(1);
        Map.Shift_Sidebar();
        Map.Flag_To_Redraw(GS_REDRAW_ALL);
        Show_Mouse();
    }
}


/**
 *  Retargets mouse messages from the scaled outer SDL window to the Win32
 *  window/control that owns the corresponding logical game-space point.
 *
 *  @author: Rampastring
 */
bool SDL_Redirect_Mouse_Message(HWND window, UINT message, WPARAM wparam, LPARAM lparam, LPARAM* translated_lparam)
{
    if (translated_lparam != nullptr) {
        *translated_lparam = lparam;
    }

    if (!SDL_Is_Mouse_Coordinate_Message(message) || MainWindow == nullptr || SDLCallingTranslatedMouseProc) {
        return false;
    }

    if (SDLForwardingMouseMessage) {
        return false;
    }

    POINT physical_point = SDL_Mouse_Message_Point_To_Main_Window(window, message, lparam);
    POINT logical_point = SDL_Physical_Point_To_Logical_Game_Point(physical_point);
    HWND target = SDL_Window_From_Logical_Point(logical_point);
    SDL_Subclass_Child_Window(target);
    LPARAM target_lparam = SDL_Logical_Mouse_LParam_For_Target(target, message, logical_point);

    if (target == window) {
        if (translated_lparam != nullptr) {
            *translated_lparam = target_lparam;
        }
        return false;
    }

    SDLForwardingMouseMessage = true;
    SendMessage(target, message, wparam, target_lparam);
    SDLForwardingMouseMessage = false;

    return true;
}


/**
 *  Installs the SDL child window proxy on any active combo box drop-downs
 *  below the given dialog.
 *
 *  @author: Rampastring
 */
void SDL_Subclass_Combo_Dropdown_Windows(HWND parent)
{
    SDL_Subclass_Combo_Dropdown_Windows_For_Parent(parent);
}


/**
 *  Records custom trackbar and scrollbar state from messages that the game
 *  sends through its owner-drawn control procedure.
 *
 *  @author: Rampastring, ZivDero
 */
void SDL_Record_Control_State_Message(HWND window, UINT message, WPARAM, LPARAM lparam)
{
    if (SDL_Is_Trackbar_Window(window)) {

        switch (message) {
        case TBM_SETRANGE:
        {
            SDLTrackbarDragInfo& state = SDLTrackbarDragState[window];
            state.Minimum = static_cast<unsigned short>(LOWORD(lparam));
            state.Maximum = static_cast<unsigned short>(HIWORD(lparam));
            state.HasRange = state.Maximum > state.Minimum;
            break;
        }

        case OD_SETTRACKSTEP:
            SDLTrackbarDragState[window].Step = static_cast<int>(lparam);
            break;

        case OD_TRACKNUMBERS:
            SDLTrackbarDragState[window].ShowNumbers = lparam != 0;
            break;

        case WM_NCDESTROY:
            SDLTrackbarDragState.erase(window);
            break;

        default:
            break;
        }

        return;
    }

    if (SDL_Is_ScrollBar_Window(window)) {

        switch (message) {
        case SBM_SETRANGE:
        {
            SDLScrollBarDragInfo& state = SDLScrollBarDragState[window];
            state.Range = static_cast<int>(lparam);
            state.HasRange = state.Range > 0;
            break;
        }

        case SBM_SETSCROLLINFO:
        {
            const SCROLLINFO* info = reinterpret_cast<const SCROLLINFO*>(lparam);
            if (info != nullptr) {
                SDLScrollBarDragInfo& state = SDLScrollBarDragState[window];
                state.Range = info->nMax;
                state.HasRange = state.Range > 0;
            }
            break;
        }

        case WM_NCDESTROY:
            SDLScrollBarDragState.erase(window);
            break;

        default:
            break;
        }
    }
}


/**
 *  Allocates all game surfaces with the given sizes.
 *
 *  @author: ZivDero, tomsons26
 */
bool SDL_Allocate_Surfaces(const Rect& hidden_rect, const Rect& composite_rect, const Rect& tile_rect, const Rect& sidebar_rect, bool hidden_first)
{
    DEBUG_INFO("Allocating new surfaces\n");

    if (AlternateSurface != nullptr) {
        DEBUG_INFO("Deleting AlternateSurface\n");
        delete AlternateSurface;
        AlternateSurface = nullptr;
    }

    if (HiddenSurface != nullptr) {
        DEBUG_INFO("Deleting HiddenSurface\n");
        delete HiddenSurface;
        HiddenSurface = nullptr;
    }

    if (CompositeSurface != nullptr) {
        DEBUG_INFO("Deleting CompositeSurface\n");
        delete CompositeSurface;
        CompositeSurface = nullptr;
    }

    if (TileSurface != nullptr) {
        DEBUG_INFO("Deleting TileSurface\n");
        delete TileSurface;
        TileSurface = nullptr;
    }

    if (SidebarSurface != nullptr) {
        DEBUG_INFO("Deleting SidebarSurface\n");
        delete SidebarSurface;
        SidebarSurface = nullptr;
    }

    if (hidden_first && hidden_rect.Is_Valid()) {
        HiddenSurface = new SDLSurface(hidden_rect.Width, hidden_rect.Height);
        HiddenSurface->Fill(0);
        DEBUG_INFO("HiddenSurface ({}x{})\n", hidden_rect.Width, hidden_rect.Height);
    }

    if (composite_rect.Is_Valid()) {
        CompositeSurface = new SDLSurface(composite_rect.Width, composite_rect.Height);
        CompositeSurface->Fill(0);
        DEBUG_INFO("CompositeSurface ({}x{})\n", composite_rect.Width, composite_rect.Height);
    }

    if (tile_rect.Is_Valid()) {
        TileSurface = new SDLSurface(tile_rect.Width, tile_rect.Height);
        TileSurface->Fill(0);
        DEBUG_INFO("TileSurface ({}x{})\n", tile_rect.Width, tile_rect.Height);
    }

    if (sidebar_rect.Is_Valid()) {
        SidebarSurface = new SDLSurface(sidebar_rect.Width, sidebar_rect.Height);
        SidebarSurface->Fill(0);
        DEBUG_INFO("SidebarSurface ({}x{})\n", sidebar_rect.Width, sidebar_rect.Height);
    }

    if (!hidden_first && hidden_rect.Is_Valid()) {
        HiddenSurface = new SDLSurface(hidden_rect.Width, hidden_rect.Height);
        HiddenSurface->Fill(0);
        DEBUG_INFO("HiddenSurface ({}x{})\n", hidden_rect.Width, hidden_rect.Height);
    }

    if (hidden_rect.Is_Valid()) {
        AlternateSurface = new SDLSurface(hidden_rect.Width, hidden_rect.Height);
        AlternateSurface->Fill(0);
        DEBUG_INFO("AlternateSurface ({}x{})\n", hidden_rect.Width, hidden_rect.Height);
    }

    return true;
}


/**
 *  Initializes the SDL presentation layer.
 *
 *  @author: ZivDero
 */
bool SDL_Set_Video_Mode(HWND, int width, int height, int bits_per_pixel)
{
    if (SDLWindow == nullptr) {
        DEBUG_ERROR("SDLWindow is null!\n");
        return false;
    }
    
    /**
     *  We need to delete the existing presentation layer first.
     */
    SDL_Reset_Video_Mode();
    
    /**
     *  Query the window's pixel format.
     */
    SDL_PixelFormat pixel_format = SDL_GetWindowPixelFormat(SDLWindow);
    if (pixel_format == SDL_PIXELFORMAT_UNKNOWN || SDL_BITSPERPIXEL(pixel_format) < 16) {
        DEBUG_ERROR("SDL3 window pixel format unsupported: {} ({} bpp)\n", SDL_GetPixelFormatName(pixel_format), SDL_BITSPERPIXEL(pixel_format));
        return false;
    }

    DEBUG_INFO("Pixel format: {} ({} bpp)\n", SDL_GetPixelFormatName(pixel_format), SDL_BITSPERPIXEL(pixel_format));

    /**
     *  Apply the renderer backend selection before creating the renderer.
     */
    SDL_Apply_Renderer_Driver_Hint();

    /**
     *  Create the renderer for window.
     */
    SDLWindowRenderer = SDL_CreateRenderer(SDLWindow, nullptr);
    if (SDLWindowRenderer == nullptr) {
        DEBUG_ERROR("SDLWindowRenderer could not be created! SDL Error: {}\n", SDL_GetError());
        return false;
    }
    DEBUG_INFO("SDLWindowRenderer created.\n");

    const char* driver_name = SDL_GetRendererName(SDLWindowRenderer);
    DEBUG_INFO("Renderer driver: {}\n", driver_name != nullptr ? driver_name : "<unknown>");

    /**
     *  Toggle VSync.
     */
    SDL_SetRenderVSync(SDLWindowRenderer, OptionsExtension->IsVSync ? 1 : 0);

    /**
     *  Set the scaling mode if specified.
     */
    if (OptionsExtension->ScaleMode != SDL_SCALEMODE_INVALID) {
        SDL_SetDefaultTextureScaleMode(SDLWindowRenderer, OptionsExtension->ScaleMode);
    }

    /**
     *  Create the window texture.
     */
    SDLWindowTexture = SDL_CreateTexture(SDLWindowRenderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (SDLWindowTexture == nullptr) {
        DEBUG_ERROR("SDLWindowTexture could not be created! SDL_Error: {}\n", SDL_GetError());
        return false;
    }
    DEBUG_INFO("SDLWindowTexture created.\n");

    /**
     *  Save video mode information.
     */
    VideoWidth = width;
    VideoHeight = height;
    VideoBitsPerPixel = bits_per_pixel;

    if (!ViniferaImGui::Initialize(MainWindow, SDLWindowRenderer)) {
        DEBUG_ERROR("Vinifera ImGui could not be initialized.\n");
    }

    return true;
}


/**
 *  Resets video mode and deletes the SDL presentation layer.
 *
 *  @author: ZivDero
 */
void SDL_Reset_Video_Mode()
{
    ViniferaImGui::Shutdown();

    /**
     *  Destroy the renderer.
     */
    SDL_DestroyRenderer(SDLWindowRenderer);
    SDLWindowRenderer = nullptr;

    /**
     *  Deallocate the texture.
     */
    SDL_DestroyTexture(SDLWindowTexture);
    SDLWindowTexture = nullptr;

    /**
     *  Clear video mode information.
     */
    VideoWidth = 0;
    VideoHeight = 0;
    VideoBitsPerPixel = 0;
}


/**
 *  Pointer to the window procedure set by SDL.
 */
static WNDPROC SDL_Proc = nullptr;

/**
 *  Replacement window procedure for the main window.
 *
 *  @author: tomsons26, ZivDero
 */
LRESULT CALLBACK SDL_Windows_Procedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    const LPARAM original_lParam = lParam;

    if (ViniferaImGui::Process_Window_Message(hwnd, message, wParam, original_lParam)) {
        return 0;
    }

    LPARAM translated_lParam = lParam;
    if (SDL_Redirect_Mouse_Message(hwnd, message, wParam, lParam, &translated_lParam)) {
        return 0;
    }
    lParam = translated_lParam;

    /*
    **  Pass on any messages intended for the winsock message handler.
    */
    if (PacketTransport) {
        if (message == (UINT)PacketTransport->Protocol_Event_Message()) {
            if (PacketTransport->Message_Handler(hwnd, message, wParam, lParam)) {
                return DefWindowProc(hwnd, message, wParam, lParam);
            } else {
                return 0;
            }
        }
    }

    Map.Message_Handler(hwnd, message, wParam, lParam);

    switch (message) {

        /*
        **  Refresh the window.
        */
    case WM_PAINT:
        if (Vinifera_ModernMoviePlaying) {
            SDL_Movie_Repaint();
        } else if (MouseCursor != nullptr && VisibleSurface != nullptr && HiddenSurface != nullptr && CompositeSurface != nullptr) {
            if (TacticalActive == true) {
                Update_Visible_Surface(MouseCursor->Is_Captured(), CompositeSurface);
                Map.Blit_Sidebar(true);
            } else if (Movie_Is_Playing() == true) {
                Movie_Update_Visible_Surface();
            } else {
                Update_Visible_Surface(MouseCursor->Is_Captured(), HiddenSurface);
            }
        }

        /*
        **  Tell SDL that the window needs refreshing to simulate what it does itself.
        */
        SDL_Event event;
        event.type = SDL_EVENT_WINDOW_EXPOSED;
        event.window.windowID = SDL_GetWindowID(SDLWindow);
        event.window.data1 = 0;
        event.window.data2 = 0;
        SDL_PushEvent(&event);

        /*
        **  But don't let SDL handle this event, or it will break Win32 controls' drawing.
        */
        return DefWindowProc(hwnd, message, wParam, lParam);

    case WM_CLOSE:
        break;

        /*
        **  Windoze message says we have to shut down. Try and do it cleanly.
        */
    case WM_DESTROY:
        if (ToolTips != nullptr) {
            delete ToolTips;
            ToolTips = nullptr;
        }
        MainWindow = nullptr;

        /*
        **  If we are shutting down gracefully than flag that the message loop has finished.
        **  If this is a forced shutdown (ReadyToQuit == 0) then try and close down everything
        **  before we exit.
        */
        switch (ReadyToQuit) {
        default:
        case 1:
            ReadyToQuit = 2;
            break;

        case 0:
            break;
        }
        return 0;

    case WM_ACTIVATEAPP:
        if (hwnd == MainWindow && GameInFocus != (wParam != 0)) {
            GameInFocus = wParam != 0;
            if (GameInFocus) {
                Focus_Restore();

                /*
                **  Force all child controls to redraw when regaining focus.
                */
                EnumChildWindows(
                    hwnd,
                    [](HWND child, LPARAM) -> BOOL {
                        RedrawWindow(child, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
                        return TRUE;
                    },
                    0);

                RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
            } else {
                Focus_Loss();
            }
        }
        return 0;

    case WM_RBUTTONUP:

        /*
        **  Set some kind of scolling flag, perhaps "CanScroll".
        */
        Map.field_1D0C = false;
        break;

    case WM_MOVING:
        On_WM_MOVING(hwnd, wParam, lParam);
        return CallWindowProc(SDL_Proc, hwnd, message, wParam, lParam);

    case WM_MOUSEWHEEL:
        if (!_MouseWheel) {
            _MouseWheel = true;

            /**
             *  If we are not currently playing a scenario, no need to execute this command.
             */
            if (TacticalActive && ScenarioActive) {
                if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) {
                    Do_Command("SidebarDown");
                } else {
                    Do_Command("SidebarUp");
                }
            }
            _MouseWheel = false;
        }
        break;

    case WM_SYSCOMMAND:
        switch (wParam) {

        case SC_CLOSE:
            /*
            **  TS Client users are used to Alt+F4 aborting the game, which in turn closes the game
            **  because there is no main menu in the TS Client.
            */
            if (GameActive) {
                Queue_Exit();
            }

            /*
            **  Windows sent us a close message. Probably in response to Alt-F4. Ignore it by
            **  pretending to handle the message and returning true;
            */
            return 0;

        case SC_SCREENSAVE:

            /*
            **  Windoze is about to start the screen saver. If we just return without passing
            **  this message to DefWindowProc then the screen saver will not be allowed to start.
            */
            return 0;

        default:
            break;
        }
        break;

    default:
        break;
    }

    /*
    **  Pass this message through to the keyboard handler.
    */
    Keyboard->Message_Handler(hwnd, message, wParam, lParam);

    return CallWindowProc(SDL_Proc, hwnd, message, wParam, lParam);
}


/**
 *  Creates the main game window.
 *
 *  @author: ZivDero, CCHyper
 */
bool SDL_Create_Main_Window(HINSTANCE instance, int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        DEBUG_ERROR("SDL_Init failed! SDL_Error: {}\n", SDL_GetError());
        return false;
    }

    SDL_PropertiesID props = SDL_CreateProperties();

    if (WindowedMode) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
    } else {
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);
    }

    DWORD dwPid = GetProcessId(GetCurrentProcess());
    if (!dwPid) {
        DEBUG_ERROR("Create_Main_Window() - Failed to get the process id!\n");
        return false;
    }

    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, Vinifera_Get_Window_Title(dwPid));

    /**
     *  OpenGL and Vulkan renderers need a matching graphics-capable window from the start on Windows.
     */
    if (OptionsExtension->RendererDriver == OptionsClassExtension::RENDERER_DRIVER_OPENGL) {
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_OPENGL_BOOLEAN, true);
    } else if (OptionsExtension->RendererDriver == OptionsClassExtension::RENDERER_DRIVER_VULKAN) {
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_VULKAN_BOOLEAN, true);
    }

    /**
     *  Create the window.
     */
    SDLWindow = SDL_CreateWindowWithProperties(props);
    if (SDLWindow == nullptr) {
        DEBUG_ERROR("SDLWindow could not be created! SDL_Error: {}\n", SDL_GetError());
        return false;
    }
    DEBUG_INFO("SDLWindow created.\n");
    
    /**
     *  Record the size that the window has been created at.
     */
    SDL_GetWindowSize(SDLWindow, &SDLWindowWidth, &SDLWindowHeight);
    DEBUG_INFO("SDLWindow size: {} X {}.\n", SDLWindowWidth, SDLWindowHeight);

    /**
     *  Save the window handle for the game to use.
     */
    props = SDL_GetWindowProperties(SDLWindow);
    MainWindow = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

    /**
     *  We draw Win32 child windows as part of the main window, so we need to disable clipping.
     *  Otherwise, we will see black boxes where child windows are.
     */
    LONG_PTR style = GetWindowLongPtr(MainWindow, GWL_STYLE);
    style &= ~(WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    SetWindowLongPtr(MainWindow, GWL_STYLE, style);

    /**
     *  Set the window to use our window procedure, save the one SDL set.
     */
    SDL_Proc = (WNDPROC)SetWindowLongPtr(MainWindow, GWLP_WNDPROC, (LONG_PTR)SDL_Windows_Procedure);

    /**
     *  Explicitly set input focus to the window.
     */
    SDL_RaiseWindow(SDLWindow);
    GameInFocus = true; // The SDL window needs this initially otherwise we need to alt-tab to gain focus.

    /**
     *  This used to happen on WM_CREATE but our proc is no longer the proc that's used when
     *  the window is created, so it never happens.
     */
    if (!ToolTips) {
        ToolTips = new CCToolTip(MainWindow);
        if (ToolTips) {
            ToolTips->Set_Timer_Delay(500);
        }
    }

    return true;
}


/**
 *  Destroys the main game window.
 *
 *  @author: CCHyper
 */
void SDL_Destroy_Main_Window()
{
    /**
     *  Destroy window.
     */
    SDL_DestroyWindow(SDLWindow);
    SDLWindow = nullptr;
}


/**
 *  Update the screen with any rendering performed since the previous call.
 *
 *  @author: ZivDero, CCHyper, tomsons26
 */
bool SDL_Update_Screen(Surface* surface)
{
    SDL_RenderClear(SDLWindowRenderer);

    /**
     *  Blit game's surface to SDL's window surface.
     */
    if (surface) {

        /**
         *  First, update the texture with the pixels from the game's surface.
         */
        if (void* pixels = surface->Lock()) {
#if 0
            void* tex_pixels;
            int tex_pitch;
            SDL_LockTexture(SDLWindowTexture, nullptr, &tex_pixels, &tex_pitch);
            memcpy(tex_pixels, pixels, surface->Get_Height() * surface->Stride());
            SDL_UnlockTexture(SDLWindowTexture);
#else
            SDL_UpdateTexture(SDLWindowTexture, nullptr, pixels, surface->Stride());
#endif
            surface->Unlock();
        }

        /**
         *  Then, copy the texture to the renderer.
         */
        SDL_RenderTexture(SDLWindowRenderer, SDLWindowTexture, nullptr, nullptr);
    }

    /**
     *  Present the image to the window.
     */
    ViniferaImGui::Render();

    SDL_RenderPresent(SDLWindowRenderer);

    return true;
}


/**
 *  Changes the display mode to the given resolution.
 *
 *  @author: ZivDero, tomsons26
 */
bool SDL_Change_Display_Mode(int width, int height)
{
    DEBUG_INFO("About to set video mode\n");

    Rect old_visible_rect = VisibleRect;
    if (!old_visible_rect.Is_Valid() && VideoWidth > 0 && VideoHeight > 0) {
        old_visible_rect = Rect(0, 0, VideoWidth, VideoHeight);
    }

    const int old_video_width = VideoWidth;
    const int old_video_height = VideoHeight;
    const int old_video_bits_per_pixel = VideoBitsPerPixel > 0 ? VideoBitsPerPixel : 16;

    int old_window_x = 0;
    int old_window_y = 0;
    int old_window_width = SDLWindowWidth;
    int old_window_height = SDLWindowHeight;

    Hide_Mouse();

    /**
     *  Delete the old primary surface.
     */
    if (VisibleSurface != nullptr) {
        DEBUG_INFO("Deleting VisibleSurface\n");
        delete VisibleSurface;
        VisibleSurface = nullptr;
    }

    /**
     *  If the window size isn't set manually, resize the window to refect the new resolution.
     */
    if (WindowedMode) {
        int window_width = width;
        int window_height = height;

        /**
         *  If the window size isn't set manually, resize the window to refect the new resolution.
         */
        if (OptionsExtension->WindowWidth > 0 && OptionsExtension->WindowHeight > 0) {
            window_width = OptionsExtension->WindowWidth;
            window_height = OptionsExtension->WindowHeight;
        }

        /**
         *  Get the current window size and position.
         */
        SDL_GetWindowPosition(SDLWindow, &old_window_x, &old_window_y);
        SDL_GetWindowSize(SDLWindow, &old_window_width, &old_window_height);

        /**
         *  Compute the current center point.
         */
        int center_x = old_window_x + old_window_width / 2;
        int center_y = old_window_y + old_window_height / 2;

        /**
         *  Compute new top-left corner so that the center stays the same.
         */
        int new_x = center_x - window_width / 2;
        int new_y = center_y - window_height / 2;

        /**
         *  Apply and save the new position and size.
         */
        SDL_SetWindowPosition(SDLWindow, new_x, new_y);
        SDL_SetWindowSize(SDLWindow, window_width, window_height);

        SDLWindowWidth = window_width;
        SDLWindowHeight = window_height;
        DEBUG_INFO("SDLWindow size: {} X {}.\n", SDLWindowWidth, SDLWindowHeight);
    }

    /**
     *  Recreate all the SDL intermediates (texture, renderer).
     */
    if (!Set_Video_Mode(MainWindow, width, height, 16)) {
        DEBUG_ERROR("Set_Video_Mode failed.\n");

        if (WindowedMode) {
            SDL_SetWindowPosition(SDLWindow, old_window_x, old_window_y);
            SDL_SetWindowSize(SDLWindow, old_window_width, old_window_height);
            SDLWindowWidth = old_window_width;
            SDLWindowHeight = old_window_height;
            DEBUG_INFO("SDLWindow size restored: {} X {}.\n", SDLWindowWidth, SDLWindowHeight);
        }

        if (old_visible_rect.Is_Valid() && old_video_width > 0 && old_video_height > 0) {
            DEBUG_WARNING("Restoring previous display mode.\n");

            if (!Set_Video_Mode(MainWindow, old_video_width, old_video_height, old_video_bits_per_pixel)) {
                DEBUG_ERROR("Failed to restore previous video mode.\n");
                Show_Mouse();
                return false;
            }

            SDL_Rebuild_Display_State(old_visible_rect);
        } else {
            DEBUG_ERROR("Previous display mode is invalid and cannot be restored.\n");
        }

        Show_Mouse();
        return false;
    }

    /**
     *  Set the new surface resolution and reallocate the game surfaces.
     */
    SDL_Rebuild_Display_State(Rect(0, 0, width, height));
    DEBUG_INFO("VisibleRect: {}x{}\n", width, height);

    DEBUG_INFO("Mode change complete.\n");

    return true;
}
