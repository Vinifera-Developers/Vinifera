/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  UI controls and overrides.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "point.h"
#include "rgb.h"
#include "tibsun_defines.h"
#include "typelist.h"
#include <string>
#include <vector>

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

SidebarViewType Sidebar_View_From_Name(const char* name, SidebarViewType default_type);
const char* Name_From_Sidebar_View_Type(SidebarViewType view_type);


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
    int PowerBarWidth = 12;
    int PowerPipHeight = 4;
    int PowerBarHeightAdjust = 3;
    TPoint2D<int> CameoSize = {64, 51};
    int CameoNameOffset = 41;
    TPoint2D<int> CameoTextOffset = {30, 2};
    TPoint2D<int> QueueCountOffset = {60, 2};
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
    TPoint2D<int> RightStripPosition = {91, 26};
    int RowSpacing = 0;
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
    SidebarTabbedLayout()
    {
        PowerBarPosition = { 8, 53 };
        PowerBarHeightAdjust = 0;
    }
    void Read_INI(CCINIClass const& ini, const char* section) override;

    TPoint2D<int> TabButtonPosition[4] = { {20, 24}, {55, 24}, {90, 24}, {125, 24} };
    TPoint2D<int> StripPosition = {24, 54};
    int RowSpacing = 0;
    int ColumnSpacing = 2;
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
        UIControlsClass() = default;
        ~UIControlsClass() = default;

        bool Read_INI(CCINIClass const& ini);
        bool Read_INI_File(const char *filename, bool reset_to_defaults = false);

        const BattleSidebarLayoutBase& Get_Battle_Sidebar_Config(SidebarViewType view_type) const
        {
            return view_type == SIDEBAR_TABBED ? static_cast<const BattleSidebarLayoutBase&>(TabbedSidebarLayoutConfig)
                                               : static_cast<const BattleSidebarLayoutBase&>(ClassicSidebarLayoutConfig);
        }
        
        struct LoadingScreenSize {
            Point2D Size;
            Point2D SPPosition;
            Point2D MPPosition;
        };

        struct LoadingScreen {
            LoadingScreen() = default;
            LoadingScreen(char const* entry);
            LoadingScreen(HousesType house, std::string const& filename, LoadingScreenSize const& size) : IsValid(true), House(house), Filename(filename), Size(size) {};

            bool IsValid = false;
            HousesType House = HOUSE_NONE;
            std::string Filename;
            LoadingScreenSize Size {};
        };

    private:
        static const LoadingScreenSize DefaultLoadingScreenSizes[];
        static LoadingScreenSize const& Pick_Default_Loading_Screen_Size();

    public:
        TPoint2D<int> Get_Group_Number_Offset(RTTIType type, bool has_pip) const;
        TPoint2D<int> Get_Veterancy_Pip_Offset(RTTIType type) const;
        TPoint2D<int> Get_Special_Pip_Offset(RTTIType type) const;

        LoadingScreen Pick_Loading_Screen(HousesType house) const;

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
        TypeList<RGBClass> BandBoxTintColors = { {0, 0, 0}, {255, 255, 255} };

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
        std::string BeaconPreviewText[7] = {"Expand", "Attack", "Move", "", "", "Defend", ""};

        /**
         *  Subtitle styling. Used by TacticalExtension::Draw_Subtitle to render
         *  the currently playing VOX's text at the bottom of the tactical view.
         */
        std::string SubtitleFontName = "Arial";
        int SubtitleFontHeight = 22;
        int SubtitleFontWeight = FW_BOLD;
        RGBStruct SubtitleTextColor = { 255, 255, 255 };
        RGBStruct SubtitleOutlineColor = { 0, 0, 0 };
        int SubtitleOutlineWidth = 2;
        int SubtitleMarginX = 40;
        int SubtitleMarginBottom = 24;

        std::vector<LoadingScreen> LoadingScreens;
};

extern UIControlsClass* UIControls;
