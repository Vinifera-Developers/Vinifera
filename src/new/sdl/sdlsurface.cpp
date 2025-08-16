/*
**	Command & Conquer Generals(tm)
**	Copyright 2025 Electronic Arts Inc.
**
**	This program is free software: you can redistribute it and/or modify
**	it under the terms of the GNU General Public License as published by
**	the Free Software Foundation, either version 3 of the License, or
**	(at your option) any later version.
**
**	This program is distributed in the hope that it will be useful,
**	but WITHOUT ANY WARRANTY; without even the implied warranty of
**	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**	GNU General Public License for more details.
**
**	You should have received a copy of the GNU General Public License
**	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /G/wwlib/SDLSurface.cpp                                       $*
 *                                                                                             *
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/23/00 2:26p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 *   SDLSurface::Blit_From -- Blit from one surface to this one.                                 *
 *   SDLSurface::Blit_From -- Blit graphic memory from one rectangle to another.                 *
 *   SDLSurface::Build_Hicolor_Pixel -- Construct a hicolor pixel according to the surface pixel *
 *   SDLSurface::Build_Remap_Table -- Build a highcolor remap table.                             *
 *   SDLSurface::Bytes_Per_Pixel -- Fetches the bytes per pixel of the surface.                  *
 *   SDLSurface::Create_Primary -- Creates a primary (visible) surface.                          *
 *   SDLSurface::SDLSurface -- Create a surface attached to specified DDraw Surface Object.        *
 *   SDLSurface::SDLSurface -- Default constructor for surface object.                             *
 *   SDLSurface::SDLSurface -- Off screen direct draw surface constructor.                         *
 *   SDLSurface::Fill_Rect -- Fills a rectangle with clipping control.                           *
 *   SDLSurface::Fill_Rect -- This routine will fill the specified rectangle.                    *
 *   SDLSurface::Lock -- Fetches a working pointer into surface memory.                          *
 *   SDLSurface::Restore_Check -- Checks for and restores surface memory if necessary.           *
 *   SDLSurface::Stride -- Fetches the bytes between rows.                                       *
 *   SDLSurface::Unlock -- Unlock a previously locked surface.                                   *
 *   SDLSurface::~SDLSurface -- Destructor for a direct draw surface object.                       *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#include "always.h"
#include "sdlsurface.h"

#include <algorithm>

#include "debughandler.h"
#include "dsurface.h"
#include "options.h"
#include "sdl_init.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "SDL3/SDL_oldnames.h"


/*
**
*/
const SDL_PixelFormatDetails* SDLSurface::PixelFormat = nullptr;


/***********************************************************************************************
 * SDLSurface::SDLSurface -- Off screen direct draw surface constructor.                           *
 *                                                                                             *
 *    This constructor will create a Direct Draw enabled surface in video memory if possible.  *
 *    Such a surface will be able to use hardware assist if possible. The surface created      *
 *    is NOT visible. It only exists as a work surface and cannot be flipped to the visible    *
 *    surface. It can only be blitted to the visible surface.                                  *
 *                                                                                             *
 * INPUT:   width    -- The width of the surface to create.                                    *
 *                                                                                             *
 *          height   -- The height of the surface to create.                                   *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The surface pixel format is the same as that of the visible display mode. It    *
 *             is important to construct surfaces using this routine, only AFTER the display   *
 *             mode has been set.                                                              *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
SDLSurface::SDLSurface(int width, int height) :
    XSurface(width, height),
    BytesPerPixel(0),
    LockPtr(nullptr),
    IsPrimary(false),
    SurfacePtr(nullptr)
{
    SurfacePtr = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGB565);
    if (SurfacePtr == nullptr) {
        DEBUG_ERROR("SurfacePtr could not be created! SDL Error: %s\n", SDL_GetError());
        return;
    }

    BytesPerPixel = SDL_GetPixelFormatDetails(SurfacePtr->format)->bytes_per_pixel;
    Width = SurfacePtr->w;
    Height = SurfacePtr->h;
}


/***********************************************************************************************
 * SDLSurface::~SDLSurface -- Destructor for a direct draw surface object.                     *
 *                                                                                             *
 *    This will destruct (make invalid) the direct draw surface.                               *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
SDLSurface::~SDLSurface()
{
    if (SurfacePtr != nullptr) {
        SDL_DestroySurface(SurfacePtr);
    }
    SurfacePtr = nullptr;
}


/***********************************************************************************************
 * SDLSurface::SDLSurface -- Default constructor for surface object.                           *
 *                                                                                             *
 *    This default constructor for a surface object should not be used. Although it properly   *
 *    creates a non-functional surface, there is no use for such a surface. This default       *
 *    constructor is provided for those rare cases where semantics require a default           *
 *    constructor.                                                                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
SDLSurface::SDLSurface() :
    BytesPerPixel(0),
    LockPtr(nullptr),
    IsPrimary(false),
    SurfacePtr(nullptr)
{

}


/**
 *  Calculate bit shifts to properly extract channel data.
 */
static void Calculate_Mask_Info(unsigned int mask, int& right, int& left)
{
    /**
     *  Figure out how far to shift bits to the left.
     */
    for (int index = 0; index < 16; index++) {
        if (mask & 0x01) break;
        mask >>= 1;
        right++;
    }

    /**
     *  Figure out how far to shift bits to the right.
     */
    for (int index = 0; index < 8; index++) {
        if (mask & 0x80) break;
        mask <<= 1;
        left++;
    }
}


/***********************************************************************************************
 * SDLSurface::Create_Primary -- Creates a primary (visible) surface.                            *
 *                                                                                             *
 *    This routine is used to create the surface object that represents the currently          *
 *    visible display. The surface is not allocated, it is merely linked to the preexisting    *
 *    surface that the Windows GDI is also currently using.                                    *
 *                                                                                             *
 * INPUT:   backsurface -- Optional pointer to specify where the backpage (flip enabled)       *
 *                         pointer will be placed. If this parameter is NULL, then no          *
 *                         back surface will be created.                                       *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the primary surface.                                     *
 *                                                                                             *
 * WARNINGS:   There can be only one primary surface. If an additional call to this routine    *
 *             is made, another surface pointer will be returned, but it will point to the     *
 *             same surface as before.                                                         *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
SDLSurface* SDLSurface::Create_Primary(SDLSurface** backsurface1)
{
    DEBUG_INFO("SDLSurface::Create_Primary\n");

    DSurface::AllowStretchBlits = false;
    DSurface::AllowHWFill = false;

    DEBUG_INFO("SDLSurface::Create_Primary - Creating surface\n");
    SDLSurface* surface = new SDLSurface(Options.ScreenWidth, Options.ScreenHeight);

    surface->IsPrimary = true;

    /**
     *  Fetch the pixel format for the surface.
     */
    PixelFormat = SDL_GetPixelFormatDetails(surface->SurfacePtr->format);

    /**
     *  If this is a hicolor surface, then build the shift values for
     *  building and extracting the colors from the hicolor pixel.
     */
    if (surface->Bytes_Per_Pixel() == 2) {
        Calculate_Mask_Info(PixelFormat->Rmask, DSurface::RedRight, DSurface::RedLeft);
        Calculate_Mask_Info(PixelFormat->Gmask, DSurface::GreenRight, DSurface::GreenLeft);
        Calculate_Mask_Info(PixelFormat->Bmask, DSurface::BlueRight, DSurface::BlueLeft);

        DSurface::PrimaryColorMode = COLORMODE_INVALID;

        /**
         *  Create the halfbright mask.
         */
        DSurface::HalfbrightMask = static_cast<unsigned short>(DSurface::Build_Hicolor_Pixel(127, 127, 127));
        DSurface::QuarterbrightMask = static_cast<unsigned short>(DSurface::Build_Hicolor_Pixel(63, 63, 63));
        DSurface::EighthbrightMask = static_cast<unsigned short>(DSurface::Build_Hicolor_Pixel(31, 31, 31));

        if (DSurface::BlueRight == 0 && DSurface::BlueLeft == 3 && DSurface::GreenRight == 5 && DSurface::GreenLeft == 3 && DSurface::RedRight == 10 && DSurface::RedLeft == 3) {
            DSurface::PrimaryColorMode = COLORMODE_555;
        } else if (DSurface::BlueRight == 0 && DSurface::BlueLeft == 2 && DSurface::GreenRight == 6 && DSurface::GreenLeft == 3 && DSurface::RedRight == 11 && DSurface::RedLeft == 3) {
            DSurface::PrimaryColorMode = COLORMODE_556;
        } else if (DSurface::BlueRight == 0 && DSurface::BlueLeft == 3 && DSurface::GreenRight == 5 && DSurface::GreenLeft == 2 && DSurface::RedRight == 11 && DSurface::RedLeft == 3) {
            DSurface::PrimaryColorMode = COLORMODE_565;
        } else if (DSurface::BlueRight == 0 && DSurface::BlueLeft == 3 && DSurface::GreenRight == 5 && DSurface::GreenLeft == 3 && DSurface::RedRight == 11 && DSurface::RedLeft == 2) {
            DSurface::PrimaryColorMode = COLORMODE_655;
        }
    }
    DEBUG_INFO("SDLSurface::Create_Primary done\n");

    return surface;
}


/***********************************************************************************************
 * SDLSurface::SDLSurface -- Create a surface attached to specified DDraw Surface Object.          *
 *                                                                                             *
 *    If an existing Direct Draw Surface Object is available, use this constructor to create   *
 *    a SDLSurface object that is attached to the surface specified.                             *
 *                                                                                             *
 * INPUT:   surfaceptr  -- Pointer to a preexisting Direct Draw Surface Object.                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
SDLSurface::SDLSurface(SDL_Surface* surfaceptr) :
    BytesPerPixel(0),
    LockPtr(nullptr),
    IsPrimary(false),
    SurfacePtr(surfaceptr)
{
    if (SurfacePtr != nullptr) {
        BytesPerPixel = SDL_GetPixelFormatDetails(SurfacePtr->format)->bytes_per_pixel;
        Width = SurfacePtr->w;
        Height = SurfacePtr->h;
    }
}


/***********************************************************************************************
 * SDLSurface::GetDC -- Get the windows device context from our surface                          *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS: Any current locks will get unlocked while the DC is held                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/21/2000 NAK : Created.                                                                 *
 *=============================================================================================*/
HDC SDLSurface::GetDC()
{
    if (GDIDC) { // Already created — just bump lock count
        LockCount++;
        return GDIDC;
    }

    // Lock SDL surface to ensure we have access to pixel buffer
    if (!SDL_LockSurface(SurfacePtr)) {
        return nullptr;
    }

    // Use SDL's pixel buffer directly
    GDIBuffer = SurfacePtr->pixels;

    GDIDC = CreateCompatibleDC(nullptr);

    BITMAPINFO bmi = {};
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = SurfacePtr->w;
    bmi.bmiHeader.biHeight = -SurfacePtr->h; // top-down
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 16;
    bmi.bmiHeader.biCompression = BI_BITFIELDS;
    *reinterpret_cast<DWORD*>(&bmi.bmiColors[0]) = PixelFormat->Rmask;
    *reinterpret_cast<DWORD*>(&bmi.bmiColors[1]) = PixelFormat->Gmask;
    *reinterpret_cast<DWORD*>(&bmi.bmiColors[2]) = PixelFormat->Bmask;

    // Create a DIB section that *uses* SDL's pixel memory
    GDIBitmap = CreateDIBSection(GDIDC, &bmi, DIB_RGB_COLORS, &GDIBuffer, nullptr, 0);

    if (!GDIBitmap) {
        DeleteDC(GDIDC);
        GDIDC = nullptr;
        SDL_UnlockSurface(SurfacePtr);
        GDIBuffer = nullptr;
        return nullptr;
    }

    SelectObject(GDIDC, GDIBitmap);

    LockCount++;
    return GDIDC;
}


/***********************************************************************************************
 * SDLSurface::ReleaseDC -- Release the windows device context from our surface                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS: Restores any locks held before the call to GetDC()                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   06/21/2000 NAK : Created.                                                                 *
 *=============================================================================================*/
int SDLSurface::ReleaseDC(HDC hdc)
{
    if (!GDIDC || hdc != GDIDC) {
        return 0;
    }

    // Unlock SDL surface (GDI has already written into shared pixels)
    if (LockCount > 0) {
        LockCount--;
    }

    // Cleanup
    DeleteObject(GDIBitmap);
    GDIBitmap = nullptr;

    DeleteDC(GDIDC);
    GDIDC = nullptr;

    SDL_UnlockSurface(SurfacePtr);
    GDIBuffer = nullptr;

    return 1;
}


/***********************************************************************************************
 * SDLSurface::Bytes_Per_Pixel -- Fetches the bytes per pixel of the surface.                    *
 *                                                                                             *
 *    This routine will return with the number of bytes that each pixel consumes. The value    *
 *    is dependant upon the graphic mode of the display.                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the bytes per pixel of the surface object.                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int SDLSurface::Bytes_Per_Pixel() const
{
    return BytesPerPixel;
}


/***********************************************************************************************
 * SDLSurface::Stride -- Fetches the bytes between rows.                                         *
 *                                                                                             *
 *    This routine will return the number of bytes to add so that the pointer will be          *
 *    positioned at the same column, but one row down the screen. This value may very well     *
 *    NOT be equal to the width multiplied by the bytes per pixel.                             *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  Returns with the byte difference between subsequent pixel rows.                    *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
int SDLSurface::Stride() const
{
    return SurfacePtr ? SurfacePtr->pitch : 0;
}


/***********************************************************************************************
 * SDLSurface::Lock -- Fetches a working pointer into surface memory.                            *
 *                                                                                             *
 *    This routine will return with a pointer to the pixel at the location specified. In order *
 *    to directly manipulate surface memory, the surface memory must be mapped into the        *
 *    program's logical address space. In addition, all blitter activity on the surface will   *
 *    be suspended. Every call to Lock must be have a corresponding call to Unlock if the      *
 *    pointer returned is not equal to NULL.                                                   *
 *                                                                                             *
 * INPUT:   point -- Pixel coordinate to return a pointer to.                                  *
 *                                                                                             *
 * OUTPUT:  Returns with a pointer to the pixel specified. If the return value is NULL, then   *
 *          the surface could not be locked and no call to Unlock should be performed.         *
 *                                                                                             *
 * WARNINGS:   It is important not to keep a surface locked indefinately since the blitter     *
 *             will not be able to function. Due to the time that locking consumes, it is      *
 *             also important to not perform unnecessarily frequent Lock calls.                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
void* SDLSurface::Lock(Point2D point) const
{
    if (point.X < 0 || point.Y < 0) return nullptr;

    if (LockCount == 0) {
        if (SDL_MUSTLOCK(SurfacePtr)) {
            if (!SDL_LockSurface(SurfacePtr)) {
                return nullptr; // failed to lock
            }
        }
        BytesPerPixel = SDL_GetPixelFormatDetails(SurfacePtr->format)->bytes_per_pixel;
        LockPtr = SurfacePtr->pixels;
    }
    XSurface::Lock();
    return static_cast<char*>(LockPtr) + point.Y * Stride() + point.X * Bytes_Per_Pixel();
}


bool SDLSurface::Can_Lock(int x, int y) const
{
    return SurfacePtr != nullptr;
}


bool SDLSurface::Can_Blit() const
{
    return SurfacePtr != nullptr;
}


/***********************************************************************************************
 * SDLSurface::Unlock -- Unlock a previously locked surface.                                     *
 *                                                                                             *
 *    After a surface has been successfully locked, a call to the Unlock() function is         *
 *    required.                                                                                *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; Was the unlock successful?                                                   *
 *                                                                                             *
 * WARNINGS:   Only pair a call to Unlock if the prior Lock actually returned a non-NULL       *
 *             value.                                                                          *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SDLSurface::Unlock() const
{
    if (LockCount > 0) {
        XSurface::Unlock();
        if (LockCount == 0) {
            if (SDL_MUSTLOCK(SurfacePtr)) {
                SDL_UnlockSurface(SurfacePtr);
            }
            LockPtr = nullptr;
        }
        return true;
    }
    return false;
}


/***********************************************************************************************
 * SDLSurface::Restore_Check -- Checks for and restores surface memory if necessary.             *
 *                                                                                             *
 *    This routine will check to see if surface memory has been lost to the surface. If it     *
 *    has, then the surface memory will be restored.                                           *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SDLSurface::Restore_Check() const
{
    return true;
}


void SDLSurface::Blit_To_Window(Rect const* region) const
{
    return;
    if (/*!IsPrimary ||*/ !SDLWindow || !SurfacePtr) return;

    //SDL_Surface* win_surf = SDL_GetWindowSurface(SDLWindow);
    //if (win_surf) {
    //    SDL_Rect dst;
    //    dst.x = 0;
    //    dst.y = 0;
    //    dst.w = Get_Width();
    //    dst.h = Get_Height();

    //    // If VisibleSurface is a wrapper around SDL_Surface, just copy pixels
    //    SDL_BlitSurface(Get_SDL_Surface(), nullptr, win_surf, &dst);

    //    // Present the new framebuffer
    //    SDL_UpdateWindowSurface(SDLWindow);
    //}
    //return;


    SDL_Rect sdl_rect;
    int w = SurfacePtr->w;
    int h = SurfacePtr->h;

    if (region) {
        sdl_rect.x = region->X;
        sdl_rect.y = region->Y;
        sdl_rect.w = region->Width;
        sdl_rect.h = region->Height;
        w = region->Width;
        h = region->Height;
    } else {
        sdl_rect.x = sdl_rect.y = 0;
        sdl_rect.w = w;
        sdl_rect.h = h;
    }

    SDL_FRect sdl_frect = {
        static_cast<float>(sdl_rect.x),
        static_cast<float>(sdl_rect.y),
        static_cast<float>(sdl_rect.w),
        static_cast<float>(sdl_rect.h)};

    void* pixels;
    int pitch;
    SDL_LockTexture(SDLWindowTexture, &sdl_rect, &pixels, &pitch);

    SDL_ConvertPixels(w, h, SurfacePtr->format, (uint8_t*)SurfacePtr->pixels + sdl_rect.y * SurfacePtr->pitch + sdl_rect.x * Bytes_Per_Pixel(), SurfacePtr->pitch, SDLWindowTexture->format, pixels, pitch);

    SDL_UnlockTexture(SDLWindowTexture);

    SDL_RenderTexture(SDLWindowRenderer, SDLWindowTexture, &sdl_frect, &sdl_frect);
    SDL_RenderPresent(SDLWindowRenderer);

    DEBUG_INFO("Blit_To_Window: Blitted %dx%d pixels to window at (%d, %d)\n", sdl_rect.w, sdl_rect.h, sdl_rect.x, sdl_rect.y);
}


/***********************************************************************************************
 * SDLSurface::Blit_From -- Blit graphic memory from one rectangle to another.                   *
 *                                                                                             *
 *    This routine will use the blitter (if possible) to blit a block of graphic memory from   *
 *    one screen rectangle to another. If the rectangles do no match in size, scaling may      *
 *    be performed.                                                                            *
 *                                                                                             *
 * INPUT:   destrect -- The destination rectangle.                                             *
 *                                                                                             *
 *          ssource  -- The source surface to blit from.                                       *
 *                                                                                             *
 *          sourecrect  -- The source rectangle.                                               *
 *                                                                                             *
 *          trans    -- Should transparency checking be performed?                             *
 *                                                                                             *
 * OUTPUT:  bool; Was the blit performed without error?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SDLSurface::Blit_From(Rect const& destrect, Surface const& ssource, Rect const& sourcerect, bool trans, bool a5)
{
    return Blit_From(Get_Rect(), destrect, ssource, ssource.Get_Rect(), sourcerect, trans, a5);
}


/***********************************************************************************************
 * SDLSurface::Blit_From -- Blit from one surface to this one.                                   *
 *                                                                                             *
 *    Use this routine to blit a rectangle from the specified surface to this surface while    *
 *    performing clipping upon the blit rectangles specified.                                  *
 *                                                                                             *
 * INPUT:   dcliprect   -- The clipping rectangle to use for this surface.                     *
 *                                                                                             *
 *          destrect    -- The destination rectangle of the blit. The is relative to the       *
 *                         dcliprect parameter.                                                *
 *                                                                                             *
 *          ssource     -- The source surface of the blit.                                     *
 *                                                                                             *
 *          scliprect   -- The source clipping rectangle.                                      *
 *                                                                                             *
 *          sourcrect   -- The source rectangle of the blit. This rectangle is relative to     *
 *                         the source clipping rectangle.                                      *
 *                                                                                             *
 *          trans       -- Is this a transparent blit request?                                 *
 *                                                                                             *
 * OUTPUT:  bool; Was there a blit performed? A 'false' return value would indicate that the   *
 *                blit was clipped into nothing.                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SDLSurface::Blit_From(Rect const& dcliprect, Rect const& destrect, Surface const& ssource, Rect const& scliprect, Rect const& sourcerect, bool trans, bool a7)
{
    if (XSurface::Blit_From(dcliprect, destrect, ssource, scliprect, sourcerect, trans, a7)) {
        Blit_To_Window(&dcliprect);
        return true;
    }

    return false;
}


/***********************************************************************************************
 * SDLSurface::Fill_Rect -- This routine will fill the specified rectangle.                      *
 *                                                                                             *
 *    This routine will fill the specified rectangle with a color.                             *
 *                                                                                             *
 * INPUT:   fillrect -- The rectangle to fill.                                                 *
 *                                                                                             *
 *          color    -- The color to fill with.                                                *
 *                                                                                             *
 * OUTPUT:  bool; Was the fill performed without error?                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   02/07/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SDLSurface::Fill_Rect(Rect const& fillrect, int color)
{
    return SDLSurface::Fill_Rect(Get_Rect(), fillrect, color);
}


/***********************************************************************************************
 * SDLSurface::Fill_Rect -- Fills a rectangle with clipping control.                             *
 *                                                                                             *
 *    This routine will fill a rectangle on this surface, but will clip the request against    *
 *    a clipping rectangle first.                                                              *
 *                                                                                             *
 * INPUT:   cliprect -- The clipping rectangle to use for this surface.                        *
 *                                                                                             *
 *          fillrect -- The rectangle to fill with the specified color. The rectangle is       *
 *                      relative to the clipping rectangle.                                    *
 *                                                                                             *
 *          color    -- The color (surface dependant format) to use when filling the rectangle *
 *                      pixels.                                                                *
 *                                                                                             *
 * OUTPUT:  bool; Was a fill operation performed? A 'false' return value would mean that the   *
 *                fill request was clipped into nothing.                                       *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/27/1997 JLB : Created.                                                                 *
 *=============================================================================================*/
bool SDLSurface::Fill_Rect(Rect const& cliprect, Rect const& fillrect, int color)
{
    /**
     *  If the buffer is locked, then using the blitter to perform the fill is not possible.
     *  In such a case, perform a manual fill of the region.
     */
    if (XSurface::Fill_Rect(cliprect, fillrect, color)) {
        Blit_To_Window(&cliprect);
        return true;
    }

    return false;
}


bool SDLSurface::Fill_Rect_Trans(Rect const& rect, const RGBClass& color, int opacity)
{
    if (Bytes_Per_Pixel() < 2) {
        return false;
    }

    if (!rect.Is_Valid()) {
        return false;
    }

    Rect newrect = Intersect(Get_Rect(), rect);
    if (!newrect.Is_Valid()) {
        return false;
    }

    unsigned short r_mask = static_cast<unsigned short>(255u >> static_cast<unsigned short>(DSurface::RedLeft)) << static_cast<unsigned short>(DSurface::RedRight);
    unsigned short g_mask = static_cast<unsigned short>(255u >> static_cast<unsigned short>(DSurface::GreenLeft)) << static_cast<unsigned short>(DSurface::GreenRight);
    unsigned short b_mask = static_cast<unsigned short>(255u >> static_cast<unsigned short>(DSurface::BlueLeft)) << static_cast<unsigned short>(DSurface::BlueRight);

    unsigned short* ptr = static_cast<unsigned short*>(Lock(newrect.Top_Left()));
    if (ptr == nullptr) {
        return false;
    }

    opacity = std::min(opacity, 100);

    unsigned scale = opacity * 255 / 100;
    unsigned short delta = 255 - scale;

    unsigned short rgb = DSurface::Build_Hicolor_Pixel(color.Red, color.Green, color.Blue);

    for (int y = 0; y < newrect.Height; y++) {
        int pos = y * (Stride() / 2);

        for (int x = 0; x < newrect.Width; x++) {
            unsigned short* p = &ptr[pos];
            pos++;
            int c = *p;
            *p = scale * static_cast<unsigned short>(rgb & b_mask) + delta * (c & static_cast<unsigned int>(b_mask)) >> 8 | g_mask & scale * static_cast<unsigned short>(rgb & g_mask) + delta * (c & static_cast<unsigned int>(g_mask)) >> 8 | r_mask & scale * static_cast<unsigned short>(rgb & r_mask) + delta * (c & static_cast<unsigned int>(r_mask)) >> 8;
        }
    }

    Unlock();
    Blit_To_Window(&newrect);

    return true;
}


bool SDLSurface::Fill(int color)
{
    if (XSurface::Fill(color)) {
        SDL_Update_Screen(this);
        return true;
    }
    return false;
}

//bool SDLSurface::Draw_Ellipse(Point2D point, int radius_x, int radius_y, Rect clip, int color)
//{
//    if (XSurface::Draw_Ellipse(point, radius_x, radius_y, clip, color)) {
//        Blit_To_Window(&clip);
//        return true;
//    }
//    return false;
//}
//
//bool SDLSurface::Put_Pixel(Point2D const& point, int color)
//{
//    if (XSurface::Put_Pixel(point, color)) {
//        Blit_To_Window();
//        return true;
//    }
//    return false;
//}
//
//bool SDLSurface::Draw_Line(Point2D const& startpoint, Point2D const& endpoint, int color)
//{
//    if (XSurface::Draw_Line(startpoint, endpoint, color)) {
//        Blit_To_Window();
//        return true;
//    }
//    return false;
//}
//
//bool SDLSurface::Draw_Line(Rect const& cliprect, Point2D const& startpoint, Point2D const& endpoint, int color)
//{
//    if (XSurface::Draw_Line(cliprect, startpoint, endpoint, color)) {
//        Blit_To_Window(&cliprect);
//        return true;
//    }
//    return false;
//}
//
//bool SDLSurface::Draw_Line_entry_34(Rect const& cliprect, Point2D const& startpoint, Point2D const& endpoint, unsigned color, int a5, int a6, bool z_only)
//{
//    if (XSurface::Draw_Line_entry_34(cliprect, startpoint, endpoint, color, a5, a6, z_only)) {
//        Blit_To_Window(&cliprect);
//        return true;
//    }
//    return false;
//}
//
//bool SDLSurface::Draw_Line_entry_38(Rect const& cliprect, Point2D const& startpoint, Point2D const& endpoint, int a4, int a5, int a6, bool a7)
//{
//    if (XSurface::Draw_Line_entry_38(cliprect, startpoint, endpoint, a4, a5, a6, a7)) {
//        Blit_To_Window(&cliprect);
//        return true;
//    }
//    return false;
//}
//
//bool SDLSurface::Draw_Line_entry_3C(Rect const& cliprect, Point2D const& startpoint, Point2D const& endpoint, RGBClass& color, int a5, int a6, bool a7, bool a8, bool a9, bool a10, float a11)
//{
//    if (XSurface::Draw_Line_entry_3C(cliprect, startpoint, endpoint, color, a5, a6, a7, a8, a9, a10, a11)) {
//        Blit_To_Window(&cliprect);
//        return true;
//    }
//    return false;
//}
//
//bool SDLSurface::Plot_Line(Rect& area, Point2D& start, Point2D& end, void (*drawer_callback)(Point2D&))
//{
//    if (XSurface::Plot_Line(area, start, end, drawer_callback)) {
//        Blit_To_Window(&area);
//        return true;
//    }
//    return false;
//}
//
//int SDLSurface::Draw_Dashed_Line(Point2D& start, Point2D& end, unsigned color, bool pattern[], int offset)
//{
//    int ret = XSurface::Draw_Dashed_Line(start, end, color, pattern, offset);
//    Blit_To_Window();
//    return ret;
//}
//
//int SDLSurface::entry_48(Point2D& start, Point2D& end, unsigned color, bool pattern[], int offset, bool a6)
//{
//    int ret = XSurface::entry_48(start, end, color, pattern, offset, a6);
//    Blit_To_Window();
//    return ret;
//}
//
//bool SDLSurface::entry_4C(Point2D& start, Point2D& end, unsigned a4, bool a5)
//{
//    if (XSurface::entry_4C(start, end, a4, a5)) {
//        Blit_To_Window();
//        return true;
//    }
//    return false;
//}
//
//bool SDLSurface::Draw_Rect(Rect const& rect, int color)
//{
//    if (XSurface::Draw_Rect(rect, color)) {
//        Blit_To_Window(&rect);
//        return true;
//    }
//    return false;
//}
//
//bool SDLSurface::Draw_Rect(Rect const& cliprect, Rect const& rect, int color)
//{
//    if (XSurface::Draw_Rect(cliprect, rect, color)) {
//        Blit_To_Window(&rect);
//        return true;
//    }
//    return false;
//}
//
//bool SDLSurface::entry_84(Point2D const& point, int color, Rect const& rect)
//{
//    if (XSurface::entry_84(point, color, rect)) {
//        Blit_To_Window(&rect);
//        return true;
//    }
//    return false;
//}
