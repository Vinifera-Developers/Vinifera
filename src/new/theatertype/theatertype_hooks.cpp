/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          THEATERTYPE_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for TheaterTypeClass.
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
#include "theatertype_hooks.h"
#include "theatertype.h"
#include "ccini.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "animtype.h"
#include "buildingtype.h"
#include "overlaytype.h"
#include "smudgetype.h"
#include "terrain.h"
#include "terraintype.h"
#include "isotiletype.h"
#include "iomap.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Patch to add support for new theaters when initialising the theater data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Theater_Patch)
{
    GET_REGISTER_STATIC(TheaterType, theater, ebp);
    GET_REGISTER_STATIC(void *, _imp_wsprintfA, edi); // Just to make sure we are not overwriting it.
    LEA_STACK_STATIC(char *, root_name, esp, 0x14); // char [16]
    LEA_STACK_STATIC(char *, iso_root, esp, 0x34); // char [16]
    LEA_STACK_STATIC(char *, suffix_name, esp, 0x24); // char [16]
    static const char *root;

    std::snprintf(root_name, 16, "%s.MIX", TheaterTypeClass::Root_From(theater));
    std::snprintf(iso_root, 16, "%s.MIX", TheaterTypeClass::IsoRoot_From(theater));
    std::snprintf(suffix_name, 16, "%s.MIX", TheaterTypeClass::Suffix_From(theater));
    
    DEBUG_INFO("Init theater \"%s\"\n"
               "  %s\n"
               "  %s\n"
               "  %s\n",
        TheaterTypeClass::Name_From(theater), root_name, iso_root, suffix_name);

    /**
     *  Code further down in the function expects to have theater root without
     *  the extension, so we restore this here, along with the EDI function pointer.
     */
    root = TheaterTypeClass::Root_From(theater);
    _asm { mov ebx, root }

    _asm { mov edi, _imp_wsprintfA } // Restore EDI.
    _asm { mov ebp, theater }        // Restore EBP.

    JMP(0x004E7BB0);
}


/**
 *  Patch to add support for new theaters when initialising the theater control INI.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Theater_INI_Patch)
{
    LEA_STACK_STATIC(char *, ini_name, esp, 0x344); // char [20]
    GET_STACK_STATIC(TheaterType, theater, esp, 0x98);

    std::snprintf(ini_name, 20, "%s.INI", TheaterTypeClass::Root_From(theater));

    JMP_REG(ecx, 0x004F3D88);
}


/**
 *  Patch to add support for new theaters when loading a AnimTypes image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimTypeClass_Init_Theater_Patch)
{
    GET_REGISTER_STATIC(AnimTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, ebp);
    LEA_STACK_STATIC(char *, fullname, esp, 0x10); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x00418942);
}


/**
 *  Patch to add support for new theaters when loading a AnimTypes image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimTypeClass_entry_64_Theater_Patch)
{
    GET_REGISTER_STATIC(AnimTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, eax);
    LEA_STACK_STATIC(char *, fullname, esp, 0x4); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x00418A35);
}


/**
 *  Patch to add support for new theaters when re-loading an AnimTypes image
 *  after a save game as been loaded.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimTypeClass_Load_Theater_Patch)
{
    GET_REGISTER_STATIC(AnimTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, eax);
    LEA_STACK_STATIC(char *, fullname, esp, 0x8); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x00419742);
}


/**
 *  Patch to add support for new theaters when loading a AnimTypes image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimTypeClass_Get_Image_Data_Theater_Patch)
{
    GET_REGISTER_STATIC(AnimTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, eax);
    LEA_STACK_STATIC(char *, fullname, esp, 0x68); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x00419AB3);
}


/**
 *  Patch to add support for new theaters when loading a BuildingTypes image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingTypeClass_Init_Theater_Patch)
{
    GET_REGISTER_STATIC(BuildingTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, edi);
    LEA_STACK_STATIC(char *, fullname, esp, 0x14); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->GraphicName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x0043FCD4);
}


/**
 *  Patch to add support for new theaters when loading a BuildingTypes buildup image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingTypeClass_Init_Buildup_Theater_Patch)
{
    GET_REGISTER_STATIC(BuildingTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, edi);
    LEA_STACK_STATIC(char *, fullname, esp, 0x14); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->BuildupFilename, TheaterTypeClass::Suffix_From(theater));

    JMP(0x0043FD1E);
}


/**
 *  Patch to add support for new theaters when re-loading an BuildingTypes image
 *  after a save game as been loaded.
 * 
 *  @warning: This patch is jumps around stack adjustments, be careful when modifying! 
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingTypeClass_Load_Shape_Data_Theater_Patch)
{
    GET_REGISTER_STATIC(BuildingTypeClass *, this_ptr, ebx);
    GET_REGISTER_STATIC(TheaterType, theater, esi);
    LEA_STACK_STATIC(char *, buff, esp, 0x64); // char [_MAX_FNAME]

    if (!this_ptr->IsTheater || (theater == THEATER_NONE || theater >= TheaterTypes.Count())) {
        std::snprintf(buff, 512, "%s.SHP", this_ptr->GraphicName);
    } else {
        std::snprintf(buff, 512, "%s.%s", this_ptr->GraphicName, TheaterTypeClass::Suffix_From(theater));
    }

    _asm { mov eax, theater }

    JMP_REG(ecx, 0x004406D1);
}


/**
 *  Patch to add support for new theaters when loading a OverlayTypes image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OverlayTypeClass_Init_Theater_Patch)
{
    GET_REGISTER_STATIC(OverlayTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, edi);
    LEA_STACK_STATIC(char *, fullname, esp, 0x0C); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->GraphicName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x0058D3F9);
}


/**
 *  Patch to add support for new theaters when re-loading an OverlayTypes image
 *  after a save game as been loaded.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OverlayTypeClass_Load_Theater_Patch)
{
    GET_REGISTER_STATIC(OverlayTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, eax);
    LEA_STACK_STATIC(char *, fullname, esp, 0x8); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->GraphicName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x0058D88F);
}


/**
 *  Patch to add support for new theaters when loading a SmudgeTypes image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SmudgeTypeClass_Init_Theater_Patch)
{
    GET_REGISTER_STATIC(SmudgeTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, edi);
    LEA_STACK_STATIC(char *, fullname, esp, 0x0C); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x005FB419);
}


/**
 *  Patch to add support for new theaters when loading a SmudgeTypes theater image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SmudgeTypeClass_Read_INI_Theater_Patch)
{
    GET_REGISTER_STATIC(SmudgeTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, eax);
    LEA_STACK_STATIC(char *, fullname, esp, 0x0C); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->GraphicName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x005FB69B);
}


/**
 *  Patch to add support for new theaters when loading the Veinhole monsters image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_VeinholeMonsterClass_Init_Theater_Patch)
{
    GET_REGISTER_STATIC(TheaterType, theater, ecx);
    LEA_STACK_STATIC(char *, buffer, esp, 0x0);

    std::snprintf(buffer, 32, "VEINHOLE.%s", TheaterTypeClass::Suffix_From(theater));

    JMP(0x00661A10);
}


/**
 *  Patch to add support for new theaters when loading a ObjectTypes theater image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ObjectTypeClass_Load_Theater_Art_Theater_Patch)
{
    //GET_REGISTER_STATIC(ObjectTypeClass *, this_ptr, edi);
    GET_REGISTER_STATIC(TheaterType, theater, eax);
    GET_REGISTER_STATIC(char *, ini_name, esi);
    LEA_STACK_STATIC(char *, fullname, esp, 0x0C); // char [PATH_MAX]

    std::snprintf(fullname, PATH_MAX, "%s.%s", ini_name, TheaterTypeClass::Suffix_From(theater));

    JMP(0x0058890A);
}


/**
 *  Patch to add support for new theaters when loading a OverlayTypes image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OverlayTypeClass_Get_Image_Data_Theater_Patch)
{
    GET_REGISTER_STATIC(OverlayTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, eax);
    LEA_STACK_STATIC(char *, fullname, esp, 0x68); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->GraphicName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x0058DB8C);
}


/**
 *  Patch to add support for new theaters when loading a TerrainTypes image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TerrainTypeClass_Init_Theater_Patch)
{
    GET_REGISTER_STATIC(SmudgeTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TheaterType, theater, edi);
    LEA_STACK_STATIC(char *, fullname, esp, 0x0C); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", this_ptr->IniName, TheaterTypeClass::Suffix_From(theater));

    JMP(0x00641710);
}


/**
 *  Patch to add support for new theaters when loading the slope z-data shape.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_Read_INI_SlopeZ_Theater_Patch)
{
    LEA_STACK_STATIC(char *, fullname, esp, 0x1D8); // char [20]
    GET_REGISTER_STATIC(TheaterType, theater, esi);
    static const char *suffix;

    suffix = (char *)TheaterTypeClass::Suffix_From(theater);
    std::snprintf(fullname, 20, "SLOP01Z.%s", suffix);

    /**
     *  EDI is expected further down in the function to be the theater suffix.
     */
    _asm { mov edi, suffix }
    _asm { mov [esp+0x138], edi } // ext/suffix

    JMP(0x004F3B90);
}


/**
 *  Patch to add support for new theaters checking processing a cell with a ice tile.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapClass_Process_Ice_Tile_Theater_Patch)
{
    /**
     *  Stolen bytes/code.
     */
    _asm { push esi }
    _asm { push edi }

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto return_false;
    }

continue_checks:
    JMP(0x0052071F);

return_false:
    JMP(0x005208A9);
}


/**
 *  Patch to add support for new theaters checking the cracked ice growth logic.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapClass_Cracked_Ice_AI_Theater_Patch)
{
    /**
     *  Stolen bytes/code.
     */
    _asm { push ebp }
    _asm { push esi }
    _asm { push edi }

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto return_false;
    }

continue_checks:
    JMP(0x00520F70);

return_false:
    JMP(0x00520F59);
}


/**
 *  Patch to add support for new theaters checking if a unit is passing over a ice tile.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Per_Cell_Process_Ice_Check_Theater_Patch)
{
    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto not_allowed;
    }

passes_check:
    JMP(0x00651A53);

not_allowed:
    JMP(0x00651B2A);
}


/**
 *  Patch to add support for new theaters when initialising the theater on map load.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Theater_Palette_Theater_Patch)
{
    GET_REGISTER_STATIC(TheaterType, theater, ebp);
    static char _buffer[PATH_MAX];

    if (theater == THEATER_NONE || theater >= TheaterTypes.Count()) {
        DEBUG_WARNING("Invalid theater in Init_Theater()!\n");
    }

    std::snprintf(_buffer, sizeof(_buffer), "UNIT%s.PAL", TheaterTypeClass::Suffix_From(theater));

retrieve_file:
    _asm { lea ecx, dword ptr _buffer }
    JMP(0x004E7D8F);
}


/**
 *  Patch to add support for new theaters when setting terrain occupy flags.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TerrainClass_Set_Occupy_Bit_Theater_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, ecx);
    GET_REGISTER_STATIC(ScenarioClass *, scen, eax);

    /**
     *  Is this theater considered arctic?
     */
    if (!TheaterTypeClass::Is_Arctic(scen->Theater)) {
        goto temperate_bits;
    }

snow_bits:
    _asm { mov ecx, this_ptr } // restore "this".
    JMP(0x0063F9BB);  

temperate_bits:
    _asm { mov ecx, this_ptr } // restore "this".
    JMP(0x0063F9B0);
}


/**
 *  Patch to add support for new theaters when clearing terrain occupy flags.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TerrainClass_Clear_Occupy_Bit_Theater_Patch)
{
    GET_REGISTER_STATIC(TerrainClass *, this_ptr, ecx);
    GET_REGISTER_STATIC(ScenarioClass *, scen, eax);

    /**
     *  Is this theater considered arctic?
     */
    if (!TheaterTypeClass::Is_Arctic(scen->Theater)) {
        goto temperate_bits;
    }

snow_bits:
    _asm { mov ecx, this_ptr } // restore "this".
    JMP(0x0063F92B);  

temperate_bits:
    _asm { mov ecx, this_ptr } // restore "this".
    JMP(0x0063F920);
}


/**
 *  Patch to add support for new theaters when placing the veinholes in generated maps.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapSeedClass_Generate_Allow_Veinholes_Theater_Patch)
{
    /**
     *  Stolen bytes/code.
     */
    _asm { add esp, 0x4 }

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
    JMP(0x0053D376);

skip_generation:
    JMP(0x0053D37D);
}


/**
 *  Patch to add support for new theaters when checking if the ice growth
 *  timer needs updating.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapClass_Ice_Growth_AI_Theater_Patch)
{
    GET_REGISTER_STATIC(ScenarioClass *, scen, eax);

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(scen->Theater)) {
        goto return_false;
    }

continue_check:
    _asm { mov eax, scen } // restore ScenarioClass
    _asm { xor ecx, ecx }
    JMP_REG(edx, 0x00520DC8);  

return_false:
    _asm { xor eax, eax }
    JMP_REG(edx, 0x00520F2F);
}


/**
 *  Patch to add support for new theaters checking if the object is moving over
 *  a cell with contains a an ice tile.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapClass_Moving_Over_Ice_Theater_Patch)
{
    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto return_false;
    }

continue_check:
    JMP(0x005209C2);  

return_false:
    _asm { xor eax, eax }
    JMP_REG(ecx, 0x00520D8F);
}


/**
 *  Patch to add support for new theaters when smoothing over the ice shore tilesets.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapClass_Smooth_Ice_Shore_Theater_Patch)
{
    GET_REGISTER_STATIC(ScenarioClass *, scen, eax);

    /**
     *  Stolen bytes/code.
     */
    _asm { push esi }
    _asm { push edi }

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(scen->Theater)) {
        goto function_return;
    }

continue_function:
    JMP(0x0051FBDF);  

function_return:
    JMP(0x005206E8);
}


/**
 *  Patch to add support for new theaters when smoothing over the ice tilesets.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapClass_Smooth_Ice_Theater_Patch_1)
{
    GET_REGISTER_STATIC(ScenarioClass *, scen, eax);

    /**
     *  Stolen bytes/code.
     */
    _asm { push esi }
    _asm { push edi }

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(scen->Theater)) {
        goto function_return;
    }

continue_function:
    JMP(0x0051F5CF);  

function_return:
    JMP(0x0051FBA9);
}


/**
 *  Patch to add support for new theaters when smoothing over the ice tilesets.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapClass_Smooth_Ice_Theater_Patch_2)
{
    GET_REGISTER_STATIC(ScenarioClass *, scen, eax);

    /**
     *  Stolen bytes/code.
     */
    _asm { push esi }
    _asm { push edi }

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(scen->Theater)) {
        goto function_return;
    }

continue_function:
    JMP(0x0051F03F);  

function_return:
    JMP(0x0051F59D);
}


/**
 *  Patch to add support for new theaters when smoothing over the ice tilesets.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapClass_Smooth_Ice_Theater_Patch_3)
{
    GET_REGISTER_STATIC(MapClass *, this_ptr, ecx);
    GET_REGISTER_STATIC(ScenarioClass *, scen, eax);

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(scen->Theater)) {
        goto function_return;
    }

continue_function:
    _asm { mov ecx, this_ptr }
    JMP(0x0051EBEB);  

function_return:
    _asm { mov ecx, this_ptr }
    JMP(0x0051F015);
}


/**
 *  Patch to add support for new theaters when choosing if the ice growth timer should be updated.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_LogicClass_Old_AI_Ice_Timer_Theater_Patch)
{
    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto skip_ice_update;
    }

update_ice_timer:
    JMP(0x00507329);

skip_ice_update:
    JMP(0x00507388);
}


/**
 *  Patch to add support for new theaters when choosing if the ice growth timer should be updated.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_LogicClass_AI_Ice_Timer_Theater_Patch)
{
    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(Scen->Theater)) {
        goto skip_ice_update;
    }

update_ice_timer:
    JMP(0x00506F54);

skip_ice_update:
    JMP(0x00507000);
}


/**
 *  Patch to add support for new theaters when fixing up ice tilesets on theater control load.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_Read_INI_Process_Ice_Tilesets_Theater_Patch)
{
    GET_STACK_STATIC(TheaterType, theater, esp, 0x98);

    /**
     *  Is this theater flagged to handle the ice growth logic?
     */
    if (!TheaterTypeClass::Ice_Growth_Allowed(theater)) {
        goto function_return;
    }

process_ice_tilesets:
    JMP(0x004F554D);

function_return:
    JMP(0x004F55F2);
}


/**
 *  Patch to add support for new theaters when loading the marble madness tiles.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_IsometricTileTypeClass_Read_INI_MarbleMadness_Theater_Patch)
{
    GET_STACK_STATIC(TheaterType, theater, esp, 0x98);
    LEA_STACK_STATIC(char *, filename, esp, 0x56C); // char [128]
    LEA_STACK_STATIC(char *, fullname, esp, 0x6AC); // char [_MAX_FNAME+_MAX_EXT]

    std::snprintf(fullname, 512, "%s.%s", filename, TheaterTypeClass::MMSuffix_From(theater));
    //DEV_DEBUG_INFO("MM: %s\n", fullname);

    JMP_REG(edx, 0x004F51E4);
}


/**
 *  Patch to add support for new theaters when calculating cell brightness on the radar.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CellClass_Cell_Color_Theater_Patch_1)
{
    GET_REGISTER_STATIC(TheaterType, theater, edx);
    static long rgb;    // Actually is a RGBClass instance.
    static long val;

    _asm { mov rgb, ecx }

    /**
     *  Evil trick borrowed from Quake III Arena's "Q_rsqrt" which also
     *  forces the use of general registers rather than the FPU.
     * 
     *  See:
     *  https://en.wikipedia.org/wiki/Fast_inverse_square_root
     */
    val  = *(long *)&TheaterTypeClass::As_Reference(theater).LowRadarBrightness1;
    
    _asm { mov ecx, rgb }
    _asm { mov eax, val }
    JMP_REG(edx, 0x00451ECB);
}


/**
 *  Patch to add support for new theaters when calculating cell brightness on the radar.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CellClass_Cell_Color_Theater_Patch_2)
{
    GET_REGISTER_STATIC(TheaterType, theater, edx);
    static long rgb;    // Actually is a RGBClass instance.
    static long val;
    
    _asm { mov rgb, ecx }

    /**
     *  Evil trick borrowed from Quake III Arena's "Q_rsqrt" which also
     *  forces the use of general registers rather than the FPU.
     * 
     *  See:
     *  https://en.wikipedia.org/wiki/Fast_inverse_square_root
     */
    val  = *(long *)&TheaterTypeClass::As_Reference(theater).LowRadarBrightness2;
    
    _asm { mov ecx, rgb }
    _asm { mov eax, val }
    JMP_REG(edx, 0x00451EFC);
}


/**
 *  Patch to add support for new theaters when calculating cell brightness on the radar.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CellClass_Cell_Color_Theater_Patch_3)
{
    GET_REGISTER_STATIC(TheaterType, theater, edx);
    static long rgb;    // Actually is a RGBClass instance.
    static long val;
    
    _asm { mov rgb, ecx }

    /**
     *  Evil trick borrowed from Quake III Arena's "Q_rsqrt" which also
     *  forces the use of general registers rather than the FPU.
     * 
     *  See:
     *  https://en.wikipedia.org/wiki/Fast_inverse_square_root
     */
    val  = *(long *)&TheaterTypeClass::As_Reference(theater).HighRadarBrightness1;
    
    _asm { mov ecx, rgb }
    _asm { mov eax, val }
    JMP_REG(edx, 0x00451F2D);
}


/**
 *  Patch to add support for new theaters when calculating cell brightness on the radar.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CellClass_Cell_Color_Theater_Patch_4)
{
    GET_REGISTER_STATIC(TheaterType, theater, eax);
    static long rgb;    // Actually is an address to a RGBClass instance.
    static long val;
    
    _asm { mov rgb, edx }

    /**
     *  Evil trick borrowed from Quake III Arena's "Q_rsqrt" which also
     *  forces the use of general registers rather than the FPU.
     * 
     *  See:
     *  https://en.wikipedia.org/wiki/Fast_inverse_square_root
     */
    val  = *(long *)&TheaterTypeClass::As_Reference(theater).HighRadarBrightness2;
    
    _asm { mov edx, rgb }
    _asm { mov ecx, val }
    JMP_REG(eax, 0x00451F4F);
}


/**
 *  Patch to add support for new theaters when setting cell occupation bits.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CellClass_Set_Attributes_Theater_Patch)
{
    GET_REGISTER_STATIC(CellClass *, this_ptr, edi);
    GET_REGISTER_STATIC(TerrainClass *, terrain, esi);

    /**
     *  Is this theater considered arctic?
     */
    if (TheaterTypeClass::Is_Arctic(Scen->Theater)) {
        if (terrain->Class->SnowOccupationBits != TERRAIN_OCCUPY_ALL) {
            goto set_passability_FREE_SPOTS;
        }
    } else {
        if (terrain->Class->TemperateOccupationBits != TERRAIN_OCCUPY_ALL) {
            goto set_passability_FREE_SPOTS;
        }
    }

set_passability_WALL:
    JMP(0x00459A51);

set_passability_FREE_SPOTS:
    JMP(0x00459B62);
}


/**
 *  Main function for patching the hooks.
 */
void TheaterTypeClassExtension_Hooks()
{
    Patch_Jump(0x004E7B63, &_Init_Theater_Patch);
    Patch_Byte_Range(0x004E7BB5, 0x90, 3); // Fixup stack from above patch.

    Patch_Jump(0x004F3D6C, &_Init_Theater_INI_Patch);
    Patch_Jump(0x004E7D78, &_Init_Theater_Palette_Theater_Patch);

    Patch_Jump(0x00418921, &_AnimTypeClass_Init_Theater_Patch);
    Patch_Jump(0x00418A15, &_AnimTypeClass_entry_64_Theater_Patch);
    Patch_Jump(0x00419722, &_AnimTypeClass_Load_Theater_Patch);
    Patch_Jump(0x00419A93, &_AnimTypeClass_Get_Image_Data_Theater_Patch);
    Patch_Jump(0x0043FCB3, &_BuildingTypeClass_Init_Theater_Patch);
    Patch_Jump(0x0043FCFD, &_BuildingTypeClass_Init_Buildup_Theater_Patch);

    Patch_Jump(0x0044065F, &_BuildingTypeClass_Load_Shape_Data_Theater_Patch);

    Patch_Jump(0x0058D3D6, &_OverlayTypeClass_Init_Theater_Patch);
    Patch_Jump(0x0058D86C, &_OverlayTypeClass_Load_Theater_Patch);
    Patch_Jump(0x0058DB69, &_OverlayTypeClass_Get_Image_Data_Theater_Patch);
    Patch_Jump(0x005FB3F9, &_SmudgeTypeClass_Init_Theater_Patch);
    Patch_Jump(0x005FB678, &_SmudgeTypeClass_Read_INI_Theater_Patch);
    Patch_Jump(0x005888ED, &_ObjectTypeClass_Load_Theater_Art_Theater_Patch);
    Patch_Jump(0x006416E9, &_TerrainTypeClass_Init_Theater_Patch);

    Patch_Jump(0x004F3B67, &_IsometricTileTypeClass_Read_INI_SlopeZ_Theater_Patch);
    Patch_Jump(0x004F5535, &_IsometricTileTypeClass_Read_INI_Process_Ice_Tilesets_Theater_Patch);

    Patch_Jump(0x004F518F, &_IsometricTileTypeClass_Read_INI_MarbleMadness_Theater_Patch);
    Patch_Byte_Range(0x004F51E8, 0x90, 3); // Fixup stack from above patch.

    Patch_Jump(0x006619F6, &_VeinholeMonsterClass_Init_Theater_Patch);

    Patch_Jump(0x00651A40, &_UnitClass_Per_Cell_Process_Ice_Check_Theater_Patch);
    Patch_Jump(0x0063F9A6, &_TerrainClass_Set_Occupy_Bit_Theater_Patch);
    Patch_Jump(0x0063F916, &_TerrainClass_Clear_Occupy_Bit_Theater_Patch);

    Patch_Jump(0x0053D365, &_MapSeedClass_Generate_Allow_Veinholes_Theater_Patch);

    Patch_Jump(0x0052070E, &_MapClass_Process_Ice_Tile_Theater_Patch);
    Patch_Jump(0x00520F4B, &_MapClass_Cracked_Ice_AI_Theater_Patch);
    Patch_Jump(0x00520DBB, &_MapClass_Ice_Growth_AI_Theater_Patch);
    Patch_Jump(0x005209B1, &_MapClass_Moving_Over_Ice_Theater_Patch);
    Patch_Jump(0x0051FBCE, &_MapClass_Smooth_Ice_Shore_Theater_Patch);
    Patch_Jump(0x0051F5BE, &_MapClass_Smooth_Ice_Theater_Patch_1);
    Patch_Jump(0x0051F02E, &_MapClass_Smooth_Ice_Theater_Patch_2);
    Patch_Jump(0x0051EBDE, &_MapClass_Smooth_Ice_Theater_Patch_3);

    Patch_Jump(0x00507320, &_LogicClass_Old_AI_Ice_Timer_Theater_Patch);
    Patch_Jump(0x00506F47, &_LogicClass_AI_Ice_Timer_Theater_Patch);

    Patch_Jump(0x00451EC4, &_CellClass_Cell_Color_Theater_Patch_1);
    Patch_Jump(0x00451EF5, &_CellClass_Cell_Color_Theater_Patch_2);
    Patch_Jump(0x00451F26, &_CellClass_Cell_Color_Theater_Patch_3);
    Patch_Jump(0x00451F48, &_CellClass_Cell_Color_Theater_Patch_4);
    Patch_Jump(0x00459B26, &_CellClass_Set_Attributes_Theater_Patch);
}
