/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Shared scrollable sidebar column view implementation. Displays one
 *          BuildCategory worth of items with scroll buttons, select buttons,
 *          and hover tracking.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sidebar_strip_view.h"

#include "battleui.h"
#include "bsurface.h"
#include "cameo_button.h"
#include "colorscheme.h"
#include "drawshape.h"
#include "dsurface.h"
#include "extension.h"
#include "factory.h"
#include "fetchres.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "language.h"
#include "mouse.h"
#include "rules.h"
#include "scenario.h"
#include "sidebar.h"
#include "sidebar_model.h"
#include "sideext.h"
#include "spritecollection.h"
#include "super.h"
#include "supertype.h"
#include "supertypeext.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "textprint.h"
#include "tibsun_defines.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "voc.h"

#include <algorithm>


/***************************************************************************
**  Lifecycle and setup
***************************************************************************/


/**
 *  Default constructor for SidebarStripView.
 *
 *  @author: ZivDero
 */
SidebarStripView::SidebarStripView() :
    ID(0),
    Columns(2),
    ColumnX(0),
    ColumnY(0),
    IsActive(false),
    IsScrollingDown(false),
    IsScrolling(false),
    TopIndex(0),
    Scroller(0),
    Slid(0),
    LastSlid(0),
    MaxVisibleCount(0),
    Category(nullptr),
    Layout(),
    Art(),
    UpButton(),
    DownButton(),
    SelectButtons(),
    RecalcSnapshot(),
    HasRecalcSnapshot(false)
{
}


/**
 *  Destructor for SidebarStripView. Frees allocated select buttons.
 *
 *  @author: ZivDero
 */
SidebarStripView::~SidebarStripView()
{
    for (int i = 0; i < SelectButtons.Count(); i++) {
        delete SelectButtons[i];
    }
    SelectButtons.Clear();
}


/**
 *  One-time initialization. Loads shapes needed by the strip.
 *
 *  @author: ZivDero
 */
void SidebarStripView::One_Time(int id)
{
    ID = id;
}


/**
 *  Resets strip state for a new scenario.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Init_Clear()
{
    IsActive = false;
    IsScrollingDown = false;
    IsScrolling = false;
    TopIndex = 0;
    Scroller = 0;
    Slid = 0;
    LastSlid = 0;
    RecalcSnapshot.Clear();
    HasRecalcSnapshot = false;
}


/**
 *  Initializes IO gadgets (scroll buttons and select buttons).
 *  Allocates select buttons dynamically based on the visible count.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Init_IO(int id, int columns)
{
    ID = id;
    Columns = columns;


    /**
     *  Set up the strip-owned scroll buttons.
     */
    UpButton.IsSticky = true;
    UpButton.ID = BUTTON_UP + id;
    UpButton.DrawnOnSidebarSurface = true;
    UpButton.ShapeDrawer = SidebarDrawer;
    UpButton.Flags = GadgetClass::RIGHTRELEASE | GadgetClass::RIGHTPRESS | GadgetClass::LEFTRELEASE | GadgetClass::LEFTPRESS;

    DownButton.IsSticky = true;
    DownButton.ID = BUTTON_DOWN + id;
    DownButton.DrawnOnSidebarSurface = true;
    DownButton.ShapeDrawer = SidebarDrawer;
    DownButton.Flags = GadgetClass::RIGHTRELEASE | GadgetClass::RIGHTPRESS | GadgetClass::LEFTRELEASE | GadgetClass::LEFTPRESS;

    /**
     *  Calculate the number of visible cameo slots.
     */
    MaxVisibleCount = Visible_Button_Count();

    /**
     *  Allocate select buttons for the visible slots.
     */
    Allocate_Select_Buttons(MaxVisibleCount);
}


/**
 *  Applies a precomputed layout configuration to this strip.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Set_Layout(const StripLayout& layout)
{
    Layout = layout;
}


/**
 *  Loads house-specific shapes for the scroll buttons.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Init_For_House()
{
    UpButton.Set_Shape(Art.ScrollUpButtonShape);
    UpButton.ShapeDrawer = SidebarDrawer;

    DownButton.Set_Shape(Art.ScrollDownButtonShape);
    DownButton.ShapeDrawer = SidebarDrawer;
}


/**
 *  Applies the art set used by this strip while drawing.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Set_Art(const StripArt& art)
{
    Art = art;
}


/**
 *  Reflows the strip layout from the current column origin.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Shift_Sidebar()
{
    const bool was_active = IsActive;
    if (was_active) {
        Deactivate();
    }

    ColumnX = Layout.Position.X;
    ColumnY = Layout.Position.Y;

    int new_count = Visible_Button_Count();

    /**
     *  If the visible count changed (resolution change), reallocate buttons.
     */
    if (new_count != MaxVisibleCount) {
        MaxVisibleCount = new_count;
        Allocate_Select_Buttons(MaxVisibleCount);
    }

    /**
     *  Update button positions for each visible slot.
     */
    for (int i = 0; i < MaxVisibleCount; i++) {
        CameoButtonClass* btn = SelectButtons[i];
        btn->Width = Object_Width();
        btn->Height = Object_Height();

        if (Columns == 1) {
            btn->X = SidebarRect.X + ColumnX;
            btn->Y = SidebarRect.Y + ColumnY + i * Row_Pitch();
        } else {
            btn->X = SidebarRect.X + (i % 2 == 0 ? ColumnX : ColumnX + Column_Spacing());
            btn->Y = SidebarRect.Y + ColumnY + (i / 2) * Row_Pitch();
        }
    }

    const Point2D up_position = Get_Up_Button_Position();
    const Point2D down_position = Get_Down_Button_Position();

    UpButton.Set_Position(SidebarRect.X + up_position.X, SidebarRect.Y + up_position.Y);
    UpButton.Flag_To_Redraw();
    UpButton.DrawX = -SidebarRect.X;

    DownButton.Set_Position(SidebarRect.X + down_position.X, SidebarRect.Y + down_position.Y);
    DownButton.Flag_To_Redraw();
    DownButton.DrawX = -SidebarRect.X;

    if (was_active) {
        Activate();
    }
}


/**
 *  Registers all gadgets with the button linked list.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Activate()
{
    IsActive = true;

    if (Layout.IsUpButtonVisible) {
        UpButton.Zap();
        Map.Add_A_Button(UpButton);
    }

    if (Layout.IsDownButtonVisible) {
        DownButton.Zap();
        Map.Add_A_Button(DownButton);
    }

    for (int i = 0; i < MaxVisibleCount; i++) {
        SelectButtons[i]->Zap();
        Map.Add_A_Button(*SelectButtons[i]);
    }
}


/**
 *  Removes all gadgets from the button linked list.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Deactivate()
{
    IsActive = false;

    Map.Remove_A_Button(UpButton);
    Map.Remove_A_Button(DownButton);

    for (int i = 0; i < MaxVisibleCount; i++) {
        Map.Remove_A_Button(*SelectButtons[i]);
    }
}


/***************************************************************************
**  Runtime behavior
***************************************************************************/


/**
 *  Per-frame logic. Handles scroll button input and strip state updates.
 *
 *  @author: ZivDero
 */
bool SidebarStripView::AI(KeyNumType& input, Point2D&)
{
    if (Category == nullptr) {
        return false;
    }

    /**
     *  Handle scroll button presses.
     */
    if (input == KeyNumType(UpButton.ID | KN_BUTTON)) {
        UpButton.IsPressed = false;
        if (!Scroll(true)) {
            Sound_Effect(Rule->ScoldSound);
        }
    }
    if (input == KeyNumType(DownButton.ID | KN_BUTTON)) {
        DownButton.IsPressed = false;
        if (!Scroll(false)) {
            Sound_Effect(Rule->ScoldSound);
        }
    }

    return Update_State();
}


/**
 *  Advances scroll animation and selection flash state.
 *
 *  @author: ZivDero
 */
bool SidebarStripView::Update_State()
{
    if (Category == nullptr) {
        return false;
    }

    bool redraw = false;
    int item_count = Category->Items.Count();

    const int clamped_top = std::clamp(TopIndex, 0, Max_Top_Index(item_count));
    if (clamped_top != TopIndex) {
        TopIndex = clamped_top;
        redraw = true;
    }

    /**
     *  Reflect the scroll desired direction/value into the scroll
     *  logic handler.
     */
    if (!IsScrolling && Scroller) {
        if (item_count <= MaxVisibleCount) {
            Scroller = 0;
        } else {
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
                if (TopIndex + MaxVisibleCount > item_count) {
                    Scroller = 0;
                } else {
                    Scroller--;
                    Slid = Row_Pitch();
                    IsScrollingDown = true;
                    IsScrolling = true;
                }
            }
        }
    }

    /**
     *  Scroll animation tick.
     */
    if (IsScrolling) {
        if (IsScrollingDown) {
            Slid -= Scroll_Step();
            if (Slid <= 0) {
                IsScrolling = false;
                Slid = 0;
                TopIndex += Columns;
            }
        } else {
            Slid += Scroll_Step();
            if (Slid >= Row_Pitch()) {
                IsScrolling = false;
                Slid = 0;
            }
        }
        redraw = true;
    }

    return redraw;
}


/**
 *  Captures the currently visible item keys so TopIndex can be restored
 *  sensibly after the category is purged/re-sorted.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Prepare_Model_Recalc()
{
    RecalcSnapshot.Clear();
    HasRecalcSnapshot = false;

    if (Category == nullptr || MaxVisibleCount <= 0) {
        return;
    }

    for (int slot = 0; slot < MaxVisibleCount; ++slot) {
        RecalcSnapshotItem snapshot_item;
        const BuildItem* item = Get_Visible_Item(slot);
        if (item != nullptr) {
            snapshot_item.Type = item->Type;
            snapshot_item.ID = item->ID;
        }
        RecalcSnapshot.Add(snapshot_item);
    }

    HasRecalcSnapshot = true;
}


/**
 *  Restores TopIndex after the category changes by anchoring to the first
 *  previously visible item that still exists.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Finish_Model_Recalc()
{
    if (Category == nullptr) {
        RecalcSnapshot.Clear();
        HasRecalcSnapshot = false;
        return;
    }

    if (!HasRecalcSnapshot) {
        TopIndex = std::clamp(TopIndex, 0, Max_Top_Index(Category->Items.Count()));
        return;
    }

    bool found_old = false;
    bool found_new = false;
    int old_position = 0;
    int new_position = 0;

    for (old_position = 0; old_position < RecalcSnapshot.Count(); ++old_position) {
        const RecalcSnapshotItem& snapshot_item = RecalcSnapshot[old_position];
        if (!snapshot_item.Is_Valid()) {
            continue;
        }

        found_old = true;

        for (new_position = 0; new_position < Category->Items.Count(); ++new_position) {
            const BuildItem& current_item = Category->Items[new_position];
            if (current_item.Type == snapshot_item.Type && current_item.ID == snapshot_item.ID) {
                found_new = true;
                break;
            }
        }

        if (found_new) {
            break;
        }
    }

    if (found_old && found_new) {
        TopIndex = new_position - old_position;
    } else {
        TopIndex = 0;
    }

    if (Columns > 1 && TopIndex > 0) {
        TopIndex -= TopIndex % Columns;
    }

    TopIndex = std::clamp(TopIndex, 0, Max_Top_Index(Category->Items.Count()));
    Scroller = 0;
    IsScrolling = false;
    IsScrollingDown = false;
    Slid = 0;
    LastSlid = 0;

    RecalcSnapshot.Clear();
    HasRecalcSnapshot = false;
}


/**
 *  Draws the strip's scroll buttons and all visible cameo items.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw(Surface& surface, const Rect& rect)
{
    if (Category == nullptr) {
        return;
    }

    /**
     *  Draw scroll buttons.
     */
    if (Layout.IsUpButtonVisible) {
        UpButton.Draw_Me(true);
    }

    if (Layout.IsDownButtonVisible) {
        DownButton.Draw_Me(true);
    }

    /**
     *  Draw all visible cameo items.
     */
    Draw_Strip_Items(surface, rect);

    LastSlid = Slid;
}


/**
 *  Scrolls the strip up or down by one row (2 items).
 *
 *  @author: ZivDero
 */
bool SidebarStripView::Scroll(bool up)
{
    if (Category == nullptr) {
        return false;
    }

    int item_count = Category->Items.Count();

    if (up) {
        if (!TopIndex) {
            return false;
        }
        Scroller--;
    } else {
        if (TopIndex + MaxVisibleCount >= item_count + item_count % Columns) {
            return false;
        }
        Scroller++;
    }

    return true;
}


/**
 *  Scrolls the strip by one full page.
 *
 *  @author: ZivDero
 */
bool SidebarStripView::Scroll_Page(bool up)
{
    if (Category == nullptr) {
        return false;
    }

    int item_count = Category->Items.Count();
    int rows_per_page = MaxVisibleCount / Columns;

    if (up) {
        if (!TopIndex) {
            return false;
        }
        Scroller -= rows_per_page;
    } else {
        if (TopIndex + MaxVisibleCount >= item_count + item_count % Columns) {
            return false;
        }
        Scroller += rows_per_page;
    }

    return true;
}


/**
 *  Returns the visible item mapped to the given slot, or nullptr.
 *
 *  @author: ZivDero
 */
BuildItem* SidebarStripView::Get_Visible_Item(int slot)
{
    if (Category == nullptr || slot < 0) {
        return nullptr;
    }

    const int item_index = TopIndex + slot;
    if (item_index < 0 || item_index >= Category->Items.Count()) {
        return nullptr;
    }

    return &Category->Items[item_index];
}


/**
 *  Returns the visible item mapped to the given slot, or nullptr.
 *
 *  @author: ZivDero
 */
const BuildItem* SidebarStripView::Get_Visible_Item(int slot) const
{
    if (Category == nullptr || slot < 0) {
        return nullptr;
    }

    const int item_index = TopIndex + slot;
    if (item_index < 0 || item_index >= Category->Items.Count()) {
        return nullptr;
    }

    return &Category->Items[item_index];
}


/**
 *  Allocates cameo selection buttons for the current visible slot count.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Allocate_Select_Buttons(int count)
{
    for (int i = 0; i < SelectButtons.Count(); i++) {
        delete SelectButtons[i];
    }
    SelectButtons.Clear();

    for (int i = 0; i < count; i++) {
        CameoButtonClass* btn = new CameoButtonClass();
        btn->ID = BUTTON_SELECT;
        btn->Width = Object_Width();
        btn->Height = Object_Height();
        btn->Set_Owner(*this, i);
        SelectButtons.Add(btn);
    }
}


/***************************************************************************
**  Visibility and layout queries
***************************************************************************/


/**
 *  Returns true if any item in the category has completed production.
 *
 *  @author: ZivDero
 */
bool SidebarStripView::Has_Ready() const
{
    if (Category == nullptr) {
        return false;
    }

    for (int i = 0; i < Category->Items.Count(); i++) {
        if (Category->Items[i].Is_Completed()) {
            return true;
        }
    }

    return false;
}


/**
 *  Computes how many cameo buttons fit in the strip.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Visible_Button_Count() const
{
    return Visible_Buttons_Per_Column() * Columns;
}


/**
 *  Returns the last valid TopIndex for the current item count.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Max_Top_Index(int item_count) const
{
    if (item_count <= 0 || Columns <= 0 || MaxVisibleCount <= 0) {
        return 0;
    }

    const int visible_rows = std::max(1, MaxVisibleCount / Columns);
    const int row_count = (item_count + Columns - 1) / Columns;
    return std::max(0, (row_count - visible_rows) * Columns);
}


/**
 *  Computes how many cameo rows fit in one strip column.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Visible_Buttons_Per_Column() const
{
    if (SidebarSurface != nullptr && Art.BackgroundTopHeight > 0 && Art.BackgroundBottomHeight > 0) {
        return std::max(1, Available_Content_Height() / Row_Pitch());
    }

    return SidebarClass::StripClass::MAX_VISIBLE;
}


/***************************************************************************
**  Item resolution helpers
***************************************************************************/


/**
 *  Resolves the draw point for a visible slot, including scroll offset.
 *
 *  @author: ZivDero
 */
Point2D SidebarStripView::Get_Item_Point(int slot) const
{
    int x = ColumnX;
    int y = ColumnY;

    if (Columns == 1) {
        y += slot * Row_Pitch();
    } else {
        x = slot % 2 == 0 ? ColumnX : ColumnX + Column_Spacing();
        y += (slot / 2) * Row_Pitch();
    }

    if (IsScrolling) {
        y -= Row_Pitch() - Slid;
    }

    return {x, SidebarRect.Y + y};
}


/**
 *  Resolves all runtime draw state needed for one visible build item.
 *
 *  @author: ZivDero
 */
SidebarStripView::StripItemDrawState SidebarStripView::Get_Item_Draw_State(const BuildItem& item) const
{
    StripItemDrawState state;

    if (item.Type == RTTI_SPECIAL) {
        const SuperWeaponType superweapon = static_cast<SuperWeaponType>(item.ID);
        state.Name = SuperWeaponTypes[superweapon]->GivenName.c_str();
        state.Production = true;
        state.Completed = !PlayerPtr->SuperWeapon[superweapon]->Needs_Redraw();
        state.IsReady = PlayerPtr->SuperWeapon[superweapon]->Can_Place();
        state.StateText = PlayerPtr->SuperWeapon[superweapon]->Ready_String();
        state.Stage = PlayerPtr->SuperWeapon[superweapon]->Anim_Stage();

        return state;
    }

    const TechnoTypeClass* object = Fetch_Techno_Type(item.Type, item.ID);
    if (object == nullptr) {
        return state;
    }

    state.Name = object->GivenName.c_str();

    if (object->RTTI == RTTI_BUILDINGTYPE) {
        state.Darken = Extension::Fetch(PlayerPtr)->Fetch_Factory(item.Type, TechnoTypeClassExtension::Get_Production_Flags(object)) != nullptr;
    }

    if (!object->Who_Can_Build_Me(true, true, true, PlayerPtr) || (!state.Darken && PlayerPtr->Can_Build(object, false, false) == -1)) {
        state.Darken = true;
    }

    state.Factory = item.Factory;
    if (state.Factory != nullptr) {
        state.Production = true;
        state.Completed = item.Is_Completed();
        state.IsOnHold = item.Is_On_Hold();
        state.Stage = item.Completion_Stage();
        state.StateText = state.Completed ? Fetch_String(TXT_READY) : nullptr;
        state.Darken = false;
    }

    state.QueueCount = Get_Item_Queue_Count(*object);
    return state;
}


/**
 *  Returns the displayed queue count for the specified techno type.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Get_Item_Queue_Count(const TechnoTypeClass& object) const
{
    FactoryClass* queued_factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(object.RTTI, TechnoTypeClassExtension::Get_Production_Flags(&object));
    if (queued_factory == nullptr) {
        return 0;
    }

    const int total = queued_factory->Total_Queued(object);
    if (total > 1) {
        return total;
    }

    if (total > 0 && (queued_factory->Object == nullptr
        || (queued_factory->Object->TClass != nullptr && queued_factory->Object->TClass != &object))) {
        return total;
    }

    return 0;
}


/***************************************************************************
**  Draw primitives
***************************************************************************/


/**
 *  Draws one strip overlay shape with the sidebar drawer.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Shape_Overlay(Surface& surface, const ShapeSet* shape, const Rect& rect, const Point2D& point, int frame, int flags)
{
    if (shape != nullptr) {
        Draw_Shape(surface, *SidebarDrawer, shape, frame, point, rect, static_cast<ShapeFlags_Type>(SHAPE_WIN_REL | flags));
    }
}


/**
 *  Draws the base cameo image or shape for a build item.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Cameo(Surface& surface, const Rect& rect, const BuildItem& item, const Point2D& point)
{
    const ShapeSet* shapefile = nullptr;
    BSurface* image_surface = nullptr;

    if (item.Type != RTTI_SPECIAL) {
        const TechnoTypeClass* object = Fetch_Techno_Type(item.Type, item.ID);
        if (object != nullptr) {
            shapefile = object->Get_Cameo_Data();

            const auto* technoext = Extension::Fetch(object);
            if (technoext->CameoImageSurface != nullptr) {
                image_surface = technoext->CameoImageSurface;
            }
        } else {
            shapefile = SidebarClass::StripClass::LogoShape;
        }
    } else {
        const SuperWeaponType superweapon = static_cast<SuperWeaponType>(item.ID);
        shapefile = Map.Column[0].Get_Special_Cameo(superweapon);

        const auto* supertypeext = Extension::Fetch(PlayerPtr->SuperWeapon[superweapon]->Class);
        if (supertypeext->CameoImageSurface != nullptr) {
            image_surface = supertypeext->CameoImageSurface;
        }

        if (superweapon == SUPER_NONE) {
            shapefile = SidebarClass::StripClass::LogoShape;
        }
    }

    if (image_surface != nullptr) {
        Rect image_rect(rect.X + point.X, rect.Y + point.Y, image_surface->Get_Width(), image_surface->Get_Height());
        SpriteCollection.Draw(image_rect, surface, *image_surface);
    } else if (shapefile != nullptr) {
        Draw_Shape(surface, *CameoDrawer, shapefile, 0, point, rect, SHAPE_WIN_REL);
    }
}


/**
 *  Draws the standard production clock overlay.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Clock_Overlay(Surface& surface, const Rect& rect, const Point2D& point, int stage)
{
    Draw_Shape_Overlay(surface, Art.ClockShape, rect, point, stage + 1, SHAPE_TRANS50);
}


/**
 *  Draws the superweapon recharge clock overlay.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Recharge_Clock(Surface& surface, const Rect& rect, const Point2D& point, int stage)
{
    Draw_Shape_Overlay(surface, Art.RechargeClockShape, rect, point, stage + 1, SHAPE_TRANS50);
}


/**
 *  Draws the unavailable-item darken overlay.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Darken_Overlay(Surface& surface, const Rect& rect, const Point2D& point)
{
    Draw_Shape_Overlay(surface, Art.DarkenShape, rect, point, 0, SHAPE_DARKEN);
}


/**
 *  Draws ready-state text centered on a cameo.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Ready_Text(Surface& surface, const Rect& rect, const Point2D& point, const char* text)
{
    if (text == nullptr) {
        return;
    }

    Fancy_Text_Print(text, surface, rect, point, Fetch_Scheme_By_Name("LightBlue", 1), COLOR_TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_8POINT);
}


/**
 *  Draws hold-state text for paused production.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Hold_Text(Surface& surface, const Rect& rect, const Point2D& point, bool has_queue_count)
{
    if (has_queue_count) {
        Fancy_Text_Print(TXT_HOLD, surface, rect, point, Fetch_Scheme_By_Name("LightGrey", 1), COLOR_TBLACK, TPF_FULLSHADOW | TPF_8POINT);
    } else {
        Point2D centered(point.X + Object_Width() / 2, point.Y);
        Fancy_Text_Print(TXT_HOLD, surface, rect, centered, Fetch_Scheme_By_Name("LightGrey", 1), COLOR_TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_8POINT);
    }
}


/**
 *  Draws the queue count text for a cameo.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Queue_Count(Surface& surface, const Rect& rect, const Point2D& point, int count)
{
    Fancy_Text_Print("%d", surface, rect, point, Fetch_Scheme_By_Name("LightGrey", 1), COLOR_TBLACK, TPF_RIGHT | TPF_FULLSHADOW | TPF_8POINT, count);
}


/**
 *  Draws the hover outline for a cameo slot.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Hover_Highlight(Surface& surface, const Rect& cameo_rect)
{
    const ColorSchemeType colorscheme = Extension::Fetch(Sides[PlayerPtr->Class->Side])->HoverHighlightColor;
    surface.Draw_Rect(cameo_rect, DSurface::Build_Hicolor_Pixel(ColorSchemes[colorscheme]->HSV.operator RGBClass()));
}


/**
 *  Draws the cameo name text below the icon.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Cameo_Name(const Rect& rect, const Point2D& point, const char* name)
{
    if (name != nullptr) {
        Print_Cameo_Text(name, point, rect, Object_Width());
    }
}


/***************************************************************************
**  Per-item rendering
***************************************************************************/


/**
 *  Draws production-specific overlays for a visible cameo slot.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Item_Production(Surface& surface, const Rect& rect, const Point2D& drawpoint, const StripItemDrawState& state)
{
    if (!state.Production) {
        return;
    }

    if (state.StateText != nullptr) {
        Draw_Ready_Text(surface, rect, drawpoint + Text_Offset(), state.StateText);
    }

    if (state.Completed) {
        return;
    }

    if (state.IsReady) {
        Draw_Recharge_Clock(surface, rect, drawpoint, state.Stage);
    } else {
        Draw_Clock_Overlay(surface, rect, drawpoint, state.Stage);
    }

    if (state.Factory != nullptr && state.IsOnHold) {
        Draw_Hold_Text(surface, rect, drawpoint + Point2D(0, Text_Offset().Y), state.QueueCount > 0);
    }
}

/**
 *  Draws one visible cameo slot and all of its overlays.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Item(Surface& surface, const Rect& rect, int slot)
{
    const BuildItem* item = Get_Visible_Item(slot);
    if (item == nullptr) {
        return;
    }

    const Point2D drawpoint = Get_Item_Point(slot);
    const StripItemDrawState state = Get_Item_Draw_State(*item);

    Draw_Cameo(surface, rect, *item, drawpoint);

    if (slot < SelectButtons.Count()) {
        const bool is_hovered = SelectButtons[slot]->IsMousedOver;
        if (is_hovered && !Scen->InputLock && !state.Darken) {
            Rect cameo_rect(drawpoint.X, drawpoint.Y, Object_Width(), std::max(1, Object_Height() - 3));
            Draw_Hover_Highlight(surface, cameo_rect);
        }
    }

    if (state.Darken) {
        Draw_Darken_Overlay(surface, rect, drawpoint);
    }

    if (state.Name != nullptr) {
        Draw_Cameo_Name(rect, drawpoint + Point2D(0, Object_Name_Offset()), state.Name);
    }

    if (state.QueueCount > 0) {
        Draw_Queue_Count(surface, rect, drawpoint + Queue_Count_Offset(), state.QueueCount);
    }

    Draw_Item_Production(surface, rect, drawpoint, state);
}


/**
 *  Internal: renders all visible cameo items in the strip.
 *  Ported from StripClassExt::_Draw_It with strip-owned render helpers.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Strip_Items(Surface& surface, const Rect& rect)
{
    const int visible_count = MaxVisibleCount + (IsScrolling ? Columns : 0);
    for (int slot = 0; slot < visible_count; slot++) {
        Draw_Item(surface, rect, slot);
    }
}


/***************************************************************************
**  Layout helpers
***************************************************************************/


/**
 *  Returns the vertical draw space available to cameo content.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Available_Content_Height() const
{
    return SidebarRect.Height - Art.BackgroundBottomHeight - Art.BackgroundTopHeight;
}


/**
 *  Returns the configured cameo width.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Object_Width() const
{
    return std::max(1, Layout.CameoSize.X);
}


/**
 *  Returns the configured cameo height.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Object_Height() const
{
    return std::max(1, Layout.CameoSize.Y);
}


/**
 *  Returns the configured Y offset for the cameo name.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Object_Name_Offset() const
{
    return Layout.CameoNameOffset;
}


/**
 *  Returns the configured per-row cameo pitch.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Row_Pitch() const
{
    return std::max(1, Layout.CameoSize.Y + Layout.RowSpacing);
}


/**
 *  Returns the configured spacing between strip columns.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Column_Spacing() const
{
    return std::max(1, Layout.CameoSize.X + Layout.ColumnSpacing);
}


/**
 *  Returns the current scroll animation step.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Scroll_Step() const
{
    return std::min(static_cast<int>(SCROLL_RATE), Row_Pitch());
}


/**
 *  Returns the configured text offset used for state labels.
 *
 *  @author: ZivDero
 */
Point2D SidebarStripView::Text_Offset() const
{
    return Layout.CameoTextOffset;
}


/**
 *  Returns the configured queue count text offset.
 *
 *  @author: ZivDero
 */
Point2D SidebarStripView::Queue_Count_Offset() const
{
    return Layout.QueueCountOffset;
}


/**
 *  Resolves the up scroll button position for the current layout.
 *
 *  @author: ZivDero
 */
Point2D SidebarStripView::Get_Up_Button_Position() const
{
    if (Layout.UpButtonPosition != Point2D { StripLayout::AUTO_POSITION, StripLayout::AUTO_POSITION }) {
        return Layout.UpButtonPosition;
    }

    return {ColumnX + 1, ColumnY + Visible_Buttons_Per_Column() * Row_Pitch() - 1};
}


/**
 *  Resolves the down scroll button position for the current layout.
 *
 *  @author: ZivDero
 */
Point2D SidebarStripView::Get_Down_Button_Position() const
{
    if (Layout.DownButtonPosition != Point2D {StripLayout::AUTO_POSITION, StripLayout::AUTO_POSITION}) {
        return Layout.DownButtonPosition;
    }

    return {ColumnX + (Columns == 1 ? 30 : Column_Spacing() + 3), ColumnY + Visible_Buttons_Per_Column() * Row_Pitch() - 1};
}
