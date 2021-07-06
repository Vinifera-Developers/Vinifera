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
 *  Main function for patching the hooks.
 */
void MapSeedClassExtension_Hooks()
{
    Patch_Jump(0x0053E48B, &_MapSeedClass_Init_Random_Map_Init_Houses_Patch);
}
