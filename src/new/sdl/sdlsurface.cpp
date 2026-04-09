/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SDLSURFACE.CPP
 *
 *  @author        ZivDero, tomsons26
 *
 *  @brief         SDL Surface class.
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

#include "always.h"

#include "sdlsurface.h"

#include "debughandler.h"
#include "dsurface.h"
#include "sdl_functions.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"


/**
 *  The pixel format of the SDL surfaces created.
 */
const SDL_PixelFormatDetails* SDLSurface::PixelFormat = nullptr;


/**
 *  Struct used to create GDI DIB sections.
 */
struct BitmapInfo
{
    BITMAPINFOHEADER Header;
    DWORD Masks[3];
};


/**
 *  SDLSurface constructor.
 *
 *  @author: ZivDero
 */
SDLSurface::SDLSurface(int width, int height) :
    DSurface(), // use the default constructor so that we don't initialize the DDraw portions of the surface
    SDLSurfacePtr(nullptr),
    GDIDC(nullptr),
    GDIBitmap(nullptr),
    GDIBuffer(nullptr),
    Pitch(0)
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
        DEBUG_ERROR("CreateDIBSection failed! Error = %lu\n", GetLastError());
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


/**
 *  SDLSurface destructor.
 *
 *  @author: ZivDero
 */
SDLSurface::~SDLSurface()
{
    if (SDLSurfacePtr) {
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
 *
 *  @author: ZivDero, tomsons26
 */
static void Calculate_Mask_Info(unsigned int mask, unsigned int& right, unsigned int& left)
{
    right = 0;
    left = 0;

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


/**
 *  With DSurface, this would create the primary (visible) surface.
 *  There is no such thing with SDL, but we take this opportunity to
 *  initialize some static variables used for color conversions.
 *
 *  @author: ZivDero, tomsons26
 */
SDLSurface* SDLSurface::Create_Primary(void*)
{
    DEBUG_INFO("SDLSurface::Create_Primary\n");

    AllowStretchBlits = false;
    AllowHWFill = false;

    DEBUG_INFO("SDLSurface::Create_Primary - Creating surface\n");
    SDLSurface* surface = new SDLSurface(VideoWidth, VideoHeight);

    /**
     *  If this is a hicolor surface, then build the shift values for
     *  building and extracting the colors from the hicolor pixel.
     */
    if (PrimaryColorMode == COLORMODE_INVALID) {
        Calculate_Mask_Info(PixelFormat->Rmask, RedRight, RedLeft);
        Calculate_Mask_Info(PixelFormat->Gmask, GreenRight, GreenLeft);
        Calculate_Mask_Info(PixelFormat->Bmask, BlueRight, BlueLeft);

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


/**
 *  Blit from one surface to this one.
 *
 *  @author: ZivDero, tomsons26
 */
bool SDLSurface::Blit_From(Rect const& dcliprect, Rect const& destrect, Surface const& ssource, Rect const& scliprect, Rect const& sourcerect, bool trans, bool)
{
    if (!dcliprect.Is_Valid() || !scliprect.Is_Valid() || !destrect.Is_Valid() || !sourcerect.Is_Valid()) return false;

    return XSurface::Blit_From(destrect, ssource, sourcerect, trans, true);
}


/**
 *  This routine will fill the specified rectangle.
 *
 *  @author: ZivDero, tomsons26
 */
bool SDLSurface::Fill_Rect(Rect const& fillrect, int color)
{
    return SDLSurface::Fill_Rect(Get_Rect(), fillrect, color);
}


/**
 *  Fills a rectangle with clipping control.
 *
 *  @author: ZivDero, tomsons26
 */
bool SDLSurface::Fill_Rect(Rect const& cliprect, Rect const& fillrect, int color)
{
    if (SDLSurfacePtr == nullptr || !fillrect.Is_Valid()) return false;

    /**
     *  Ensure that the clipping rectangle is legal.
     */
    Rect crect = Intersect(cliprect, Get_Rect());

    /**
     *  Bias the fill rect to the clipping rectangle.
     */
    Rect frect = fillrect.Bias_To(cliprect);

    /**
     *  Find the region that should be filled after being clipped by the
     *  clipping rectangle. This could result in no fill operation being performed
     *  if the desired fill rectangle has been completely clipped away.
     */
    frect = Intersect(frect, crect);
    if (!frect.Is_Valid()) return false;

    SDL_Rect rect;
    rect.x = frect.X;
    rect.y = frect.Y;
    rect.w = frect.Width;
    rect.h = frect.Height;

    return SDL_FillSurfaceRect(SDLSurfacePtr, &rect, color);
}


/**
 *  Get the windows device context from our surface.
 *
 *  @author: ZivDero
 */
HDC SDLSurface::GetDC()
{
    if (GDIDC == nullptr) {
        return nullptr;
    }

    LockCount++;
    return GDIDC;
}


/**
 *  Release the windows device context from our surface.
 *
 *  @author: ZivDero
 */
int SDLSurface::ReleaseDC(HDC hdc)
{
    if (!GDIDC || hdc != GDIDC) {
        return 0;
    }

    if (LockCount > 0) {
        LockCount--;
    }

    return 1;
}


/**
 *  Fetches the bytes between rows.
 *
 *  @author: ZivDero
 */
int SDLSurface::Stride() const
{
    return Pitch;
}


/**
 *  Fetches a working pointer into surface memory.
 *
 *  @author: ZivDero, tomsons26
 */
void* SDLSurface::Lock(Point2D point) const
{
    if (point.X < 0 || point.Y < 0) return nullptr;

    if (LockCount == 0) {
        if (SDL_MUSTLOCK(SDLSurfacePtr)) {
            if (!SDL_LockSurface(SDLSurfacePtr)) {
                return nullptr; // failed to lock
            }
        }
        LockPtr = SDLSurfacePtr->pixels;
    }
    XSurface::Lock();
    return static_cast<char*>(LockPtr) + point.Y * Stride() + point.X * Bytes_Per_Pixel();
}


/**
 *  Returns if the surface can be locked.
 *
 *  @author: ZivDero
 */
bool SDLSurface::Can_Lock(int x, int y) const
{
    return SDLSurfacePtr != nullptr;
}


/**
 *  Returns if the surface can be blitted to.
 *
 *  @author: ZivDero
 */
bool SDLSurface::Can_Blit() const
{
    return SDLSurfacePtr != nullptr;
}


/**
 *  Unlock a previously locked surface.
 *
 *  @author: ZivDero
 */
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
