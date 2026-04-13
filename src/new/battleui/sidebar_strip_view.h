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
class Surface;


/**
 *  A scrollable column of cameo icons. Reads items from a BuildCategory
 *  reference and renders them using the shared render utilities. Owns
 *  its own scroll buttons, select buttons, and scroll animation state.
 */
class SidebarStripView : public StageClass
{
public:
    struct StripLayout
    {
        static constexpr int AUTO_POSITION = INT_MIN;

        StripLayout() :
            Position(0, 0),
            VisibleRows(0),
            RowPitch(51),
            ColumnSpacing(67),
            UpButtonPosition(AUTO_POSITION, AUTO_POSITION),
            DownButtonPosition(AUTO_POSITION, AUTO_POSITION),
            UpButtonVisible(true),
            DownButtonVisible(true)
        {
        }

        TPoint2D<int> Position;
        int VisibleRows;
        int RowPitch;
        int ColumnSpacing;
        TPoint2D<int> UpButtonPosition;
        TPoint2D<int> DownButtonPosition;
        bool UpButtonVisible;
        bool DownButtonVisible;
    };

    enum StripEnums {
        BUTTON_UP = 200,
        BUTTON_DOWN = 210,
        BUTTON_SELECT = 220,

        OBJECT_HEIGHT = 51,
        OBJECT_WIDTH = 64,
        OBJECT_NAME_OFFSET = 41,
        COLUMN_SPACING = 67,
        SCROLL_RATE = 51,

        TEXT_X_OFFSET = 30,
        TEXT_Y_OFFSET = 2,
        QUEUE_COUNT_X_OFFSET = 60,
    };

    SidebarStripView();
    ~SidebarStripView();

    void One_Time(int id);
    void Init_Clear();
    void Init_IO(int id, int columns = 2);
    void Init_For_House(int id);
    void Set_Layout(const StripLayout& layout);
    void Set_Dimensions();
    void Activate();
    void Deactivate();
    bool AI(KeyNumType& input, Point2D& xy);
    bool Update_State();
    void Draw(Surface& surface, const Rect& rect, bool complete);
    bool Scroll(bool up);
    bool Scroll_Page(bool up);
    void Flag_To_Redraw();

    void Set_Category(BuildCategory* cat) { Category = cat; }
    BuildCategory* Get_Category() const { return Category; }
    BuildItem* Get_Visible_Item(int slot);
    const BuildItem* Get_Visible_Item(int slot) const;

    bool Has_Ready() const;
    int Visible_Button_Count() const;
    int Visible_Buttons_Per_Column() const;

    int Get_ID() const { return ID; }
    bool Is_Scrolling() const { return IsScrolling; }

public:
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

    ShapeButtonClass UpButton;
    ShapeButtonClass DownButton;
    DynamicVectorClass<CameoButtonClass*> SelectButtons;

private:
    int Available_Content_Height() const;
    int Effective_Row_Pitch() const;
    int Effective_Column_Spacing() const;
    int Scroll_Step() const;
    Point2D Resolve_Up_Button_Position() const;
    Point2D Resolve_Down_Button_Position() const;
    void Allocate_Select_Buttons(int count);
    void Draw_Strip_Items(Surface& surface, const Rect& rect);
};
