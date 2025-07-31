/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EBOLT.CPP
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
#include "ebolt.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "technoext.h"
#include "techno.h"
#include "particlesys.h"
#include "weapontype.h"
#include "weapontypeext.h"
#include "tactical.h"
#include "rgb.h"
#include "dsurface.h"
#include "rules.h"
#include "random.h"
#include "wwmath.h"
#include "clipline.h"
#include "extension.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  Class constructor
 * 
 *  @author: CCHyper
 */
EBoltClass::EBoltClass() :
    StartCoord(),
    EndCoord(),
    ZAdjust(0),
    Deviation(EBOLT_DEFAULT_DEVIATION),
    Source(nullptr),
    Weapon(nullptr),
    WeaponSlot(WEAPON_SLOT_PRIMARY),
    Lifetime(EBOLT_DEFAULT_LIFETIME),
    IterationCount(EBOLT_DEFAULT_INTERATIONS),
    LineColor1(EBOLT_DEFAULT_COLOR_1),
    LineColor2(EBOLT_DEFAULT_COLOR_2),
    LineColor3(EBOLT_DEFAULT_COLOR_3),
    LineSegmentCount(EBOLT_DEFAULT_LINE_SEGEMENTS),
    LineDrawList(),
    DrawFrame(-1)
{
}


/**
 *  Class destructor
 * 
 *  @author: CCHyper
 */
EBoltClass::~EBoltClass()
{
    EBolts.Delete(this);
    Clear();
}


/**
 *  Detaches the electric bolt from its source.
 * 
 *  @author: CCHyper
 */
void EBoltClass::Clear()
{
    if (Source) {
        Extension::Fetch(Source)->ElectricBolt = nullptr;
        Source = nullptr;
    }

    LineDrawList.Clear();
}


/**
 *  Draws the electric bolt to the screen.
 * 
 *  @author: tomsons26, CCHyper
 */
void EBoltClass::Draw_It()
{
    if (DrawFrame == Frame) {

        /**
         *  This is our draw frame, so draw!
         */
        if (LineDrawList.Count()) {
            Draw_Bolts();
        }
    
    } else {

        /**
         *  Clear previous lines, we are about to plot a new set.
         */
        LineDrawList.Clear();

        for (int i = 0; i < IterationCount; ++i) {
            if (Lifetime) {

                Point2D pixel_start;
                Point2D pixel_end;

                TacticalMap->Coord_To_Pixel(StartCoord, pixel_start);
                TacticalMap->Coord_To_Pixel(EndCoord, pixel_end);

                if (Clip_Line(pixel_start, pixel_end, TacticalRect)) {
                    Plot_Bolt(StartCoord, EndCoord);
                }
            }
        }

        /**
         *  Draw the initial set of lines.
         */
        if (LineDrawList.Count()) {
            Draw_Bolts();
        }

        /**
         *  Update the lifetime.
         */
        --Lifetime;

        DrawFrame = Frame;
    }
}


/**
 *  
 * 
 *  @author: CCHyper
 */
void EBoltClass::Create(Coord &start, Coord &end, int z_adjust)
{
    StartCoord = start;
    EndCoord = end;

    ZAdjust = z_adjust;

    EBolts.Add(this);

    /**
     *  Spawn a spark particle at the destination of the electric bolt.
     */
    ParticleSystemClass *particlesys = new ParticleSystemClass(Rule->DefaultSparkSystem, end);
    ASSERT(particlesys != nullptr);
}


/**
 *  The coordinate of the source of this electric bolt.
 * 
 *  @author: CCHyper
 */
Coord EBoltClass::Source_Coord() const
{
    Coord coord = COORD_NONE;
    if (Source) {
        coord = Source->Fire_Coord(WeaponSlot);
    }
    return coord;
}


/**
 *  Assigns the firing source object and weapon for this electric bolt.
 * 
 *  @author: CCHyper
 */
void EBoltClass::Set_Properties(TechnoClass *techno, const WeaponTypeClass *weapon, WeaponSlotType slot)
{
    if (techno) {
        if (techno->IsActive) {
            if (!techno->IsInLimbo) {
                Source = techno;
                Weapon = weapon;
                WeaponSlot = slot;

                /**
                 *  Copy the color overrides from the firing objects weapon.
                 */
                WeaponTypeClassExtension *weapontypeext = Extension::Fetch(weapon);

                LineColor1 = weapontypeext->ElectricBoltColor1;
                LineColor2 = weapontypeext->ElectricBoltColor2;
                LineColor3 = weapontypeext->ElectricBoltColor3;
                IterationCount = weapontypeext->ElectricBoltIterationCount;
                LineSegmentCount = weapontypeext->ElectricBoltSegmentCount;
                Lifetime = std::clamp(weapontypeext->ElectricBoltLifetime, 0, EBOLT_MAX_LIFETIME);
                Deviation = weapontypeext->ElectricBoltDeviation;
            }
        }
    }
}


/**
 *  Draws all active electric bolts to the screen.
 * 
 *  @author: CCHyper
 */
void EBoltClass::Draw_All()
{
    for (int i = EBolts.Count()-1; i >= 0; --i) {
        EBoltClass *ebolt = EBolts[i];
        if (!ebolt) {
            DEV_DEBUG_WARNING("Invalid EBolt!\n");
            continue;
        }

        /**
         *  Is the source object has left the game world, remove this bolt.
         */
        if (ebolt->Source && (!ebolt->Source->IsActive || ebolt->Source->IsInLimbo)) {
            delete ebolt;
            continue;
        }

        /**
         *  Update the source coord.
         */
        Coord coord = ebolt->Source_Coord();
        if (coord != COORD_NONE) {
            ebolt->StartCoord = coord;
        }

        /**
         *  Draw the current bolt line-set.
         */
        ebolt->Draw_It();

        /**
         *  Electric bolt has expired, delete it.
         */
        if (ebolt->Lifetime <= 0) {
            delete ebolt;
        }
    }
}


/**
 *  Removes all electric bolts from the game world.
 * 
 *  @author: CCHyper
 */
void EBoltClass::Clear_All()
{
    while (EBolts.Count()) {
        delete EBolts[0];
    }
}


/**
 *  Plots the complete electric bolt from source to target.
 * 
 *  @author: tomsons26, CCHyper
 */
void EBoltClass::Plot_Bolt(Coord &start, Coord &end)
{
    struct EBoltPlotStruct
    {
        Coord StartCoords[EBOLT_DEFAULT_SEGMENT_LINES];
        Coord EndCoords[EBOLT_DEFAULT_SEGMENT_LINES];
        int Distance;
        int Deviation;
        int StartZ;
        int EndZ;

        bool operator==(const EBoltPlotStruct &that) const { return std::memcmp(this, &that, sizeof(EBoltPlotStruct)) == 0; }
        bool operator!=(const EBoltPlotStruct &that) const { return std::memcmp(this, &that, sizeof(EBoltPlotStruct)) != 0; }
    };

    int SEGEMENT_COORDS_SIZE = sizeof(Coord)*EBOLT_DEFAULT_SEGMENT_LINES;

    VectorClass<EBoltPlotStruct> ebolt_plots(LineSegmentCount);

    Coord start_coords[EBOLT_DEFAULT_SEGMENT_LINES];
    Coord end_coords[EBOLT_DEFAULT_SEGMENT_LINES];
    Coord working_coords[EBOLT_DEFAULT_SEGMENT_LINES];

    int deviation_values[6];

    bool init_deviation_values = true;
    int plot_index = 0;

    /**
     *  Check to make sure there is actual distance between the two coords.
     */
    int distance = Distance(start, end);
    if (distance) {

        for (int i = 0; i < EBOLT_DEFAULT_SEGMENT_LINES; ++i) {
            end_coords[i] = end;
            start_coords[i] = start;
        }

        int line_start_z = ZAdjust;
        int line_end_z = 0;

        int dist_a = (102 * distance / CELL_LEPTON_W);

        /**
         *  Max distance from line center, with "Deviation" as delta.
         */
        int desired_deviation = 23;
        int line_deviation = ((desired_deviation * Deviation) * distance / CELL_LEPTON_W);

        while (true) {

            while (distance > (CELL_LEPTON_W/4) && plot_index < ebolt_plots.Length()) {

                for (int i = 0; i < EBOLT_DEFAULT_SEGMENT_LINES; ++i) {
                    working_coords[i].X = (end_coords[i].X + start_coords[i].X) / 2;
                    working_coords[i].Y = (end_coords[i].Y + start_coords[i].Y) / 2;
                    working_coords[i].Z = (end_coords[i].Z + start_coords[i].Z) / 2;
                }

                /**
                 *  Initialises the line deviation values.
                 */
                if (init_deviation_values) {

                    for (int i = 0; i < std::size(deviation_values); ++i) {
                        deviation_values[i] = (WWMath::Sin((double)Sim_Random_Pick(0, 256) * WWMATH_PI / (double)(i + 7)) * (double)line_deviation);
                    }

                    for (int i = 0; i < EBOLT_DEFAULT_SEGMENT_LINES; ++i) {
                        working_coords[i].X += deviation_values[0] + deviation_values[3];
                        working_coords[i].Y += deviation_values[1] + deviation_values[5];
                        working_coords[i].Z += (deviation_values[2] + deviation_values[4] + 2 * line_deviation) / 2;
                    }

                    init_deviation_values = false;
                }

                if (distance <= (CELL_LEPTON_W/2)) {
                    working_coords[0].X += 2 * line_deviation * Sim_Random_Pick(-1, 1);
                    working_coords[0].Y += 2 * line_deviation * Sim_Random_Pick(-1, 1);
                    working_coords[0].Z += 2 * line_deviation * Sim_Random_Pick(-1, 1);
                } else {
                    working_coords[0].X += Sim_Random_Pick(-line_deviation, line_deviation);
                    working_coords[0].Y += Sim_Random_Pick(-line_deviation, line_deviation);
                    working_coords[0].Z += Sim_Random_Pick(-line_deviation, line_deviation);
                }

                if (distance > dist_a) {
                    for (int i = 1; i < EBOLT_DEFAULT_SEGMENT_LINES; ++i) {
                        working_coords[i].X = working_coords[0].X + (Sim_Random_Pick(-line_deviation, line_deviation) / 2);
                        working_coords[i].Y = working_coords[0].Y + (Sim_Random_Pick(-line_deviation, line_deviation) / 2);
                        working_coords[i].Z = working_coords[0].Z + (Sim_Random_Pick(-line_deviation, line_deviation) / 2);

                    }

                } else {
                    for (int i = 1; i < EBOLT_DEFAULT_SEGMENT_LINES; ++i) {
                        working_coords[i].X += Sim_Random_Pick(-line_deviation, line_deviation);
                        working_coords[i].Y += Sim_Random_Pick(-line_deviation, line_deviation);
                        working_coords[i].Z += Sim_Random_Pick(-line_deviation, line_deviation);
                    }
                }

                line_deviation /= 2;
                distance /= 2;

                EBoltPlotStruct &plot = ebolt_plots[plot_index];

                std::memcpy(plot.StartCoords, working_coords, SEGEMENT_COORDS_SIZE);
                std::memcpy(plot.EndCoords, end_coords, SEGEMENT_COORDS_SIZE);
                std::memcpy(end_coords, working_coords, SEGEMENT_COORDS_SIZE);

                plot.Distance = distance;
                plot.Deviation = line_deviation;
                plot.StartZ = (line_end_z + line_start_z) / 2;
                plot.EndZ = line_end_z;

                line_end_z = (line_end_z + line_start_z) / 2;

                ++plot_index;
            }

            /**
             *  Add the line segments to the draw list.
             */
            Add_Plot_Line(start_coords[1], end_coords[1], LineColor2, line_start_z, line_end_z);
            Add_Plot_Line(start_coords[2], end_coords[2], LineColor3, line_start_z, line_end_z);
            Add_Plot_Line(start_coords[0], end_coords[0], LineColor1, line_start_z, line_end_z);

            if (--plot_index < 0) {
                break;
            }

            EBoltPlotStruct &plot = ebolt_plots[plot_index];

            distance = plot.Distance;
            line_deviation = plot.Deviation;
            line_start_z = plot.StartZ;
            line_end_z = plot.EndZ;

            std::memcpy(start_coords, plot.StartCoords, SEGEMENT_COORDS_SIZE);
            std::memcpy(end_coords, plot.EndCoords, SEGEMENT_COORDS_SIZE);
        }

    }
}


/**
 *  Draw all pending bolts to the game surface.
 * 
 *  @author: tomsons26, CCHyper
 */
void EBoltClass::Draw_Bolts()
{
    for (int i = 0; i < LineDrawList.Count(); ++i) {
        LineDrawDataStruct &data = LineDrawList[i];

        Point2D start_pixel;
        Point2D end_pixel;

        TacticalMap->Coord_To_Pixel(data.Start, start_pixel);
        TacticalMap->Coord_To_Pixel(data.End, end_pixel);

        int start_z = data.StartZ - TacticalMap->Z_Lepton_To_Pixel(data.Start.Z) - 2;
        int end_z = data.EndZ - TacticalMap->Z_Lepton_To_Pixel(data.End.Z) - 2;

        unsigned color = DSurface::RGB_To_Pixel(data.Color.Red, data.Color.Green, data.Color.Blue);

        CompositeSurface->Draw_Line_entry_34(TacticalRect, start_pixel, end_pixel, color, start_z, end_z);
    }
}
