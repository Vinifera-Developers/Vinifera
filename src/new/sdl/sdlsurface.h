/***********************************************************************************************
 ***              C O N F I D E N T I A L  ---  W E S T W O O D  S T U D I O S               ***
 ***********************************************************************************************
 *                                                                                             *
 *                 Project Name : Command & Conquer                                            *
 *                                                                                             *
 *                     $Archive:: /G/wwlib/SDLSurface.h                                         $*
 *                                                                                             *
 *                      $Author:: Neal_k                                                      $*
 *                                                                                             *
 *                     $Modtime:: 6/23/00 2:24p                                               $*
 *                                                                                             *
 *                    $Revision:: 2                                                           $*
 *                                                                                             *
 *---------------------------------------------------------------------------------------------*
 * Functions:                                                                                  *
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */

#pragma once

#include "palette.h"
#include "dsurface.h"
#include "SDL3/SDL_surface.h"


enum SDLSurfaceColorMode {
    COLORMODE_INVALID = -1,
    COLORMODE_555,
    COLORMODE_556,
    COLORMODE_565,
    COLORMODE_655,
};


/**
 *  This is a concrete surface class that is based on the DirectDraw
 *  API.
 */
class SDLSurface : public DSurface
{
    typedef XSurface BASECLASS;

public:
    ~SDLSurface() override;

    /**
     *  Default constructor.
     */
    SDLSurface();

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

    /*
    **  Queries information about the surface.
    */
    int Stride() const override;

    /*
    **  Verifies that this is not a direct draw enabled surface.
    */
    bool Is_Direct_Draw() const override { return false; }

    bool Can_Blit() const override;
    SDL_Surface* Get_SDL_Surface() const { return SDLSurfacePtr; }
    bool Restore_Check() const;
    void Blit_To_Window(Rect const* region = nullptr) const;

protected:

    /**
     *  Direct draw specific data.
     */
    SDL_Surface* SDLSurfacePtr;

    int Pitch;

    mutable HDC GDIDC;
    mutable HBITMAP GDIBitmap;
    mutable void* GDIBuffer; // Points directly to SDL's pixel buffer

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
