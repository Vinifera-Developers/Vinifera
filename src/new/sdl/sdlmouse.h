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

#include "actiontype.h"
#include "xmouse.h"
#include "SDL3/SDL_mouse.h"

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
    **  Private constructor.
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
    **  Takes control of and releases control of the mouse with
    **  respect to the operating system. The mouse must be released
    **  during operations with the operating system. When the mouse is
    **  relased, it may move outside of the confining rectangle and its
    **  shape is controlled by the operating sytem.
    */
    void Release_Mouse() override;
    void Capture_Mouse() override;
    bool Is_Captured() const override { return true; }

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
    **  The following two routines can be used to render the mouse onto an alternate
    **  surface.
    */
    void Draw_Mouse(Surface* = nullptr, bool = false) override;
    void Erase_Mouse(Surface* = nullptr, bool = false) override;

    /*
    **  Converts O/S screen coordinates into game coordinates.
    */
    void Convert_Coordinate(int& x, int& y) const override;

    void Recacl_Cursor_Image();

private:
    /*
    **  This specifies the mouse shape data. It records the shape set
    **  data as well as the particular image contained within.
    */
    ShapeSet const* MouseShape;
    int ShapeNumber;
    Point2D Hotspot;
    std::vector<SDL_Surface*> CursorSurfaces;

    SDL_Cursor* Cursor;

    /*
    **  This is the last recorded mouse position that it was drawn to.
    */
    int MouseX;
    int MouseY;

    /*
    **  Maintenance timer handle.
    */
    MMRESULT TimerHandle;

    void Update_Mouse_Position(int x, int y, bool forced);

    void Delete_Cursor_Image();
    void Convert_Cursor_Image(ShapeSet const* shapes);
    void Clear_Cursor();
    void Replace_Cursor(SDL_Cursor* cursor);
    void Set_System_Cursor();

    static int Get_Cursor_Scale();
};

void CALLBACK SDL_Callback_Process_Mouse(UINT, UINT, DWORD, DWORD, DWORD);
