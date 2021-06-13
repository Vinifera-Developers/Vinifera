/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UNITEXT_HOOKS.H
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended UnitClass.
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
#include "vinifera_globals.h"
#include "unit.h"
#include "unittype.h"
#include "target.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-#6
 * 
 *  A "quality of life" patch for harvesters so they auto harvest
 *  when they have just been kicked out of the war factory.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Per_Cell_Process_AutoHarvest_Assign_Harvest_Mission_Patch)
{
	GET_REGISTER_STATIC(UnitClass *, this_ptr, ebp);
	GET_REGISTER_STATIC(TARGET, target, ebp);
	static BuildingClass *building_contact;
	static UnitTypeClass *unittype;

	/**
	 *  Is the unit we are processing a harvester?
	 */
	unittype = reinterpret_cast<UnitTypeClass *>(this_ptr->Class_Of());
	if (unittype->IsToHarvest || unittype->IsToVeinHarvest) {

		/**
		 *  Order the unit to harvest.
		 */
		this_ptr->Assign_Mission(MISSION_HARVEST);

		goto continue_check_scatter;
	}

	/**
	 *  Stolen bytes/code from here on, continues function flow.
	 */

	/**
	 *  Find out if the target is a building. (flagged to not use dynamic_cast).
	 */
continue_function:
	building_contact = Target_As_Building(target, false);
	_asm { mov eax, building_contact }
	JMP_REG(ecx, 0x006517D2);

continue_check_scatter:
	JMP(0x0065194E);
}


/**
 *  Main function for patching the hooks.
 */
void UnitClassExtension_Hooks()
{
	Patch_Jump(0x006517BE, &_UnitClass_Per_Cell_Process_AutoHarvest_Assign_Harvest_Mission_Patch);
}
