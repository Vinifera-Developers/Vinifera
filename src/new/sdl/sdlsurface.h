/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SDLSURFACE.H
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

#pragma once

#include "SDL3/SDL_surface.h"
#include "dsurface.h"


enum SDLSurfaceColorMode {
    COLORMODE_INVALID = -1,
    COLORMODE_555,
    COLORMODE_556,
    COLORMODE_565,
    COLORMODE_655,
};


/**
 *  This is a concrete surface class that allocates memory as  GDI DIB and
 *  wraps it in an SDL_Surface structure for use with SDL rendering.
 *  It is derived from DSurface to inherit most of the drawing routines.
 */
class SDLSurface : public DSurface
{
public:
    ~SDLSurface() override;

    /**
     *  Constructs a working surface (not visible).
     */
    SDLSurface(int width, int height);

    /**
     *  Copies regions from one surface to another.
     */
    bool Blit_From(Rect const& dcliprect, Rect const& destrect, Surface const& source, Rect const& scliprect, Rect const& sourcerect, bool trans = false, bool = true) override;

    /**
     *  Fills a region with a constant color.
     */
    bool Fill_Rect(Rect const& rect, int color) override;
    bool Fill_Rect(Rect const& cliprect, Rect const& fillrect, int color) override;

    /**
     *  Get/Release a windows device context from a DirectX surface
     */
    HDC GetDC();
    int ReleaseDC(HDC hdc);

    /**
     *  Create a surface object that represents the currently visible screen.
     */
    static SDLSurface* Create_Primary(void* = nullptr);

    /**
     *  Gets and frees a direct pointer to the video memory.
     */
    void* Lock(Point2D point = Point2D(0, 0)) const override;
    bool Unlock() const override;
    bool Can_Lock(int x = 0, int y = 0) const override;

    /**
     *  Queries information about the surface.
     */
    int Stride() const override;

    /**
     *  Verifies that this is not a direct draw enabled surface.
     */
    bool Is_Direct_Draw() const override { return false; }

    bool Can_Blit() const override;
    SDL_Surface* Get_SDL_Surface() const { return SDLSurfacePtr; }

protected:

    /**
     *  The SDL_Surface representation of this surface.
     */
    SDL_Surface* SDLSurfacePtr;

    /**
     *  The GDI representation of this surface.
     *  GDI is what actually owns the memory.
     */
    mutable HDC GDIDC;
    mutable HBITMAP GDIBitmap;
    mutable void* GDIBuffer;

    /**
     *  The surface's pitch.
     */
    int Pitch;

    /**
     *  Pixel format of primary surface.
     */
    static const SDL_PixelFormatDetails* PixelFormat;

private:

    /**
     *  This prevents the creation of a surface in ways that are not
     *  supported.
     */
    SDLSurface(SDLSurface const& rvalue) = delete;
    SDLSurface const operator=(SDLSurface const& rvalue) = delete;
};
