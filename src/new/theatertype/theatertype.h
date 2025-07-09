/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          THEATERTYPE.H
 *
 *  @authors       CCHyper
 *
 *  @brief         Map theater type class.
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
#include "vector.h"
#include "tibsun_defines.h"


class CCINIClass;


class TheaterTypeClass
{
    private:
        TheaterTypeClass();

    public:
        TheaterTypeClass(const char *name);
        TheaterTypeClass(const char *name, const char *root, const char *iso_root, const char *suffix,
            const char *mm_suffix, char letter, bool arctic, bool ice_growth, bool vein_growth, bool mapgen_allowed,
            bool mapgen_veins, float low_brightness_1, float low_brightness_2, float high_brightness_1, float high_brightness_2);
        TheaterTypeClass(const NoInitClass &noinit);
        ~TheaterTypeClass();

        bool Read_INI(CCINIClass &ini);

        static bool One_Time();

        static bool Read_Theaters_INI(CCINIClass &ini);
        
        static const TheaterTypeClass &As_Reference(TheaterType type);
        static const TheaterTypeClass *As_Pointer(TheaterType type);
        static const TheaterTypeClass &As_Reference(const char *name);
        static const TheaterTypeClass *As_Pointer(const char *name);
        static TheaterType From_Name(const char *name);
        static const char *Name_From(TheaterType type);

        static const char *Root_From(TheaterType type);
        static const char *IsoRoot_From(TheaterType type);
        static const char *Suffix_From(TheaterType type);
        static const char *MMSuffix_From(TheaterType type);
        static const char ImageLetter_From(TheaterType type);

        static bool Is_Arctic(TheaterType type);
        static bool Ice_Growth_Allowed(TheaterType type);
        static bool Vein_Growth_Allowed(TheaterType type);
        static bool Allowed_In_Map_Generator(TheaterType type);
        static bool Veins_Allowed_In_Map_Generator(TheaterType type);
        static float Low_Radar_Brightness(TheaterType type);
        static float High_Radar_Brightness(TheaterType type);

    private:
        static const TheaterTypeClass *Find_Or_Make(const char *name);
    
    private:
        /**
         *  The name of this theater. This is the name as defined in the INI
         *  and is also used for identifying the theater for a scenario.
         */
        char Name[16];
        
        /**
         *  The root name for the theater data and control INI.
         */
        char Root[10];
        
        /**
         *  The root name for the theater tileset data.
         */
        char IsoRoot[10];
        
        /**
         *  The file suffix for loading the theaters tilesets.
         */
        char Suffix[4];
        
        /**
         *  The suffix for the "marble madness" tiles.
         */
        char MMSuffix[4];

        /**
         *  The theater image letter, used to fixup graphics.
         */
        char ImageLetter;

        /**
         *  The name of this theater as it appears in the map generator.
         */
        char BiomeName[32];

        /**
         *  Is this theater the "arctic" theater set? (used for deciding which occupy bits are used).
         */
        bool IsArctic;
        
        /**
         *  Is the ice growth logic enabled for this theater?
         */
        bool IsIceGrowthEnabled;
        
        /**
         *  Is the vein growth logic enabled for this theater?
         */
        bool IsVeinGrowthEnabled;
        
        /**
         *  Is this theater allowed to be used in the map generator?
         */
        bool IsAllowedInMapGenerator;
        
        /**
         *  Should the map generator produce veinholes for this theater?
         */
        bool IsGenerateVeinholesInMapGenerator;

    public:
        /**
         *  The brightness of the lowest height level cells when drawn on the radar.
         */
        float LowRadarBrightness1;
        float LowRadarBrightness2;

        /**
         *  The brightness of the highest height level cells when drawn on the radar.
         */
        float HighRadarBrightness1;
        float HighRadarBrightness2;
};
