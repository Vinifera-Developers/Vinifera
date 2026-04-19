/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_TABBED_VIEW.H
 *
 *  @author        ZivDero
 *
 *  @brief         Tabbed sidebar view. Four tabs (Structure / Infantry /
 *                 Unit / Special) with a single 2-column strip per tab.
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

#include "gcntrl.h"
#include "ihoverable_gadget.h"
#include "shapeset.h"
#include "sidebar.h"
#include "sidebar_strip_view.h"
#include "isidebar_view.h"
#include "ttimer.h"


/**
 *  A tab button for switching between sidebar categories.
 */
class TabButtonClass : public ControlClass, public IHoverableGadget
{
private:
    enum {
        FRAME_NORMAL,
        FRAME_SELECTED,
        FRAME_DISABLED,

        FLASH_TIME = 60,
        FLASH_FRAME_COUNT = 2,
        FLASH_FRAME_MIN = FRAME_DISABLED + 1,
        FLASH_FRAME_MAX = FLASH_FRAME_MIN + (FLASH_FRAME_COUNT - 1),
        FLASH_FRAME_START = FLASH_FRAME_MIN,
        FLASH_RATE = FLASH_TIME / FLASH_FRAME_COUNT,
    };

public:
    TabButtonClass();
    TabButtonClass(unsigned id, const ShapeSet* shapes, int x, int y,
                   ConvertClass* drawer = nullptr, int w = 0, int h = 0);
    ~TabButtonClass() override = default;

    /**
     *  ControlClass and hover behavior.
     */
    bool Action(unsigned flags, KeyNumType& key) override;
    void Disable() override;
    void Enable() override;
    bool Draw_Me(bool forced = false) override;

    void On_Mouse_Enter() override;
    void On_Mouse_Leave() override;

    /**
     *  Tab button state transitions.
     */
    void Set_Shape(const ShapeSet* data, int width = 0, int height = 0);
    const ShapeSet* Get_Shape_Data() const { return ShapeData; }

    void Start_Flashing();
    void Stop_Flashing();
    void Select();
    void Deselect();

public:
    /**
     *  Public button state retained for existing sidebar integration.
     */
    int DrawX;
    int DrawY;
    ConvertClass* ShapeDrawer;
    const ShapeSet* ShapeData;

    bool IsFlashing;
    CDTimerClass<SystemTimerClass> FlashTimer;
    int FlashFrame;

    bool IsSelected;
    bool IsMousedOver;
};


/**
 *  Tabbed sidebar view — four production tabs with a shared 2-column
 *  cameo grid. Only the active tab's strip is displayed.
 */
class TabbedSidebarView : public ISidebarView
{
public:
    /**
     *  Sidebar tab identifiers.
     */
    enum SidebarTabType {
        SIDEBAR_TAB_STRUCTURE,
        SIDEBAR_TAB_INFANTRY,
        SIDEBAR_TAB_UNIT,
        SIDEBAR_TAB_SPECIAL,

        SIDEBAR_TAB_COUNT,
        SIDEBAR_TAB_NONE = -1,
    };

    enum TabbedEnums {
        BUTTON_TAB_1 = 115,
        BUTTON_TAB_2,
        BUTTON_TAB_3,
        BUTTON_TAB_4,
    };

    TabbedSidebarView(SidebarModel* model);
    virtual ~TabbedSidebarView() override = default;

    /**
     *  ISidebarView lifecycle and rendering.
     */
    virtual void One_Time() override;
    virtual void Init_Clear() override;
    virtual void Init_IO() override;
    virtual void Init_For_House() override;
    virtual void Shift_Sidebar() override;
    virtual void AI(KeyNumType& input, Point2D& xy) override;
    virtual void Draw() override;
    virtual void Blit(bool complete) override;
    virtual void Activate(int control) override;

    virtual bool Scroll(bool up, int column) override;
    virtual bool Scroll_Page(bool up, int column) override;

    virtual const char* Help_Text(int gadget_id) override;
    virtual int Visible_Button_Count() const override;
    virtual int Visible_Buttons_Per_Column() const override;
    virtual void Prepare_Model_Recalc() override;
    virtual void Finish_Model_Recalc() override;

    virtual bool Change_Tab(int index) override;
    virtual void Notify_Production_Complete(int category_index) override;

    /**
     *  Tab and strip queries.
     */
    SidebarTabType First_Active_Tab() const;

    SidebarStripView& Current_Strip() { return Strip[TabIndex]; }
    const SidebarStripView& Current_Strip() const { return Strip[TabIndex]; }

private:
    /**
     *  Internal helpers.
     */
    int Background_Row_Count() const;
    void Tab_Button_AI(int tab_index);

    /**
     *  Number of cameo tooltip IDs registered during the last layout pass.
     */
    int RegisteredTooltipCount;

    /**
     *  Currently selected sidebar tab.
     */
    SidebarTabType TabIndex;

    /**
     *  Per-tab strip views; only the active tab's strip is drawn and accepts input.
     */
    SidebarStripView Strip[SIDEBAR_TAB_COUNT];

    /**
     *  Per-tab category buttons used to switch the active strip.
     */
    TabButtonClass TabButtons[SIDEBAR_TAB_COUNT];

    /**
     *  Sidebar background gadget registered with the map while the view is active.
     */
    SidebarClass::SBGadgetClass Background;

    /**
     *  Shapes for the various sidebar graphics.
     */
    const ShapeSet* BackgroundTopShape;
    const ShapeSet* BackgroundMiddleShape;
    const ShapeSet* BackgroundBottomShape;
    const ShapeSet* BackgroundAddonShape;
    const ShapeSet* ClockShape;
    const ShapeSet* RechargeClockShape;
};
