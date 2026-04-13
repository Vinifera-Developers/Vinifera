/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_STRIP_VIEW.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Shared scrollable sidebar column view implementation.
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

#include "sidebar_strip_view.h"

#include "cameo_button.h"
#include "sidebar_model.h"
#include "sidebar_render_utils.h"

#include "bsurface.h"
#include "drawshape.h"
#include "dsurface.h"
#include "eventext.h"
#include "extension.h"
#include "factory.h"
#include "factoryext.h"
#include "fetchres.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "mouse.h"
#include "rules.h"
#include "scenario.h"
#include "sidebar.h"
#include "language.h"
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
#include "vox.h"


/**
 *  Default constructor for SidebarStripView.
 *
 *  @author: ZivDero
 */
SidebarStripView::SidebarStripView() :
    StageClass(),
    ID(0),
    Columns(2),
    ColumnX(0),
    ColumnY(0),
    IsToRedraw(true),
    IsBuilding(false),
    IsScrollingDown(false),
    IsScrolling(false),
    TopIndex(0),
    Scroller(0),
    Slid(0),
    LastSlid(0),
    MaxVisibleCount(0),
    Category(nullptr),
    SelectButtons()
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

    /**
     *  DarkenShape is a static on StripClass, loaded once.
     */
    SidebarClass::StripClass::DarkenShape = MFCD::RetrieveT<ShapeSet>("DARKEN.SHP");
}


/**
 *  Resets strip state for a new scenario.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Init_Clear()
{
    IsScrollingDown = false;
    IsScrolling = false;
    IsBuilding = false;
    IsToRedraw = true;
    TopIndex = 0;
    Scroller = 0;
    Slid = 0;
    LastSlid = 0;

    Set_Rate(0);
    Set_Stage(0);
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
     *  Set up the scroll buttons. These are shared statics on StripClass —
     *  in the new sidebar, they use index [0] since there's only one active
     *  strip set at a time.
     */
    auto& up = SidebarClass::StripClass::UpButton[0];
    up.IsSticky = true;
    up.ID = BUTTON_UP;
    up.DrawnOnSidebarSurface = true;
    up.ShapeDrawer = SidebarDrawer;
    up.Flags = GadgetClass::RIGHTRELEASE | GadgetClass::RIGHTPRESS
             | GadgetClass::LEFTRELEASE | GadgetClass::LEFTPRESS;

    auto& down = SidebarClass::StripClass::DownButton[0];
    down.IsSticky = true;
    down.ID = BUTTON_DOWN;
    down.DrawnOnSidebarSurface = true;
    down.ShapeDrawer = SidebarDrawer;
    down.Flags = GadgetClass::RIGHTRELEASE | GadgetClass::RIGHTPRESS
               | GadgetClass::LEFTRELEASE | GadgetClass::LEFTPRESS;

    /**
     *  Calculate the number of visible cameo slots.
     */
    MaxVisibleCount = Max_Visible();

    /**
     *  Allocate select buttons for the visible slots.
     */
    for (int i = 0; i < SelectButtons.Count(); i++) {
        delete SelectButtons[i];
    }
    SelectButtons.Clear();

    for (int i = 0; i < MaxVisibleCount; i++) {
        CameoButtonClass* btn = new CameoButtonClass();
        btn->ID = BUTTON_SELECT;
        btn->Width = OBJECT_WIDTH;
        btn->Height = OBJECT_HEIGHT;
        SelectButtons.Add(btn);
    }
}


/**
 *  Loads house-specific shapes for the scroll buttons.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Init_For_House(int id)
{
    SidebarClass::StripClass::UpButton[0].Set_Shape(MFCD::RetrieveT<ShapeSet>("R-UP.SHP"));
    SidebarClass::StripClass::UpButton[0].ShapeDrawer = SidebarDrawer;

    SidebarClass::StripClass::DownButton[0].Set_Shape(MFCD::RetrieveT<ShapeSet>("R-DN.SHP"));
    SidebarClass::StripClass::DownButton[0].ShapeDrawer = SidebarDrawer;
}


/**
 *  Recalculates button positions from the given column origin.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Set_Dimensions(int column_x, int column_y)
{
    ColumnX = column_x;
    ColumnY = column_y;

    int new_count = Max_Visible();

    /**
     *  If the visible count changed (resolution change), reallocate buttons.
     */
    if (new_count != MaxVisibleCount) {
        for (int i = 0; i < SelectButtons.Count(); i++) {
            delete SelectButtons[i];
        }
        SelectButtons.Clear();

        MaxVisibleCount = new_count;
        for (int i = 0; i < MaxVisibleCount; i++) {
            CameoButtonClass* btn = new CameoButtonClass();
            btn->ID = BUTTON_SELECT;
            btn->Width = OBJECT_WIDTH;
            btn->Height = OBJECT_HEIGHT;
            SelectButtons.Add(btn);
        }
    }

    /**
     *  Update button positions for each visible slot.
     */
    for (int i = 0; i < MaxVisibleCount; i++) {
        CameoButtonClass* btn = SelectButtons[i];
        if (Columns == 1) {
            btn->X = SidebarRect.X + ColumnX;
            btn->Y = SidebarRect.Y + ColumnY + i * OBJECT_HEIGHT;
        } else {
            btn->X = SidebarRect.X + (i % 2 == 0 ? ColumnX : ColumnX + 67);
            btn->Y = SidebarRect.Y + ColumnY + (i / 2) * OBJECT_HEIGHT;
        }
    }
}


/**
 *  Registers all gadgets with the button linked list.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Activate()
{
    SidebarClass::StripClass::UpButton[0].Zap();
    Map.Add_A_Button(SidebarClass::StripClass::UpButton[0]);

    SidebarClass::StripClass::DownButton[0].Zap();
    Map.Add_A_Button(SidebarClass::StripClass::DownButton[0]);

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
    Map.Remove_A_Button(SidebarClass::StripClass::UpButton[0]);
    Map.Remove_A_Button(SidebarClass::StripClass::DownButton[0]);

    for (int i = 0; i < MaxVisibleCount; i++) {
        Map.Remove_A_Button(*SelectButtons[i]);
    }
}


/**
 *  Per-frame logic. Handles scroll animation, production completion
 *  announcements, and flash feedback.
 *
 *  @author: ZivDero
 */
bool SidebarStripView::AI(KeyNumType& input, Point2D& xy)
{
    if (Category == nullptr) {
        return false;
    }

    bool redraw = false;
    int item_count = Category->Items.Count();

    /**
     *  Check if any item has an active factory — this drives the building
     *  animation checks below.
     */
    IsBuilding = false;
    for (int i = 0; i < item_count; i++) {
        if (Category->Items[i].Factory != nullptr) {
            IsBuilding = true;
            break;
        }
    }

    /**
     *  Handle building clock animation logic — check each item's factory
     *  for completed production or state changes.
     */
    if (IsBuilding) {
        for (int i = 0; i < item_count; i++) {
            BuildItem& item = Category->Items[i];
            FactoryClass* factory = item.Factory;
            if (factory != nullptr && (factory->Has_Changed() || Extension::Fetch(factory)->IsHoldingExit)) {
                redraw = true;
                if (factory->Has_Completed()) {
                    TechnoClass* pending = factory->Get_Object();
                    if (pending != nullptr) {
                        switch (pending->RTTI) {
                        case RTTI_UNIT:
                        case RTTI_INFANTRY:
                        case RTTI_AIRCRAFT:
                            OutList.Add(EventClassExt(pending->Owner(), EVENT_PLACE, pending->RTTI, CELL_NONE, TechnoTypeClassExtension::Get_Production_Flags(pending)).As_Event());
                            break;

                        case RTTI_BUILDING:
                            Speak(VOX_CONSTRUCTION);
                            break;

                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    /**
     *  Handle scroll button presses.
     */
    if (input == KeyNumType(SidebarClass::StripClass::UpButton[0].ID | KN_BUTTON)) {
        SidebarClass::StripClass::UpButton[0].IsPressed = false;
        if (!Scroll(true)) {
            Sound_Effect(Rule->ScoldSound);
        }
    }
    if (input == KeyNumType(SidebarClass::StripClass::DownButton[0].ID | KN_BUTTON)) {
        SidebarClass::StripClass::DownButton[0].IsPressed = false;
        if (!Scroll(false)) {
            Sound_Effect(Rule->ScoldSound);
        }
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
                    Slid = OBJECT_HEIGHT;
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
            Slid -= SCROLL_RATE;
            if (Slid <= 0) {
                IsScrolling = false;
                Slid = 0;
                TopIndex += Columns;
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

    /**
     *  Handle selection flash logic.
     */
    if (Graphic_Logic()) {
        redraw = true;
        if (Fetch_Stage() >= 7) {
            Set_Rate(0);
            Set_Stage(0);
        }
    }

    if (redraw) {
        IsToRedraw = true;
        Flag_To_Redraw();
        RedrawSidebar = true;
    }

    return redraw;
}


/**
 *  Main draw entry point. Draws the strip if it needs redrawing.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw(Surface& surface, const Rect& rect, bool complete)
{
    if (Category == nullptr) {
        return;
    }

    if (IsToRedraw || complete) {
        IsToRedraw = false;
        RedrawSidebar = true;

        /**
         *  Draw scroll buttons.
         */
        SidebarClass::StripClass::UpButton[0].Draw_Me(true);
        SidebarClass::StripClass::DownButton[0].Draw_Me(true);

        /**
         *  Draw all visible cameo items.
         */
        Draw_Strip_Items(surface, rect);

        LastSlid = Slid;
        return;
    }

    /**
     *  Even if the strip didn't need full redraw, check if scroll
     *  buttons drew themselves and flag the sidebar for blit.
     */
    if (SidebarClass::StripClass::UpButton[0].IsDrawn) {
        RedrawSidebar = true;
        SidebarClass::StripClass::UpButton[0].IsDrawn = false;
    }

    if (SidebarClass::StripClass::DownButton[0].IsDrawn) {
        RedrawSidebar = true;
        SidebarClass::StripClass::DownButton[0].IsDrawn = false;
    }
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
 *  Flags the strip for redraw.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Flag_To_Redraw()
{
    IsToRedraw = true;
}


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
 *  Computes how many cameo slots fit in the sidebar area.
 *  Uses the same formula as the old SidebarClassExtension::Max_Visible.
 *
 *  @author: ZivDero
 */
int SidebarStripView::Max_Visible() const
{
    if (SidebarSurface != nullptr && SidebarClass::SidebarShape != nullptr) {
        int rows = (SidebarRect.Height
                    - SidebarClass::SidebarBottomShape->Get_Height()
                    - SidebarClass::SidebarShape->Get_Height())
                   / SidebarClass::SidebarMiddleShape->Get_Height();
        return rows * Columns;
    }

    return SidebarClass::StripClass::MAX_VISIBLE;
}


/**
 *  Internal: renders all visible cameo items in the strip.
 *  Ported from StripClassExt::_Draw_It with shared render utilities.
 *
 *  @author: ZivDero
 */
void SidebarStripView::Draw_Strip_Items(Surface& surface, const Rect& rect)
{
    int item_count = Category->Items.Count();
    int visible = MaxVisibleCount + (IsScrolling ? Columns : 0);

    for (int i = 0; i < visible; i++) {
        int index = i + TopIndex;
        int x, y;

        if (Columns == 1) {
            x = ColumnX;
            y = ColumnY + i * OBJECT_HEIGHT;
        } else {
            x = i % 2 == 0 ? ColumnX : ColumnX + 67;
            y = ColumnY + (i / 2) * OBJECT_HEIGHT;
        }

        bool production = false;
        bool completed = false;
        int stage = 0;
        bool darken = false;
        FactoryClass* factory = nullptr;
        bool isready = false;
        const char* state = nullptr;
        const char* name = nullptr;
        const TechnoTypeClass* obj = nullptr;

        /**
         *  Adjust for smooth scrolling.
         */
        if (IsScrolling) {
            y -= OBJECT_HEIGHT - Slid;
        }

        /**
         *  Fetch the data for the object at this index.
         */
        if (index < item_count) {
            BuildItem& item = Category->Items[index];

            if (item.Type != RTTI_SPECIAL) {
                obj = Fetch_Techno_Type(item.Type, item.ID);
                if (obj != nullptr) {
                    name = obj->GivenName.c_str();
                    darken = false;

                    /**
                     *  If there is already a factory producing a building, then all
                     *  buildings are displayed in a disabled state.
                     */
                    if (obj->RTTI == RTTI_BUILDINGTYPE) {
                        darken = Extension::Fetch(PlayerPtr)->Fetch_Factory(item.Type, TechnoTypeClassExtension::Get_Production_Flags(obj)) != nullptr;
                    }

                    /**
                     *  If there is no factory that can produce this, or the factory that
                     *  can produce this is currently busy, objects of this type are
                     *  displayed in a disabled state.
                     */
                    if (!obj->Who_Can_Build_Me(true, true, true, PlayerPtr)
                        || (!darken && PlayerPtr->Can_Build(Fetch_Techno_Type(item.Type, item.ID), false, false) == -1)) {
                        darken = true;
                    }

                    factory = item.Factory;
                    if (factory != nullptr) {
                        production = true;
                        completed = item.Is_Completed();
                        if (completed) {
                            state = Fetch_String(TXT_READY);
                        }
                        stage = item.Completion_Percent();
                        darken = false;
                    } else {
                        production = false;
                    }
                }
            } else {
                SuperWeaponType spc = static_cast<SuperWeaponType>(item.ID);

                name = SuperWeaponTypes[spc]->GivenName.c_str();
                production = true;
                completed = !PlayerPtr->SuperWeapon[spc]->Needs_Redraw();
                isready = PlayerPtr->SuperWeapon[spc]->Can_Place();
                state = PlayerPtr->SuperWeapon[spc]->Ready_String();
                stage = PlayerPtr->SuperWeapon[spc]->Anim_Stage();
                darken = false;
            }

            /**
             *  Draw the cameo icon.
             */
            Point2D drawpoint(x, y);
            Draw_Cameo(surface, rect, item, drawpoint);

            /**
             *  Draw hover highlight if moused over and available.
             */
            if (i < SelectButtons.Count()) {
                bool over = SelectButtons[i]->MousedOver;
                if (over && !Scen->InputLock && !darken) {
                    Rect cameo_rect(x, SidebarRect.Y + y, OBJECT_WIDTH, OBJECT_HEIGHT - 3);
                    Draw_Hover_Highlight(surface, cameo_rect);
                }
            }

            /**
             *  Darken unavailable items.
             */
            if (darken) {
                Point2D dp(x, y);
                Draw_Darken_Overlay(surface, *SidebarDrawer, rect, dp);
            }

            /**
             *  Draw the cameo name.
             */
            if (name != nullptr) {
                Point2D namepoint(x, y + OBJECT_NAME_OFFSET);
                Draw_Cameo_Name(surface, rect, namepoint, name, OBJECT_WIDTH);
            }

            /**
             *  Draw queue count.
             */
            bool has_queue_count = false;
            if (obj != nullptr) {
                RTTIType rtti = obj->RTTI;
                FactoryClass* queued_factory = Extension::Fetch(PlayerPtr)->Fetch_Factory(rtti, TechnoTypeClassExtension::Get_Production_Flags(obj));
                if (queued_factory != nullptr) {
                    int total = queued_factory->Total_Queued(*obj);
                    if (total > 1
                        || (total > 0 && (queued_factory->Object == nullptr
                            || (queued_factory->Object->TClass != nullptr && queued_factory->Object->TClass != obj)))) {
                        Point2D qp(x + QUEUE_COUNT_X_OFFSET, y + TEXT_Y_OFFSET);
                        Draw_Queue_Count(surface, rect, qp, total);
                        has_queue_count = true;
                    }
                }
            }

            /**
             *  Draw production overlay: clock, ready text, hold text.
             */
            if (production) {
                if (state != nullptr) {
                    Point2D sp(x + TEXT_X_OFFSET, y + TEXT_Y_OFFSET);
                    Draw_Ready_Text(surface, rect, sp, state, 0);
                }

                if (!completed) {
                    Point2D cp(x, y);
                    if (isready) {
                        Draw_Recharge_Clock(surface, *SidebarDrawer, rect, cp, stage);
                    } else {
                        Draw_Clock_Overlay(surface, *SidebarDrawer, rect, cp, stage);
                    }

                    /**
                     *  Display "HOLD" text if production is paused.
                     */
                    if (factory != nullptr && item.Is_On_Hold()) {
                        Point2D hp(x, y + TEXT_Y_OFFSET);
                        Draw_Hold_Text(surface, rect, hp, OBJECT_WIDTH, has_queue_count);
                    }
                }
            }
        }
    }
}
