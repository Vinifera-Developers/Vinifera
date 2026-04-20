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
#include "debughandler.h"
#include "tibsun_inline.h"


UIControlsClass *UIControls = nullptr;


const UIControlsClass::LoadingScreenSize UIControlsClass::DefaultLoadingScreenSizes[] = {
    {{640, 400}, {436, 155}, {566, 152}},
    {{640, 480}, {436, 186}, {566, 177}},
    {{800, 600}, {546, 233}, {711, 227}},
};


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
    BeaconAnimFramesPerSecond(25),
    RadarBeaconAnimFramesPerSecond(25),
    BeaconTextOffset(32),
    BeaconPreviewTextOffset(20),
    LoadingScreens()
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

    static char const* const LOADING_SCREENS = "LoadingScreens";
    for (int i = 0; i < ini.Entry_Count(LOADING_SCREENS); i++) {
        LoadingScreen screen(ini.Get_Entry(LOADING_SCREENS, i));
        if (screen.IsValid) {
            LoadingScreens.emplace_back(screen);
        }
    }

    return true;
}


UIControlsClass::LoadingScreen::LoadingScreen(char const* entry)
{
    static char buffer[1024];
    std::strncpy(buffer, entry, sizeof(buffer));

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
    if (House <= HOUSE_FIRST) {
        DEBUG_ERROR("Invalid loading screen entry: \"%s\"!", buffer);
        return;
    }

    token = std::strtok(nullptr, ",");
    Filename = token;
    if (Filename.empty()) {
        DEBUG_ERROR("Invalid loading screen entry: \"%s\"!", buffer);
        return;
    }

    token = std::strtok(nullptr, ",");
    int width = parse_number(token);
    if (width <= 0) {
        DEBUG_ERROR("Invalid loading screen entry: \"%s\"!", buffer);
        return;
    }

    int height = 0;
    token = std::strtok(nullptr, ",");
    height = parse_number(token);
    if (height <= 0) {
        DEBUG_ERROR("Invalid loading screen entry: \"%s\"!", buffer);
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


/**
 *  Helper to get the group number drawing offset based on the object type.
 */
TPoint2D<int> UIControlsClass::Get_Group_Number_Offset(RTTIType type, bool has_pip) const
{
    switch (type) {
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
 *  Helper to get the veterancy pip drawing offset based on the object type.
 */
TPoint2D<int> UIControlsClass::Get_Veterancy_Pip_Offset(RTTIType type) const
{
    switch (type) {
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
 *  Helper to get the special pip drawing offset based on the object type.
 */
TPoint2D<int> UIControlsClass::Get_Special_Pip_Offset(RTTIType type) const
{
    switch (type) {
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
    std::snprintf(name, sizeof(name), "LOAD%03d%c.PCX", size.Size.Y, letter);

    return LoadingScreen {house, name, size};
}
