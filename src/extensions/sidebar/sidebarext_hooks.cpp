/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAREXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended SidebarClass.
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

#include "sidebarext_hooks.h"

#include "bsurface.h"
#include "building.h"
#include "buildingtype.h"
#include "convert.h"
#include "debughandler.h"
#include "drawshape.h"
#include "event.h"
#include "eventext.h"
#include "extension.h"
#include "factory.h"
#include "factoryext.h"
#include "fatal.h"
#include "fetchres.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "language.h"
#include "miscutil.h"
#include "mouse.h"
#include "msgbox.h"
#include "optionsext.h"
#include "playmovie.h"
#include "rules.h"
#include "rulesext.h"
#include "scenarioext.h"
#include "session.h"
#include "sidebar.h"
#include "sideext.h"
#include "spritecollection.h"
#include "super.h"
#include "supertype.h"
#include "supertypeext.h"
#include "syringe.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "textprint.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "uicontrol.h"
#include "vinifera_globals.h"
#include "voc.h"
#include "vox.h"
#include "wwmouse.h"

#include "battleui_component.h"
#include "sidebar_tabbed_view.h"
#include "cameo_button.h"

#include <algorithm>


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class SidebarClassExt : public SidebarClass
{
public:
    void _One_Time();
    void _Init_Clear();
    void _Init_IO();
    void _Init_For_House();
    void _Init_Strips();
    bool _Factory_Link(FactoryClass* factory, RTTIType type, int id);
    bool _Add(RTTIType type, int id);
    bool _Activate(int control);
    bool _Scroll(bool up, int column);
    bool _Scroll_Page(bool up, int column);
    void _Draw_It(bool complete);
    void _AI(KeyNumType& input, Point2D& xy);
    void _Recalc();
    bool _Abandon_Production(RTTIType type, FactoryClass* factory);
    void _Set_Dimensions();
    const char* _Help_Text(int gadget_id);
    int _Max_Visible();
    int _Which_Column(RTTIType type);
};


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class StripClassExt : public SidebarClass::StripClass
{
public:
    void _Flag_To_Redraw();
};


static int& SidebarDialogCount = Make_Global<int>(0x007E492C);


static void Request_Sidebar_Redraw(SidebarClass& sidebar, int flags = 0)
{
    sidebar.IsToRedraw = true;
    RedrawSidebar = true;
    sidebar.Flag_To_Redraw(flags);
}


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: ZivDero
 */
/**
 *  Reimplements the entire SidebarClass::One_Time function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_One_Time()
{
    RadarClass::One_Time();

    BattleUI.One_Time();
}


/**
 *  Reimplements the entire SidebarClass::Init_Clear function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Init_Clear()
{
    RadarClass::Init_Clear();

    IsToRedraw = true;
    IsRepairActive = false;
    IsUpgradeActive = false;

    BattleUI.Init_Clear();

    Activate(0);
}


/**
 *  Reimplements the entire SidebarClass::Init_IO function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Init_IO()
{
    RadarClass::Init_IO();

    SidebarRect.X = TacticalRect.Width + TacticalRect.X;
    SidebarRect.Y = 148;
    SidebarRect.Width = 641 - (TacticalRect.Width + TacticalRect.X);
    SidebarRect.Height = TacticalRect.Height + TacticalRect.Y - SidebarRect.Y;

    if (!Debug_Map) {
        BattleUI.Init_IO();
        BattleUI.Set_Dimensions();

        if (IsSidebarActive) {
            IsSidebarActive = false;
            Activate(1);
        }
    }
}


/**
 *  Reimplements the entire SidebarClass::Init_For_House function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Init_For_House()
{
    RadarClass::Init_For_House();
    BattleUI.Init_For_House();
}


/**
 *  Reimplements the entire SidebarClass::Init_Strips function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Init_Strips()
{
    BattleUI.Get_Sidebar().Reload_Layout();
}


/**
 *  Reimplements the entire SidebarClass::Factory_Link function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Factory_Link(FactoryClass* factory, RTTIType type, int id)
{
    return BattleUI.Get_Sidebar().Factory_Link(factory, type, id);
}


/**
 *  Reimplements the entire SidebarClass::Add function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Add(RTTIType type, int id)
{
    if (!Debug_Map) {
        if (BattleUI.Get_Sidebar().Add(type, id)) {
            Activate(1);
            Request_Sidebar_Redraw(*this);
            return true;
        }
    }

    return false;
}


/**
 *  Reimplements the entire SidebarClass::Activate function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Activate(int control)
{
    bool old = IsSidebarActive;

    if (Session.Play && !Session.Singleplayer_Game()) {
        return old;
    }

    switch (control) {
    case -1:
        IsSidebarActive = IsSidebarActive == false;
        break;
    case 1:
        IsSidebarActive = true;
        break;
    default:
    case 0:
        IsSidebarActive = false;
        break;
    }

    if (IsSidebarActive != old) {
        if (IsSidebarActive) {
            Set_Dimensions();
            IsToRedraw = true;

            RadarButton.Zap();
            Add_A_Button(RadarButton);

            BattleUI.Get_Sidebar().Activate(1);
        } else {
            End_Ingame_Movie();

            Remove_A_Button(RadarButton);

            BattleUI.Get_Sidebar().Activate(0);
        }

        Flag_To_Redraw(2);
    }

    return old;
}


/**
 *  Reimplements the entire SidebarClass::Scroll function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Scroll(bool up, int column)
{
    if (SidebarDialogCount != 0) {
        return false;
    }

    if (BattleUI.Get_Sidebar().Scroll(up, column)) {
        Request_Sidebar_Redraw(*this);
        return true;
    }

    Sound_Effect(Rule->ScoldSound);
    return false;
}


/**
 *  Reimplements the entire SidebarClass::Scroll_Page function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Scroll_Page(bool up, int column)
{
    if (BattleUI.Get_Sidebar().Scroll_Page(up, column)) {
        Request_Sidebar_Redraw(*this);
        return true;
    }

    Sound_Effect(Rule->ScoldSound);
    return false;
}


/**
 *  Reimplements the entire SidebarClass::AI function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_AI(KeyNumType& input, Point2D& xy)
{
    if (!Debug_Map) {
        Activate(1);
    }

    BattleUI.AI(input, xy);

    RadarClass::AI(input, xy);
}


/**
 *  Reimplements the entire SidebarClass::Draw_It function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Draw_It(bool complete)
{
    complete |= IsToFullRedraw;
    Map.LastDrawRect = Rect(0, 0, 0, 0);
    RadarClass::Draw_It(complete);

    BattleUI.Draw(complete);
    BattleUI.Blit(complete);
}


/**
 *  Reimplements the entire SidebarClass::Recalc function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Recalc()
{
    BattleUI.Get_Sidebar().Recalc();
    Request_Sidebar_Redraw(*this);
}


/**
 *  Reimplements the entire SidebarClass::Abandon_Production function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Abandon_Production(RTTIType type, FactoryClass* factory)
{
    DEBUG_FATAL("The legacy version of SidebarClass::Abandon_Production has been called! If you see this, please notify the developers. The game will now exit.\n");
    DEBUG_FATAL("Return address: %p\n", _ReturnAddress());
    WWMessageBox().Process("The legacy version of SidebarClass::Abandon_Production has been called! If you see this, please notify the developers. The game will now exit.", 0, TXT_OK);
    Emergency_Exit(0);
    return false;
}


/**
 *  Reimplements the entire SidebarClass::Set_Dimensions function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Set_Dimensions()
{
    SidebarRect.X = Options.SidebarSide ? TacticalRect.X + TacticalRect.Width : 0;
    SidebarRect.Y = 148;
    SidebarRect.Width = 168;
    SidebarRect.Height = TacticalRect.Y + TacticalRect.Height - 148;

    RadarClass::Set_Dimensions();

    BattleUI.Set_Dimensions();
}


/**
 *  Reimplements the entire SidebarClass::Help_Text function.
 *
 *  @author: ZivDero
 */
const char* SidebarClassExt::_Help_Text(int gadget_id)
{
    return BattleUI.Help_Text(gadget_id);
}


/**
 *  Reimplements the entire SidebarClass::Max_Visible function.
 *
 *  @author: ZivDero
 */
int SidebarClassExt::_Max_Visible()
{
    return BattleUI.Get_Sidebar().Visible_Buttons_Per_Column();
}


/**
 *  Reimplements the entire SidebarClass::Which_Column function.
 *
 *  @author: ZivDero
 */
int SidebarClassExt::_Which_Column(RTTIType type)
{
    return BattleUI.Get_Sidebar().Get_Model().Route_To_Category(type, 0);
}


/**
 *  Reimplements SidebarClass::StripClass::Flag_To_Redraw with simpler
 *  battle UI redraw semantics.
 *
 *  Classic redraws all visible strips, while the tabbed view redraws only
 *  the active strip.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Flag_To_Redraw()
{
    IsToRedraw = true;
    BattleUI.Get_Sidebar().Flag_Strip_To_Redraw();
    Request_Sidebar_Redraw(Map);
}


/**
 *  Reference to last gadget that the user has hovered their mouse cursor on.
 */
static GadgetClass* LastHovered;


/**
 *  Function for checking which gadget has been hovered over.
 *
 *  @author: ZivDero
 */
static void Check_Hover(GadgetClass* gadget, int mousex, int mousey)
{
    GadgetClass* to_enter = gadget->Extract_Gadget_At_Mouse(mousex, mousey);
    if (to_enter != LastHovered)
    {
        if (LastHovered)
        {
            if (auto select = dynamic_cast<CameoButtonClass*>(LastHovered))
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
            if (auto select = dynamic_cast<CameoButtonClass*>(to_enter))
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


/**
 *  Patch in GadgetClass::Input to handle hover effects for SelectClass.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_GadgetClass_Input_Mouse_Enter_Leave)
{
    GET_REGISTER_STATIC(int, key, EAX);
    GET_REGISTER_STATIC(int, mousex, EBP);
    GET_REGISTER_STATIC(int, mousey, EBX);
    GET_REGISTER_STATIC(unsigned, flags, EDI);
    GET_REGISTER_STATIC(GadgetClass*, this_ptr, ESI);

    _asm push eax
    _asm push edx
    Check_Hover(this_ptr, mousex, mousey);
    _asm pop edx
    _asm pop eax

    // Stolen code

    /**
     *  Set the mouse button state flags. These will be passed to the individual
     *  buttons so that they can determine what action to perform (if any).
     */
    flags = 0;
    if (key)
    {
        if (key == KN_LMOUSE)
            flags |= GadgetClass::LEFTPRESS;

        if (key == KN_RMOUSE)
            flags |= GadgetClass::RIGHTPRESS;

        if (key == (KN_LMOUSE | KN_RLSE_BIT))
            flags |= GadgetClass::LEFTRELEASE;

        if (key == (KN_RMOUSE | KN_RLSE_BIT))
            flags |= GadgetClass::RIGHTRELEASE;

        /**
         *  If the mouse wasn't responsible for this key code, then it must be from
         *  the keyboard. Flag this fact.
         */
        if (!flags)
            flags |= GadgetClass::KEYBOARD;

        _asm mov edi, flags
        JMP_REG(ecx, 0x004A9F7F);
    }

    _asm mov edi, flags
    JMP_REG(ecx, 0x004A9F4D);
}


/**
 *  Main function for patching the hooks.
 */
void SidebarClassExtension_Hooks()
{
    Patch_Jump(0x005F2610, &SidebarClassExt::_One_Time);
    Patch_Jump(0x005F2660, &SidebarClassExt::_Init_Clear);
    Patch_Jump(0x005F2720, &SidebarClassExt::_Init_IO);
    Patch_Jump(0x005F2900, &SidebarClassExt::_Init_For_House);
    Patch_Jump(0x005F2B00, &SidebarClassExt::_Init_Strips);
    Patch_Jump(0x005F2C30, &SidebarClassExt::_Which_Column);
    Patch_Jump(0x005F2C50, &SidebarClassExt::_Factory_Link);
    Patch_Jump(0x005F2E20, &SidebarClassExt::_Add);
    Patch_Jump(0x005F2E90, &SidebarClassExt::_Scroll);
    Patch_Jump(0x005F30F0, &SidebarClassExt::_Scroll_Page);
    Patch_Jump(0x005F3560, &SidebarClassExt::_Draw_It);
    Patch_Jump(0x005F3C70, &SidebarClassExt::_AI);
    Patch_Jump(0x005F3E20, &SidebarClassExt::_Recalc);
    Patch_Jump(0x005F3E60, &SidebarClassExt::_Activate);
    Patch_Jump(0x005F6080, &SidebarClassExt::_Set_Dimensions);
    Patch_Jump(0x005F6620, &SidebarClassExt::_Help_Text);
    Patch_Jump(0x005F6670, &SidebarClassExt::_Max_Visible);
    Patch_Jump(0x005F5F70, &SidebarClassExt::_Abandon_Production);
    Patch_Jump(0x005F48F0, &StripClassExt::_Flag_To_Redraw);

    Patch_Jump(0x004A9F0F, _GadgetClass_Input_Mouse_Enter_Leave);

    // NOP away tooltip length check for formatting
    Patch_Byte(0x0044E486, 0x90);
    Patch_Byte(0x0044E486 + 1, 0x90);

    // Change jle to jl to allow rendering tooltips that are exactly as wide as the sidebar
    Patch_Byte(0x0044E605 + 1, 0x8C);

    Patch_Jump(0x004E5C70, 0x004E5D4A); // Don't add LSidebarUpCommandClass and RSidebarUpCommandClass
    Patch_Jump(0x004E5DB7, 0x004E5E91); // Don't add LSidebarDownCommandClass and RSidebarDownCommandClass
    Patch_Jump(0x004E5EFE, 0x004E5FD8); // Don't add LSidebarPageUpCommandClass and RSidebarPageUpCommandClass
    Patch_Jump(0x004E6045, 0x004E611F); // Don't add LSidebarPageDownCommandClass and RSidebarPageDownCommandClass
}
