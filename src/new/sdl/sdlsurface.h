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
#include "xsurface.h"
#include "SDL3/SDL_surface.h"

#include <ddraw.h>


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
class SDLSurface : public XSurface
{
    typedef XSurface BASECLASS;

public:
    virtual ~SDLSurface();

    /**
     *  Default constructor.
     */
    SDLSurface();

    /**
     *  Constructs a working surface (not visible).
     */
    SDLSurface(int width, int height);

    /**
     *  Creates a surface from a previously created DirectDraw surface object.
     */
    SDLSurface(SDL_Surface* surfaceptr);

    /**
     *  Get/Release a windows device context from a DirectX surface
     */
    HDC GetDC();
    int ReleaseDC(HDC hdc);

    /**
     *  Create a surface object that represents the currently visible screen.
     */
    static SDLSurface* Create_Primary(SDLSurface** backsurface1 = nullptr);

    /**
     *  Copies regions from one surface to another.
     */
    virtual bool Blit_From(Rect const& dcliprect, Rect const& destrect, Surface const& source, Rect const& scliprect, Rect const& sourcerect, bool trans = false, bool a7 = true) override;
    virtual bool Blit_From(Rect const& destrect, Surface const& source, Rect const& sourcerect, bool trans = false, bool a5 = true) override;
    virtual bool Blit_From(Surface const& source, bool trans = false, bool a3 = true) override { return XSurface::Blit_From(source, trans, a3); }

    /**
     *  Fills a region with a constant color.
     */
    virtual bool Fill_Rect(Rect const& rect, int color) override;
    virtual bool Fill_Rect(Rect const& cliprect, Rect const& fillrect, int color) override;
    virtual bool Fill_Rect_Trans(Rect const& rect, RGBClass const& color, int opacity) override;

    /**
     *  Gets and frees a direct pointer to the video memory.
     */
    virtual void* Lock(Point2D point = Point2D(0, 0)) const override;
    virtual bool Unlock() const override;
    virtual bool Can_Lock(int x = 0, int y = 0) const override;

    /**
     *  Queries information about the surface.
     */
    virtual int Bytes_Per_Pixel() const override;
    virtual int Stride() const override;

    /**
     *  Verifies that this is a direct draw enabled surface.
     */
    virtual bool Is_Direct_Draw() const override { return true; }

    virtual bool Can_Blit() const;
    SDL_Surface* Get_SDL_Surface() const { return SurfacePtr; }
    bool Restore_Check() const;
    void Blit_To_Window() const;

protected:

    /**
     *  Convenient copy of the bytes per pixel value to speed accessing it. It
     *  gets accessed frequently.
     */
    mutable int BytesPerPixel;

    /**
     *  Lock count and pointer values. This is used to keep track of the levels
     *  of locking the graphic data. This is only here because DirectDraw prohibits
     *  the blitter from working on a surface that has been locked.
     */
    mutable void* LockPtr;

    /**
     *  If this surface object represents the one that is visible and associated
     *  with the system GDI, then this flag will be true.
     */
    bool IsPrimary;

    /**
     *  Direct draw specific data.
     */
    SDL_Surface* SurfacePtr;

    mutable HDC GdiDC = nullptr;
    mutable HBITMAP GdiBitmap = nullptr;
    mutable void* GdiBuffer = nullptr; // Points directly to SDL's pixel buffer

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
