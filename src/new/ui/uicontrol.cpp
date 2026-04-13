/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UICONTROL.CPP
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

#include "always.h"

#include "uicontrol.h"

#include "asserthandler.h"
#include "ccini.h"

#include <algorithm>
#include <cstring>


namespace
{
bool Has_INI_Entry(CCINIClass& ini, const char* section, const char* entry)
{
    char buffer[256];
    return ini.Get_String(section, entry, "", buffer, sizeof(buffer)) > 0;
}


void Read_Button_Layout(CCINIClass& ini, const char* section, const char* pos_key, const char* visible_key,
                        const SidebarButtonLayout& defaults, SidebarButtonLayout& layout)
{
    layout.Position = ini.Get_Point(section, pos_key, defaults.Position);
    layout.Visible = ini.Get_Bool(section, visible_key, defaults.Visible);
}


void Read_Optional_Point(CCINIClass& ini, const char* section, const char* entry,
                         const TPoint2D<int>& defaults, TPoint2D<int>& point, bool& has_custom_point)
{
    has_custom_point = Has_INI_Entry(ini, section, entry);
    point = has_custom_point ? ini.Get_Point(section, entry, defaults) : defaults;
}
}


UIControlsClass *UIControls = nullptr;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
UIControlsClass::UIControlsClass() :
    /**
     *  #issue-541
     * 
     *  The health bar graphics "Y" position on selection boxes is off by 1 pixel.
     * 
     *  @author: CCHyper
     */
    UnitHealthBarDrawPos(-25, -16), // Y was -15
    InfantryHealthBarDrawPos(-24, -5),
    IsTextLabelOutline(true),
    TextLabelBackgroundTransparency(50),
    UnitGroupNumberOffset(-4, -4),
    InfantryGroupNumberOffset(-4, -4),
    BuildingGroupNumberOffset(-4, -4),
    AircraftGroupNumberOffset(-4, -4),
    UnitWithPipGroupNumberOffset(-4, -8),
    InfantryWithPipGroupNumberOffset(-4, -8),
    BuildingWithPipGroupNumberOffset(-4, -8),
    AircraftWithPipGroupNumberOffset(-4, -8),
    UnitVeterancyPipOffset(10, 6),
    InfantryVeterancyPipOffset(5, 2),
    BuildingVeterancyPipOffset(10, 6),
    AircraftVeterancyPipOffset(10, 6),
    UnitSpecialPipOffset(0, -8),
    InfantrySpecialPipOffset(0, -8),
    BuildingSpecialPipOffset(0, -8),
    AircraftSpecialPipOffset(0, -8),
    IsBandBoxDropShadow(false),
    IsBandBoxThick(false),
    BandBoxColor{ 255, 255, 255 },
    BandBoxDropShadowColor{ 0, 0, 0 },
    BandBoxTintTransparency(0),
    BandBoxTintColors(),
    IsAlwaysShowActionLines(false),
    IsMovementLineDashed(false),
    IsMovementLineDropShadow(false),
    IsMovementLineThick(false),
    MovementLineColor{ 0, 170, 0 }, // COLOR_GREEN
    MovementLineDropShadowColor{ 0, 0, 0 },
    IsTargetLineDashed(false),
    IsTargetLineDropShadow(false),
    IsTargetLineThick(false),
    TargetLineColor{ 173, 0, 0 }, // COLOR_RED
    TargetLineDropShadowColor{ 0, 0, 0 },
    IsTargetLaserDashed(true),
    IsTargetLaserDropShadow(false),
    IsTargetLaserThick(false),
    TargetLaserColor{ 173, 0, 0 }, // COLOR_RED
    TargetLaserDropShadowColor{ 0, 0, 0 },
    TargetLaserTime(15),
    IsShowNavComQueueLines(true),
    IsNavComQueueLineDashed(false),
    IsNavComQueueLineDropShadow(false),
    IsNavComQueueLineThick(false),
    NavComQueueLineColor{ 74, 77, 255 }, // COLOR_LTBLUE
    NavComQueueLineDropShadowColor{ 0, 0, 0 },
    IsCenterSidebarButtonsOnRadar(false),
    SidebarLayout(),
    ClassicSidebarLayoutConfig(),
    TabbedSidebarLayoutConfig(),
    BattleSidebarViewType(SIDEBAR_CLASSIC),
    BeaconAnimFramesPerSecond(25),
    RadarBeaconAnimFramesPerSecond(25),
    BeaconTextOffset(32),
    BeaconPreviewTextOffset(20)
{
    BandBoxTintColors.Add(RGBStruct{ 0, 0, 0 });
    BandBoxTintColors.Add(RGBStruct{ 255, 255, 255 });

    BeaconText[0] = "Expand";
    BeaconText[1] = "Attack";
    BeaconText[2] = "Move";
    BeaconText[5] = "Defend";

    BeaconPreviewText[0] = "Expand";
    BeaconPreviewText[1] = "Attack";
    BeaconPreviewText[2] = "Move";
    BeaconPreviewText[5] = "Defend";
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
UIControlsClass::UIControlsClass(const NoInitClass &noinit) :
    BandBoxTintColors(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
UIControlsClass::~UIControlsClass()
{
}


/**
 *  Process the UI controls from INI.
 *  
 *  @author: CCHyper
 */
bool UIControlsClass::Read_INI(CCINIClass &ini)
{
    static char const * const INGAME = "Ingame";
    static char const * const SIDEBAR_SECTION = "Sidebar";
    static char const * const SIDEBAR_CLASSIC_SECTION = "SidebarClassic";
    static char const * const SIDEBAR_TABBED_SECTION = "SidebarTabbed";

    const SidebarSharedLayout default_sidebar_layout;
    const SidebarClassicLayout default_classic_layout;
    const SidebarTabbedLayout default_tabbed_layout;

    char sidebar_view[64];
    ini.Get_String(SIDEBAR_SECTION, "ViewType", "Classic", sidebar_view, sizeof(sidebar_view));
    if (_stricmp(sidebar_view, "Tabbed") == 0) {
        BattleSidebarViewType = SIDEBAR_TABBED;
    } else {
        BattleSidebarViewType = SIDEBAR_CLASSIC;
    }

    UnitHealthBarDrawPos = ini.Get_Point(INGAME, "UnitHealthBarPos", UnitHealthBarDrawPos);
    InfantryHealthBarDrawPos = ini.Get_Point(INGAME, "InfantryHealthBarPos", InfantryHealthBarDrawPos);

    IsTextLabelOutline = ini.Get_Bool(INGAME, "TextLabelOutline", IsTextLabelOutline);
    TextLabelBackgroundTransparency = ini.Get_Int(INGAME, "TextLabelBackgroundTransparency", TextLabelBackgroundTransparency);
    TextLabelBackgroundTransparency = std::clamp(TextLabelBackgroundTransparency, 0u, 100u);

    UnitGroupNumberOffset = ini.Get_Point(INGAME, "UnitGroupNumberOffset", UnitGroupNumberOffset);
    InfantryGroupNumberOffset = ini.Get_Point(INGAME, "InfantryGroupNumberOffset", InfantryGroupNumberOffset);
    BuildingGroupNumberOffset = ini.Get_Point(INGAME, "BuildingGroupNumberOffset", BuildingGroupNumberOffset);
    AircraftGroupNumberOffset = ini.Get_Point(INGAME, "AircraftGroupNumberOffset", AircraftGroupNumberOffset);
    UnitWithPipGroupNumberOffset = ini.Get_Point(INGAME, "UnitWithPipGroupNumberOffset", UnitWithPipGroupNumberOffset);
    InfantryWithPipGroupNumberOffset = ini.Get_Point(INGAME, "InfantryWithPipGroupNumberOffset", InfantryWithPipGroupNumberOffset);
    BuildingWithPipGroupNumberOffset = ini.Get_Point(INGAME, "BuildingWithPipGroupNumberOffset", BuildingWithPipGroupNumberOffset);
    AircraftWithPipGroupNumberOffset = ini.Get_Point(INGAME, "AircraftWithPipGroupNumberOffset", AircraftWithPipGroupNumberOffset);
    UnitVeterancyPipOffset = ini.Get_Point(INGAME, "UnitVeterancyPipOffset", UnitVeterancyPipOffset);
    InfantryVeterancyPipOffset = ini.Get_Point(INGAME, "InfantryVeterancyPipOffset", InfantryVeterancyPipOffset);
    BuildingVeterancyPipOffset = ini.Get_Point(INGAME, "BuildingVeterancyPipOffset", BuildingVeterancyPipOffset);
    AircraftVeterancyPipOffset = ini.Get_Point(INGAME, "AircraftVeterancyPipOffset", AircraftVeterancyPipOffset);
    UnitSpecialPipOffset = ini.Get_Point(INGAME, "UnitSpecialPipOffset", UnitSpecialPipOffset);
    InfantrySpecialPipOffset = ini.Get_Point(INGAME, "InfantrySpecialPipOffset", InfantrySpecialPipOffset);
    BuildingSpecialPipOffset = ini.Get_Point(INGAME, "BuildingSpecialPipOffset", BuildingSpecialPipOffset);
    AircraftSpecialPipOffset = ini.Get_Point(INGAME, "AircraftSpecialPipOffset", AircraftSpecialPipOffset);

    IsBandBoxDropShadow = ini.Get_Bool(INGAME, "BandBoxDropShadow", IsBandBoxDropShadow);
    IsBandBoxThick = ini.Get_Bool(INGAME, "BandBoxThick", IsBandBoxThick);
    BandBoxColor = ini.Get_RGBColor(INGAME, "BandBoxColor", BandBoxColor);
    BandBoxDropShadowColor = ini.Get_RGBColor(INGAME, "BandBoxDropShadowColor", BandBoxDropShadowColor);
    BandBoxTintTransparency = ini.Get_Int(INGAME, "BandBoxTintTransparency", BandBoxTintTransparency);
    BandBoxTintTransparency = std::clamp(BandBoxTintTransparency, 0u, 100u);
    BandBoxTintColors = ini.Get_RGBColors(INGAME, "BandBoxTintColors", BandBoxTintColors);

    ASSERT_PRINT(BandBoxTintColors.Count() == 2, "BandBoxTintColors must contain two valid entries!");

    IsAlwaysShowActionLines = ini.Get_Bool(INGAME, "AlwaysShowActionLines", IsAlwaysShowActionLines);
    IsMovementLineDashed = ini.Get_Bool(INGAME, "MovementLineDashed", IsMovementLineDashed);
    IsMovementLineDropShadow = ini.Get_Bool(INGAME, "MovementLineDropShadow", IsMovementLineDropShadow);
    IsMovementLineThick = ini.Get_Bool(INGAME, "MovementLineThick", IsMovementLineThick);
    MovementLineColor = ini.Get_RGBColor(INGAME, "MovementLineColor", MovementLineColor);
    MovementLineDropShadowColor = ini.Get_RGBColor(INGAME, "MovementLineDropShadowColor", MovementLineDropShadowColor);

    IsTargetLineDashed = ini.Get_Bool(INGAME, "TargetLineDashed", IsTargetLineDashed);
    IsTargetLineDropShadow = ini.Get_Bool(INGAME, "TargetLineDropShadow", IsTargetLineDropShadow);
    IsTargetLineThick = ini.Get_Bool(INGAME, "TargetLineThick", IsTargetLineThick);
    TargetLineColor = ini.Get_RGBColor(INGAME, "TargetLineColor", TargetLineColor);
    TargetLineDropShadowColor = ini.Get_RGBColor(INGAME, "TargetLineDropShadowColor", TargetLineDropShadowColor);

    IsTargetLaserDashed = ini.Get_Bool(INGAME, "TargetLaserDashed", IsTargetLaserDashed);
    IsTargetLaserDropShadow = ini.Get_Bool(INGAME, "TargetLaserDropShadow", IsTargetLaserDropShadow);
    IsTargetLaserThick = ini.Get_Bool(INGAME, "TargetLaserThick", IsTargetLaserThick);
    TargetLaserColor = ini.Get_RGBColor(INGAME, "TargetLaserColor", TargetLaserColor);
    TargetLaserDropShadowColor = ini.Get_RGBColor(INGAME, "TargetLaserDropShadowColor", TargetLaserDropShadowColor);
    TargetLaserTime = ini.Get_Int(INGAME, "TargetLaserTime", TargetLaserTime);

    IsShowNavComQueueLines = ini.Get_Bool(INGAME, "ShowNavComQueueLines", IsShowNavComQueueLines);
    IsNavComQueueLineDashed = ini.Get_Bool(INGAME, "NavComQueueLineDashed", IsNavComQueueLineDashed);
    IsNavComQueueLineDropShadow = ini.Get_Bool(INGAME, "NavComQueueLineDropShadow", IsNavComQueueLineDropShadow);
    IsNavComQueueLineThick = ini.Get_Bool(INGAME, "NavComQueueLineThick", IsNavComQueueLineThick);
    NavComQueueLineColor = ini.Get_RGBColor(INGAME, "NavComQueueLineColor", NavComQueueLineColor);
    NavComQueueLineDropShadowColor = ini.Get_RGBColor(INGAME, "NavComQueueLineDropShadowColor", NavComQueueLineDropShadowColor);

    Read_Button_Layout(ini, SIDEBAR_SECTION, "RepairButtonPos", "RepairButtonVisible",
        default_sidebar_layout.RepairButton, SidebarLayout.RepairButton);
    Read_Button_Layout(ini, SIDEBAR_SECTION, "SellButtonPos", "SellButtonVisible",
        default_sidebar_layout.SellButton, SidebarLayout.SellButton);
    Read_Button_Layout(ini, SIDEBAR_SECTION, "PowerButtonPos", "PowerButtonVisible",
        default_sidebar_layout.PowerButton, SidebarLayout.PowerButton);
    Read_Button_Layout(ini, SIDEBAR_SECTION, "WaypointButtonPos", "WaypointButtonVisible",
        default_sidebar_layout.WaypointButton, SidebarLayout.WaypointButton);
    SidebarLayout.PowerBarPosition = ini.Get_Point(SIDEBAR_SECTION, "PowerBarPos", default_sidebar_layout.PowerBarPosition);

    ClassicSidebarLayoutConfig.LeftStripPosition = ini.Get_Point(SIDEBAR_CLASSIC_SECTION, "LeftStripPos", default_classic_layout.LeftStripPosition);
    ClassicSidebarLayoutConfig.RightStripPosition = ini.Get_Point(SIDEBAR_CLASSIC_SECTION, "RightStripPos", default_classic_layout.RightStripPosition);
    ClassicSidebarLayoutConfig.VisibleRows = ini.Get_Int(SIDEBAR_CLASSIC_SECTION, "VisibleRows", default_classic_layout.VisibleRows);
    ClassicSidebarLayoutConfig.RowPitch = std::max(1, ini.Get_Int(SIDEBAR_CLASSIC_SECTION, "RowPitch", default_classic_layout.RowPitch));
    Read_Optional_Point(ini, SIDEBAR_CLASSIC_SECTION, "LeftUpButtonPos", default_classic_layout.LeftUpButtonPosition,
        ClassicSidebarLayoutConfig.LeftUpButtonPosition, ClassicSidebarLayoutConfig.HasCustomLeftUpButtonPosition);
    Read_Optional_Point(ini, SIDEBAR_CLASSIC_SECTION, "LeftDownButtonPos", default_classic_layout.LeftDownButtonPosition,
        ClassicSidebarLayoutConfig.LeftDownButtonPosition, ClassicSidebarLayoutConfig.HasCustomLeftDownButtonPosition);
    Read_Optional_Point(ini, SIDEBAR_CLASSIC_SECTION, "RightUpButtonPos", default_classic_layout.RightUpButtonPosition,
        ClassicSidebarLayoutConfig.RightUpButtonPosition, ClassicSidebarLayoutConfig.HasCustomRightUpButtonPosition);
    Read_Optional_Point(ini, SIDEBAR_CLASSIC_SECTION, "RightDownButtonPos", default_classic_layout.RightDownButtonPosition,
        ClassicSidebarLayoutConfig.RightDownButtonPosition, ClassicSidebarLayoutConfig.HasCustomRightDownButtonPosition);
    ClassicSidebarLayoutConfig.LeftUpButtonVisible = ini.Get_Bool(SIDEBAR_CLASSIC_SECTION, "LeftUpButtonVisible", default_classic_layout.LeftUpButtonVisible);
    ClassicSidebarLayoutConfig.LeftDownButtonVisible = ini.Get_Bool(SIDEBAR_CLASSIC_SECTION, "LeftDownButtonVisible", default_classic_layout.LeftDownButtonVisible);
    ClassicSidebarLayoutConfig.RightUpButtonVisible = ini.Get_Bool(SIDEBAR_CLASSIC_SECTION, "RightUpButtonVisible", default_classic_layout.RightUpButtonVisible);
    ClassicSidebarLayoutConfig.RightDownButtonVisible = ini.Get_Bool(SIDEBAR_CLASSIC_SECTION, "RightDownButtonVisible", default_classic_layout.RightDownButtonVisible);

    TabbedSidebarLayoutConfig.TabButtonPosition[0] = ini.Get_Point(SIDEBAR_TABBED_SECTION, "Tab1Pos", default_tabbed_layout.TabButtonPosition[0]);
    TabbedSidebarLayoutConfig.TabButtonPosition[1] = ini.Get_Point(SIDEBAR_TABBED_SECTION, "Tab2Pos", default_tabbed_layout.TabButtonPosition[1]);
    TabbedSidebarLayoutConfig.TabButtonPosition[2] = ini.Get_Point(SIDEBAR_TABBED_SECTION, "Tab3Pos", default_tabbed_layout.TabButtonPosition[2]);
    TabbedSidebarLayoutConfig.TabButtonPosition[3] = ini.Get_Point(SIDEBAR_TABBED_SECTION, "Tab4Pos", default_tabbed_layout.TabButtonPosition[3]);
    TabbedSidebarLayoutConfig.StripPosition = ini.Get_Point(SIDEBAR_TABBED_SECTION, "StripPos", default_tabbed_layout.StripPosition);
    TabbedSidebarLayoutConfig.VisibleRows = ini.Get_Int(SIDEBAR_TABBED_SECTION, "VisibleRows", default_tabbed_layout.VisibleRows);
    TabbedSidebarLayoutConfig.RowPitch = std::max(1, ini.Get_Int(SIDEBAR_TABBED_SECTION, "RowPitch", default_tabbed_layout.RowPitch));
    TabbedSidebarLayoutConfig.ColumnSpacing = std::max(1, ini.Get_Int(SIDEBAR_TABBED_SECTION, "ColumnSpacing", default_tabbed_layout.ColumnSpacing));
    Read_Optional_Point(ini, SIDEBAR_TABBED_SECTION, "UpButtonPos", default_tabbed_layout.UpButtonPosition,
        TabbedSidebarLayoutConfig.UpButtonPosition, TabbedSidebarLayoutConfig.HasCustomUpButtonPosition);
    Read_Optional_Point(ini, SIDEBAR_TABBED_SECTION, "DownButtonPos", default_tabbed_layout.DownButtonPosition,
        TabbedSidebarLayoutConfig.DownButtonPosition, TabbedSidebarLayoutConfig.HasCustomDownButtonPosition);
    TabbedSidebarLayoutConfig.UpButtonVisible = ini.Get_Bool(SIDEBAR_TABBED_SECTION, "UpButtonVisible", default_tabbed_layout.UpButtonVisible);
    TabbedSidebarLayoutConfig.DownButtonVisible = ini.Get_Bool(SIDEBAR_TABBED_SECTION, "DownButtonVisible", default_tabbed_layout.DownButtonVisible);

    IsCenterSidebarButtonsOnRadar = ini.Get_Bool(INGAME, "CenterSidebarButtonsOnRadar", IsCenterSidebarButtonsOnRadar);

    BeaconAnimFramesPerSecond = ini.Get_Int(INGAME, "BeaconAnimFramesPerSecond", BeaconAnimFramesPerSecond);
    RadarBeaconAnimFramesPerSecond = ini.Get_Int(INGAME, "RadarBeaconAnimFramesPerSecond", RadarBeaconAnimFramesPerSecond);
    BeaconTextOffset = ini.Get_Int(INGAME, "BeaconTextOffset", BeaconTextOffset);
    BeaconPreviewTextOffset = ini.Get_Int(INGAME, "BeaconPreviewTextOffset", BeaconPreviewTextOffset);

    for (int i = 0; i < std::size(BeaconText); i++) {
        char key[32], buffer[512];
        std::sprintf(key, "BeaconText%d", i + 1);
        if (ini.Get_String(INGAME, key, BeaconText[i].c_str(), buffer, std::size(buffer)) > 0) {
            BeaconText[i] = buffer;
        }
        std::sprintf(key, "BeaconPreviewText%d", i + 1);
        if (ini.Get_String(INGAME, key, BeaconPreviewText[i].c_str(), buffer, std::size(buffer)) > 0) {
            BeaconPreviewText[i] = buffer;
        }
    }

    return true;
}
