#pragma once
#include "rect.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"

class Surface;
class SDLSurface;
struct SDL_Rect;


bool SDL_Allocate_Surfaces(const Rect& hidden_rect, const Rect& composite_rect, const Rect& tile_rect, const Rect& sidebar_rect, bool hidden_first);
void Prep_SDL();
void Destroy_SDL();
bool SDL_Set_Video_Mode(HWND, int w, int h, int bits_per_pixel);
void SDL_Reset_Video_Mode();
void SDL_Update_Visible_Surface(bool flip_mouse, Surface* surface, Rect* rect);
bool SDL_Create_Main_Window(HINSTANCE instance, int width, int height);
void SDL_Destroy_Main_Window();
bool SDL_Update_Screen(Surface* surface);
bool SDL_Should_Scale();
bool SDL_Change_Display_Mode(int width, int height);

inline float SDL_XScale()
{
    return static_cast<float>(VideoWidth) / static_cast<float>(SDLWindowWidth);
}

inline float SDL_YScale()
{
    return static_cast<float>(VideoHeight) / static_cast<float>(SDLWindowHeight);
}

