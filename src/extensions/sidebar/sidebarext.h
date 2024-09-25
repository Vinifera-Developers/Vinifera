/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAREXT.H
 *
 *  @author        ZivDero
 *
 *  @brief         Extended SidebarClass class.
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

#include "always.h"
#include "extension.h"
#include "mouse.h"
#include "sidebar.h"
#include "vinifera_globals.h"


class SidebarClassExtension final : public GlobalExtensionClass<SidebarClass>
{
public:
    enum SidebarTabType
    {
        SIDEBAR_TAB_STRUCTURE,
        SIDEBAR_TAB_INFANTRY,
        SIDEBAR_TAB_UNIT,
        SIDEBAR_TAB_SPECIAL,

        SIDEBAR_TAB_COUNT,
        SIDEBAR_TAB_NONE = -1
    };

    enum SidebarExtGeneralEnums
    {
        COLUMN_Y = 54,
        BUTTON_REPAIR_X_OFFSET = 36,
        UP_X_OFFSET = 1,				                            // Scroll up arrow coordinates.
        UP_Y_OFFSET = COLUMN_Y - 1,
        DOWN_X_OFFSET = UP_X_OFFSET,				                // Scroll down arrow coordinates.
        DOWN_Y_OFFSET = UP_Y_OFFSET,
        TAB_Y_OFFSET = 24,
        TAB_ONE_X_OFFSET = 20,
        TAB_TWO_X_OFFSET = TAB_ONE_X_OFFSET + 35,
        TAB_THREE_X_OFFSET = TAB_TWO_X_OFFSET + 35,
        TAB_FOUR_X_OFFSET = TAB_THREE_X_OFFSET + 35,
    };

    enum ButtonNumberType {
        BUTTON_TAB_1 = 115,
        BUTTON_TAB_2,
        BUTTON_TAB_3,
        BUTTON_TAB_4
    };

    /**
     *  New class for the tab buttons.
     */
    class TabButtonClass : public ControlClass
    {
    private:
        enum
        {
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
        TabButtonClass(unsigned id, const ShapeFileStruct* shapes, int x, int y, ConvertClass* drawer = SidebarDrawer, int w = 0, int h = 0);
        virtual ~TabButtonClass() override = default;

        virtual bool Action(unsigned flags, KeyNumType& key) override;
        virtual void Disable() override;
        virtual void Enable() override;
        virtual bool Draw_Me(bool forced = false) override;
        virtual void Set_Shape(const ShapeFileStruct* data, int width = 0, int height = 0);
        virtual void On_Mouse_Enter();
        virtual void On_Mouse_Leave();

        const ShapeFileStruct* Get_Shape_Data() const { return ShapeData; }
        void Start_Flashing();
        void Stop_Flashing();
        void Select();
        void Deselect();

    public:
        /**
         *  Graphics
         */
        int DrawX;
        int DrawY;
        ConvertClass* ShapeDrawer;
        const ShapeFileStruct* ShapeData;

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

        bool MousedOver = false;
    };


    /**
     *  An extended SelectClass to support hover effects.
     */
    class ViniferaSelectClass : public SidebarClass::StripClass::SelectClass
    {
    public:
        ViniferaSelectClass() = default;
        ViniferaSelectClass(const NoInitClass& x) : SelectClass(x) {}

        virtual void On_Mouse_Enter();
        virtual void On_Mouse_Leave();

    public:
        bool MousedOver = false;
    };

public:
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

public:
        SidebarClassExtension(const SidebarClass *this_ptr);
        SidebarClassExtension(const NoInitClass &noinit);
        virtual ~SidebarClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual const char *Name() const override { return "Sidebar"; }
        virtual const char *Full_Name() const override { return "Sidebar"; }

        void Init_Strips();
        void Init_IO();
        void Init_For_House();
        void Set_Dimensions();
        bool Change_Tab(SidebarTabType index);

        SidebarClass::StripClass& Current_Tab() { return Column[TabIndex];}
        SidebarClass::StripClass& Get_Tab(RTTIType type) { return Column[Which_Tab(type)]; }
        SidebarTabType First_Active_Tab();

        static SidebarTabType Which_Tab(RTTIType type);

        bool Is_On_Sidebar(RTTIType type, int id) const
        {
            const int column = Which_Tab(type);
            return Column[column].Is_On_Sidebar(type, id);
        }

        void Flag_Strip_To_Redraw(RTTIType type)
        {
            if (Vinifera_NewSidebar)
                Get_Tab(type).Flag_To_Redraw();
            else
                Map.Column[Map.Which_Column(type)].Flag_To_Redraw();
        }

        static int Max_Visible(bool one_strip = false)
        {
            if (SidebarSurface && SidebarClass::SidebarShape)
            {
                int total = (SidebarRect.Height - SidebarClass::SidebarBottomShape->Get_Height() - SidebarClass::SidebarShape->Get_Height()) /
                    SidebarClass::SidebarMiddleShape->Get_Height();

                if (one_strip)
                    return total;
                
                return total * 2;
            }
            else
            {
                return SidebarClass::StripClass::MAX_VISIBLE;
            }
        }

    public:
        /**
         *  Index of the current sidebar tab.
         */
        SidebarTabType TabIndex;

        /**
         *  Replacement strips.
         */
        SidebarClass::StripClass Column[SIDEBAR_TAB_COUNT];

        /**
         *  Replacement select buttons.
         */
        ViniferaSelectClass SelectButton[SIDEBAR_TAB_COUNT][SidebarClass::StripClass::MAX_BUILDABLES];

        /**
         *  Buttons for the tabs.
         */
        TabButtonClass TabButtons[SIDEBAR_TAB_COUNT];

        /**
         *  Reference to last gadget that the user has hovered their mouse cursor on.
         */
        static GadgetClass* LastHovered;

        /**
         *  Function for checking which gadget has been hovered over.
         */
        static void Check_Hover(GadgetClass* gadget, int mousex, int mousey);
};
