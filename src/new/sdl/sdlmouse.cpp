/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  SDL Mouse class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sdlmouse.h"

#include "SDL3/SDL_hints.h"
#include "SDL3/SDL_mouse.h"
#include "convert.h"
#include "optionsext.h"
#include "sdl_functions.h"
#include "sdlsurface.h"
#include "shapeset.h"

#include <algorithm>


/**
 *  Constructor for mouse handler object.
 *
 *  @author: ZivDero, tomsons26
 */
SDLMouseClass::SDLMouseClass() :
    MouseShape(nullptr),
    ShapeNumber(0),
    OriginalHotspot(0, 0),
    Hotspot(0, 0),
    Cursor(nullptr),
    CursorOwned(false),
    SystemCursorCache{},
    IsOverriding(false),
    IsOverrideHidden(false),
    CurrentOverrideId(SDL_SYSTEM_CURSOR_DEFAULT),
    IsCaptured(false)
{
    // Ensure the mouse image won't get scaled by SDL
    SDL_SetHint(SDL_HINT_MOUSE_DPI_SCALE_CURSORS, "0");
}


/**
 *  Destructor for mouse handler object.
 *
 *  @author: ZivDero, tomsons26
 */
SDLMouseClass::~SDLMouseClass()
{
    Delete_Cursor_Image();
    if (Cursor && CursorOwned) {
        SDL_DestroyCursor(Cursor);
    }
    Cursor = nullptr;
    CursorOwned = false;
    for (int i = 0; i < SDL_SYSTEM_CURSOR_COUNT; ++i) {
        if (SystemCursorCache[i] != nullptr) {
            SDL_DestroyCursor(SystemCursorCache[i]);
            SystemCursorCache[i] = nullptr;
        }
    }
}


/**
 *  Set the mouse cursor shape.
 *
 *  @author: ZivDero, tomsons26
 */
void SDLMouseClass::Set_Cursor(Point2D const& hotspot, ShapeSet const* cursor, int shape)
{
    if (cursor == nullptr || shape < 0 || shape >= cursor->Get_Count()) {
        Delete_Cursor_Image();
        Set_System_Cursor();
        return;
    }

    if (MouseShape == cursor && ShapeNumber == shape) {
        return;
    }

    if (cursor != MouseShape) {
        Delete_Cursor_Image();
        Convert_Cursor_Image(cursor);
        CursorCache.assign(CursorSurfaces.size(), CachedCursor{});
    }

    MouseShape = cursor;
    ShapeNumber = shape;

    /**
     *  Stash the unscaled hotspot so Recalc_Cursor_Image() can re-apply the
     *  current scale without compounding from a previously-scaled value.
     */
    OriginalHotspot = hotspot;

    /**
     *  Scale the hotspot. The max value is surface dimension - 1 as required by SDL.
     */
    const int scale = Get_Cursor_Scale();
    Hotspot.X = std::clamp(OriginalHotspot.X * scale, 0, CursorSurfaces[shape]->w - 1);
    Hotspot.Y = std::clamp(OriginalHotspot.Y * scale, 0, CursorSurfaces[shape]->h - 1);

    /**
     *  Cache hit: the SDL_Cursor for this (frame, hotspot) already exists.
     *  Just hand it to SDL without allocating a new Win32 HCURSOR.
     */
    CachedCursor& entry = CursorCache[shape];
    if (entry.cursor != nullptr && entry.hotspot_x == Hotspot.X && entry.hotspot_y == Hotspot.Y) {
        Replace_Cursor(entry.cursor, false);
        return;
    }

    /**
     *  Cache miss: drop any stale entry for this frame (e.g. hotspot changed),
     *  create a new SDL_Cursor, and store it for reuse.
     */
    if (entry.cursor != nullptr) {
        SDL_DestroyCursor(entry.cursor);
    }
    entry.cursor = SDL_CreateColorCursor(CursorSurfaces[shape], Hotspot.X, Hotspot.Y);
    entry.hotspot_x = Hotspot.X;
    entry.hotspot_y = Hotspot.Y;
    Replace_Cursor(entry.cursor, false);
}


/**
 *  Hides the mouse from the screen.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Hide_Mouse()
{
    SDL_HideCursor();
}


/**
 *  Shows the mouse on the screen.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Show_Mouse()
{
    SDL_ShowCursor();
}


/**
 *  Releases the mouse from its confinement area.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Release_Mouse()
{
    if (WindowedMode || !IsCaptured) {
        return;
    }

    /**
     *  Release system capture and unlock cursor.
     */
    ClipCursor(nullptr);

    IsCaptured = false;
}


/**
 *  Confines the mouse to the window's area.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Capture_Mouse()
{
    if (WindowedMode || IsCaptured) {
        return;
    }

    /**
     *  Compute the client area in screen coordinates.
     */
    RECT client_rect;
    GetClientRect(MainWindow, &client_rect);
    POINT ul = {client_rect.left, client_rect.top};
    POINT lr = {client_rect.right, client_rect.bottom};
    MapWindowPoints(MainWindow, nullptr, &ul, 1);
    MapWindowPoints(MainWindow, nullptr, &lr, 1);
    RECT clip_rect = {ul.x, ul.y, lr.x, lr.y};

    /**
     *  Lock cursor inside window.
     */
    ClipCursor(&clip_rect);

    IsCaptured = true;
}


/**
 *  Would have hidden the mouse if it overlaps the region specified.
 *  In reality, hides the mouse unconditionally in vanilla, and is thus
 *  implemented identically here.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Conditional_Hide_Mouse(Rect)
{
    Hide_Mouse();
}


/**
 *  The counterpart to Conditional_Hide_Mouse.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Conditional_Show_Mouse()
{
    Show_Mouse();
}


/**
 *  Fetch the current mouse visibility state.
 *  Returns with the current mouse visibility state. If the return value is less than
 *  0 (i.e., negative), then the mouse is hidden.
 *
 *  @author: ZivDero
 */
int SDLMouseClass::Get_Mouse_State() const
{
    return SDL_CursorVisible() ? 1 : -1;
}


/**
 *  Returns the current mouse cursor X position in window coordinates.
 *
 *  @author: ZivDero
 */
int SDLMouseClass::Get_Mouse_X() const
{
    float x, y;
    SDL_GetMouseState(&x, &y);
    return static_cast<int>(x);
}


/**
 *  Returns the current mouse cursor Y position in window coordinates.
 *
 *  @author: ZivDero
 */
int SDLMouseClass::Get_Mouse_Y() const
{
    float x, y;
    SDL_GetMouseState(&x, &y);
    return static_cast<int>(y);
}


/**
 *  Returns the current mouse cursor position in window coordinates.
 *
 *  @author: ZivDero
 */
Point2D SDLMouseClass::Get_Mouse_Point() const
{
    float x, y;
    SDL_GetMouseState(&x, &y);
    return Point2D(static_cast<int>(x), static_cast<int>(y));
}


/**
 *  Deletes the cached cursor image data.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Delete_Cursor_Image()
{
    /**
     *  Destroy every cached cursor before the surfaces they were built from go away.
     *  If the currently-active Cursor pointer is borrowed from the cache, clear it
     *  too so we don't leave it dangling.
     */
    for (CachedCursor& entry : CursorCache) {
        if (entry.cursor != nullptr) {
            if (Cursor == entry.cursor) {
                Cursor = nullptr;
                CursorOwned = false;
            }
            SDL_DestroyCursor(entry.cursor);
            entry.cursor = nullptr;
        }
    }
    CursorCache.clear();

    while (!CursorSurfaces.empty()) {
        SDL_DestroySurface(*CursorSurfaces.begin());
        CursorSurfaces.erase(CursorSurfaces.begin());
    }

    MouseShape = nullptr;
    ShapeNumber = 0;
}


/**
 *  Converts the cursor shape set into SDL_Surfaces.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Convert_Cursor_Image(ShapeSet const* shapes)
{
    if (!shapes) {
        return;
    }

    /**
     *  Convert the mouse drawer into an SDL palette.
     */
    static SDL_Palette* palette = nullptr;
    if (palette == nullptr) {
        palette = SDL_CreatePalette(256);
        for (int i = 0; i < 256; ++i) {
            uint16_t c = static_cast<uint16_t*>(MouseDrawer->Translator)[i];
            unsigned r, g, b;
            DSurface::Build_Locolor_Pixel(c, &r, &g, &b);
            palette->colors[i].r = r;
            palette->colors[i].g = g;
            palette->colors[i].b = b;
            palette->colors[i].a = (i == 0 ? 0 : 255);
        }
    }

    /**
     *  Each shape frame becomes a surface.
     */
    for (int i = 0; i < shapes->Get_Count(); i++) {

        /**
         *  Full image dimensions.
         */
        int width = shapes->Get_Width();
        int height = shapes->Get_Height();

        /**
         *  Non-empty area (cropped frame region).
         */
        Rect r = shapes->Get_Rect(i);

        /**
         *  Create 8-bit surface for the shape.
         */
        SDL_Surface* source = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
        SDL_SetSurfacePalette(source, palette);
        SDL_SetSurfaceColorKey(source, true, 0);

        uint8_t* dst = static_cast<uint8_t*>(source->pixels);
        const uint8_t* src = static_cast<const uint8_t*>(shapes->Get_Data(i));

        /**
         *  Copy frame data into the correct offset in the full surface.
         */
        for (int y = 0; y < r.Height; ++y) {
            uint8_t* dst_row = dst + (r.Y + y) * source->pitch + r.X;
            const uint8_t* src_row = src + y * r.Width;
            memcpy(dst_row, src_row, r.Width);
        }

        /**
         *  Now create ARGB destination with correct scaling.
         */
        SDL_Surface* destination = SDL_CreateSurface(width * Get_Cursor_Scale(), height * Get_Cursor_Scale(), SDL_PIXELFORMAT_ARGB8888);

        /**
         *  Blit to the full color surface with scaling.
         *  Use pixel-art scaling for crisp edges.
         */
        SDL_BlitSurfaceScaled(source, nullptr, destination, nullptr, SDL_SCALEMODE_PIXELART);

        CursorSurfaces.emplace_back(destination);
        SDL_DestroySurface(source);
    }
}


/**
 *  Replaces the current cursor with the given one.
 *  `owned` indicates whether this class is responsible for destroying `cursor`
 *  when it is replaced. Cursors that come from the cache are not owned here.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Replace_Cursor(SDL_Cursor* cursor, bool owned)
{
    SDL_Cursor* old_cursor = Cursor;
    bool old_owned = CursorOwned;

    Cursor = cursor;
    CursorOwned = owned;
    SDL_SetCursor(Cursor);

    if (old_cursor != nullptr && old_owned) {
        SDL_DestroyCursor(old_cursor);
    }
}


/**
 *  Resets the cursor to the system default. The HCURSOR is allocated lazily
 *  on first use and kept alive for the lifetime of this class, so repeated
 *  calls don't churn Win32 cursor handles.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Set_System_Cursor()
{
    Replace_Cursor(Get_System_Cursor(SDL_SYSTEM_CURSOR_DEFAULT), false);
}


/**
 *  Lazily allocates and returns a cached SDL_Cursor for the given system id.
 *  Cached for the lifetime of the class; freed in the destructor.
 *
 *  @author: ZivDero
 */
SDL_Cursor* SDLMouseClass::Get_System_Cursor(SDL_SystemCursor id)
{
    if (SystemCursorCache[id] == nullptr) {
        SystemCursorCache[id] = SDL_CreateSystemCursor(id);
    }
    return SystemCursorCache[id];
}


/**
 *  Overrides the cursor with a system cursor. Always re-applies SDL_cursor
 *  since game code may have changed it between our last call and this one.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Set_Override_System_Cursor(SDL_SystemCursor id)
{
    if (IsOverrideHidden) {
        SDL_ShowCursor();
        IsOverrideHidden = false;
    }

    Replace_Cursor(Get_System_Cursor(id), false);
    IsOverriding = true;
    CurrentOverrideId = id;
}


/**
 *  Hides the OS cursor as part of an override.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Hide_Override_Cursor()
{
    if (IsOverrideHidden) {
        return;
    }
    SDL_HideCursor();
    IsOverrideHidden = true;
    IsOverriding = true;
}


/**
 *  Ends a cursor override and restores the prior game cursor.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Clear_Cursor_Override()
{
    if (!IsOverriding && !IsOverrideHidden) {
        return;
    }

    if (IsOverrideHidden) {
        SDL_ShowCursor();
        IsOverrideHidden = false;
    }

    IsOverriding = false;

    if (MouseShape == nullptr) {
        Set_System_Cursor();
        return;
    }

    /**
     *  Null MouseShape so Set_Cursor's early-return can't fire, then re-apply
     *  via the CursorCache path (no surface / HCURSOR reallocation).
     */
    ShapeSet const* shape = MouseShape;
    int shape_number = ShapeNumber;
    Point2D unscaled_hotspot = OriginalHotspot;

    MouseShape = nullptr;
    Set_Cursor(unscaled_hotspot, shape, shape_number);
}


/**
 *  Recalculates the cursor's image using the same shape.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Recalc_Cursor_Image()
{
    /**
     *  Skip while overriding; the next Clear_Cursor_Override rebuilds the cache.
     */
    if (IsOverriding || IsOverrideHidden) {
        return;
    }

    if (MouseShape == nullptr) {
        return;
    }

    ShapeSet const* shape = MouseShape;
    int shape_number = ShapeNumber;
    Point2D unscaled_hotspot = OriginalHotspot;

    Delete_Cursor_Image();
    Set_Cursor(unscaled_hotspot, shape, shape_number);
}


/**
 *  Returns the current cursor scale factor.
 *
 *  @author: ZivDero
 */
int SDLMouseClass::Get_Cursor_Scale()
{
    /**
     *  If we aren't scaling the game, don't scale the cursor.
     *  This is so that the cursor doesn't loook gigantic in
     *  Windows menus.
     */
    if (!SDL_Should_Scale()) {
        return 1;
    }

    /**
     *  Negative values mean no scaling.
     */
    if (OptionsExtension->CursorScale < 0) {
        return 1;
    }

    /**
     *  Positive values are an explicit user-set scaling factor.
     */
    if (OptionsExtension->CursorScale > 0) {
        return OptionsExtension->CursorScale;
    }

    /**
     *  Scale automatically based on the Y-axis scaling factor.
     */
    return std::max(1, static_cast<int>(std::round(1.0 / SDL_YScale())));
}
