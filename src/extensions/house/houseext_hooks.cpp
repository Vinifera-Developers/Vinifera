/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOUSEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended HouseClass.
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
#include "houseext_init.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "house.h"
#include "housetype.h"
#include "technotype.h"
#include "super.h"
#include "building.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "rules.h"
#include "rulesext.h"
#include "iomap.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class HouseClassExt final : public HouseClass
{
    public:
        Cell _Find_Build_Location(BuildingTypeClass *buildingtype, int (__fastcall *callback)(int, Cell &, int, int), int a3 = -1);
};


/**
 *  #issue-531
 * 
 *  Interception of Find_Build_Location. This allows us to find a suitable building
 *  location for the specific buildings, such as the Naval Yard.
 * 
 *  @author: CCHyper
 */
Cell HouseClassExt::_Find_Build_Location(BuildingTypeClass *buildingtype, int (__fastcall *callback)(int, Cell &, int, int), int a3)
{
    /**
     *  Find the type class extension instance.
     */
    BuildingTypeClassExtension *buildingtypeext = BuildingTypeClassExtensions.find(buildingtype);
    if (buildingtypeext && buildingtypeext->IsNavalYard) {

        if (RulesExtension) {

            DEV_DEBUG_INFO("Find_Build_Location(%s): Searching for Naval Yard \"%s\" build location...\n", Name(), buildingtype->Name());

            Cell cell(0,0);

            //BuildingTypeClass *desired_navalyard = Get_First_Ownable(RulesExtension->BuildNavalYard);
            //ASSERT_PRINT(desired_navalyard != nullptr, "Failed to find a ownable building in BuildNavalYard list!");

            /**
             *  Get the cell footprint for the Naval Yard, then add a safety margin of 2.
             */
            //int area_w = desired_navalyard->Width() + 2;
            //int area_h = desired_navalyard->Height() + 2;
            int area_w = buildingtype->Width() + 2;
            int area_h = buildingtype->Height() + 2;

            /**
             *  find a nearby location from the center of the base that fits our naval yard.
             */
            Cell found_cell = Map.Nearby_Location(Coord_Cell(Center), SPEED_FLOAT, -1, MZONE_NORMAL, 0, area_w, area_h);
            if (found_cell) {

                DEV_DEBUG_INFO("Find_Build_Location(%s): Found possible Naval Yard location at %d,%d...\n", Name(), found_cell.X, found_cell.Y);

                /**
                 *  Iterate over all owned construction yards and find the first that is closest to our cell.
                 */
                for (int i = 0; i < ConstructionYards.Count(); ++i) {
                    BuildingClass *conyard = ConstructionYards[i];
                    if (conyard) {

                        Coordinate conyard_coord = conyard->Center_Coord();
                        Coordinate found_coord = Map[found_cell].Center_Coord();

                        /**
                         *  Is this location close enough to the construction yard for us to use?
                         */
                        if (Distance(conyard_coord, found_coord) <= Cell_To_Lepton(RulesExtension->AINavalYardAdjacency)) {
                            DEV_DEBUG_INFO("Find_Build_Location(%s): Using location %d,%d for Naval Yard.\n", Name(), found_cell.X, found_cell.Y);
                            cell = found_cell;
                            break;
                        }

                    }

                }

            }
                
            if (!cell) {
                DEV_DEBUG_WARNING("Find_Build_Location(%s): Failed to find suitable location for \"%s\"!\n", Name(), buildingtype->Name());
            }

            return cell;
        }

    }

    /**
     *  Call the original function to find a location for land buildings.
     */
    return HouseClass::Find_Build_Location(buildingtype, callback, a3);
}


/**
 *  Patch for InstantSuperRechargeCommandClass
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_HouseClass_Super_Weapon_Handler_InstantRecharge_Patch)
{
    GET_REGISTER_STATIC(HouseClass *, this_ptr, edi);
    GET_REGISTER_STATIC(SuperClass *, special, esi);
    static bool is_player;

    is_player = false;
    if (this_ptr == PlayerPtr) {
        is_player = true;
    }

    if (Vinifera_DeveloperMode) {

        if (!special->IsReady) {

            /**
             *  If AIInstantBuild is toggled on, make sure this is a non-human AI house.
             */
            if (Vinifera_Developer_AIInstantSuperRecharge
                && !this_ptr->Is_Human_Control() && this_ptr != PlayerPtr) {

                special->Forced_Charge(is_player);

            /**
             *  If InstantBuild is toggled on, make sure the local player is a human house.
             */
            } else if (Vinifera_Developer_InstantSuperRecharge
                && this_ptr->Is_Human_Control() && this_ptr == PlayerPtr) {
                
                special->Forced_Charge(is_player);

            /**
             *  If the AI has taken control of the player house, it needs a special
             *  case to handle the "player" instant recharge mode.
             */
            } else if (Vinifera_Developer_InstantSuperRecharge) {
                if (Vinifera_Developer_AIControl && this_ptr == PlayerPtr) {
                    
                    special->Forced_Charge(is_player);
                }
            }

        }

    }

    /**
     *  Stolen bytes/code.
     */
    if (!special->AI(is_player)) {
        goto continue_function;
    }

add_to_sidebar:
    JMP(0x004BD320);

continue_function:
    JMP(0x004BD332);
}


/**
 *  Patch for BuildCheatCommandClass
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_HouseClass_Can_Build_BuildCheat_Patch)
{
    GET_REGISTER_STATIC(HouseClass *, this_ptr, ebp);
    GET_REGISTER_STATIC(int, vector_count, ecx);
    GET_STACK_STATIC(TechnoTypeClass *, objecttype, esp, 0x30);

    if (Vinifera_DeveloperMode && Vinifera_Developer_BuildCheat) {

        /**
         *  AI houses have access to everything, so we can just
         *  filter to the human houses only.
         */
        if (this_ptr->IsHuman && this_ptr->IsPlayerControl) {

            /**
             *  Check that the object has this house set as one of its owners.
             *  if true, force this 
             */
            if (((1 << this_ptr->Class->ID) & objecttype->Get_Ownable()) != 0) {
                //DEBUG_INFO("Forcing \"%s\" available.\n", objecttype->IniName);
                goto return_true;
            }
        }
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    _asm { xor eax, eax }
    _asm { mov [esp+0x34], eax }

    _asm { mov ecx, vector_count }
    _asm { test ecx, ecx }

    _asm { mov ecx, 0x004BBD2E }; // Need to use ECX as EAX is used later on.
    _asm { jmp ecx };

return_true:
    JMP(0x004BBD17);
}


/**
 *  Main function for patching the hooks.
 */
void HouseClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    HouseClassExtension_Init();

    Patch_Jump(0x004BBD26, &_HouseClass_Can_Build_BuildCheat_Patch);
    Patch_Jump(0x004BD30B, &_HouseClass_Super_Weapon_Handler_InstantRecharge_Patch);

    Patch_Call(0x0042D460, &HouseClassExt::_Find_Build_Location);
    Patch_Call(0x0042D53C, &HouseClassExt::_Find_Build_Location);
    Patch_Call(0x004C8104, &HouseClassExt::_Find_Build_Location);
}
