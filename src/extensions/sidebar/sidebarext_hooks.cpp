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

#include "debughandler.h"
#include "fatal.h"
#include "asserthandler.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "optionsext.h"
#include "vinifera_globals.h"


/**
  *  A fake class for implementing new member functions which allow
  *  access to the "this" pointer of the intended class.
  *
  *  @note: This must not contain a constructor or deconstructor!
  *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
  */
static class SidebarClassExt final : public SidebarClass
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
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class StripClassExt final : public SidebarClass::StripClass
{
public:
    void _One_Time(int id);
    void _Init_IO(int id);
    void _Init_For_House(int id);
    void _Activate();
    void _Deactivate();
    bool _Scroll(bool up);
    bool _Scroll_Page(bool up);
    bool _AI(KeyNumType& input, Point2D& xy);
    const char* _Help_Text(int gadget_id);
    void _Draw_It(bool complete);
    bool _Factory_Link(FactoryClass* factory, RTTIType type, int id);
    void _Tab_Button_AI();
    void _Fake_Flag_To_Redraw_Special();
    void _Fake_Flag_To_Redraw_Current();
};


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_SidebarClass_Constructor_Patch)
{
    GET_REGISTER_STATIC(SidebarClass*, this_ptr, esi); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    SidebarExtension = Extension::Singleton::Make<SidebarClass, SidebarClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
    _asm
    {
        mov eax, this_ptr
        pop edi
        pop esi
        pop ebp
        pop ebx
        add esp, 14h
        ret
    }
}


/**
 *  Patch for including the extended class members in the destruction process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_SidebarClass_Destructor_Patch)
{
    //GET_REGISTER_STATIC(SidebarClass*, this_ptr, edi);

    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<SidebarClass, SidebarClassExtension>(SidebarExtension);

    /**
     *  Stolen bytes here.
     */
    _asm
    {
        pop edi
        pop esi
        pop ebp
        pop ebx
        ret
    }
}


/**
 *  Reimplements the entire SidebarClass::One_Time function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_One_Time()
{
    PowerClass::One_Time();

    for (int i = 0; i < SidebarClassExtension::SIDEBAR_TAB_COUNT; i++)
        SidebarExtension->Column[i].One_Time(i);

    /*
    **  Load the sidebar shapes in at this time.
    */
    StripClass::RechargeClockShape = MFCC::RetrieveT<ShapeFileStruct>("RCLOCK2.SHP");
    StripClass::ClockShape = MFCC::RetrieveT<ShapeFileStruct>("GCLOCK2.SHP");
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

    SidebarExtension->TabIndex = SidebarClassExtension::SIDEBAR_TAB_STRUCTURE;

    for (int i = 0; i < SidebarClassExtension::SIDEBAR_TAB_COUNT; i++)
        SidebarExtension->Column[i].Init_Clear();

    Activate(0);
}


/**
 *  Reimplements the entire SidebarClass::Init_IO function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Init_IO()
{
    PowerClass::Init_IO();

    SidebarRect.X = TacticalRect.Width + TacticalRect.X;
    SidebarRect.Y = 148;
    SidebarRect.Width = 641 - (TacticalRect.Width + TacticalRect.X);
    SidebarRect.Height = TacticalRect.Height + TacticalRect.Y - SidebarRect.Y;

    /*
    ** Add the sidebar's buttons only if we're not in editor mode.
    */
    if (!Debug_Map)
    {
        Repair.X = TacticalRect.Width + TacticalRect.X;
        Sell.X = TacticalRect.Width + TacticalRect.X + 27;
        Power.X = TacticalRect.Width + TacticalRect.X + 54;
        Waypoint.X = TacticalRect.Width + TacticalRect.X + 81;

        Repair.IsSticky = true;
        Repair.ID = BUTTON_REPAIR;
        Repair.Y = 148;
        Repair.DrawX = -480;
        Repair.DrawY = 3;
        Repair.DrawnOnSidebarSurface = true;
        Repair.ShapeDrawer = SidebarDrawer;
        Repair.IsPressed = false;
        Repair.IsToggleType = true;
        Repair.ReflectButtonState = true;

        Sell.IsSticky = true;
        Sell.ID = BUTTON_SELL;
        Sell.Y = 148;
        Sell.DrawX = -480;
        Sell.DrawY = 3;
        Sell.DrawnOnSidebarSurface = true;
        Sell.ShapeDrawer = SidebarDrawer;
        Sell.IsPressed = false;
        Sell.IsToggleType = true;
        Sell.ReflectButtonState = true;

        Power.IsSticky = true;
        Power.ID = BUTTON_POWER;
        Power.Y = 148;
        Power.DrawX = -480;
        Power.DrawY = 3;
        Power.DrawnOnSidebarSurface = true;
        Power.ShapeDrawer = SidebarDrawer;
        Power.IsPressed = false;
        Power.IsToggleType = true;
        Power.ReflectButtonState = true;

        Waypoint.IsSticky = true;
        Waypoint.ID = BUTTON_WAYPOINT;
        Waypoint.Y = 148;
        Waypoint.DrawX = -480;
        Waypoint.DrawY = 3;
        Waypoint.DrawnOnSidebarSurface = true;
        Waypoint.ShapeDrawer = SidebarDrawer;
        Waypoint.IsPressed = false;
        Waypoint.IsToggleType = true;
        Waypoint.ReflectButtonState = true;
        Waypoint.Enable();

        SidebarExtension->Init_IO();

        for (int i = 0; i < SidebarClassExtension::SIDEBAR_TAB_COUNT; i++)
            SidebarExtension->Column[i].Init_IO(i);

        Set_Dimensions();

        /*
        ** If a game was loaded & the sidebar was enabled, pop it up now.
        */
        if (IsSidebarActive)
        {
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
    PowerClass::Init_For_House();

    PaletteClass pal("SIDEBAR.PAL");

    delete SidebarDrawer;
    SidebarDrawer = new ConvertClass(&pal, &pal, PrimarySurface, 1);

    Sell.Set_Shape(MFCC::RetrieveT<ShapeFileStruct>("SELL.SHP"));
    Sell.ShapeDrawer = SidebarDrawer;

    Power.Set_Shape(MFCC::RetrieveT<ShapeFileStruct>("POWER.SHP"));
    Power.ShapeDrawer = SidebarDrawer;

    Waypoint.Set_Shape(MFCC::RetrieveT<ShapeFileStruct>("WAYP.SHP"));
    Waypoint.ShapeDrawer = SidebarDrawer;

    Repair.Set_Shape(MFCC::RetrieveT<ShapeFileStruct>("REPAIR.SHP"));
    Repair.ShapeDrawer = SidebarDrawer;

    SidebarShape = MFCC::RetrieveT<ShapeFileStruct>("SIDE1.SHP");
    SidebarMiddleShape = MFCC::RetrieveT<ShapeFileStruct>("SIDE2.SHP");
    SidebarBottomShape = MFCC::RetrieveT<ShapeFileStruct>("SIDE3.SHP");
    SidebarAddonShape = MFCC::RetrieveT<ShapeFileStruct>("ADDON.SHP");

    SidebarExtension->Init_For_House();

    for (int i = 0; i < SidebarClassExtension::SIDEBAR_TAB_COUNT; ++i)
        static_cast<StripClassExt*>(&SidebarExtension->Column[i])->_Init_For_House(i);
}


/**
 *  Reimplements the entire SidebarClass::Init_Strips function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Init_Strips()
{
    SidebarExtension->Init_Strips();
}


/**
 *  Reimplements the entire SidebarClass::Factory_Link function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Factory_Link(FactoryClass* factory, RTTIType type, int id)
{
    return SidebarExtension->Get_Tab(type).Factory_Link(factory, type, id);
}


/**
 *  Comparison function for sorting sidebar icons (BuildTypes)
 *
 *  @author: Rampastring, ZivDero
 */
int __cdecl BuildType_Comparison(const void* p1, const void* p2)
{
    auto firstSide = [](unsigned owners) -> int
        {
            int side = INT_MAX;

            for (int i = 0; i < HouseTypes.Count(); i++)
            {
                if (owners & (1 << i))
                {
                    if (HouseTypes[i]->Side < side)
                        side = HouseTypes[i]->Side;
                }
            }

            return side != INT_MAX ? side : SIDE_NONE;
        };

    auto isSideOwner = [](const HouseClass* house, unsigned owners) -> int
        {
            // The house owns the object directly
            if (owners & 1 << house->ActLike)
                return true;

            const SideType side = house->Class->Side;
            for (int i = 0; i < HouseTypes.Count(); i++)
            {
                if ((owners & 1 << i) && HouseTypes[i]->Side == side)
                    return true;
            }

            return false;
        };


    const auto bt1 = static_cast<const SidebarClass::StripClass::BuildType*>(p1);
    const auto bt2 = static_cast<const SidebarClass::StripClass::BuildType*>(p2);

    if (bt1->BuildableType == bt2->BuildableType)
    {
        /**
         *  If both are SWs, the one that recharges quicker goes first,
         *  otherwise sort by ID.
         */
        if (bt1->BuildableType == RTTI_SPECIAL || bt1->BuildableType == RTTI_SUPERWEAPONTYPE)
        {
            if (SuperWeaponTypes[bt1->BuildableID]->RechargeTime != SuperWeaponTypes[bt2->BuildableID]->RechargeTime)
                return SuperWeaponTypes[bt1->BuildableID]->RechargeTime - SuperWeaponTypes[bt2->BuildableID]->RechargeTime;

            return bt1->BuildableID - bt2->BuildableID;
        }


        const TechnoTypeClass* t1 = Fetch_Techno_Type(bt1->BuildableType, bt1->BuildableID);
        const TechnoTypeClass* t2 = Fetch_Techno_Type(bt2->BuildableType, bt2->BuildableID);

        /**
         *  If both are Buildings, non-defenses come first, then walls, then gates, then base defenses
         */
        if (bt1->BuildableType == RTTI_BUILDINGTYPE && OptionsExtension->SortDefensesAsLast)
        {
            const auto b1 = static_cast<const BuildingTypeClass*>(t1), b2 = static_cast<const BuildingTypeClass*>(t2);

            const auto ext1 = Extension::Fetch<TechnoTypeClassExtension>(t1);
            const auto ext2 = Extension::Fetch<TechnoTypeClassExtension>(t2);

            enum
            {
                BCAT_NORMAL,
                BCAT_WALL,
                BCAT_GATE,
                BCAT_DEFENSE
            };

            int building_category1 = (b1->IsWall || b1->IsFirestormWall || b1->IsLaserFencePost || b1->IsLaserFence) ? BCAT_WALL : (b1->IsGate ? BCAT_GATE : (ext1->SortCameoAsBaseDefense ? BCAT_DEFENSE : BCAT_NORMAL));
            int building_category2 = (b2->IsWall || b2->IsFirestormWall || b2->IsLaserFencePost || b2->IsLaserFence) ? BCAT_WALL : (b2->IsGate ? BCAT_GATE : (ext2->SortCameoAsBaseDefense ? BCAT_DEFENSE : BCAT_NORMAL));

            // Compare based on category priority
            if (building_category1 != building_category2)
                return building_category1 - building_category2;
        }

        /**
         *  If your side owns one of the objects, but not another, yours comes first
         */
        const int owns1 = isSideOwner(PlayerPtr, t1->Get_Ownable()),
            owns2 = isSideOwner(PlayerPtr, t2->Get_Ownable());

        if (owns1 != owns2)
            return owns2 - owns1;

        /**
         *  If you don't own either of the objects, then sort by side index
         */
        if (!owns1 && !owns2)
        {
            const int side1 = firstSide(t1->Get_Ownable()),
                side2 = firstSide(t2->Get_Ownable());

            if (side1 != side2)
                return side1 - side2;
        }

        /**
         *  If both are Units, non-naval units come first
         */
        //if (bt1->BuildableType == RTTI_UNITTYPE)
        //{
        //    const auto ext1 = Extension::Fetch<UnitTypeClassExtension>(t1);
        //    const auto ext2 = Extension::Fetch<UnitTypeClassExtension>(t2);

        //    if (ext1->IsNaval != ext2->IsNaval)
        //        return (int)ext1->IsNaval - (int)ext2->IsNaval;
        //}

        return bt1->BuildableID - bt2->BuildableID;
    }

    if (bt1->BuildableType == RTTI_SPECIAL || bt1->BuildableType == RTTI_SUPERWEAPONTYPE)
        return -1;

    if (bt2->BuildableType == RTTI_SPECIAL || bt2->BuildableType == RTTI_SUPERWEAPONTYPE)
        return 1;

    if (bt1->BuildableType == RTTI_INFANTRYTYPE)
        return -1;

    if (bt2->BuildableType == RTTI_INFANTRYTYPE)
        return 1;

    if (bt1->BuildableType == RTTI_UNITTYPE)
        return -1;

    if (bt2->BuildableType == RTTI_UNITTYPE)
        return 1;

    if (bt1->BuildableType == RTTI_AIRCRAFTTYPE)
        return -1;

    if (bt2->BuildableType == RTTI_AIRCRAFTTYPE)
        return 1;

    return bt1->BuildableID - bt2->BuildableID;
}


/**
 *  Reimplements the entire SidebarClass::Add function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Add(RTTIType type, int id)
{
    if (!Debug_Map)
    {
        if (SidebarExtension->Get_Tab(type).Add(type, id))
        {
            Activate(1);
            IsToRedraw = true;
            Flag_To_Redraw(false);
            qsort(&SidebarExtension->Get_Tab(type).Buildables, SidebarExtension->Get_Tab(type).BuildableCount, sizeof(StripClass::BuildType), &BuildType_Comparison);
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

    if (Session.Play && !Session.Singleplayer_Game())
        return old;

    /*
    **	Determine the new state of the sidebar.
    */
    switch (control)
    {
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

    /*
    **	Only if there is a change in the state of the sidebar will anything
    **	be done to change it.
    */
    if (IsSidebarActive != old)
    {
        /*
        **	If the sidebar is activated but was on the right side of the screen, then
        **	activate it on the left side of the screen.
        */
        if (IsSidebarActive)
        {
            Set_Dimensions();
            IsToRedraw = true;
            Repair.Zap();
            Add_A_Button(Repair);
            Sell.Zap();
            Add_A_Button(Sell);
            Power.Zap();
            Add_A_Button(Power);
            Waypoint.Zap();
            Add_A_Button(Waypoint);
            SidebarExtension->Current_Tab().Activate();
            Background.Zap();
            Add_A_Button(Background);
            for (int i = 0; i < SidebarClassExtension::SIDEBAR_TAB_COUNT; i++)
            {
                SidebarExtension->TabButtons[i].Zap();
                Add_A_Button(SidebarExtension->TabButtons[i]);
            }
            RadarButton.Zap();
            Add_A_Button(RadarButton);
        }
        else
        {
            End_Ingame_Movie();
            Remove_A_Button(Repair);
            Remove_A_Button(Sell);
            Remove_A_Button(Power);
            Remove_A_Button(Waypoint);
            Remove_A_Button(Background);
            for (int i = 0; i < SidebarClassExtension::SIDEBAR_TAB_COUNT; i++)
            {
                SidebarExtension->Column[i].Deactivate();
                Remove_A_Button(SidebarExtension->TabButtons[i]);
            }
            Remove_A_Button(RadarButton);
        }

        /*
        **	Since the sidebar status has changed, update the map so that the graphics
        **	will be rendered correctly.
        */
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
    if (*reinterpret_cast<int*>(0x007E492C))
        return false;

    bool scr = SidebarExtension->Current_Tab().Scroll(up);

    if (scr)
    {
        IsToRedraw = true;
        Flag_To_Redraw(false);
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
    bool scr = SidebarExtension->Current_Tab().Scroll_Page(up);

    if (scr)
    {
        IsToRedraw = true;
        Flag_To_Redraw(false);
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
    if (!Debug_Map)
    {
        Activate(1);
        // The original code deducts the X coordinate by 480 here. Why? No one knows, but let's do the same
        // 480 also appears as the draw offset of the sidebar buttons
        Point2D newpoint(xy.X - 480, xy.Y);
        for (int i = 0; i < SidebarClassExtension::SIDEBAR_TAB_COUNT; i++)
            SidebarExtension->Column[i].AI(input, newpoint);
    }

    if (IsSidebarActive)
    {
        /*
        **	If there are any buildings in the player's inventory, then allow the repair
        **	option.
        */

        if (PlayerPtr->CurBuildings > 0)
        {
            Activate_Repair(true);
        }
        else
        {
            Activate_Repair(false);
        }

        if (input == (BUTTON_REPAIR | KN_BUTTON))
        {
            Repair_Mode_Control(-1);
        }

        if (input == (BUTTON_POWER | KN_BUTTON))
        {
            Power_Mode_Control(-1);
        }

        if (input == (BUTTON_WAYPOINT | KN_BUTTON))
        {
            Waypoint_Mode_Control(-1, false);
        }

        if (input == (BUTTON_SELL | KN_BUTTON))
        {
            Sell_Mode_Control(-1);
        }

        if (input == (SidebarClassExtension::BUTTON_TAB_1 | KN_BUTTON))
        {
            SidebarExtension->Change_Tab(SidebarClassExtension::SIDEBAR_TAB_STRUCTURE);
        }

        if (input == (SidebarClassExtension::BUTTON_TAB_2 | KN_BUTTON))
        {
            SidebarExtension->Change_Tab(SidebarClassExtension::SIDEBAR_TAB_INFANTRY);
        }

        if (input == (SidebarClassExtension::BUTTON_TAB_3 | KN_BUTTON))
        {
            SidebarExtension->Change_Tab(SidebarClassExtension::SIDEBAR_TAB_UNIT);
        }

        if (input == (SidebarClassExtension::BUTTON_TAB_4 | KN_BUTTON))
        {
            SidebarExtension->Change_Tab(SidebarClassExtension::SIDEBAR_TAB_SPECIAL);
        }
    }

    if (!IsRepairMode && Repair.IsOn)
    {
        Repair.Turn_Off();
    }

    if (!IsSellMode && Sell.IsOn)
    {
        Sell.Turn_Off();
    }

    if (!IsPowerMode && Power.IsOn)
    {
        Power.Turn_Off();
    }

    if (!IsWaypointMode && Waypoint.IsOn)
    {
        Waypoint.Turn_Off();
    }

    /*
    **	If for some reason the current tab's button is not selected, select it
    */
    if (!SidebarExtension->TabButtons[SidebarExtension->TabIndex].IsSelected)
        SidebarExtension->TabButtons[SidebarExtension->TabIndex].Select();

    /*
    **	If our current tab no longer has any buildables, try to change to one that has some
    */
    if (SidebarExtension->Current_Tab().BuildableCount < 1)
    {
        SidebarClassExtension::SidebarTabType newtab = SidebarExtension->First_Active_Tab();
        if (newtab != SidebarClassExtension::SIDEBAR_TAB_NONE)
            SidebarExtension->Change_Tab(newtab);
    }

    PowerClass::AI(input, xy);
}


/**
 *  Reimplements the entire SidebarClass::Draw_It function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Draw_It(bool complete)
{
    complete |= IsToFullRedraw;
    Map.field_1214 = Rect();
    PowerClass::Draw_It(complete);

    DSurface* oldsurface = LogicSurface;
    LogicSurface = SidebarSurface;

    Rect rect(0, 0, SidebarSurface->Get_Width(), SidebarSurface->Get_Height());

    if (IsSidebarActive && (IsToRedraw || complete) && !Debug_Map)
    {
        if (complete || SidebarExtension->Current_Tab().IsToRedraw)
        {
            Point2D xy(0, SidebarRect.Y);
            CC_Draw_Shape(SidebarSurface, SidebarDrawer, SidebarShape, 0, &xy, &rect, SHAPE_WIN_REL, 0, 0, ZGRAD_GROUND, 1000, nullptr, 0, 0, 0);

            int max_visible = SidebarClassExtension::Max_Visible(true);
            int y = SidebarRect.Y + SidebarShape->Get_Height();

            for (int i = 0; i < max_visible; i++, y += SidebarMiddleShape->Get_Height())
            {
                xy = Point2D(0, y);
                CC_Draw_Shape(SidebarSurface, SidebarDrawer, SidebarMiddleShape, 0, &xy, &rect, SHAPE_WIN_REL, 0, 0, ZGRAD_GROUND, 1000, nullptr, 0, 0, 0);
            }

            xy = Point2D(0, y);
            CC_Draw_Shape(SidebarSurface, SidebarDrawer, SidebarBottomShape, 0, &xy, &rect, SHAPE_WIN_REL, 0, 0, ZGRAD_GROUND, 1000, nullptr, 0, 0, 0);

            xy = Point2D(0, y + SidebarBottomShape->Get_Height());
            CC_Draw_Shape(SidebarSurface, SidebarDrawer, SidebarAddonShape, 0, &xy, &rect, SHAPE_WIN_REL, 0, 0, ZGRAD_GROUND, 1000, nullptr, 0, 0, 0);

            SidebarExtension->Current_Tab().IsToRedraw = true;
        }

        Repair.Draw_Me(true);
        Sell.Draw_Me(true);
        Power.Draw_Me(true);
        Waypoint.Draw_Me(true);

        RedrawSidebar = true;
    }

    /*
    **	Since the tabs might be blinking, draw them all the time.
    */
    for (int i = 0; i < SidebarClassExtension::SIDEBAR_TAB_COUNT; i++)
        SidebarExtension->TabButtons[i].Draw_Me(true);

    /*
    **	Draw the side strip elements by calling their respective draw functions.
    */
    if (IsSidebarActive)
    {
        SidebarExtension->Current_Tab().Draw_It(complete);
    }

    if (Repair.IsDrawn)
    {
        RedrawSidebar = true;
        Repair.IsDrawn = false;
    }

    if (Sell.IsDrawn)
    {
        RedrawSidebar = true;
        Sell.IsDrawn = false;
    }

    if (Power.IsDrawn)
    {
        RedrawSidebar = true;
        Power.IsDrawn = false;
    }

    if (Waypoint.IsDrawn)
    {
        RedrawSidebar = true;
        Waypoint.IsDrawn = false;
    }

    for (int i = 0; i < SidebarClassExtension::SIDEBAR_TAB_COUNT; i++)
    {
        if (SidebarExtension->TabButtons[i].IsDrawn)
        {
            RedrawSidebar = true;
            SidebarExtension->TabButtons[i].IsDrawn = false;
        }
    }

    if (ToolTipHandler)
        ToolTipHandler->Force_Redraw(true);

    IsToRedraw = false;
    IsToFullRedraw = false;
    Blit_Sidebar(complete);
    LogicSurface = oldsurface;
}


/**
 *  Reimplements the entire SidebarClass::Recalc function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Recalc()
{
    bool redraw = false;
    for (int i = 0; i < SidebarClassExtension::SIDEBAR_TAB_COUNT; i++)
        redraw |= SidebarExtension->Column[i].Recalc();

    if (redraw)
    {
        IsToRedraw = true;
        Flag_To_Redraw();
    }
}


/**
 *  Reimplements the entire SidebarClass::Abandon_Production function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Abandon_Production(RTTIType type, FactoryClass* factory)
{
    return SidebarExtension->Get_Tab(type).Abandon_Production(factory);
}


/**
 *  Reimplements the entire SidebarClass::Set_Dimensions function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Set_Dimensions()
{
    /*
    **  Position the sidebar itself.
    */

    SidebarRect.X = Options.SidebarOn ? TacticalRect.X + TacticalRect.Width : 0;
    SidebarRect.Y = 148;
    SidebarRect.Width = 168;
    SidebarRect.Height = TacticalRect.Y + TacticalRect.Height - 148;

    PowerClass::Set_Dimensions();

    if (!SidebarShape)
    {
        SidebarShape = MFCC::RetrieveT<ShapeFileStruct>("SIDEGDI1.SHP");
        SidebarMiddleShape = MFCC::RetrieveT<ShapeFileStruct>("SIDEGDI2.SHP");
        SidebarBottomShape = MFCC::RetrieveT<ShapeFileStruct>("SIDEGDI3.SHP");
    }

    /*
    **  Position the sidebar's buttons.
    */

    Background.Set_Position(SidebarRect.X + 16, TacticalRect.Y);
    Background.Flag_To_Redraw();

    Repair.Set_Position(SidebarRect.X + SidebarClassExtension::BUTTON_REPAIR_X_OFFSET, SidebarRect.Y + BUTTON_REPAIR_Y_OFFSET);
    Repair.Flag_To_Redraw();
    Repair.DrawX = -SidebarRect.X;

    Sell.Set_Position(Repair.X + BUTTON_SELL_X_OFFSET, Repair.Y);
    Sell.Flag_To_Redraw();
    Sell.DrawX = -SidebarRect.X;

    Power.Set_Position(Sell.X + BUTTON_POWER_X_OFFSET, Sell.Y);
    Power.Flag_To_Redraw();
    Power.DrawX = -SidebarRect.X;

    Waypoint.Set_Position(Power.X + BUTTON_WAYPOINT_X_OFFSET, Power.Y);
    Waypoint.Flag_To_Redraw();
    Waypoint.DrawX = -SidebarRect.X;

    SidebarExtension->Set_Dimensions();

    /*
    **  Create the tooltips for the sidebar.
    */

    if (ToolTipHandler)
    {
        ToolTip tooltip;

        for (int i = 0; i < 100; i++)
        {
            ToolTipHandler->Remove(1000 + i);
        }

        int max_visible = SidebarClassExtension::Max_Visible();

        StripClass::UpButton[0].Set_Position(SidebarRect.X + COLUMN_ONE_X + SidebarClassExtension::UP_X_OFFSET, SidebarRect.Y + StripClass::OBJECT_HEIGHT * max_visible / 2 + SidebarClassExtension::UP_Y_OFFSET);
        StripClass::UpButton[0].Flag_To_Redraw();
        StripClass::UpButton[0].DrawX = -SidebarRect.X;
        StripClass::DownButton[0].Set_Position(SidebarRect.X + COLUMN_TWO_X + SidebarClassExtension::DOWN_X_OFFSET, SidebarRect.Y + StripClass::OBJECT_HEIGHT * max_visible / 2 + SidebarClassExtension::DOWN_Y_OFFSET);
        StripClass::DownButton[0].Flag_To_Redraw();
        StripClass::DownButton[0].DrawX = -SidebarRect.X;

        for (int tab = 0; tab < SidebarClassExtension::SIDEBAR_TAB_COUNT; tab++)
        {
            for (int i = 0; i < max_visible; i++)
            {
                const int x = SidebarRect.X + ((i % 2 == 0) ? COLUMN_ONE_X : COLUMN_TWO_X);
                const int y = SidebarRect.Y + SidebarClassExtension::COLUMN_Y + ((i / 2) * StripClass::OBJECT_HEIGHT);
                SidebarExtension->SelectButton[tab][i].Set_Position(x, y);
            }
        }

        for (int i = 0; i < max_visible; i++)
        {
            tooltip.Region = Rect(SidebarExtension->SelectButton[0][i].X, SidebarExtension->SelectButton[0][i].Y, SidebarExtension->SelectButton[0][i].Width, SidebarExtension->SelectButton[0][i].Height);
            tooltip.ID = 1000 + i;
            tooltip.Text = TXT_NONE;
            ToolTipHandler->Add(&tooltip);
        }

        tooltip.Region = Rect(Repair.X, Repair.Y, Repair.Width, Repair.Height);
        tooltip.ID = BUTTON_REPAIR;
        tooltip.Text = TXT_REPAIR_MODE;
        ToolTipHandler->Remove(tooltip.ID);
        ToolTipHandler->Add(&tooltip);

        tooltip.Region = Rect(Power.X, Power.Y, Power.Width, Power.Height);
        tooltip.ID = BUTTON_POWER;
        tooltip.Text = TXT_POWER_MODE;
        ToolTipHandler->Remove(tooltip.ID);
        ToolTipHandler->Add(&tooltip);

        tooltip.Region = Rect(Sell.X, Sell.Y, Sell.Width, Sell.Height);
        tooltip.ID = BUTTON_SELL;
        tooltip.Text = TXT_SELL_MODE;
        ToolTipHandler->Remove(tooltip.ID);
        ToolTipHandler->Add(&tooltip);

        tooltip.Region = Rect(Waypoint.X, Waypoint.Y, Waypoint.Width, Waypoint.Height);
        tooltip.ID = BUTTON_WAYPOINT;
        tooltip.Text = TXT_WAYPOINTMODE;
        ToolTipHandler->Remove(tooltip.ID);
        ToolTipHandler->Add(&tooltip);
    }

    Background.Set_Position(Options.SidebarOn ? TacticalRect.X + TacticalRect.Width : 0, RadarButton.Height + RadarButton.Y);
    Background.Set_Size(SidebarSurface->Get_Width(), SidebarSurface->Get_Height() - RadarButton.Height + RadarButton.Y);
}


/**
 *  Reimplements the entire SidebarClass::Help_Text function.
 *
 *  @author: ZivDero
 */
const char* SidebarClassExt::_Help_Text(int gadget_id)
{
    const char* text = PowerClass::Help_Text(gadget_id);
    if (text == nullptr)
    {
        /*
        **	New help text for sidebar tabs gets returned here
        */
        switch (gadget_id)
        {
        case SidebarClassExtension::BUTTON_TAB_1:
            return SidebarExtension->TabButtons[SidebarClassExtension::SIDEBAR_TAB_STRUCTURE].Is_Enabled() ? "Structures Tab" : "Structures Tab@(Disabled)";
        case SidebarClassExtension::BUTTON_TAB_2:
            return SidebarExtension->TabButtons[SidebarClassExtension::SIDEBAR_TAB_INFANTRY].Is_Enabled() ? "Infantry Tab" : "Infantry Tab@(Disabled)";
        case SidebarClassExtension::BUTTON_TAB_3:
            return SidebarExtension->TabButtons[SidebarClassExtension::SIDEBAR_TAB_UNIT].Is_Enabled() ? "Vehicles Tab" : "Vehicles Tab@(Disabled)";
        case SidebarClassExtension::BUTTON_TAB_4:
            return SidebarExtension->TabButtons[SidebarClassExtension::SIDEBAR_TAB_SPECIAL].Is_Enabled() ? "Specials Tab" : "Specials Tab@(Disabled)";
        default:
            break;
        }

        /*
        **	If it's a SelectClass, get the help text for the buildable
        */
        const int id = gadget_id - 1000;
        if (id >= 0 && id < SidebarExtension->Current_Tab().BuildableCount)
            return SidebarExtension->Current_Tab().Help_Text(gadget_id - 1000);
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
    return SidebarClassExtension::Max_Visible(true);
}


/**
 *  Reimplements the entire SidebarClass::StripClass::One_Time function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_One_Time(int id)
{
    DarkenShape = MFCC::RetrieveT<ShapeFileStruct>("DARKEN.SHP");
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Init_IO function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Init_IO(int id)
{
    ID = id;

    UpButton[0].IsSticky = true;
    UpButton[0].ID = BUTTON_UP;
    UpButton[0].DrawnOnSidebarSurface = true;
    UpButton[0].ShapeDrawer = SidebarDrawer;
    UpButton[0].Flags = GadgetClass::RIGHTRELEASE | GadgetClass::RIGHTPRESS | GadgetClass::LEFTRELEASE | GadgetClass::LEFTPRESS;

    DownButton[0].IsSticky = true;
    DownButton[0].ID = BUTTON_DOWN;
    DownButton[0].DrawnOnSidebarSurface = true;
    DownButton[0].ShapeDrawer = SidebarDrawer;
    DownButton[0].Flags = GadgetClass::RIGHTRELEASE | GadgetClass::RIGHTPRESS | GadgetClass::LEFTRELEASE | GadgetClass::LEFTPRESS;

    int max_visible = SidebarClassExtension::Max_Visible();
    for (int index = 0; index < max_visible; index++)
    {
        SelectClass& g = SidebarExtension->SelectButton[ID][index];
        g.ID = BUTTON_SELECT;
        g.X = SidebarRect.X + ((index % 2 == 0) ? SidebarClass::COLUMN_ONE_X : SidebarClass::COLUMN_TWO_X);
        g.Y = SidebarRect.Y + SidebarClassExtension::COLUMN_Y + ((index / 2) * OBJECT_HEIGHT);
        g.Width = OBJECT_WIDTH;
        g.Height = OBJECT_HEIGHT;
        g.Set_Owner(*this, index);
    }
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Init_For_House function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Init_For_House(int id)
{
    UpButton[0].Set_Shape(MFCC::RetrieveT<ShapeFileStruct>("R-UP.SHP"));
    UpButton[0].ShapeDrawer = SidebarDrawer;

    DownButton[0].Set_Shape(MFCC::RetrieveT<ShapeFileStruct>("R-DN.SHP"));
    DownButton[0].ShapeDrawer = SidebarDrawer;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Activate function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Activate()
{
    UpButton[0].Zap();
    Map.Add_A_Button(UpButton[0]);

    DownButton[0].Zap();
    Map.Add_A_Button(DownButton[0]);

    int max_visible = SidebarClassExtension::Max_Visible();
    for (int index = 0; index < max_visible; index++)
    {
        SidebarExtension->SelectButton[ID][index].Zap();
        Map.Add_A_Button(SidebarExtension->SelectButton[ID][index]);
    }
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Deactivate function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Deactivate()
{
    Map.Remove_A_Button(UpButton[0]);
    Map.Remove_A_Button(DownButton[0]);

    int max_visible = SidebarClassExtension::Max_Visible();
    for (int index = 0; index < max_visible; index++)
    {
        Map.Remove_A_Button(SidebarExtension->SelectButton[ID][index]);
    }
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Scroll function.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_Scroll(bool up)
{
    if (up)
    {
        if (!TopIndex)
            return false;
        Scroller--;
    }
    else
    {
        if (TopIndex + SidebarClassExtension::Max_Visible() >= BuildableCount + BuildableCount % 2)
            return false;
        Scroller++;
    }

    return true;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Scroll_Page function.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_Scroll_Page(bool up)
{
    if (up)
    {
        if (!TopIndex)
            return false;
        Scroller -= SidebarClassExtension::Max_Visible(true);
    }
    else
    {
        if (TopIndex + SidebarClassExtension::Max_Visible() >= BuildableCount + BuildableCount % 2)
            return false;
        Scroller += SidebarClassExtension::Max_Visible(true);
    }
    return true;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::AI function.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_AI(KeyNumType& input, Point2D&)
{
    bool redraw = false;

    _Tab_Button_AI();

    /*
    **	Handle any building clock animation logic.
    */
    if (IsBuilding)
    {
        for (int index = 0; index < BuildableCount; index++)
        {
            FactoryClass* factory = Buildables[index].Factory;
            if (factory && factory->Has_Changed())
            {
                redraw = true;
                if (factory->Has_Completed())
                {
                    /*
                    **	Construction has been completed. Announce this fact to the player and
                    **	try to get the object to automatically leave the factory. Buildings are
                    **	the main exception to the ability to leave the factory under their own
                    **	power.
                    */
                    TechnoClass* pending = factory->Get_Object();
                    if (pending != nullptr)
                    {
                        switch (pending->Kind_Of())
                        {
                        case RTTI_UNIT:
                        case RTTI_AIRCRAFT:
                            OutList.Add(EventClass(pending->Owner(), EVENT_PLACE, pending->Kind_Of(), &INVALID_CELL));
                            Speak(VOX_UNIT_READY);
                            break;

                        case RTTI_BUILDING:
                            SidebarExtension->TabButtons[ID].Start_Flashing();
                            Speak(VOX_CONSTRUCTION);
                            break;

                        case RTTI_INFANTRY:
                            OutList.Add(EventClass(pending->Owner(), EVENT_PLACE, pending->Kind_Of(), &INVALID_CELL));
                            Speak(VOX_UNIT_READY);
                            break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    /*
    **	If this is not the currently active tab, return and do not redraw.
    */
    if (SidebarExtension->TabIndex != ID)
        return false;

    /*
    **	If this is scroll button for this side strip, then scroll the strip as
    **	indicated.
    */
    if (input == (UpButton[0].ID | KN_BUTTON))
    {
        UpButton[0].IsPressed = false;
        if (!Scroll(true))
            Sound_Effect(Rule->ScoldSound);
    }
    if (input == (DownButton[0].ID | KN_BUTTON))
    {
        DownButton[0].IsPressed = false;
        if (!Scroll(false))
            Sound_Effect(Rule->ScoldSound);
    }

    /*
    **	Reflect the scroll desired direction/value into the scroll
    **	logic handler. This might result in up or down scrolling.
    */
    if (!IsScrolling && Scroller)
    {
        if (BuildableCount <= SidebarClassExtension::Max_Visible())
        {
            Scroller = 0;
        }
        else
        {
            /*
            **	Top of list is moving toward lower ordered entries in the object list. It looks like
            **	the "window" to the object list is moving up even though the actual object images are
            **	scrolling downward.
            */
            if (Scroller < 0)
            {
                if (!TopIndex)
                {
                    Scroller = 0;
                }
                else
                {
                    Scroller++;
                    IsScrollingDown = false;
                    IsScrolling = true;
                    TopIndex -= 2;
                    Slid = 0;
                }

            }
            else
            {
                if (TopIndex + SidebarClassExtension::Max_Visible() > BuildableCount)
                {
                    Scroller = 0;
                }
                else
                {
                    Scroller--;
                    Slid = OBJECT_HEIGHT;
                    IsScrollingDown = true;
                    IsScrolling = true;
                }
            }
        }
    }

    /*
    **	Scroll logic is handled here.
    */
    if (IsScrolling)
    {
        if (IsScrollingDown)
        {
            Slid -= SCROLL_RATE;
            if (Slid <= 0)
            {
                IsScrolling = false;
                Slid = 0;
                TopIndex += 2;
            }
        }
        else
        {
            Slid += SCROLL_RATE;
            if (Slid >= OBJECT_HEIGHT)
            {
                IsScrolling = false;
                Slid = 0;
            }
        }
        redraw = true;
    }

    /*
    **	Handle any flashing logic. Flashing occurs when the player selects an object
    **	and provides the visual feedback of a recognized and legal selection.
    */
    if (Flasher != -1)
    {
        if (Graphic_Logic())
        {
            redraw = true;
            if (Fetch_Stage() >= 7)
            {
                Set_Rate(0);
                Set_Stage(0);
                Flasher = -1;
            }
        }
    }

    /*
    **	If any of the logic determined that this side strip needs to be redrawn, then
    **	set the redraw flag for this side strip.
    */
    if (redraw)
    {
        IsToRedraw = true;
        Flag_To_Redraw();
        RedrawSidebar = true;
    }

    return redraw;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Help_Text function.
 *
 *  @author: ZivDero
 */
const char* StripClassExt::_Help_Text(int gadget_id)
{
    static char _buffer[512];

    int i = gadget_id + TopIndex;

    if (GameActive)
    {
        if (i < BuildableCount && BuildableCount < MAX_BUILDABLES)
        {
            if (Buildables[i].BuildableType == RTTI_SPECIAL)
                return SuperWeaponTypes[Buildables[i].BuildableID]->Full_Name();

            const TechnoTypeClass* ttype = Fetch_Techno_Type(Buildables[i].BuildableType, Buildables[i].BuildableID);

            // Bugfix from YR.
            if (!ttype)
                return nullptr;

            /**
             *  Adds support for extended sidebar tooltips.
             *
             *  @author: Rampastring
             */
            const TechnoTypeClassExtension* technotypeext = Extension::Fetch<TechnoTypeClassExtension>(ttype);
            const char* description = technotypeext->Description;

            if (description[0] == '\0')
            {
                // If there is no extended description, then simply show the name and price.
                std::snprintf(_buffer, sizeof(_buffer), "%s@$%d", ttype->Full_Name(), ttype->Cost_Of(PlayerPtr));
            }
            else
            {
                // If there is an extended description, then show the name, price, and the description.
                std::snprintf(_buffer, sizeof(_buffer), "%s@$%d@@%s", ttype->Full_Name(), ttype->Cost_Of(PlayerPtr), technotypeext->Description);
            }

            return _buffer;
        }
    }

    return nullptr;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Draw_It function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Draw_It(bool complete)
{
    if (IsToRedraw || complete)
    {
        IsToRedraw = false;
        RedrawSidebar = true;

        Rect rect = Rect(0, SidebarRect.Y, SidebarRect.Width, SidebarRect.Height);

        /*
        **	Redraw the scroll buttons.
        */
        UpButton[0].Draw_Me(true);
        DownButton[0].Draw_Me(true);

        int maxvisible = SidebarClassExtension::Max_Visible();

        /*
        **	Loop through all the buildable objects that are visible in the strip and render
        **	them. Their Y offset may be adjusted if the strip is in the process of scrolling.
        */
        for (int i = 0; i < maxvisible + (IsScrolling ? 1 : 0); i++)
        {
            bool production = false;
            bool completed = false;
            int  stage = 0;
            bool darken = false;
            ShapeFileStruct const* shapefile = nullptr;
            BSurface* image_surface = nullptr;
            FactoryClass* factory = nullptr;
            int index = i + TopIndex;
            int x = i % 2 == 0 ? SidebarClass::COLUMN_ONE_X : SidebarClass::COLUMN_TWO_X;
            int y = SidebarClassExtension::COLUMN_Y + ((i / 2) * OBJECT_HEIGHT);

            bool isready = false;
            const char* state = nullptr;
            const char* name = nullptr;
            TechnoTypeClass const* obj = nullptr;

            /*
            **	If the strip is scrolling, then the offset is adjusted accordingly.
            */
            if (IsScrolling)
            {
                y -= OBJECT_HEIGHT - Slid;
            }

            /*
            **	Fetch the shape number for the object type located at this current working
            **	slot. This shape pointer is used to draw the underlying graphic there.
            */
            if (index < BuildableCount)
            {
                SpecialWeaponType spc = SPECIAL_NONE;

                if (Buildables[index].BuildableType != RTTI_SPECIAL)
                {
                    obj = Fetch_Techno_Type(Buildables[index].BuildableType, Buildables[index].BuildableID);
                    if (obj != nullptr)
                    {
                        name = obj->FullName;
                        darken = false;

                        /*
                        **	If there is already a factory producing a building, then all
                        **	buildings are displayed in a disabled state.
                        */
                        if (obj->Kind_Of() == RTTI_BUILDINGTYPE)
                        {
                            darken = PlayerPtr->Fetch_Factory(Buildables[index].BuildableType) != nullptr;
                        }

                        /*
                        **	If there is no factory that can produce this, or the factory that
                        *	can produce this is currently busy,
                        **	objects of this type are displayed in a disabled state.
                        */
                        if (!obj->Who_Can_Build_Me(true, true, true, PlayerPtr)
                            || (!darken && PlayerPtr->Can_Build(Fetch_Techno_Type(Buildables[index].BuildableType, Buildables[index].BuildableID), false, false) == -1))
                        {
                            darken = true;
                        }

                        shapefile = obj->Get_Cameo_Data();
                        auto technotypeext = Extension::Fetch<TechnoTypeClassExtension>(obj);
                        if (technotypeext->CameoImageSurface != nullptr)
                            image_surface = technotypeext->CameoImageSurface;

                        factory = Buildables[index].Factory;
                        if (factory != nullptr)
                        {
                            production = true;
                            completed = factory->Has_Completed();
                            if (completed)
                            {
                                /*
                                **	Display text showing that the object is ready to place.
                                */
                                state = Fetch_String(TXT_READY);
                            }
                            stage = factory->Completion();
                            darken = false;
                        }
                        else
                        {
                            production = false;
                        }
                    }
                    else
                    {
                        shapefile = LogoShape;
                    }

                }
                else
                {
                    spc = (SpecialWeaponType)Buildables[index].BuildableID;

                    name = SuperWeaponTypes[spc]->FullName;
                    shapefile = Get_Special_Cameo(spc);
                    auto supertypeext = Extension::Fetch<SuperWeaponTypeClassExtension>(PlayerPtr->SuperWeapon[spc]->Class);
                    if (supertypeext->CameoImageSurface != nullptr)
                        image_surface = supertypeext->CameoImageSurface;

                    production = true;
                    completed = !PlayerPtr->SuperWeapon[spc]->Needs_Redraw();
                    isready = PlayerPtr->SuperWeapon[spc]->Is_Ready();
                    state = PlayerPtr->SuperWeapon[spc]->Ready_String();
                    stage = PlayerPtr->SuperWeapon[spc]->Anim_Stage();
                    darken = false;

                    if (spc == SPECIAL_NONE)
                    {
                        shapefile = LogoShape;
                    }
                }
            }
            else
            {
                shapefile = LogoShape;
                production = false;
            }

            /*
            **	Now that the shape of the object at the current working slot has been found,
            **	draw it and any graphic overlays as necessary.
            */
            if (shapefile != LogoShape)
            {
                Point2D drawpoint(x, y);

                /**
                 *  #issue-487
                 *
                 *  Adds support for PCX/PNG cameo icons.
                 *
                 *  @author: CCHyper
                 */
                if (image_surface != nullptr)
                {
                    Rect pcxrect(rect.X + drawpoint.X, rect.Y + drawpoint.Y, image_surface->Get_Width(), image_surface->Get_Height());
                    SpriteCollection.Draw(pcxrect, *SidebarSurface, *image_surface);
                }
                else if (shapefile != nullptr)
                {
                    CC_Draw_Shape(SidebarSurface, CameoDrawer, shapefile,
                        0, &drawpoint, &rect, SHAPE_WIN_REL, 0, 0, ZGRAD_GROUND, 1000, nullptr, 0, 0, 0);
                }


                /*
                **	Draw a selection box around the cameo if we're currently hovering over it
                **	and it is available.
                */
                bool overbutton = SidebarExtension->SelectButton[ID][index - TopIndex].MousedOver;

                if (overbutton && !Scen->UserInputLocked && !darken)
                {
                    Rect cameo_hover_rect(x, SidebarRect.Y + y, OBJECT_WIDTH, OBJECT_HEIGHT - 3);
                    SidebarSurface->Draw_Rect(cameo_hover_rect, DSurface::RGB_To_Pixel(ColorSchemes[0]->HSV.operator RGBClass()));
                }


                /*
                **	Darken this object because it cannot be produced or is otherwise
                **	unavailable.
                */
                if (darken)
                {
                    CC_Draw_Shape(SidebarSurface, SidebarDrawer, DarkenShape,
                        0, &drawpoint, &rect, SHAPE_WIN_REL | SHAPE_DARKEN, 0, 0, ZGRAD_GROUND, 1000, nullptr, 0, 0, 0);
                }
            }

            if (name != nullptr)
            {
                Point2D drawpoint(x, y + OBJECT_NAME_OFFSET);
                Print_Cameo_Text(name, drawpoint, rect, OBJECT_WIDTH);
            }

            /*
            **	Draw the number of queued objects
            */
            bool hasqueuecount = false;
            if (obj != nullptr)
            {
                RTTIType rtti = obj->Kind_Of();
                FactoryClass* factory = PlayerPtr->Fetch_Factory(rtti);

                if (factory != nullptr)
                {
                    int total = factory->Total_Queued(*obj);
                    if (total > 1 ||
                        total > 0 && (factory->Object == nullptr ||
                            factory->Object->Techno_Type_Class() != nullptr && factory->Object->Techno_Type_Class() != obj))
                    {
                        Point2D drawpoint(x + QUEUE_COUNT_X_OFFSET, y + TEXT_Y_OFFSET);
                        Fancy_Text_Print("%d", SidebarSurface, &rect, &drawpoint, ColorScheme::As_Pointer("LightGrey", 1), COLOR_TBLACK, TPF_RIGHT | TPF_FULLSHADOW | TPF_8POINT, total);
                        hasqueuecount = true;
                    }
                }
            }

            /*
            **	Draw the overlapping clock shape if this is object is being constructed.
            **	If the object is completed, then display "Ready" with no clock shape.
            */
            if (production)
            {
                if (state != nullptr)
                {
                    Point2D drawpoint(x + TEXT_X_OFFSET + 3, y + TEXT_Y_OFFSET);
                    Fancy_Text_Print(state, SidebarSurface, &rect, &drawpoint, ColorScheme::As_Pointer("LightBlue", 1), COLOR_TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_8POINT);
                }

                if (!completed)
                {
                    int shapenum;
                    const ShapeFileStruct* shape;
                    Point2D drawpoint;

                    if (isready)
                    {
                        shapenum = stage + 1;
                        drawpoint = Point2D(x, y);
                        shape = RechargeClockShape;
                    }
                    else
                    {
                        shapenum = stage + 1;
                        drawpoint = Point2D(x, y);
                        shape = ClockShape;
                    }

                    CC_Draw_Shape(SidebarSurface, SidebarDrawer, shape,
                        shapenum, &drawpoint, &rect, SHAPE_WIN_REL | SHAPE_TRANS50, 0, 0, ZGRAD_GROUND, 1000, nullptr, 0, 0, 0);

                    /*
                    **	Display text showing that the construction is temporarily on hold.
                    */
                    if (factory && !factory->Is_Building())
                    {
                        if (hasqueuecount)
                        {
                            Point2D drawpoint2(x, y + TEXT_Y_OFFSET);
                            Fancy_Text_Print(TXT_HOLD, SidebarSurface, &rect, &drawpoint2, ColorScheme::As_Pointer("LightGrey", 1), COLOR_TBLACK, TPF_FULLSHADOW | TPF_8POINT);
                        }
                        else
                        {
                            Point2D drawpoint2(x + TEXT_X_OFFSET, y + TEXT_Y_OFFSET);
                            Fancy_Text_Print(TXT_HOLD, SidebarSurface, &rect, &drawpoint2, ColorScheme::As_Pointer("LightGrey", 1), COLOR_TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_8POINT);
                        }
                    }
                }
            }

        }

        LastSlid = Slid;
        return;
    }

    if (UpButton[0].IsDrawn)
    {
        RedrawSidebar = true;
        UpButton[0].IsDrawn = false;
    }

    if (DownButton[0].IsDrawn)
    {
        RedrawSidebar = true;
        DownButton[0].IsDrawn = false;
    }
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Factory_Link function.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_Factory_Link(FactoryClass* factory, RTTIType type, int id)
{
    for (int i = 0; i < BuildableCount; i++)
    {
        if (Buildables[i].BuildableType == type &&
            Buildables[i].BuildableID == id)
        {
            Buildables[i].Factory = factory;
            IsBuilding = true;
            /*
            ** Flag that all the icons on this strip need to be redrawn
            */
            Flag_To_Redraw();
            return true;
        }
    }

    return false;
}


/**
 *  Handles the state of this strip's tab button.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Tab_Button_AI()
{
    if (BuildableCount > 0)
    {
        if (!SidebarExtension->TabButtons[ID].Is_Enabled())
            SidebarExtension->TabButtons[ID].Enable();

        int building_tab = SidebarClassExtension::Which_Tab(RTTI_BUILDINGTYPE);
        if (ID == building_tab)
        {
            if (SidebarExtension->TabButtons[ID].IsFlashing)
            {
                FactoryClass* fptr = PlayerPtr->Fetch_Factory(RTTI_BUILDINGTYPE);
                if (fptr == nullptr || !fptr->Has_Completed())
                    SidebarExtension->TabButtons[ID].Stop_Flashing();
            }
        }

        int special_tab = SidebarClassExtension::Which_Tab(RTTI_SPECIAL);
        if (ID == special_tab)
        {
            bool ready_sw = false;
            for (int i = 0; i < PlayerPtr->SuperWeapon.Count(); i++)
            {
                SuperClass* sw = PlayerPtr->SuperWeapon[i];
                if (sw->Is_Ready() && !sw->Class->IsUseChargeDrain) // Firestorm is always "ready", so don't flash for it.
                {
                    ready_sw = true;
                    break;
                }
            }

            if (ready_sw && !SidebarExtension->TabButtons[ID].IsFlashing)
                SidebarExtension->TabButtons[ID].Start_Flashing();
            else if (!ready_sw && SidebarExtension->TabButtons[ID].IsFlashing)
                SidebarExtension->TabButtons[ID].Stop_Flashing();
        }
    }
    else
    {
        if (SidebarExtension->TabButtons[ID].Is_Enabled())
            SidebarExtension->TabButtons[ID].Disable();
    }
}


/**
 *  Fake function to patch calls to redraw a specific vanilla strip.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Fake_Flag_To_Redraw_Special()
{
    SidebarExtension->Get_Tab(RTTI_SPECIAL).Flag_To_Redraw();
}


/**
 *  Fake function to patch calls to redraw a specific vanilla strip.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Fake_Flag_To_Redraw_Current()
{
    SidebarExtension->Current_Tab().Flag_To_Redraw();
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
    SidebarClassExtension::Check_Hover(this_ptr, mousex, mousey);
    _asm pop edx
    _asm pop eax

    // Stolen code

    /*
    **	Set the mouse button state flags. These will be passed to the individual
    **	buttons so that they can determine what action to perform (if any).
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

        /*
        **	If the mouse wasn't responsible for this key code, then it must be from
        **	the keyboard. Flag this fact.
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
 *  Moves the power bar to accomodate for the taller SIDE1.SHP.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_PowerClass_Draw_It_Move_Power_Bar)
{
    static int y;
    y = SidebarRect.Y + SidebarClassExtension::COLUMN_Y - 1;
    _asm mov esi, y

    static int max_visible;
    max_visible = SidebarClassExtension::Max_Visible(true);
    _asm
    {
        mov eax, max_visible
        mov ecx, max_visible
    }

    JMP_REG(ebx, 0x005AB4D9);
}


/**
 *  Patch SelectClass to redraw the new strips.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_SelectClass_Action_Redraw_Column)
{
    SidebarExtension->Current_Tab().IsToRedraw = true;
    JMP(0x005F5C95);
}


static const ObjectTypeClass *_SidebarClass_StripClass_obj = nullptr;
static const SuperWeaponTypeClass *_SidebarClass_StripClass_spc = nullptr;
static BSurface *_SidebarClass_StripClass_CustomImage = nullptr;


/**
 *  #issue-487
 * 
 *  Adds support for PCX/PNG cameo icons.
 * 
 *  The following two patches store the PCX/PNG image for the factory object or special.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SidebarClass_StripClass_ObjectTypeClass_Custom_Cameo_Image_Patch)
{
    GET_REGISTER_STATIC(const ObjectTypeClass *, obj, ebp);
    static const TechnoTypeClassExtension *technotypeext;
    static const ShapeFileStruct *shapefile;

    shapefile = obj->Get_Cameo_Data();

    _SidebarClass_StripClass_obj = obj;
    _SidebarClass_StripClass_CustomImage = nullptr;

    technotypeext = Extension::Fetch<TechnoTypeClassExtension>(reinterpret_cast<const TechnoTypeClass *>(obj));
    if (technotypeext->CameoImageSurface) {
        _SidebarClass_StripClass_CustomImage = technotypeext->CameoImageSurface;
    }

    _asm { mov eax, shapefile }

    JMP_REG(ebx, 0x005F5193);
}

DECLARE_PATCH(_SidebarClass_StripClass_SuperWeaponType_Custom_Cameo_Image_Patch)
{
    GET_REGISTER_STATIC(const SuperWeaponTypeClass *, supertype, eax);
    static const SuperWeaponTypeClassExtension *supertypeext;
    static const ShapeFileStruct *shapefile;

    shapefile = supertype->SidebarIcon;

    _SidebarClass_StripClass_spc = supertype;
    _SidebarClass_StripClass_CustomImage = nullptr;

    supertypeext = Extension::Fetch<SuperWeaponTypeClassExtension>(supertype);
    if (supertypeext->CameoImageSurface) {
        _SidebarClass_StripClass_CustomImage = supertypeext->CameoImageSurface;
    }

    _asm { mov ebx, shapefile }

    JMP(0x005F5220);
}


/**
 *  #issue-487
 * 
 *  Adds support for PCX/PNG cameo icons.
 * 
 *  @author: CCHyper
 */
static Point2D pointxy;
static Rect pcxrect;
DECLARE_PATCH(_SidebarClass_StripClass_Custom_Cameo_Image_Patch)
{
    GET_STACK_STATIC(SidebarClass::StripClass *, this_ptr, esp, 0x24);
    LEA_STACK_STATIC(Rect *, window_rect, esp, 0x34);
    GET_REGISTER_STATIC(int, pos_x, edi);
    GET_REGISTER_STATIC(int, pos_y, esi);
    GET_REGISTER_STATIC(const ShapeFileStruct *, shapefile, ebx);
    static BSurface *image_surface;

    image_surface = nullptr;

    /**
     *  Was a factory object or special image found?
     */
    if (_SidebarClass_StripClass_CustomImage) {
        image_surface = _SidebarClass_StripClass_CustomImage;
    }

    /**
     *  Draw the cameo pcx image.
     */
    if (image_surface) {
        pcxrect.X = window_rect->X + pos_x;
        pcxrect.Y = window_rect->Y + pos_y;
        pcxrect.Width = image_surface->Get_Width();
        pcxrect.Height = image_surface->Get_Height();

        SpriteCollection.Draw(pcxrect, *SidebarSurface, *image_surface);

    /**
     *  Draw shape cameo image.
     */
    } else if (shapefile) {
        pointxy.X = pos_x;
        pointxy.Y = pos_y;

        CC_Draw_Shape(SidebarSurface, CameoDrawer, shapefile, 0, &pointxy, window_rect, SHAPE_WIN_REL|SHAPE_NORMAL);
    }

    _SidebarClass_StripClass_CustomImage = nullptr;

    /**
     *  Next, draw the clock darken shape.
     */
draw_darken_shape:
    JMP(0x005F52F3);
}


/**
 *  Main function for patching the hooks.
 */
void SidebarClassExtension_Hooks()
{
    Patch_Jump(0x005F23AC, &_SidebarClass_Constructor_Patch);
    Patch_Jump(0x005B8B7D, &_SidebarClass_Destructor_Patch);

    /**
     *  This patch is compatible with the vanilla sidebar.
     */
    Patch_Jump(0x005F4E40, &StripClassExt::_Help_Text);

    /**
     *  Legacy patches for the old sidebar.
     */
    Patch_Jump(0x005F5188, &_SidebarClass_StripClass_ObjectTypeClass_Custom_Cameo_Image_Patch);
    Patch_Jump(0x005F5216, &_SidebarClass_StripClass_SuperWeaponType_Custom_Cameo_Image_Patch);
    Patch_Jump(0x005F52AF, &_SidebarClass_StripClass_Custom_Cameo_Image_Patch);
}


/**
 *  Function for patching the hooks that require use to read VINIFERA.INI first.
 */
void SidebarClassExtension_Conditional_Hooks()
{
    if (Vinifera_NewSidebar)
    {
        Patch_Jump(0x005F2610, &SidebarClassExt::_One_Time);
        Patch_Jump(0x005F2660, &SidebarClassExt::_Init_Clear);
        Patch_Jump(0x005F2720, &SidebarClassExt::_Init_IO);
        Patch_Jump(0x005F2900, &SidebarClassExt::_Init_For_House);
        Patch_Jump(0x005F2B00, &SidebarClassExt::_Init_Strips);
        Patch_Jump(0x005F2C30, &SidebarClassExtension::Which_Tab);
        Patch_Jump(0x005F2C50, &SidebarClassExt::_Factory_Link);
        Patch_Jump(0x005F2E20, &SidebarClassExt::_Add);
        Patch_Jump(0x005F2E90, &SidebarClassExt::_Scroll);
        Patch_Jump(0x005F30F0, &SidebarClassExt::_Scroll_Page);
        Patch_Jump(0x005F3560, &SidebarClassExt::_Draw_It);
        Patch_Jump(0x005F3C70, &SidebarClassExt::_AI);
        Patch_Jump(0x005F3E20, &SidebarClassExt::_Recalc);
        Patch_Jump(0x005F3E60, &SidebarClassExt::_Activate);
        Patch_Jump(0x005F5F70, &SidebarClassExt::_Abandon_Production);
        Patch_Jump(0x005F6080, &SidebarClassExt::_Set_Dimensions);
        Patch_Jump(0x005F6620, &SidebarClassExt::_Help_Text);
        Patch_Jump(0x005F6670, &SidebarClassExt::_Max_Visible);

        Patch_Jump(0x005F4210, &StripClassExt::_One_Time);
        Patch_Jump(0x005F42A0, &StripClassExt::_Init_IO);
        Patch_Jump(0x005F4450, &StripClassExt::_Activate);
        Patch_Jump(0x005F4560, &StripClassExt::_Deactivate);
        Patch_Jump(0x005F46B0, &StripClassExt::_Scroll);
        Patch_Jump(0x005F4760, &StripClassExt::_Scroll_Page);
        Patch_Jump(0x005F4910, &StripClassExt::_AI);
        Patch_Jump(0x005F4F10, &StripClassExt::_Draw_It);
        Patch_Jump(0x005F5F10, &StripClassExt::_Factory_Link);

        Patch_Jump(0x004A9F0F, _GadgetClass_Input_Mouse_Enter_Leave);
        Patch_Jump(0x005AB4CF, _PowerClass_Draw_It_Move_Power_Bar);
        Patch_Jump(0x005F5C01, _SelectClass_Action_Redraw_Column);

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
    }
}
