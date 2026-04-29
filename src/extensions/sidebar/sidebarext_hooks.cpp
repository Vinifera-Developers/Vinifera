/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended SidebarClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sidebarext_hooks.h"

#include "battleui.h"
#include "building.h"
#include "debughandler.h"
#include "extension.h"
#include "factory.h"
#include "fatal.h"
#include "hooker.h"
#include "house.h"
#include "language.h"
#include "mouse.h"
#include "msgbox.h"
#include "optionsext.h"
#include "playmovie.h"
#include "rules.h"
#include "session.h"
#include "sidebar.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "voc.h"
#include "vox.h"
#include "wwmouse.h"


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
    void _Shift_Sidebar();
    const char* _Help_Text(int gadget_id);
    int _Max_Visible();
    int _Which_Column(RTTIType type);
    void _Blit_Sidebar(bool complete);
};


static int& _dialog_count = Make_Global<int>(0x007E492C);


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
        BattleUI.Shift_Sidebar();

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
    BattleUI.Get_Sidebar().Shift_Sidebar();
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

    /**
     *  Only if there is a change in the state of the sidebar will anything
     *  be done to change it.
     */
    if (IsSidebarActive != old) {
        /**
         *  If the sidebar is activated but was on the right side of the screen, then
         *  activate it on the left side of the screen.
         */
        if (IsSidebarActive) {
            Shift_Sidebar();
            IsToRedraw = true;

            RadarButton.Zap();
            Add_A_Button(RadarButton);

            BattleUI.Get_Sidebar().Activate(true);
        } else {
            End_Ingame_Movie();

            Remove_A_Button(RadarButton);

            BattleUI.Get_Sidebar().Activate(false);
        }

        Flag_To_Redraw(GS_REDRAW_ALL);
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
    if (_dialog_count != 0) {
        return false;
    }

    if (BattleUI.Get_Sidebar().Scroll(up, column)) {
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
    RadarClass::Draw_It(complete);

    BattleUI.Draw();
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
 *  Reimplements the entire SidebarClass::Shift_Sidebar function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Shift_Sidebar()
{
    SidebarRect.X = Options.SidebarSide ? TacticalRect.X + TacticalRect.Width : 0;
    SidebarRect.Y = 148;
    SidebarRect.Width = 168;
    SidebarRect.Height = TacticalRect.Y + TacticalRect.Height - 148;

    RadarClass::Shift_Sidebar();

    BattleUI.Shift_Sidebar();
}


/**
 *  Reimplements the entire SidebarClass::Help_Text function.
 *
 *  @author: ZivDero
 */
const char* SidebarClassExt::_Help_Text(int gadget_id)
{
    const char* text = RadarClass::Help_Text(gadget_id);
    if (text == nullptr) {
        text = BattleUI.Help_Text(gadget_id);
    }
    return text;
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
    return BattleUI.Get_Sidebar().Get_Model().Which_Category(type, 0);
}


/**
 *  Reimplements the entire SidebarClass::Blit_Sidebar function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Blit_Sidebar(bool)
{
    if (IsSidebarActive && GameActive && ScenarioActive) {

        /**
         *  Blit the entire sidebar surface.
         */
        Rect sb_rect = SidebarSurface->Get_Rect();
        VisibleSurface->Blit_From(sb_rect + Point2D(TacticalRect.Width, 0), *SidebarSurface, sb_rect);
    }
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
    Patch_Jump(0x005F6080, &SidebarClassExt::_Shift_Sidebar);
    Patch_Jump(0x005F6620, &SidebarClassExt::_Help_Text);
    Patch_Jump(0x005F6670, &SidebarClassExt::_Max_Visible);
    Patch_Jump(0x005F5F70, &SidebarClassExt::_Abandon_Production);
    Patch_Jump(0x005F38C0, &SidebarClassExt::_Blit_Sidebar);

    // NOP away tooltip length check for formatting
    Patch_Byte(0x0044E486, 0x90);
    Patch_Byte(0x0044E486 + 1, 0x90);

    // Change jle to jl to allow rendering tooltips that are exactly as wide as the sidebar
    Patch_Byte(0x0044E605 + 1, 0x8C);
}
