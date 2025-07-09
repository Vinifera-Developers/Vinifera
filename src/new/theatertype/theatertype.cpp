/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          THEATERTYPE.CPP
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
#include "theatertype.h"
#include "vinifera_globals.h"
#include "ccini.h"
#include "colorscheme.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <string>


/**
 *  Default class constructor.
 * 
 *  #NOTE: This constructor will not add the object to the vector.
 * 
 *  @author: CCHyper
 */
TheaterTypeClass::TheaterTypeClass() :
    Name("<none>"),
    Root("X"),
    IsoRoot("X"),
    Suffix("X"),
    MMSuffix("X"),
    ImageLetter('X'),
    IsArctic(false),
    IsIceGrowthEnabled(false),
    IsVeinGrowthEnabled(false),
    IsAllowedInMapGenerator(false),
    IsGenerateVeinholesInMapGenerator(false),
    LowRadarBrightness1(1.0f),
    LowRadarBrightness2(1.0f),
    HighRadarBrightness1(1.0f),
    HighRadarBrightness2(1.0f)
{
}


/**
 *  Basic constructor for theater objects.
 * 
 *  @author: CCHyper
 */
TheaterTypeClass::TheaterTypeClass(const char *name) :
    Name("<none>"),
    Root("X"),
    IsoRoot("X"),
    Suffix("X"),
    MMSuffix("X"),
    ImageLetter('X'),
    IsArctic(false),
    IsIceGrowthEnabled(false),
    IsVeinGrowthEnabled(false),
    IsAllowedInMapGenerator(false),
    IsGenerateVeinholesInMapGenerator(false),
    LowRadarBrightness1(1.0f),
    LowRadarBrightness2(1.0f),
    HighRadarBrightness1(1.0f),
    HighRadarBrightness2(1.0f)
{
    if (name[0] != '\0' && std::strlen(name)) {
        std::strncpy(Name, name, sizeof(Name));
        Name[sizeof(Name)-1] = '\0';
    }

    TheaterTypes.Add(this);
}


/**
 *  Constructor for theater objects.
 * 
 *  @author: CCHyper
 */
TheaterTypeClass::TheaterTypeClass(const char *name,
    const char *root, const char *data_root, const char *suffix, const char *mm_suffix,
    char letter, bool arctic, bool ice_growth, bool vein_growth, bool mapgen_allowed, bool mapgen_veins,
    float low_brightness_1, float low_brightness_2, float high_brightness_1, float high_brightness_2) :

    Name("<none>"),
    Root("X"),
    IsoRoot("X"),
    Suffix("X"),
    MMSuffix("X"),
    ImageLetter('X'),
    BiomeName("<missing>"),
    IsArctic(arctic),
    IsIceGrowthEnabled(ice_growth),
    IsVeinGrowthEnabled(vein_growth),
    IsAllowedInMapGenerator(mapgen_allowed),
    IsGenerateVeinholesInMapGenerator(mapgen_veins),
    LowRadarBrightness1(low_brightness_1),
    LowRadarBrightness2(low_brightness_2),
    HighRadarBrightness1(high_brightness_1),
    HighRadarBrightness2(high_brightness_2)
{
    if (name[0] != '\0' && std::strlen(name)) {
        std::strncpy(Name, name, sizeof(Name));
        Name[sizeof(Name)-1] = '\0';
    }
    if (root[0] != '\0' && std::strlen(root)) {
        std::strncpy(Root, root, sizeof(Root));
        Root[sizeof(Root)-1] = '\0';
    }
    if (data_root[0] != '\0' && std::strlen(data_root)) {
        std::strncpy(IsoRoot, data_root, sizeof(IsoRoot));
        IsoRoot[sizeof(IsoRoot)-1] = '\0';
    }
    if (suffix[0] != '\0' && std::strlen(suffix)) {
        std::strncpy(Suffix, suffix, sizeof(Suffix));
        Suffix[sizeof(Suffix)-1] = '\0';
    }
    if (mm_suffix[0] != '\0' && std::strlen(mm_suffix)) {
        std::strncpy(MMSuffix, mm_suffix, sizeof(MMSuffix));
        MMSuffix[sizeof(MMSuffix)-1] = '\0';
    }
    if (isascii(letter)) {
        ImageLetter = std::toupper(letter);
    }
    std::strncpy(BiomeName, "New Biome", sizeof(BiomeName));
    BiomeName[sizeof(BiomeName)-1] = '\0';

    TheaterTypes.Add(this);
}


/**
 *  Explicit NoInitClass constructor.
 * 
 *  @author: CCHyper
 */
TheaterTypeClass::TheaterTypeClass(const NoInitClass &noinit)
{
}


/**
 *  Class destructor.
 * 
 *  @author: CCHyper
 */
TheaterTypeClass::~TheaterTypeClass()
{
    TheaterTypes.Delete(this);
}


/**
 *  Reads theater object data from an INI file.
 * 
 *  @author: CCHyper
 */
bool TheaterTypeClass::Read_INI(CCINIClass &ini)
{
    if (!ini.Is_Present(Name)) {
        return false;
    }

    ini.Get_String(Name, "Root", Root, Root, sizeof(Root));
    ini.Get_String(Name, "IsoRoot", IsoRoot, IsoRoot, sizeof(IsoRoot));
    ini.Get_String(Name, "Suffix", Suffix, Suffix, sizeof(Suffix));
    ini.Get_String(Name, "MMSuffix", MMSuffix, MMSuffix, sizeof(MMSuffix));

    char chr[2] = { '\0' };
    ini.Get_String(Name, "ImageLetter", chr, sizeof(chr));
    if (isascii(chr[0])) {
        ImageLetter = std::toupper(chr[0]);
    }
    
    ini.Get_String(Name, "BiomeName", BiomeName, BiomeName, sizeof(BiomeName));

    IsArctic = ini.Get_Bool(Name, "IsArctic", IsArctic);
    IsIceGrowthEnabled = ini.Get_Bool(Name, "IsIceGrowthEnabled", IsIceGrowthEnabled);
    IsVeinGrowthEnabled = ini.Get_Bool(Name, "IsVeinGrowthEnabled", IsVeinGrowthEnabled);
    IsAllowedInMapGenerator = ini.Get_Bool(Name, "IsAllowedInMapGenerator", IsAllowedInMapGenerator);
    IsGenerateVeinholesInMapGenerator = ini.Get_Bool(Name, "IsGenerateVeinholesInMapGenerator", IsGenerateVeinholesInMapGenerator);

    LowRadarBrightness1 = ini.Get_Float(Name, "LowRadarBrightness", LowRadarBrightness1);
    LowRadarBrightness2 = LowRadarBrightness1;
    HighRadarBrightness1 = ini.Get_Float(Name, "HighRadarBrightness", HighRadarBrightness1);
    HighRadarBrightness2 = HighRadarBrightness1;

    return true;
}


/**
 *  Performs one time initialization of the theater type class.
 * 
 *  @warning: Do not change this function, otherwise it will break support
 *            with the original game!
 * 
 *  @author: CCHyper
 */
bool TheaterTypeClass::One_Time()
{
    TheaterTypeClass *theater = nullptr;

    /**
     *  Create the default Temperate theater control.
     */
    theater = new TheaterTypeClass("TEMPERATE",
                                   "TEMPERAT",
                                   "ISOTEMP",
                                   "TEM",
                                   "MMT",
                                   'T',
                                   false,
                                   false,
                                   true,
                                   true,
                                   true,
                                   1.0f,
                                   1.0f,
                                   1.6f,
                                   1.6f);
    ASSERT(theater != nullptr);

    /**
     *  Create the default Arctic theater control.
     */
    theater = new TheaterTypeClass("SNOW",
                                   "SNOW",
                                   "ISOSNOW",
                                   "SNO",
                                   "MMS",
                                   'A',
                                   true,
                                   true,
                                   true,
                                   true,
                                   true,
                                   0.8f,
                                   0.8f,
                                   1.1f,
                                   1.1f);
    ASSERT(theater != nullptr);

    return true;
}


/**
 *  Reads theaters from the INI file.
 * 
 *  @author: CCHyper
 */
bool TheaterTypeClass::Read_Theaters_INI(CCINIClass &ini)
{
    static char const * const THEATERS = "TheaterTypes";

    if (!ini.Is_Present(THEATERS)) {
        return false;
    }

    char buf[128];
    TheaterTypeClass *theatertype = nullptr;

    int counter = ini.Entry_Count(THEATERS);
    for (int index = 0; index < counter; ++index) {
        const char *entry = ini.Get_Entry(THEATERS, index);

        /**
         *  Get a theater entry.
         */
        if (ini.Get_String(THEATERS, entry, buf, sizeof(buf))) {

            /**
             *  Find or create a theater type of the name specified.
             */
            theatertype = (TheaterTypeClass *)TheaterTypeClass::Find_Or_Make(buf);
            if (theatertype) {
                DEV_DEBUG_INFO("Reading TheaterType \"%s\".\n", buf);

                /**
                 *  
                 */
                theatertype->Read_INI(ini);

            } else {
                DEV_DEBUG_WARNING("Error reading TheaterType \"%s\"!\n", buf);
            }

        }

    }

    return counter > 0;
}


/**
 *  Fetches a reference to the theater specified.
 * 
 *  @author: CCHyper
 */
const TheaterTypeClass &TheaterTypeClass::As_Reference(TheaterType type)
{
    static const TheaterTypeClass _x;

    //ASSERT(type != THEATER_NONE && type < TheaterTypes.Count());

    if (type == THEATER_NONE || type >= TheaterTypes.Count()) {
        return _x;
    }

    return *TheaterTypes[type];
}


/**
 *  Converts a theater number into a theater object pointer.
 * 
 *  @author: CCHyper
 */
const TheaterTypeClass *TheaterTypeClass::As_Pointer(TheaterType type)
{
    //ASSERT(type != THEATER_NONE && type < TheaterTypes.Count());
    return type != THEATER_NONE && type < TheaterTypes.Count() ? TheaterTypes[type] : nullptr;
}


/**
 *  Fetches a reference to the theater specified.
 * 
 *  @author: CCHyper
 */
const TheaterTypeClass &TheaterTypeClass::As_Reference(const char *name)
{
    return As_Reference(From_Name(name));
}


/**
 *  Converts a theater name into a theater object pointer.
 * 
 *  @author: CCHyper
 */
const TheaterTypeClass *TheaterTypeClass::As_Pointer(const char *name)
{
    return As_Pointer(From_Name(name));
}


/**
 *  Retrieves the TheaterType for given name.
 * 
 *  @author: CCHyper
 */
TheaterType TheaterTypeClass::From_Name(const char *name)
{
    ASSERT(name != nullptr);

    if (!strcasecmp(name, "<none>") || !strcasecmp(name, "none")) {
        return THEATER_NONE;
    }

    if (name != nullptr) {
        for (TheaterType index = THEATER_FIRST; index < TheaterTypes.Count(); ++index) {
            if (!strcasecmp(As_Reference(index).Name, name)) {
                return index;
            }
        }
    }

    return THEATER_NONE;
}


/**
 *  Returns name for given theater type.
 * 
 *  @author: CCHyper
 */
const char *TheaterTypeClass::Name_From(TheaterType type)
{
    return (type != THEATER_NONE && type < TheaterTypes.Count() ? As_Reference(type).Name : "<none>");
}


/**
 *  Returns the theater root name.
 * 
 *  @author: CCHyper
 */
const char *TheaterTypeClass::Root_From(TheaterType type)
{
    return As_Reference(type).Root;
}


/**
 *  Returns the theater data root name suffix.
 * 
 *  @author: CCHyper
 */
const char *TheaterTypeClass::IsoRoot_From(TheaterType type)
{
    return As_Reference(type).IsoRoot;
}


/**
 *  Returns the theater filename suffix.
 * 
 *  @author: CCHyper
 */
const char *TheaterTypeClass::Suffix_From(TheaterType type)
{
    return As_Reference(type).Suffix;
}


/**
 *  Returns the theater marble madness filename suffix.
 * 
 *  @author: CCHyper
 */
const char *TheaterTypeClass::MMSuffix_From(TheaterType type)
{
    return As_Reference(type).MMSuffix;
}


/**
 *  Returns the theater image character.
 * 
 *  @author: CCHyper
 */
const char TheaterTypeClass::ImageLetter_From(TheaterType type)
{
    return As_Reference(type).ImageLetter;
}


/**
 *  Find or create a theater of the type specified.
 * 
 *  @author: CCHyper
 */
const TheaterTypeClass *TheaterTypeClass::Find_Or_Make(const char *name)
{
    ASSERT(name != nullptr);

    if (!strcasecmp(name, "<none>") || !strcasecmp(name, "none")) {
        return nullptr;
    }

    for (TheaterType index = THEATER_FIRST; index < TheaterTypes.Count(); ++index) {
        if (!strcasecmp(TheaterTypes[index]->Name, name)) {
            return TheaterTypes[index];
        }
    }

    TheaterTypeClass *ptr = new TheaterTypeClass(name);
    ASSERT(ptr != nullptr);
    return ptr;
}


/**
 *  Does this theater type support ice growth?
 * 
 *  @author: CCHyper
 */
bool TheaterTypeClass::Is_Arctic(TheaterType type)
{
    return As_Reference(type).IsArctic;
}


/**
 *  Does this theater type support ice growth?
 * 
 *  @author: CCHyper
 */
bool TheaterTypeClass::Ice_Growth_Allowed(TheaterType type)
{
    return As_Reference(type).IsIceGrowthEnabled;
}


/**
 *  Does this theater type support vein growth?
 * 
 *  @author: CCHyper
 */
bool TheaterTypeClass::Vein_Growth_Allowed(TheaterType type)
{
    return As_Reference(type).IsVeinGrowthEnabled;
}


/**
 *  Is this theater type allowed to be used in the map generator?
 * 
 *  @author: CCHyper
 */
bool TheaterTypeClass::Allowed_In_Map_Generator(TheaterType type)
{
    return As_Reference(type).IsAllowedInMapGenerator;
}


/**
 *  Does this theater allow vein growth in in the map generator?
 * 
 *  @author: CCHyper
 */
bool TheaterTypeClass::Veins_Allowed_In_Map_Generator(TheaterType type)
{
    return As_Reference(type).IsGenerateVeinholesInMapGenerator;
}


/**
 *  Returns the low radar brightness value.
 * 
 *  @author: CCHyper
 */
float TheaterTypeClass::Low_Radar_Brightness(TheaterType type)
{
    return As_Reference(type).LowRadarBrightness1;
}


/**
 *  Returns the high radar brightness value.
 * 
 *  @author: CCHyper
 */
float TheaterTypeClass::High_Radar_Brightness(TheaterType type)
{
    return As_Reference(type).HighRadarBrightness1;
}
