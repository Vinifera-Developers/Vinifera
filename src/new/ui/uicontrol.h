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

#include "always.h"
#include "extension_globals.h"
#include "optionsext.h"
#include "point.h"
#include "tibsun_defines.h"
#include "typelist.h"

#include <string>
#include <vector>


struct IStream;
class CCINIClass;
class NoInitClass;


class UIControlsClass
{
public:
    UIControlsClass();
    UIControlsClass(const NoInitClass& noinit);
    ~UIControlsClass();

    bool Read_INI(CCINIClass& ini);
    bool Read_Sidebar_INI(CCINIClass& ini, SideType side);

    enum SidebarPresetType {
        PRESET_VANILLA,
        PRESET_4TABS,
        PRESET_4TABSWIDE,
        PRESET_6TABS
    };

    void Set_Sidebar_Defaults(SidebarPresetType preset);

    /**
     *  Helper to get the group number drawing offset based on the object type.
     */
    Point2D Get_Group_Number_Offset(RTTIType type, bool has_pip) const;

    /**
     *  Helper to get the veterancy pip drawing offset based on the object type.
     */
    Point2D Get_Veterancy_Pip_Offset(RTTIType type) const;

    /**
     *  Helper to get the special pip drawing offset based on the object type.
     */
    Point2D Get_Special_Pip_Offset(RTTIType type) const;

public:
    /**
     *  Health bar draw positions.
     */
    Point2D UnitHealthBarDrawPos;
    Point2D InfantryHealthBarDrawPos;

    /**
     *  Should the text label be drawn with an outline?
     */
    bool IsTextLabelOutline;

    /**
     *  Transparency of the text background.
     */
    unsigned TextLabelBackgroundTransparency;

    /**
     *  Customizable offsets for drawing different pips.
     */
    Point2D UnitGroupNumberOffset;
    Point2D InfantryGroupNumberOffset;
    Point2D BuildingGroupNumberOffset;
    Point2D AircraftGroupNumberOffset;
    Point2D UnitWithPipGroupNumberOffset;
    Point2D InfantryWithPipGroupNumberOffset;
    Point2D BuildingWithPipGroupNumberOffset;
    Point2D AircraftWithPipGroupNumberOffset;
    Point2D UnitVeterancyPipOffset;
    Point2D InfantryVeterancyPipOffset;
    Point2D BuildingVeterancyPipOffset;
    Point2D AircraftVeterancyPipOffset;
    Point2D UnitSpecialPipOffset;
    Point2D InfantrySpecialPipOffset;
    Point2D BuildingSpecialPipOffset;
    Point2D AircraftSpecialPipOffset;

    /**
     *  Should the tactical rubber band box be drawn with a drop shadow?
     */
    bool IsBandBoxDropShadow;

    /**
     *  Should the tactical rubber band box be drawn with a thick border?
     */
    bool IsBandBoxThick;

    /**
     *  Color to draw the tactical rubber band box with.
     */
    RGBStruct BandBoxColor;

    /**
     *  Color to draw the tactical rubber band box's shadow with.
     */
    RGBStruct BandBoxDropShadowColor;

    /**
     *  Transparency of the tactical rubber band.
     */
    unsigned BandBoxTintTransparency;

    /**
     *  Two tint colors, interpolated between based on the current ambient light level.
     */
    TypeList<RGBStruct> BandBoxTintColors;

    /**
     *  Should action lines remain visible continuously, instead of disappearing after some time?
     */
    bool IsAlwaysShowActionLines;

    /**
     *  Should movement lines be drawn with dashes?
     */
    bool IsMovementLineDashed;

    /**
     *  Should movement lines be drawn with a drop shadow?
     */
    bool IsMovementLineDropShadow;

    /**
     *  Should movement lines be drawn with a thick line?
     */
    bool IsMovementLineThick;

    /**
     *  Color to draw movement lines with.
     */
    RGBStruct MovementLineColor;

    /**
     *  Color to draw movement lines' drop shadow with.
     */
    RGBStruct MovementLineDropShadowColor;

    /**
     *  Should target lines be drawn with dashes?
     */
    bool IsTargetLineDashed;

    /**
     *  Should target lines be drawn with a drop shadow?
     */
    bool IsTargetLineDropShadow;

    /**
     *  Should target lines be drawn with a thick line?
     */
    bool IsTargetLineThick;

    /**
     *  Color to target movement lines with.
     */
    RGBStruct TargetLineColor;

    /**
     *  Color to draw target lines' drop shadow with.
     */
    RGBStruct TargetLineDropShadowColor;

    /**
     *  Should target laser be drawn with dashes?
     */
    bool IsTargetLaserDashed;

    /**
     *  Should target laser be drawn with a drop shadow?
     */
    bool IsTargetLaserDropShadow;

    /**
     *  Should target laser be drawn with a thick line?
     */
    bool IsTargetLaserThick;

    /**
     *  Color to draw the target laser with.
     */
    RGBStruct TargetLaserColor;

    /**
     *  Color to draw the target laser's drop shadow with.
     */
    RGBStruct TargetLaserDropShadowColor;

    /**
     *  Time in frames the target laser should be drawn for when the unit fires.
     */
    unsigned TargetLaserTime;

    /**
     *  Should NavCom queue lines be displayed?
     */
    bool IsShowNavComQueueLines;

    /**
     *  Should NavCom queue lines be drawn with dashes?
     */
    bool IsNavComQueueLineDashed;

    /**
     *  Should NavCom queue lines be drawn with a drop shadow?
     */
    bool IsNavComQueueLineDropShadow;

    /**
     *  Should NavCom queue lines be drawn with a thick line?
     */
    bool IsNavComQueueLineThick;

    /**
     *  Color to draw the NavCom queue lines with.
     */
    RGBStruct NavComQueueLineColor;

    /**
     *  Color to draw the NavCom queue lines' drop shadow with.
     */
    RGBStruct NavComQueueLineDropShadowColor;

    /**
     *  Should the sidebar repair, etc. buttons use the old X position, centered on the radar?
     */
    bool IsCenterSidebarButtonsOnRadar;

    struct SidebarControlsType {

        int CameoWidth = 64;
        int CameoHeight = 48;
        int CameoXSpacing = 3;
        int CameoYSpacing = 3;
        Point2D CameoNameOffset = Point2D(0, 41);
        Point2D CameoQueueCountOffset = Point2D(61, 2);
        Point2D CameoStateOffset = Point2D(33, 2);
        Point2D CameoQueueStateOffset = Point2D(0, 2);

        Point2D StripPosition = Point2D(24, 26);

        int ScrollRate = 51;

        int Get_Object_Width() const { return CameoWidth + CameoXSpacing; }
        __declspec(property(get = Get_Object_Width)) int ObjectWidth;

        int Get_Object_Height() const { return CameoHeight + CameoYSpacing; }
        __declspec(property(get = Get_Object_Height)) int ObjectHeight;

        Point2D PowerPosition = Point2D(8, 25);
        int PowerWidth = 12;
        int PowerHeightFudge = 1;
        int PowerPipHeight = 4;

        int RadarHeight = 134;
        int RadarTopHeight = 0;
        Rect RadarMapRect = Rect(15, 12, 140, 108);

        Point2D RepairButtonPosition = Point2D(31, -9);
        Point2D SellButtonPosition = Point2D(58, -9);
        Point2D PowerButtonPosition = Point2D(85, -9);
        Point2D WaypointButtonPosition = Point2D(112, -9);

        std::vector<Point2D> TabButtonOffset;

        Point2D UpButtonOffset = Point2D(2, -1);
        Point2D DownButtonOffset = Point2D(31, -1);

        std::string StateColor = "LightBlue";
        std::string OnHoldColor = "LightGrey";

    } SidebarControls;
};

extern UIControlsClass* UIControls;
