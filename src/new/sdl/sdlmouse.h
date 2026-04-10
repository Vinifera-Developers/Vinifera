/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SDLMOUSE.H
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

#pragma once

#include "SDL3/SDL_mouse.h"
#include "xmouse.h"

#include <mmsystem.h>
#include <vector>

class SDLSurface;
class ShapeSet;

/*
**  Handles the mouse as it relates to the C&C game engine. It is expected that only
**  one object of this type will be created during the lifetime of the game.
*/
class SDLMouseClass : public Mouse
{
public:
    /*
    **  Constructor/destructor.
    */
    SDLMouseClass();
    ~SDLMouseClass() override;

    /*
    **  Maintenance callback routine.
    */
    void Process_Mouse();

    /*
    **  Sets the game-drawn mouse imagery.
    */
    void Set_Cursor(Point2D const& hotspot, ShapeSet const* cursor, int shape) override;

    /*
    **  Controls visibility of the game-drawn mouse.
    */
    bool Is_Hidden() const override { return Get_Mouse_State() < 0; }
    void Hide_Mouse() override;
    void Show_Mouse() override;

    /*
    **  Confines or releases the mouse cursor from the window rect.
    **  Only operates in full screen mode.
    */
    void Release_Mouse() override;
    void Capture_Mouse() override;
    bool Is_Captured() const override { return IsCaptured; }

    /*
    **  Hide the mouse if it falls within this game screen region.
    */
    void Conditional_Hide_Mouse(Rect region) override;
    void Conditional_Show_Mouse() override;

    /*
    **  Query about the mouse visiblity state and location.
    */
    int Get_Mouse_State() const override;
    int Get_Mouse_X() const override { return MouseX; }
    int Get_Mouse_Y() const override { return MouseY; }
    Point2D Get_Mouse_Point() const override { return Point2D(MouseX, MouseY); }

    /*
    **  The following two routines would render the mouse onto a surface.
    **  However, we now use a hardware cursor, so these are no-ops.
    */
    void Draw_Mouse(Surface* = nullptr, bool = false) override {}
    void Erase_Mouse(Surface* = nullptr, bool = false) override {}

    /*
    **  Would convert O/S coordinates to game coordinates.
    **  However, SDL uses client coordinates directly, so this is a no-op.
    */
    void Convert_Coordinate(int& x, int& y) const override {}

    /*
    **  Recalculates the cursor's image using the same shape.
    */
    void Recalc_Cursor_Image();

private:
    /*
    **  This specifies the mouse shape data. It records the shape set
    **  data as well as the particular image contained within.
    */
    ShapeSet const* MouseShape;
    int ShapeNumber;

    /*
    **  This vector contains pointers to SDL_Surfaces that contain
    **  the converted shape frames.
    */
    std::vector<SDL_Surface*> CursorSurfaces;

    /*
    **  The hotspot for the currently used cursor image.
    */
    Point2D Hotspot;

    /*
    **  The currently used cursor.
    */
    SDL_Cursor* Cursor;

    /*
    **  If the mouse is being managed by this class (for the game), then this flag
    **  will be true. When the mouse has been released to be managed by the operating
    **  system, this flag will be false. However, this class will still track the mouse
    **  position.
    */
    bool IsCaptured;

    /*
    **  This is the last recorded mouse position that it was drawn to.
    */
    int MouseX;
    int MouseY;

    /*
    **  Maintenance timer handle.
    */
    MMRESULT TimerHandle;

    /*
    **  Various private utility routines.
    */
    void Update_Mouse_Position(int x, int y);
    void Delete_Cursor_Image();
    void Convert_Cursor_Image(ShapeSet const* shapes);
    void Replace_Cursor(SDL_Cursor* cursor);
    void Set_System_Cursor();

    static int Get_Cursor_Scale();
};

void CALLBACK SDL_Callback_Process_Mouse(UINT, UINT, DWORD, DWORD, DWORD);
