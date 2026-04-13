/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAREXT.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Extended SidebarClass class.
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

#include "sidebarext.h"

#include "drawshape.h"
#include "extension.h"
#include "house.h"
#include "housetype.h"
#include "scenarioext.h"
#include "sideext.h"
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "wwmouse.h"


GadgetClass* SidebarClassExtension::LastHovered;


/**
 *  Returns which tab a type belongs to.
 *
 *  @author: ZivDero
 */
SidebarClassExtension::SidebarTabType SidebarClassExtension::Which_Tab(RTTIType type, ProductionFlags flags)
{
    switch (type) {
    case RTTI_BUILDINGTYPE:
    case RTTI_BUILDING:
        return SIDEBAR_TAB_STRUCTURE;

    case RTTI_INFANTRYTYPE:
    case RTTI_INFANTRY:
        return SIDEBAR_TAB_INFANTRY;

    case RTTI_UNITTYPE:
    case RTTI_UNIT:
        if (flags & PRODFLAG_NAVAL) {
            return SIDEBAR_TAB_SPECIAL;
        } else {
            return SIDEBAR_TAB_UNIT;
        }

    case RTTI_AIRCRAFTTYPE:
    case RTTI_AIRCRAFT:
    case RTTI_SUPERWEAPONTYPE:
    case RTTI_SUPERWEAPON:
    case RTTI_SPECIAL:
    default:
        return SIDEBAR_TAB_SPECIAL;
    }
}


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
SidebarClassExtension::TabButtonClass::TabButtonClass() :
ControlClass(0, 0, 0, 0, 0, LEFTPRESS | LEFTRELEASE, true),
DrawX(0),
DrawY(0),
ShapeDrawer(SidebarDrawer),
ShapeData(nullptr),
IsFlashing(false),
FlashTimer(0),
FlashFrame(0),
IsSelected(false),
IsDrawn(false)
{
}


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
SidebarClassExtension::TabButtonClass::TabButtonClass(unsigned id, const ShapeSet* shapes, int x, int y, ConvertClass* drawer, int w, int h) :
    ControlClass(id, x, y, w, h, LEFTPRESS | LEFTRELEASE, true),
DrawX(0),
DrawY(0),
ShapeDrawer(drawer),
ShapeData(shapes),
IsFlashing(false),
FlashTimer(0),
FlashFrame(0),
IsSelected(false),
IsDrawn(false)
{
}


/**
 *  Handles mouse clicks on the button.
 *
 *  @author: ZivDero
 */
bool SidebarClassExtension::TabButtonClass::Action(unsigned flags, KeyNumType& key)
{
    /*
    **  If there are no action flag bits set, then this must be a forced call. A forced call
    **  must never actually function like a real call, but rather only performs any necessary
    **  graphic updating.
    */
    if (!flags)
    {
        Flag_To_Redraw();
    }

    /*
    **  Handle the sticky state for this gadget. It must be processed here
    **  because the event flags might be cleared before the action function
    **  is called.
    */
    Sticky_Process(flags);

    /*
    **  Pass the mouse press.
    */
    if (flags & LEFTPRESS)
    {
        flags &= ~LEFTPRESS;
        ControlClass::Action(flags, key);
        key = KN_NONE;				        // erase the event
        return true;		                // stop processing other buttons now
    }

    /*
    **  Act on mouse release.
    */
    if (flags & LEFTRELEASE)
    {
        bool overbutton = (Get_Mouse_X() - X) < Width && (Get_Mouse_Y() - Y) < Height;
        if (!IsSelected && overbutton)
        {
            IsSelected = true;
            Flag_To_Redraw();
        }
        else
        {
            flags &= ~LEFTRELEASE;
        }
    }
    
    /*
    **  Do normal button processing. This ends up causing the button's ID number to
    **  be returned from the controlling Input() function.
    */
    return ControlClass::Action(flags, key);
}


/**
 *  Disables the button.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Disable()
{
    IsSelected = false;
    Stop_Flashing();

    ControlClass::Disable();
}


/**
 *  Enables the button.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Enable()
{
    IsSelected = false;
    Stop_Flashing();

    ControlClass::Enable();
}


/**
 *  The draw routine for the button.
 *
 *  @author: ZivDero
 */
bool SidebarClassExtension::TabButtonClass::Draw_Me(bool forced)
{
    if (!ControlClass::Draw_Me(forced))
        return false;

    if (!ShapeData)
        return false;

    if (!ShapeDrawer)
        return false;

    int shapenum;

    // A disabled tab always looks darkened
    if (IsDisabled)
    {
        shapenum = FRAME_DISABLED;
    }
    else if (IsSelected)
    {
        shapenum = FRAME_SELECTED;
    }
    else if (IsFlashing)
    {
        if (FlashTimer.Expired())
        {
            // If we're at the edge of flashing frames, restart
            if (FlashFrame == FLASH_FRAME_MAX)
                FlashFrame = FLASH_FRAME_MIN;
            else
                FlashFrame++;

            FlashTimer = FLASH_RATE;
        }

        shapenum = FlashFrame;
    }
    else
    {
        // Just the normal unselected tab
        shapenum = FRAME_NORMAL;
    }

    Draw_Shape(*SidebarSurface, *ShapeDrawer, ShapeData, shapenum, Point2D(X + DrawX, Y + DrawY), VisibleRect, SHAPE_NORMAL);

    if (MousedOver && !Scen->InputLock && !IsDisabled && !IsSelected)
    {
        Rect hover_rect(X + DrawX, Y + DrawY, Width - 1, Height - 1);
        const ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->UIColor;
        SidebarSurface->Draw_Rect(hover_rect, DSurface::Build_Hicolor_Pixel(ColorSchemes[colorschemetype]->HSV.operator RGBClass()));
    }

    IsDrawn = true;

    return true;
}


/**
 *  Sets the shape of the button.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Set_Shape(const ShapeSet* data, int width, int height)
{
    ShapeData = data;
    if (ShapeData)
    {
        Width = ShapeData->Get_Width();
        Height = ShapeData->Get_Height();
    }

    if (width != 0)
        Width = width;

    if (height != 0)
        Height = height;
}


/**
 *  Function that gets called when the mouse enters the button.
 *  Used for hover effects.
 *
 *  @author: Rampastring
 */
void SidebarClassExtension::TabButtonClass::On_Mouse_Enter()
{
    MousedOver = true;
    Map.IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Function that gets called when the mouse leaves the button.
 *  Used for hover effects.
 *
 *  @author: Rampastring
 */
void SidebarClassExtension::TabButtonClass::On_Mouse_Leave()
{
    MousedOver = false;
    Map.IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Makes the button start flashing.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Start_Flashing()
{
    IsFlashing = true;
    FlashTimer.Start();
    FlashTimer = FLASH_RATE;
    FlashFrame = FLASH_FRAME_START;
}


/**
 *  Makes the button stop flashing.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Stop_Flashing()
{
    IsFlashing = false;
    FlashTimer.Stop();
    FlashFrame = FLASH_FRAME_START;
}


/**
 *  Selects the button.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Select()
{
    IsSelected = true;
}


/**
 *  Deselects the button.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::TabButtonClass::Deselect()
{
    IsSelected = false;
}


/**
 *  Function that gets called when the mouse enters the button.
 *  Used for hover effects.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::ViniferaSelectClass::On_Mouse_Enter()
{
    MousedOver = true;
    Map.IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Function that gets called when the mouse leaves the button.
 *  Used for hover effects.
 *
 *  @author: ZivDero
 */
void SidebarClassExtension::ViniferaSelectClass::On_Mouse_Leave()
{
    MousedOver = false;
    Map.IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Function that checks if the mouse has entered/left a button.
 *  This function is hooked into GadgetClass::Input()
 *
 *  @author: ZivDero, Rampastring
 */
void SidebarClassExtension::Check_Hover(GadgetClass* gadget, int mousex, int mousey)
{
    GadgetClass* to_enter = gadget->Extract_Gadget_At_Mouse(mousex, mousey);
    if (to_enter != LastHovered)
    {
        if (LastHovered)
        {
            // The hovered-on control can be an instance of either ViniferaSelectClass or TabButtonClass
            if (auto select = dynamic_cast<ViniferaSelectClass*>(LastHovered))
            {
                select->On_Mouse_Leave();
            }
            else if (auto tab_button = dynamic_cast<TabButtonClass*>(LastHovered))
            {
                tab_button->On_Mouse_Leave();
            }

            LastHovered = nullptr;
        }

        if (to_enter)
        {
            if (auto select = dynamic_cast<ViniferaSelectClass*>(to_enter))
            {
                LastHovered = select;
                select->On_Mouse_Enter();
            }
            else if (auto tab_button = dynamic_cast<TabButtonClass*>(to_enter))
            {
                LastHovered = tab_button;
                tab_button->On_Mouse_Enter();
            }
        }
    }
}
