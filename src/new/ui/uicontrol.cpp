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
#include "uicontrol.h"
#include "ccini.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "side.h"


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
    IsCenterSidebarButtonsOnRadar(false)
{
    BandBoxTintColors.Add(RGBStruct{ 0, 0, 0 });
    BandBoxTintColors.Add(RGBStruct{ 255, 255, 255 });
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
    TextLabelBackgroundTransparency = ini.Get_Int_Clamp(INGAME, "TextLabelBackgroundTransparency", 0, 100, TextLabelBackgroundTransparency);

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
    BandBoxColor = ini.Get_RGB(INGAME, "BandBoxColor", BandBoxColor);
    BandBoxDropShadowColor = ini.Get_RGB(INGAME, "BandBoxDropShadowColor", BandBoxDropShadowColor);
    BandBoxTintTransparency = ini.Get_Int_Clamp(INGAME, "BandBoxTintTransparency", 0, 100, BandBoxTintTransparency);
    BandBoxTintColors = ini.Get_RGBs(INGAME, "BandBoxTintColors", BandBoxTintColors);

    ASSERT_PRINT(BandBoxTintColors.Count() == 2, "BandBoxTintColors must contain two valid entries!");

    IsAlwaysShowActionLines = ini.Get_Bool(INGAME, "AlwaysShowActionLines", IsAlwaysShowActionLines);
    IsMovementLineDashed = ini.Get_Bool(INGAME, "MovementLineDashed", IsMovementLineDashed);
    IsMovementLineDropShadow = ini.Get_Bool(INGAME, "MovementLineDropShadow", IsMovementLineDropShadow);
    IsMovementLineThick = ini.Get_Bool(INGAME, "MovementLineThick", IsMovementLineThick);
    MovementLineColor = ini.Get_RGB(INGAME, "MovementLineColor", MovementLineColor);
    MovementLineDropShadowColor = ini.Get_RGB(INGAME, "MovementLineDropShadowColor", MovementLineDropShadowColor);

    IsTargetLineDashed = ini.Get_Bool(INGAME, "TargetLineDashed", IsTargetLineDashed);
    IsTargetLineDropShadow = ini.Get_Bool(INGAME, "TargetLineDropShadow", IsTargetLineDropShadow);
    IsTargetLineThick = ini.Get_Bool(INGAME, "TargetLineThick", IsTargetLineThick);
    TargetLineColor = ini.Get_RGB(INGAME, "TargetLineColor", TargetLineColor);
    TargetLineDropShadowColor = ini.Get_RGB(INGAME, "TargetLineDropShadowColor", TargetLineDropShadowColor);

    IsTargetLaserDashed = ini.Get_Bool(INGAME, "TargetLaserDashed", IsTargetLaserDashed);
    IsTargetLaserDropShadow = ini.Get_Bool(INGAME, "TargetLaserDropShadow", IsTargetLaserDropShadow);
    IsTargetLaserThick = ini.Get_Bool(INGAME, "TargetLaserThick", IsTargetLaserThick);
    TargetLaserColor = ini.Get_RGB(INGAME, "TargetLaserColor", TargetLaserColor);
    TargetLaserDropShadowColor = ini.Get_RGB(INGAME, "TargetLaserDropShadowColor", TargetLaserDropShadowColor);
    TargetLaserTime = ini.Get_Int(INGAME, "TargetLaserTime", TargetLaserTime);

    IsShowNavComQueueLines = ini.Get_Bool(INGAME, "ShowNavComQueueLines", IsShowNavComQueueLines);
    IsNavComQueueLineDashed = ini.Get_Bool(INGAME, "NavComQueueLineDashed", IsNavComQueueLineDashed);
    IsNavComQueueLineDropShadow = ini.Get_Bool(INGAME, "NavComQueueLineDropShadow", IsNavComQueueLineDropShadow);
    IsNavComQueueLineThick = ini.Get_Bool(INGAME, "NavComQueueLineThick", IsNavComQueueLineThick);
    NavComQueueLineColor = ini.Get_RGB(INGAME, "NavComQueueLineColor", NavComQueueLineColor);
    NavComQueueLineDropShadowColor = ini.Get_RGB(INGAME, "NavComQueueLineDropShadowColor", NavComQueueLineDropShadowColor);

    IsCenterSidebarButtonsOnRadar = ini.Get_Bool(INGAME, "CenterSidebarButtonsOnRadar", IsCenterSidebarButtonsOnRadar);

    return true;
}

bool UIControlsClass::Read_Sidebar_INI(CCINIClass& ini, SideType side)
{
    char section[128] = "";
    std::snprintf(section, sizeof(section), "Sidebar%d", side);

    char buffer[512];
    ini.Get_String(section, "Preset", "", buffer, sizeof(buffer));

    SidebarPresetType preset = PRESET_VANILLA;

    if (strnicmp(buffer, "Vanilla", sizeof(buffer)) == 0) {
        preset = PRESET_VANILLA;
    } else if (strnicmp(buffer, "4Tabs", sizeof(buffer)) == 0) {
        preset = PRESET_4TABS;
    } else if (strnicmp(buffer, "4TabsWide", sizeof(buffer)) == 0) {
        preset = PRESET_4TABSWIDE;
    } else if (strnicmp(buffer, "6Tabs", sizeof(buffer)) == 0) {
        preset = PRESET_6TABS;
    }

    Set_Sidebar_Defaults(preset);

    SidebarControls.CameoWidth = ini.Get_Int(section, "CameoWidth", SidebarControls.CameoWidth);
    SidebarControls.CameoHeight = ini.Get_Int(section, "CameoHeight", SidebarControls.CameoHeight);
    SidebarControls.CameoXSpacing = ini.Get_Int(section, "CameoXSpacing", SidebarControls.CameoXSpacing);
    SidebarControls.CameoYSpacing = ini.Get_Int(section, "CameoYSpacing", SidebarControls.CameoYSpacing);
    SidebarControls.CameoNameOffset = ini.Get_Point(section, "CameoNameOffset", SidebarControls.CameoNameOffset);
    SidebarControls.CameoQueueCountOffset = ini.Get_Point(section, "CameoQueueCountOffset", SidebarControls.CameoQueueCountOffset);
    SidebarControls.CameoStateOffset = ini.Get_Point(section, "CameoStateOffset", SidebarControls.CameoStateOffset);
    SidebarControls.CameoQueueStateOffset = ini.Get_Point(section, "CameoQueueStateOffset", SidebarControls.CameoQueueStateOffset);
    SidebarControls.StripPosition = ini.Get_Point(section, "StripPosition", SidebarControls.StripPosition);
    SidebarControls.ScrollRate = ini.Get_Int(section, "ScrollRate", SidebarControls.ScrollRate);

    SidebarControls.PowerPosition = ini.Get_Point(section, "PowerPosition", SidebarControls.PowerPosition);
    SidebarControls.PowerWidth = ini.Get_Int(section, "PowerWidth", SidebarControls.PowerWidth);
    SidebarControls.PowerHeightFudge = ini.Get_Int(section, "PowerHeightFudge", SidebarControls.PowerHeightFudge);
    SidebarControls.PowerPipHeight = ini.Get_Int(section, "PowerPipHeight", SidebarControls.PowerPipHeight);

    SidebarControls.RadarHeight = ini.Get_Int(section, "RadarHeight", SidebarControls.RadarHeight);
    SidebarControls.RadarTopHeight = ini.Get_Int(section, "RadarTopHeight", SidebarControls.RadarTopHeight);

    if (ini.Is_Present(section, "RadarMapRect")) { // seems to be bugged and instead of using the defvalue returns (0,0,0,0) if not present
        SidebarControls.RadarMapRect = ini.Get_Rect(section, "RadarMapRect", SidebarControls.RadarMapRect);
    }

    SidebarControls.RepairButtonPosition = ini.Get_Point(section, "RepairButtonPosition", SidebarControls.RepairButtonPosition);
    SidebarControls.SellButtonPosition = ini.Get_Point(section, "SellButtonPosition", SidebarControls.SellButtonPosition);
    SidebarControls.PowerButtonPosition = ini.Get_Point(section, "PowerButtonPosition", SidebarControls.PowerButtonPosition);
    SidebarControls.WaypointButtonPosition = ini.Get_Point(section, "WaypointButtonPosition", SidebarControls.WaypointButtonPosition);

    SidebarControls.TabButtonOffset.resize(OptionsExtension->SidebarControls.Tabs, Point2D(0, 0));
    for (int i = 0; i < OptionsExtension->SidebarControls.Tabs; i++) {
        char key[32];
        std::snprintf(key, sizeof(key), "TabButton%dOffset", i);
        SidebarControls.TabButtonOffset[i] = ini.Get_Point(section, key, SidebarControls.TabButtonOffset[i]);
    }

    SidebarControls.UpButtonOffset = ini.Get_Point(section, "UpButtonOffset", SidebarControls.UpButtonOffset);
    SidebarControls.DownButtonOffset = ini.Get_Point(section, "DownButtonOffset", SidebarControls.DownButtonOffset);

    if (ini.Get_String(section, "StateColor", SidebarControls.StateColor.c_str(), buffer, sizeof(buffer)) > 0) {
        SidebarControls.StateColor = buffer;
    }

    if (ini.Get_String(section, "OnHoldColor", SidebarControls.OnHoldColor.c_str(), buffer, sizeof(buffer)) > 0) {
        SidebarControls.OnHoldColor = buffer;
    }

    return true;
}

void UIControlsClass::Set_Sidebar_Defaults(SidebarPresetType preset)
{
    switch (preset) {
    case PRESET_VANILLA:
        break;

    case PRESET_4TABS: {
        SidebarControls.StripPosition = Point2D(24, 54);

        SidebarControls.RepairButtonPosition = Point2D(31, -9);
        SidebarControls.SellButtonPosition = Point2D(58, -9);
        SidebarControls.PowerButtonPosition = Point2D(85, -9);
        SidebarControls.WaypointButtonPosition = Point2D(112, -9);

        SidebarControls.TabButtonOffset.resize(OptionsExtension->SidebarControls.Tabs, Point2D(0, 0));
        SidebarControls.TabButtonOffset[0] = Point2D(20, 27);
        SidebarControls.TabButtonOffset[1] = Point2D(55, 27);
        SidebarControls.TabButtonOffset[2] = Point2D(90, 27);
        SidebarControls.TabButtonOffset[3] = Point2D(125, 27);

        SidebarControls.UpButtonOffset = Point2D(2, -1);
        SidebarControls.DownButtonOffset = Point2D(68, -1);

        SidebarControls.PowerPosition = Point2D(8, 53);
        break;
    }

    case PRESET_4TABSWIDE: {
        SidebarControls.StripPosition = Point2D(24, 54);

        SidebarControls.RepairButtonPosition = Point2D(31, -9);
        SidebarControls.SellButtonPosition = Point2D(58, -9);
        SidebarControls.PowerButtonPosition = Point2D(85, -9);
        SidebarControls.WaypointButtonPosition = Point2D(112, -9);

        SidebarControls.TabButtonOffset.resize(OptionsExtension->SidebarControls.Tabs, Point2D(0, 0));
        SidebarControls.TabButtonOffset[0] = Point2D(20, 27);
        SidebarControls.TabButtonOffset[1] = Point2D(55, 27);
        SidebarControls.TabButtonOffset[2] = Point2D(90, 27);
        SidebarControls.TabButtonOffset[3] = Point2D(125, 27);

        SidebarControls.UpButtonOffset = Point2D(2, -1);
        SidebarControls.DownButtonOffset = Point2D(68, -1);

        SidebarControls.PowerPosition = Point2D(8, 53);
        SidebarControls.PowerHeightFudge = 3;
        break;
    }

    case PRESET_6TABS: {
        SidebarControls.StripPosition = Point2D(24, 54);

        SidebarControls.RepairButtonPosition = Point2D(31, -9);
        SidebarControls.SellButtonPosition = Point2D(58, -9);
        SidebarControls.PowerButtonPosition = Point2D(85, -9);
        SidebarControls.WaypointButtonPosition = Point2D(112, -9);

        SidebarControls.TabButtonOffset.resize(OptionsExtension->SidebarControls.Tabs, Point2D(0, 0));
        SidebarControls.TabButtonOffset[0] = Point2D(20, 27);
        SidebarControls.TabButtonOffset[1] = Point2D(55, 27);
        SidebarControls.TabButtonOffset[2] = Point2D(90, 27);
        SidebarControls.TabButtonOffset[3] = Point2D(125, 27);
        SidebarControls.TabButtonOffset[4] = Point2D(160, 27);
        SidebarControls.TabButtonOffset[5] = Point2D(195, 27);

        SidebarControls.UpButtonOffset = Point2D(2, -1);
        SidebarControls.DownButtonOffset = Point2D(68, -1);

        SidebarControls.PowerPosition = Point2D(8, 53);
        SidebarControls.PowerHeightFudge = 3;

        SidebarControls.RadarHeight = 188;
        SidebarControls.RadarMapRect = Rect(15, 12, 196, 151);
        break;
    }
    }
}


Point2D UIControlsClass::Get_Group_Number_Offset(RTTIType type, bool has_pip) const
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
        return Point2D(0, 0);
    }
}

/**
 *  Helper to get the veterancy pip drawing offset based on the object type.
 */
Point2D UIControlsClass::Get_Veterancy_Pip_Offset(RTTIType type) const
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
        return Point2D(0, 0);
    }
}

/**
 *  Helper to get the special pip drawing offset based on the object type.
 */
Point2D UIControlsClass::Get_Special_Pip_Offset(RTTIType type) const
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
        return Point2D(0, 0);
    }
}
