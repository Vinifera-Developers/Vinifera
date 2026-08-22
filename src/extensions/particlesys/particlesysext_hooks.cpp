/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended ParticleSystemClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "particlesysext_hooks.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "particlesys.h"


/**
 *  Fixes a bug (observed by comparing with Red Alert 2) where spawned
 *  particles have an invalid particle system.
 *
 *  @author: CCHyper, tomsons26
 */
DECLARE_PATCH(_ParticleSystemClass_Spawn_Particle_Particle_System_Patch)
{
    GET_REGISTER_STATIC(ParticleSystemClass *, this_ptr, ESI);

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
