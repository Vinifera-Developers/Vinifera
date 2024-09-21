/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MOUSEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended MouseClass.
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
#include "mouseext_hooks.h"
#include "mouse.h"
#include "mousetype.h"
#include "vinifera_globals.h"
#include "wwmouse.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class MouseClassExt final : public MouseClass
{
    public:
        void _AI(KeyNumType &input, Point2D &xy);
        bool _Override_Mouse_Shape(MouseType mouse, bool wsmall = false);
        void _Mouse_Small(bool wsmall = true);
        int _Get_Mouse_Current_Frame(MouseType mouse, bool wsmall = false) const;
        Point2D _Get_Mouse_Hotspot(MouseType mouse) const;
        int _Get_Mouse_Start_Frame(MouseType mouse) const;
        int _Get_Mouse_Frame_Count(MouseType mouse) const;
};


/**
 *  Controls the sizing of the mouse.
 *
 *  @author: 09/21/1995 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 *           CCHyper - Change use of MouseControl to MouseTypes.
 */
void MouseClassExt::_Mouse_Small(bool wsmall)
{
    //MouseStruct const * control = &MouseControl[CurrentMouseShape];
    MouseTypeClass const * control = MouseTypeClass::As_Pointer(CurrentMouseShape);

    if (IsSmall == wsmall) {
        return;
    }

    IsSmall = wsmall;

    int frame = Get_Mouse_Current_Frame(CurrentMouseShape, wsmall);
    Point2D hotspot = Get_Mouse_Hotspot(CurrentMouseShape);

    WWMouse->Set_Cursor(&hotspot, MouseShapes, frame);
}


/**
 *  Alters the shape of the mouse.
 *
 *  @author: 03/10/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 *           CCHyper - Change use of MouseControl to MouseTypes.
 */
bool MouseClassExt::_Override_Mouse_Shape(MouseType mouse, bool wsmall)
{
    ASSERT((unsigned)mouse < MOUSE_COUNT);

    //MouseStruct const * control = &MouseControl[mouse];
    MouseTypeClass const * control = MouseTypeClass::As_Pointer(mouse);
    static bool startup = false;
    int baseshp;

    /**
     *  Only certain mouse shapes have a small counterpart. If the requested mouse
     *  shape is not one of these, then force the small size override flag to false.
     */
    if (control->SmallFrame == -1 || !control->SmallFrameCount) {
        wsmall = false;
    }

    /**
     *  If the mouse shape is going to change, then inform the mouse driver of the
     *  change.
     */
    if (!startup || (MouseShapes && ((mouse != CurrentMouseShape) || (wsmall != IsSmall)))) {
        startup = true;

        Timer = wsmall ? control->SmallFrameRate : control->FrameRate;
        Frame = 0;

        IsSmall = wsmall;

        baseshp = Get_Mouse_Current_Frame(mouse, wsmall);
        Point2D hotspot = Get_Mouse_Hotspot(mouse);
        WWMouse->Set_Cursor(&hotspot, MouseShapes, baseshp);
        CurrentMouseShape = mouse;
        return true;
    }
    return false;
}


/**
 *  Process player input as it relates to the mouse.
 *
 *  @author: 12/24/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 *           CCHyper - Change use of MouseControl to MouseTypes.
 */
void MouseClassExt::_AI(KeyNumType &input, Point2D &xy)
{
    //MouseStruct const * control = &MouseControl[CurrentMouseShape];
    MouseTypeClass const * control = MouseTypeClass::As_Pointer(CurrentMouseShape);

    if (((IsSmall && control->SmallFrameRate) || control->FrameRate) && Timer == 0) {

        Frame++;
        Frame %= IsSmall ? control->SmallFrameCount : control->FrameCount;
        Timer = IsSmall ? control->SmallFrameRate : control->FrameRate;
        int baseframe = Get_Mouse_Current_Frame(CurrentMouseShape, IsSmall);
        Point2D hotspot = Get_Mouse_Hotspot(CurrentMouseShape);
        WWMouse->Set_Cursor(&hotspot, MouseShapes, baseframe);
    }

    ScrollClass::AI(input, xy);
}


/**
 *  Get the animation shape frame of the current mouse.
 * 
 *  @author: CCHyper - Reimplemented from Tiberian Sun.
 *           CCHyper - Change use of MouseControl to MouseTypes.
 */
int MouseClassExt::_Get_Mouse_Current_Frame(MouseType mouse, bool wsmall) const
{
    //MouseStruct const * control = &MouseControl[mouse];
    MouseTypeClass const * control = MouseTypeClass::As_Pointer(mouse);

    if (wsmall) {
        if (control->SmallFrame != -1) {
            return control->SmallFrame + Frame;
        }
    }

    return control->StartFrame + Frame;
}


/**
 *  Get the action hotspot of the current mouse.
 *
 *  @author: CCHyper - Reimplemented from Tiberian Sun.
 *           CCHyper - Change use of MouseControl to MouseTypes.
 */
Point2D MouseClassExt::_Get_Mouse_Hotspot(MouseType mouse) const
{
    Point2D hotspot(0,0);

    if (MouseShapes) {

        //MouseStruct const * control = &MouseControl[mouse];
        MouseTypeClass const * control = MouseTypeClass::As_Pointer(mouse);

        int hotspot_x = IsSmall ? control->SmallHotspot.X : control->Hotspot.X;
        int hotspot_y = IsSmall ? control->SmallHotspot.X : control->Hotspot.X;

        switch (hotspot_x) {
            case MOUSE_HOTSPOT_CENTER:
                hotspot.X = MouseShapes->Get_Width() / 2;
                break;
            case MOUSE_HOTSPOT_MAX:
                hotspot.X = MouseShapes->Get_Width();
                break;
            case MOUSE_HOTSPOT_MIN:
            default:
                hotspot.X = std::clamp(hotspot_x, -MouseShapes->Get_Width(), MouseShapes->Get_Width());
                break;
        };

        switch (hotspot_y) {
            case MOUSE_HOTSPOT_CENTER:
                hotspot.Y = MouseShapes->Get_Height() / 2;
                break;
            case MOUSE_HOTSPOT_MAX:
                hotspot.Y = MouseShapes->Get_Height();
                break;
            case MOUSE_HOTSPOT_MIN:
            default:
                hotspot.Y = std::clamp(hotspot_y, -MouseShapes->Get_Height(), MouseShapes->Get_Height());
                break;
        };

    }

    return hotspot;
}


/**
 *  Returns the starting frame of the mouse.
 *
 *  @author: CCHyper - Reimplemented from Tiberian Sun.
 *           CCHyper - Change use of MouseControl to MouseTypes.
 */
int MouseClassExt::_Get_Mouse_Start_Frame(MouseType mouse) const
{
    //return MouseControl[mouse].StartFrame;
    return MouseTypeClass::As_Pointer(mouse)->StartFrame;
}


/**
 *  Returns the frame count of the mouse.
 *
 *  @author: CCHyper - Reimplemented from Tiberian Sun.
 *           CCHyper - Change use of MouseControl to MouseTypes.
 */
int MouseClassExt::_Get_Mouse_Frame_Count(MouseType mouse) const
{
    //return MouseControl[mouse].FrameCount;
    return MouseTypeClass::As_Pointer(mouse)->FrameCount;
}


void MouseClassExtension_Hooks()
{
    Patch_Jump(0x00562200, &MouseClassExt::_Mouse_Small);
    Patch_Jump(0x005622D0, &MouseClassExt::_Get_Mouse_Current_Frame);
    Patch_Jump(0x00562310, &MouseClassExt::_Get_Mouse_Hotspot);
    Patch_Jump(0x00562390, &MouseClassExt::_Override_Mouse_Shape);
    Patch_Jump(0x005624D0, &MouseClassExt::_AI);
    Patch_Jump(0x00563220, &MouseClassExt::_Get_Mouse_Start_Frame);
    Patch_Jump(0x00563240, &MouseClassExt::_Get_Mouse_Frame_Count);
}
