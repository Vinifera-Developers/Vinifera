/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UICONTROL.H
 *
 *  @author        CCHyper
 *
 *  @brief         UI controls and overrides.
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

#include "point.h"
#include "rgb.h"
#include "tibsun_defines.h"
#include "typelist.h"

#include <climits>
#include <string>


struct IStream;
class CCINIClass;
class NoInitClass;


enum SidebarViewType {
    SIDEBAR_CLASSIC,
    SIDEBAR_TABBED,

    SIDEBAR_COUNT
};


struct SidebarButtonLayout
{
    TPoint2D<int> Position = {0, 0};
    bool IsVisible = true;
};


struct BattleSidebarLayoutBase
{
    virtual ~BattleSidebarLayoutBase() = default;
    virtual void Read_INI(CCINIClass const& ini, const char* section);

    SidebarButtonLayout RepairButton = { {31, -9} };
    SidebarButtonLayout SellButton = { {58, -9} };
    SidebarButtonLayout PowerButton = { {85, -9} };
    SidebarButtonLayout WaypointButton = { {112, -9} };
    TPoint2D<int> PowerBarPosition = {8, 25};
    std::string SidebarShape = "SIDE1.SHP";
    std::string SidebarMiddleShape = "SIDE2.SHP";
    std::string SidebarBottomShape = "SIDE3.SHP";
    std::string SidebarAddonShape = "ADDON.SHP";
    std::string ClockShape = "GCLOCK2.SHP";
    std::string RechargeClockShape = "RCLOCK2.SHP";
    std::string DarkenShape = "DARKEN.SHP";
    std::string ScrollUpButtonShape = "R-UP.SHP";
    std::string ScrollDownButtonShape = "R-DN.SHP";
    std::string RepairButtonShape = "REPAIR.SHP";
    std::string SellButtonShape = "SELL.SHP";
    std::string PowerButtonShape = "POWER.SHP";
    std::string WaypointButtonShape = "WAYP.SHP";
    std::string PowerPipShape = "POWERP.SHP";
};


struct SidebarClassicLayout : BattleSidebarLayoutBase
{
    void Read_INI(CCINIClass const& ini, const char* section) override;

    TPoint2D<int> LeftStripPosition = {24, 26};
    TPoint2D<int> RightStripPosition = {92, 26};
    int VisibleRows = 0;
    int RowPitch = 51;
    TPoint2D<int> LeftUpButtonPosition = {INT_MIN, INT_MIN};
    TPoint2D<int> LeftDownButtonPosition = {INT_MIN, INT_MIN};
    TPoint2D<int> RightUpButtonPosition = {INT_MIN, INT_MIN};
    TPoint2D<int> RightDownButtonPosition = {INT_MIN, INT_MIN};
    bool IsLeftUpButtonVisible = true;
    bool IsLeftDownButtonVisible = true;
    bool IsRightUpButtonVisible = true;
    bool IsRightDownButtonVisible = true;
};


struct SidebarTabbedLayout : BattleSidebarLayoutBase
{
    void Read_INI(CCINIClass const& ini, const char* section) override;

    TPoint2D<int> TabButtonPosition[4] = { {20, 24}, {55, 24}, {90, 24}, {125, 24} };
    TPoint2D<int> StripPosition = {24, 54};
    int VisibleRows = 0;
    int RowPitch = 51;
    int ColumnSpacing = 67;
    TPoint2D<int> UpButtonPosition = {INT_MIN, INT_MIN};
    TPoint2D<int> DownButtonPosition = {INT_MIN, INT_MIN};
    bool IsUpButtonVisible = true;
    bool IsDownButtonVisible = true;
    std::string StructureTabShape = "TAB-BLD.SHP";
    std::string InfantryTabShape = "TAB-INF.SHP";
    std::string UnitTabShape = "TAB-UNT.SHP";
    std::string SpecialTabShape = "TAB-SPC.SHP";
};


class UIControlsClass
{
    public:
        UIControlsClass();
        UIControlsClass(const NoInitClass &noinit);
        ~UIControlsClass();

        HRESULT Load(IStream *pStm);
        HRESULT Save(IStream *pStm, BOOL fClearDirty);
        int Get_Object_Size() const;

        bool Read_INI(CCINIClass const& ini);
        bool Read_INI_File(const char *filename, bool reset_to_defaults = false);

        const BattleSidebarLayoutBase& Get_Battle_Sidebar_Config(SidebarViewType view_type) const
        {
            return view_type == SIDEBAR_TABBED ? static_cast<const BattleSidebarLayoutBase&>(TabbedSidebarLayoutConfig)
                                               : static_cast<const BattleSidebarLayoutBase&>(ClassicSidebarLayoutConfig);
        }

        TPoint2D<int> Get_Group_Number_Offset(RTTIType type, bool has_pip) const;
        TPoint2D<int> Get_Veterancy_Pip_Offset(RTTIType type) const;
        TPoint2D<int> Get_Special_Pip_Offset(RTTIType type) const;

    public:
        /**
         *  Health bar draw positions.
         *  #issue-541: The health bar graphics "Y" position on selection boxes was off by 1 pixel.
         */
        TPoint2D<int> UnitHealthBarDrawPos = {-25, -16}; // Y was -15
        TPoint2D<int> InfantryHealthBarDrawPos = {-24, -5};

        /**
         *  Should the text label be drawn with an outline?
         */
        bool IsTextLabelOutline = true;

        /**
         *  Transparency of the text background.
         */
        unsigned TextLabelBackgroundTransparency = 50;

        /**
         *  Customizable offsets for drawing different pips.
         */
        TPoint2D<int> UnitGroupNumberOffset = {-4, -4};
        TPoint2D<int> InfantryGroupNumberOffset = {-4, -4};
        TPoint2D<int> BuildingGroupNumberOffset = {-4, -4};
        TPoint2D<int> AircraftGroupNumberOffset = {-4, -4};
        TPoint2D<int> UnitWithPipGroupNumberOffset = {-4, -8};
        TPoint2D<int> InfantryWithPipGroupNumberOffset = {-4, -8};
        TPoint2D<int> BuildingWithPipGroupNumberOffset = {-4, -8};
        TPoint2D<int> AircraftWithPipGroupNumberOffset = {-4, -8};
        TPoint2D<int> UnitVeterancyPipOffset = {10, 6};
        TPoint2D<int> InfantryVeterancyPipOffset = {5, 2};
        TPoint2D<int> BuildingVeterancyPipOffset = {10, 6};
        TPoint2D<int> AircraftVeterancyPipOffset = {10, 6};
        TPoint2D<int> UnitSpecialPipOffset = {0, -8};
        TPoint2D<int> InfantrySpecialPipOffset = {0, -8};
        TPoint2D<int> BuildingSpecialPipOffset = {0, -8};
        TPoint2D<int> AircraftSpecialPipOffset = {0, -8};

        /**
         *  Should the tactical rubber band box be drawn with a drop shadow?
         */
        bool IsBandBoxDropShadow = false;

        /**
         *  Should the tactical rubber band box be drawn with a thick border?
         */
        bool IsBandBoxThick = false;

        /**
         *  Color to draw the tactical rubber band box with.
         */
        RGBStruct BandBoxColor = { 255, 255, 255 };

        /**
         *  Color to draw the tactical rubber band box's shadow with.
         */
        RGBStruct BandBoxDropShadowColor = { 0, 0, 0 };

        /**
         *  Transparency of the tactical rubber band.
         */
        unsigned BandBoxTintTransparency = 0;

        /**
         *  Two tint colors, interpolated between based on the current ambient light level.
         */
        TypeList<RGBClass> BandBoxTintColors;

        /**
         *  Should action lines remain visible continuously, instead of disappearing after some time?
         */
        bool IsAlwaysShowActionLines = false;

        /**
         *  Should movement lines be drawn with dashes?
         */
        bool IsMovementLineDashed = false;

        /**
         *  Should movement lines be drawn with a drop shadow?
         */
        bool IsMovementLineDropShadow = false;

        /**
         *  Should movement lines be drawn with a thick line?
         */
        bool IsMovementLineThick = false;

        /**
         *  Color to draw movement lines with.
         */
        RGBStruct MovementLineColor = { 0, 170, 0 }; // COLOR_GREEN

        /**
         *  Color to draw movement lines' drop shadow with.
         */
        RGBStruct MovementLineDropShadowColor = { 0, 0, 0 };

        /**
         *  Should target lines be drawn with dashes?
         */
        bool IsTargetLineDashed = false;

        /**
         *  Should target lines be drawn with a drop shadow?
         */
        bool IsTargetLineDropShadow = false;

        /**
         *  Should target lines be drawn with a thick line?
         */
        bool IsTargetLineThick = false;

        /**
         *  Color to target movement lines with.
         */
        RGBStruct TargetLineColor = { 173, 0, 0 }; // COLOR_RED

        /**
         *  Color to draw target lines' drop shadow with.
         */
        RGBStruct TargetLineDropShadowColor = { 0, 0, 0 };

        /**
         *  Should target laser be drawn with dashes?
         */
        bool IsTargetLaserDashed = true;

        /**
         *  Should target laser be drawn with a drop shadow?
         */
        bool IsTargetLaserDropShadow = false;

        /**
         *  Should target laser be drawn with a thick line?
         */
        bool IsTargetLaserThick = false;

        /**
         *  Color to draw the target laser with.
         */
        RGBStruct TargetLaserColor = { 173, 0, 0 }; // COLOR_RED

        /**
         *  Color to draw the target laser's drop shadow with.
         */
        RGBStruct TargetLaserDropShadowColor = { 0, 0, 0 };

        /**
         *  Time in frames the target laser should be drawn for when the unit fires.
         */
        unsigned TargetLaserTime = 15;

        /**
         *  Should NavCom queue lines be displayed?
         */
        bool IsShowNavComQueueLines = true;

        /**
         *  Should NavCom queue lines be drawn with dashes?
         */
        bool IsNavComQueueLineDashed = false;

        /**
         *  Should NavCom queue lines be drawn with a drop shadow?
         */
        bool IsNavComQueueLineDropShadow = false;

        /**
         *  Should NavCom queue lines be drawn with a thick line?
         */
        bool IsNavComQueueLineThick = false;

        /**
         *  Color to draw the NavCom queue lines with.
         */
        RGBStruct NavComQueueLineColor = { 74, 77, 255 }; // COLOR_LTBLUE

        /**
         *  Color to draw the NavCom queue lines' drop shadow with.
         */
        RGBStruct NavComQueueLineDropShadowColor = { 0, 0, 0 };

        SidebarClassicLayout ClassicSidebarLayoutConfig;
        SidebarTabbedLayout TabbedSidebarLayoutConfig;

        /**
         *  Selected sidebar layout for the battle UI.
         */
        SidebarViewType BattleSidebarViewType = SIDEBAR_CLASSIC;

        /**
         *  Beacon animations are not tied to game FPS, this is the FPS at which they play.
         */
        int BeaconAnimFramesPerSecond = 25;
        int RadarBeaconAnimFramesPerSecond = 25;

        /**
         *  The offset at which beacon text is drawn (on actual beacons, and during preview).
         */
        int BeaconTextOffset = 32;
        int BeaconPreviewTextOffset = 20;

        /**
         *  Text presets for beacons when holding modifier keys when placing.
         */
        std::string BeaconText[7] = { "Expand", "Attack", "Move", "", "", "Defend", "" };

        /**
         *  Text presets for beacons *shown as a tooltip* when holding modifier keys when placing.
         */
        std::string BeaconPreviewText[7] = { "Expand", "Attack", "Move", "", "", "Defend", "" };
};

extern UIControlsClass *UIControls;
