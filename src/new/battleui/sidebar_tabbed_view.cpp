/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_TABBED_VIEW.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Tabbed sidebar view implementation.
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

#include "sidebar_tabbed_view.h"

#include "cameo_button.h"
#include "power_model.h"
#include "sidebar_model.h"
#include "sidebar_render_utils.h"

#include "colorscheme.h"
#include "convert.h"
#include "drawshape.h"
#include "extension.h"
#include "factory.h"
#include "fetchres.h"
#include "house.h"
#include "housetype.h"
#include "houseext.h"
#include "language.h"
#include "mouse.h"
#include "options.h"
#include "palette.h"
#include "power.h"
#include "scenario.h"
#include "sidebar.h"
#include "sideext.h"
#include "super.h"
#include "supertype.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "uicontrol.h"
#include "vinifera_defines.h"
#include "wwmouse.h"


/***************************************************************************
**  TabButtonClass
***************************************************************************/


/**
 *  Default constructor.
 *
 *  @author: ZivDero
 */
TabButtonClass::TabButtonClass() :
    ControlClass(0, 0, 0, 0, 0, LEFTPRESS | LEFTRELEASE, true),
    DrawX(0),
    DrawY(0),
    ShapeDrawer(SidebarDrawer),
    ShapeData(nullptr),
    IsFlashing(false),
    FlashTimer(0),
    FlashFrame(0),
    IsSelected(false),
    IsDrawn(false),
    MousedOver(false)
{
}


/**
 *  Parameterized constructor.
 *
 *  @author: ZivDero
 */
TabButtonClass::TabButtonClass(unsigned id, const ShapeSet* shapes, int x, int y,
                               ConvertClass* drawer, int w, int h) :
    ControlClass(id, x, y, w, h, LEFTPRESS | LEFTRELEASE, true),
    DrawX(0),
    DrawY(0),
    ShapeDrawer(drawer),
    ShapeData(shapes),
    IsFlashing(false),
    FlashTimer(0),
    FlashFrame(0),
    IsSelected(false),
    IsDrawn(false),
    MousedOver(false)
{
}


/**
 *  Handles mouse input for the tab button.
 *
 *  @author: ZivDero
 */
bool TabButtonClass::Action(unsigned flags, KeyNumType& key)
{
    if (!flags) {
        Flag_To_Redraw();
    }

    Sticky_Process(flags);

    if (flags & LEFTPRESS) {
        flags &= ~LEFTPRESS;
        ControlClass::Action(flags, key);
        key = KN_NONE;
        return true;
    }

    if (flags & LEFTRELEASE) {
        bool overbutton = (Get_Mouse_X() - X) < Width && (Get_Mouse_Y() - Y) < Height;
        if (!IsSelected && overbutton) {
            IsSelected = true;
            Flag_To_Redraw();
        } else {
            flags &= ~LEFTRELEASE;
        }
    }

    return ControlClass::Action(flags, key);
}


/**
 *  Disables the tab button.
 *
 *  @author: ZivDero
 */
void TabButtonClass::Disable()
{
    IsSelected = false;
    Stop_Flashing();
    ControlClass::Disable();
}


/**
 *  Enables the tab button.
 *
 *  @author: ZivDero
 */
void TabButtonClass::Enable()
{
    IsSelected = false;
    Stop_Flashing();
    ControlClass::Enable();
}


/**
 *  Draws the tab button.
 *
 *  @author: ZivDero
 */
bool TabButtonClass::Draw_Me(bool forced)
{
    if (!ControlClass::Draw_Me(forced)) {
        return false;
    }

    if (!ShapeData || !ShapeDrawer) {
        return false;
    }

    int shapenum;

    if (IsDisabled) {
        shapenum = FRAME_DISABLED;
    } else if (IsSelected) {
        shapenum = FRAME_SELECTED;
    } else if (IsFlashing) {
        if (FlashTimer.Expired()) {
            if (FlashFrame == FLASH_FRAME_MAX) {
                FlashFrame = FLASH_FRAME_MIN;
            } else {
                FlashFrame++;
            }
            FlashTimer = FLASH_RATE;
        }
        shapenum = FlashFrame;
    } else {
        shapenum = FRAME_NORMAL;
    }

    Draw_Shape(*SidebarSurface, *ShapeDrawer, ShapeData, shapenum,
               Point2D(X + DrawX, Y + DrawY), VisibleRect, SHAPE_NORMAL);

    if (MousedOver && !Scen->InputLock && !IsDisabled && !IsSelected) {
        Rect hover_rect(X + DrawX, Y + DrawY, Width - 1, Height - 1);
        const ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->UIColor;
        SidebarSurface->Draw_Rect(hover_rect,
                                  DSurface::Build_Hicolor_Pixel(ColorSchemes[colorschemetype]->HSV.operator RGBClass()));
    }

    IsDrawn = true;
    return true;
}


/**
 *  Called when the mouse enters the tab button.
 *
 *  @author: ZivDero
 */
void TabButtonClass::On_Mouse_Enter()
{
    MousedOver = true;
    Map.IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Called when the mouse leaves the tab button.
 *
 *  @author: ZivDero
 */
void TabButtonClass::On_Mouse_Leave()
{
    MousedOver = false;
    Map.IsToFullRedraw = true;
    Map.Flag_To_Redraw();
    RedrawSidebar = true;
}


/**
 *  Sets the shape data for this button and adjusts its dimensions.
 *
 *  @author: ZivDero
 */
void TabButtonClass::Set_Shape(const ShapeSet* data, int width, int height)
{
    ShapeData = data;
    if (ShapeData) {
        Width = ShapeData->Get_Width();
        Height = ShapeData->Get_Height();
    }

    if (width != 0) {
        Width = width;
    }
    if (height != 0) {
        Height = height;
    }
}


/**
 *  Starts the flash animation on this tab.
 *
 *  @author: ZivDero
 */
void TabButtonClass::Start_Flashing()
{
    IsFlashing = true;
    FlashTimer.Start();
    FlashTimer = FLASH_RATE;
    FlashFrame = FLASH_FRAME_START;
}


/**
 *  Stops the flash animation on this tab.
 *
 *  @author: ZivDero
 */
void TabButtonClass::Stop_Flashing()
{
    IsFlashing = false;
    FlashTimer.Stop();
    FlashFrame = FLASH_FRAME_START;
}


/**
 *  Selects this tab button.
 *
 *  @author: ZivDero
 */
void TabButtonClass::Select()
{
    IsSelected = true;
}


/**
 *  Deselects this tab button.
 *
 *  @author: ZivDero
 */
void TabButtonClass::Deselect()
{
    IsSelected = false;
}


/***************************************************************************
**  TabbedSidebarView
***************************************************************************/


/**
 *  TabbedSidebarView constructor.
 *
 *  @author: ZivDero
 */
TabbedSidebarView::TabbedSidebarView(SidebarModel* model) :
    ISidebarView(model),
    TabIndex(SIDEBAR_TAB_STRUCTURE),
    Strip(),
    TabButtons()
{
}


/**
 *  One-time initialization. Loads shared shapes.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::One_Time()
{
    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        Strip[i].One_Time(i);
    }

    SidebarClass::StripClass::RechargeClockShape = MFCD::RetrieveT<ShapeSet>("RCLOCK2.SHP");
    SidebarClass::StripClass::ClockShape = MFCD::RetrieveT<ShapeSet>("GCLOCK2.SHP");
}


/**
 *  Clears state for a new scenario.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Init_Clear()
{
    TabIndex = SIDEBAR_TAB_STRUCTURE;

    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        Strip[i].Init_Clear();
        TabButtons[i].Stop_Flashing();
        TabButtons[i].IsSelected = false;
        TabButtons[i].IsDisabled = true;
    }
}


/**
 *  Initializes IO gadgets — action buttons, tab buttons, and strips.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Init_IO()
{
    if (Debug_Map) {
        return;
    }

    /**
     *  Set up action buttons.
     */
    auto& Repair = SidebarClass::Repair;
    auto& Sell = SidebarClass::Sell;
    auto& PowerBtn = SidebarClass::Power;
    auto& Waypoint = SidebarClass::Waypoint;

    Repair.X = TacticalRect.Width + TacticalRect.X;
    Sell.X = TacticalRect.Width + TacticalRect.X + 27;
    PowerBtn.X = TacticalRect.Width + TacticalRect.X + 54;
    Waypoint.X = TacticalRect.Width + TacticalRect.X + 81;

    Repair.IsSticky = true;
    Repair.ID = SidebarClass::BUTTON_REPAIR;
    Repair.Y = 148;
    Repair.DrawX = -480;
    Repair.DrawY = 3;
    Repair.DrawnOnSidebarSurface = true;
    Repair.ShapeDrawer = SidebarDrawer;
    Repair.IsPressed = false;
    Repair.IsToggleType = true;
    Repair.ReflectButtonState = true;

    Sell.IsSticky = true;
    Sell.ID = SidebarClass::BUTTON_SELL;
    Sell.Y = 148;
    Sell.DrawX = -480;
    Sell.DrawY = 3;
    Sell.DrawnOnSidebarSurface = true;
    Sell.ShapeDrawer = SidebarDrawer;
    Sell.IsPressed = false;
    Sell.IsToggleType = true;
    Sell.ReflectButtonState = true;

    PowerBtn.IsSticky = true;
    PowerBtn.ID = SidebarClass::BUTTON_POWER;
    PowerBtn.Y = 148;
    PowerBtn.DrawX = -480;
    PowerBtn.DrawY = 3;
    PowerBtn.DrawnOnSidebarSurface = true;
    PowerBtn.ShapeDrawer = SidebarDrawer;
    PowerBtn.IsPressed = false;
    PowerBtn.IsToggleType = true;
    PowerBtn.ReflectButtonState = true;

    Waypoint.IsSticky = true;
    Waypoint.ID = SidebarClass::BUTTON_WAYPOINT;
    Waypoint.Y = 148;
    Waypoint.DrawX = -480;
    Waypoint.DrawY = 3;
    Waypoint.DrawnOnSidebarSurface = true;
    Waypoint.ShapeDrawer = SidebarDrawer;
    Waypoint.IsPressed = false;
    Waypoint.IsToggleType = true;
    Waypoint.ReflectButtonState = true;
    Waypoint.Enable();

    /**
     *  Set up tab buttons.
     */
    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        TabButtons[i].IsSticky = true;
        TabButtons[i].ID = BUTTON_TAB_1 + i;
        TabButtons[i].Y = 148;
        TabButtons[i].DrawX = -480;
        TabButtons[i].DrawY = 3;
        TabButtons[i].IsSelected = false;
        TabButtons[i].IsDisabled = true;
    }

    /**
     *  Initialize the strips as 2-column mode (default).
     */
    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        Strip[i].Init_IO(i, 2);
    }

    /**
     *  Link strip views to model categories.
     */
    if (Model->Category_Count() >= SIDEBAR_TAB_COUNT) {
        for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
            Strip[i].Set_Category(&Model->Get_Category(i));
        }
    }

    Set_Dimensions();
}


/**
 *  Loads house-specific shapes for the sidebar and tab buttons.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Init_For_House()
{
    PaletteClass pal("SIDEBAR.PAL");

    delete SidebarDrawer;
    SidebarDrawer = new ConvertClass(&pal, &pal, VisibleSurface, 1);

    SidebarClass::Sell.Set_Shape(MFCD::RetrieveT<ShapeSet>("SELL.SHP"));
    SidebarClass::Sell.ShapeDrawer = SidebarDrawer;

    SidebarClass::Power.Set_Shape(MFCD::RetrieveT<ShapeSet>("POWER.SHP"));
    SidebarClass::Power.ShapeDrawer = SidebarDrawer;

    SidebarClass::Waypoint.Set_Shape(MFCD::RetrieveT<ShapeSet>("WAYP.SHP"));
    SidebarClass::Waypoint.ShapeDrawer = SidebarDrawer;

    SidebarClass::Repair.Set_Shape(MFCD::RetrieveT<ShapeSet>("REPAIR.SHP"));
    SidebarClass::Repair.ShapeDrawer = SidebarDrawer;

    SidebarClass::SidebarShape = MFCD::RetrieveT<ShapeSet>("SIDE1.SHP");
    SidebarClass::SidebarMiddleShape = MFCD::RetrieveT<ShapeSet>("SIDE2.SHP");
    SidebarClass::SidebarBottomShape = MFCD::RetrieveT<ShapeSet>("SIDE3.SHP");
    SidebarClass::SidebarAddonShape = MFCD::RetrieveT<ShapeSet>("ADDON.SHP");

    /**
     *  Load tab button shapes.
     */
    TabButtons[SIDEBAR_TAB_STRUCTURE].Set_Shape(MFCD::RetrieveT<ShapeSet>("TAB-BLD.SHP"));
    TabButtons[SIDEBAR_TAB_STRUCTURE].ShapeDrawer = SidebarDrawer;

    TabButtons[SIDEBAR_TAB_INFANTRY].Set_Shape(MFCD::RetrieveT<ShapeSet>("TAB-INF.SHP"));
    TabButtons[SIDEBAR_TAB_INFANTRY].ShapeDrawer = SidebarDrawer;

    TabButtons[SIDEBAR_TAB_UNIT].Set_Shape(MFCD::RetrieveT<ShapeSet>("TAB-UNT.SHP"));
    TabButtons[SIDEBAR_TAB_UNIT].ShapeDrawer = SidebarDrawer;

    TabButtons[SIDEBAR_TAB_SPECIAL].Set_Shape(MFCD::RetrieveT<ShapeSet>("TAB-SPC.SHP"));
    TabButtons[SIDEBAR_TAB_SPECIAL].ShapeDrawer = SidebarDrawer;

    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        Strip[i].Init_For_House(i);
    }
}


/**
 *  Recalculates positions of all buttons, tabs, and the active strip.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Set_Dimensions()
{
    auto& Repair = SidebarClass::Repair;
    auto& Sell = SidebarClass::Sell;
    auto& PowerBtn = SidebarClass::Power;
    auto& Waypoint = SidebarClass::Waypoint;
    auto& Background = SidebarClass::Background;

    Background.Set_Position(SidebarRect.X + 16, TacticalRect.Y);
    Background.Flag_To_Redraw();

    Repair.Set_Position(SidebarRect.X + SidebarClass::BUTTON_REPAIR_X_OFFSET, SidebarRect.Y + SidebarClass::BUTTON_REPAIR_Y_OFFSET);
    Repair.Flag_To_Redraw();
    Repair.DrawX = -SidebarRect.X;

    Sell.Set_Position(Repair.X + SidebarClass::BUTTON_SELL_X_OFFSET, Repair.Y);
    Sell.Flag_To_Redraw();
    Sell.DrawX = -SidebarRect.X;

    PowerBtn.Set_Position(Sell.X + SidebarClass::BUTTON_POWER_X_OFFSET, Sell.Y);
    PowerBtn.Flag_To_Redraw();
    PowerBtn.DrawX = -SidebarRect.X;

    Waypoint.Set_Position(PowerBtn.X + SidebarClass::BUTTON_WAYPOINT_X_OFFSET, PowerBtn.Y);
    Waypoint.Flag_To_Redraw();
    Waypoint.DrawX = -SidebarRect.X;

    /**
     *  Position the tab buttons.
     */
    TabButtons[0].Set_Position(SidebarRect.X + TAB_ONE_X_OFFSET, SidebarRect.Y + TAB_Y_OFFSET);
    TabButtons[0].Flag_To_Redraw();
    TabButtons[0].DrawX = -SidebarRect.X;

    TabButtons[1].Set_Position(SidebarRect.X + TAB_TWO_X_OFFSET, TabButtons[0].Y);
    TabButtons[1].Flag_To_Redraw();
    TabButtons[1].DrawX = -SidebarRect.X;

    TabButtons[2].Set_Position(SidebarRect.X + TAB_THREE_X_OFFSET, TabButtons[1].Y);
    TabButtons[2].Flag_To_Redraw();
    TabButtons[2].DrawX = -SidebarRect.X;

    TabButtons[3].Set_Position(SidebarRect.X + TAB_FOUR_X_OFFSET, TabButtons[2].Y);
    TabButtons[3].Flag_To_Redraw();
    TabButtons[3].DrawX = -SidebarRect.X;

    /**
     *  Position the active strip. All strips share the same position.
     */
    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        Strip[i].Set_Dimensions(COLUMN_X, COLUMN_Y);
    }

    /**
     *  Position the scroll buttons (only one pair for the active strip).
     */
    int max_vis = Strip[0].Max_Visible() / 2;
    auto& up0 = SidebarClass::StripClass::UpButton[0];
    auto& down0 = SidebarClass::StripClass::DownButton[0];

    up0.Set_Position(SidebarRect.X + UP_X_OFFSET,
                     SidebarRect.Y + SidebarClass::StripClass::OBJECT_HEIGHT * max_vis + UP_Y_OFFSET);
    up0.Flag_To_Redraw();
    up0.DrawX = -SidebarRect.X;

    down0.Set_Position(SidebarRect.X + DOWN_X_OFFSET,
                       SidebarRect.Y + SidebarClass::StripClass::OBJECT_HEIGHT * max_vis + DOWN_Y_OFFSET);
    down0.Flag_To_Redraw();
    down0.DrawX = -SidebarRect.X;

    /**
     *  Set up tooltips.
     */
    if (ToolTips) {
        ToolTip tooltip;

        for (int i = 0; i < 200; i++) {
            ToolTips->Remove(1000 + i);
        }

        for (int i = 0; i < Strip[TabIndex].MaxVisibleCount; i++) {
            CameoButtonClass* btn = Strip[TabIndex].SelectButtons[i];
            tooltip.Region = Rect(btn->X, btn->Y, btn->Width, btn->Height);
            tooltip.ID = 1000 + i;
            tooltip.Text = TXT_NONE;
            ToolTips->Add(&tooltip);
        }

        for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
            tooltip.Region = Rect(TabButtons[i].X, TabButtons[i].Y, TabButtons[i].Width, TabButtons[i].Height);
            tooltip.ID = BUTTON_TAB_1 + i;
            tooltip.Text = TXT_NONE;
            ToolTips->Remove(tooltip.ID);
            ToolTips->Add(&tooltip);
        }

        tooltip.Region = Rect(Repair.X, Repair.Y, Repair.Width, Repair.Height);
        tooltip.ID = SidebarClass::BUTTON_REPAIR;
        tooltip.Text = TXT_REPAIR_MODE;
        ToolTips->Remove(tooltip.ID);
        ToolTips->Add(&tooltip);

        tooltip.Region = Rect(Sell.X, Sell.Y, Sell.Width, Sell.Height);
        tooltip.ID = SidebarClass::BUTTON_SELL;
        tooltip.Text = TXT_SELL_MODE;
        ToolTips->Remove(tooltip.ID);
        ToolTips->Add(&tooltip);

        tooltip.Region = Rect(PowerBtn.X, PowerBtn.Y, PowerBtn.Width, PowerBtn.Height);
        tooltip.ID = SidebarClass::BUTTON_POWER;
        tooltip.Text = TXT_POWER_MODE;
        ToolTips->Remove(tooltip.ID);
        ToolTips->Add(&tooltip);

        tooltip.Region = Rect(Waypoint.X, Waypoint.Y, Waypoint.Width, Waypoint.Height);
        tooltip.ID = SidebarClass::BUTTON_WAYPOINT;
        tooltip.Text = TXT_WAYPOINTMODE;
        ToolTips->Remove(tooltip.ID);
        ToolTips->Add(&tooltip);
    }
}


/**
 *  Per-frame logic. Processes tab input, strip AI, and tab flash.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::AI(KeyNumType& input, Point2D& xy)
{
    /**
     *  Handle tab button input.
     */
    if (input == (BUTTON_TAB_1 | KN_BUTTON)) {
        Change_Tab(SIDEBAR_TAB_STRUCTURE);
    }
    if (input == (BUTTON_TAB_2 | KN_BUTTON)) {
        Change_Tab(SIDEBAR_TAB_INFANTRY);
    }
    if (input == (BUTTON_TAB_3 | KN_BUTTON)) {
        Change_Tab(SIDEBAR_TAB_UNIT);
    }
    if (input == (BUTTON_TAB_4 | KN_BUTTON)) {
        Change_Tab(SIDEBAR_TAB_SPECIAL);
    }

    /**
     *  Make sure the current tab's button stays selected.
     */
    if (!TabButtons[TabIndex].IsSelected) {
        TabButtons[TabIndex].Select();
    }

    /**
     *  If the current tab has no items, try to switch to one that does.
     */
    if (Strip[TabIndex].Get_Category() && Strip[TabIndex].Get_Category()->Items.Count() < 1) {
        SidebarTabType newtab = First_Active_Tab();
        if (newtab != SIDEBAR_TAB_NONE) {
            Change_Tab(newtab);
        }
    }

    /**
     *  Run AI on all strips (for production tracking) but only the
     *  active strip processes input.
     */
    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        Tab_Button_AI(i);

        if (i == TabIndex) {
            Strip[i].AI(input, xy);
        }
    }
}


/**
 *  Updates a single tab button's enabled/disabled state and flash.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Tab_Button_AI(int tab_index)
{
    BuildCategory* cat = Strip[tab_index].Get_Category();
    if (!cat) {
        return;
    }

    if (cat->Items.Count() > 0) {
        if (!TabButtons[tab_index].Is_Enabled()) {
            TabButtons[tab_index].Enable();
        }

        /**
         *  Building tab: flash when a defense building completes.
         */
        if (tab_index == SIDEBAR_TAB_STRUCTURE) {
            if (TabButtons[tab_index].IsFlashing) {
                FactoryClass* fptr = Extension::Fetch(PlayerPtr)->Fetch_Factory(RTTI_BUILDINGTYPE, PRODFLAG_DEFENSE);
                if (fptr == nullptr || !fptr->Has_Completed()) {
                    TabButtons[tab_index].Stop_Flashing();
                }
            }
        }

        /**
         *  Special tab: flash when a super weapon is ready.
         */
        if (tab_index == SIDEBAR_TAB_SPECIAL) {
            bool ready_sw = false;
            for (int i = 0; i < PlayerPtr->SuperWeapon.Count(); i++) {
                SuperClass* sw = PlayerPtr->SuperWeapon[i];
                if (sw->Can_Place() && !sw->Class->IsUseChargeDrain) {
                    ready_sw = true;
                    break;
                }
            }

            if (ready_sw && !TabButtons[tab_index].IsFlashing) {
                TabButtons[tab_index].Start_Flashing();
            } else if (!ready_sw && TabButtons[tab_index].IsFlashing) {
                TabButtons[tab_index].Stop_Flashing();
            }
        }
    } else {
        if (TabButtons[tab_index].Is_Enabled()) {
            TabButtons[tab_index].Disable();
        }
    }
}


/**
 *  Draws the tabbed sidebar — background, tabs, action buttons, and active strip.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Draw(bool complete)
{
    Surface* oldsurface = LogicalSurface;
    LogicalSurface = SidebarSurface;

    Rect rect(0, 0, SidebarSurface->Get_Width(), SidebarSurface->Get_Height());

    if (Map.IsSidebarActive && (Map.IsToRedraw || complete) && !Debug_Map) {
        if (complete || Strip[TabIndex].IsToRedraw) {
            /**
             *  Draw the sidebar background shapes.
             */
            int y = SidebarRect.Y;
            Point2D xy(0, y);
            Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarClass::SidebarShape, 0, xy, rect, SHAPE_WIN_REL);
            y += SidebarClass::SidebarShape->Get_Height();

            int rows = Max_Visible();
            for (int i = 0; i < rows; i++, y += SidebarClass::SidebarMiddleShape->Get_Height()) {
                xy = Point2D(0, y);
                Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarClass::SidebarMiddleShape, 0, xy, rect, SHAPE_WIN_REL);
            }

            xy = Point2D(0, y);
            Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarClass::SidebarBottomShape, 0, xy, rect, SHAPE_WIN_REL);

            xy = Point2D(0, y + SidebarClass::SidebarBottomShape->Get_Height());
            Draw_Shape(*SidebarSurface, *SidebarDrawer, SidebarClass::SidebarAddonShape, 0, xy, rect, SHAPE_WIN_REL);

            Strip[TabIndex].IsToRedraw = true;
        }

        /**
         *  Draw action buttons.
         */
        SidebarClass::Repair.Draw_Me(true);
        SidebarClass::Sell.Draw_Me(true);
        SidebarClass::Power.Draw_Me(true);
        SidebarClass::Waypoint.Draw_Me(true);

        RedrawSidebar = true;
    }

    /**
     *  Tab buttons always redraw (they might be flashing).
     */
    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        TabButtons[i].Draw_Me(true);
    }

    /**
     *  Draw the active strip only.
     */
    if (Map.IsSidebarActive) {
        Strip[TabIndex].Draw(*SidebarSurface, rect, complete);
    }

    /**
     *  Check if action buttons or tabs drew themselves.
     */
    if (SidebarClass::Repair.IsDrawn) {
        RedrawSidebar = true;
        SidebarClass::Repair.IsDrawn = false;
    }
    if (SidebarClass::Sell.IsDrawn) {
        RedrawSidebar = true;
        SidebarClass::Sell.IsDrawn = false;
    }
    if (SidebarClass::Power.IsDrawn) {
        RedrawSidebar = true;
        SidebarClass::Power.IsDrawn = false;
    }
    if (SidebarClass::Waypoint.IsDrawn) {
        RedrawSidebar = true;
        SidebarClass::Waypoint.IsDrawn = false;
    }

    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        if (TabButtons[i].IsDrawn) {
            RedrawSidebar = true;
            TabButtons[i].IsDrawn = false;
        }
    }

    if (ToolTips) {
        ToolTips->Force_Redraw(true);
    }

    Map.IsToRedraw = false;
    Map.IsToFullRedraw = false;

    Blit(complete);

    LogicalSurface = oldsurface;
}


/**
 *  Blits the sidebar surface to the visible screen.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Blit(bool complete)
{
    Map.Blit_Sidebar(complete);
}


/**
 *  Activates or deactivates the sidebar.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Activate(int control)
{
    if (control) {
        Strip[TabIndex].Activate();

        for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
            TabButtons[i].Zap();
            Map.Add_A_Button(TabButtons[i]);
        }
    } else {
        for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
            Strip[i].Deactivate();
            Map.Remove_A_Button(TabButtons[i]);
        }
    }
}


/**
 *  Scrolls the active tab's strip.
 *
 *  @author: ZivDero
 */
bool TabbedSidebarView::Scroll(bool up, int column)
{
    return Strip[TabIndex].Scroll(up);
}


/**
 *  Page-scrolls the active tab's strip.
 *
 *  @author: ZivDero
 */
bool TabbedSidebarView::Scroll_Page(bool up, int column)
{
    return Strip[TabIndex].Scroll_Page(up);
}


/**
 *  Flags the active tab strip for redraw.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Flag_Current_Strip_To_Redraw()
{
    Strip[TabIndex].Flag_To_Redraw();
}


/**
 *  Flags the routed tab strip for redraw.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Flag_Strip_To_Redraw(RTTIType type, ProductionFlags flags)
{
    SidebarTabType tab = SIDEBAR_TAB_SPECIAL;

    switch (type) {
    case RTTI_BUILDINGTYPE:
    case RTTI_BUILDING:
        tab = SIDEBAR_TAB_STRUCTURE;
        break;

    case RTTI_INFANTRYTYPE:
    case RTTI_INFANTRY:
        tab = SIDEBAR_TAB_INFANTRY;
        break;

    case RTTI_UNITTYPE:
    case RTTI_UNIT:
        tab = (flags & PRODFLAG_NAVAL) ? SIDEBAR_TAB_SPECIAL : SIDEBAR_TAB_UNIT;
        break;

    case RTTI_AIRCRAFTTYPE:
    case RTTI_AIRCRAFT:
    case RTTI_SUPERWEAPONTYPE:
    case RTTI_SUPERWEAPON:
    case RTTI_SPECIAL:
    default:
        tab = SIDEBAR_TAB_SPECIAL;
        break;
    }

    Strip[tab].Flag_To_Redraw();
}


/**
 *  Returns tooltip text for a cameo slot in the active tab.
 *
 *  @author: ZivDero
 */
const char* TabbedSidebarView::Help_Text(int gadget_id)
{
    int slot = gadget_id - 1000;
    if (slot < 0) {
        return nullptr;
    }

    SidebarStripView& strip = Strip[TabIndex];
    if (slot >= strip.MaxVisibleCount) {
        return nullptr;
    }

    BuildCategory* category = strip.Get_Category();
    if (category == nullptr) {
        return nullptr;
    }

    int item_index = strip.TopIndex + slot;
    if (item_index < 0 || item_index >= category->Items.Count()) {
        return nullptr;
    }

    return Format_Cameo_Tooltip(category->Items[item_index]);
}


/**
 *  Returns the number of visible rows.
 *
 *  @author: ZivDero
 */
int TabbedSidebarView::Max_Visible() const
{
    if (SidebarSurface != nullptr && SidebarClass::SidebarShape != nullptr) {
        return (SidebarRect.Height
                - SidebarClass::SidebarBottomShape->Get_Height()
                - SidebarClass::SidebarShape->Get_Height())
               / SidebarClass::SidebarMiddleShape->Get_Height();
    }
    return SidebarClass::StripClass::MAX_VISIBLE;
}


/**
 *  Switches to a different tab.
 *
 *  @author: ZivDero
 */
bool TabbedSidebarView::Change_Tab(SidebarTabType index)
{
    if (TabIndex == index) {
        return false;
    }

    BuildCategory* cat = Strip[index].Get_Category();
    if (!cat || cat->Items.Count() < 1) {
        return false;
    }

    Strip[TabIndex].Deactivate();
    TabButtons[TabIndex].Deselect();

    TabIndex = index;

    Strip[TabIndex].Activate();
    TabButtons[TabIndex].Select();

    Map.IsToFullRedraw = true;
    return true;
}


/**
 *  Returns the first tab that has buildable items, or SIDEBAR_TAB_NONE.
 *
 *  @author: ZivDero
 */
TabbedSidebarView::SidebarTabType TabbedSidebarView::First_Active_Tab() const
{
    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        BuildCategory* cat = Strip[i].Get_Category();
        if (cat && cat->Items.Count() > 0) {
            return static_cast<SidebarTabType>(i);
        }
    }

    return SIDEBAR_TAB_NONE;
}
