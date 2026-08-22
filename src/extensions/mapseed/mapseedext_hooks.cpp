/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended MapSeedClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "ccini.h"
#include "debughandler.h"
#include "hooker.h"
#include "house.h"
#include "housetype.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "vinifera_defines.h"


/**
 *  #issue-496
 *
 *  Sanity check to make sure HouseType "Neutral" exists before using it.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0054E7DE, _MapSeedClass_Generate_Place_Units_And_Infantry_Neutral_House_Crash_Fix, 0)
{
    /**
     *  Stolen bytes/code.
     */
    R->Stack(0x2C, R->ESI());

    HousesType house = HouseTypeClass::From_Name("Neutral");
    HouseClass* hptr = House_From_HousesType(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"{}\"!\n", "Neutral");
        goto function_return;
    }

continue_function:
    R->EAX(hptr);
    return 0x0054E7F30;

function_return:
    return 0x0054EB6D;
}

DEFINE_HOOK(0x0054E498, _MapSeedClass_Generate_Place_Town_Buildings_Neutral_House_Crash_Fix, 0)
{
    HousesType house = HouseTypeClass::From_Name("Neutral");
    HouseClass* hptr = House_From_HousesType(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"{}\"!\n", "Neutral");
        goto function_return;
    }

continue_function:
    R->EAX(hptr);
    return 0x0054E4A9;

function_return:
    return 0x0054E79C;
}

DEFINE_HOOK(0x0054C701, _MapSeedClass_Generate_Place_Town_Infantry_Neutral_House_Crash_Fix, 0)
{
    /**
     *  Stolen bytes/code.
     */
    R->Stack(0x2C, R->ESI());

    HousesType house = HouseTypeClass::From_Name("Neutral");
    HouseClass* hptr = House_From_HousesType(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"{}\"!\n", "Neutral");
        goto function_return;
    }

place_building:
    R->EAX(hptr);
    return 0x0054C716;

function_return:
    return 0x0054CA6A;
}

DEFINE_HOOK(0x0054C31C, _MapSeedClass_Generate_Place_City_Buildings_Neutral_House_Crash_Fix, 0)
{
    HousesType house = HouseTypeClass::From_Name("Neutral");
    HouseClass *hptr = House_From_HousesType(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"{}\"!\n", "Neutral");
        goto function_return;
    }

continue_function:
    R->EAX(hptr);
    return 0x0054C32D;

function_return:
    return 0x0054C6C2;
}

DEFINE_HOOK(0x00546A4B, _MapSeedClass_Generate_Place_Tiberium_Wildlife_Neutral_House_Crash_Fix, 0)
{
    /**
     *  Stolen bytes/code.
     */
    R->Stack(0x58, R->EAX());

    HousesType house = HouseTypeClass::From_Name("Neutral");
    HouseClass* hptr = House_From_HousesType(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"{}\"!\n", "Neutral");
        goto return_false;
    }

continue_function:
    R->EAX(hptr);
    return 0x00546A60;

return_false:
    return 0x005471A1;
}

DEFINE_HOOK(0x00535434, _MapSeedClass_Generate_Bridge_Hut_Neutral_House_Crash_Fix, 0)
{
    HousesType house = HouseTypeClass::From_Name("Neutral");
    HouseClass* hptr = House_From_HousesType(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"{}\"!\n", "Neutral");
        goto return_false;
    }

place_building:
    R->ECX(hptr);
    return 0x00535445;

return_false:
    return 0x00535428;
}


/**
 *  #issue-496
 * 
 *  Sanity check to make sure HouseType "Special" exists before using it.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0054F41D, _MapSeedClass_Generate_Add_Lights_Special_House_Crash_Fix, 0)
{
    HousesType house = HouseTypeClass::From_Name("Special");
    HouseClass* hptr = House_From_HousesType(house);

    /**
     *  Make sure the house exists before placing the bridge repair hut.
     */
    if (!hptr) {
        DEBUG_WARNING("Unable to find house \"{}\"!\n", "Special");
        //goto return_false;
    }

    R->ECX(hptr);
    return 0x0054F42E;
}


/**
 *  Initialises all houses for the Random Map Generator.
 * 
 *  @author: CCHyper
 */
static void MapSeedClass_Init_Houses(CCINIClass &ini)
{
    /**
     *  Iterate over all house types and create and init a house instance for them.
     */
    for (int i = 0; i < HouseTypes.Count(); ++i) {
        HouseTypeClass* housetype = HouseTypes[i];
        if (housetype) {
            HouseClass* house = new HouseClass(housetype);
            if (house) {
                DEBUG_INFO("  Created house \"{}\".\n", housetype->Name());
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
DEFINE_HOOK(0x0053E48B, _MapSeedClass_Init_Random_Map_Init_Houses_Patch, 0)
{
    LEA_STACK(CCINIClass *, ini, 0x128);

    DEBUG_INFO("Initialising houses for RMG...\n");

    MapSeedClass_Init_Houses(*ini);

    R->EBX(0);
    return 0x0053E55E;
}


/**
 *  #issue-71
 *
 *  Increases the amount of available waypoints (see ScenarioClassExtension for implementation).
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005104FD, _MapClass_Set_Map_Dimensions_WaypointMax, 0)
{
    GET(int, i, ESI);

    if (i < NEW_WAYPOINT_COUNT) {
        return 0x005104B7;
    } else {
        return 0x00510502;
    }
}


/**
 *  Main function for patching the hooks.
 */
void MapSeedClassExtension_Hooks()
{

}
