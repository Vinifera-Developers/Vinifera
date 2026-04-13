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

#include "action_bar_component.h"
#include "sidebar_component.h"
#include "sidebar_tabbed_view.h"
#include "cameo_button.h"
#include "battleui_component.h"
#include "power_component.h"

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
    void _One_Time(int id);
    void _Init_IO(int id);
    void _Init_For_House(int id);
    bool _Recalc();
    void _Activate();
    bool _Add(RTTIType type, int id);
    void _Deactivate();
    bool _Scroll(bool up);
    bool _Scroll_Page(bool up);
    bool _AI(KeyNumType& input, Point2D const& xy);
    bool _AI_Vanilla(KeyNumType& input, Point2D const& xy);
    const char* _Help_Text(int gadget_id);
    void _Draw_It(bool complete);
    bool _Factory_Link(FactoryClass* factory, RTTIType type, int id);
    void _Fake_Flag_To_Redraw_Special();
    void _Fake_Flag_To_Redraw_Current();
};


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class SelectClassExt : public SidebarClass::StripClass::SelectClass
{
public:
    bool _Action(unsigned flags, KeyNumType& key);
};


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

    ActionBar.One_Time();
    Sidebar.One_Time();
    PowerBar.One_Time();
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
    Sidebar.Init_Strips();
}


/**
 *  Reimplements the entire SidebarClass::Factory_Link function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Factory_Link(FactoryClass* factory, RTTIType type, int id)
{
    return Sidebar.Factory_Link(factory, type, id);
}


/**
 *  Reimplements the entire SidebarClass::Add function.
 *
 *  @author: ZivDero
 */
bool SidebarClassExt::_Add(RTTIType type, int id)
{
    if (!Debug_Map) {
        if (Sidebar.Add(type, id)) {
            Activate(1);
            IsToRedraw = true;
            Flag_To_Redraw(false);
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

            ActionBar.Activate(1);
            RadarButton.Zap();
            Add_A_Button(RadarButton);

            Sidebar.Activate(1);
        } else {
            End_Ingame_Movie();

            ActionBar.Deactivate();
            Remove_A_Button(RadarButton);

            Sidebar.Activate(0);
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
    if (*reinterpret_cast<int*>(0x007E492C)) {
        return false;
    }

    if (Sidebar.Scroll(up, column)) {
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
    if (Sidebar.Scroll_Page(up, column)) {
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
}


/**
 *  Reimplements the entire SidebarClass::Recalc function.
 *
 *  @author: ZivDero
 */
void SidebarClassExt::_Recalc()
{
    Sidebar.Recalc();
    IsToRedraw = true;
    Flag_To_Redraw();
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
    return Sidebar.Max_Visible();
}


/**
 *  Reimplements the entire SidebarClass::Which_Column function.
 *
 *  @author: ZivDero
 */
int SidebarClassExt::_Which_Column(RTTIType type)
{
    return Sidebar.Get_Model().Route_To_Category(type, 0);
}


/**
 *  Reimplements the entire SidebarClass::StripClass::One_Time function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_One_Time(int id)
{
    (void)id;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Init_IO function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Init_IO(int id)
{
    (void)id;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Init_For_House function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Init_For_House(int id)
{
    UpButton[0].Set_Shape(MFCD::RetrieveT<ShapeSet>("R-UP.SHP"));
    UpButton[0].ShapeDrawer = SidebarDrawer;

    DownButton[0].Set_Shape(MFCD::RetrieveT<ShapeSet>("R-DN.SHP"));
    DownButton[0].ShapeDrawer = SidebarDrawer;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Recalc function.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_Recalc()
{
    bool ok;

    if (Debug_Map || !BuildableCount) {
        return false;
    }

    bool scroll = false;
    int max_visible = Sidebar.Max_Visible(false);
    BuildType* unshifted = new BuildType[max_visible];

    for (int i = 0; i < max_visible; i++) {
        if (i + TopIndex < BuildableCount) {
            unshifted[i] = Buildables[i + TopIndex];
        }
    }

    /**
     *  Sweep through all objects listed in the sidebar. If any of those object can
     *  not be created -- even in theory -- then they must be removed form the sidebar and
     *  any current production must be abandoned.
     */
    bool redraw = false;
    for (int index = 0; index < BuildableCount; index++) {
        TechnoTypeClass const* tech = Fetch_Techno_Type(Buildables[index].BuildableType, Buildables[index].BuildableID);
        if (tech != nullptr) {
            BuildingClass const* who = tech->Who_Can_Build_Me(true, false, false, PlayerPtr);
            ok = who != nullptr && who->House->Can_Build(tech, !RuleExtension->IsRecheckPrerequisites, true);
        }
        else {
            if (Buildables[index].BuildableID < PlayerPtr->SuperWeapon.Count()) {
                ok = PlayerPtr->SuperWeapon[Buildables[index].BuildableID]->Is_Present();
            }
            else {
                ok = false;
            }
        }

        if (!ok) {
            for (int i = 0; i < max_visible; i++) {
                if (unshifted[i] == Buildables[index]) {
                    unshifted[i] = BuildType(0, RTTI_NONE);
                }
            }

            /*
            **  Removes this entry from the list.
            */
            if (BuildableCount > 1 && index < BuildableCount - 1) {
                memmove(&Buildables[index], &Buildables[index + 1], sizeof(Buildables[0]) * (BuildableCount - index - 1));
            }
            redraw = true;
            scroll = true;
            Buildables[BuildableCount - 1].Factory = nullptr;
            IsToRedraw = true;
            BuildableCount--;
            index--;
        }
    }

    if (scroll) {
        bool got_old = false;
        bool got_new = false;

        int oldpos;
        for (oldpos = 0; oldpos < max_visible; oldpos++) {
            if (unshifted[oldpos] != BuildType(0, RTTI_NONE)) {
                got_old = true;
                break;
            }
        }

        int newpos;
        if (got_old && BuildableCount != 0) {
            for (newpos = 0; newpos < BuildableCount; newpos++) {
                if (Buildables[newpos] == unshifted[oldpos]) {
                    got_new = true;
                    break;
                }
            }
        }

        if (got_old && got_new) {
            TopIndex = newpos - oldpos;
            TopIndex = std::max(0, std::min(TopIndex, BuildableCount - max_visible));
        }
        else {
            TopIndex = 0;
        }
    }

    delete[] unshifted;
    return redraw;
}



/**
 *  Reimplements the entire SidebarClass::StripClass::Activate function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Activate()
{
}


/**
 *  Comparison function for sorting sidebar icons (BuildTypes)
 *
 *  @author: Rampastring, ZivDero
 */
static int __cdecl BuildType_Comparison(const void* p1, const void* p2)
{
    auto firstSide = [](unsigned owners) -> int
        {
            int side = INT_MAX;

            for (int i = 0; i < HouseTypes.Count(); i++)
            {
                if (owners & 1 << i)
                    side = std::min<int>(HouseTypes[i]->Side, side);
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
                if (owners & 1 << i && HouseTypes[i]->Side == side)
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

            const auto ext1 = Extension::Fetch(t1);
            const auto ext2 = Extension::Fetch(t2);

            enum
            {
                BCAT_NORMAL,
                BCAT_WALL,
                BCAT_GATE,
                BCAT_DEFENSE
            };

            int building_category1 = b1->IsWall || b1->IsFirestormWall || b1->IsLaserFencePost || b1->IsLaserFence ? BCAT_WALL : b1->IsGate ? BCAT_GATE : ext1->IsSortCameoAsBaseDefense ? BCAT_DEFENSE : BCAT_NORMAL;
            int building_category2 = b2->IsWall || b2->IsFirestormWall || b2->IsLaserFencePost || b2->IsLaserFence ? BCAT_WALL : b2->IsGate ? BCAT_GATE : ext2->IsSortCameoAsBaseDefense ? BCAT_DEFENSE : BCAT_NORMAL;

            // Compare based on category priority
            if (building_category1 != building_category2)
                return building_category1 - building_category2;
        }

        /**
         *  If both are Units, non-naval units come first
         */
        if (bt1->BuildableType == RTTI_UNITTYPE)
        {
            const auto ext1 = Extension::Fetch(t1);
            const auto ext2 = Extension::Fetch(t2);

            if (ext1->IsNaval != ext2->IsNaval)
                return static_cast<int>(ext1->IsNaval) - static_cast<int>(ext2->IsNaval);
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

    if (bt1->BuildableType == RTTI_AIRCRAFTTYPE)
        return -1;

    if (bt2->BuildableType == RTTI_AIRCRAFTTYPE)
        return 1;

    if (bt1->BuildableType == RTTI_UNITTYPE)
        return -1;

    if (bt2->BuildableType == RTTI_UNITTYPE)
        return 1;

    return bt1->BuildableID - bt2->BuildableID;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Add function.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_Add(RTTIType type, int id)
{
    if (BuildableCount < MAX_BUILDABLES)
    {
        for (int index = 0; index < BuildableCount; index++)
        {
            if (Buildables[index].BuildableType == type && Buildables[index].BuildableID == id)
                return false;
        }

        if (!ScenarioInit && type != RTTI_SPECIAL)
            Speak(VOX_NEW_CONSTRUCT);

        Buildables[BuildableCount].BuildableType = type;
        Buildables[BuildableCount].BuildableID = id;
        BuildableCount++;
        IsToRedraw = true;
        qsort(&Buildables, BuildableCount, sizeof(BuildType), &BuildType_Comparison);

        return true;
    }

    return false;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Deactivate function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Deactivate()
{
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Scroll function.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_Scroll(bool up)
{
    (void)up;
    return false;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Scroll_Page function.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_Scroll_Page(bool up)
{
    (void)up;
    return false;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::AI function.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_AI(KeyNumType& input, Point2D const&)
{
    (void)input;
    return false;
}


/**
 *  Reimplementation of SidebarClass::StripClass::AI, but for the vanilla sidebar.
 *  Used when the new sidebar is turned off.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_AI_Vanilla(KeyNumType& input, Point2D const& xy)
{
    KeyNumType key = KeyNumType(input & ~16384);
    bool redraw = false;

    /*
    **  If this is scroll button for this side strip, then scroll the strip as
    **  indicated.
    */
    if (key == KeyNumType(UpButton[ID].ID | KN_BUTTON)) {
        UpButton[ID].IsPressed = false;
        if ((input & 16384) != 0) {
            if (!Scroll_Page(true)) {
                Sound_Effect(Rule->ScoldSound);
            }
        } else {
            if (!Scroll(true)) {
                Sound_Effect(Rule->ScoldSound);
            }
        }
    } else if (key == KeyNumType(DownButton[ID].ID | KN_BUTTON)) {
        DownButton[ID].IsPressed = false;
        if ((input & 16384) != 0) {
            if (!Scroll_Page(false)) {
                Sound_Effect(Rule->ScoldSound);
            }
        } else {
            if (!Scroll(false)) {
                Sound_Effect(Rule->ScoldSound);
            }
        }
    }

    /*
    **  Reflect the scroll desired direction/value into the scroll
    **  logic handler. This might result in up or down scrolling.
    */
    if (!IsScrolling && Scroller) {
        if (BuildableCount <= Sidebar.Max_Visible(true)) {
            Scroller = 0;
        } else {

            /*
            **  Top of list is moving toward lower ordered entries in the object list. It looks like
            **  the "window" to the object list is moving up even though the actual object images are
            **  scrolling downward.
            */
            if (Scroller < 0) {
                if (!TopIndex) {
                    Scroller = 0;
                } else {
                    Scroller++;
                    IsScrollingDown = false;
                    IsScrolling = true;
                    TopIndex--;
                    Slid = 0;
                }

            } else {
                if (TopIndex + Sidebar.Max_Visible(true) >= BuildableCount) {
                    Scroller = 0;
                } else {
                    Scroller--;
                    Slid = OBJECT_HEIGHT;
                    IsScrollingDown = true;
                    IsScrolling = true;
                }
            }
        }
    }

    /*
    **  Scroll logic is handled here.
    */
    if (IsScrolling) {
        if (IsScrollingDown) {
            Slid -= SCROLL_RATE;
            if (Slid <= 0) {
                IsScrolling = false;
                Slid = 0;
                TopIndex++;
            }
        } else {
            Slid += SCROLL_RATE;
            if (Slid >= OBJECT_HEIGHT) {
                IsScrolling = false;
                Slid = 0;
            }
        }
        redraw = true;
    }

    /*
    **  Handle any flashing logic. Flashing occurs when the player selects an object
    **  and provides the visual feedback of a recognized and legal selection.
    */
    if (Flasher != -1) {
        if (Graphic_Logic()) {
            redraw = true;
            if (Fetch_Stage() >= 7) {
                Set_Rate(0);
                Set_Stage(0);
                Flasher = -1;
            }
        }
    }

    /*
    **  Handle any building clock animation logic.
    */
    if (IsBuilding) {
        for (int index = 0; index < BuildableCount; index++) {
            FactoryClass* factory = Buildables[index].Factory;

            if (factory && factory->Has_Changed()) {
                redraw = true;
                if (factory->Has_Completed()) {

                    /*
                    **  Construction has been completed. Announce this fact to the player and
                    **  try to get the object to automatically leave the factory. Buildings are
                    **  the main exception to the ability to leave the factory under their own
                    **  power.
                    */
                    TechnoClass* pending = factory->Get_Object();
                    if (pending != nullptr) {
                        switch (pending->RTTI) {
                        case RTTI_UNIT:
                        case RTTI_AIRCRAFT:
                            OutList.Add(EventClassExt(pending->Owner(), EVENT_PLACE, pending->Fetch_RTTI(), CELL_NONE, TechnoTypeClassExtension::Get_Production_Flags(pending)).As_Event());
                            //Speak(VOX_UNIT_READY);
                            break;

                        case RTTI_BUILDING:
                            Speak(VOX_CONSTRUCTION);
                            break;

                        case RTTI_INFANTRY:
                            OutList.Add(EventClassExt(pending->Owner(), EVENT_PLACE, pending->Fetch_RTTI(), CELL_NONE, TechnoTypeClassExtension::Get_Production_Flags(pending)).As_Event());
                            //Speak(VOX_UNIT_READY);
                            break;
                        }
                    }
                }
            }
        }
    }

    /*
    **  If any of the logic determined that this side strip needs to be redrawn, then
    **  set the redraw flag for this side strip.
    */
    if (redraw) {
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
    return BattleUI.Help_Text(gadget_id);
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Draw_It function.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Draw_It(bool complete)
{
    (void)complete;
}


/**
 *  Reimplements the entire SidebarClass::StripClass::Factory_Link function.
 *
 *  @author: ZivDero
 */
bool StripClassExt::_Factory_Link(FactoryClass* factory, RTTIType type, int id)
{
    (void)factory;
    (void)type;
    (void)id;
    return false;
}


/**
 *  Fake function to patch calls to redraw a specific vanilla strip.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Fake_Flag_To_Redraw_Special()
{
    Sidebar.Flag_Strip_To_Redraw(RTTI_SPECIAL, PRODFLAG_NONE);
}


/**
 *  Fake function to patch calls to redraw a specific vanilla strip.
 *
 *  @author: ZivDero
 */
void StripClassExt::_Fake_Flag_To_Redraw_Current()
{
    Sidebar.Flag_Current_Strip_To_Redraw();
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


static const ObjectTypeClass* _SidebarClass_StripClass_obj = nullptr;
static const SuperWeaponTypeClass* _SidebarClass_StripClass_spc = nullptr;
static BSurface* _SidebarClass_StripClass_CustomImage = nullptr;


/**
 *  #issue-487
 *
 *  Adds support for PCX/PNG cameo icons.
 *
 *  The following two patches store the PCX/PNG image for the factory object or special.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005F5188, _SidebarClass_StripClass_ObjectTypeClass_Custom_Cameo_Image_Patch, 0)
{
    GET(const ObjectTypeClass*, obj, EBP);

    const ShapeSet* shapefile = obj->Get_Cameo_Data();

    _SidebarClass_StripClass_obj = obj;
    _SidebarClass_StripClass_CustomImage = nullptr;

    auto technotypeext = Extension::Fetch(reinterpret_cast<const TechnoTypeClass*>(obj));
    if (technotypeext->CameoImageSurface) {
        _SidebarClass_StripClass_CustomImage = technotypeext->CameoImageSurface;
    }

    R->EAX(shapefile);

    return 0x005F5193;
}

DEFINE_HOOK(0x005F5216, _SidebarClass_StripClass_SuperWeaponType_Custom_Cameo_Image_Patch, 0)
{
    GET(const SuperWeaponTypeClass*, supertype, EAX);

    const ShapeSet* shapefile = supertype->SidebarIcon;

    _SidebarClass_StripClass_spc = supertype;
    _SidebarClass_StripClass_CustomImage = nullptr;

    auto supertypeext = Extension::Fetch(supertype);
    if (supertypeext->CameoImageSurface) {
        _SidebarClass_StripClass_CustomImage = supertypeext->CameoImageSurface;
    }

    R->EBX(shapefile);

    return 0x005F5220;
}


/**
 *  #issue-487
 *
 *  Adds support for PCX/PNG cameo icons.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005F52AF, _SidebarClass_StripClass_Custom_Cameo_Image_Patch, 0)
{
    GET_STACK(SidebarClass::StripClass*, this_ptr, 0x24);
    REF_STACK(const Rect, window_rect, 0x34);
    GET(int, pos_x, EDI);
    GET(int, pos_y, ESI);
    GET(const ShapeSet*, shapefile, EBX);

    Surface* image_surface = nullptr;

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
        Rect pcxrect;
        pcxrect.X = window_rect.X + pos_x;
        pcxrect.Y = window_rect.Y + pos_y;
        pcxrect.Width = image_surface->Get_Width();
        pcxrect.Height = image_surface->Get_Height();

        SpriteCollection.Draw(pcxrect, *SidebarSurface, *image_surface);
    }
    /**
     *  Draw shape cameo image.
     */
    else if (shapefile) {
        Point2D pointxy;
        pointxy.X = pos_x;
        pointxy.Y = pos_y;

        Draw_Shape(*SidebarSurface, *CameoDrawer, shapefile, 0, pointxy, window_rect, SHAPE_WIN_REL | SHAPE_NORMAL);
    }

    _SidebarClass_StripClass_CustomImage = nullptr;

    /**
     *  Next, draw the clock darken shape.
     */
draw_darken_shape:
    return 0x005F52F3;
}


/**
 *  Adds support for extended sidebar tooltips.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x005F4EDD, _SidebarClass_StripClass_Help_Text_Extended_Tooltip_Patch, 0)
{
    GET(int, cost, EAX);
    GET(TechnoTypeClass*, technotype, ESI);
    static char extended_description[512];

    TechnoTypeClassExtension* technotypeext = Extension::Fetch(technotype);
    char* description = technotypeext->Description;

    // Using sprintf below will affect the stack, but the compiler should also clean it up,
    // so there should be no issue.
    if (description[0] == '\0') {
        // If there is no extended description, then simply show the name and price.
        sprintf(extended_description, "%s@$%d", technotype->GivenName.c_str(), cost);
    }
    else {
        // If there is an extended description, then show the name, price, and the description.
        sprintf(extended_description, "%s@$%d@@%s", technotype->GivenName.c_str(), cost, technotypeext->Description);
    }

    // Set up return value
    R->EAX(extended_description);
    return 0x005F4EFF;
}


/**
 *  Adds support for extended factories.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005F5120, _StripClass_Draw_It_Fetch_Factory_Patch1, 0)
{
    GET(TechnoTypeClass*, ttype, EBP);

    FactoryClass* factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(ttype->RTTI, TechnoTypeClassExtension::Get_Production_Flags(ttype));

    R->EAX(factory);
    return 0x005F5132;
}

DEFINE_HOOK(0x005F537C, _StripClass_Draw_It_Fetch_Factory_Patch2, 0)
{
    GET(TechnoTypeClass*, ttype, ECX);

    FactoryClass*  factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(ttype->RTTI, TechnoTypeClassExtension::Get_Production_Flags(ttype));

    R->EBX(factory);
    return 0x005F538A;
}


/**
 *  Re-implementation of SelectClass::Action.
 *
 *  @author: ZivDero
 */
bool SelectClassExt::_Action(unsigned flags, KeyNumType& key)
{
    if (!Strip) {
        return true;
    }

    int index = Strip->TopIndex + Index;

    if (index >= std::size(Strip->Buildables)) {
        return true;
    }

    RTTIType otype = Strip->Buildables[index].BuildableType;
    int oid = Strip->Buildables[index].BuildableID;

    TechnoTypeClass const* choice = nullptr;
    SuperWeaponType spc = SUPER_NONE;

    /*
    **  Determine the factory number that would apply to objects of the type
    **  the mouse is currently addressing. This doesn't mean that the factory number
    **  fetched is actually producing the indicated object, merely that that particular
    **  kind of factory is specified by the "genfactory" value. This can be used to see
    **  if the factory type is currently busy or not.
    */
    FactoryClass* factory = Strip->Buildables[index].Factory;

    Map.Override_Mouse_Shape(MOUSE_NORMAL);

    if (index < Strip->BuildableCount) {
        if (otype != RTTI_SPECIAL) {
            choice = Fetch_Techno_Type(otype, oid);
        } else {
            spc = (SuperWeaponType)oid;
        }
    }

    if (spc != SUPER_NONE) {

        /*
        **  Display the help text if the mouse is over the button.
        */
        if (flags & LEFTUP) {
            flags &= ~LEFTUP;
        }

        /*
        **  A right mouse button signals "cancel".  If we are in targeting
        **  mode then we don't want to be any more.
        */
        if (flags & RIGHTPRESS) {
            Map.TargettingType = SUPER_NONE;
        }

        /*
        **  A left mouse press signal "activate".  If our weapon type is
        **  available then we should activate it.
        */
        if (flags & LEFTPRESS) {

            if (spc < PlayerPtr->SuperWeapon.Count()) {
                if (PlayerPtr->SuperWeapon[spc]->Can_Place()) {
                    if (PlayerPtr->SuperWeapon[spc]->Class->Action != ACTION_NONE) {
                        Map.TargettingType = spc;
                        Unselect_All();
                        Speak(VOX_SELECT_TARGET);
                    } else {
                        OutList.Add(EventClass(PlayerPtr->Fetch_Heap_ID(), EVENT_SPECIAL_PLACE, PlayerPtr->SuperWeapon[spc]->Class->HeapID, CELL_NONE));
                    }
                } else {
                    PlayerPtr->SuperWeapon[spc]->Impatient_Click();
                }
            }
        }

    } else {
        if (choice != nullptr) {

            /*
            **  Display the help text if the mouse is over the button.
            */
            if (flags & LEFTUP) {
                flags &= ~LEFTUP;
            }

            /*
            **  A right mouse button signals "cancel".
            */
            if (flags & RIGHTPRESS) {

                /*
                **  If production is in progress, put it on hold. If production is already
                **  on hold, then abandon it. Money will be refunded, the factory
                **  manager deleted, and the object under construction is returned to
                **  the free pool.
                */
                if (factory != nullptr) {
                    /*
                    **  Cancels placement mode if the sidebar factory is abandoned or
                    **  suspended.
                    */
                    if (Map.PendingObjectPtr && Map.PendingObjectPtr->Is_Techno()) {
                        Map.PendingObjectPtr = nullptr;
                        Map.PendingObject = nullptr;
                        Map.PendingHouse = HOUSE_NONE;
                        Map.Set_Cursor_Shape(nullptr);
                    }

                    if (!factory->Is_Building()) {
                        Speak(VOX_CANCELED);

                        int count_to_abandon = 1;

                        if (Key_Down(VK_SHIFT))
                            count_to_abandon = factory->Total_Queued(*choice);
                        else if (Key_Down(VK_CONTROL))
                            count_to_abandon = std::clamp(5, 0, factory->Total_Queued(*choice));

                        for (int i = 0; i < count_to_abandon; i++)
                            OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_ABANDON, otype, oid, TechnoTypeClassExtension::Get_Production_Flags(otype, oid)).As_Event());
                    } else {
                        Speak(VOX_SUSPENDED);
                        OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_SUSPEND, otype, oid, TechnoTypeClassExtension::Get_Production_Flags(otype, oid)).As_Event());
                        Sidebar.Flag_Strip_To_Redraw(otype, TechnoTypeClassExtension::Get_Production_Flags(otype, oid));
                        
                    }
                } else {
                    factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(otype, TechnoTypeClassExtension::Get_Production_Flags(choice));
                    if (factory && factory->Is_Queued(*choice)) {
                        int count_to_abandon = 1;

                        if (Key_Down(VK_SHIFT))
                            count_to_abandon = factory->Total_Queued(*choice);
                        else if (Key_Down(VK_CONTROL))
                            count_to_abandon = std::clamp(5, 0, factory->Total_Queued(*choice));

                        for (int i = 0; i < count_to_abandon; i++)
                            OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_ABANDON, otype, oid, TechnoTypeClassExtension::Get_Production_Flags(otype, oid)).As_Event());
                    }
                }
            }

            if (flags & LEFTPRESS) {
                /*
                **  If this object is currently being built, then give a scold sound and text and then
                **  bail.
                */
                if (factory != nullptr && !factory->Is_Building() && !Extension::Fetch(factory)->IsHoldingExit) {

                    /*
                    **  If production has completed, then attempt to have the object exit
                    **  the factory or go into placement mode.
                    */
                    if (factory->Has_Completed()) {

                        TechnoClass* pending = factory->Get_Object();
                        if (!pending) {
                            if (factory->Get_Special_Item() != -1) {
                                Map.TargettingType = SUPER_ANY;
                            }
                        } else {
                            BuildingClass* builder = pending->Who_Can_Build_Me(false, false);
                            if (!builder) {
                                OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_ABANDON, otype, oid, TechnoTypeClassExtension::Get_Production_Flags(otype, oid)).As_Event());
                                Speak(VOX_NO_FACTORY);
                            } else {

                                /*
                                **  If the completed object is a building, then change the
                                **  game state into building placement mode. This fact is
                                **  not transmitted to any linked computers until the moment
                                **  the building is actually placed down.
                                */
                                if (pending->Fetch_RTTI() == RTTI_BUILDING) {
                                    PlayerPtr->Manual_Place(builder, (BuildingClass*)pending);
                                } else {

                                    /*
                                    **  For objects that can leave the factory under their own
                                    **  power, queue this event and process through normal house
                                    **  production channels.
                                    */
                                    OutList.Add(EventClassExt(pending->Owner(), EVENT_PLACE, otype, CELL_NONE, TechnoTypeClassExtension::Get_Production_Flags(pending)).As_Event());
                                }
                            }
                        }
                    } else {

                        /*
                        **  The factory must have been in a suspended state. Resume construction
                        **  normally.
                        */
                        if (otype == RTTI_INFANTRYTYPE) {
                            Speak(VOX_TRAINING);
                        } else {
                            Speak(VOX_BUILDING);
                        }
                        OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_PRODUCE,
                            Strip->Buildables[index].BuildableType, Strip->Buildables[index].BuildableID,
                            TechnoTypeClassExtension::Get_Production_Flags(Strip->Buildables[index].BuildableType, Strip->Buildables[index].BuildableID)).As_Event());
                    }

                } else {

                    /*
                    **  If there is already a factory attached to this strip but the player didn't click
                    **  on the icon that has the attached factory, then say that the factory is busy and
                    **  ignore the click.
                    */
                    factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(otype, TechnoTypeClassExtension::Get_Production_Flags(choice));
                    bool produce = false;
                    if (factory != nullptr && (factory->Is_Building() || factory->Has_Production_Target())) {
                        if (otype == RTTI_BUILDINGTYPE) {
                            Speak(VOX_NO_FACTORY);
                        } else {
                            produce = true;
                        }
                    } else {

                        /*
                        **  If this side strip is already busy with production, then ignore the
                        **  input and announce this fact.
                        */
                        if (otype == RTTI_INFANTRYTYPE) {
                            Speak(VOX_TRAINING);
                        } else {
                            Speak(VOX_BUILDING);
                        }
                        produce = true;
                    }
                    if (produce) {
                        const int count_to_produce = Key_Down(VK_SHIFT) ? 5 : 1;
                        for (int i = 0; i < count_to_produce; i++) {
                            OutList.Add(EventClassExt(PlayerPtr->Fetch_Heap_ID(), EVENT_PRODUCE, Strip->Buildables[index].BuildableType, Strip->Buildables[index].BuildableID, TechnoTypeClassExtension::Get_Production_Flags(Strip->Buildables[index].BuildableType, Strip->Buildables[index].BuildableID)).As_Event());
                        }
                    }
                }
            }
        } else {
            flags = 0;
        }
    }

    ControlClass::Action(flags, key);
    return true;
}


/**
 *  Main function for patching the hooks.
 */
void SidebarClassExtension_Hooks()
{
    /**
     *  These patches are compatible with the vanilla sidebar.
     */
    Patch_Jump(0x005F5F70, &SidebarClassExt::_Abandon_Production);
    Patch_Jump(0x005F4E40, &StripClassExt::_Help_Text);
    Patch_Jump(0x005F4630, &StripClassExt::_Add);
    Patch_Jump(0x005F5610, &StripClassExt::_Recalc);
    Patch_Jump(0x005F4910, &StripClassExt::_AI_Vanilla);
    Patch_Jump(0x005F59A0, &SelectClassExt::_Action);

    // NOP away tooltip length check for formatting
    Patch_Byte(0x0044E486, 0x90);
    Patch_Byte(0x0044E486 + 1, 0x90);

    // Change jle to jl to allow rendering tooltips that are exactly as wide as the sidebar
    Patch_Byte(0x0044E605 + 1, 0x8C);

    // Paired with _SidebarClass_StripClass_Help_Text_Extended_Tooltip_Patch
    Patch_Byte(0x005F4EF7 + 2, 0x14); // Pop one more argument passed to sprintf

    Patch_Jump(0x005F4DD0, 0x005F4DD5); // Skip a call to Speak as we now speak UNIT_READY in HouseClassExt::Place_Object
}


/**
 *  Function for patching the hooks that require use to read VINIFERA.INI first.
 */
void SidebarClassExtension_Conditional_Hooks()
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

    // There are a bunch of calls to vanilla strips to redraw them.
    // We patch them to either redraw the supers' strip or the current strip.
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
}
