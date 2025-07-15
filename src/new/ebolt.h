/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EBOLT.H
 *
 *  @author        CCHyper, tomsons26
 *
 *  @brief         Graphical electric bolts for weapons.
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
#include "rgb.h"
#include "vector.h"
#include "tibsun_defines.h"


class TechnoClass;
class WeaponTypeClass;


/**
 *  Default values for the drawing of electric bolts.
 */
#define EBOLT_DEFAULT_DEVIATION         1.0f
#define EBOLT_DEFAULT_INTERATIONS       1
#define EBOLT_DEFAULT_LINE_SEGEMENTS    8
#define EBOLT_DEFAULT_SEGMENT_LINES     3
#define EBOLT_DEFAULT_LIFETIME          17
#define EBOLT_MAX_LIFETIME              60
#define EBOLT_DEFAULT_COLOR_1           RGBClass(255,255,255)    // White
#define EBOLT_DEFAULT_COLOR_2           RGBClass(82,81,255)      // Dark Blue
#define EBOLT_DEFAULT_COLOR_3           RGBClass(82,81,255)      // Dark Blue


class EBoltClass
{
    public:
        EBoltClass();
        ~EBoltClass();

        void Draw_It();
        void Create(Coord &start, Coord &end, int z_adjust);

        Coord Source_Coord() const;
        void Set_Properties(TechnoClass *techno, const WeaponTypeClass *weapon, WeaponSlotType slot);

        void Flag_To_Delete() { Lifetime = 0; }

        static void Draw_All();
        static void Clear_All();

    private:
        void Clear();

        void Add_Plot_Line(Coord &start, Coord &end, RGBClass &line_color, int start_z, int end_z)
        {
            LineDrawList.Add( LineDrawDataStruct { start, end, line_color, start_z, end_z } );
        }

        void Plot_Bolt(Coord &start, Coord &end);
        void Draw_Bolts();

    private:
        /**
         *  The start coordinate for this electric bolt.
         */
        Coord StartCoord;

        /**
         *  The end coordinate for this electric bolt.
         */
        Coord EndCoord;

        /**
         *  The initial z draw adjustment value.
         */
        int ZAdjust;

        /**
         *  The deviation distance. The higher this value is, the more "wild"
         *  in variation the bolts will appear.
         */
        float Deviation;

        /**
         *  The object that fired (or by other means) this electric bolt.
         */
        TechnoClass *Source;

        /**
         *  The weapon from the object that fired me, if one actually did.
         */
        const WeaponTypeClass *Weapon;
        WeaponSlotType WeaponSlot;

        /**
         *  The lifetime that this electric bolt should stay around for.
         */
        int Lifetime;

        /**
         *  How many plot and draw iterations should we perform?
         */
        int IterationCount;

        /**
         *  The number of line segments this electric bolt should draw.
         */
        int LineCount;

        /**
         *  Line segment colors, copied from the firing object's weapon on creation.
         */
        RGBClass LineColor1;
        RGBClass LineColor2;
        RGBClass LineColor3;

        /**
         *  How many segment blocks this electric bolt is made up from.
         */
        int LineSegmentCount;

        /**
         *  The list of pending lines to draw.
         */
        struct LineDrawDataStruct
        {
            Coord Start;
            Coord End;
            RGBClass Color;
            int StartZ;
            int EndZ;

            bool operator==(const LineDrawDataStruct &that) const { return std::memcmp(this, &that, sizeof(LineDrawDataStruct)) == 0; }
            bool operator!=(const LineDrawDataStruct &that) const { return std::memcmp(this, &that, sizeof(LineDrawDataStruct)) != 0; }
        };
        DynamicVectorClass<LineDrawDataStruct> LineDrawList;

        /**
         *  The frame in which we should draw on. This helps clamp the drawing
         *  to the games internal frame tick.
         */
        int DrawFrame;
};
