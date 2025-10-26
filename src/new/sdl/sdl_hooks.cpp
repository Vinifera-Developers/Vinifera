/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SDL_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the SDL system.
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
#include "bsurface.h"
#include "dsurface.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "sdl_init.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "vinifera_globals.h"
#include "winuser.h"
#include "xmouse.h"
#include "SDL3/SDL_timer.h"
#include <dsound.h>
#include <algorithm>


DECLARE_PATCH(_Update_Visible_Surface_SDL_Update_Window_Patch)
{
    SDL_Update_Screen(VisibleSurface);

    _asm { test bl, bl }
    _asm { pop edi }
    _asm { pop ebp }
    _asm { pop ebx }
    JMP(0x004B9A47);
}


/**
 *  Flip hidden surface onto the primary SDL surface when drawing movie frame.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Movie_Blit_To_Screen_SDL_Update_Window_Patch)
{
    // VisibleSurface (ecx) -> Blit_From
    _asm { call [edx + 0x8] }

    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }

    SDL_Update_Screen(VisibleSurface);
    
    JMP(0x005640D3);
}


/**
 *  Flip hidden surface onto the primary SDL surface when drawing movie frame.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Movie_Update_Visisble_Surface_SDL_Update_Window_Patch)
{
    // VisibleSurface (ecx) -> Blit_From
    _asm { call [edx + 0x8] }

    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }

    SDL_Update_Screen(VisibleSurface);
    
    JMP(0x0056478D);
}


DECLARE_PATCH(_MSEngine_BlitAll_SDL_Update_Window_Patch)
{
    // VisibleSurface (ecx) -> Blit_From
    _asm { mov edx, [ecx] }
    _asm { push eax }
    _asm { call [edx + 0x8] }

    SDL_Update_Screen(VisibleSurface);

    JMP(0x0057111C);
}

DECLARE_PATCH(_MSEngine_BlitRect_SDL_Update_Window_Patch)
{
    // VisibleSurface (ecx) -> Blit_From
    _asm { mov eax, [ecx] }
    _asm { push edx }
    _asm { call [eax+8] }

    SDL_Update_Screen(VisibleSurface);

    JMP(0x005711F8);
}


BOOL WINAPI Fake_ClientToScreen(HWND hwnd, LPPOINT point)
{
    return TRUE;
}


BOOL WINAPI Fake_ValidateRect(HWND hwnd, LPCRECT lpRect)
{
    if (hwnd == MainWindow) return TRUE; // do nothing for SDL main window
    return ValidateRect(hwnd, lpRect);   // let USER32 handle child dialogs normally
}


LRESULT CALLBACK CtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
DEFINE_IMPLEMENTATION(LRESULT CALLBACK CtrlProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam), 0x00592340);


LRESULT CALLBACK CtrlProcProxy(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    LRESULT result = CtrlProc(window, message, wparam, lparam);
    if (message == WM_PAINT) {
        SDL_Update_Screen(VisibleSurface);
    }
    return result;
}


DECLARE_PATCH(_CtrlProc_SDL_Update_Screen1)
{
    AlternateSurface->Unlock();
    VisibleSurface->Unlock();
    SDL_Update_Screen(VisibleSurface);
    JMP(0x00593FA3);
}


DECLARE_PATCH(_CtrlProc_SDL_Update_Screen2)
{
    AlternateSurface->Unlock();
    VisibleSurface->Unlock();
    SDL_Update_Screen(VisibleSurface);
    JMP(0x00594117);
}


DECLARE_PATCH(_CtrlProc_SDL_Update_Screen3)
{
    AlternateSurface->Unlock();
    VisibleSurface->Unlock();
    SDL_Update_Screen(VisibleSurface);
    JMP(0x00594387);
}


DECLARE_PATCH(_CtrlProc_SDL_Update_Screen4)
{
    AlternateSurface->Unlock();
    VisibleSurface->Unlock();
    SDL_Update_Screen(VisibleSurface);
    JMP(0x005944B5);
}


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


BOOL _GetWindowRect(HWND window, LPRECT rect)
{
    return GetWindowRect(window, rect);
}


void Update_ToolTip_Mouse_Pos(ToolTipManager* tooltips)
{
    tooltips->LastMousePos = MouseCursor->Get_Mouse_Point();
}


//64743E
DECLARE_PATCH(_ToolTopManager_Message_Handler_Mouse_Pos_Patch_)
{
    GET_REGISTER_STATIC(ToolTipManager*, tooltips, esi);

    Update_ToolTip_Mouse_Pos(tooltips);

    JMP(0x00647450);
}



int* _EnumDisplayModes(DWORD minw, DWORD minh, DWORD maxw, DWORD maxh, DWORD bitdepth)
{
    std::vector<std::pair<int, int>> modes;
    DEVMODE devmode;
    DWORD mode_index = 0;

    // Enumerate all available display modes
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

    // Sort and remove duplicates
    std::sort(modes.begin(), modes.end());
    modes.erase(std::unique(modes.begin(), modes.end()), modes.end());

    // Allocate contiguous buffer for result (two ints per mode, plus a trailing 0)
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
    // These 3 need to use the real ClientToScreen so that dialogs are where they should be
    Patch_Jump(0x005A0BA0, &_ODMoveDialog);
    Patch_Jump(0x00685600, &_Center_Window_Within_Window);
    Patch_Jump(0x00682F80, &_GetDisplayRect);

    // Except for this GetDisplayRect, it's used only for drawing offset, and returning GetWindowRect makes the offset 0
    Patch_Call(0x005924F0, &_GetWindowRect);
    
    // Disable some drawing calls
    Patch_Dword(0x006CA384, (uintptr_t)&Fake_ClientToScreen);
    Patch_Dword(0x006CA3C4, (uintptr_t)&Fake_ValidateRect);

    // Skip Wait_Blit
    Patch_Jump(0x00473330, 0x00473348);

    // SDL prep
    Patch_Jump(0x004E7310, &SDL_Allocate_Surfaces);
    Patch_Jump(0x00472DF0, &SDL_Set_Video_Mode);
    Patch_Jump(0x00472FF0, &SDL_Reset_Video_Mode);
    Patch_Jump(0x0050AC30, &SDL_Change_Display_Mode);

    // VQA
    Patch_Jump(0x005640CD, &_Movie_Blit_To_Screen_SDL_Update_Window_Patch);
    Patch_Jump(0x00564787, &_Movie_Update_Visisble_Surface_SDL_Update_Window_Patch);

    // MSEngine
    Patch_Jump(0x00571116, &_MSEngine_BlitAll_SDL_Update_Window_Patch);
    Patch_Jump(0x005711F2, &_MSEngine_BlitRect_SDL_Update_Window_Patch);

    // Most other cases
    Patch_Jump(0x004B9A42, &_Update_Visible_Surface_SDL_Update_Window_Patch);

    // Copy over the surface after a control has been redrawn
    Patch_Dword(0x00591739 + 1, (uintptr_t)&CtrlProcProxy);

    // Update surface during the window sliding open animation
    Patch_Jump(0x00593F8D, &_CtrlProc_SDL_Update_Screen1);
    Patch_Jump(0x00594101, &_CtrlProc_SDL_Update_Screen2);
    Patch_Jump(0x0059437C, &_CtrlProc_SDL_Update_Screen3);
    Patch_Jump(0x0059449F, &_CtrlProc_SDL_Update_Screen4);

    // call Set_Video_Mode even when windowed
    Patch_Jump(0x006016B8, 0x006016F3);
    Patch_Jump(0x006016BF, 0x006015A9);

    // patch ToolTipManager to use MouseCursor for the mouse coordinates instead of asking Windows
    Patch_Jump(0x0064743E, &_ToolTopManager_Message_Handler_Mouse_Pos_Patch_);

    Change_Virtual_Address(0x006EC110, (uintptr_t)&"NQXZJYVPRKMTLUGHSBDCFIEWOAQRMZNPLXTYVJHKSQGBFUACEL.DLL"); // replace DDRAW.DLL by a very unlikely library in the import table
    Patch_Jump(0x00472AD3, 0x00472B16); // skip Prep_Direct_Draw

    Patch_Jump(0x00473400, &_EnumDisplayModes);
}