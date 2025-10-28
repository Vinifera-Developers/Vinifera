/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SDLMOUSE.CPP
 *
 *  @author        ZivDero, tomsons26
 *
 *  @brief         SDL Mouse class.
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
#include "sdlmouse.h"
#include "blit.h"
#include "bsurface.h"
#include "shapeset.h"
#include "surface.h"
#include "wwmouse.h"
#include <assert.h>
#include "always.h"
#include "convert.h"
#include "debughandler.h"
#include "drawshape.h"
#include "options.h"
#include "optionsext.h"
#include "sdl_functions.h"
#include "sdlsurface.h"
#include "SDL3/SDL_mouse.h"

/**
 *  Persistent mouse object pointer that is used to facilitate access to the mouse
 *  handler object outside the context of a member function. This will be set to the
 *  mouse object most recently created.
 */
static SDLMouseClass* _MousePtr = nullptr;


/**
 *  Mouse O/S callback function.
 *  This routine is called periodically by the operating system. It handles updating the
 *  mouse cursor position to match the mouse movement.
 *
 *  @author: JLB
 */
void CALLBACK SDL_Callback_Process_Mouse(UINT, UINT, DWORD, DWORD, DWORD)
{
    if (_MousePtr != nullptr) {
        _MousePtr->Process_Mouse();
    }
}


/**
 *  Constructor for mouse handler object.
 *
 *  @author: ZivDero, tomsons26
 */
SDLMouseClass::SDLMouseClass() :
    MouseShape(nullptr),
    ShapeNumber(0),
    Hotspot(0, 0),
    Cursor(nullptr),
    IsCaptured(false),
    MouseX(0),
    MouseY(0),
    TimerHandle(0)
{
    _MousePtr = this;
    TimerHandle = timeSetEvent(1000 / 60, 1, SDL_Callback_Process_Mouse, 0, TIME_PERIODIC);
}


/**
 *  Destructor for mouse handler object.
 *
 *  @author: ZivDero, tomsons26
 */
SDLMouseClass::~SDLMouseClass()
{
    if (TimerHandle != NULL) {
        timeKillEvent(TimerHandle);
        _MousePtr = nullptr;
        TimerHandle = NULL;
    }
    if (Cursor) {
        SDL_DestroyCursor(Cursor);
        Cursor = nullptr;
    }
    if (_MousePtr == this) _MousePtr = nullptr;
}


/**
 *  Mouse processing callback routine.
 *
 *  @author: ZivDero, tomsons26
 */
void SDLMouseClass::Process_Mouse()
{
    float x, y;
    SDL_GetMouseState(&x, &y);
    Update_Mouse_Position(x, y);
}


/**
 *  Set the mouse cursor shape.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Set_Cursor(Point2D const& hotspot, ShapeSet const* cursor, int shape)
{
    if (cursor == nullptr || shape < 0 || shape >= cursor->Get_Count()) {
        Delete_Cursor_Image();
        Set_System_Cursor();
        return;
    }

    if (cursor != MouseShape) {
        Delete_Cursor_Image();
        Convert_Cursor_Image(cursor);
        MouseShape = cursor;
    }

    Hotspot = hotspot;
    Hotspot.X = std::clamp(Hotspot.X * Get_Cursor_Scale(), 0, CursorSurfaces[shape]->w - 1);
    Hotspot.Y = std::clamp(Hotspot.Y * Get_Cursor_Scale(), 0, CursorSurfaces[shape]->h - 1);

    Replace_Cursor(SDL_CreateColorCursor(CursorSurfaces[shape], Hotspot.X, Hotspot.Y));
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
 *  Updates the mouse position to match that specified.
 *
 *  @author: ZivDero, tomsons26
 */
void SDLMouseClass::Update_Mouse_Position(int x, int y)
{
    /**
     *  If the desired position is not the same as the current
     *  position, then hide the mouse, reposition it, then show
     *  the mouse.
     */
    if (x != MouseX || y != MouseY) {
        MouseX = x;
        MouseY = y;
    }
}


/**
 *  Deletes the cached cursor image data.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Delete_Cursor_Image()
{
    while (!CursorSurfaces.empty()) {
        SDL_DestroySurface(*CursorSurfaces.begin());
        CursorSurfaces.erase(CursorSurfaces.begin());
    }

    MouseShape = nullptr;
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
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Replace_Cursor(SDL_Cursor* cursor)
{
    SDL_Cursor* old_cursor = Cursor;

    Cursor = cursor;
    SDL_SetCursor(Cursor);

    if (old_cursor != nullptr) {
        SDL_DestroyCursor(old_cursor);
    }
}


/**
 *  Resets the cursor to the system default.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Set_System_Cursor()
{
    Replace_Cursor(SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT));
}


/**
 *  Recalculates the cursor's image using the same shape.
 *
 *  @author: ZivDero
 */
void SDLMouseClass::Recacl_Cursor_Image()
{
    ShapeSet const* shape = MouseShape;
    int shape_number = ShapeNumber;

    Delete_Cursor_Image();
    Set_Cursor(Hotspot, shape, shape_number);
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
    return static_cast<int>(std::round(1.0 / SDL_YScale()));
}
