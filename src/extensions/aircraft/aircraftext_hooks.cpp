/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AIRCRAFTEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended AircraftClass.
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
#include "aircraftext_hooks.h"
#include "aircraft.h"
#include "aircrafttype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-469
 * 
 *  Fixes a bug where IsCloakable has no effect on Aircrafts. This was
 *  because the TechnoType value was not copied to the Aircraft instance
 *  when it is created.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_AircraftClass_Init_IsCloakable_BugFix_Patch)
{
	GET_REGISTER_STATIC(AircraftClass *, this_ptr, esi);
	GET_REGISTER_STATIC(AircraftTypeClass *, aircrafttype, eax);

	/**
	 *  Stolen bytes/code.
	 */
	this_ptr->Strength = aircrafttype->MaxStrength;
	this_ptr->Ammo = aircrafttype->MaxAmmo;

	/**
	 *  This is the line that was missing (maybe it was by design?).
	 */
	this_ptr->IsCloakable = aircrafttype->IsCloakable;

	JMP_REG(ecx, 0x004088AA);
}


/**
 *  Main function for patching the hooks.
 */
void AircraftClassExtension_Hooks()
{
	Patch_Jump(0x00408898, &_AircraftClass_Init_IsCloakable_BugFix_Patch);
}
