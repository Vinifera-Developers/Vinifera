/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended MouseClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "mouseext_hooks.h"

#include "asserthandler.h"
#include "cell.h"
#include "extension.h"
#include "hooker.h"
#include "mouse.h"
#include "mousetype.h"
#include "sdlmouse.h"
#include "syringe.h"
#include "techno.h"
#include "tibsun_functions.h"
#include "vinifera_globals.h"
#include "vinifera_imgui.h"
#include "weapontype.h"
#include "weapontypeext.h"
#include "windialog.h"
#include "wwmouse.h"

#include <imgui.h>


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class MouseClassExt : public MouseClass
{
public:
    void _AI(KeyNumType& input, Point2D& xy);
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
    MouseTypeClass const * control = MouseTypes[CurrentMouseShape];

    if (IsSmall == wsmall) {
        return;
    }

    IsSmall = wsmall;

    int frame = Get_Mouse_Current_Frame(CurrentMouseShape, wsmall);
    Point2D hotspot = Get_Mouse_Hotspot(CurrentMouseShape);

    MouseCursor->Set_Cursor(hotspot, MouseShapes, frame);
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
    ASSERT((unsigned)mouse < MouseTypes.Count());

    MouseTypeClass const * control = MouseTypes[mouse];
    static bool startup = false;

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

        MouseCursor->Set_Cursor(Get_Mouse_Hotspot(mouse), MouseShapes, Get_Mouse_Current_Frame(mouse, wsmall));
        CurrentMouseShape = mouse;
        IsSmall = wsmall;
        return true;
    }
    return false;
}


/**
 *  Maps an ImGui cursor shape to an SDL system cursor id.
 */
static SDL_SystemCursor Map_ImGui_Cursor(ImGuiMouseCursor c)
{
    switch (c) {
    case ImGuiMouseCursor_TextInput:  return SDL_SYSTEM_CURSOR_TEXT;
    case ImGuiMouseCursor_ResizeAll:  return SDL_SYSTEM_CURSOR_MOVE;
    case ImGuiMouseCursor_ResizeNS:   return SDL_SYSTEM_CURSOR_NS_RESIZE;
    case ImGuiMouseCursor_ResizeEW:   return SDL_SYSTEM_CURSOR_EW_RESIZE;
    case ImGuiMouseCursor_ResizeNESW: return SDL_SYSTEM_CURSOR_NESW_RESIZE;
    case ImGuiMouseCursor_ResizeNWSE: return SDL_SYSTEM_CURSOR_NWSE_RESIZE;
    case ImGuiMouseCursor_Hand:       return SDL_SYSTEM_CURSOR_POINTER;
    case ImGuiMouseCursor_Wait:       return SDL_SYSTEM_CURSOR_WAIT;
    case ImGuiMouseCursor_Progress:   return SDL_SYSTEM_CURSOR_PROGRESS;
    case ImGuiMouseCursor_NotAllowed: return SDL_SYSTEM_CURSOR_NOT_ALLOWED;
    default:                          return SDL_SYSTEM_CURSOR_DEFAULT;
    }
}


/**
 *  Process player input as it relates to the mouse.
 *
 *  @author: 12/24/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun.
 *           CCHyper - Change use of MouseControl to MouseTypes.
 *           ZivDero - Override the OS cursor for Windows dialogs and ImGui.
 */
void MouseClassExt::_AI(KeyNumType &input, Point2D &xy)
{
    SDLMouseClass* sdl_mouse = static_cast<SDLMouseClass*>(MouseCursor);

    /**
     *  Decide up front; apply at the end. ScrollClass::AI runs the game's
     *  hover logic and may call Override_Mouse_Shape, which would otherwise
     *  clobber our override mid-tick.
     */
    bool want_override = false;
    bool want_hide = false;
    SDL_SystemCursor override_id = SDL_SYSTEM_CURSOR_DEFAULT;

    if (WSDialogCount > 0) {
        want_override = true;
    } else if (ViniferaImGui::Is_Initialized()) {
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantCaptureMouse) {
            ImGuiMouseCursor c = ImGui::GetMouseCursor();
            if (c == ImGuiMouseCursor_None || io.MouseDrawCursor) {
                want_hide = true;
            } else {
                want_override = true;
                override_id = Map_ImGui_Cursor(c);
            }
        }
    }

    if (!want_override && !want_hide) {
        MouseTypeClass const * control = MouseTypes[CurrentMouseShape];

        if (((IsSmall && control->SmallFrameRate) || control->FrameRate) && Timer == 0) {

            Frame++;
            Frame %= IsSmall ? control->SmallFrameCount : control->FrameCount;
            Timer = IsSmall ? control->SmallFrameRate : control->FrameRate;

            MouseCursor->Set_Cursor(Get_Mouse_Hotspot(CurrentMouseShape), MouseShapes, Get_Mouse_Current_Frame(CurrentMouseShape, IsSmall));
        }
    }

    ScrollClass::AI(input, xy);

    if (want_hide) {
        sdl_mouse->Hide_Override_Cursor();
    } else if (want_override) {
        sdl_mouse->Set_Override_System_Cursor(override_id);
    } else {
        sdl_mouse->Clear_Cursor_Override();
    }
}


/**
 *  Get the animation shape frame of the current mouse.
 * 
 *  @author: CCHyper - Reimplemented from Tiberian Sun.
 *           CCHyper - Change use of MouseControl to MouseTypes.
 */
int MouseClassExt::_Get_Mouse_Current_Frame(MouseType mouse, bool wsmall) const
{
    MouseTypeClass const * control = MouseTypes[mouse];

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

        MouseTypeClass const * control = MouseTypes[mouse];

        int hotspot_x = IsSmall ? control->SmallHotspot.X : control->Hotspot.X;
        int hotspot_y = IsSmall ? control->SmallHotspot.Y : control->Hotspot.Y;

        switch (hotspot_x) {
            case MOUSE_HOTSPOT_CENTER:
                hotspot.X = MouseShapes->Get_Width() / 2;
                break;
            case MOUSE_HOTSPOT_MAX:
                hotspot.X = MouseShapes->Get_Width();
                break;
            case MOUSE_HOTSPOT_MIN:
            default:
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
    return MouseTypes[mouse]->StartFrame;
}


/**
 *  Returns the frame count of the mouse.
 *
 *  @author: CCHyper - Reimplemented from Tiberian Sun.
 *           CCHyper - Change use of MouseControl to MouseTypes.
 */
int MouseClassExt::_Get_Mouse_Frame_Count(MouseType mouse) const
{
    return MouseTypes[mouse]->FrameCount;
}


/**
 *  Gets the action type for the the given object.
 *
 *  @author: ZivDero
 */
static ActionType Get_Action(ObjectClass* obj, Cell& cellnum, bool check_fog)
{
    ActionType action;
    TechnoClass* selected = Best_Selected_Object();

    if (obj) {
        action = selected->What_Action(obj, false);
    }
    else {
        action = selected->What_Action(cellnum, check_fog, false);
    }

    /**
     *  For ACTION_ATTACK, we fetch a different action from the weapon for the visuals.
     */
    if (action == ACTION_ATTACK) {
        const auto weapon = selected->Get_Weapon(selected->What_Weapon_Should_I_Use(obj));

        if (weapon->Weapon) {
            const auto weapon_ext = Extension::Fetch(weapon->Weapon);

            if (cellnum != CELL_NONE
                && CurrentObjects.Count() == 1
                && CurrentObjects[0]->Is_Techno()
                && static_cast<TechnoClass*>(CurrentObjects[0])->In_Range_Of(&Map[cellnum])) {

                return weapon_ext->CursorStayAttack;
            }

            return weapon_ext->CursorAttack;
        }
    }

    return action;
}


/**
 *  Patch that replaces the action type for the attack cursor.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005E8920, _ScrollClass_What_Action_Attack_Cursor_Patch, 0)
{
    GET_STACK(Cell*, cellnum, 0x18);
    GET_STACK(ObjectClass*, obj, 0x1C);
    GET_STACK(bool, check_fog, 0x20);

    ActionType action = Get_Action(obj, *cellnum, check_fog);

    // return action;
    R->EAX(action);
    return 0x005E8936;
}


/**
 *  Main function for patching the hooks.
 */
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
