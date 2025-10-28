/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SDL_FUNCTIONS.H
 *
 *  @author        ZivDero
 *
 *  @brief         Contains functions for the SDL system.
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
#include "rect.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"

class Surface;

bool SDL_Allocate_Surfaces(const Rect& hidden_rect, const Rect& composite_rect, const Rect& tile_rect, const Rect& sidebar_rect, bool hidden_first);
bool SDL_Set_Video_Mode(HWND, int width, int height, int bits_per_pixel);
void SDL_Reset_Video_Mode();
void SDL_Update_Visible_Surface(bool flip_mouse, Surface* surface, Rect* rect);
bool SDL_Create_Main_Window(HINSTANCE instance, int width, int height);
void SDL_Destroy_Main_Window();
bool SDL_Update_Screen(Surface* surface);
bool SDL_Should_Scale();
bool SDL_Change_Display_Mode(int width, int height);

/**
 *  Returns the current X-axis scaling factor.
 *
 *  @author: ZivDero
 */
inline float SDL_XScale()
{
    return static_cast<float>(VideoWidth) / static_cast<float>(SDLWindowWidth);
}

/**
 *  Returns the current X-axis scaling factor.
 *
 *  @author: ZivDero
 */
inline float SDL_YScale()
{
    return static_cast<float>(VideoHeight) / static_cast<float>(SDLWindowHeight);
}

