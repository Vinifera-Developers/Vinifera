/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the SDL system.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "gscreen.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "movie.h"
#include "sdl_functions.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "vinifera_globals.h"
#include "vqa.h"
#include "winuser.h"
#include "xmouse.h"

#include <algorithm>
#include <dsound.h>


/**
 *  Update the window after updating the visible surface.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004B9A42, _Update_Visible_Surface_SDL_Update_Window_Patch, 5)
{
    SDL_Update_Screen(VisibleSurface);

    return 0;
}


/**
 *  Update the window after updating the visible surface when drawing a movie frame.
 * 
 *  @author: tomsons26, ZivDero
 */
void SDL_Movie_Blit_To_Screen()
{
    if (!VQA_Get_Option(OPTION_NO_STRETCH)) {
        VisibleSurface->Blit_From(CurrentVQ->StretchRect, *CurrentVQ->DrawSurface, CurrentVQ->InitialRect);
        SDL_Update_Screen(VisibleSurface);
    }
}

void SDL_Movie_Update_Visible_Surface()
{
    if (CurrentVQ != nullptr) {
        AlternateSurface->Fill(0);
        Update_Visible_Surface(false, AlternateSurface);
        SDL_Movie_Blit_To_Screen();
    }
}


/**
 *  Update the window after updating the visible surface when drawing in MSEngine.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0057111C, _MSEngine_BlitAll_SDL_Update_Window_Patch, 7)
{
    SDL_Update_Screen(VisibleSurface);

    return 0;
}


/**
 *  Update the window after updating the visible surface when drawing in MSEngine.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005711F8, _MSEngine_BlitRect_SDL_Update_Window_Patch, 6)
{
    SDL_Update_Screen(VisibleSurface);

    return 0;
}


/**
 *  Update the window after updating the visible surface when drawing in ScoreClass.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005E6468, _ScoreClass_Call_Back_Delay_SDL_Update_Window_Patch, 6)
{
    SDL_Update_Screen(VisibleSurface);

    return 0;
}


/**
 *  Dummy replacement for ClientToScreen. SDL uses client coordinates directly,
 *  so most of these calls are now not necessary.
 *
 *  @author: ZivDero
 */
BOOL WINAPI Fake_ClientToScreen(HWND hwnd, LPPOINT point)
{
    return TRUE;
}


/**
 *  CtrlProc is the main window procedure for all Tiberian Sun window controls.
 *  We substitute it with a proxy so that after it is done, can update the screen.
 *
 *  @author: ZivDero
 */
LRESULT CALLBACK CtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
DEFINE_IMPLEMENTATION(LRESULT CALLBACK CtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam), 0x00592340);

LRESULT CALLBACK CtrlProcProxy(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    SDL_Record_Control_State_Message(window, message, wparam, lparam);

    LPARAM translated_lparam = lparam;
    if (SDL_Redirect_Mouse_Message(window, message, wparam, lparam, &translated_lparam)) {
        return 0;
    }

    LRESULT result = CtrlProc(window, message, wparam, translated_lparam);
    if (message == CB_SHOWDROPDOWN && wparam != 0) {
        SDL_Subclass_Combo_Dropdown_Windows(GetParent(window));
    }

    if (message == WM_PAINT) {
        SDL_Update_Screen(VisibleSurface);
    }
    return result;
}


/**
 *  Windows have a sliding opening animation that happens within a single WM_PAINT call,
 *  so we need to hook multiple locations within CtrlProc to ensure the screen is updated.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00593F8D, _CtrlProc_SDL_Update_Screen_Patch, 6)
{
    SDL_Update_Screen(VisibleSurface);
    return 0;
}
DEFINE_HOOK_AGAIN(0x00594101, _CtrlProc_SDL_Update_Screen_Patch, 6);
DEFINE_HOOK_AGAIN(0x0059437C, _CtrlProc_SDL_Update_Screen_Patch, 6);
DEFINE_HOOK_AGAIN(0x0059449F, _CtrlProc_SDL_Update_Screen_Patch, 6);


/**
 *  The kick player dialog needs separate hooks to update the screen after drawing.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005B3F72, _Kick_Player_Dialog_SDL_Update_Screen_Patch, 6)
{
    SDL_Update_Screen(VisibleSurface);
    return 0;
}
DEFINE_HOOK_AGAIN(0x005B3F81, _Kick_Player_Dialog_SDL_Update_Screen_Patch, 5);
DEFINE_HOOK_AGAIN(0x005B3FA1, _Kick_Player_Dialog_SDL_Update_Screen_Patch, 6);

/**
 *  This function moves a dialog window to a specified position.
 *  It needs to use the real ClientToScreen so that dialogs are where they should be.
 *
 *  @author: tomsons26, ZivDero
 */
int _ODMoveDialog(HWND window, int x, int y)
{
    int xpos;
    int ypos;

    RECT rect1;
    rect1.left = 0;
    rect1.top = 0;
    rect1.right = VideoWidth;
    rect1.bottom = VideoHeight;

    ClientToScreen(MainWindow, (LPPOINT)&rect1);
    ClientToScreen(MainWindow, (LPPOINT)&rect1.right);

    RECT rect2;
    GetWindowRect(window, &rect2);

    rect2.right -= rect2.left;
    rect2.bottom -= rect2.top;

    if (x == -1) {
        xpos = rect2.left - rect1.left;
    } else {
        xpos = x;
    }
    rect2.left = xpos;

    if (y == -1) {
        ypos = rect2.top - rect1.top;
    } else {
        ypos = y;
    }
    rect2.top = ypos;

    return MoveWindow(window, rect2.left, rect2.top, rect2.right, rect2.bottom, FALSE);
}


/**
 *  This function centers a window within a parent window.
 *  It needs to use the real ClientToScreen so that dialogs are where they should be.
 *
 *  @author: tomsons26, ZivDero
 */
void _Center_Window_Within_Window(HWND window, HWND parent)
{
    RECT rcl;
    GetClientRect(parent, &rcl);

    if (parent == MainWindow) {
        rcl.right = VideoWidth;
        rcl.bottom = VideoHeight;
    }

    ClientToScreen(parent, (LPPOINT)&rcl);
    ClientToScreen(parent, (LPPOINT)&rcl.right);
    rcl.right -= rcl.left;
    rcl.bottom -= rcl.top;

    RECT rect;
    GetClientRect(window, &rect);
    ClientToScreen(window, (LPPOINT)&rect);
    ClientToScreen(window, (LPPOINT)&rect.right);
    rect.right -= rect.left;
    rect.bottom -= rect.top;
    int x = (rcl.right - rect.right + 1) / 2;
    int y = (rcl.bottom - rect.bottom + 1) / 2;

    x = std::max(x, 0);
    y = std::max(y, 0);

    SetWindowPos(window, nullptr, x, y, -1, -1, SWP_NOSIZE | SWP_NOZORDER);
}


/**
 *  This function gets the display rectangle of a window.
 *  It needs to use the real ClientToScreen so that dialogs are where they should be.
 *
 *  @author: tomsons26, ZivDero
 */
BOOL _GetDisplayRect(HWND window, LPRECT rect)
{
    RECT c;
    BOOL res = GetWindowRect(window, rect);
    if (!res) {
        return res;
    }
    GetClientRect(MainWindow, &c);
    ClientToScreen(MainWindow, (LPPOINT)&c);
    rect->left -= c.left;
    rect->right -= c.left;
    rect->top -= c.top;
    rect->bottom -= c.top;
    return res;
}


/**
 *  Proxy for GetWindowRect because we cannot take the address of GetWindowRect directly.
 *
 *  @author: ZivDero
 */
BOOL _GetWindowRect(HWND window, LPRECT rect)
{
    return GetWindowRect(window, rect);
}


/**
 *  Patch to make tooltips use MouseCursor for mouse position instead of querying Windows.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0064743E, _ToolTipManager_Message_Handler_Mouse_Pos_Patch_, 0)
{
    GET(ToolTipManager*, tooltips, ESI);

    tooltips->LastMousePos = MouseCursor->Get_Mouse_Point();

    return 0x00647450;
}


/**
 *  Replacement for EnumDisplayModes that uses Windows API to enumerate display modes
 *  instead of relying on DirectDraw.
 *
 *  @author: ZivDero
 */
int* _EnumDisplayModes(DWORD minw, DWORD minh, DWORD maxw, DWORD maxh, DWORD bitdepth)
{
    std::vector<std::pair<int, int>> modes;
    DEVMODE devmode;
    DWORD mode_index = 0;

    /**
     *  Enumerate all available display modes.
     */
    while (EnumDisplaySettings(nullptr, mode_index++, &devmode)) {
        const DWORD w = devmode.dmPelsWidth;
        const DWORD h = devmode.dmPelsHeight;
        const DWORD bpp = devmode.dmBitsPerPel;

        if (w >= minw && h >= minh && w <= maxw && h <= maxh && bpp == bitdepth) {
            modes.emplace_back(static_cast<int>(w), static_cast<int>(h));
        }
    }

    if (modes.empty()) {
        return nullptr;
    }

    /**
     *  Sort and remove duplicates.
     */
    std::sort(modes.begin(), modes.end());
    modes.erase(std::unique(modes.begin(), modes.end()), modes.end());

    /**
     *  Allocate contiguous buffer for result (two ints per mode, plus a trailing 0).
     */
    const size_t count = modes.size();
    const size_t bytes = sizeof(int) * (count * 2 + 1);

    int* list = static_cast<int*>(operator new[](bytes));
    std::memset(list, 0, bytes);

    int* ptr = list;
    for (const auto& mode : modes) {
        *ptr++ = mode.first;
        *ptr++ = mode.second;
    }

    return list;
}


/**
 *  Main function for patching the hooks.
 */
void SDL_Hooks()
{
    /**
     *  Disable ClientToScreen.
     */
    Patch_Dword(0x006CA384, (uintptr_t)&Fake_ClientToScreen);

    /**
     *  But these 3 need to use the real ClientToScreen so that dialogs are where they should be.
     */
    Patch_Jump(0x005A0BA0, &_ODMoveDialog);
    Patch_Jump(0x00685600, &_Center_Window_Within_Window);
    Patch_Jump(0x00682F80, &_GetDisplayRect);

    /**
     *  Except for this GetDisplayRect, it's used only for drawing offset, and returning GetWindowRect makes the offset 0.
     */
    Patch_Call(0x005924F0, &_GetWindowRect);

    /**
     *  ComboDropWinCtrlProc, fix the dropdown being offset by the main window origin by patching out 2 add instructions.
     */
    Patch_Byte_Range(0x0058FFB2, 0x90, 2);
    Patch_Byte_Range(0x0058FFBF, 0x90, 2);

    /**
     *  Skip Wait_Blit.
     */
    Patch_Jump(0x00473330, 0x00473348);

    /**
     *  SDL rendering prep.
     */
    Patch_Jump(0x004E7310, &SDL_Allocate_Surfaces);
    Patch_Jump(0x00472DF0, &SDL_Set_Video_Mode);
    Patch_Jump(0x00472FF0, &SDL_Reset_Video_Mode);
    Patch_Jump(0x0050AC30, &SDL_Change_Display_Mode);

    /**
     *  Update the window surface when the game updates its VisibleSurface.
     */
    Patch_Jump(0x00564050, &SDL_Movie_Blit_To_Screen);
    Patch_Jump(0x005646E0, &SDL_Movie_Update_Visible_Surface);
    Patch_Dword(0x00591739 + 1, (uintptr_t)&CtrlProcProxy); // Windows controls

    /**
     *  Call Set_Video_Mode even when windowed.
     */
    Patch_Jump(0x006016B8, 0x006016F3);
    Patch_Jump(0x006016BF, 0x006015A9);

    /**
     *  Disable DirectDraw.
     */
    Change_Virtual_Address(0x006EC110, (uintptr_t)&"NQXZJYVPRKMTLUGHSBDCFIEWOAQRMZNPLXTYVJHKSQGBFUACEL.DLL"); // replace DDRAW.DLL by a very unlikely library in the import table
    Patch_Jump(0x00472AD3, 0x00472B16); // skip Prep_Direct_Draw
    Patch_Jump(0x00473400, &_EnumDisplayModes); // relies on DirectDraw to enumerate display modes
}
