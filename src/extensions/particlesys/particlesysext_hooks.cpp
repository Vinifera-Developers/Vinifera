/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PARTICLESYSEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended ParticleSystemClass.
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
#include "particlesysext_hooks.h"
#include "particlesys.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Fixes a bug (observed by comparing with Red Alert 2) where spawned
 *  particles have an invalid particle system.
 * 
 *  @author: CCHyper, tomsons26
 */
DECLARE_PATCH(_ParticleSystemClass_Spawn_Particle_Particle_System_Patch)
{
	GET_REGISTER_STATIC(ParticleSystemClass *, this_ptr, esi);

	_asm { mov ecx, [esp+0x10] }
	_asm { mov edx, [esp+0x0C] }

	/**
	 *  Original code pushed "NULL" (possible default argument for the ParticleClass constructor).
	 */
	_asm { push esi }

	JMP_REG(edi, 0x005A5A6F);
}


/**
 *  Main function for patching the hooks.
 */
void ParticleSystemClassExtension_Hooks()
{
	Patch_Jump(0x005A5A65, &_ParticleSystemClass_Spawn_Particle_Particle_System_Patch);
}
