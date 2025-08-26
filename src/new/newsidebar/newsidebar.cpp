/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NEWSIDEBAR.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         Sidebar re-implementation class.
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

#include "newsidebar.h"

#include "bsurface.h"
#include "building.h"
#include "convert.h"
#include "drawshape.h"
#include "eventext.h"
#include "extension_globals.h"
#include "factory.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "language.h"
#include "mouse.h"
#include "optionsext.h"
#include "playmovie.h"
#include "rules.h"
#include "session.h"
#include "shapeset.h"
#include "sideext.h"
#include "spritecollection.h"
#include "super.h"
#include "supertypeext.h"
#include "technotypeext.h"
#include "tooltip.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"
#include "voc.h"
#include "vox.h"
#include "wwmouse.h"


ShapeSet const* NewSidebarClass::SidebarTopShape = nullptr;
ShapeSet const* NewSidebarClass::SidebarShape = nullptr;
ShapeSet const* NewSidebarClass::SidebarMiddleShape = nullptr;
ShapeSet const* NewSidebarClass::SidebarBottomShape = nullptr;
ShapeSet const* NewSidebarClass::SidebarAddonShape = nullptr;

/***************************************************************************
**	This points to the main sidebar shapes. These include the upgrade and
**	repair buttons.
*/

typedef enum ButtonNumberType {
    BUTTON_RADAR = 100,
    BUTTON_REPAIR,
    BUTTON_POWER,
    BUTTON_SELL,
    BUTTON_WAYPOINT,
    BUTTON_UP = 200,
    BUTTON_DOWN = 210,
    BUTTON_TAB = 220,
    BUTTON_SELECT = 230,
} ButtonNumberType;

/*
** Sidebar buttons
*/
NewSidebarClass::SBGadgetClass NewSidebarClass::Background;
ShapeButtonClass NewSidebarClass::Repair;
ShapeButtonClass NewSidebarClass::Sell;
ShapeButtonClass NewSidebarClass::Power;
ShapeButtonClass NewSidebarClass::Waypoint;

/*
** Shape data pointers
*/
ShapeSet const* NewSidebarClass::StripClass::LogoShapes = nullptr;
ShapeSet const* NewSidebarClass::StripClass::ClockShapes = nullptr;
ShapeSet const* NewSidebarClass::StripClass::RechargeClockShapes = nullptr;
ShapeSet const* NewSidebarClass::StripClass::DarkenShapes = nullptr;

GadgetClass* IHoverableGadget::LastHovered = nullptr;


NewSidebarClass::SBGadgetClass::SBGadgetClass() :
    GadgetClass(0, 0, 1, 1, LEFTUP)
{
    
}


/**
 *  
 *
 *  @author: ZivDero
 */
NewSidebarClass::NewSidebarClass() :
    CurrentTab(0)
    //IsCameoText(false),
    //IsSidebarActive(false)
    //IsToRedraw(true),
    //field_1CD8(false)//,
    //IsRepairActive(false),
    //IsUpgradeActive(false),
    //IsDemolishActive(false)
{
    /*
    **  Set up the coordinates for the sidebar strips. These coordinates are for
    **  the upper left corner.
    */
    if (!OptionsExtension->SidebarControls.IsTabs) {
        Column.emplace_back(0, Point2D(OptionsExtension->SidebarControls.Strip1X, OptionsExtension->SidebarControls.StripYOffset), 1);
        Column.emplace_back(1, Point2D(OptionsExtension->SidebarControls.Strip2X, OptionsExtension->SidebarControls.StripYOffset), 1);
    } else {
        for (int i = 0; i < OptionsExtension->SidebarControls.Tabs; i++) {
            Column.emplace_back(i, Point2D(OptionsExtension->SidebarControls.Strip1X, OptionsExtension->SidebarControls.StripYOffset), OptionsExtension->SidebarControls.Columns);
        }
    }
}


NewSidebarClass::NewSidebarClass(NoInitClass const& x)
{
}


HRESULT NewSidebarClass::Load(IStream* stream)
{
    Column.clear(); // don't leak memory

    HRESULT result = stream->Read(this, sizeof(*this), nullptr);
    if (FAILED(result)) return result;

    new (this) NewSidebarClass(NoInitClass());

    int count;
    result = stream->Read(&count, sizeof(count), nullptr);
    if (FAILED(result)) return result;

    for (int i = 0; i < count; i++) {
        Column.emplace_back(NoInitClass());
        result = Column.back().Load(stream);
        if (FAILED(result)) return result;
    }

    return result;
}


HRESULT NewSidebarClass::Save(IStream* stream) const
{
    HRESULT result = stream->Write(this, sizeof(*this), nullptr);
    if (FAILED(result)) return result;

    int count = static_cast<int>(Column.size());
    result = stream->Write(&count, sizeof(count), nullptr);
    if (FAILED(result)) return result;

    for (const auto& strip : Column) {
        result = strip.Save(stream);
        if (FAILED(result)) return result;
    }

    return S_OK;
}


void NewSidebarClass::Toggle_Cameo_Text(bool on)
{
    if (on != Map.SidebarClass::IsCameoText) {
        Map.SidebarClass::IsCameoText = on;
        Map.SidebarClass::IsToRedraw = true;
        Map.Flag_To_Redraw();
    }
}


void NewSidebarClass::Init_Clear()
{
    CurrentTab = 0;

    for (auto& strip : Column) {
        strip.Init_Clear();
    }

    Activate(0);
}


void NewSidebarClass::Init_IO()
{
    SidebarRect.X = TacticalRect.X + TacticalRect.Width;
    SidebarRect.Y = OptionsExtension->SidebarControls.RadarHeight + OptionsExtension->SidebarControls.RadarTopHeight + OptionsExtension->SidebarControls.TabHeight;
    SidebarRect.Width = OptionsExtension->SidebarControls.SidebarWidth;
    SidebarRect.Height = TacticalRect.Y + TacticalRect.Height - SidebarRect.Y;

    /*
    ** Add the sidebar's buttons only if we're not in editor mode.
    */
    if (!Debug_Map) {
        int xoff = -480;
        int yoff = 3;

        Repair.IsSticky = true;
        Repair.ID = BUTTON_REPAIR;
        Repair.X = SidebarRect.X + OptionsExtension->SidebarControls.RepairButtonPosition.X;
        Repair.Y = SidebarRect.Y + OptionsExtension->SidebarControls.RepairButtonPosition.Y;
        Repair.DrawOffsetX = xoff;
        Repair.DrawOffsetY = yoff;
        Repair.DrawOnSidebar = true;
        Repair.ShapeDrawer = SidebarDrawer;
        Repair.IsPressed = false;
        Repair.IsToggleType = true;
        Repair.ReflectButtonState = true;

        Sell.IsSticky = true;
        Sell.ID = BUTTON_SELL;
        Sell.X = SidebarRect.X + OptionsExtension->SidebarControls.SellButtonPosition.X;
        Sell.Y = SidebarRect.Y + OptionsExtension->SidebarControls.SellButtonPosition.Y;
        Sell.DrawOffsetX = xoff;
        Sell.DrawOffsetY = yoff;
        Sell.DrawOnSidebar = true;
        Sell.ShapeDrawer = SidebarDrawer;
        Sell.IsPressed = false;
        Sell.IsToggleType = true;
        Sell.ReflectButtonState = true;

        Power.IsSticky = true;
        Power.ID = BUTTON_POWER;
        Power.X = SidebarRect.X + OptionsExtension->SidebarControls.PowerButtonPosition.X;
        Power.Y = SidebarRect.Y + OptionsExtension->SidebarControls.PowerButtonPosition.Y;
        Power.DrawOffsetX = xoff;
        Power.DrawOffsetY = yoff;
        Power.DrawOnSidebar = true;
        Power.ShapeDrawer = SidebarDrawer;
        Power.IsPressed = false;
        Power.IsToggleType = true;
        Power.ReflectButtonState = true;

        Waypoint.IsSticky = true;
        Waypoint.ID = BUTTON_WAYPOINT;
        Waypoint.X = SidebarRect.X + OptionsExtension->SidebarControls.WaypointButtonPosition.X;
        Waypoint.Y = SidebarRect.Y + OptionsExtension->SidebarControls.WaypointButtonPosition.Y;
        Waypoint.DrawOffsetX = xoff;
        Waypoint.DrawOffsetY = yoff;
        Waypoint.DrawOnSidebar = true;
        Waypoint.ShapeDrawer = SidebarDrawer;
        Waypoint.IsPressed = false;
        Waypoint.IsToggleType = true;
        Waypoint.ReflectButtonState = true;

        Waypoint.Enable();

        for (auto& strip : Column) {
            strip.Init_IO();
        }

        Set_Dimensions();

        /*
        ** If a game was loaded & the sidebar was enabled, pop it up now
        */
        if (Map.SidebarClass::IsSidebarActive) {
            Map.SidebarClass::IsSidebarActive = false;
            Activate(1);
        }
    }
}


void NewSidebarClass::Init_For_House()
{
    PaletteClass pal("SIDEBAR.PAL");

    delete SidebarDrawer;
    SidebarDrawer = new ConvertClass(&pal, &pal, VisibleSurface, 1);

    Sell.Set_Shape(MFCD::RetrieveT<ShapeSet>("SELL.SHP"));
    Sell.ShapeDrawer = SidebarDrawer;

    Power.Set_Shape(MFCD::RetrieveT<ShapeSet>("POWER.SHP"));
    Power.ShapeDrawer = SidebarDrawer;

    Waypoint.Set_Shape(MFCD::RetrieveT<ShapeSet>("WAYP.SHP"));
    Waypoint.ShapeDrawer = SidebarDrawer;

    Repair.Set_Shape(MFCD::RetrieveT<ShapeSet>("REPAIR.SHP"));
    Repair.ShapeDrawer = SidebarDrawer;

    SidebarTopShape = MFCD::RetrieveT<ShapeSet>("TOP.SHP");
    SidebarShape = MFCD::RetrieveT<ShapeSet>("SIDE1.SHP");
    SidebarMiddleShape = MFCD::RetrieveT<ShapeSet>("SIDE2.SHP");
    SidebarBottomShape = MFCD::RetrieveT<ShapeSet>("SIDE3.SHP");
    SidebarAddonShape = MFCD::RetrieveT<ShapeSet>("ADDON.SHP");

    for (auto& strip : Column) {
        strip.Init_For_House();
    }
}


int NewSidebarClass::Which_Column(RTTIType type, ProductionFlags flags)
{
    if (!OptionsExtension->SidebarControls.IsTabs) {
        return type == RTTI_BUILDING || type == RTTI_BUILDINGTYPE ? 0 : 1;
    }

    int column = -1;

    switch (type) {
    case RTTI_BUILDINGTYPE:
    case RTTI_BUILDING:
        if (flags & PRODFLAG_DEFENSE) {
            column = OptionsExtension->SidebarControls.DefensesTab;
        } else {
            column = OptionsExtension->SidebarControls.BuildingsTab;
        }
        break;

    case RTTI_INFANTRYTYPE:
    case RTTI_INFANTRY:
        column = OptionsExtension->SidebarControls.InfantryTab;
        break;

    case RTTI_UNITTYPE:
    case RTTI_UNIT:
        if (flags & PRODFLAG_NAVAL) {
            column = OptionsExtension->SidebarControls.NavalTab;
        } else {
            column = OptionsExtension->SidebarControls.UnitsTab;
        }
        break;

    case RTTI_AIRCRAFTTYPE:
    case RTTI_AIRCRAFT:
        column = OptionsExtension->SidebarControls.AircraftTab;
        break;

    case RTTI_SUPERWEAPONTYPE:
    case RTTI_SUPERWEAPON:
    case RTTI_SPECIAL:
        column = OptionsExtension->SidebarControls.SpecialTab;
        break;
    }

    ASSERT_PRINT(column != -1, "RTTI %s (%d) has no tab assigned!", Name_From_RTTI(type), type);
    return column;
}


bool NewSidebarClass::Factory_Link(FactoryClass* factory, RTTIType type, int id)
{
    ASSERT(type < RTTI_COUNT);
    ASSERT(id >= 0);

    return Column[Which_Column(type, TechnoTypeClassExtension::Get_Production_Flags(type, id))].Factory_Link(factory, type, id);
}

bool NewSidebarClass::Change_Tab(int index)
{
    // Don't switch to the same tab
    if (CurrentTab == index) return false;

    // Do not switch to inactive tabs
    if (Column[index].Buildables.empty()) return false;

    Column[CurrentTab].Deactivate();
    CurrentTab = index;
    Column[CurrentTab].Activate();

    Map.SidebarClass::IsToFullRedraw = true;
    return true;
}

int NewSidebarClass::First_Active_Tab() const
{
    for (int i = 0; i < Column.size(); i++) {
        if (!Column[i].Buildables.empty()) {
            return i;
        }
    }

    return -1;
}

bool NewSidebarClass::Is_On_Sidebar(RTTIType type, int id) const
{
    const int column = Which_Column(type, TechnoTypeClassExtension::Get_Production_Flags(type, id));
    return Column[column].Is_On_Sidebar(type, id);
}


bool NewSidebarClass::Add(RTTIType type, int id)
{
    ASSERT(type < RTTI_COUNT);

    /*
    ** Add the sidebar only if we're not in editor mode.
    */
    if (!Debug_Map) {
        int column = Which_Column(type, TechnoTypeClassExtension::Get_Production_Flags(type, id));

        if (Column[column].Add(type, id)) {
            Activate(1);
            Map.SidebarClass::IsToRedraw = true;
            Map.Flag_To_Redraw();
            return true;
        }
        return false;
    }

    return false;
}


bool NewSidebarClass::Scroll(bool up, int column)
{
    static int& _dialog_count = Make_Global<int>(0x007E492C);

    bool scr = false;
    if (_dialog_count == 0) {
        if (OptionsExtension->SidebarControls.IsTabs) {
            scr = Current_Tab().Scroll(up);
        } else {
            if (column == -1) {
                for (auto& strip : Column) {
                    scr |= strip.Scroll(up);
                }
            } else {
                scr = Column[column].Scroll(up);
            }
        }
        if (scr) {
            Map.SidebarClass::IsToRedraw = true;
            Map.Flag_To_Redraw();
            return true;
        } else {
            if (!scr) {
                Sound_Effect(Rule->ScoldSound);
            }
            return false;
        }

    }
    return false;
}


bool NewSidebarClass::Page(bool up, int column)
{
    static int& _dialog_count = Make_Global<int>(0x007E492C);

    bool scr = false;
    if (_dialog_count == 0) {
        if (OptionsExtension->SidebarControls.IsTabs) {
            scr = Current_Tab().Page(up);
        } else {
            if (column == -1) {
                for (auto& strip : Column) {
                    scr |= strip.Page(up);
                }
            } else {
                scr = Column[column].Page(up);
            }
        }
        if (scr) {
            Map.SidebarClass::IsToRedraw = true;
            Map.Flag_To_Redraw();
            return true;
        } else {
            if (!scr) {
                Sound_Effect(Rule->ScoldSound);
            }
            return false;
        }
    }
    return false;
}


void NewSidebarClass::Draw_It(bool complete)
{
    complete = complete || Map.SidebarClass::IsToFullRedraw;
    Map.LastDrawRect = RECT_NONE;

    Map.PowerClass::Draw_It(complete);

    DSurface* old = LogicalSurface;
    LogicalSurface = SidebarSurface;

    Rect window(0, 0, SidebarSurface->Get_Width(), SidebarSurface->Get_Height());

    if (Map.SidebarClass::IsSidebarActive && (Map.SidebarClass::IsToRedraw || complete) && !Debug_Map) {

        bool redraw_strip = false;
        for (auto& strip : Column) {
            if (strip.IsActive) redraw_strip |= strip.IsToRedraw;
        }

        if (complete || redraw_strip) {

            Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarTopShape, 0, Point2D(0, OptionsExtension->SidebarControls.TabHeight), window, SHAPE_WIN_REL);

            int y = SidebarRect.Y;

            /*
            ** The sidebar shape is too big in 640x400 so it needs to be drawn in three chunks.
            */
            Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarShape, 0, Point2D(0, y), window, SHAPE_WIN_REL);
            y += NewSidebarClass::SidebarShape->Get_Height();

            for (int i = 0; i < Max_Visible(); i++) {
                Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarMiddleShape, 0, Point2D(0, y), window, SHAPE_WIN_REL);
                y += SidebarMiddleShape->Get_Height();
            }

            Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarBottomShape, 0, Point2D(0, y), window, SHAPE_WIN_REL);
            Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarAddonShape, 0, Point2D(0, y + SidebarBottomShape->Get_Height()), window, SHAPE_WIN_REL);

            for (auto& strip : Column) {
                if (strip.IsActive) strip.IsToRedraw = true;
            }
        }

        Repair.Draw_Me(true);
        Sell.Draw_Me(true);
        Power.Draw_Me(true);
        Waypoint.Draw_Me(true);
        RedrawSidebar = true;
    }

    /*
    **	Draw the side strip elements by calling their respective draw functions.
    */
    if (Map.SidebarClass::IsSidebarActive) {
        for (auto& strip : Column) {
            if (strip.IsActive) strip.Draw_It(complete);
        }
    }
    if (Repair.IsDrawn) {
        RedrawSidebar = true;
        Repair.IsDrawn = false;
    }
    if (Sell.IsDrawn) {
        RedrawSidebar = true;
        Sell.IsDrawn = false;
    }
    if (Power.IsDrawn) {
        RedrawSidebar = true;
        Power.IsDrawn = false;
    }
    if (Waypoint.IsDrawn) {
        RedrawSidebar = true;
        Waypoint.IsDrawn = false;
    }
    if (ToolTips != nullptr) {
        ToolTips->Force_Redraw(true);
    }
    if (OptionsExtension->SidebarControls.IsTabs) {
        for (auto& strip : Column) {
            strip.TabButton.Draw_Me(true);
        }
    }
    Map.SidebarClass::IsToRedraw = false;
    Map.SidebarClass::IsToFullRedraw = false;

    Map.Blit_Sidebar(complete);
    LogicalSurface = old;
}


void NewSidebarClass::AI(KeyNumType& input, Point2D const& xy)
{
    if (!Debug_Map) {
        Activate(1); // Force the sidebar always on in Win95 mode
    }

    if (!Debug_Map) {
        for (auto& strip : Column) {
            strip.AI(input);
        }
    }

    if (Map.SidebarClass::IsSidebarActive) {

        /*
        **	If there are any buildings in the payer's inventory, then allow the repair
        **	option.
        */
        if (PlayerPtr->CurBuildings > 0) {
            Map.Activate_Repair(true);
        } else {
            Map.Activate_Repair(false);
        }

        if (input == (BUTTON_REPAIR | KN_BUTTON)) {
            Map.Repair_Mode_Control(-1);
        }

        if (input == (BUTTON_POWER | KN_BUTTON)) {
            Map.Power_Mode_Control(-1);
        }

        if (input == (BUTTON_WAYPOINT | KN_BUTTON)) {
            Map.Waypoint_Mode_Control(-1, false);
        }

        if (input == (BUTTON_SELL | KN_BUTTON)) {
            Map.Sell_Mode_Control(-1);
        }

        if (input >= (BUTTON_TAB | KN_BUTTON) && input < (BUTTON_SELECT | KN_BUTTON)) {
            Change_Tab((input & ~KN_BUTTON) - BUTTON_TAB);
        }
    }

    if (!Map.IsRepairMode && Repair.IsOn) {
        Repair.Turn_Off();
    }

    if (!Map.IsSellMode && Sell.IsOn) {
        Sell.Turn_Off();
    }

    if (!Map.IsPowerMode && Power.IsOn) {
        Power.Turn_Off();
    }

    if (!Map.IsWaypointMode && Waypoint.IsOn) {
        Waypoint.Turn_Off();
    }

    if (OptionsExtension->SidebarControls.IsTabs) {
        if (!Current_Tab().TabButton.IsSelected) {
            Current_Tab().TabButton.Select();
        }

        if (Current_Tab().Buildables.empty()) {
            int newtab = First_Active_Tab();
            if (newtab != -1) Change_Tab(newtab);
        }
    }

    Map.PowerClass::AI(input, const_cast<Point2D&>(xy));
}


void NewSidebarClass::Recalc()
{
    bool redraw = false;
    for (auto& strip : Column) {
        redraw |= strip.Recalc();
    }

    if (redraw) {
        Map.SidebarClass::IsToRedraw = true;
        Map.Flag_To_Redraw();
    }
}


bool NewSidebarClass::Activate(int control)
{
    bool old = Map.SidebarClass::IsSidebarActive;

    if (Session.Play && !Session.Singleplayer_Game()) return old;

    /*
    **	Determine the new state of the sidebar.
    */
    switch (control) {
    case -1:
        Map.SidebarClass::IsSidebarActive = Map.SidebarClass::IsSidebarActive == false;
        break;

    case 1:
        Map.SidebarClass::IsSidebarActive = true;
        break;

    default:
    case 0:
        Map.SidebarClass::IsSidebarActive = false;
        break;
    }

    /*
    **	Only if there is a change in the state of the sidebar will anything
    **	be done to change it.
    */
    if (Map.SidebarClass::IsSidebarActive != old) {

        /*
        **	If the sidebar is activated but was on the right side of the screen, then
        **	activate it on the left side of the screen.
        */
        if (Map.SidebarClass::IsSidebarActive) {
            Set_Dimensions();
            Map.SidebarClass::IsToRedraw = true;
            Repair.Zap();
            Map.Add_A_Button(Repair);
            Sell.Zap();
            Map.Add_A_Button(Sell);
            Power.Zap();
            Map.Add_A_Button(Power);
            Waypoint.Zap();
            Map.Add_A_Button(Waypoint);
            if (OptionsExtension->SidebarControls.IsTabs) {
                for (auto& strip : Column) {
                    strip.TabButton.Zap();
                    Map.Add_A_Button(strip.TabButton);
                }
                Current_Tab().Activate();
            } else {
                for (auto& strip : Column) {
                    strip.Activate();
                }
            }

            Background.Zap();
            Map.Add_A_Button(Background);
            RadarClass::RadarButton.Zap();
            Map.Add_A_Button(RadarClass::RadarButton);
        } else {
            End_Ingame_Movie();
            Map.Remove_A_Button(Repair);
            Map.Remove_A_Button(Sell);
            Map.Remove_A_Button(Power);
            Map.Remove_A_Button(Waypoint);
            Map.Remove_A_Button(Background);
            for (auto& strip : Column) {
                strip.Deactivate();
                if (OptionsExtension->SidebarControls.IsTabs) {
                    Map.Remove_A_Button(strip.TabButton);
                }
            }
            Map.Remove_A_Button(RadarClass::RadarButton);
        }

        /*
        **	Since the sidebar status has changed, update the map so that the graphics
        **	will be rendered correctly.
        */
        Map.Flag_To_Redraw(GS_REDRAW_ALL);
    }

    return old;
}


bool NewSidebarClass::Abandon_Production(RTTIType type, FactoryClass* factory, ProductionFlags flags)
{
    return Column[Which_Column(type, flags)].Abandon_Production(factory);
}


char const* NewSidebarClass::Help_Text(int id)
{
    const char* text = Map.PowerClass::Help_Text(id);
    if (text == nullptr) {
        if (id >= BUTTON_TAB && id < BUTTON_SELECT) {

        }


        id -= 1000;
        int index = id >> 8;
        if (index < Column.size() && index >= 0) {
            text = Column[index].Help_Text(id % 256U);
        }
    }
    return text;
}


int NewSidebarClass::Max_Visible()
{
    if (SidebarSurface && SidebarShape) {
        return (SidebarRect.Height - SidebarBottomShape->Get_Height() - SidebarShape->Get_Height()) / SidebarMiddleShape->Get_Height();
    } else {
        return SidebarClass::StripClass::MAX_VISIBLE;
    }
}


void NewSidebarClass::Set_Dimensions()
{
    /*
    **	Position the sidebar.
    */
    SidebarRect.X = TacticalRect.X + TacticalRect.Width;
    SidebarRect.Y = OptionsExtension->SidebarControls.RadarHeight + OptionsExtension->SidebarControls.RadarTopHeight + OptionsExtension->SidebarControls.TabHeight;
    SidebarRect.Width = OptionsExtension->SidebarControls.SidebarWidth;
    SidebarRect.Height = TacticalRect.Y + TacticalRect.Height - SidebarRect.Y;

    /*
    **	Position the sidebar's buttons.
    */
    Point2D position = SidebarRect.TopLeft + OptionsExtension->SidebarControls.RepairButtonPosition;
    Repair.Set_Position(position.X, position.Y);
    Repair.Flag_To_Redraw();
    Repair.DrawOffsetX = -SidebarRect.X;

    position = SidebarRect.TopLeft + OptionsExtension->SidebarControls.SellButtonPosition;
    Sell.Set_Position(position.X, position.Y);
    Sell.Flag_To_Redraw();
    Sell.DrawOffsetX = -SidebarRect.X;

    position = SidebarRect.TopLeft + OptionsExtension->SidebarControls.PowerButtonPosition;
    Power.Set_Position(position.X, position.Y);
    Power.Flag_To_Redraw();
    Power.DrawOffsetX = -SidebarRect.X;

    position = SidebarRect.TopLeft + OptionsExtension->SidebarControls.WaypointButtonPosition;
    Waypoint.Set_Position(position.X, position.Y);
    Waypoint.Flag_To_Redraw();
    Waypoint.DrawOffsetX = -SidebarRect.X;

    /*
    **	Create the tooltips for the sidebar.
    */
    if (ToolTips) {
        ToolTip tooltip;

        for (int index = 0; index < Column.size(); index++) {
            for (int j = 0; j < 100; j++) {
                ToolTips->Remove((j | index << 8) + 1000);
            }
        }

        for (auto& strip : Column) {
            strip.Set_Dimensions();
        }

        tooltip.ID = BUTTON_REPAIR;
        tooltip.Text = TXT_REPAIR_MODE;
        tooltip.Region.Set(Repair.X, Repair.Y, Repair.Width, Repair.Height);
        ToolTips->Remove(tooltip.ID);
        ToolTips->Add(&tooltip);

        tooltip.ID = BUTTON_POWER;
        tooltip.Text = TXT_POWER_MODE;
        tooltip.Region.Set(Power.X, Power.Y, Power.Width, Power.Height);
        ToolTips->Remove(tooltip.ID);
        ToolTips->Add(&tooltip);

        tooltip.ID = BUTTON_SELL;
        tooltip.Text = TXT_SELL_MODE;
        tooltip.Region.Set(Sell.X, Sell.Y, Sell.Width, Sell.Height);
        ToolTips->Remove(tooltip.ID);
        ToolTips->Add(&tooltip);

        tooltip.ID = BUTTON_WAYPOINT;
        tooltip.Text = TXT_WAYPOINTMODE;
        tooltip.Region.Set(Waypoint.X, Waypoint.Y, Waypoint.Width, Waypoint.Height);
        ToolTips->Remove(tooltip.ID);
        ToolTips->Add(&tooltip);
    }

    int y = RadarClass::RadarButton.Height + RadarClass::RadarButton.Y;
    Background.Set_Position(TacticalRect.X + TacticalRect.Width, y);
    Background.Set_Size(SidebarSurface->Get_Width(), SidebarSurface->Get_Height() - y);
    Background.Flag_To_Redraw();
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::StripClass -- Default constructor for the side strip class.       *
 *                                                                                             *
 *    This constructor is used to reset the side strip to default empty state.                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
NewSidebarClass::StripClass::StripClass(int id, Point2D origin, int columns) :
    Position(origin),
    Columns(columns),
    ID(id),
    IsActive(false),
    IsToRedraw(true),
    IsBuilding(false),
    IsScrollingDown(false),
    IsScrolling(false),
    TopIndex(0),
    Scroller(0),
    Slid(0),
    LastSlid(0)
{
}


HRESULT NewSidebarClass::StripClass::Load(IStream* stream)
{
    Buildables.clear(); // don't leak memory

    HRESULT result = stream->Read(this, sizeof(*this), nullptr);
    if (FAILED(result)) return result;

    new (this) StripClass(NoInitClass());

    int count;
    result = stream->Read(&count, sizeof(count), nullptr);
    if (FAILED(result)) return result;

    for (int i = 0; i < count; i++) {
        BuildType buildable;
        result = stream->Read(&buildable, sizeof(buildable), nullptr);
        if (FAILED(result)) return result;
        Buildables.push_back(buildable);
    }

    for (auto& buildable : Buildables) {
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(buildable.Factory, "BuildType::Factory");
    }

    return result;
}


HRESULT NewSidebarClass::StripClass::Save(IStream* stream) const
{
    HRESULT result = stream->Write(this, sizeof(*this), nullptr);
    if (FAILED(result)) return result;

    int count = static_cast<int>(Buildables.size());
    result = stream->Write(&count, sizeof(count), nullptr);
    if (FAILED(result)) return result;

    for (const auto& buildable : Buildables) {
        result = stream->Write(&buildable, sizeof(buildable), nullptr);
        if (FAILED(result)) return result;
    }

    return S_OK;
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::One_Time -- Performs one time actions necessary for the side stri *
 *                                                                                             *
 *    Call this routine ONCE at the beginning of the game. It handles retrieving pointers to   *
 *    the shape files it needs for rendering.                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void NewSidebarClass::StripClass::One_Time()
{
    RechargeClockShapes = MFCD::RetrieveT<ShapeSet>("RCLOCK2.SHP");
    ClockShapes = MFCD::RetrieveT<ShapeSet>("GCLOCK2.SHP");
    DarkenShapes = MFCD::RetrieveT<ShapeSet>("DARKEN.SHP");
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::Init_Clear -- Sets sidebar to a known (and deactivated) state     *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void NewSidebarClass::StripClass::Init_Clear()
{
    IsScrollingDown = false;
    IsScrolling = false;
    IsBuilding = false;
    TopIndex = 0;
    Slid = 0;
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::Init_IO -- Initializes the strip's buttons                        *
 *                                                                                             *
 * This routine doesn't actually add any buttons to the list.                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/24/1994 JLB : Created.                                                                 *
 *=============================================================================================*/
void NewSidebarClass::StripClass::Init_IO()
{
    UpButton.IsSticky = true;
    UpButton.ID = BUTTON_UP + ID;
    UpButton.DrawOnSidebar = true;
    UpButton.ShapeDrawer = SidebarDrawer;
    UpButton.Set_Flags(GadgetClass::RIGHTRELEASE | GadgetClass::RIGHTPRESS | GadgetClass::LEFTRELEASE | GadgetClass::LEFTPRESS);

    DownButton.IsSticky = true;
    DownButton.ID = BUTTON_DOWN + ID;
    DownButton.DrawOnSidebar = true;
    DownButton.ShapeDrawer = SidebarDrawer;
    DownButton.Set_Flags(GadgetClass::RIGHTRELEASE | GadgetClass::RIGHTPRESS | GadgetClass::LEFTRELEASE | GadgetClass::LEFTPRESS);

    int max_visible = Max_Visible() * Columns;
    SelectButton.resize(max_visible);

    for (int index = 0; index < SelectButton.size(); index++) {
        SelectClass& g = SelectButton[index];
        g.ID = BUTTON_SELECT;
        g.X = SidebarRect.X + Position.X + index % Columns * OptionsExtension->SidebarControls.ObjectWidth;
        g.Y = SidebarRect.Y + Position.Y + index / Columns * OptionsExtension->SidebarControls.ObjectHeight;
        g.Width = OptionsExtension->SidebarControls.CameoWidth;
        g.Height = OptionsExtension->SidebarControls.CameoHeight;
        g.Set_Owner(*this, index);
    }

    if (OptionsExtension->SidebarControls.IsTabs) {
        TabButton.IsSticky = true;
        TabButton.ID = BUTTON_TAB + ID;
        TabButton.ShapeDrawer = SidebarDrawer;
        TabButton.IsSelected = false;
        TabButton.IsDisabled = true;
    }
}


void NewSidebarClass::StripClass::Set_Dimensions()
{
    Point2D up_position = SidebarRect.TopLeft + Position + Point2D(0, Max_Visible() * OptionsExtension->SidebarControls.ObjectHeight) + OptionsExtension->SidebarControls.UpButtonOffset;
    UpButton.Set_Position(up_position.X, up_position.Y);
    UpButton.Flag_To_Redraw();
    UpButton.DrawOffsetX = -SidebarRect.X;

    Point2D down_position = SidebarRect.TopLeft + Position + Point2D(0, Max_Visible() * OptionsExtension->SidebarControls.ObjectHeight) + OptionsExtension->SidebarControls.DownButtonOffset;
    DownButton.Set_Position(down_position.X, down_position.Y);
    DownButton.Flag_To_Redraw();
    DownButton.DrawOffsetX = -SidebarRect.X;

    for (int index = 0; index < SelectButton.size(); index++) {
        const int x = SidebarRect.X + Position.X + index % Columns * OptionsExtension->SidebarControls.ObjectWidth;
        const int y = SidebarRect.Y + Position.Y + index / Columns * OptionsExtension->SidebarControls.ObjectHeight;
        SelectButton[index].Set_Position(x, y);
    }

    for (int i = 0; i < SelectButton.size(); i++) {
        ToolTip tooltip;
        tooltip.Region = Rect(SelectButton[i].X, SelectButton[i].Y, SelectButton[i].Width, SelectButton[i].Height);
        tooltip.ID = (i | (ID << 8)) + 1000;
        tooltip.Text = TXT_NONE;
        ToolTips->Add(&tooltip);
    }

    if (OptionsExtension->SidebarControls.IsTabs) {
        TabButton.Set_Position(SidebarRect.X + OptionsExtension->SidebarControls.TabButtonOffset[ID].X, SidebarRect.Y + OptionsExtension->SidebarControls.TabButtonOffset[ID].Y);
        TabButton.DrawOffsetX = -SidebarRect.X;
    }
}



/***********************************************************************************************
 * NewSidebarClass::StripClass::Activate -- Adds the strip buttons to the input system.           *
 *                                                                                             *
 *    This routine will add the side strip buttons to the map's input system. This routine     *
 *    should be called once when the sidebar activates.                                        *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Never call this routine a second time without first calling Deactivate().       *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void NewSidebarClass::StripClass::Activate()
{
    IsActive = true;

    UpButton.Zap();
    Map.Add_A_Button(UpButton);

    DownButton.Zap();
    Map.Add_A_Button(DownButton);

    for (auto& select : SelectButton) {
        select.Zap();
        Map.Add_A_Button(select);
    }

    if (OptionsExtension->SidebarControls.IsTabs) {
        TabButton.Select();
    }
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::Deactivate -- Removes the side strip buttons from the input syste *
 *                                                                                             *
 *    Call this routine to remove all the buttons on the side strip from the map's input       *
 *    system.                                                                                  *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   Never call this routine unless the Activate() function was previously called.   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void NewSidebarClass::StripClass::Deactivate()
{
    IsActive = false;
    Map.Remove_A_Button(UpButton);
    Map.Remove_A_Button(DownButton);
    for (auto& select : SelectButton) {
        Map.Remove_A_Button(select);
    }
    if (OptionsExtension->SidebarControls.IsTabs) {
        TabButton.Deselect();
    }
}


/**
 *  Comparison function for sorting sidebar icons (BuildTypes)
 *
 *  @author: Rampastring, ZivDero
 */
static bool BuildType_Less(const NewSidebarClass::StripClass::BuildType& lhs, const NewSidebarClass::StripClass::BuildType& rhs)
{
    auto first_side = [](unsigned owners) -> int {
        int side = INT_MAX;
        for (int i = 0; i < HouseTypes.Count(); i++) {
            if (owners & (1 << i)) side = std::min<int>(HouseTypes[i]->Side, side);
        }
        return side != INT_MAX ? side : SIDE_NONE;
    };

    auto is_side_owner = [](const HouseClass* house, unsigned owners) -> int {
        // The house owns the object directly
        if (owners & 1 << house->ActLike) return true;

        const SideType side = house->Class->Side;
        for (int i = 0; i < HouseTypes.Count(); i++) {
            if ((owners & 1 << i) && HouseTypes[i]->Side == side) return true;
        }

        return false;
    };

    if (lhs.BuildableType == rhs.BuildableType) {

        /**
         *  If both are SWs, the one that recharges quicker goes first,
         *  otherwise sort by ID.
         */
        if (lhs.BuildableType == RTTI_SPECIAL || lhs.BuildableType == RTTI_SUPERWEAPONTYPE) {
            if (SuperWeaponTypes[lhs.BuildableID]->RechargeTime != SuperWeaponTypes[rhs.BuildableID]->RechargeTime) {
                return (SuperWeaponTypes[lhs.BuildableID]->RechargeTime - SuperWeaponTypes[rhs.BuildableID]->RechargeTime) < 0;
            }

            return (lhs.BuildableID - rhs.BuildableID) < 0;
        }


        const TechnoTypeClass* t1 = Fetch_Techno_Type(lhs.BuildableType, lhs.BuildableID);
        const TechnoTypeClass* t2 = Fetch_Techno_Type(rhs.BuildableType, rhs.BuildableID);

        /**
         *  If both are Buildings, non-defenses come first, then walls, then gates, then base defenses
         */
        if (lhs.BuildableType == RTTI_BUILDINGTYPE && OptionsExtension->SortDefensesAsLast) {
            const auto b1 = static_cast<const BuildingTypeClass*>(t1);
            const auto b2 = static_cast<const BuildingTypeClass*>(t2);
            const auto ext1 = Extension::Fetch(t1);
            const auto ext2 = Extension::Fetch(t2);

            enum {
                BCAT_NORMAL,
                BCAT_WALL,
                BCAT_GATE,
                BCAT_DEFENSE
            };

            int building_category1 = (b1->IsWall || b1->IsFirestormWall || b1->IsLaserFencePost || b1->IsLaserFence) ? BCAT_WALL : (b1->IsGate ? BCAT_GATE : (ext1->IsSortCameoAsBaseDefense ? BCAT_DEFENSE : BCAT_NORMAL));
            int building_category2 = (b2->IsWall || b2->IsFirestormWall || b2->IsLaserFencePost || b2->IsLaserFence) ? BCAT_WALL : (b2->IsGate ? BCAT_GATE : (ext2->IsSortCameoAsBaseDefense ? BCAT_DEFENSE : BCAT_NORMAL));

            // Compare based on category priority
            if (building_category1 != building_category2) return (building_category1 - building_category2) < 0;
        }

        /**
         *  If both are Units, non-naval units come first
         */
        if (lhs.BuildableType == RTTI_UNITTYPE) {
            const auto ext1 = Extension::Fetch(t1);
            const auto ext2 = Extension::Fetch(t2);
            if (ext1->IsNaval != ext2->IsNaval) {
                return (static_cast<int>(ext1->IsNaval) - static_cast<int>(ext2->IsNaval)) < 0;
            }
        }

        /**
         *  If your side owns one of the objects, but not another, yours comes first
         */
        const int owns1 = is_side_owner(PlayerPtr, t1->Get_Ownable());
        const int owns2 = is_side_owner(PlayerPtr, t2->Get_Ownable());
        if (owns1 != owns2) {
            return (owns2 - owns1) < 0;
        }

        /**
         *  If you don't own either of the objects, then sort by side index
         */
        if (!owns1 && !owns2) {
            const int side1 = first_side(t1->Get_Ownable());
            const int side2 = first_side(t2->Get_Ownable());
            if (side1 != side2) {
                return (side1 - side2) < 0;
            }
        }

        return (lhs.BuildableID - rhs.BuildableID) < 0;
    }

    if (lhs.BuildableType == RTTI_SPECIAL || lhs.BuildableType == RTTI_SUPERWEAPONTYPE) return false;
    if (rhs.BuildableType == RTTI_SPECIAL || rhs.BuildableType == RTTI_SUPERWEAPONTYPE) return true;
    if (lhs.BuildableType == RTTI_INFANTRYTYPE) return false;
    if (rhs.BuildableType == RTTI_INFANTRYTYPE) return true;
    if (lhs.BuildableType == RTTI_AIRCRAFTTYPE) return false;
    if (rhs.BuildableType == RTTI_AIRCRAFTTYPE) return true;
    if (lhs.BuildableType == RTTI_UNITTYPE) return false;
    if (rhs.BuildableType == RTTI_UNITTYPE) return true;

    return (lhs.BuildableID - rhs.BuildableID) < 0;
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::Add -- Add an object to the side strip.                           *
 *                                                                                             *
 *    Use this routine to add a buildable object to the side strip.                            *
 *                                                                                             *
 * INPUT:   object   -- Pointer to the object type that can be built and is to be added to     *
 *                      the side strip.                                                        *
 *                                                                                             *
 * OUTPUT:  bool; Was the object successfully added to the side strip? Failure could be the    *
 *                result of running out of room in the side strip array or the object might    *
 *                already be in the list.                                                      *
 *                                                                                             *
 * WARNINGS:   none.                                                                           *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   9/24/2019 3:17PM : Added via capture parameter for new sidebar functionality              *
 *=============================================================================================*/
bool NewSidebarClass::StripClass::Add(RTTIType type, int id)
{
    for (auto& buildable : Buildables) {
        if (buildable.BuildableType == type && buildable.BuildableID == id) {
            return false;
        }
    }

    if (!ScenarioInit && type != RTTI_SPECIAL) {
        Speak(VOX_NEW_CONSTRUCT);
    }
    Buildables.emplace_back(id, type);
    std::sort(Buildables.begin(), Buildables.end(), BuildType_Less);
    IsToRedraw = true;
    return true;
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::Scroll -- Causes the side strip to scroll.                        *
 *                                                                                             *
 *    Use this routine to flag the side strip to scroll. The direction scrolled is controlled  *
 *    by the parameter. Scrolling is merely initiated by this routine. Subsequent calls to     *
 *    the AI function and the Draw_It function are required to properly give the appearance    *
 *    of scrolling.                                                                            *
 *                                                                                             *
 * INPUT:   bool; Should the side strip scroll UP? If it is to scroll down then pass false.    *
 *                                                                                             *
 * OUTPUT:  bool; Was the side strip started to scroll in the desired direction?               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   07/29/1995 JLB : Simplified scrolling logic.                                              *
 *=============================================================================================*/
bool NewSidebarClass::StripClass::Scroll(bool up)
{
    if (up) {
        if (!TopIndex) return false;
        Scroller -= Columns;
    } else {
        if (TopIndex + Max_Visible() * Columns >= static_cast<int>(Buildables.size())) return false;
        Scroller += Columns;
    }
    return true;
}


bool NewSidebarClass::StripClass::Page(bool up)
{
    if (up) {
        if (!TopIndex) return false;
        Scroller -= Max_Visible() * Columns;
    } else {
        if (TopIndex + Max_Visible() * Columns >= static_cast<int>(Buildables.size())) return false;
        Scroller += Max_Visible() * Columns;
    }
    return true;
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::Flag_To_Redraw -- Flags the sidebar strip to be redrawn.          *
 *                                                                                             *
 *    This utility routine is called when something changes on the sidebar and it must be      *
 *    reflected the next time drawing is performed.                                            *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void NewSidebarClass::StripClass::Flag_To_Redraw()
{
    IsToRedraw = true;
    Map.Flag_To_Redraw();
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::AI -- Input and AI processing for the side strip.                 *
 *                                                                                             *
 *    The side strip AI processing is performed by this function. This function not only       *
 *    checks for player input, but also handles any graphic logic updating necessary as a      *
 *    result of flashing or construction animation.                                            *
 *                                                                                             *
 * INPUT:   input -- The player input code.                                                    *
 *                                                                                             *
 *          x,y   -- Mouse coordinate to use.                                                  *
 *                                                                                             *
 * OUTPUT:  bool; Did the AI detect that it will need a rendering change? If this routine      *
 *                returns true, then the Draw_It function should be called at the              *
 *                earliest opportunity.                                                        *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   12/31/1994 JLB : Uses mouse coordinate parameters.                                        *
 *=============================================================================================*/
bool NewSidebarClass::StripClass::AI(KeyNumType& input)
{
#define KN_UNK 0x4000

    KeyNumType key = static_cast<KeyNumType>(input & ~KN_UNK);
    bool redraw = false;

    Tab_AI();

    /*
    **	Handle any building clock animation logic.
    */
    if (IsBuilding) {
        for (auto& buildable : Buildables) {
            FactoryClass* factory = buildable.Factory;

            if (factory && factory->Has_Changed()) {
                redraw = true;
                if (factory->Has_Completed()) {

                    /*
                    **	Construction has been completed. Announce this fact to the player and
                    **	try to get the object to automatically leave the factory. Buildings are
                    **	the main exception to the ability to leave the factory under their own
                    **	power.
                    */
                    TechnoClass* pending = factory->Get_Object();
                    if (pending != nullptr) {
                        switch (pending->RTTI) {
                        case RTTI_UNIT:
                        case RTTI_INFANTRY:
                        case RTTI_AIRCRAFT:
                            OutList.Add(EventClassExt(pending->Owner(), EVENT_PLACE, pending->RTTI, CELL_NONE, TechnoTypeClassExtension::Get_Production_Flags(pending)).As_Event());
                            break;

                        case RTTI_BUILDING:
                            if (OptionsExtension->SidebarControls.IsTabs) {
                                TabButton.Start_Flashing();
                            }
                            break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    if (!IsActive) {
        return false;
    }

    /*
    **	If this is scroll button for this side strip, then scroll the strip as
    **	indicated.
    */
    if (key == static_cast<KeyNumType>(UpButton.ID | KN_BUTTON)) {
        UpButton.IsPressed = false;
        if ((input & KN_UNK) != 0) {
            if (!Page(true)) {
                Sound_Effect(Rule->ScoldSound);
            }
        } else {
            if (!Scroll(true)) {
                Sound_Effect(Rule->ScoldSound);
            }
        }
    } else if (key == static_cast<KeyNumType>(DownButton.ID | KN_BUTTON)) {
        DownButton.IsPressed = false;
        if ((input & KN_UNK) != 0) {
            if (!Page(false)) {
                Sound_Effect(Rule->ScoldSound);
            }
        } else {
            if (!Scroll(false)) {
                Sound_Effect(Rule->ScoldSound);
            }
        }
    }

    /*
    **	Reflect the scroll desired direction/value into the scroll
    **	logic handler. This might result in up or down scrolling.
    */
    if (!IsScrolling && Scroller) {
        if (Buildables.size() <= Max_Visible() * Columns) {
            Scroller = 0;
        } else {

            /*
            **	Top of list is moving toward lower ordered entries in the object list. It looks like
            **	the "window" to the object list is moving up even though the actual object images are
            **	scrolling downward.
            */
            if (Scroller < 0) {
                if (TopIndex <= 0) {
                    TopIndex = 0;
                    Scroller = 0;
                } else {
                    Scroller++;
                    IsScrollingDown = false;
                    IsScrolling = true;
                    TopIndex -= Columns;
                    Slid = 0;
                }

            } else {
                if (TopIndex + Max_Visible() * Columns >= Buildables.size()) {
                    Scroller = 0;
                } else {
                    Scroller--;
                    Slid = OptionsExtension->SidebarControls.ObjectHeight;
                    IsScrollingDown = true;
                    IsScrolling = true;
                }
            }
        }
    }

    /*
    **	Scroll logic is handled here.
    */
    if (IsScrolling) {
        if (IsScrollingDown) {
            Slid -= OptionsExtension->SidebarControls.ScrollRate;
            if (Slid <= 0) {
                IsScrolling = false;
                Slid = 0;
                TopIndex += Columns;
            }
        } else {
            Slid += OptionsExtension->SidebarControls.ScrollRate;
            if (Slid >= OptionsExtension->SidebarControls.ObjectHeight) {
                IsScrolling = false;
                Slid = 0;
            }
        }
        redraw = true;
    }

    /*
    **	If any of the logic determined that this side strip needs to be redrawn, then
    **	set the redraw flag for this side strip.
    */
    if (redraw) {
        Flag_To_Redraw();
        RedrawSidebar = true;
    }

    return redraw;
}


char const* NewSidebarClass::StripClass::Help_Text(int id) const
{
    int index = id + TopIndex;

    if (GameActive) {
        if (index < Buildables.size()) {
            return Buildables[index].Help_Text();
        }
    }

    return nullptr;
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::Draw_It -- Render the sidebar display.                            *
 *                                                                                             *
 *    Use this routine to render the sidebar display. It checks to see if it needs to be       *
 *    redrawn and only redraw if necessary. If the "complete" parameter is true, then it       *
 *    will force redraw the entire strip.                                                      *
 *                                                                                             *
 * INPUT:   complete -- Should the redraw be forced? A force redraw will ignore the redraw     *
 *                      flag.                                                                  *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   12/31/1994 JLB : Created.                                                                 *
 *   08/06/1995 JLB : Handles multi factory tracking in same strip.                            *
 *=============================================================================================*/
void NewSidebarClass::StripClass::Draw_It(bool complete)
{
    if (IsToRedraw || complete) {
        IsToRedraw = false;
        RedrawSidebar = true;

        /*
        **	Redraw the scroll buttons.
        */
        UpButton.Draw_Me(true);
        DownButton.Draw_Me(true);

        /*
        **	Loop through all the buildable objects that are visible in the strip and render
        **	them. Their Y offset may be adjusted if the strip is in the process of scrolling.
        */
        for (int index = TopIndex; index < Buildables.size() && index < TopIndex + Max_Visible() * Columns + (IsScrolling ? Columns : 0); index++) {

            int x = Position.X + (index - TopIndex) % Columns * OptionsExtension->SidebarControls.ObjectWidth;
            int y = Position.Y + (index - TopIndex) / Columns * OptionsExtension->SidebarControls.ObjectHeight;

            /*
            **	If the strip is scrolling, then the offset is adjusted accordingly.
            */
            if (IsScrolling) {
                y -= OptionsExtension->SidebarControls.ObjectHeight - Slid;
            }

            Buildables[index].Draw_It(Point2D(x, y), SelectButton[index - TopIndex].Is_Moused_Over());
        }

        LastSlid = Slid;
    }

    if (UpButton.IsDrawn) {
        RedrawSidebar = true;
        UpButton.IsDrawn = false;
    }

    if (DownButton.IsDrawn) {
        RedrawSidebar = true;
        DownButton.IsDrawn = false;
    }
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::Recalc -- Revalidates the current sidebar list of objects.        *
 *                                                                                             *
 *    This routine will revalidate all the buildable objects in the sidebar. This routine      *
 *    comes in handy when a factory has been destroyed, and the sidebar needs to reflect any   *
 *    change that this requires. It checks every object to see if there is a factory available *
 *    that could produce it. If none can be found, then the object is removed from the         *
 *    sidebar.                                                                                 *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  bool; The sidebar has changed as a result of this call?                            *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *   06/26/1995 JLB : Doesn't collapse sidebar when buildables removed.                        *
 *=============================================================================================*/
bool NewSidebarClass::StripClass::Recalc()
{
    int ok;

    if (Debug_Map || Buildables.empty()) {
        return false;
    }

    bool scroll = false;
    std::vector<BuildType> unshifted = Buildables;

    /*
    **	Sweep through all objects listed in the sidebar. If any of those object can
    **	not be created -- even in theory -- then they must be removed form the sidebar and
    **	any current production must be abandoned.
    */
    bool redraw = false;
    for (int index = 0; index < Buildables.size(); index++) {
        TechnoTypeClass const* tech = Fetch_Techno_Type(Buildables[index].BuildableType, Buildables[index].BuildableID);
        if (tech != nullptr) {
            BuildingClass const* who = tech->Who_Can_Build_Me(true, false, false, PlayerPtr);
            ok = who != nullptr && who->House->Can_Build(tech, true, true);
        } else {
            if (Buildables[index].BuildableID < PlayerPtr->SuperWeapon.Count()) {
                ok = PlayerPtr->SuperWeapon[Buildables[index].BuildableID]->Is_Present();
            } else {
                ok = false;
            }
        }

        if (!ok) {
            for (auto& buildable : unshifted) {
                if (buildable == Buildables[index]) {
                    buildable = BuildType(0, RTTI_NONE);
                }
            }

            /*
            **	Removes this entry from the list.
            */
            Buildables.erase(Buildables.begin() + index);
            redraw = true;
            scroll = true;
            IsToRedraw = true;
            index--;
        }
    }

    if (scroll) {
        bool got_old = false;
        bool got_new = false;

        int oldpos = 0;
        for (oldpos = 0; oldpos < unshifted.size(); oldpos++) {
            if (unshifted[oldpos] != BuildType(0, RTTI_NONE)) {
                got_old = true;
                break;
            }
        }

        int newpos = 0;
        if (got_old && !Buildables.empty()) {
            for (newpos = 0; newpos < Buildables.size(); newpos++) {
                if (Buildables[newpos] == unshifted[oldpos]) {
                    got_new = true;
                    break;
                }
            }
        }

        if (got_old && got_new) {
            TopIndex = newpos - oldpos;
            TopIndex = std::max(0, std::min(TopIndex, static_cast<int>(Buildables.size()) - Max_Visible() * Columns));
        } else {
            TopIndex = 0;
        }
    }

    return redraw;
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::SelectClass::SelectClass -- Default constructor.                  *
 *                                                                                             *
 *    This is the default constructor for the button that controls the buildable cameos on     *
 *    the sidebar strip.                                                                       *
 *                                                                                             *
 * INPUT:   none                                                                               *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   The coordinates are set to zero by this routine. They must be set to the        *
 *             correct values before this button will function.                                *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
NewSidebarClass::StripClass::SelectClass::SelectClass() :
    ControlClass(0, 0, 0, OptionsExtension->SidebarControls.CameoWidth, OptionsExtension->SidebarControls.CameoHeight, LEFTPRESS | RIGHTPRESS | LEFTUP),
    Strip(nullptr),
    Index(0) {}


/***********************************************************************************************
 * NewSidebarClass::StripClass::SelectClass:: -- Assigns special values to a buildable select but *
 *                                                                                             *
 *    Use this routine to set custom buildable vars for this particular select button. It      *
 *    uses this information to properly know what buildable object to start or stop production *
 *    on.                                                                                      *
 *                                                                                             *
 * INPUT:   strip    -- Reference to the strip that owns this buildable button.                *
 *                                                                                             *
 *          index    -- The index (0 .. MAX_VISIBLE-1) of this button. This is used to let     *
 *                      the owning strip know what index this button refers to.                *
 *                                                                                             *
 * OUTPUT:  none                                                                               *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
void NewSidebarClass::StripClass::SelectClass::Set_Owner(StripClass& strip, int index)
{
    Strip = &strip;
    Index = index;
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::SelectClass:: -- Action function when buildable cameo is selected *
 *                                                                                             *
 *    This function is called when the buildable icon (cameo) is clicked on. It handles        *
 *    starting and stopping production as indicated.                                           *
 *                                                                                             *
 * INPUT:   flags -- The input event that triggered the call.                                  *
 *                                                                                             *
 *          key   -- The keyboard value at the time of the input.                              *
 *                                                                                             *
 * OUTPUT:  Returns with whether the input list should be scanned further.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   01/19/1995 JLB : Created.                                                                 *
 *   10/09/1996 JLB : Sonar pulse converted to regular event type.                             *
 *=============================================================================================*/
int NewSidebarClass::StripClass::SelectClass::Action(unsigned flags, KeyNumType& key)
{
    if (Strip == nullptr) {
        return 1;
    }

    Map.Override_Mouse_Shape(MOUSE_NORMAL);

    int index = Strip->TopIndex + Index;
    if (index >= Strip->Buildables.size()) {
        return 1;
    }

    BuildType& buildable = Strip->Buildables[index];

    /*
    **	Display the help text if the mouse is over the button.
    */
    if (flags & LEFTUP) {
        flags &= ~LEFTUP;
    }

    /*
    **	A right mouse button signals "cancel".  If we are in targeting
    ** mode then we don't want to be any more.
    */
    if (flags & RIGHTPRESS) {
        buildable.On_Right_Press(flags);
    }

    /*
    **	A left mouse press signal "activate".  If our weapon type is
    ** available then we should activate it.
    */
    if (flags & LEFTPRESS) {
        buildable.On_Left_Press(flags);
    }

    ControlClass::Action(flags, key);

    return true;
}


/**
 *  Function that gets called when the mouse enters the button.
 *  Used for hover effects.
 *
 *  @author: ZivDero
 */
void NewSidebarClass::StripClass::SelectClass::On_Mouse_Enter()
{
    MousedOver = true;
    Map.SidebarClass::IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Function that gets called when the mouse leaves the button.
 *  Used for hover effects.
 *
 *  @author: ZivDero
 */
void NewSidebarClass::StripClass::SelectClass::On_Mouse_Leave()
{
    MousedOver = false;
    Map.SidebarClass::IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/***********************************************************************************************
 * NewSidebarClass::SBGadgetClass::Action -- Special function that controls the mouse over the si *
 *                                                                                             *
 *    This routine is called whenever the mouse is over the sidebar. It makes sure that the    *
 *    mouse is always the normal shape while over the sidebar.                                 *
 *                                                                                             *
 * INPUT:   flags -- The event flags that resulted in this routine being called.               *
 *                                                                                             *
 *          key   -- Reference the keyboard code that may be present.                          *
 *                                                                                             *
 * OUTPUT:  Returns that no further keyboard processing is necessary.                          *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   03/28/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
int NewSidebarClass::SBGadgetClass::Action(unsigned, KeyNumType&)
{
    Map.Override_Mouse_Shape(MOUSE_NORMAL, false);
    return true;
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::Factory_Link -- Links a factory to a sidebar button.              *
 *                                                                                             *
 *    This routine will link the specified factory to this sidebar strip. The exact button to  *
 *    link to is determined from the object type and id specified. A linked button is one that *
 *    will show appropriate construction animation (clock shape) that matches the state of     *
 *    the factory.                                                                             *
 *                                                                                             *
 * INPUT:   factory  -- The factory number to link to the sidebar.                             *
 *                                                                                             *
 *          type     -- The object type that this factory refers to.                           *
 *                                                                                             *
 *          id       -- The object sub-type that this factory refers to.                       *
 *                                                                                             *
 * OUTPUT:  Was the factory successfully attached? Failure would indicate that there is no     *
 *          object of the specified type and sub-type in the sidebar list.                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *=============================================================================================*/
bool NewSidebarClass::StripClass::Factory_Link(FactoryClass* factory, RTTIType type, int id)
{
    for (auto& Buildable : Buildables) {
        if (Buildable.BuildableType == type && Buildable.BuildableID == id) {
            Buildable.Factory = factory;

            IsBuilding = true;
            /*
            **  Flag that all the icons on this strip need to be redrawn
            */
            Flag_To_Redraw();
            return true;
        }
    }
    return false;
}

void NewSidebarClass::StripClass::Tab_AI()
{
    if (!OptionsExtension->SidebarControls.IsTabs) {
        return;
    }

    if (Buildables.empty()) {
        if (TabButton.Is_Enabled()) {
            TabButton.Disable();
        }
    } else {
        if (!TabButton.Is_Enabled()) {
            TabButton.Enable();
        }

        int building_tab = Which_Column(RTTI_BUILDINGTYPE, PRODFLAG_NONE);
        int defenses_tab = Which_Column(RTTI_BUILDINGTYPE, PRODFLAG_DEFENSE);
        if (ID == building_tab || ID == defenses_tab) {
            if (TabButton.IsFlashing) {
                FactoryClass* fptr = Extension::Fetch(PlayerPtr)->Fetch_Factory(RTTI_BUILDINGTYPE, ID == defenses_tab ? PRODFLAG_DEFENSE : PRODFLAG_NONE);
                if (fptr == nullptr || !fptr->Has_Completed()) TabButton.Stop_Flashing();
            }
        }

        int special_tab = Which_Column(RTTI_SPECIAL, PRODFLAG_NONE);
        if (ID == special_tab) {
            bool ready_sw = false;
            for (int i = 0; i < PlayerPtr->SuperWeapon.Count(); i++) {
                SuperClass* sw = PlayerPtr->SuperWeapon[i];
                if (sw->Can_Place() && !sw->Class->IsUseChargeDrain) { // Firestorm is always "ready", so don't flash for it.
                    ready_sw = true;
                    break;
                }
            }

            if (ready_sw && !TabButton.IsFlashing) {
                TabButton.Start_Flashing();
            } else if (!ready_sw && TabButton.IsFlashing) {
                TabButton.Stop_Flashing();
            }
        }
    }
}

bool NewSidebarClass::StripClass::Is_On_Sidebar(RTTIType type, int id) const
{
    for (auto& build : Buildables) {
        if (build.BuildableType == type && build.BuildableID == id) {
            return true;
        }
    }
    return false;
}


/***********************************************************************************************
 * NewSidebarClass::StripClass::Abandon_Produ -- Abandons production associated with sidebar.     *
 *                                                                                             *
 *    Production of the object associated with this sidebar is abandoned when this routine is  *
 *    called.                                                                                  *
 *                                                                                             *
 * INPUT:   factory  -- The factory index that is to be suspended.                             *
 *                                                                                             *
 * OUTPUT:  Was the production abandonment successful?                                         *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   05/18/1995 JLB : Created.                                                                 *
 *   08/06/1995 JLB : More intelligent abandon logic for multiple factories.                   *
 *=============================================================================================*/
bool NewSidebarClass::StripClass::Abandon_Production(FactoryClass const* factory)
{
    bool noprod = true;
    bool abandon = false;
    for (auto& buildable : Buildables) {
        if (buildable.Factory == factory) {
            const_cast<FactoryClass*>(factory)->Abandon();
            buildable.Factory = nullptr;
            abandon = true;
        } else {
            if (buildable.Factory != nullptr) {
                noprod = false;
            }
        }
    }

    /*
    **	If there was a change to the strip, then flag the strip to be redrawn.
    */
    if (abandon) {
        Flag_To_Redraw();
    }

    /*
    **	If there is no production whatsoever on this strip, then flag it so.
    */
    if (noprod) {
        IsBuilding = false;
    }
    return abandon;
}


void NewSidebarClass::StripClass::Init_For_House()
{
    UpButton.Set_Shape(MFCD::RetrieveT<ShapeSet>("R-UP.SHP"));
    UpButton.ShapeDrawer = SidebarDrawer;

    DownButton.Set_Shape(MFCD::RetrieveT<ShapeSet>("R-DN.SHP"));
    DownButton.ShapeDrawer = SidebarDrawer;

    char const* TabShapes[6] = {"TAB-BLD.SHP", "TAB-INF.SHP", "TAB-UNT.SHP", "TAB-SPC.SHP", "TAB-SPC.SHP", "TAB-SPC.SHP"};

    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "TAB%02d.SHP", ID);

    TabButton.Set_Shape(MFCD::RetrieveT<ShapeSet>(buffer));
    TabButton.ShapeDrawer = SidebarDrawer;
}


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
NewSidebarClass::StripClass::TabButtonClass::TabButtonClass() :
ControlClass(0, 0, 0, 0, 0, LEFTPRESS | LEFTRELEASE, true),
    DrawOffsetX(0),
    DrawOffsetY(0),
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
NewSidebarClass::StripClass::TabButtonClass::TabButtonClass(unsigned id, const ShapeSet* shapes, int x, int y, ConvertClass* drawer, int w, int h) :
    ControlClass(id, x, y, w, h, LEFTPRESS | LEFTRELEASE, true),
    DrawOffsetX(0),
    DrawOffsetY(0),
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
int NewSidebarClass::StripClass::TabButtonClass::Action(unsigned flags, KeyNumType& key)
{
    /*
    **  If there are no action flag bits set, then this must be a forced call. A forced call
    **  must never actually function like a real call, but rather only performs any necessary
    **  graphic updating.
    */
    if (!flags) {
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
    if (flags & LEFTPRESS) {
        flags &= ~LEFTPRESS;
        ControlClass::Action(flags, key);
        key = KN_NONE; // erase the event
        return true;   // stop processing other buttons now
    }

    /*
    **  Act on mouse release.
    */
    if (flags & LEFTRELEASE) {
        bool overbutton = MouseCursor->Get_Mouse_X() - X < Width && MouseCursor->Get_Mouse_Y() - Y < Height;
        if (!IsSelected && overbutton) {
            IsSelected = true;
            Flag_To_Redraw();
        } else {
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
void NewSidebarClass::StripClass::TabButtonClass::Disable()
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
void NewSidebarClass::StripClass::TabButtonClass::Enable()
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
bool NewSidebarClass::StripClass::TabButtonClass::Draw_Me(bool forced)
{
    if (!ControlClass::Draw_Me(forced)) return false;

    if (!ShapeData) return false;

    if (!ShapeDrawer) return false;

    int shapenum;

    // A disabled tab always looks darkened
    if (IsDisabled) {
        shapenum = FRAME_DISABLED;
    } else if (IsSelected) {
        shapenum = FRAME_SELECTED;
    } else if (IsFlashing) {
        if (FlashTimer.Expired()) {
            // If we're at the edge of flashing frames, restart
            if (FlashFrame == FLASH_FRAME_MAX) {
                FlashFrame = FLASH_FRAME_MIN;
            } else {
                FlashFrame++;
            }

            FlashTimer = FLASH_RATE;
        }

        shapenum = FlashFrame;
    } else {
        // Just the normal unselected tab
        shapenum = FRAME_NORMAL;
    }

    Draw_Shape(*SidebarSurface, *ShapeDrawer, ShapeData, shapenum, Point2D(X + DrawOffsetX, Y + DrawOffsetY), VisibleRect, SHAPE_NORMAL);

    if (MousedOver && !Scen->InputLock && !IsDisabled && !IsSelected) {
        Rect hover_rect(X + DrawOffsetX, Y + DrawOffsetY, Width - 1, Height - 1);
        const ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->UIColor;
        SidebarSurface->Draw_Rect(hover_rect, DSurface::RGB_To_Pixel(ColorSchemes[colorschemetype]->HSV.operator RGBClass()));
    }

    IsDrawn = true;

    return true;
}


/**
 *  Sets the shape of the button.
 *
 *  @author: ZivDero
 */
void NewSidebarClass::StripClass::TabButtonClass::Set_Shape(const ShapeSet* data, int width, int height)
{
    ShapeData = data;
    if (ShapeData) {
        Width = ShapeData->Get_Width();
        Height = ShapeData->Get_Height();
    }

    if (width != 0) Width = width;

    if (height != 0) Height = height;
}


/**
 *  Function that gets called when the mouse enters the button.
 *  Used for hover effects.
 *
 *  @author: Rampastring
 */
void NewSidebarClass::StripClass::TabButtonClass::On_Mouse_Enter()
{
    MousedOver = true;
    Map.SidebarClass::IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Function that gets called when the mouse leaves the button.
 *  Used for hover effects.
 *
 *  @author: Rampastring
 */
void NewSidebarClass::StripClass::TabButtonClass::On_Mouse_Leave()
{
    MousedOver = false;
    Map.SidebarClass::IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Makes the button start flashing.
 *
 *  @author: ZivDero
 */
void NewSidebarClass::StripClass::TabButtonClass::Start_Flashing()
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
void NewSidebarClass::StripClass::TabButtonClass::Stop_Flashing()
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
void NewSidebarClass::StripClass::TabButtonClass::Select()
{
    IsSelected = true;
}


/**
 *  Deselects the button.
 *
 *  @author: ZivDero
 */
void NewSidebarClass::StripClass::TabButtonClass::Deselect()
{
    IsSelected = false;
}


/**
 *  Function that checks if the mouse has entered/left a button.
 *  This function is hooked into GadgetClass::Input()
 *
 *  @author: ZivDero
 */
void IHoverableGadget::Process_Hover(GadgetClass* gadget, int mousex, int mousey)
{
    GadgetClass* to_enter = gadget->Extract_Gadget_At_Mouse(mousex, mousey);
    if (to_enter != LastHovered) {
        if (LastHovered) {
            if (auto hover = dynamic_cast<IHoverableGadget*>(LastHovered)) {
                hover->On_Mouse_Leave();
            }
            LastHovered = nullptr;
        }

        if (to_enter) {
            if (auto hover = dynamic_cast<IHoverableGadget*>(to_enter)) {
                LastHovered = to_enter;
                hover->On_Mouse_Enter();
            }
        }
    }
}


void NewSidebarClass::StripClass::BuildType::On_Left_Press(unsigned& flags)
{
    TechnoTypeClass const* choice = nullptr;
    SuperWeaponType spc = SUPER_NONE;
    FactoryClass* factory = Factory;

    if (BuildableType != RTTI_SPECIAL) {
        choice = Fetch_Techno_Type(BuildableType, BuildableID);
    } else {
        spc = static_cast<SuperWeaponType>(BuildableID);
    }

    if (spc != SUPER_NONE) {

        /*
        **	A left mouse press signal "activate".  If our weapon type is
        ** available then we should activate it.
        */
        if (spc < PlayerPtr->SuperWeapon.Count()) {
            if (PlayerPtr->SuperWeapon[spc]->Can_Place()) {
                if (PlayerPtr->SuperWeapon[spc]->Class->Action == ACTION_NONE) {
                    OutList.Add(EventClass(PlayerPtr->HeapID, EVENT_SPECIAL_PLACE, PlayerPtr->SuperWeapon[spc]->Class->HeapID, Cell(0, 0)));
                } else {
                    Map.IsTargettingMode = spc;
                    Unselect_All();
                    Speak(VOX_SELECT_TARGET);
                }
            } else {
                PlayerPtr->SuperWeapon[spc]->Impatient_Click();
            }
        }
    } else {
        if (choice != nullptr) {
            if (factory != nullptr && !factory->Is_Building()) {

                /*
                **	If production has completed, then attempt to have the object exit
                **	the Factory or go into placement mode.
                */
                if (factory->Has_Completed()) {

                    TechnoClass* pending = factory->Get_Object();
                    if (!pending) {
                        if (factory->Get_Special_Item() != -1) {
                            Map.IsTargettingMode = SUPER_ANY;
                        }
                    } else {
                        BuildingClass* builder = pending->Who_Can_Build_Me(false, false);
                        if (!builder) {
                            OutList.Add(EventClassExt(PlayerPtr->HeapID, EVENT_ABANDON, BuildableType, BuildableID, TechnoTypeClassExtension::Get_Production_Flags(BuildableType, BuildableID)).As_Event());
                            Speak(VOX_NO_FACTORY);
                        } else {

                            /*
                            **	If the completed object is a building, then change the
                            **	game state into building placement mode. This fact is
                            **	not transmitted to any linked computers until the moment
                            **	the building is actually placed down.
                            */
                            if (pending->RTTI == RTTI_BUILDING) {
                                PlayerPtr->Manual_Place(builder, static_cast<BuildingClass*>(pending));
                            } else {

                                /*
                                **	For objects that can leave the Factory under their own
                                **	power, queue this event and process through normal house
                                **	production channels.
                                */
                                OutList.Add(EventClassExt(pending->Owner(), EVENT_PLACE, BuildableType, CELL_NONE, TechnoTypeClassExtension::Get_Production_Flags(pending)).As_Event());
                            }
                        }
                    }
                } else {

                    /*
                    **	The Factory must have been in a suspended state. Resume construction
                    **	normally.
                    */
                    if (BuildableType == RTTI_INFANTRYTYPE) {
                        Speak(VOX_TRAINING);
                    } else {
                        Speak(VOX_BUILDING);
                    }
                    OutList.Add(EventClassExt(PlayerPtr->HeapID, EVENT_PRODUCE, BuildableType, BuildableID, TechnoTypeClassExtension::Get_Production_Flags(BuildableType, BuildableID)).As_Event());
                }

            } else {

                /*
                **	If there is already a Factory attached to this strip but the player didn't click
                **	on the icon that has the attached Factory, then say that the Factory is busy and
                **	ignore the click.
                */
                factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(BuildableType, TechnoTypeClassExtension::Get_Production_Flags(BuildableType, BuildableID));
                bool produce = false;
                if (factory != nullptr && (factory->Is_Building() || factory->Has_Production_Target())) {
                    if (BuildableType == RTTI_BUILDINGTYPE) {
                        Speak(VOX_NO_FACTORY);
                    } else {
                        produce = true;
                    }

                } else {

                    /*
                    **	If this side strip is already busy with production, then ignore the
                    **	input and announce this fact.
                    */
                    if (BuildableType == RTTI_INFANTRYTYPE) {
                        Speak(VOX_TRAINING);
                    } else {
                        Speak(VOX_BUILDING);
                    }
                    produce = true;
                }
                if (produce) {
                    const int count_to_produce = GetAsyncKeyState(VK_SHIFT) & 0x8000 ? 5 : 1;
                    for (int i = 0; i < count_to_produce; i++) {
                        OutList.Add(EventClassExt(PlayerPtr->HeapID, EVENT_PRODUCE, BuildableType, BuildableID, TechnoTypeClassExtension::Get_Production_Flags(BuildableType, BuildableID)).As_Event());
                    }
                }
            }
        } else {
            flags = 0;
        }
    }
}

void NewSidebarClass::StripClass::BuildType::On_Right_Press(unsigned& flags)
{
    TechnoTypeClass const* choice = nullptr;
    SuperWeaponType spc = SUPER_NONE;

    Map.Override_Mouse_Shape(MOUSE_NORMAL);

    if (BuildableType != RTTI_SPECIAL) {
        choice = Fetch_Techno_Type(BuildableType, BuildableID);
    } else {
        spc = static_cast<SuperWeaponType>(BuildableID);
    }

    if (spc != SUPER_NONE) {

        /*
        **	A right mouse button signals "cancel".  If we are in targeting
        ** mode then we don't want to be any more.
        */
        Map.IsTargettingMode = SUPER_NONE;

    } else {

        if (choice != nullptr) {

            /*
            **	A right mouse button signals "cancel".
            */

            /*
            **	If production is in progress, put it on hold. If production is already
            **	on hold, then abandon it. Money will be refunded, the Factory
            **	manager deleted, and the object under construction is returned to
            **	the free pool.
            */
            if (Factory != nullptr) {

                /*
                **	Cancels placement mode if the sidebar Factory is abandoned or
                **	suspended.
                */
                if (Map.PendingObjectPtr && As_TechnoClass(Map.PendingObjectPtr)) {
                    Map.PendingObjectPtr = nullptr;
                    Map.PendingObject = nullptr;
                    Map.PendingHouse = HOUSE_NONE;
                    Map.Set_Cursor_Shape(nullptr);
                }

                if (!Factory->Is_Building()) {
                    Speak(VOX_CANCELED);
                    int count_to_abandon = 1;

                    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                        count_to_abandon = Factory->Total(choice);
                    } else if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                        count_to_abandon = std::clamp(5, 0, Factory->Total(choice));
                    }

                    for (int i = 0; i < count_to_abandon; i++) {
                        OutList.Add(EventClassExt(PlayerPtr->HeapID, EVENT_ABANDON, BuildableType, BuildableID, TechnoTypeClassExtension::Get_Production_Flags(BuildableType, BuildableID)).As_Event());
                    }
                } else {
                    Speak(VOX_SUSPENDED);
                    OutList.Add(EventClassExt(PlayerPtr->HeapID, EVENT_SUSPEND, BuildableType, BuildableID, TechnoTypeClassExtension::Get_Production_Flags(BuildableType, BuildableID)).As_Event());
                    Sidebar->Get_Column(BuildableType, TechnoTypeClassExtension::Get_Production_Flags(BuildableType, BuildableID)).IsToRedraw = true;
                }
            } else {

                /*
                **	Cancel a queued production.
                */
                FactoryClass* fptr = Extension::Fetch(PlayerPtr)->Fetch_Factory(BuildableType, TechnoTypeClassExtension::Get_Production_Flags(choice));
                if (fptr != nullptr && fptr->Is_Queued(choice)) {
                    int count_to_abandon = 1;

                    if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
                        count_to_abandon = Factory->Total(choice);
                    } else if (GetAsyncKeyState(VK_CONTROL) & 0x8000) {
                        count_to_abandon = std::clamp(5, 0, Factory->Total(choice));
                    }

                    for (int i = 0; i < count_to_abandon; i++) {
                        OutList.Add(EventClassExt(PlayerPtr->HeapID, EVENT_ABANDON, BuildableType, BuildableID, TechnoTypeClassExtension::Get_Production_Flags(BuildableType, BuildableID)).As_Event());
                    }
                }
            }
        } else {
            flags = 0;
        }
    }
}

void NewSidebarClass::StripClass::BuildType::Draw_It(Point2D const& position, bool highlight) const
{
    Rect cliprect = SidebarRect;
    cliprect.X = 0;

    /*
     **	Fetch the shape number for the object type located at this current working
     **	slot. This shape pointer is used to draw the underlying graphic there.
     */
    ShapeSet const* shapefile = LogoShapes;
    BSurface* pcx_surface = nullptr;

    const TechnoTypeClass* ttype = nullptr;
    const SuperWeaponTypeClass* swtype = nullptr;

    if (BuildableType != RTTI_SPECIAL) {
        ttype = Fetch_Techno_Type(BuildableType, BuildableID);
        if (ttype != nullptr) {
            shapefile = ttype->Get_Cameo_Data();
            auto technotypeext = Extension::Fetch(ttype);
            if (technotypeext->CameoImageSurface != nullptr) {
                pcx_surface = technotypeext->CameoImageSurface;
            }
        }
    } else {
        if (BuildableID > SUPER_NONE && BuildableID < SuperWeaponTypes.Count()) {
            swtype = SuperWeaponTypes[BuildableID];
            shapefile = swtype->CameoData;
            auto supertypeext = Extension::Fetch(swtype);
            if (supertypeext->CameoImageSurface != nullptr) {
                pcx_surface = supertypeext->CameoImageSurface;
            }
        }
    }

    /*
     **	Now that the shape of the object at the current working slot has been found,
     **	draw it and any graphic overlays as necessary.
     */
    if (pcx_surface != nullptr || shapefile != LogoShapes) {

        /**
         *  #issue-487
         *
         *  Adds support for PCX/PNG cameo icons.
         *
         *  @author: CCHyper
         */
        if (pcx_surface != nullptr) {
            Rect pcxrect(cliprect.TopLeft + position, pcx_surface->Get_Width(), pcx_surface->Get_Height());
            SpriteCollection.Draw(pcxrect, *SidebarSurface, *pcx_surface);
        } else if (shapefile != nullptr) {
            Draw_Shape(*SidebarSurface, *CameoDrawer, shapefile, 0, position, cliprect, SHAPE_WIN_REL);
        }

        /*
        **	Darken this object because it cannot be produced or is otherwise
        **	unavailable.
        */
        if (Is_Darkened()) {
            Draw_Shape(*SidebarSurface, *SidebarDrawer, DarkenShapes, 0, position, cliprect, SHAPE_WIN_REL | SHAPE_DARKEN);
        }
    }

    char const* name = Cameo_Text();
    if (name != nullptr) {
        Print_Cameo_Text(name, position + OptionsExtension->SidebarControls.CameoNameOffset, cliprect, OptionsExtension->SidebarControls.CameoWidth - 2);
    }

    bool hasqueuecount = false;
    if (ttype != nullptr) {
        FactoryClass* factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(ttype->RTTI, TechnoTypeClassExtension::Get_Production_Flags(ttype));

        if (factory != nullptr) {
            int total = factory->Total(ttype);
            if (total > 1 || total > 0 && !factory->Is_Currently_Producing(ttype)) {
                Fancy_Text_Print("%d", *SidebarSurface, cliprect, position + OptionsExtension->SidebarControls.CameoQueueCountOffset, Fetch_Scheme_By_Name("LightGrey"), TBLACK, TPF_RIGHT | TPF_FULLSHADOW | TPF_8POINT, total);
                hasqueuecount = true;
            }
        }
    }

    /*
    **	Draw the overlapping clock shape if this is object is being constructed.
    **	If the object is completed, then display "Ready" with no clock shape.
    */
    if (Is_Clock()) {

        /*
        **	Display text showing that the object is ready to place.
        */
        char const* state = State_Text();
        if (state != nullptr) {
            Fancy_Text_Print(state, *SidebarSurface, cliprect, position + OptionsExtension->SidebarControls.CameoStateOffset, Fetch_Scheme_By_Name(OptionsExtension->SidebarControls.StateColor.c_str()), TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_8POINT);
        }

        if (!Is_Completed()) {

            int stage = Clock_Stage();

            if (!Is_Ready()) {
                Draw_Shape(*SidebarSurface, *SidebarDrawer, ClockShapes, stage + 1, position, cliprect, SHAPE_WIN_REL | SHAPE_TRANS50);
            } else {
                Draw_Shape(*SidebarSurface, *SidebarDrawer, RechargeClockShapes, stage + 1, position, cliprect, SHAPE_WIN_REL | SHAPE_TRANS50);
            }

            /*
            **	Display text showing that the construction is temporarily on hold.
            */
            if (Factory && !Factory->Is_Building()) {
                if (!hasqueuecount) {
                    Fancy_Text_Print(TXT_HOLD, *SidebarSurface, cliprect, position + OptionsExtension->SidebarControls.CameoStateOffset, Fetch_Scheme_By_Name(OptionsExtension->SidebarControls.OnHoldColor.c_str()), TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_8POINT);
                } else {
                    Fancy_Text_Print(TXT_HOLD, *SidebarSurface, cliprect, position + OptionsExtension->SidebarControls.CameoQueueStateOffset, Fetch_Scheme_By_Name(OptionsExtension->SidebarControls.OnHoldColor.c_str()), TBLACK, TPF_FULLSHADOW | TPF_8POINT);
                }
            }
        }
    }

    /**
     *  Draw a selection box around the cameo if we're currently hovering over it
     *  and it is available.
     */
    if (highlight && !Is_Darkened()) {
        SidebarSurface->Draw_Rect(Rect(position + Point2D(0, SidebarRect.Y) - Point2D(1, 1), OptionsExtension->SidebarControls.CameoWidth + 2, OptionsExtension->SidebarControls.CameoHeight + 2), DSurface::RGB_To_Pixel(ColorSchemes[Extension::Fetch(Sides[PlayerPtr->Class->Side])->UIColor]->HSV.operator RGBClass()));
    }
}


bool NewSidebarClass::StripClass::BuildType::Is_Darkened() const
{
    if (BuildableType != RTTI_SPECIAL) {
        const TechnoTypeClass* ttype = Fetch_Techno_Type(BuildableType, BuildableID);
        if (Factory == nullptr && ttype != nullptr) {
            if (ttype->RTTI == RTTI_BUILDINGTYPE && Extension::Fetch(PlayerPtr)->Fetch_Factory(BuildableType, TechnoTypeClassExtension::Get_Production_Flags(ttype)) != nullptr) {
                return true;
            }
            if (ttype->Who_Can_Build_Me(true, true, true, PlayerPtr) == nullptr) {
                return true;
            }
            if (PlayerPtr->Can_Build(Fetch_Techno_Type(BuildableType, BuildableID), false, false) == -1) {
                return true;
            }
        }
    }
    return false;
}

bool NewSidebarClass::StripClass::BuildType::Is_Clock() const
{
    if (BuildableType != RTTI_SPECIAL) {
        if (Factory == nullptr) {
            return false;
        }
    }
    return true;
}

int NewSidebarClass::StripClass::BuildType::Clock_Stage() const
{
    if (BuildableType != RTTI_SPECIAL) {
        if (Factory != nullptr) {
            return Factory->Completion();
        }
    } else {
        if (BuildableID > SUPER_NONE && BuildableID < SuperWeaponTypes.Count()) {
            return PlayerPtr->SuperWeapon[BuildableID]->Anim_Stage();
        }
    }
    return 0;
}


bool NewSidebarClass::StripClass::BuildType::Is_Completed() const
{
    if (BuildableType != RTTI_SPECIAL) {
        if (Factory != nullptr) {
            return Factory->Has_Completed();
        }
    } else {
        if (BuildableID > SUPER_NONE && BuildableID < SuperWeaponTypes.Count()) {
            return PlayerPtr->SuperWeapon[BuildableID]->Is_Charging() == false;
        }
    }
    return false;
}

bool NewSidebarClass::StripClass::BuildType::Is_Ready() const
{
    if (BuildableType == RTTI_SPECIAL) {
        if (BuildableID > SUPER_NONE && BuildableID < SuperWeaponTypes.Count()) {
            return PlayerPtr->SuperWeapon[BuildableID]->Can_Place();
        }
    }
    return false;
}


char const* NewSidebarClass::StripClass::BuildType::Cameo_Text() const
{
    if (BuildableType != RTTI_SPECIAL) {
        const TechnoTypeClass* ttype = Fetch_Techno_Type(BuildableType, BuildableID);
        if (ttype != nullptr) {
            return ttype->Full_Name();
        }
    } else {
        if (BuildableID > SUPER_NONE && BuildableID < SuperWeaponTypes.Count()) {
            return SuperWeaponTypes[BuildableID]->Full_Name();
        }
    }
    return nullptr;
}

char const* NewSidebarClass::StripClass::BuildType::Help_Text() const
{
    static char _buffer[512];

    if (BuildableType == RTTI_SPECIAL) {
        const SuperWeaponTypeClass* swtype = SuperWeaponTypes[BuildableID];
        const SuperWeaponTypeClassExtension* swtypeext = Extension::Fetch(swtype);

        if (strlen(swtypeext->Description) == 0) {
            // If there is no extended description, then simply show the name and price.
            return swtype->Full_Name();
        }

        // If there is an extended description, then show the name and the description.
        std::snprintf(_buffer, sizeof(_buffer), "%s@@%s", swtype->Full_Name(), swtypeext->Description);
    } else {

        const TechnoTypeClass* choice = Fetch_Techno_Type(BuildableType, BuildableID);

        // Bugfix from YR.
        if (!choice) return nullptr;

        /**
         *  Adds support for extended sidebar tooltips.
         *
         *  @author: Rampastring
         */
        const TechnoTypeClassExtension* technotypeext = Extension::Fetch(choice);
        const char* description = technotypeext->Description;

        if (strlen(description) > 0) {
            // If there is an extended description, then show the name, price, and the description.
            std::snprintf(_buffer, sizeof(_buffer), "%s@$%d@@%s", choice->Full_Name(), choice->Cost_Of(PlayerPtr), technotypeext->Description);
        } else {
            // If there is no extended description, then simply show the name and price.
            std::snprintf(_buffer, sizeof(_buffer), "%s@$%d", choice->Full_Name(), choice->Cost_Of(PlayerPtr));
        }
    }

    return _buffer;
}

char const* NewSidebarClass::StripClass::BuildType::State_Text() const
{
    if (BuildableType != RTTI_SPECIAL) {
        if (Is_Completed()) {
            return Fetch_String(TXT_READY);
        }
    } else {
        if (BuildableID > SUPER_NONE && BuildableID < SuperWeaponTypes.Count()) {
            return PlayerPtr->SuperWeapon[BuildableID]->State_String();
        }
    }
    return nullptr;
}

