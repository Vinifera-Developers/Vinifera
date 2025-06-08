/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera (Dawn of the Tiberium Age Build)
 *
 *  @file          PARTICLEEXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended ParticleClass.
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
#include "building.h"
#include "house.h"
#include "housetype.h"
#include "particlesysext_hooks.h"
#include "particlesys.h"
#include "rules.h"
#include "scenario.h"
#include "unit.h"
#include "tibsun_globals.h"
#include "tibsun_defines.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


UnitClass* Create_Visceroid(ObjectClass* destroyedobject)
{
	if (destroyedobject->RTTI == RTTI_INFANTRY ||
		(destroyedobject->RTTI == RTTI_UNIT && reinterpret_cast<UnitClass*>(destroyedobject)->Class->IsCrew) ||
		(destroyedobject->RTTI == RTTI_BUILDING && reinterpret_cast<BuildingClass*>(destroyedobject)->Class->IsCrew))
	{
		return new UnitClass(Rule->SmallVisceroid, HouseClass::As_Pointer(HouseTypeClass::From_Name("Neutral")));
	}

	return nullptr;
}


/**
 *  Fixes a bug where gas clouds are able to turn everything into visceroids, including
 *  non-crewed vehicles and even terrain objects.
 */
DECLARE_PATCH(_ParticleClass_Smoke_And_WeakGas_Behaviour_AI_Tiberium_Death_Patch)
{
	GET_REGISTER_STATIC(ObjectClass*, destroyedobject, esi);
	GET_REGISTER_STATIC(ResultType, result, eax);
	GET_STACK_STATIC(ObjectClass*, nextobject, esp, 0x40);
	static UnitClass* visceroid;

	enum {
		ContinueVisceroidPlacement = 0x005A38FC,
		SkipToNextObjectOnCell = 0x005A3965
	};

	_asm { mov esi, dword ptr ds:nextobject }

	if (result != RESULT_DESTROYED) {
		// Object was not destroyed, do not create visceroid.
		JMP(SkipToNextObjectOnCell);
	}

	if (!Scen->IsTiberiumDeathToVisceroid) {
		// Visceroids spawning from Tiberium death is disabled, do not create visceroid.
		JMP(SkipToNextObjectOnCell);
	}

	visceroid = Create_Visceroid(destroyedobject);
	if (visceroid == nullptr) {
		// No visceroid was created.
		JMP(SkipToNextObjectOnCell);
	}

	_asm { mov edi, dword ptr ds:visceroid }
	JMP(ContinueVisceroidPlacement);
}


/**
 *  Main function for patching the hooks.
 */
void ParticleClassExtension_Hooks()
{
	Patch_Jump(0x005A389C, &_ParticleClass_Smoke_And_WeakGas_Behaviour_AI_Tiberium_Death_Patch);
}
