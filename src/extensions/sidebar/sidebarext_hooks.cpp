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
#include "sidebarext_hooks.h"

#include <algorithm>

#include "bsurface.h"
#include "buildingtype.h"
#include "convert.h"
#include "drawshape.h"
#include "event.h"
#include "extension.h"
#include "factory.h"
#include "fetchres.h"
#include "house.h"
#include "housetype.h"
#include "language.h"
#include "mouse.h"
#include "playmovie.h"
#include "rules.h"
#include "scenarioext.h"
#include "session.h"
#include "sidebar.h"
#include "sidebarext.h"
#include "spritecollection.h"
#include "super.h"
#include "supertype.h"
#include "supertypeext.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "textprint.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "unittypeext.h"
#include "voc.h"
#include "vox.h"
#include "wwmouse.h"
#include "sideext.h"
#include "debughandler.h"
#include "fatal.h"
#include "asserthandler.h"
#include "building.h"
#include "eventext.h"
#include "factoryext.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "houseext.h"
#include "msgbox.h"
#include "newsidebar.h"
#include "optionsext.h"
#include "rulesext.h"
#include "sidebar.h"
#include "uicontrol.h"
#include "vinifera_globals.h"


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
    void _Fake_Flag_To_Redraw_Special();
    void _Fake_Flag_To_Redraw_Current();
};


/**
 *  Reimplements the entire SidebarClass::One_Time function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_One_Time()
{
    PowerClass::One_Time();

    NewSidebarClass::StripClass::One_Time();
}


/**
 *  Reimplements the entire SidebarClass::Init_Clear function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Init_Clear()
{
    PowerClass::Init_Clear();

    IsToRedraw = true;
    IsRepairActive = false;
    IsUpgradeActive = false;
    IsUpgradeActive = false;

    //Sidebar->Init_Clear();
}


/**
 *  Reimplements the entire SidebarClass::Init_IO function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Init_IO()
{
    PowerClass::Init_IO();

    Sidebar->Init_IO();
}


/**
 *  Reimplements the entire SidebarClass::Init_For_House function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Init_For_House()
{
    PowerClass::Init_For_House();

    Sidebar->Init_For_House();
}


/**
 *  Reimplements the entire SidebarClass::Init_Strips function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Init_Strips()
{
    
}


/**
 *  Reimplements the entire SidebarClass::Factory_Link function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Factory_Link(FactoryClass* factory, RTTIType type, int id)
{
    return Sidebar->Factory_Link(factory, type, id);
}


/**
 *  Reimplements the entire SidebarClass::Add function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Add(RTTIType type, int id)
{
    return Sidebar->Add(type, id);
}


/**
 *  Reimplements the entire SidebarClass::Activate function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Activate(int control)
{
    return Sidebar->Activate(control);
}


/**
 *  Reimplements the entire SidebarClass::Scroll function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Scroll(bool up, int column)
{
    return Sidebar->Scroll(up, column);
}


/**
 *  Reimplements the entire SidebarClass::Scroll_Page function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Scroll_Page(bool up, int column)
{
    return Sidebar->Page(up, column);
}


/**
 *  Reimplements the entire SidebarClass::AI function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_AI(KeyNumType& input, Point2D& xy)
{
    Sidebar->AI(input, xy);
}


/**
 *  Reimplements the entire SidebarClass::Draw_It function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Draw_It(bool complete)
{
    Sidebar->Draw_It(complete);
}


/**
 *  Reimplements the entire SidebarClass::Recalc function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Recalc()
{
    Sidebar->Recalc();
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
    Sidebar->Set_Dimensions();
}


/**
 *  Reimplements the entire SidebarClass::Help_Text function.
 *
 *  @author: ZivDero
 */
const char* SidebarClassExt::_Help_Text(int gadget_id)
{
    return Sidebar->Help_Text(gadget_id);
}


/**
 *  Reimplements the entire SidebarClass::Max_Visible function.
 *
 *  @author: ZivDero
 */
int SidebarClassExt::_Max_Visible()
{
    return NewSidebarClass::Max_Visible();
}


/**
 *  Fake function to patch calls to redraw a specific vanilla strip.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Fake_Flag_To_Redraw_Special()
{
    Sidebar->Get_Column(RTTI_SPECIAL, PRODFLAG_NONE).Flag_To_Redraw();
}


/**
 *  Fake function to patch calls to redraw a specific vanilla strip.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Fake_Flag_To_Redraw_Current()
{
    Sidebar->Current_Tab().Flag_To_Redraw();
}


/**
 *  Patch in GadgetClass::Input to handle hover effects for SelectClass.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_GadgetClass_Input_Mouse_Enter_Leave)
{
    GET_REGISTER_STATIC(int, key, eax);
    GET_REGISTER_STATIC(int, mousex, ebp);
    GET_REGISTER_STATIC(int, mousey, ebx);
    GET_REGISTER_STATIC(unsigned, flags, edi);
    GET_REGISTER_STATIC(GadgetClass*, this_ptr, esi);

    _asm push eax
    _asm push edx
    IHoverableGadget::Process_Hover(this_ptr, mousex, mousey);
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
    Patch_Jump(0x005F5F70, &SidebarClassExt::_Abandon_Production);
    Patch_Jump(0x005F2610, &SidebarClassExt::_One_Time);
    Patch_Jump(0x005F2660, &SidebarClassExt::_Init_Clear);
    Patch_Jump(0x005F2720, &SidebarClassExt::_Init_IO);
    Patch_Jump(0x005F2900, &SidebarClassExt::_Init_For_House);
    Patch_Jump(0x005F2B00, &SidebarClassExt::_Init_Strips);
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

    Patch_Jump(0x004A9F0F, _GadgetClass_Input_Mouse_Enter_Leave);

    // There are a bunch of calls to vanilla strips to redraw them.
    // We patch them to either redraw the supers' strip or the current strip
    Patch_Call(0x00458ADB, &StripClassExt::_Fake_Flag_To_Redraw_Special);
    Patch_Call(0x004BD32D, &StripClassExt::_Fake_Flag_To_Redraw_Special);
    Patch_Call(0x004CB585, &StripClassExt::_Fake_Flag_To_Redraw_Special);
    Patch_Call(0x004CB6F8, &StripClassExt::_Fake_Flag_To_Redraw_Special);
    Patch_Call(0x00619F9A, &StripClassExt::_Fake_Flag_To_Redraw_Special);
    Patch_Call(0x0061C09C, &StripClassExt::_Fake_Flag_To_Redraw_Special);
    Patch_Call(0x0061C0FD, &StripClassExt::_Fake_Flag_To_Redraw_Special);

    Patch_Call(0x004BD1E0, &StripClassExt::_Fake_Flag_To_Redraw_Current);
    Patch_Call(0x004BD1EA, &StripClassExt::_Fake_Flag_To_Redraw_Current);
    Patch_Call(0x004C9859, &StripClassExt::_Fake_Flag_To_Redraw_Current);
    Patch_Call(0x004C9863, &StripClassExt::_Fake_Flag_To_Redraw_Current);

    Patch_Jump(0x004E5C70, 0x004E5D4A); // Don't add LSidebarUpCommandClass and RSidebarUpCommandClass
    Patch_Jump(0x004E5DB7, 0x004E5E91); // Don't add LSidebarDownCommandClass and RSidebarDownCommandClass
    Patch_Jump(0x004E5EFE, 0x004E5FD8); // Don't add LSidebarPageUpCommandClass and RSidebarPageUpCommandClass
    Patch_Jump(0x004E6045, 0x004E611F); // Don't add LSidebarPageDownCommandClass and RSidebarPageDownCommandClass

    // NOP away tooltip length check for formatting
    Patch_Byte(0x0044E486, 0x90);
    Patch_Byte(0x0044E486 + 1, 0x90);

    // Change jle to jl to allow rendering tooltips that are exactly as wide as the sidebar
    Patch_Byte(0x0044E605 + 1, 0x8C);
}
