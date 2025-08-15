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
#include "tibsun_functions.h"
#include "tibsun_globals.h"
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
SDLSurface::SDLSurface(int width, int height) : XSurface(width, height), BytesPerPixel(0), LockPtr(nullptr), IsPrimary(false), SurfacePtr(nullptr)
{
    SurfacePtr = SDL_CreateSurface(width, height, SDL_PIXELFORMAT_RGB565);
    if (SurfacePtr == nullptr) {
        DEBUG_ERROR("VideoSurface could not be created! SDL Error: %s\n", SDL_GetError());
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
SDLSurface::SDLSurface() : BytesPerPixel(0), LockPtr(nullptr), IsPrimary(false), SurfacePtr(nullptr)
{

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

    /*
    **	Setup parameter for creating the primary surface. This will
    **	always be the visible surface plus optional back buffers of identical
    **	dimensions.
    */
    //surface->Description->dwFlags = DDSD_CAPS;
    //surface->Description->ddsCaps.dwCaps = DDSCAPS_PRIMARYSURFACE;
    //if (backcount > 0) {
    //    surface->Description->ddsCaps.dwCaps |= DDSCAPS_FLIP | DDSCAPS_COMPLEX;
    //    surface->Description->dwFlags |= DDSD_BACKBUFFERCOUNT;
    //    surface->Description->dwBackBufferCount = backcount;
    //}

    DEBUG_INFO("SDLSurface::Create_Primary - Creating surface\n");
    SDLSurface* surface = new SDLSurface(Options.ScreenWidth, Options.ScreenHeight);

    /*
    **	If the primary surface object was created, then fetch a pointer to the
    **	back buffer if there is one present.
    */
    DEBUG_INFO("CreateSurface OK\n");

    ///*
    //**	Get a description of the surface that was just allocated.
    //*/
    //memset(surface->Description, '\0', sizeof(DSDLSurfaceDESC));
    //surface->Description->dwSize = sizeof(DSDLSurfaceDESC);

    //result = surface->SurfacePtr->GetSurfaceDesc(surface->Description);
    //if (result != DD_OK) {
    //    DebugString("Failed to get description of primary surface\n");
    //}

    //surface->BytesPerPixel = (surface->Description->ddpfPixelFormat.dwRGBBitCount + 7) / 8;
    //surface->IsVideoRam = (surface->Description->ddsCaps.dwCaps & DDSCAPS_VIDEOMEMORY) != 0;
    surface->IsPrimary = true;

    ////		surface->Window.Set(Rect(0, 0, surface->Description->dwWidth, surface->Description->dwHeight));
    //surface->Width = surface->Description->dwWidth;
    //surface->Height = surface->Description->dwHeight;
    //PaletteSurface = surface->SurfacePtr;

    /*
    **	Fetch the pixel format for the surface.
    */
    PixelFormat = SDL_GetPixelFormatDetails(surface->SurfacePtr->format);

    /*
    **	If this is a hicolor surface, then build the shift values for
    **	building and extracting the colors from the hicolor pixel.
    */
    if (surface->Bytes_Per_Pixel() == 2) {
        int index;
        int shift = PixelFormat->Rmask;
        DSurface::RedRight = 0;
        DSurface::RedLeft = 0;
        for (index = 0; index < 16; index++) {
            if (shift & 0x01) break;
            shift >>= 1;
            DSurface::RedRight++;
        }
        for (index = 0; index < 8; index++) {
            if (shift & 0x80) break;
            shift <<= 1;
            DSurface::RedLeft++;
        }

        shift = PixelFormat->Gmask;
        DSurface::GreenRight = 0;
        DSurface::GreenLeft = 0;
        for (index = 0; index < 16; index++) {
            if (shift & 0x01) break;
            DSurface::GreenRight++;
            shift >>= 1;
        }
        for (index = 0; index < 8; index++) {
            if (shift & 0x80) break;
            DSurface::GreenLeft++;
            shift <<= 1;
        }

        shift = PixelFormat->Bmask;
        DSurface::BlueRight = 0;
        DSurface::BlueLeft = 0;
        for (index = 0; index < 16; index++) {
            if (shift & 0x01) break;
            DSurface::BlueRight++;
            shift >>= 1;
        }
        for (index = 0; index < 8; index++) {
            if (shift & 0x80) break;
            DSurface::BlueLeft++;
            shift <<= 1;
        }

        DSurface::PrimaryColorMode = COLORMODE_INVALID;

        /*
        **	Create the halfbright mask.
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
SDLSurface::SDLSurface(SDL_Surface* surfaceptr) : BytesPerPixel(0), LockPtr(nullptr), IsPrimary(false), SurfacePtr(surfaceptr)
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
    if (gdi_dc) {
        // Already created — just bump lock count
        LockCount++;
        return gdi_dc;
    }

    // Lock SDL surface to ensure we have access to pixel buffer
    if (!SDL_LockSurface(SurfacePtr)) {
        return nullptr;
    }

    // Use SDL's pixel buffer directly
    gdi_pixels = SurfacePtr->pixels;

    gdi_dc = CreateCompatibleDC(nullptr);

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
    gdi_bitmap = CreateDIBSection(gdi_dc, &bmi, DIB_RGB_COLORS, &gdi_pixels, nullptr, 0);

    if (!gdi_bitmap) {
        DeleteDC(gdi_dc);
        gdi_dc = nullptr;
        SDL_UnlockSurface(SurfacePtr);
        gdi_pixels = nullptr;
        return nullptr;
    }

    SelectObject(gdi_dc, gdi_bitmap);

    LockCount++;
    return gdi_dc;
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
    if (!gdi_dc || hdc != gdi_dc) {
        return 0;
    }

    // Unlock SDL surface (GDI has already written into shared pixels)
    if (LockCount > 0) {
        LockCount--;
    }

    // Cleanup
    DeleteObject(gdi_bitmap);
    gdi_bitmap = nullptr;

    DeleteDC(gdi_dc);
    gdi_dc = nullptr;

    SDL_UnlockSurface(SurfacePtr);
    gdi_pixels = nullptr;

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
    return false;
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
    if (!dcliprect.Is_Valid() || !scliprect.Is_Valid() || !destrect.Is_Valid() || !sourcerect.Is_Valid()) return false;

    bool use_xsurface = false;

    /*
    **	For non-direct draw surfaces, perform a manual blit operation. This is also
    **	necessary if any of the surfaces are currently locked. It is also necessary if the
    **	blit regions overlap and the blitter cannot handle overlapped regions.
    **
    ** NOTE: Its legal to blit to a locked surface but not from a locked surface.
    ** 	 	ST - 4/23/97 1:03AM
    */
    if (!ssource.Is_Direct_Draw() == true || ((SDLSurface&)ssource).Is_Locked() == true || trans == true) {
        use_xsurface = true;
    } else {
        if (Bytes_Per_Pixel() != ssource.Bytes_Per_Pixel()) {
            use_xsurface = true;
        }
    }

    if (IsPrimary == true && WindowedMode == true) {
        a7 = false;
    }

    if (use_xsurface == false && a7 == true && /*(IsVideoRam == false || ((SDLSurface&)ssource).Is_Direct_Draw() == true && ((SDLSurface&)ssource).IsVideoRam == false)*/ false && sourcerect.Width == destrect.Width && sourcerect.Height == destrect.Height) { 
        use_xsurface = true; // SDL surfaces in RAM
    }

    if (Restore_Check() == false) {
        return false;
    }

    if (use_xsurface == true) {
        return XSurface::Blit_From(destrect, ssource, sourcerect, trans, true);
    }

    SDLSurface const& source = static_cast<SDLSurface const&>(ssource);

    Rect drect = destrect;
    Rect srect = sourcerect;
    Rect swindow = Intersect(scliprect, ssource.Get_Rect());
    Rect dwindow = Intersect(dcliprect, Get_Rect());
    if (Blit_Clip(drect, dwindow, srect, swindow)) {
        SDL_Rect xdestrect;
        xdestrect.x = drect.X + dwindow.X;
        xdestrect.y = drect.Y + dwindow.Y;
        xdestrect.w = drect.Width;
        xdestrect.h = drect.Height;

        SDL_Rect xsrcrect;
        xsrcrect.x = srect.X + swindow.X;
        xsrcrect.y = srect.Y + swindow.Y;
        xsrcrect.w = srect.Width;
        xsrcrect.h = srect.Height;

        bool result = SDL_BlitSurface(source.SurfacePtr, &xsrcrect, SurfacePtr, &xdestrect);

        if (!result) {
            DEBUG_INFO("SDL_BlitSurface failed: %s", SDL_GetError());
        }

        return result;
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
    if (SurfacePtr == nullptr || !fillrect.Is_Valid()) return false;

    /*
    **	If the buffer is locked, then using the blitter to perform the fill is not possible.
    **	In such a case, perform a manual fill of the region.
    */
    if (!DSurface::AllowHWFill || Is_Locked() || !SDLSurface::Can_Blit()) {
        return XSurface::Fill_Rect(cliprect, fillrect, color);
    }

    if (!Restore_Check()) return false;

    /*
    **	Ensure that the clipping rectangle is legal.
    */
    Rect crect = Intersect(cliprect, Get_Rect());

    /*
    **	Bias the fill rect to the clipping rectangle.
    */
    Rect frect = fillrect.Bias_To(cliprect);

    /*
    **	Find the region that should be filled after being clipped by the
    **	clipping rectangle. This could result in no fill operation being performed
    **	if the desired fill rectangle has been completely clipped away.
    */
    frect = Intersect(frect, crect);
    if (!frect.Is_Valid()) return false;

    SDL_Rect rect;
    rect.x = frect.X;
    rect.y = frect.Y;
    rect.w = frect.Width;
    rect.h = frect.Height;

    //// Fill color should be mapped to the surface format
    //Uint32 sdl_color = SDL_MapRGB(SurfacePtr->format,
    //                              (color >> 16) & 0xFF, // R
    //                              (color >> 8) & 0xFF,  // G
    //                              (color) & 0xFF);      // B

    bool result = SDL_FillSurfaceRect(SurfacePtr, &rect, color);

    if (!result) {
        DEBUG_INFO("SDL_FillRect failed: %s", SDL_GetError());
    }

    return result;
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
    return true;
}
