/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains functions for the SDL system.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
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

