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
    TargetLaserDropShadowColor{ 0, 0, 0 }
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

    return true;
}
