/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          PARTICLETYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended ParticleTypeClass.
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
#include "particletypeext_hooks.h"
#include "particletypeext_init.h"
#include "particletypeext.h"
#include "particletype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  Main function for patching the hooks.
 */
void ParticleTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    ParticleTypeClassExtension_Init();
}
