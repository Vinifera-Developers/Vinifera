/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SDL_HOOKS.H
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
#pragma once
#include "bsurface.h"
#include "dsurface.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "options.h"
#include "sdl_init.h"
#include "sdlsurface.h"
#include "shapeset.h"
#include "tibsun_globals.h"
#include "wwmouse.h"
#include "winuser.h"
#include "SDL3/SDL_timer.h"
#include <dsound.h>
#include <algorithm>


void _Wait_Blit()
{
}


void _Set_Palette(void const* palette)
{
}


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
DECLARE_PATCH(_Movie_Blit_To_Screen_SDL_Update_Window_Patch_1)
{
    // VisibleSurface (ecx) -> Blit_From
    _asm { mov eax, [edx+8] }
    _asm { call eax }

    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }

    SDL_Update_Screen(VisibleSurface /*, &src_rect, &dest_rect*/);
    
    JMP(0x005640D3);
}


/**
 *  Flip hidden surface onto the primary SDL surface when drawing movie frame.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Movie_Update_Visisble_Surface_SDL_Update_Window_Patch_2)
{
    // VisibleSurface (ecx) -> Blit_From
    _asm { mov eax, [edx+8] }
    _asm { call eax }

    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }

    SDL_Update_Screen(VisibleSurface);
    
    JMP(0x0056478D);
}


DECLARE_PATCH(_MSEngine_BlitAll_SDL_Update_Window_Patch)
{
    // VisibleSurface (ecx) -> Blit_From
    _asm { push eax }
    _asm { mov edx, [ecx] }
    _asm { mov eax, [edx+0x8] }
    _asm { call eax }

    DEBUG_INFO("MSEngine::Blit() - Copying to VisibleSurface.\n");
    SDL_Update_Screen(VisibleSurface);

    JMP(0x0057111C);
}

DECLARE_PATCH(_MSEngine_BlitRect_SDL_Update_Window_Patch)
{
    // VisibleSurface (ecx) -> Blit_From
    _asm { push edx }
    _asm { mov ecx, [ecx] }
    _asm { mov eax, [ecx+0x8] }
    _asm { call eax }

    DEBUG_INFO("MSEngine::Draw() - Copying to VisibleSurface.\n");
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


BOOL WINAPI Fake_InvalidateRect(HWND hwnd, LPCRECT lpRect, BOOL bErase)
{
    if (hwnd == MainWindow) return TRUE; // do nothing for SDL main window
    return InvalidateRect(hwnd, lpRect, bErase);   // let USER32 handle child dialogs normally
}

bool& AllowNegativeY = Make_Global<bool>(0x00867510);

class WWMouseClassExt : public WWMouseClass
{
public:
    void _Get_Bounded_Position(int& x, int& y) const
    {
        /*
        ** Get the mouse's current real cursor position
        */
        POINT pt;
        GetCursorPos(&pt); // get the current cursor position
        ScreenToClient(Window, &pt);
        x = pt.x;
        y = pt.y;
        Convert_Coordinate(x, y);
    }

    void _Process_Mouse()
    {
        static bool _forced = false;

        if (SurfacePtr != nullptr) {
            Block_Mouse();

            /*
            ** Fetch and update the mouse position.
            */
            int x;
            int y;
            Get_Bounded_Position(x, y);
            if (!SurfacePtr->entry_64() && !_forced) {
                MouseX = x;
                MouseY = y;
                _forced = true;
            } else {
                Update_Mouse_Position(x, y, _forced);
                _forced = false;
            }

            Unblock_Mouse();
        }
    }
};


void CALLBACK _Callback_Process_Mouse(UINT, UINT, DWORD, DWORD, DWORD)
{
    if (MouseCursor != nullptr) {
        MouseCursor->Process_Mouse();
    }
}


int DefaultDialogProc(HWND window, UINT message, WPARAM wparam, LPARAM lparam);
DEFINE_IMPLEMENTATION(int DefaultDialogProc(HWND, UINT, WPARAM, LPARAM), 0x005A0840);


int DefaultDialogProcProxy(HWND window, UINT message, WPARAM wparam, LPARAM lparam)
{
    int res = DefaultDialogProc(window, message, wparam, lparam);
    if (message == WM_PAINT) {
        SDL_Update_Screen(VisibleSurface);
    }
    return res;
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


DECLARE_PATCH(_Windows_Procudure_Return_Patch)
{
    _asm {
        xor eax, eax
        pop esi
        pop ebp
        retn 10h
    }
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

    return (MoveWindow(window, rect2.left, rect2.top, rect2.right, rect2.bottom, FALSE));
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
        return (res);
    }
    GetClientRect(MainWindow, &c);
    ClientToScreen(MainWindow, (LPPOINT)&c);
    rect->left -= c.left;
    rect->right -= c.left;
    rect->top -= c.top;
    rect->bottom -= c.top;
    return (res);
}

/**
 *  Main function for patching the hooks.
 */
void SDL_Hooks()
{
    Patch_Jump(0x005A0BA0, &_ODMoveDialog);
    Patch_Jump(0x00685600, &_Center_Window_Within_Window);
    Patch_Jump(0x00682F80, &_GetDisplayRect);

    Patch_Dword(0x006CA384, (uintptr_t)&Fake_ClientToScreen);
    Patch_Dword(0x006CA3C4, (uintptr_t)&Fake_ValidateRect);
    //Patch_Dword(0x006CA3C8, (uintptr_t)&Fake_InvalidateRect);
    Patch_Dword(0x00591739 + 1, (uintptr_t)&CtrlProcProxy);

    // Dummies
    Patch_Jump(0x00473330, &_Wait_Blit);
    Patch_Jump(0x00473280, &_Wait_Blit);
    Patch_Jump(0x00472AD0, &Prep_SDL);
    Patch_Jump(0x00472BC0, &Destroy_SDL);

    Patch_Jump(0x004E7310, &SDL_Allocate_Surfaces);
    Patch_Jump(0x00472DF0, &SDL_Set_Video_Mode);
    Patch_Jump(0x00472FF0, &SDL_Reset_Video_Mode);

    Patch_Jump(0x006A6420, &WWMouseClassExt::_Get_Bounded_Position);
    Patch_Jump(0x006A66C0, &WWMouseClassExt::_Process_Mouse);
    Patch_Jump(0x006A4E10, &_Callback_Process_Mouse);

    // VQA
    Patch_Jump(0x005640CD, &_Movie_Blit_To_Screen_SDL_Update_Window_Patch_1);
    Patch_Jump(0x00564787, &_Movie_Update_Visisble_Surface_SDL_Update_Window_Patch_2);

    // MSEngine
    Patch_Jump(0x00571116, &_MSEngine_BlitAll_SDL_Update_Window_Patch);
    Patch_Jump(0x005711F5, &_MSEngine_BlitRect_SDL_Update_Window_Patch);

    // Most other cases
    Patch_Jump(0x004B9A42, &_Update_Visible_Surface_SDL_Update_Window_Patch);

    // TODO: Doesn't work, DefaultDialogProc is called too early, before anything is actually drawn
    //Patch_Call(0x00407019, &DefaultDialogProcProxy);
    //Patch_Call(0x00407019, &DefaultDialogProcProxy);
    //Patch_Call(0x004B68CC, &DefaultDialogProcProxy);
    //Patch_Call(0x004B6F39, &DefaultDialogProcProxy);
    //Patch_Call(0x004E321A, &DefaultDialogProcProxy);
    //Patch_Call(0x004E4B38, &DefaultDialogProcProxy);
    //Patch_Call(0x004E523C, &DefaultDialogProcProxy);
    //Patch_Call(0x00504A5A, &DefaultDialogProcProxy);
    //Patch_Call(0x00504B6A, &DefaultDialogProcProxy);
    //Patch_Call(0x00504C5A, &DefaultDialogProcProxy);
    //Patch_Call(0x0050A719, &DefaultDialogProcProxy);
    //Patch_Call(0x0050A7B9, &DefaultDialogProcProxy);
    //Patch_Call(0x0050B379, &DefaultDialogProcProxy);
    //Patch_Call(0x0050B3CC, &DefaultDialogProcProxy);
    //Patch_Call(0x0053A51A, &DefaultDialogProcProxy);
    //Patch_Call(0x0055A8E8, &DefaultDialogProcProxy);
    //Patch_Call(0x0055D4E9, &DefaultDialogProcProxy);
    //Patch_Call(0x00564CBA, &DefaultDialogProcProxy);
    //Patch_Call(0x005729FA, &DefaultDialogProcProxy);
    //Patch_Call(0x00581379, &DefaultDialogProcProxy);
    //Patch_Call(0x0058A66A, &DefaultDialogProcProxy);
    //Patch_Call(0x005A0D99, &DefaultDialogProcProxy);
    //Patch_Call(0x005A82F9, &DefaultDialogProcProxy);
    //Patch_Call(0x005A8F3C, &DefaultDialogProcProxy);
    //Patch_Call(0x005AE168, &DefaultDialogProcProxy);
    //Patch_Call(0x005EB5C7, &DefaultDialogProcProxy);
    //Patch_Call(0x005F74FA, &DefaultDialogProcProxy);
    //Patch_Call(0x005FC1A0, &DefaultDialogProcProxy);
    //Patch_Call(0x0069F329, &DefaultDialogProcProxy);
}