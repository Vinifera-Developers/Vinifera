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
#include "sdl_init.h"
#include "sdlsurface.h"
#include "SDL3/SDL_mouse.h"

/*
**  Persistent mouse object pointer that is used to facilitate access to the mouse
**  handler object outside the context of a member function. This will be set to the
**  mouse object most recently created.
*/
static SDLMouseClass* _MousePtr = nullptr;


/***********************************************************************************************
 * Callback_Process_Mouse -- Mouse O/S callback function.                                      *
 *                                                                                             *
 *    This routine is called periodically by the operating system. It handles updating the     *
 *    mouse cursor position to match the mouse movement.                                       *
 *                                                                                             *
 * INPUT:   n/a                                                                                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void CALLBACK SDL_Callback_Process_Mouse(UINT, UINT, DWORD, DWORD, DWORD)
{
    if (_MousePtr != nullptr) {
        _MousePtr->Process_Mouse();
    }
}


/***********************************************************************************************
 * SDLMouseClass::SDLMouseClass -- Constructor for mouse handler object.                         *
 *                                                                                             *
 *    This is the constructor for the mouse handler object. It is assigned to a surface and    *
 *    given a confining rectangle. The mouse begins in a non-captured state.                   *
 *                                                                                             *
 * INPUT:   surfaceptr  -- Pointer to the visible display surface that will show the mouse.    *
 *                                                                                             *
 *          confine     -- The confining rectangle within the visible surface. The mouse       *
 *                         coordinates are bound to this rectangle.                            *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
SDLMouseClass::SDLMouseClass() :
    MouseShape(nullptr),
    ShapeNumber(0),
    Cursor(nullptr),
    MouseX(0),
    MouseY(0),
    TimerHandle(0)
{
    _MousePtr = this;
    TimerHandle = timeSetEvent(1000 / 60, 1, SDL_Callback_Process_Mouse, 0, TIME_PERIODIC);
}


/***********************************************************************************************
 * SDLMouseClass::~SDLMouseClass -- Destructor for mouse handler object.                         *
 *                                                                                             *
 *    This will remove the mouse handler object from being processed. It returns the mouse     *
 *    back to O/S control.                                                                     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
SDLMouseClass::~SDLMouseClass()
{
    if (TimerHandle != NULL) {
        timeKillEvent(TimerHandle);
        _MousePtr = NULL;
#ifdef FIXIT_BUGS
        TimerHandle = NULL;
#endif
    }
    if (Cursor) {
        SDL_DestroyCursor(Cursor);
        Cursor = nullptr;
    }
    if (_MousePtr == this) _MousePtr = nullptr;
}


/***********************************************************************************************
 * SDLMouseClass::Process_Mouse -- Mouse processing callback routine.                           *
 *                                                                                             *
 *    This routine should be called periodically (recommended 15 times per second). It will    *
 *    examine the operating system mouse position and then update the visible mouse to match.  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Process_Mouse()
{
    float x, y;
    SDL_GetMouseState(&x, &y);
    Update_Mouse_Position(x, y, false);
}


/***********************************************************************************************
 * SDLMouseClass::Set_Cursor -- Set the mouse cursor shape.                                     *
 *                                                                                             *
 *    This routine sets the mouse cursor image and hot-spot. The shape only applies to the     *
 *    mouse when it is captured (the normal case). Repeated calls to this routine is used      *
 *    to give the mouse animation.                                                             *
 *                                                                                             *
 * INPUT:   xhotspot, yhotspot   -- The X,Y offset from the upper left corner of the shape     *
 *                                  that specifies the hot-spot of the image. Positive values  *
 *                                  are right and down from the upper left corner.             *
 *                                                                                             *
 *          cursor   -- Pointer to the shape data.                                             *
 *                                                                                             *
 *          shape    -- The shape number to use within the shape data set specified.           *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Set_Cursor(Point2D const& hotspot, ShapeSet const* cursor, int shape)
{
    if (cursor == nullptr) {
        Delete_Cursor_Image();
        Set_System_Cursor();
        return;
    }

    if (cursor != MouseShape || shape < 0 || shape >= cursor->Get_Count()) {
        Delete_Cursor_Image();
        Convert_Cursor_Image(cursor);
        MouseShape = cursor;
    }

    Clear_Cursor();

    Point2D scaled_hotspot = hotspot;
    scaled_hotspot.X = std::clamp(scaled_hotspot.X * Get_Cursor_XScale(), 0, CursorSurfaces[shape]->w - 1);
    scaled_hotspot.Y = std::clamp(scaled_hotspot.Y * Get_Cursor_YScale(), 0, CursorSurfaces[shape]->h - 1);
    Cursor = SDL_CreateColorCursor(CursorSurfaces[shape], scaled_hotspot.X, scaled_hotspot.Y);
    SDL_SetCursor(Cursor);
}


/***********************************************************************************************
 * SDLMouseClass::Hide_Mouse -- Hides the mouse from the visible surface.                       *
 *                                                                                             *
 *    This routine is called when the mouse is desired to be hidden from the visible surface.  *
 *    Typically, this must occur if the pixels where the mouse is located will be accessed.    *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Hide_Mouse()
{
    SDL_HideCursor();
}


/***********************************************************************************************
 * SDLMouseClass::Show_Mouse -- Shows the mouse on the visible surface.                         *
 *                                                                                             *
 *    This routine is called when the mouse can be shown on the visible surface.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Show_Mouse()
{
    SDL_ShowCursor();
}


/***********************************************************************************************
 * SDLMouseClass::Release_Mouse -- Release the mouse back to the O/S.                           *
 *                                                                                             *
 *    This is the counterpart routine to Capture_Mouse. This routine will return the drawing   *
 *    and movement controls back to the operating system. Although the mouse will probably     *
 *    be able to roam outside the confining rectangle, the coordinates returned by this class  *
 *    are clipped to the confining rectangle anyway. This gives the impression that the mouse  *
 *    is still at a legal position. The presumption is that the mouse needs to be released to  *
 *    the O/S for reasons outside of the game itself. As such, the shouldn't detect any        *
 *    illegal mouse position.                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   All mouse shape changes won't be relected while the mouse is released. The O/S  *
 *             handles drawing the mouse in that case.                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Release_Mouse()
{
    //SDL_CaptureMouse(false);
    SDL_SetWindowMouseGrab(SDLWindow, false);
    //SDL_SetWindowRelativeMouseMode(SDL_GetWindowFromID(1), false);
}


/***********************************************************************************************
 * SDLMouseClass::Capture_Mouse -- Capture the mouse into the mouse handler region.             *
 *                                                                                             *
 *    This routine will confine the mouse to the confining rectangle and take over drawing     *
 *    of the mouse image from the operating system. The typical state is to keep the mouse     *
 *    captured throughout the lifetime of the owning program.                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Capture_Mouse()
{
    //SDL_CaptureMouse(true);
    SDL_SetWindowMouseGrab(SDLWindow, true);
    //SDL_SetWindowRelativeMouseMode(SDL_GetWindowFromID(1), true);
}


/***********************************************************************************************
 * SDLMouseClass::Conditional_Hide_Mouse -- Hides the mouse if it would overlap the region spec *
 *                                                                                             *
 *    This routine will hide the mouse if it lies within the region specified or if it moves   *
 *    within the region.                                                                       *
 *                                                                                             *
 * INPUT:   rect  -- The rectangle that the mouse should not be drawn within.                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Conditional_Hide_Mouse(Rect)
{
    Hide_Mouse();
}


/***********************************************************************************************
 * SDLMouseClass::Conditional_Show_Mouse -- Releases the mouse hiding region tracking.          *
 *                                                                                             *
 *    This routine will release the region hiding tracking that was set up with a previous     *
 *    call to Conditional_Hide_Mouse().                                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Conditional_Show_Mouse()
{
    Show_Mouse();
}


/***********************************************************************************************
 * SDLMouseClass::Get_Mouse_State -- Fetch the current mouse visibility state.                  *
 *                                                                                             *
 *    This routine is used to retrieve the current mouse state as it relates to visiblity.     *
 *    By using this routine it is possible to determine if the mouse is visible.               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the current mouse visibility state. If the return value is less than  *
 *          0 (i.e., negative), then the mouse is hidden.                                      *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int SDLMouseClass::Get_Mouse_State() const
{
    return SDL_CursorVisible() ? 1 : -1;
}


/***********************************************************************************************
 * SDLMouseClass::Draw_Mouse -- Manually draw the mouse to the surface specified.               *
 *                                                                                             *
 *    This is a kludge function that can be used to reduce mouse flicker. Normally the mouse   *
 *    must be hidden before an image is copied to the visible surface. By drawing the mouse    *
 *    in the correct position on the source image prior to the copy, the mouse doesn't need    *
 *    to be hidden and no mouse flicker occurs. This routine handles this manual draw process. *
 *                                                                                             *
 * INPUT:   surface  -- Pointer to the surface that the mouse will be drawn to.                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The destination surface is presumed to be the exact dimensions of the bouding   *
 *             rectangle specified in the mouse constructor. The call to Erase_Mouse must      *
 *             occur as soon as possible since the mouse is frozen until it is called.         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Draw_Mouse(Surface*, bool)
{

}


/***********************************************************************************************
 * SDLMouseClass::Erase_Mouse -- Restores the surface after a Draw_Mouse call.                  *
 *                                                                                             *
 *    This is the counterpart routine to Draw_Mouse. It will restore the specified surface     *
 *    back to its original state. The mouse is frozen between the calls to Draw_Mouse and      *
 *    Erase_Mouse, so it is imperative to call this routine at the first legal opportunity.    *
 *                                                                                             *
 * INPUT:   surface  -- Pointer to the surface that the mouse was previously drawn to.         *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Erase_Mouse(Surface*, bool)
{

}


/***********************************************************************************************
 * SDLMouseClass::Convert_Coordinate -- Convert an O/S coordinate into a logical coordinate.    *
 *                                                                                             *
 *    Sometimes you come across system mouse coordinates and they need to be converted into    *
 *    game logical coordinates. This routine will perform this function.                       *
 *                                                                                             *
 * INPUT:   x,y   -- Reference to the coordinates that will be converted into game logical     *
 *                   coordinates.                                                              *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The coordinates will be bound as well as transformed by the confining rectangle.*
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Convert_Coordinate(int& x, int& y) const
{

}


/***********************************************************************************************
 * SDLMouseClass::Update_Mouse_Position -- Updates the mouse position to match that specified.  *
 *                                                                                             *
 *    This routine will update the mouse to match the position speicifed.                      *
 *                                                                                             *
 * INPUT:   x,y   -- The coordinates to position the mouse (hot spot). The coordinates are     *
 *                   specified as logical not O/S relative.                                    *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/10/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void SDLMouseClass::Update_Mouse_Position(int x, int y, bool forced)
{
    /*
    **  If the desired position is not the same as the current
    **  position, then hide the mouse, reposition it, then show
    **  the mouse.
    */
    if (x != MouseX || y != MouseY || forced) {
        MouseX = x;
        MouseY = y;
    }
}


void SDLMouseClass::Delete_Cursor_Image()
{
    while (!CursorSurfaces.empty()) {
        SDL_DestroySurface(*CursorSurfaces.begin());
        CursorSurfaces.erase(CursorSurfaces.begin());
    }
}


void SDLMouseClass::Convert_Cursor_Image(ShapeSet const* shapes)
{
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

    for (int i = 0; i < shapes->Get_Count(); i++) {
        // Full image dimensions
        int width = shapes->Get_Width();
        int height = shapes->Get_Height();

        // Non-empty area (cropped data region)
        Rect r = shapes->Get_Rect(i);

        // Create 8-bit surface for the shape
        SDL_Surface* source = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_INDEX8);
        SDL_SetSurfacePalette(source, palette);
        SDL_SetSurfaceColorKey(source, true, 0);

        uint8_t* dst = static_cast<uint8_t*>(source->pixels);
        const uint8_t* src = static_cast<const uint8_t*>(shapes->Get_Data(i));

        // Copy cropped data into the correct offset in the full surface
        for (int y = 0; y < r.Height; ++y) {
            uint8_t* dst_row = dst + (r.Y + y) * source->pitch + r.X;
            const uint8_t* src_row = src + y * r.Width;
            memcpy(dst_row, src_row, r.Width);
        }

        // Now create ARGB destination with correct scaling
        SDL_Surface* destination = SDL_CreateSurface(width * Get_Cursor_XScale(), height * Get_Cursor_YScale(), SDL_PIXELFORMAT_ARGB8888);

        // Use pixel-art scaling for crisp edges
        SDL_BlitSurfaceScaled(source, nullptr, destination, nullptr, SDL_SCALEMODE_PIXELART);

        CursorSurfaces.emplace_back(destination);
        SDL_DestroySurface(source);
    }
}

void SDLMouseClass::Clear_Cursor()
{
    SDL_SetCursor(nullptr);
    if (Cursor != nullptr) {
        SDL_DestroyCursor(Cursor);
        Cursor = nullptr;
    }
}

void SDLMouseClass::Set_System_Cursor()
{
    Clear_Cursor();
    Cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    SDL_SetCursor(Cursor);
}


int SDLMouseClass::Get_Cursor_XScale()
{
    if (OptionsExtension->CursorScale < 0) {
        return 1;
    }

    if (OptionsExtension->CursorScale > 0) {
        return OptionsExtension->CursorScale;
    }

    return static_cast<int>(std::round(1.0 / SDL_XScale()));
}


int SDLMouseClass::Get_Cursor_YScale()
{
    if (OptionsExtension->CursorScale < 0) {
        return 1;
    }

    if (OptionsExtension->CursorScale > 0) {
        return OptionsExtension->CursorScale;
    }

    return static_cast<int>(std::round(1.0 / SDL_YScale()));
}


