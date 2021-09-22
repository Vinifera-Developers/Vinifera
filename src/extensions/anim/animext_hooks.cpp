/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ANIMEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended AnimClass.
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
#include "animext_hooks.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "anim.h"
#include "animtype.h"
#include "animtypeext.h"
#include "cell.h"
#include "scenario.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Patch to intercept the start of the AnimClass::AI function.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_AI_Beginning_Patch)
{
	GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
	static AnimTypeClass *animtype;
	static AnimTypeClassExtension *animtypeext;
	static CellClass *cell;

	animtype = this_ptr->Class;
	animtypeext = AnimTypeClassExtensions.find(animtype);

	/**
	 *  Stolen bytes/code.
	 */
	if (animtype->IsFlamingGuy) {
		this_ptr->Flaming_Guy_AI();
		this_ptr->ObjectClass::AI();
	}

	/**
	 *  Do we have a valid extension instance?
	 */
	if (animtypeext) {

		cell = this_ptr->Get_Cell_Ptr();

		/**
		 *  #issue-560
		 * 
		 *  Implements IsHideIfNotTiberium for Anims.
		 * 
		 *  @author: CCHyper
		 */
		if (animtypeext->IsHideIfNotTiberium) {

			if (!cell || !cell->Get_Tiberium_Value()) {
				this_ptr->IsInvisible = true;
			}

		}

	}

	JMP_REG(edx, 0x00414EAA);
}


/**
 *  Patch for setting initial anim values in the class constructor.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AnimClass_Constructor_Init_Class_Values_Patch)
{
	GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
	static AnimTypeClassExtension *animtypeext;

	/**
	 *  Stolen bytes/code.
	 */
	this_ptr->IsActive = true;

	/**
	 *  #BUGFIX:
	 * 
	 *  This check was observed in Red Alert 2, so there must be an edge case
	 *  where anims are created with a null type instance. So lets do that
	 *  here and also report a warning to the debug log.
	 */
	if (!this_ptr->Class) {
		goto destroy_anim;
	}

	/**
	 *  #issue-561
	 * 
	 *  Implements ZAdjust override for Anims. This will only have an effect
	 *  if the anim is created with a z-adjustment value of "0" (default value).
	 * 
	 *  @author: CCHyper
	 */
	if (!this_ptr->ZAdjust) {
		animtypeext = AnimTypeClassExtensions.find(this_ptr->Class);
		if (animtypeext) {
			this_ptr->ZAdjust = animtypeext->ZAdjust;
		}
	}

	/**
	 *  Restore some registers.
	 */
	_asm { mov ecx, this_ptr }
	_asm { mov edx, [ecx+0x64] } // this->Class
	_asm { mov ecx, edx }

	JMP_REG(edx, 0x00413C80);

	/**
	 *  Report that the anim type instance was invalid.
	 */
destroy_anim:
	DEBUG_WARNING("Anim: Invalid anim type instance!\n");

	/**
	 *  Remove the anim from the game world.
	 */
	this_ptr->entry_E4();
	
	_asm { mov esi, this_ptr }
	JMP_REG(edx, 0x00414157);
}


/**
 *  #issue-114
 * 
 *  Animations that use RandomLoopDelay incorrectly use the unsynchronized
 *  random-number generator to generate their loop delay. This causes such
 *  animations to cause sync errors in multiplayer.
 * 
 *  @author: CCHyper (based on research by Rampastring)
 */
DECLARE_PATCH(_AnimClass_AI_RandomLoop_Randomiser_BugFix_Patch)
{
	GET_REGISTER_STATIC(AnimClass *, this_ptr, esi);
	static AnimTypeClass *animtype;

	animtype = reinterpret_cast<AnimTypeClass *>(this_ptr->Class_Of());

	/**
	 *  Generate a random delay using the network synchronized RNG.
	 */
	this_ptr->Delay = Random_Pick(animtype->RandomLoopDelayMin, animtype->RandomLoopDelayMax);

	/**
	 *  Return from the function.
	 */
function_return:
	JMP(0x00415AF2);
}


/**
 *  Main function for patching the hooks.
 */
void AnimClassExtension_Hooks()
{
	Patch_Jump(0x00415ADA, &_AnimClass_AI_RandomLoop_Randomiser_BugFix_Patch);
	Patch_Jump(0x00413C79, &_AnimClass_Constructor_Init_Class_Values_Patch);
	Patch_Jump(0x00414E8F, &_AnimClass_AI_Beginning_Patch);
}
