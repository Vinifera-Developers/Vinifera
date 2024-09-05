/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MAPSEEDEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended MapSeedClass.
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
#include "houseext_hooks.h"
#include "tibsun_globals.h"
#include "house.h"
#include "housetype.h"
#include "ccini.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "vinifera_defines.h"


/**
 *  #issue-496
 * 
 *  Sanity check to make sure HouseType "Neutral" exists before using it.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapSeedClass_Generate_Place_Units_And_Infantry_Neutral_House_Crash_Fix)
{
    static const HouseTypeClass *housetype;
    static HousesType house;
    static HouseClass *hptr;

    /**
     *  Stolen bytes/code.
     */
    _asm { mov [esp+0x2C], esi }

    house = HouseTypeClass::From_Name("Neutral");
    hptr = HouseClass::As_Pointer(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"%s\"!\n", "Neutral");
        goto function_return;
    }

continue_function:
    _asm { mov eax, hptr }
    JMP_REG(ebx, 0x0054E7F3);

function_return:
    JMP_REG(ebx, 0x0054EB6D);
}

DECLARE_PATCH(_MapSeedClass_Generate_Place_Town_Buildings_Neutral_House_Crash_Fix)
{
    static const HouseTypeClass *housetype;
    static HousesType house;
    static HouseClass *hptr;

    house = HouseTypeClass::From_Name("Neutral");
    hptr = HouseClass::As_Pointer(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"%s\"!\n", "Neutral");
        goto function_return;
    }

continue_function:
    _asm { mov eax, hptr }
    JMP_REG(ebp, 0x0054E4A9);

function_return:
    JMP_REG(ecx, 0x0054E79C);
}

DECLARE_PATCH(_MapSeedClass_Generate_Place_Town_Infantry_Neutral_House_Crash_Fix)
{
    static const HouseTypeClass *housetype;
    static HousesType house;
    static HouseClass *hptr;

    /**
     *  Stolen bytes/code.
     */
    _asm { mov [esp+0x2C], esi }

    house = HouseTypeClass::From_Name("Neutral");
    hptr = HouseClass::As_Pointer(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"%s\"!\n", "Neutral");
        goto function_return;
    }

place_building:
    _asm { mov eax, hptr }
    JMP_REG(ebx, 0x0054C716);

function_return:
    JMP_REG(ebx, 0x0054CA6A);
}

DECLARE_PATCH(_MapSeedClass_Generate_Place_City_Buildings_Neutral_House_Crash_Fix)
{
    static const HouseTypeClass *housetype;
    static HousesType house;
    static HouseClass *hptr;

    house = HouseTypeClass::From_Name("Neutral");
    hptr = HouseClass::As_Pointer(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"%s\"!\n", "Neutral");
        goto function_return;
    }

continue_function:
    _asm { mov eax, hptr }
    JMP_REG(ecx, 0x0054C32D);

function_return:
    JMP_REG(ecx, 0x0054C6C2);
}

DECLARE_PATCH(_MapSeedClass_Generate_Place_Tiberium_Wildlife_Neutral_House_Crash_Fix)
{
    static const HouseTypeClass *housetype;
    static HousesType house;
    static HouseClass *hptr;

    /**
     *  Stolen bytes/code.
     */
    _asm { mov [esp+0x58], eax }

    house = HouseTypeClass::From_Name("Neutral");
    hptr = HouseClass::As_Pointer(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"%s\"!\n", "Neutral");
        goto return_false;
    }

continue_function:
    _asm { mov eax, hptr }
    JMP_REG(ecx, 0x00546A60);

return_false:
    JMP_REG(ecx, 0x005471A1);
}

DECLARE_PATCH(_MapSeedClass_Generate_Bridge_Hut_Neutral_House_Crash_Fix)
{
    static const HouseTypeClass *housetype;
    static HousesType house;
    static HouseClass *hptr;

    house = HouseTypeClass::From_Name("Neutral");
    hptr = HouseClass::As_Pointer(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"%s\"!\n", "Neutral");
        goto return_false;
    }

place_building:
    _asm { mov ecx, hptr }
    JMP_REG(edi, 0x00535445);

return_false:
    JMP_REG(edi, 0x00535428);    
}


/**
 *  #issue-496
 * 
 *  Sanity check to make sure HouseType "Special" exists before using it.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapSeedClass_Generate_Add_Lights_Special_House_Crash_Fix)
{
    static const HouseTypeClass *housetype;
    static HousesType house;
    static HouseClass *hptr;

    house = HouseTypeClass::From_Name("Special");
    hptr = HouseClass::As_Pointer(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"%s\"!\n", "Special");
        //goto return_false;
    }

    _asm { mov ecx, hptr }
    JMP_REG(ecx, 0x0054F42E);
}


/**
 *  Initialises all houses for the Random Map Generator.
 * 
 *  @author: CCHyper
 */
static void MapSeedClass_Init_Houses(CCINIClass &ini)
{
    HouseClass *house;
    HouseTypeClass *housetype;

    /**
     *  Iterate over all house types and create and init a house instance for them.
     */
    for (int i = 0; i < HouseTypes.Count(); ++i) {
        housetype = HouseTypes[i];
        if (housetype) {
            house = new HouseClass(housetype);
            if (house) {
                DEBUG_INFO("  Created house \"%s\".\n", housetype->Name());
                house->Read_INI(ini);
            }
        }
    }
}

/**
 *  #issue-495
 * 
 *  Removes the limitation (and crash) with the Random Map Generator expecting
 *  the first 4 HouseTypes to be defined. It will now process HouseTypes
 *  and initialises them from the INI.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_MapSeedClass_Init_Random_Map_Init_Houses_Patch)
{
    //GET_REGISTER_STATIC(MapSeedClass *, this_ptr, edi);
    LEA_STACK_STATIC(CCINIClass *, ini, esp, 0x128);

    DEBUG_INFO("Initalising houses for RMG...\n");

    MapSeedClass_Init_Houses(*ini);

    _asm { xor ebx, ebx }
    JMP(0x0053E55E);
}


/**
 *  #issue-71
 *
 *  Increases the amount of available waypoints (see ScenarioClassExtension for implementation).
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_MapClass_Set_Map_Dimensions_WaypointMax)
{
    GET_REGISTER_STATIC(int, i, esi);

    if (i < NEW_WAYPOINT_COUNT)
    {
        JMP_REG(ecx, 0x005104B7);
    }

    JMP(0x00510502);
}


/**
 *  Main function for patching the hooks.
 */
void MapSeedClassExtension_Hooks()
{
    Patch_Jump(0x0053E48B, &_MapSeedClass_Init_Random_Map_Init_Houses_Patch);
    Patch_Jump(0x0054F41D, &_MapSeedClass_Generate_Add_Lights_Special_House_Crash_Fix);
    Patch_Jump(0x00535434, &_MapSeedClass_Generate_Bridge_Hut_Neutral_House_Crash_Fix);
    Patch_Jump(0x00546A4B, &_MapSeedClass_Generate_Place_Tiberium_Wildlife_Neutral_House_Crash_Fix);
    Patch_Jump(0x0054C31C, &_MapSeedClass_Generate_Place_City_Buildings_Neutral_House_Crash_Fix);
    Patch_Jump(0x0054C701, &_MapSeedClass_Generate_Place_Town_Infantry_Neutral_House_Crash_Fix);
    Patch_Jump(0x0054E498, &_MapSeedClass_Generate_Place_Town_Buildings_Neutral_House_Crash_Fix);
    Patch_Jump(0x0054E7DE, &_MapSeedClass_Generate_Place_Units_And_Infantry_Neutral_House_Crash_Fix);
    Patch_Jump(0x005104FD, &_MapClass_Set_Map_Dimensions_WaypointMax);
}
