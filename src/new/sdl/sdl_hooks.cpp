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
#include "dsurface.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "sdl_init.h"
#include "sdlsurface.h"
#include "tibsun_globals.h"
#include "SDL3/SDL_timer.h"


void _Wait_Blit()
{
}


void _Set_Palette(void const* palette)
{
}


DECLARE_PATCH(_GScreenClass_Do_Blit_SDL_Update_Window_Patch)
{
    SDL_Update_Screen(static_cast<SDLSurface*>(VisibleSurface));

    _asm { test bl, bl }
    _asm { pop edi }
    _asm { pop ebp }
    _asm { pop ebx }
    JMP(0x004B9A47);
}


DECLARE_PATCH(_Main_Loop_SDL_Update_Window_Patch)
{
    SDL_Update_Screen(static_cast<SDLSurface*>(VisibleSurface));
    SDL_Delay(0);

    _asm { setz al }
    _asm { pop edi }
    _asm { add esp, 0x38 }
    _asm { ret }
}


/**
 *  Flip hidden surface onto the primary SDL surface when drawing movie frame.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MovieClass_Blit_SDL_Update_Window_Patch_1)
{
    // PrimarySurface (ecx) -> Copy_From
    _asm { mov eax, [edx+8] }
    _asm { call eax }

    //DEBUG_INFO("MovieClass::Blit(1) - Copying to PrimarySurface.\n");

    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }

    SDL_Update_Screen(static_cast<SDLSurface*>(VisibleSurface /*, &src_rect, &dest_rect*/) /*, &src_rect, &dest_rect*/);
    
    JMP(0x005640D3);
}


/**
 *  Flip hidden surface onto the primary SDL surface when drawing movie frame.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MovieClass_Blit_SDL_Update_Window_Patch_2)
{
    // PrimarySurface (ecx) -> Copy_From
    _asm { mov eax, [edx+8] }
    _asm { call eax }

    //DEBUG_INFO("MovieClass::Blit(2) - Copying to PrimarySurface.\n");

    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }

    SDL_Update_Screen(static_cast<SDLSurface*>(VisibleSurface /*, &src_rect, &dest_rect*/) /*, &src_rect, &dest_rect*/);
    
    JMP(0x0056478D);
}


DECLARE_PATCH(_MSEngine_Blit_SDL_Update_Window_Patch)
{
    // PrimarySurface (ecx) -> Copy_From
    _asm { push eax }
    _asm { mov edx, [ecx] }
    _asm { mov eax, [edx+0x8] }
    _asm { call eax }

    DEBUG_INFO("MSEngine::Blit() - Copying to PrimarySurface.\n");
    SDL_Update_Screen(static_cast<SDLSurface*>(VisibleSurface));

    JMP(0x0057111C);
}

DECLARE_PATCH(_MSEngine_Draw_SDL_Update_Window_Patch)
{
    // PrimarySurface (ecx) -> Copy_From
    _asm { push edx }
    _asm { mov ecx, [ecx] }
    _asm { mov eax, [ecx+0x8] }
    _asm { call eax }

    DEBUG_INFO("MSEngine::Draw() - Copying to PrimarySurface.\n");
    SDL_Update_Screen(static_cast<SDLSurface*>(VisibleSurface));

    JMP(0x005711F8);
}


DECLARE_PATCH(_SidebarClass_Blit_Sidebar_SDL_Update_Window_Patch)
{
    SDL_Update_Screen(static_cast<SDLSurface*>(VisibleSurface));

    _asm { ret 4 }
}


DECLARE_PATCH(_OwnerDraw_DialogProc_SDL_Update_Window_Patch_Return_0)
{
    SDL_Update_Screen(static_cast<SDLSurface*>(VisibleSurface));

    _asm { xor eax, eax }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x2E8 }
    _asm { ret 0x10 }
}


DECLARE_PATCH(_OwnerDraw_DialogProc_SDL_Update_Window_Patch_Return_1)
{
    SDL_Update_Screen(static_cast<SDLSurface*>(VisibleSurface));

    _asm { mov eax, 1 }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x2E8 }
    _asm { ret 0x10 }
}


DECLARE_PATCH(_OwnerDraw_DialogProc_SDL_Update_Window_Patch_Return_Var)
{
    SDL_Update_Screen(static_cast<SDLSurface*>(VisibleSurface));

    _asm { mov eax, [esp+0x1C] }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x2E8 }
    _asm { ret 0x10 }
}


/**
 *  Main function for patching the hooks.
 */
void SDL_Hooks()
{
    Patch_Jump(0x00473330, &_Wait_Blit);
    Patch_Jump(0x00473280, &_Wait_Blit);
    Patch_Jump(0x004E7310, &SDL_Allocate_Surfaces);
    Patch_Jump(0x00472AD0, &Prep_SDL);
    Patch_Jump(0x00472BC0, &Destroy_SDL);
    Patch_Jump(0x00472DF0, &SDL_Set_Video_Mode);
    Patch_Jump(0x00472FF0, &SDL_Reset_Video_Mode);

    // Patch_Jump(0x005F3C61, &_SidebarClass_Blit_Sidebar_SDL_Update_Window_Patch);
    Patch_Jump(0x005640CD, &_MovieClass_Blit_SDL_Update_Window_Patch_1);
    Patch_Jump(0x00564787, &_MovieClass_Blit_SDL_Update_Window_Patch_2);
    Patch_Jump(0x00571116, &_MSEngine_Blit_SDL_Update_Window_Patch);
    Patch_Jump(0x005711F5, &_MSEngine_Draw_SDL_Update_Window_Patch);
    Patch_Jump(0x00592356, &_OwnerDraw_DialogProc_SDL_Update_Window_Patch_Return_1);
    Patch_Jump(0x0059264F, &_OwnerDraw_DialogProc_SDL_Update_Window_Patch_Return_0);
    Patch_Jump(0x005926D8, &_OwnerDraw_DialogProc_SDL_Update_Window_Patch_Return_1);
    Patch_Jump(0x00592802, &_OwnerDraw_DialogProc_SDL_Update_Window_Patch_Return_1);
    Patch_Jump(0x005944EF, &_OwnerDraw_DialogProc_SDL_Update_Window_Patch_Return_1);
    Patch_Jump(0x005944FE, &_OwnerDraw_DialogProc_SDL_Update_Window_Patch_Return_Var);
    Patch_Jump(0x004B9A42, &_GScreenClass_Do_Blit_SDL_Update_Window_Patch);
}