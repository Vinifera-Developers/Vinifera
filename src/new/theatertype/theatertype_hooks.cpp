/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for TheaterTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "theatertype_hooks.h"

#include "animtype.h"
#include "buildingtype.h"
#include "ccini.h"
#include "debughandler.h"
#include "hooker.h"
#include "iomap.h"
#include "overlaytype.h"
#include "smudgetype.h"
#include "syringe.h"
#include "theatertype.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"


/**
 *  Patch to add support for new theaters when initialising the theater data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004E7B63, _Init_Theater_Patch, 0)
{
    GET(TheaterType, theater, EBP);
    LEA_STACK(char *, root_name, 0x14); // char [16]
    LEA_STACK(char *, iso_root, 0x34); // char [16]
    LEA_STACK(char *, suffix_name, 0x24); // char [16]

    std::snprintf(root_name, 16, "%s.MIX", TheaterTypeClass::Root_From(theater));
    std::snprintf(iso_root, 16, "%s.MIX", TheaterTypeClass::IsoRoot_From(theater));
    std::snprintf(suffix_name, 16, "%s.MIX", TheaterTypeClass::Suffix_From(theater));
    
    DEBUG_INFO("Init theater \"{}\"\n"
               "  {}\n"
               "  {}\n"
               "  {}\n",
        TheaterTypeClass::Name_From(theater), root_name, iso_root, suffix_name);

    /**
     *  Code further down in the function expects to have theater root without
     *  the extension, so we restore this here, along with the EDI function pointer.
     */
    const char* root = TheaterTypeClass::Root_From(theater);
    R->EBX(root);

    R->ESP(R->ESP() - 0x2C); // Adjust stack for where we're returning to.

    return 0x004E7BB0;
}


/**
 *  Patch to add support for new theaters when initialising the theater control INI.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004F3D6C, _Init_Theater_INI_Patch, 0)
{
    LEA_STACK(char *, ini_name, 0x344); // char [20]
    GET_STACK(TheaterType, theater, 0x98);

    std::snprintf(ini_name, 20, "%s.INI", TheaterTypeClass::Root_From(theater));

    return 0x004F3D88;
}


/**
 *  Patch to add support for new theaters when loading a AnimTypes image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00418921, _AnimTypeClass_Init_Theater_Patch, 0)
{
    GET(AnimTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EBP);
    LEA_STACK(char *, fullname, 0x10); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x00418942;
}


/**
 *  Patch to add support for new theaters when loading a AnimTypes image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00418A15, _AnimTypeClass_entry_64_Theater_Patch, 0)
{
    GET(AnimTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EAX);
    LEA_STACK(char *, fullname, 0x4); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x00418A35;
}


/**
 *  Patch to add support for new theaters when re-loading an AnimTypes image
 *  after a save game as been loaded.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00419722, _AnimTypeClass_Load_Theater_Patch, 0)
{
    GET(AnimTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EAX);
    LEA_STACK(char *, fullname, 0x8); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x00419742;
}


/**
 *  Patch to add support for new theaters when loading a AnimTypes image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00419A93, _AnimTypeClass_Get_Image_Data_Theater_Patch, 0)
{
    GET(AnimTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EAX);
    LEA_STACK(char *, fullname, 0x68); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x00419AB3;
}


/**
 *  Patch to add support for new theaters when loading a BuildingTypes image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0043FCB3, _BuildingTypeClass_Init_Theater_Patch, 0)
{
    GET(BuildingTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EDI);
    LEA_STACK(char *, fullname, 0x14); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->GraphicName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x0043FCD4;
}


/**
 *  Patch to add support for new theaters when loading a BuildingTypes buildup image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0043FCFD, _BuildingTypeClass_Init_Buildup_Theater_Patch, 0)
{
    GET(BuildingTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EDI);
    LEA_STACK(char *, fullname, 0x14); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->BuildupFilename.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x0043FD1E;
}


/**
 *  Patch to add support for new theaters when re-loading an BuildingTypes image
 *  after a save game as been loaded.
 * 
 *  @warning: This patch is jumps around stack adjustments, be careful when modifying! 
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0044065F, _BuildingTypeClass_Load_Shape_Data_Theater_Patch, 0)
{
    GET(BuildingTypeClass *, this_ptr, EBX);
    GET(TheaterType, theater, ESI);
    LEA_STACK(char *, buff, 0x64); // char [_MAX_FNAME]

    if (!this_ptr->IsTheater || theater == THEATER_NONE || theater >= TheaterTypes.Count()) {
        std::snprintf(buff, 512, "%s.SHP", this_ptr->GraphicName.c_str());
    } else {
        std::snprintf(buff, 512, "%s.%s", this_ptr->GraphicName.c_str(), TheaterTypeClass::Suffix_From(theater));
    }

    R->EAX(theater);

    return 0x004406D1;
}


/**
 *  Patch to add support for new theaters when loading a OverlayTypes image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0058D3D6, _OverlayTypeClass_Init_Theater_Patch, 0)
{
    GET(OverlayTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EDI);
    LEA_STACK(char *, fullname, 0x0C); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->GraphicName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x0058D3F9;
}


/**
 *  Patch to add support for new theaters when re-loading an OverlayTypes image
 *  after a save game as been loaded.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0058D86C, _OverlayTypeClass_Load_Theater_Patch, 0)
{
    GET(OverlayTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EAX);
    LEA_STACK(char *, fullname, 0x8); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->GraphicName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x0058D88F;
}


/**
 *  Patch to add support for new theaters when loading a SmudgeTypes image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005FB3F9, _SmudgeTypeClass_Init_Theater_Patch, 0)
{
    GET(SmudgeTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EDI);
    LEA_STACK(char *, fullname, 0x0C); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x005FB419;
}


/**
 *  Patch to add support for new theaters when loading a SmudgeTypes theater image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005FB678, _SmudgeTypeClass_Read_INI_Theater_Patch, 0)
{
    GET(SmudgeTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EAX);
    LEA_STACK(char *, fullname, 0x08); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->GraphicName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x005FB69B;
}


/**
 *  Patch to add support for new theaters when loading the Veinhole monsters image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x006619F6, _VeinholeMonsterClass_Init_Theater_Patch, 0)
{
    GET(TheaterType, theater, ECX);
    LEA_STACK(char *, buffer, 0x0);

    std::snprintf(buffer, 32, "VEINHOLE.%s", TheaterTypeClass::Suffix_From(theater));

    return 0x00661A10;
}


/**
 *  Patch to add support for new theaters when loading a ObjectTypes theater image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005888ED, _ObjectTypeClass_Load_Theater_Art_Theater_Patch, 0)
{
    GET(TheaterType, theater, EAX);
    GET(char *, ini_name, ESI);
    LEA_STACK(char *, fullname, 0x0C); // char [PATH_MAX]

    std::snprintf(fullname, PATH_MAX, "%s.%s", ini_name, TheaterTypeClass::Suffix_From(theater));

    return 0x0058890A;
}


/**
 *  Patch to add support for new theaters when loading a OverlayTypes image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0058DB69, _OverlayTypeClass_Get_Image_Data_Theater_Patch, 0)
{
    GET(OverlayTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EAX);
    LEA_STACK(char *, fullname, 0x68); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->GraphicName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x0058DB8C;
}


/**
 *  Patch to add support for new theaters when loading a TerrainTypes image data.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x006416E9, _TerrainTypeClass_Init_Theater_Patch, 0)
{
    GET(SmudgeTypeClass *, this_ptr, ESI);
    GET(TheaterType, theater, EDI);
    LEA_STACK(char *, fullname, 0x0C); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName.c_str(), TheaterTypeClass::Suffix_From(theater));

    return 0x00641710;
}


/**
 *  Patch to add support for new theaters when loading the slope z-data shape.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004F3B67, _IsometricTileTypeClass_Read_INI_SlopeZ_Theater_Patch, 0)
{
    LEA_STACK(char *, fullname, 0x1D8); // char [20]
    GET(TheaterType, theater, ESI);

    char const* suffix = TheaterTypeClass::Suffix_From(theater);
    std::snprintf(fullname, 20, "SLOP01Z.%s", suffix);

    /**
     *  EDI is expected further down in the function to be the theater suffix.
     */
    R->EDI(suffix);
    R->Stack(0x138, suffix); // ext/suffix

    return 0x004F3B90;
}


/**
 *  Patch to add support for new theaters checking processing a cell with a ice tile.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00520719, _MapClass_Process_Ice_Tile_Theater_Patch, 0)
{
    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto return_false;
    }

continue_checks:
    return 0x0052071F;

return_false:
    return 0x005208A9;
}


/**
 *  Patch to add support for new theaters checking the cracked ice growth logic.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00520F59, _MapClass_Cracked_Ice_AI_Theater_Patch, 6)
{
    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto return_false;
    }

continue_checks:
    return 0x00520F70;

return_false:
    return 0;
}


/**
 *  Patch to add support for new theaters checking if a unit is passing over a ice tile.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00651A40, _UnitClass_Per_Cell_Process_Ice_Check_Theater_Patch, 0)
{
    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto not_allowed;
    }

passes_check:
    return 0x00651A53;

not_allowed:
    return 0x00651B2A;
}


/**
 *  Patch to add support for new theaters when initialising the theater on map load.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004E7D78, _Init_Theater_Palette_Theater_Patch, 0)
{
    GET(TheaterType, theater, EBP);
    static char _buffer[PATH_MAX];

    if (theater == THEATER_NONE || theater >= TheaterTypes.Count()) {
        DEBUG_WARNING("Invalid theater in Init_Theater()!\n");
    }

    std::snprintf(_buffer, sizeof(_buffer), "UNIT%s.PAL", TheaterTypeClass::Suffix_From(theater));

retrieve_file:
    R->ECX(_buffer);
    return 0x004E7D8F;
}


/**
 *  Patch to add support for new theaters when setting terrain occupy flags.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0063F9A6, _TerrainClass_Set_Occupy_Bit_Theater_Patch, 0)
{
    GET(ScenarioClass *, scen, EAX);

    /**
     *  Is this theater considered arctic?
     */
    if (!TheaterTypeClass::Is_Arctic(scen->Theater)) {
        goto temperate_bits;
    }

snow_bits:
    return 0x0063F9BB;

temperate_bits:
    return 0x0063F9B0;
}


/**
 *  Patch to add support for new theaters when clearing terrain occupy flags.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0063F916, _TerrainClass_Clear_Occupy_Bit_Theater_Patch, 0)
{
    GET(ScenarioClass *, scen, EAX);

    /**
     *  Is this theater considered arctic?
     */
    if (!TheaterTypeClass::Is_Arctic(scen->Theater)) {
        goto temperate_bits;
    }

snow_bits:
    return 0x0063F92B;

temperate_bits:
    return 0x0063F920;
}


/**
 *  Patch to add support for new theaters when placing the veinholes in generated maps.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0053D365, _MapSeedClass_Generate_Allow_Veinholes_Theater_Patch, 0)
{
    /**
     *  Stolen bytes/code.
     */
    R->ESP(R->ESP() + 0x4);

    /**
     *  Is this theater allowed to be used in the map generator?
     */
    if (!TheaterTypeClass::Allowed_In_Map_Generator(Scen->Theater)) {
        goto skip_generation;
    }

    /**
     *  Are veins and/or veinholes allowed to be placed in this theater?
     */
    if (!TheaterTypeClass::Veins_Allowed_In_Map_Generator(Scen->Theater)) {
        goto skip_generation;
    }

generate:
    return 0x0053D376;

skip_generation:
    return 0x0053D37D;
}


/**
 *  Patch to add support for new theaters when checking if the ice growth
 *  timer needs updating.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00520DBB, _MapClass_Ice_Growth_AI_Theater_Patch, 0)
{
    GET(ScenarioClass *, scen, EAX);

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(scen->Theater)) {
        goto return_false;
    }

continue_check:
    R->ECX(0);
    return 0x00520DC8;

return_false:
    R->EAX(0);
    return 0x00520F2F;
}


/**
 *  Patch to add support for new theaters checking if the object is moving over
 *  a cell with contains a an ice tile.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005209B1, _MapClass_Moving_Over_Ice_Theater_Patch, 0)
{
    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto return_false;
    }

continue_check:
    return 0x005209C2;

return_false:
    R->EAX(0);
    return 0x00520D8F;
}


/**
 *  Patch to add support for new theaters when smoothing over the ice shore tilesets.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0051FBD9, _MapClass_Smooth_Ice_Shore_Theater_Patch, 0)
{
    GET(ScenarioClass *, scen, EAX);

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(scen->Theater)) {
        goto function_return;
    }

continue_function:
    return 0x0051FBDF;

function_return:
    return 0x005206E8;
}


/**
 *  Patch to add support for new theaters when smoothing over the ice tilesets.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0051F5C9, _MapClass_Smooth_Ice_Theater_Patch_1, 0)
{
    GET(ScenarioClass *, scen, EAX);

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(scen->Theater)) {
        goto function_return;
    }

continue_function:
    return 0x0051F5CF;

function_return:
    return 0x0051FBA9;
}


/**
 *  Patch to add support for new theaters when smoothing over the ice tilesets.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0051F039, _MapClass_Smooth_Ice_Theater_Patch_2, 0)
{
    GET(ScenarioClass *, scen, EAX);

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(scen->Theater)) {
        goto function_return;
    }

continue_function:
    return 0x0051F03F;  

function_return:
    return 0x0051F59D;
}


/**
 *  Patch to add support for new theaters when smoothing over the ice tilesets.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0051EBDE, _MapClass_Smooth_Ice_Theater_Patch_3, 0)
{
    GET(ScenarioClass *, scen, EAX);

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(scen->Theater)) {
        goto function_return;
    }

continue_function:
    return 0x0051EBEB;

function_return:
    return 0x0051F015;
}


/**
 *  Patch to add support for new theaters when choosing if the ice growth timer should be updated.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00507320, _LogicClass_Old_AI_Ice_Timer_Theater_Patch, 0)
{
    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto skip_ice_update;
    }

update_ice_timer:
    return 0x00507329;

skip_ice_update:
    return 0x00507388;
}


/**
 *  Patch to add support for new theaters when choosing if the ice growth timer should be updated.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00506F47, _LogicClass_AI_Ice_Timer_Theater_Patch, 0)
{
    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto skip_ice_update;
    }

update_ice_timer:
    return 0x00506F54;

skip_ice_update:
    return 0x00507000;
}


/**
 *  Patch to add support for new theaters when fixing up ice tilesets on theater control load.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004F5535, _IsometricTileTypeClass_Read_INI_Process_Ice_Tilesets_Theater_Patch, 0)
{
    GET_STACK(TheaterType, theater, 0x98);

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(theater)) {
        goto function_return;
    }

process_ice_tilesets:
    return 0x004F554D;

function_return:
    return 0x004F55F2;
}


/**
 *  Patch to add support for new theaters when loading the marble madness tiles.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004F51DF, _IsometricTileTypeClass_Read_INI_MarbleMadness_Theater_Patch, 0)
{
    GET_STACK(TheaterType, theater, 0xAC);
    GET_STACK(char*, filename, 0x0C);
    GET_STACK(char*, fullname, 0x00);

    std::snprintf(fullname, 512, "%s.%s", filename, TheaterTypeClass::MMSuffix_From(theater));
    //DEV_DEBUG_INFO("MM: {}\n", fullname);

    return 0x004F51E4;
}


/**
 *  Patch to add support for new theaters when calculating cell brightness on the radar.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00451EC4, _CellClass_Cell_Color_Theater_Patch_1, 0)
{
    GET(TheaterType, theater, EDX);

    R->EAX(reinterpret_cast<DWORD const&>(TheaterTypeClass::As_Reference(theater).LowRadarBrightness1));

    return 0x00451ECB;
}


/**
 *  Patch to add support for new theaters when calculating cell brightness on the radar.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00451EF5, _CellClass_Cell_Color_Theater_Patch_2, 0)
{
    GET(TheaterType, theater, EDX);

    R->EAX(reinterpret_cast<DWORD const&>(TheaterTypeClass::As_Reference(theater).LowRadarBrightness2));

    return 0x00451EFC;
}


/**
 *  Patch to add support for new theaters when calculating cell brightness on the radar.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00451F26, _CellClass_Cell_Color_Theater_Patch_3, 0)
{
    GET(TheaterType, theater, EDX);

    R->EAX(reinterpret_cast<DWORD const&>(TheaterTypeClass::As_Reference(theater).HighRadarBrightness1));

    return 0x00451F2D;
}


/**
 *  Patch to add support for new theaters when calculating cell brightness on the radar.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00451F48, _CellClass_Cell_Color_Theater_Patch_4, 0)
{
    GET(TheaterType, theater, EAX);

    R->ECX(reinterpret_cast<DWORD const&>(TheaterTypeClass::As_Reference(theater).HighRadarBrightness2));

    return 0x00451F4F;
}


/**
 *  Main function for patching the hooks.
 */
void TheaterTypeClassExtension_Hooks()
{
    // Patch away a 2-byte jump that's in the way of our hook
    Patch_Byte_Range(0x00520F57, 0x90, 2);
}
