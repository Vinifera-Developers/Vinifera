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


struct BitmapInfo
{
    BITMAPINFOHEADER Header;
    DWORD Masks[3];
};


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
    DSurface(), // use the default constructor so that we don't initialize the DDraw portions of the surface
    SDLSurfacePtr(nullptr),
    Pitch(0),
    GDIDC(nullptr),
    GDIBitmap(nullptr),
    GDIBuffer(nullptr)
{
    /**
     *  If this is our first surface, fetch the pixel format.
     */
    if (!PixelFormat) {
        PixelFormat = SDL_GetPixelFormatDetails(SDL_PIXELFORMAT_RGB565);
        if (!PixelFormat) {
            DEBUG_ERROR("Failed to get pixel format details for RGB565.\n");
            return;
        }
    }

    /**
     *  Create persistent memory DC and DIB section.
     */
    GDIDC = CreateCompatibleDC(nullptr);
    if (!GDIDC) {
        DEBUG_ERROR("CreateCompatibleDC failed\n");
        return;
    }

    BitmapInfo bmi = {};
    bmi.Header.biSize = sizeof(BITMAPINFOHEADER);
    bmi.Header.biWidth = width;
    bmi.Header.biHeight = -height;
    bmi.Header.biPlanes = 1;
    bmi.Header.biBitCount = PixelFormat->bits_per_pixel;
    bmi.Header.biCompression = BI_BITFIELDS;
    bmi.Masks[0] = PixelFormat->Rmask;
    bmi.Masks[1] = PixelFormat->Gmask;
    bmi.Masks[2] = PixelFormat->Bmask;

    /**
     *  Create DIB section (let GDI allocate memory).
     */
    GDIBitmap = CreateDIBSection(GDIDC, (BITMAPINFO*)&bmi, DIB_RGB_COLORS, &GDIBuffer, nullptr, 0);
    if (!GDIBitmap || !GDIBuffer) {
        DEBUG_ERROR("CreateDIBSection failed\n");
        DeleteDC(GDIDC);
        GDIDC = nullptr;
        return;
    }

    SelectObject(GDIDC, GDIBitmap);

    DIBSECTION ds = {};
    GetObject(GDIBitmap, sizeof(ds), &ds);
    Pitch = ds.dsBm.bmWidthBytes;

    /**
     *  Create an SDL surface wrapping GDIBuffer.
     */
    SDLSurfacePtr = SDL_CreateSurfaceFrom(width, height, SDL_PIXELFORMAT_RGB565, GDIBuffer, Pitch);
    if (SDLSurfacePtr == nullptr) {
        DEBUG_ERROR("SurfacePtr could not be created! SDL Error: %s\n", SDL_GetError());
        return;
    }

    /**
     *  Set surface properties.
     */
    BytesPerPixel = PixelFormat->bytes_per_pixel;
    Width = SDLSurfacePtr->w;
    Height = SDLSurfacePtr->h;
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
    DSurface(),
    SDLSurfacePtr(nullptr),
    Pitch(0),
    GDIDC(nullptr),
    GDIBitmap(nullptr),
    GDIBuffer(nullptr)
{

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
    if (SDLSurfacePtr != nullptr) {
        SDL_DestroySurface(SDLSurfacePtr);
        SDLSurfacePtr = nullptr;
    }
    if (GDIBitmap) {
        DeleteObject(GDIBitmap);
        GDIBitmap = nullptr;
    }
    if (GDIDC) {
        DeleteDC(GDIDC);
        GDIDC = nullptr;
    }
}


/**
 *  Calculate bit shifts to properly extract channel data.
 */
static void Calculate_Mask_Info(unsigned int mask, unsigned int& right, unsigned int& left)
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
SDLSurface* SDLSurface::Create_Primary(void*)
{
    DEBUG_INFO("SDLSurface::Create_Primary\n");

    AllowStretchBlits = false;
    AllowHWFill = false;

    DEBUG_INFO("SDLSurface::Create_Primary - Creating surface\n");
    SDLSurface* surface = new SDLSurface(Options.ScreenWidth, Options.ScreenHeight);
    surface->IsPrimary = true;

    /**
     *  If this is a hicolor surface, then build the shift values for
     *  building and extracting the colors from the hicolor pixel.
     */
    if (surface->Bytes_Per_Pixel() == 2) {
        Calculate_Mask_Info(PixelFormat->Rmask, RedRight, RedLeft);
        Calculate_Mask_Info(PixelFormat->Gmask, GreenRight, GreenLeft);
        Calculate_Mask_Info(PixelFormat->Bmask, BlueRight, BlueLeft);

        PrimaryColorMode = COLORMODE_INVALID;

        /**
         *  Create the halfbright mask.
         */
        HalfbrightMask = static_cast<unsigned short>(Build_Hicolor_Pixel(127, 127, 127));
        QuarterbrightMask = static_cast<unsigned short>(Build_Hicolor_Pixel(63, 63, 63));
        EighthbrightMask = static_cast<unsigned short>(Build_Hicolor_Pixel(31, 31, 31));

        if (BlueRight == 0 && BlueLeft == 3 && GreenRight == 5 && GreenLeft == 3 && RedRight == 10 && RedLeft == 3) {
            PrimaryColorMode = COLORMODE_555;
        } else if (BlueRight == 0 && BlueLeft == 2 && GreenRight == 6 && GreenLeft == 3 && RedRight == 11 && RedLeft == 3) {
            PrimaryColorMode = COLORMODE_556;
        } else if (BlueRight == 0 && BlueLeft == 3 && GreenRight == 5 && GreenLeft == 2 && RedRight == 11 && RedLeft == 3) {
            PrimaryColorMode = COLORMODE_565;
        } else if (BlueRight == 0 && BlueLeft == 3 && GreenRight == 5 && GreenLeft == 3 && RedRight == 11 && RedLeft == 2) {
            PrimaryColorMode = COLORMODE_655;
        }
    }
    DEBUG_INFO("SDLSurface::Create_Primary done\n");

    return surface;
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
    if (GDIDC == nullptr) {
        return nullptr;
    }

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

    return 1;
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
    return Pitch;
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
        if (SDL_MUSTLOCK(SDLSurfacePtr)) {
            if (!SDL_LockSurface(SDLSurfacePtr)) {
                return nullptr; // failed to lock
            }
        }
        BytesPerPixel = SDL_GetPixelFormatDetails(SDLSurfacePtr->format)->bytes_per_pixel;
        LockPtr = SDLSurfacePtr->pixels;
    }
    XSurface::Lock();
    return static_cast<char*>(LockPtr) + point.Y * Stride() + point.X * Bytes_Per_Pixel();
}


bool SDLSurface::Can_Lock(int x, int y) const
{
    return SDLSurfacePtr != nullptr;
}


bool SDLSurface::Can_Blit() const
{
    return SDLSurfacePtr != nullptr;
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
            if (SDL_MUSTLOCK(SDLSurfacePtr)) {
                SDL_UnlockSurface(SDLSurfacePtr);
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
}
