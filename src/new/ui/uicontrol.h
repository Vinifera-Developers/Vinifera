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
#include "tibsun_defines.h"
#include "tpoint.h"
#include "typelist.h"


struct IStream;
class CCINIClass;
class NoInitClass;


class UIControlsClass
{
    public:
        UIControlsClass();
        UIControlsClass(const NoInitClass &noinit);
        ~UIControlsClass();

        HRESULT Load(IStream *pStm);
        HRESULT Save(IStream *pStm, BOOL fClearDirty);
        int Size_Of() const;

        bool Read_INI(CCINIClass &ini);

        /**
         *  Helper to get the group number drawing offset based on the object type.
         */
        TPoint2D<int> Get_Group_Number_Offset(RTTIType type, bool has_pip) const
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
                return TPoint2D<int>();
            }
        }

        /**
         *  Helper to get the veterancy pip drawing offset based on the object type.
         */
        TPoint2D<int> Get_Veterancy_Pip_Offset(RTTIType type) const
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
                return TPoint2D<int>();
            }
        }

        /**
         *  Helper to get the special pip drawing offset based on the object type.
         */
        TPoint2D<int> Get_Special_Pip_Offset(RTTIType type) const
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
                return TPoint2D<int>();
            }
        }

    public:
        /**
         *  Health bar draw positions.
         */
        TPoint2D<int> UnitHealthBarDrawPos;
        TPoint2D<int> InfantryHealthBarDrawPos;

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
        TPoint2D<int> UnitGroupNumberOffset;
        TPoint2D<int> InfantryGroupNumberOffset;
        TPoint2D<int> BuildingGroupNumberOffset;
        TPoint2D<int> AircraftGroupNumberOffset;
        TPoint2D<int> UnitWithPipGroupNumberOffset;
        TPoint2D<int> InfantryWithPipGroupNumberOffset;
        TPoint2D<int> BuildingWithPipGroupNumberOffset;
        TPoint2D<int> AircraftWithPipGroupNumberOffset;
        TPoint2D<int> UnitVeterancyPipOffset;
        TPoint2D<int> InfantryVeterancyPipOffset;
        TPoint2D<int> BuildingVeterancyPipOffset;
        TPoint2D<int> AircraftVeterancyPipOffset;
        TPoint2D<int> UnitSpecialPipOffset;
        TPoint2D<int> InfantrySpecialPipOffset;
        TPoint2D<int> BuildingSpecialPipOffset;
        TPoint2D<int> AircraftSpecialPipOffset;

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
};

extern UIControlsClass *UIControls;
