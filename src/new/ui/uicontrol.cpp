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
#include "ccfile.h"
#include "ccini.h"

#include <algorithm>
#include <cstring>


UIControlsClass *UIControls = nullptr;


namespace
{
void Read_String_Key(CCINIClass& ini, const char* section, const char* key, std::string& value)
{
    char buffer[260];
    if (ini.Get_String(section, key, value.c_str(), buffer, sizeof(buffer)) > 0) {
        value = buffer;
    }
}


void Read_Battle_Sidebar_Config(CCINIClass& ini, const char* section, BattleSidebarLayoutBase& layout)
{
    layout.RepairButton.Position = ini.Get_Point(section, "RepairButtonPos", layout.RepairButton.Position);
    layout.RepairButton.IsVisible = ini.Get_Bool(section, "RepairButtonVisible", layout.RepairButton.IsVisible);
    layout.SellButton.Position = ini.Get_Point(section, "SellButtonPos", layout.SellButton.Position);
    layout.SellButton.IsVisible = ini.Get_Bool(section, "SellButtonVisible", layout.SellButton.IsVisible);
    layout.PowerButton.Position = ini.Get_Point(section, "PowerButtonPos", layout.PowerButton.Position);
    layout.PowerButton.IsVisible = ini.Get_Bool(section, "PowerButtonVisible", layout.PowerButton.IsVisible);
    layout.WaypointButton.Position = ini.Get_Point(section, "WaypointButtonPos", layout.WaypointButton.Position);
    layout.WaypointButton.IsVisible = ini.Get_Bool(section, "WaypointButtonVisible", layout.WaypointButton.IsVisible);
    layout.PowerBarPosition = ini.Get_Point(section, "PowerBarPos", layout.PowerBarPosition);

    Read_String_Key(ini, section, "SidebarShape", layout.SidebarShape);
    Read_String_Key(ini, section, "SidebarMiddleShape", layout.SidebarMiddleShape);
    Read_String_Key(ini, section, "SidebarBottomShape", layout.SidebarBottomShape);
    Read_String_Key(ini, section, "SidebarAddonShape", layout.SidebarAddonShape);
    Read_String_Key(ini, section, "ClockShape", layout.ClockShape);
    Read_String_Key(ini, section, "RechargeClockShape", layout.RechargeClockShape);
    Read_String_Key(ini, section, "DarkenShape", layout.DarkenShape);
    Read_String_Key(ini, section, "ScrollUpButtonShape", layout.ScrollUpButtonShape);
    Read_String_Key(ini, section, "ScrollDownButtonShape", layout.ScrollDownButtonShape);
    Read_String_Key(ini, section, "RepairButtonShape", layout.RepairButtonShape);
    Read_String_Key(ini, section, "SellButtonShape", layout.SellButtonShape);
    Read_String_Key(ini, section, "PowerButtonShape", layout.PowerButtonShape);
    Read_String_Key(ini, section, "WaypointButtonShape", layout.WaypointButtonShape);
    Read_String_Key(ini, section, "PowerPipShape", layout.PowerPipShape);
}
}


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


void UIControlsClass::Reset_To_Defaults()
{
    *this = UIControlsClass();
}


bool UIControlsClass::Read_INI_File(const char* filename, bool reset_to_defaults)
{
    if (reset_to_defaults) {
        Reset_To_Defaults();
    }

    if (filename == nullptr || filename[0] == '\0') {
        return false;
    }

    CCFileClass file(filename);
    if (!file.Is_Available()) {
        return false;
    }

    CCINIClass ini;
    if (!ini.Load(file, false)) {
        return false;
    }

    return Read_INI(ini);
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

    char sidebar_view[64];
    ini.Get_String(SIDEBAR_SECTION, "ViewType", BattleSidebarViewType == SIDEBAR_TABBED ? "Tabbed" : "Classic", sidebar_view, sizeof(sidebar_view));
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

    Read_Battle_Sidebar_Config(ini, SIDEBAR_CLASSIC_SECTION, ClassicSidebarLayoutConfig);
    ClassicSidebarLayoutConfig.LeftStripPosition = ini.Get_Point(SIDEBAR_CLASSIC_SECTION, "LeftStripPos", ClassicSidebarLayoutConfig.LeftStripPosition);
    ClassicSidebarLayoutConfig.RightStripPosition = ini.Get_Point(SIDEBAR_CLASSIC_SECTION, "RightStripPos", ClassicSidebarLayoutConfig.RightStripPosition);
    ClassicSidebarLayoutConfig.VisibleRows = ini.Get_Int(SIDEBAR_CLASSIC_SECTION, "VisibleRows", ClassicSidebarLayoutConfig.VisibleRows);
    ClassicSidebarLayoutConfig.RowPitch = std::max(1, ini.Get_Int(SIDEBAR_CLASSIC_SECTION, "RowPitch", ClassicSidebarLayoutConfig.RowPitch));
    ClassicSidebarLayoutConfig.LeftUpButtonPosition = ini.Get_Point(SIDEBAR_CLASSIC_SECTION, "LeftUpButtonPos", ClassicSidebarLayoutConfig.LeftUpButtonPosition);
    ClassicSidebarLayoutConfig.LeftDownButtonPosition = ini.Get_Point(SIDEBAR_CLASSIC_SECTION, "LeftDownButtonPos", ClassicSidebarLayoutConfig.LeftDownButtonPosition);
    ClassicSidebarLayoutConfig.RightUpButtonPosition = ini.Get_Point(SIDEBAR_CLASSIC_SECTION, "RightUpButtonPos", ClassicSidebarLayoutConfig.RightUpButtonPosition);
    ClassicSidebarLayoutConfig.RightDownButtonPosition = ini.Get_Point(SIDEBAR_CLASSIC_SECTION, "RightDownButtonPos", ClassicSidebarLayoutConfig.RightDownButtonPosition);
    ClassicSidebarLayoutConfig.IsLeftUpButtonVisible = ini.Get_Bool(SIDEBAR_CLASSIC_SECTION, "LeftUpButtonVisible", ClassicSidebarLayoutConfig.IsLeftUpButtonVisible);
    ClassicSidebarLayoutConfig.IsLeftDownButtonVisible = ini.Get_Bool(SIDEBAR_CLASSIC_SECTION, "LeftDownButtonVisible", ClassicSidebarLayoutConfig.IsLeftDownButtonVisible);
    ClassicSidebarLayoutConfig.IsRightUpButtonVisible = ini.Get_Bool(SIDEBAR_CLASSIC_SECTION, "RightUpButtonVisible", ClassicSidebarLayoutConfig.IsRightUpButtonVisible);
    ClassicSidebarLayoutConfig.IsRightDownButtonVisible = ini.Get_Bool(SIDEBAR_CLASSIC_SECTION, "RightDownButtonVisible", ClassicSidebarLayoutConfig.IsRightDownButtonVisible);

    Read_Battle_Sidebar_Config(ini, SIDEBAR_TABBED_SECTION, TabbedSidebarLayoutConfig);
    TabbedSidebarLayoutConfig.TabButtonPosition[0] = ini.Get_Point(SIDEBAR_TABBED_SECTION, "Tab1Pos", TabbedSidebarLayoutConfig.TabButtonPosition[0]);
    TabbedSidebarLayoutConfig.TabButtonPosition[1] = ini.Get_Point(SIDEBAR_TABBED_SECTION, "Tab2Pos", TabbedSidebarLayoutConfig.TabButtonPosition[1]);
    TabbedSidebarLayoutConfig.TabButtonPosition[2] = ini.Get_Point(SIDEBAR_TABBED_SECTION, "Tab3Pos", TabbedSidebarLayoutConfig.TabButtonPosition[2]);
    TabbedSidebarLayoutConfig.TabButtonPosition[3] = ini.Get_Point(SIDEBAR_TABBED_SECTION, "Tab4Pos", TabbedSidebarLayoutConfig.TabButtonPosition[3]);
    TabbedSidebarLayoutConfig.StripPosition = ini.Get_Point(SIDEBAR_TABBED_SECTION, "StripPos", TabbedSidebarLayoutConfig.StripPosition);
    TabbedSidebarLayoutConfig.VisibleRows = ini.Get_Int(SIDEBAR_TABBED_SECTION, "VisibleRows", TabbedSidebarLayoutConfig.VisibleRows);
    TabbedSidebarLayoutConfig.RowPitch = std::max(1, ini.Get_Int(SIDEBAR_TABBED_SECTION, "RowPitch", TabbedSidebarLayoutConfig.RowPitch));
    TabbedSidebarLayoutConfig.ColumnSpacing = std::max(1, ini.Get_Int(SIDEBAR_TABBED_SECTION, "ColumnSpacing", TabbedSidebarLayoutConfig.ColumnSpacing));
    TabbedSidebarLayoutConfig.UpButtonPosition = ini.Get_Point(SIDEBAR_TABBED_SECTION, "UpButtonPos", TabbedSidebarLayoutConfig.UpButtonPosition);
    TabbedSidebarLayoutConfig.DownButtonPosition = ini.Get_Point(SIDEBAR_TABBED_SECTION, "DownButtonPos", TabbedSidebarLayoutConfig.DownButtonPosition);
    TabbedSidebarLayoutConfig.IsUpButtonVisible = ini.Get_Bool(SIDEBAR_TABBED_SECTION, "UpButtonVisible", TabbedSidebarLayoutConfig.IsUpButtonVisible);
    TabbedSidebarLayoutConfig.IsDownButtonVisible = ini.Get_Bool(SIDEBAR_TABBED_SECTION, "DownButtonVisible", TabbedSidebarLayoutConfig.IsDownButtonVisible);
    Read_String_Key(ini, SIDEBAR_TABBED_SECTION, "StructureTabShape", TabbedSidebarLayoutConfig.StructureTabShape);
    Read_String_Key(ini, SIDEBAR_TABBED_SECTION, "InfantryTabShape", TabbedSidebarLayoutConfig.InfantryTabShape);
    Read_String_Key(ini, SIDEBAR_TABBED_SECTION, "UnitTabShape", TabbedSidebarLayoutConfig.UnitTabShape);
    Read_String_Key(ini, SIDEBAR_TABBED_SECTION, "SpecialTabShape", TabbedSidebarLayoutConfig.SpecialTabShape);

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
