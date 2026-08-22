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
#include "debughandler.h"
#include "tibsun_inline.h"

#include <algorithm>


UIControlsClass *UIControls = nullptr;


const UIControlsClass::LoadingScreenSize UIControlsClass::DefaultLoadingScreenSizes[] = {
    {{640, 400}, {436, 155}, {566, 152}},
    {{640, 480}, {436, 186}, {566, 177}},
    {{800, 600}, {546, 233}, {711, 227}},
};


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

    SubtitleFontName = ini.Get_String(INGAME, "SubtitleFontName", SubtitleFontName);
    SubtitleFontHeight = ini.Get_Int(INGAME, "SubtitleFontHeight", SubtitleFontHeight);
    SubtitleFontWeight = ini.Get_Int(INGAME, "SubtitleFontWeight", SubtitleFontWeight);
    SubtitleTextColor = ini.Get_RGBColor(INGAME, "SubtitleTextColor", SubtitleTextColor);
    SubtitleOutlineColor = ini.Get_RGBColor(INGAME, "SubtitleOutlineColor", SubtitleOutlineColor);
    SubtitleOutlineWidth = ini.Get_Int(INGAME, "SubtitleOutlineWidth", SubtitleOutlineWidth);
    SubtitleMarginX = ini.Get_Int(INGAME, "SubtitleMarginX", SubtitleMarginX);
    SubtitleMarginBottom = ini.Get_Int(INGAME, "SubtitleMarginBottom", SubtitleMarginBottom);

    static char const* const LOADING_SCREENS = "LoadingScreens";
    for (int i = 0; i < ini.Entry_Count(LOADING_SCREENS); i++) {
        const char* key = ini.Get_Entry(LOADING_SCREENS, i);
        const std::string entry = ini.Get_String(LOADING_SCREENS, key, {});
        LoadingScreen screen(entry.c_str());
        if (screen.IsValid) {
            LoadingScreens.emplace_back(screen);
        }
    }

    return true;
}


UIControlsClass::LoadingScreen::LoadingScreen(char const* entry)
{
    if (entry == nullptr || entry[0] == '\0') {
        DEBUG_ERROR("Invalid empty loading screen entry!\n");
        return;
    }

    char buffer[1024];
    std::snprintf(buffer, sizeof(buffer), "%s", entry);

    auto parse_number = [](char const* s) -> int {
        if (s == nullptr) {
            return -1;
        }

        char* end = nullptr;
        long val = std::strtol(s, &end, 10);

        if (end == s || *end != '\0') {
            return -1;
        }

        return static_cast<int>(val);
    };

    char* token = std::strtok(buffer, ",");
    House = static_cast<HousesType>(parse_number(token));
    if (House < HOUSE_FIRST) {
        DEBUG_ERROR("Invalid loading screen entry: \"{}\"!\n", entry);
        return;
    }

    token = std::strtok(nullptr, ",");
    if (token == nullptr) {
        DEBUG_ERROR("Invalid loading screen entry: \"{}\"!\n", entry);
        return;
    }

    Filename = token;
    if (Filename.empty()) {
        DEBUG_ERROR("Invalid loading screen entry: \"{}\"!\n", entry);
        return;
    }

    token = std::strtok(nullptr, ",");
    int width = parse_number(token);
    if (width <= 0) {
        DEBUG_ERROR("Invalid loading screen entry: \"{}\"!\n", entry);
        return;
    }

    int height = 0;
    token = std::strtok(nullptr, ",");
    height = parse_number(token);
    if (height <= 0) {
        DEBUG_ERROR("Invalid loading screen entry: \"{}\"!\n", entry);
        return;
    }

    token = std::strtok(nullptr, ",");
    int sp_xoff = parse_number(token);

    token = std::strtok(nullptr, ",");
    int sp_yoff = parse_number(token);

    token = std::strtok(nullptr, ",");
    int mp_xoff = parse_number(token);

    token = std::strtok(nullptr, ",");
    int mp_yoff = parse_number(token);

    auto get_size = [](int width, int height) -> const LoadingScreenSize& {
        for (auto& cfg : DefaultLoadingScreenSizes) {
            if (cfg.Size.X == width && cfg.Size.Y == height) {
                return cfg;
            }
        }
        return DefaultLoadingScreenSizes[0];
    };

    if (sp_xoff <= 0 || sp_yoff <= 0) {
        auto& size = get_size(width, height);
        sp_xoff = size.SPPosition.X;
        sp_yoff = size.SPPosition.Y;
    }

    if (mp_xoff <= 0 || mp_yoff <= 0) {
        auto& size = get_size(width, height);
        mp_xoff = size.MPPosition.X;
        mp_yoff = size.MPPosition.Y;
    }

    Size = {{width, height}, {sp_xoff, sp_yoff}, {mp_xoff, mp_yoff}};
    IsValid = true;
}


UIControlsClass::LoadingScreenSize const& UIControlsClass::Pick_Default_Loading_Screen_Size()
{
    int largest_size = 0;
    int largest_index = 0;

    for (int i = 0; i < std::size(DefaultLoadingScreenSizes); i++) {
        if (VisibleRect.Width >= DefaultLoadingScreenSizes[i].Size.X && VisibleRect.Height >= DefaultLoadingScreenSizes[i].Size.Y) {
            int size = DefaultLoadingScreenSizes[i].Size.X * DefaultLoadingScreenSizes[i].Size.Y;
            if (size > largest_size) {
                largest_size = size;
                largest_index = i;
            }
        }
    }

    return DefaultLoadingScreenSizes[largest_index];
}


static char Pick_House_Letter(HousesType house)
{
    // Special handling: GDI = C/D, house 1 = A/B
    int base;
    if (house == HOUSE_GDI) {
        base = 'C';
    } else if (house == HOUSE_NOD) {
        base = 'A';
    } else {
        // Houses 2+ start at E/F, G/H, I/J, ...
        base = 'E' + (house - 2) * 2;
        if (base > 'Z') {
            // wrap around alphabet if past 'Z'
            base = 'A' + ((base - 'A') % 26);
        }
    }

    // Randomly pick first or second letter of the pair
    return static_cast<char>(base + Sim_Random_Pick(0, 1));
}


UIControlsClass::LoadingScreen UIControlsClass::Pick_Loading_Screen(HousesType house) const
{
    std::vector<LoadingScreen const*> screens;

    int largest_size = 0;
    for (auto& screen : LoadingScreens) {
        if (screen.House == house && VisibleRect.Width >= screen.Size.Size.X && VisibleRect.Height >= screen.Size.Size.Y) {
            int size = screen.Size.Size.X * screen.Size.Size.Y;
            if (size > largest_size) {
                screens.clear();
                screens.emplace_back(&screen);
                largest_size = size;
            } else if (size == largest_size) {
                screens.emplace_back(&screen);
            }
        }
    }

    if (!screens.empty()) {
        return *screens[Sim_Random_Pick(0u, screens.size() - 1)];
    }

    LoadingScreenSize size = Pick_Default_Loading_Screen_Size();

    /**
     *  Nod has the text shifted.
     */
    if (house == HOUSE_NOD) {
        size.SPPosition.Y += 7;
        size.MPPosition.Y += 7;
    }

    char name[32];
    char letter = Pick_House_Letter(house);
    std::snprintf(name, sizeof(name), "LOAD%03d%c", size.Size.Y, letter);

    return LoadingScreen {house, name, size};
}
