/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  UI controls and overrides.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "uicontrol.h"

#include "asserthandler.h"
#include "ccfile.h"
#include "ccini.h"

#include <algorithm>


UIControlsClass *UIControls = nullptr;


/***************************************************************************
**  Sidebar view type helpers
***************************************************************************/


/**
 *  Parses the sidebar view type name.
 *
 *  @author: ZivDero
 */
SidebarViewType Sidebar_View_From_Name(const char* name, SidebarViewType default_type)
{
    if (name == nullptr || *name == '\0') {
        return default_type;
    }

    if (_stricmp(name, "Tabbed") == 0) {
        return SIDEBAR_TABBED;
    }

    if (_stricmp(name, "Classic") == 0) {
        return SIDEBAR_CLASSIC;
    }

    return default_type;
}


/**
 *  Returns the INI-facing name for a sidebar view type.
 *
 *  @author: ZivDero
 */
const char* Name_From_Sidebar_View_Type(SidebarViewType view_type)
{
    return view_type == SIDEBAR_TABBED ? "Tabbed" : "Classic";
}


/***************************************************************************
**  Battle sidebar layout config
***************************************************************************/


/**
 *  Reads shared battle sidebar layout values from the given INI section.
 *
 *  @author: CCHyper
 */
void BattleSidebarLayoutBase::Read_INI(CCINIClass const& ini, const char* section)
{
    RepairButton.Position = ini.Get_Point(section, "RepairButtonPos", RepairButton.Position);
    RepairButton.IsVisible = ini.Get_Bool(section, "RepairButtonVisible", RepairButton.IsVisible);
    SellButton.Position = ini.Get_Point(section, "SellButtonPos", SellButton.Position);
    SellButton.IsVisible = ini.Get_Bool(section, "SellButtonVisible", SellButton.IsVisible);
    PowerButton.Position = ini.Get_Point(section, "PowerButtonPos", PowerButton.Position);
    PowerButton.IsVisible = ini.Get_Bool(section, "PowerButtonVisible", PowerButton.IsVisible);
    WaypointButton.Position = ini.Get_Point(section, "WaypointButtonPos", WaypointButton.Position);
    WaypointButton.IsVisible = ini.Get_Bool(section, "WaypointButtonVisible", WaypointButton.IsVisible);
    PowerBarPosition = ini.Get_Point(section, "PowerBarPos", PowerBarPosition);
    PowerBarWidth = std::max(1, ini.Get_Int(section, "PowerBarWidth", PowerBarWidth));
    PowerPipHeight = std::max(1, ini.Get_Int(section, "PowerPipHeight", PowerPipHeight));
    CameoSize = ini.Get_Point(section, "CameoSize", CameoSize);
    CameoSize.X = std::max(1, CameoSize.X);
    CameoSize.Y = std::max(1, CameoSize.Y);
    CameoNameOffset = ini.Get_Int(section, "CameoNameOffset", CameoNameOffset);
    CameoTextOffset = ini.Get_Point(section, "CameoTextOffset", CameoTextOffset);
    QueueCountOffset = ini.Get_Point(section, "QueueCountOffset", QueueCountOffset);

    SidebarShape = ini.Get_String(section, "SidebarShape", SidebarShape);
    SidebarMiddleShape = ini.Get_String(section, "SidebarMiddleShape", SidebarMiddleShape);
    SidebarBottomShape = ini.Get_String(section, "SidebarBottomShape", SidebarBottomShape);
    SidebarAddonShape = ini.Get_String(section, "SidebarAddonShape", SidebarAddonShape);
    ClockShape = ini.Get_String(section, "ClockShape", ClockShape);
    RechargeClockShape = ini.Get_String(section, "RechargeClockShape", RechargeClockShape);
    DarkenShape = ini.Get_String(section, "DarkenShape", DarkenShape);
    ScrollUpButtonShape = ini.Get_String(section, "ScrollUpButtonShape", ScrollUpButtonShape);
    ScrollDownButtonShape = ini.Get_String(section, "ScrollDownButtonShape", ScrollDownButtonShape);
    RepairButtonShape = ini.Get_String(section, "RepairButtonShape", RepairButtonShape);
    SellButtonShape = ini.Get_String(section, "SellButtonShape", SellButtonShape);
    PowerButtonShape = ini.Get_String(section, "PowerButtonShape", PowerButtonShape);
    WaypointButtonShape = ini.Get_String(section, "WaypointButtonShape", WaypointButtonShape);
    PowerPipShape = ini.Get_String(section, "PowerPipShape", PowerPipShape);
}


/**
 *  Reads classic sidebar-specific layout values from the given INI section.
 *
 *  @author: CCHyper
 */
void SidebarClassicLayout::Read_INI(CCINIClass const& ini, const char* section)
{
    BattleSidebarLayoutBase::Read_INI(ini, section);

    LeftStripPosition = ini.Get_Point(section, "LeftStripPos", LeftStripPosition);
    RightStripPosition = ini.Get_Point(section, "RightStripPos", RightStripPosition);
    RowSpacing = std::max(0, ini.Get_Int(section, "RowSpacing", RowSpacing));
    LeftUpButtonPosition = ini.Get_Point(section, "LeftUpButtonPos", LeftUpButtonPosition);
    LeftDownButtonPosition = ini.Get_Point(section, "LeftDownButtonPos", LeftDownButtonPosition);
    RightUpButtonPosition = ini.Get_Point(section, "RightUpButtonPos", RightUpButtonPosition);
    RightDownButtonPosition = ini.Get_Point(section, "RightDownButtonPos", RightDownButtonPosition);
    IsLeftUpButtonVisible = ini.Get_Bool(section, "LeftUpButtonVisible", IsLeftUpButtonVisible);
    IsLeftDownButtonVisible = ini.Get_Bool(section, "LeftDownButtonVisible", IsLeftDownButtonVisible);
    IsRightUpButtonVisible = ini.Get_Bool(section, "RightUpButtonVisible", IsRightUpButtonVisible);
    IsRightDownButtonVisible = ini.Get_Bool(section, "RightDownButtonVisible", IsRightDownButtonVisible);
    PowerBarHeightAdjust = ini.Get_Int(section, "PowerBarHeightAdjust", PowerBarHeightAdjust);
}


/**
 *  Reads tabbed sidebar-specific layout values from the given INI section.
 *
 *  @author: CCHyper
 */
void SidebarTabbedLayout::Read_INI(CCINIClass const& ini, const char* section)
{
    BattleSidebarLayoutBase::Read_INI(ini, section);

    TabButtonPosition[0] = ini.Get_Point(section, "Tab1Pos", TabButtonPosition[0]);
    TabButtonPosition[1] = ini.Get_Point(section, "Tab2Pos", TabButtonPosition[1]);
    TabButtonPosition[2] = ini.Get_Point(section, "Tab3Pos", TabButtonPosition[2]);
    TabButtonPosition[3] = ini.Get_Point(section, "Tab4Pos", TabButtonPosition[3]);
    StripPosition = ini.Get_Point(section, "StripPos", StripPosition);
    RowSpacing = std::max(0, ini.Get_Int(section, "RowSpacing", RowSpacing));
    ColumnSpacing = std::max(0, ini.Get_Int(section, "ColumnSpacing", ColumnSpacing));
    UpButtonPosition = ini.Get_Point(section, "UpButtonPos", UpButtonPosition);
    DownButtonPosition = ini.Get_Point(section, "DownButtonPos", DownButtonPosition);
    IsUpButtonVisible = ini.Get_Bool(section, "UpButtonVisible", IsUpButtonVisible);
    IsDownButtonVisible = ini.Get_Bool(section, "DownButtonVisible", IsDownButtonVisible);
    StructureTabShape = ini.Get_String(section, "StructureTabShape", StructureTabShape);
    InfantryTabShape = ini.Get_String(section, "InfantryTabShape", InfantryTabShape);
    UnitTabShape = ini.Get_String(section, "UnitTabShape", UnitTabShape);
    SpecialTabShape = ini.Get_String(section, "SpecialTabShape", SpecialTabShape);
    PowerBarHeightAdjust = ini.Get_Int(section, "PowerBarHeightAdjust", PowerBarHeightAdjust);
}


/***************************************************************************
**  UI offset queries
***************************************************************************/


/**
 *  Returns the group number offset for the given object type and pip state.
 *
 *  @author: CCHyper
 */
TPoint2D<int> UIControlsClass::Get_Group_Number_Offset(RTTIType type, bool has_pip) const
{
    switch (type)
    {
    case RTTI_UNIT:
    case RTTI_UNITTYPE:
        return has_pip ? UnitWithPipGroupNumberOffset : UnitGroupNumberOffset;
    case RTTI_INFANTRY:
    case RTTI_INFANTRYTYPE:
        return has_pip ? InfantryWithPipGroupNumberOffset : InfantryGroupNumberOffset;
    case RTTI_BUILDING:
    case RTTI_BUILDINGTYPE:
        return has_pip ? BuildingWithPipGroupNumberOffset : BuildingGroupNumberOffset;
    case RTTI_AIRCRAFT:
    case RTTI_AIRCRAFTTYPE:
        return has_pip ? AircraftWithPipGroupNumberOffset : AircraftGroupNumberOffset;
    default:
        return {0, 0};
    }
}


/**
 *  Returns the veterancy pip offset for the given object type.
 *
 *  @author: CCHyper
 */
TPoint2D<int> UIControlsClass::Get_Veterancy_Pip_Offset(RTTIType type) const
{
    switch (type)
    {
    case RTTI_UNIT:
    case RTTI_UNITTYPE:
        return UnitVeterancyPipOffset;
    case RTTI_INFANTRY:
    case RTTI_INFANTRYTYPE:
        return InfantryVeterancyPipOffset;
    case RTTI_BUILDING:
    case RTTI_BUILDINGTYPE:
        return BuildingVeterancyPipOffset;
    case RTTI_AIRCRAFT:
    case RTTI_AIRCRAFTTYPE:
        return AircraftVeterancyPipOffset;
    default:
        return {0, 0};
    }
}


/**
 *  Returns the special pip offset for the given object type.
 *
 *  @author: CCHyper
 */
TPoint2D<int> UIControlsClass::Get_Special_Pip_Offset(RTTIType type) const
{
    switch (type)
    {
    case RTTI_UNIT:
    case RTTI_UNITTYPE:
        return UnitSpecialPipOffset;
    case RTTI_INFANTRY:
    case RTTI_INFANTRYTYPE:
        return InfantrySpecialPipOffset;
    case RTTI_BUILDING:
    case RTTI_BUILDINGTYPE:
        return BuildingSpecialPipOffset;
    case RTTI_AIRCRAFT:
    case RTTI_AIRCRAFTTYPE:
        return AircraftSpecialPipOffset;
    default:
        return {0, 0};
    }
}


/***************************************************************************
**  INI loading
***************************************************************************/


/**
 *  Loads UI controls from an INI file on disk.
 *
 *  @author: CCHyper
 */
bool UIControlsClass::Read_INI_File(const char* filename, bool reset_to_defaults)
{
    if (reset_to_defaults) {
        *this = UIControlsClass();
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
bool UIControlsClass::Read_INI(CCINIClass const& ini)
{
    static char const * const INGAME = "Ingame";
    static char const * const SIDEBAR_SECTION = "Sidebar";
    static char const * const SIDEBAR_CLASSIC_SECTION = "SidebarClassic";
    static char const * const SIDEBAR_TABBED_SECTION = "SidebarTabbed";

    std::string sidebar_view = ini.Get_String(SIDEBAR_SECTION, "ViewType", Name_From_Sidebar_View_Type(BattleSidebarViewType));
    BattleSidebarViewType = Sidebar_View_From_Name(sidebar_view.c_str(), SIDEBAR_CLASSIC);

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

    ClassicSidebarLayoutConfig.Read_INI(ini, SIDEBAR_CLASSIC_SECTION);
    TabbedSidebarLayoutConfig.Read_INI(ini, SIDEBAR_TABBED_SECTION);

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
