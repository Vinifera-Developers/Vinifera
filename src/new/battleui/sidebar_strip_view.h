/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_STRIP_VIEW.H
 *
 *  @author        ZivDero
 *
 *  @brief         Shared scrollable sidebar column view. Displays one
 *                 BuildCategory worth of items with scroll buttons, select
 *                 buttons, and hover tracking. Used by both Classic (×2)
 *                 and Tabbed (×4) sidebar views.
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

#pragma once

#include "shapebtn.h"
#include "stage.h"
#include "vector.h"
#include "wwkeyboard.h"

#include "point.h"
#include "rect.h"

#include <climits>

class BuildCategory;
struct BuildItem;
class CameoButtonClass;
class FactoryClass;
class Surface;
class ShapeSet;
class TechnoTypeClass;


/**
 *  A scrollable column of cameo icons. Reads items from a BuildCategory
 *  reference and renders them directly. Owns its own scroll buttons,
 *  select buttons, and scroll animation state.
 */
class SidebarStripView : public StageClass
{
public:
    /**
     *  Shared strip art handles and derived background metrics.
     */
    struct StripArt
    {
        StripArt() :
            ScrollUpButtonShape(nullptr),
            ScrollDownButtonShape(nullptr),
            DarkenShape(nullptr),
            ClockShape(nullptr),
            RechargeClockShape(nullptr),
            BackgroundTopHeight(0),
            BackgroundBottomHeight(0)
        {
        }

        const ShapeSet* ScrollUpButtonShape;
        const ShapeSet* ScrollDownButtonShape;
        const ShapeSet* DarkenShape;
        const ShapeSet* ClockShape;
        const ShapeSet* RechargeClockShape;
        int BackgroundTopHeight;
        int BackgroundBottomHeight;
    };

    /**
     *  Per-strip layout and positioning values.
     */
    struct StripLayout
    {
        static constexpr int AUTO_POSITION = INT_MIN;

        StripLayout() :
            Position(0, 0),
            VisibleRows(0),
            RowPitch(51),
            ColumnSpacing(66),
            CameoSize(64, 51),
            CameoNameOffset(41),
            CameoTextOffset(30, 2),
            QueueCountOffset(60, 2),
            UpButtonPosition(AUTO_POSITION, AUTO_POSITION),
            DownButtonPosition(AUTO_POSITION, AUTO_POSITION),
            IsUpButtonVisible(true),
            IsDownButtonVisible(true)
        {
        }

        TPoint2D<int> Position;
        int VisibleRows;
        int RowPitch;
        int ColumnSpacing;
        TPoint2D<int> CameoSize;
        int CameoNameOffset;
        TPoint2D<int> CameoTextOffset;
        TPoint2D<int> QueueCountOffset;
        TPoint2D<int> UpButtonPosition;
        TPoint2D<int> DownButtonPosition;
        bool IsUpButtonVisible;
        bool IsDownButtonVisible;
    };

    enum StripEnums {
        BUTTON_UP = 200,
        BUTTON_DOWN = 210,
        BUTTON_SELECT = 220,
        SCROLL_RATE = 51,
    };

    /**
     *  Lifecycle and setup.
     */
    SidebarStripView();
    ~SidebarStripView();

    void One_Time(int id);
    void Init_Clear();
    void Init_IO(int id, int columns = 2);
    void Init_For_House();
    void Set_Layout(const StripLayout& layout);
    void Set_Art(const StripArt& art);
    void Shift_Sidebar();
    void Activate();
    void Deactivate();

    /**
     *  Runtime behavior.
     */
    bool AI(KeyNumType& input, Point2D& xy);
    bool Update_State();
    void Draw(Surface& surface, const Rect& rect);
    bool Scroll(bool up);
    bool Scroll_Page(bool up);
    void Flag_To_Redraw();

    /**
     *  Category access.
     */
    void Set_Category(BuildCategory* cat) { Category = cat; }
    BuildCategory* Get_Category() const { return Category; }
    BuildItem* Get_Visible_Item(int slot);
    const BuildItem* Get_Visible_Item(int slot) const;

    /**
     *  Visibility queries.
     */
    bool Has_Ready() const;
    int Visible_Button_Count() const;
    int Visible_Buttons_Per_Column() const;

    int Get_ID() const { return ID; }
    bool Is_Scrolling() const { return IsScrolling; }

public:
    /**
     *  Public strip state retained for existing view integration.
     */
    int ID;
    int Columns;
    int ColumnX;
    int ColumnY;
    bool IsToRedraw;
    bool IsActive;
    bool IsScrollingDown;
    bool IsScrolling;
    int TopIndex;
    int Scroller;
    int Slid;
    int LastSlid;
    int MaxVisibleCount;

    BuildCategory* Category;
    StripLayout Layout;
    StripArt Art;

    ShapeButtonClass UpButton;
    ShapeButtonClass DownButton;
    DynamicVectorClass<CameoButtonClass*> SelectButtons;

private:
    /**
     *  Transient resolved draw state for one visible item.
     */
    struct StripItemDrawState
    {
        StripItemDrawState() :
            Factory(nullptr),
            Name(nullptr),
            StateText(nullptr),
            Production(false),
            Completed(false),
            Darken(false),
            IsReady(false),
            IsOnHold(false),
            Stage(0),
            QueueCount(0)
        {
        }

        FactoryClass* Factory;
        const char* Name;
        const char* StateText;
        bool Production;
        bool Completed;
        bool Darken;
        bool IsReady;
        bool IsOnHold;
        int Stage;
        int QueueCount;
    };

    /**
     *  Layout helpers.
     */
    int Available_Content_Height() const;
    int Object_Width() const;
    int Object_Height() const;
    int Object_Name_Offset() const;
    int Row_Pitch() const;
    int Column_Spacing() const;
    int Scroll_Step() const;
    Point2D Text_Offset() const;
    Point2D Queue_Count_Offset() const;
    Point2D Get_Up_Button_Position() const;
    Point2D Get_Down_Button_Position() const;

    /**
     *  Item state helpers.
     */
    Point2D Get_Item_Point(int slot) const;
    StripItemDrawState Get_Item_Draw_State(const BuildItem& item) const;
    int Get_Item_Queue_Count(const TechnoTypeClass& object) const;

    /**
     *  Draw primitives and per-item rendering.
     */
    void Draw_Shape_Overlay(Surface& surface, const ShapeSet* shape, const Rect& rect, const Point2D& point, int frame, int flags);
    void Draw_Cameo(Surface& surface, const Rect& rect, const BuildItem& item, const Point2D& point);
    void Draw_Clock_Overlay(Surface& surface, const Rect& rect, const Point2D& point, int stage);
    void Draw_Recharge_Clock(Surface& surface, const Rect& rect, const Point2D& point, int stage);
    void Draw_Darken_Overlay(Surface& surface, const Rect& rect, const Point2D& point);
    void Draw_Ready_Text(Surface& surface, const Rect& rect, const Point2D& point, const char* text);
    void Draw_Hold_Text(Surface& surface, const Rect& rect, const Point2D& point, bool has_queue_count);
    void Draw_Queue_Count(Surface& surface, const Rect& rect, const Point2D& point, int count);
    void Draw_Hover_Highlight(Surface& surface, const Rect& cameo_rect);
    void Draw_Cameo_Name(const Rect& rect, const Point2D& point, const char* name);
    void Allocate_Select_Buttons(int count);
    void Draw_Item(Surface& surface, const Rect& rect, int slot);
    void Draw_Item_Production(Surface& surface, const Rect& rect, const Point2D& drawpoint, const StripItemDrawState& state);
    void Draw_Strip_Items(Surface& surface, const Rect& rect);
};
