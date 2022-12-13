/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FACTORYEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended FactoryClass.
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
#include "factoryext_hooks.h"
#include "factoryext_init.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "house.h"
#include "factory.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Patch for InstantBuildCommandClass
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_FactoryClass_AI_InstantBuild_Patch)
{
	GET_REGISTER_STATIC(FactoryClass *, this_ptr, esi);

	if (Vinifera_DeveloperMode) {

		/**
		 *  If AIInstantBuild is toggled on, make sure this is a non-human AI house.
		 */
		if (Vinifera_Developer_AIInstantBuild
			&& !this_ptr->House->Is_Human_Control() && this_ptr->House != PlayerPtr) {

			this_ptr->StageClass::Set_Stage(FactoryClass::STEP_COUNT);
			goto production_completed;
		}

		/**
		 *  If InstantBuild is toggled on, make sure the local player is a human house.
		 */
		if (Vinifera_Developer_InstantBuild
			&& this_ptr->House->Is_Human_Control() && this_ptr->House == PlayerPtr) {

			this_ptr->StageClass::Set_Stage(FactoryClass::STEP_COUNT);
			goto production_completed;
		}

		/**
		 *  If the AI has taken control of the player house, it needs a special
		 *  case to handle the "player" instant build mode.
		 */
		if (Vinifera_Developer_InstantBuild) {
			if (Vinifera_Developer_AIControl && this_ptr->House == PlayerPtr) {

				this_ptr->StageClass::Set_Stage(FactoryClass::STEP_COUNT);
				goto production_completed;
			}
		}

	}

	/**
	 *  Stolen bytes/code.
	 */
	if (this_ptr->StageClass::Fetch_Stage() == FactoryClass::STEP_COUNT) {
		goto production_completed;
	}

function_return:
	JMP(0x00496FA3);

    /**
     *  Production Completed, then suspend further production.
     */
production_completed:
	JMP(0x00496F73);
}


/**
 *  Main function for patching the hooks.
 */
void FactoryClassExtension_Hooks()
{
	/**
	 *  Initialises the extended class.
	 */
	FactoryClassExtension_Init();

	Patch_Jump(0x00496F6D, &_FactoryClass_AI_InstantBuild_Patch);
}
