/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Tabbed sidebar view implementation. Four tabs (Structure /
 *          Infantry / Unit / Special) with a single 2-column strip per tab.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sidebar_tabbed_view.h"

#include "cameo_button.h"
#include "colorscheme.h"
#include "drawshape.h"
#include "extension.h"
#include "factory.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "language.h"
#include "mouse.h"
#include "power.h"
#include "scenario.h"
#include "sidebar.h"
#include "sidebar_model.h"
#include "sidebar_render_utils.h"
#include "sideext.h"
#include "super.h"
#include "supertype.h"
#include "tibsun_globals.h"
#include "tooltip.h"
#include "uicontrol.h"
#include "vinifera_defines.h"
#include "wwmouse.h"


namespace
{
/**
 *  Returns the active tabbed sidebar layout config.
 *
 *  @author: ZivDero
 */
const SidebarTabbedLayout& Get_Tabbed_Layout()
{
    return UIControls->TabbedSidebarLayoutConfig;
}


/**
 *  Builds the configured strip layout shared by all tabbed sidebar strips.
 *
 *  @author: ZivDero
 */
SidebarStripView::StripLayout Build_Tabbed_Strip_Layout()
{
    const SidebarTabbedLayout& layout = Get_Tabbed_Layout();
    SidebarStripView::StripLayout strip_layout;

    strip_layout.Position = layout.StripPosition;
    strip_layout.RowSpacing = layout.RowSpacing;
    strip_layout.ColumnSpacing = layout.ColumnSpacing;
    strip_layout.CameoSize = layout.CameoSize;
    strip_layout.CameoNameOffset = layout.CameoNameOffset;
    strip_layout.CameoTextOffset = layout.CameoTextOffset;
    strip_layout.QueueCountOffset = layout.QueueCountOffset;
    strip_layout.UpButtonPosition = layout.UpButtonPosition;
    strip_layout.DownButtonPosition = layout.DownButtonPosition;
    strip_layout.IsUpButtonVisible = layout.IsUpButtonVisible;
    strip_layout.IsDownButtonVisible = layout.IsDownButtonVisible;

    return strip_layout;
}
}


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
    IsMousedOver(false)
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
    IsMousedOver(false)
{
}


/**
 *  Handles mouse input for the tab button.
 *
 *  @author: ZivDero, Rampastring
 */
bool TabButtonClass::Action(unsigned flags, KeyNumType& key)
{
    if (Scen->InputLock) {
        return true;
    }

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

    if (IsMousedOver && !Scen->InputLock && !IsDisabled && !IsSelected) {
        Rect hover_rect(X + DrawX, Y + DrawY, Width - 1, Height - 1);
        const ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->HoverHighlightColor;
        SidebarSurface->Draw_Rect(hover_rect,
                                  DSurface::Build_Hicolor_Pixel(ColorSchemes[colorschemetype]->HSV.operator RGBClass()));
    }

    return true;
}


/**
 *  Called when the mouse enters the tab button.
 *
 *  @author: ZivDero
 */
void TabButtonClass::On_Mouse_Enter()
{
    IsMousedOver = true;
}


/**
 *  Called when the mouse leaves the tab button.
 *
 *  @author: ZivDero
 */
void TabButtonClass::On_Mouse_Leave()
{
    IsMousedOver = false;
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


/***************************************************************************
**  Lifecycle and layout
***************************************************************************/


/**
 *  TabbedSidebarView constructor.
 *
 *  @author: ZivDero
 */
TabbedSidebarView::TabbedSidebarView(SidebarModel* model) :
    ISidebarView(model),
    RegisteredTooltipCount(0),
    TabIndex(SIDEBAR_TAB_STRUCTURE),
    Strip(),
    TabButtons(),
    BackgroundTopShape(nullptr),
    BackgroundMiddleShape(nullptr),
    BackgroundBottomShape(nullptr),
    BackgroundAddonShape(nullptr),
    ClockShape(nullptr),
    RechargeClockShape(nullptr)
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
}


/**
 *  Clears state for a new scenario.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Init_Clear()
{
    TabIndex = SIDEBAR_TAB_STRUCTURE;
    RegisteredTooltipCount = 0;

    for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
        Strip[i].Init_Clear();
        TabButtons[i].Stop_Flashing();
        TabButtons[i].IsSelected = false;
        TabButtons[i].IsDisabled = true;
    }
}


/**
 *  Initializes IO gadgets — tab buttons and strip gadgets.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Init_IO()
{
    if (Debug_Map) {
        return;
    }

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

}


/**
 *  Loads house-specific shapes for the tabbed sidebar and its tabs.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Init_For_House()
{
    const SidebarTabbedLayout& layout = Get_Tabbed_Layout();

    BackgroundTopShape = MFCD::RetrieveT<ShapeSet>(layout.SidebarShape.c_str());
    BackgroundMiddleShape = MFCD::RetrieveT<ShapeSet>(layout.SidebarMiddleShape.c_str());
    BackgroundBottomShape = MFCD::RetrieveT<ShapeSet>(layout.SidebarBottomShape.c_str());
    BackgroundAddonShape = MFCD::RetrieveT<ShapeSet>(layout.SidebarAddonShape.c_str());
    ClockShape = MFCD::RetrieveT<ShapeSet>(layout.ClockShape.c_str());
    RechargeClockShape = MFCD::RetrieveT<ShapeSet>(layout.RechargeClockShape.c_str());

    SidebarStripView::StripArt strip_art;
    strip_art.ScrollUpButtonShape = MFCD::RetrieveT<ShapeSet>(layout.ScrollUpButtonShape.c_str());
    strip_art.ScrollDownButtonShape = MFCD::RetrieveT<ShapeSet>(layout.ScrollDownButtonShape.c_str());
    strip_art.DarkenShape = MFCD::RetrieveT<ShapeSet>(layout.DarkenShape.c_str());
    strip_art.ClockShape = ClockShape;
    strip_art.RechargeClockShape = RechargeClockShape;
    strip_art.BackgroundTopHeight = BackgroundTopShape != nullptr ? BackgroundTopShape->Get_Height() : 0;
    strip_art.BackgroundBottomHeight = BackgroundBottomShape != nullptr ? BackgroundBottomShape->Get_Height() : 0;

    /**
     *  Load tab button shapes.
     */
    TabButtons[SIDEBAR_TAB_STRUCTURE].Set_Shape(MFCD::RetrieveT<ShapeSet>(layout.StructureTabShape.c_str()));
    TabButtons[SIDEBAR_TAB_STRUCTURE].ShapeDrawer = SidebarDrawer;

    TabButtons[SIDEBAR_TAB_INFANTRY].Set_Shape(MFCD::RetrieveT<ShapeSet>(layout.InfantryTabShape.c_str()));
    TabButtons[SIDEBAR_TAB_INFANTRY].ShapeDrawer = SidebarDrawer;

    TabButtons[SIDEBAR_TAB_UNIT].Set_Shape(MFCD::RetrieveT<ShapeSet>(layout.UnitTabShape.c_str()));
    TabButtons[SIDEBAR_TAB_UNIT].ShapeDrawer = SidebarDrawer;

    TabButtons[SIDEBAR_TAB_SPECIAL].Set_Shape(MFCD::RetrieveT<ShapeSet>(layout.SpecialTabShape.c_str()));
    TabButtons[SIDEBAR_TAB_SPECIAL].ShapeDrawer = SidebarDrawer;

    for (auto& strip : Strip) {
        strip.Set_Art(strip_art);
        strip.Init_For_House();
    }
}


/**
 *  Reflows the tabbed sidebar layout.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Shift_Sidebar()
{
    Background.Set_Position(SidebarRect.X + 16, TacticalRect.Y);
    Background.Flag_To_Redraw();

    const SidebarTabbedLayout& layout = Get_Tabbed_Layout();

    /**
     *  Position the tab buttons.
     */
    TabButtons[0].Set_Position(SidebarRect.X + layout.TabButtonPosition[0].X, SidebarRect.Y + layout.TabButtonPosition[0].Y);
    TabButtons[0].Flag_To_Redraw();
    TabButtons[0].DrawX = -SidebarRect.X;

    TabButtons[1].Set_Position(SidebarRect.X + layout.TabButtonPosition[1].X, SidebarRect.Y + layout.TabButtonPosition[1].Y);
    TabButtons[1].Flag_To_Redraw();
    TabButtons[1].DrawX = -SidebarRect.X;

    TabButtons[2].Set_Position(SidebarRect.X + layout.TabButtonPosition[2].X, SidebarRect.Y + layout.TabButtonPosition[2].Y);
    TabButtons[2].Flag_To_Redraw();
    TabButtons[2].DrawX = -SidebarRect.X;

    TabButtons[3].Set_Position(SidebarRect.X + layout.TabButtonPosition[3].X, SidebarRect.Y + layout.TabButtonPosition[3].Y);
    TabButtons[3].Flag_To_Redraw();
    TabButtons[3].DrawX = -SidebarRect.X;

    /**
     *  Position the active strip. All strips share the same position.
     */
    const SidebarStripView::StripLayout strip_layout = Build_Tabbed_Strip_Layout();
    for (auto& strip : Strip) {
        strip.Set_Layout(strip_layout);
        strip.Shift_Sidebar();
    }

    /**
     *  Set up tooltips.
     */
    if (ToolTips) {
        ToolTip tooltip;

        for (int i = 0; i < RegisteredTooltipCount; i++) {
            ToolTips->Remove(1000 + i);
        }

        for (int i = 0; i < Strip[TabIndex].MaxVisibleCount; i++) {
            CameoButtonClass* btn = Strip[TabIndex].SelectButtons[i];
            tooltip.Region = Rect(btn->X, btn->Y, btn->Width, btn->Height);
            tooltip.ID = 1000 + i;
            tooltip.Text = TXT_NONE;
            ToolTips->Add(&tooltip);
        }

        RegisteredTooltipCount = Strip[TabIndex].MaxVisibleCount;

        for (int i = 0; i < SIDEBAR_TAB_COUNT; i++) {
            tooltip.Region = Rect(TabButtons[i].X, TabButtons[i].Y, TabButtons[i].Width, TabButtons[i].Height);
            tooltip.ID = BUTTON_TAB_1 + i;
            tooltip.Text = TXT_NONE;
            ToolTips->Remove(tooltip.ID);
            ToolTips->Add(&tooltip);
        }

    }
}


/***************************************************************************
**  Runtime behavior
***************************************************************************/


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
        } else {
            Strip[i].Update_State();
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
    if (cat == nullptr) {
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
void TabbedSidebarView::Draw()
{
    if (!Map.IsSidebarActive || Debug_Map || SidebarSurface == nullptr) {
        return;
    }

    Surface* oldsurface = LogicalSurface;
    LogicalSurface = SidebarSurface;

    if (BackgroundTopShape == nullptr
        || BackgroundMiddleShape == nullptr
        || BackgroundBottomShape == nullptr
        || BackgroundAddonShape == nullptr) {
        LogicalSurface = oldsurface;
        return;
    }

    Rect rect(0, 0, SidebarSurface->Get_Width(), SidebarSurface->Get_Height());

    /**
     *  Repaint the sidebar background every active frame so the surface is
     *  fully refreshed before blitting.
     */
    int y = SidebarRect.Y;
    Draw_Shape(*SidebarSurface, *SidebarDrawer, BackgroundTopShape, 0, Point2D(0, y), rect, SHAPE_WIN_REL);
    y += BackgroundTopShape->Get_Height();

    int rows = Background_Row_Count();
    for (int i = 0; i < rows; i++, y += BackgroundMiddleShape->Get_Height()) {
        Draw_Shape(*SidebarSurface, *SidebarDrawer, BackgroundMiddleShape, 0, Point2D(0, y), rect, SHAPE_WIN_REL);
    }

    Draw_Shape(*SidebarSurface, *SidebarDrawer, BackgroundBottomShape, 0, Point2D(0, y), rect, SHAPE_WIN_REL);
    Draw_Shape(*SidebarSurface, *SidebarDrawer, BackgroundAddonShape, 0, Point2D(0, y + BackgroundBottomShape->Get_Height()), rect, SHAPE_WIN_REL);

    /**
     *  Tab buttons always redraw (they might be flashing).
     */
    for (auto& button : TabButtons) {
        button.Draw_Me(true);
    }

    /**
     *  Draw the active strip only.
     */
    Strip[TabIndex].Draw(*SidebarSurface, rect);

    LogicalSurface = oldsurface;
}


/**
 *  Returns how many background middle rows fit in the current sidebar height.
 *
 *  @author: ZivDero
 */
int TabbedSidebarView::Background_Row_Count() const
{
    if (SidebarSurface != nullptr
        && BackgroundTopShape != nullptr
        && BackgroundMiddleShape != nullptr
        && BackgroundBottomShape != nullptr) {
        return (SidebarRect.Height
                - BackgroundBottomShape->Get_Height()
                - BackgroundTopShape->Get_Height())
               / BackgroundMiddleShape->Get_Height();
    }

    return SidebarClass::StripClass::MAX_VISIBLE;
}


/***************************************************************************
**  View queries and control
***************************************************************************/


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
void TabbedSidebarView::Activate(bool enabled)
{
    if (enabled) {
        Background.Zap();
        Map.Add_A_Button(Background);

        Strip[TabIndex].Activate();

        for (auto& button : TabButtons) {
            button.Zap();
            Map.Add_A_Button(button);
        }
    } else {
        Map.Remove_A_Button(Background);

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

    const BuildItem* item = strip.Get_Visible_Item(slot);
    if (item == nullptr) {
        return nullptr;
    }

    return Format_Cameo_Tooltip(*item);
}


/**
 *  Returns the total number of visible buttons in the active tab.
 *
 *  @author: ZivDero
 */
int TabbedSidebarView::Visible_Button_Count() const
{
    return Current_Strip().Visible_Button_Count();
}


/**
 *  Returns the number of visible buttons in one tab column.
 *
 *  @author: ZivDero
 */
int TabbedSidebarView::Visible_Buttons_Per_Column() const
{
    return Current_Strip().Visible_Buttons_Per_Column();
}


/**
 *  Captures per-tab strip state before the sidebar model recalcs.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Prepare_Model_Recalc()
{
    for (auto& strip : Strip) {
        strip.Prepare_Model_Recalc();
    }
}


/**
 *  Restores per-tab strip state after the sidebar model recalcs.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Finish_Model_Recalc()
{
    for (auto& strip : Strip) {
        strip.Finish_Model_Recalc();
    }
}


/**
 *  Switches to a different tab.
 *
 *  @author: ZivDero
 */
bool TabbedSidebarView::Change_Tab(int index)
{
    if (index < 0 || index >= SIDEBAR_TAB_COUNT) {
        return false;
    }

    SidebarTabType tab = static_cast<SidebarTabType>(index);

    if (TabIndex == tab) {
        return false;
    }

    BuildCategory* cat = Strip[tab].Get_Category();
    if (!cat || cat->Items.Count() < 1) {
        return false;
    }

    Strip[TabIndex].Deactivate();
    TabButtons[TabIndex].Deselect();

    TabIndex = tab;

    Strip[TabIndex].Activate();
    TabButtons[TabIndex].Select();

    return true;
}


/**
 *  Notifies the view that production of a building in the given category
 *  has completed, so the corresponding tab button starts flashing.
 *
 *  @author: ZivDero
 */
void TabbedSidebarView::Notify_Production_Complete(int category_index)
{
    if (category_index >= 0 && category_index < SIDEBAR_TAB_COUNT) {
        TabButtons[category_index].Start_Flashing();
    }
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
