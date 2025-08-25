/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NEWSIDEBAR.H
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
#pragma once

#include "gcntrl.h"
#include "point.h"
#include "sidebar.h"
#include "tibsun_defines.h"
#include "vinifera_defines.h"
#include "wwkeyboard.h"

#include <unknwn.h>


class ShapeSet;
class FactoryClass;


class IHoverableGadget
{
public:
    IHoverableGadget() = default;
    virtual ~IHoverableGadget() = default;

    virtual void On_Mouse_Enter() = 0;
    virtual void On_Mouse_Leave() = 0;
    bool Is_Moused_Over() const { return MousedOver; }

protected:

    /**
     *  Is this gadget currently hovered over.
     */
    bool MousedOver = false;

    /**
     *  Reference to last gadget that the user has hovered their mouse cursor on.
     */
    static GadgetClass* LastHovered;

public:

    /**
     *  Function for checking which gadget has been hovered over.
     */
    static void Process_Hover(GadgetClass* gadget, int mousex, int mousey);
};



class NewSidebarClass {
public:
    NewSidebarClass();
    NewSidebarClass(NoInitClass const& x);
    ~NewSidebarClass() = default;

    HRESULT STDMETHODCALLTYPE Load(IStream* pStm);
    HRESULT STDMETHODCALLTYPE Save(IStream* pStm);

    void Init_Clear(); // Clears all to known state
    void Init_IO();    // Inits button list
    void Init_For_House();
    void Toggle_Cameo_Text(bool on);

    void AI(KeyNumType& input, Point2D const& xy);
    void Draw_It(bool complete);
    void Set_Dimensions();
    virtual char const* Help_Text(int id);

    bool Abandon_Production(RTTIType type, FactoryClass* factory, ProductionFlags flags);
    bool Activate(int control);
    bool Add(RTTIType type, int ID);
    void Recalc();
    bool Factory_Link(FactoryClass* factory, RTTIType type, int id);
    bool Change_Tab(int index);
    int First_Active_Tab() const;
    bool Is_On_Sidebar(RTTIType type, int id) const;

    static int Max_Visible();

    class StripClass
    {
    public:
        class SelectClass : public ControlClass, public IHoverableGadget
        {
        public:
            SelectClass();
            SelectClass(NoInitClass const& x) : ControlClass(x) {};

            void Set_Owner(StripClass& strip, int index);

            StripClass* Strip;
            int Index;

        protected:
            virtual int Action(unsigned flags, KeyNumType& key) override;
            virtual void On_Mouse_Enter() override;
            virtual void On_Mouse_Leave() override;
        };

        StripClass() = default;
        StripClass(int id, Point2D origin, int columns);
        StripClass(NoInitClass const&) {};

        bool Add(RTTIType type, int ID);
        bool Abandon_Production(FactoryClass const* factory);
        bool Scroll(bool up);
        bool Page(bool up);
        bool AI(KeyNumType& input);
        char const* Help_Text(int id) const;
        void Draw_It(bool complete);
        static void One_Time();
        void Init_Clear();
        void Init_IO();
        void Init_For_House();
        bool Recalc();
        void Activate();
        void Deactivate();
        void Flag_To_Redraw();
        void Set_Dimensions();
        bool Factory_Link(FactoryClass* factory, RTTIType type, int id);
        void Tab_AI();
        bool Is_On_Sidebar(RTTIType type, int id) const;

        /*
        **  This is the coordinate of the upper left corner that this side strip
        **  uses for rendering.
        */
        Point2D Position;

        int Columns;

        /*
        **  This is a unique identifier for the sidebar strip. Using this identifier,
        **  it is possible to differentiate the button messages that arrive from the
        **  common input button list.  It >MUST< be equal to the strip's index into
        **  the Column[] array, because the strip uses it to access the stripclass
        **  buttons.
        */
        int ID;

        bool IsActive;

        /*
        **  If this particular side strip needs to be redrawn, then this flag
        **  will be true.
        */
        bool IsToRedraw;

        /*
        **  If construction is in progress (no other objects in this strip can
        **  be started), then this flag will be true. It will be cleared when
        **  the strip is free to start production again.
        */
        bool IsBuilding;

        /*
        **  This controls the sidebar slide direction. If this is true, then the sidebar
        **  will scroll downward -- revealing previous objects.
        */
        bool IsScrollingDown;

        /*
        **  If the sidebar is scrolling, then this flag is true. Otherwise it is false.
        */
        bool IsScrolling;

        /*
        **  As the sidebar scrolls up and down, this variable holds the index for the topmost
        **  visible sidebar slot.
        */
        int TopIndex;

        /*
        **  This is the queued scroll direction and amount. The sidebar
        **  will scroll the number of slots indicated by this value. This
        **  value is set according to the scroll buttons.
        */
        int Scroller;

        /*
        **  The sidebar has smooth scrolling. This is the number of pixels the sidebar
        **  has slide down. Thus, if this value were 5, then there would be 5 pixels of
        **  the TopIndex-1 sidebar object visible. When the Slid value reaches 24, then
        **  the value resets to zero and the TopIndex is decremented. For sliding in the
        **  opposite direction, change the IsScrollingDown flag.
        */
        int Slid;

        /*
        ** The value of Slid the last time we rendered the sidebar.
        */
        int LastSlid;

        /*
        **  This is the array of buildable object types. This array is sorted in the order
        **  that it is to be displayed. This array keeps track of which objects are building
        **  and ready to be placed. The very nature of this method precludes simultaneous
        **  construction of the same object type.
        */
        typedef struct BuildType {
            BuildType() {}
            BuildType(int id, RTTIType type, FactoryClass* factory = NULL) : BuildableID(id), BuildableType(type), Factory(factory) {}

            void On_Left_Press(unsigned& flags);
            void On_Right_Press(unsigned& flags);

            void Draw_It(Point2D const& position) const;
            bool Is_Darkened() const;
            bool Is_Clock() const;
            bool Is_Completed() const;
            bool Is_Ready() const;
            int Clock_Stage() const;
            char const* Cameo_Text() const;
            char const* Help_Text() const;
            char const* State_Text() const;

            bool operator==(const BuildType& other) const { return (BuildableID == other.BuildableID && BuildableType == other.BuildableType); }
            bool operator!=(const BuildType& other) const { return (BuildableID != other.BuildableID || BuildableType != other.BuildableType); }

            int BuildableID;
            RTTIType BuildableType;
            FactoryClass* Factory; // Production manager.
        } BuildType;
        std::vector<BuildType> Buildables;

        /*
        **  Pointer to the shape data for small versions of the logos. These are used as
        **  placeholder pieces on the side bar.
        */
        static ShapeSet const* LogoShapes;

        /*
        **  This points to the animation sequence of frames used to mark the passage of time
        **  as an object is undergoing construction.
        */
        static ShapeSet const* ClockShapes;
        static ShapeSet const* RechargeClockShapes;
        static ShapeSet const* DarkenShapes;

        ShapeButtonClass UpButton;
        ShapeButtonClass DownButton;

        std::vector<SelectClass> SelectButton;

        /**
         *  New class for the tab buttons.
         */
        class TabButtonClass : public ControlClass, IHoverableGadget
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
                FLASH_RATE = FLASH_TIME / FLASH_FRAME_COUNT
            };

        public:
            TabButtonClass();
            TabButtonClass(unsigned id, const ShapeSet* shapes, int x, int y, ConvertClass* drawer = SidebarDrawer, int w = 0, int h = 0);
            virtual ~TabButtonClass() override = default;

            virtual int Action(unsigned flags, KeyNumType& key) override;
            virtual void Disable() override;
            virtual void Enable() override;
            virtual bool Draw_Me(bool forced = false) override;
            virtual void On_Mouse_Enter() override;
            virtual void On_Mouse_Leave() override;
            virtual void Set_Shape(const ShapeSet* data, int width = 0, int height = 0);

            const ShapeSet* Get_Shape_Data() const { return ShapeData; }
            void Start_Flashing();
            void Stop_Flashing();
            void Select();
            void Deselect();

        public:
            /**
             *  Graphics
             */
            int DrawOffsetX;
            int DrawOffsetY;
            ConvertClass* ShapeDrawer;
            const ShapeSet* ShapeData;

            /**
             *  Flashing
             */
            bool IsFlashing;
            CDTimerClass<SystemTimerClass> FlashTimer;
            int FlashFrame;

            /**
             *  State
             */
            bool IsSelected;
            bool IsDrawn;
        };

        TabButtonClass TabButton;
    };

    std::vector<StripClass> Column;

    //bool IsCameoText;

    /*
    **  If the sidebar is active then this flag is true.
    */
    //bool IsSidebarActive;

    /*
    **  This flag tells the rendering system that the sidebar needs to be redrawn.
    */
    //bool IsToRedraw;

    //bool FullRedraw;
    //bool field_1CD8;

    int CurrentTab;
    StripClass& Current_Tab() { return Column[CurrentTab]; }

    bool Scroll(bool up, int column);
    bool Page(bool up, int column);


    //bool Activate_Repair(int control);
    //bool Activate_Upgrade(int control);
    //bool Activate_Demolish(int control);
    static int Which_Column(RTTIType type, ProductionFlags flags);
    StripClass& Get_Column(RTTIType type, ProductionFlags flags) { return Column[Which_Column(type, flags)]; }

private:
    //bool IsRepairActive;
    //bool IsUpgradeActive;
    //bool IsDemolishActive;

    class SBGadgetClass : public GadgetClass
    {
    public:
        SBGadgetClass(void);

    protected:
        virtual int Action(unsigned flags, KeyNumType& key);
    };

    /*
    **  This is the button that is used to collapse and expand the sidebar.
    **  These buttons must be available to derived classes, for Save/Load.
    */
    static ShapeButtonClass Repair;
    static ShapeButtonClass Sell;
    static ShapeButtonClass Power;
    static ShapeButtonClass Waypoint;
    static SBGadgetClass Background;

    static ShapeSet const* SidebarShape;
    static ShapeSet const* SidebarMiddleShape;
    static ShapeSet const* SidebarBottomShape;
    static ShapeSet const* SidebarAddonShape;

};
