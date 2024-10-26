/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SPAWNHOUSES_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks making SpawnX houses work.
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

#include "spawnhouses_hooks.h"

#include "extension_globals.h"
#include "hooker.h"
#include "session.h"
#include "tibsun_globals.h"

#include "hooker_macros.h"
#include "house.h"
#include "housetype.h"
#include "scenarioext.h"
#include "unit.h"
#include "spawner.h"


/**
 *  Returns a house from a spawn house name.
 *
 *  @author: ZivDero
 */
HousesType Spawn_House_From_Name(const char* name)
{
    ASSERT(name != nullptr);

    int spawn_number;

    /**
     *  Try to read the house name as a spawn house name and extract its number.
     */
    if (std::sscanf(name, "Spawn%d", &spawn_number) == 1) {

        /**
         *  If we're successful, iterate all assigned starting positions and check if the one we want is present.
         */
        for (int i = 0; i < Session.Players.Count() + Session.Options.AIPlayers; i++) {

            /**
             *  If it is, that's our desired house.
             */
            if (ScenExtension->StartingPositions[i] == spawn_number - 1) {
                return static_cast<HousesType>(i);
            }
        }

    }

    /**
     *  Return that this isn't a spawn house present on this map.
     */
    return HOUSE_NONE;
}


/**
 *  Returns a house from a spawn house name or a normal house name.
 *
 *  @author: ZivDero
 */
HousesType House_Or_Spawn_House_From_Name(const char* name)
{
    /**
     *  In campaigns, proceed as usual.
     */
    if (Session.Type == GAME_NORMAL) {
        return HouseTypeClass::From_Name(name);
    }

    /**
     *  In skirmish/multiplayer, try to fetch a spawn house instead.
     */
    return Spawn_House_From_Name(name);
}


/**
 *  Returns a house pointer from a house type.
 *
 *  @author: ZivDero
 */
HouseClass* House_As_Pointer(HousesType house)
{
    /**
     *  In campaigns, proceed as usual.
     */
    if (Session.Type == GAME_NORMAL) {
        return HouseClass::As_Pointer(house);
    }

    /**
     *  In skirmish/multiplayer, what we get as input is a house index, so just return the house at that index.
     */
    return Houses[house];
}


/**
 *  Special unit version of House_Or_Spawn_House_From_Name that adds a
 *  null pointer to the unit vector if the house is not found.
 *
 *  @author: ZivDero
 */
HousesType House_Or_Spawn_House_From_Name_Unit(const char* name)
{
    /**
     *  In campaigns, proceed as usual.
     */
    if (Session.Type == GAME_NORMAL) {
        return HouseTypeClass::From_Name(name);
    }

    /**
     *  In skirmish/multiplayer, try to fetch a spawn house instead.
     *  If we couldn't find the spawn house, add a null pointer to the unit vector
     *  so that the "LinkedTo" numbers don't break. We'll remove these null pointers
     *  at the end.
     */
    HousesType house = Spawn_House_From_Name(name);
    //if (house == HOUSE_NONE) {
    //    Units.Add(nullptr);
    //}

    return house;
}


/**
 *  Patch to fetch the spawn house for infantry during initial placement.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_InfantryClass_Read_INI_SpawnHouses_Patch)
{
    GET_REGISTER_STATIC(char*, house_name, eax);

    static HousesType house;
    static HouseClass* hptr;

    house = House_Or_Spawn_House_From_Name(house_name);

    if (house != HOUSE_NONE)
    {
        hptr = House_As_Pointer(house);

        _asm mov edi, hptr
        JMP(0x004D7BD5);
    }

    JMP(0x004D7F30);
}


/**
 *  Link units to their followers.
 *
 *  @author: ZivDero
 */
static void Link_Units(DynamicVectorClass<int>& link_vector)
{
    /**
     *  Links the followed and followed units, checking to make sure both actually exist.
     */
    for (int i = 0; i < Units.Count(); ++i)
    {
        int follower_id = link_vector[i];
        UnitClass* unit = Units[i];

        if (unit) {

            if (follower_id != -1 && follower_id < Units.Count() && Units[follower_id]) {
                UnitClass* follower = Units[follower_id];
                unit->FollowingMe = follower;
                follower->IsFollowing = true;
            }
            else {
                unit->FollowingMe = nullptr;
            }
        }
    }

    /**
     *  We need to removes the null pointers we added from the unit vector.
     */
    for (int i = 0; i < Units.Count(); i++) {
        if (!Units[i]) {
            Units.Delete(i--);
        }
    }
}


/**
 *  Patch to remove null pointers from the unit vector.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_UnitClass_Read_INI_Link_Units)
{
    LEA_STACK_STATIC(DynamicVectorClass<int>*, link_vector, esp, 0xC);

    Link_Units(*link_vector);

    JMP(0x00658A10);
}


/**
 *  Main function for patching the hooks.
 */
void SpawnHouses_Hooks()
{
    Patch_Call(0x00658658, &House_Or_Spawn_House_From_Name_Unit); // UnitClass
    Patch_Call(0x00434843, &House_Or_Spawn_House_From_Name); // BuildingClass
    Patch_Call(0x0040E806, &House_Or_Spawn_House_From_Name); // AircraftClass

    Patch_Call(0x00658681, &House_As_Pointer); // UnitClass
    Patch_Call(0x0043484C, &House_As_Pointer); // BuildingClass
    Patch_Call(0x0040E839, &House_As_Pointer); // AircraftClass

    Patch_Jump(0x004D7B98, &_InfantryClass_Read_INI_SpawnHouses_Patch); // InfantryClass has As_Pointer inlined, so we have to do this instead
    Patch_Jump(0x006589C8, &_UnitClass_Read_INI_Link_Units); // We need units the units to their followers ourselves because of the null pointers we've added, and then remove those null pointers

    Patch_Jump(0x0043485F, 0x00434874); // Jump past check in BuildingClass::Read_INI() preventing multiplayer building spawning for players
}
