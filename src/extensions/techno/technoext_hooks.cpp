/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TechnoClass.
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
#include "technoext_hooks.h"
#include "technoext_init.h"
#include "technoext.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_inline.h"
#include "weapontype.h"
#include "rules.h"
#include "voc.h"
#include "fatal.h"
#include "vinifera_util.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-391
 * 
 *  Extends the firing animation effect to support more facings.
 * 
 *  @author: CCHyper
 */
static DirStruct &Techno_Fire_Direction(TechnoClass *this_ptr)
{
    return this_ptr->Fire_Direction();
}

DECLARE_PATCH(_TechnoClass_Fire_At_Weapon_Anim_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(WeaponTypeClass *, weapon, ebx);
    GET_REGISTER_STATIC(AnimTypeClass *, anim, edi);
    static DirStruct *dir;
    static int anim_count;
    static int index;

    anim_count = weapon->Anim.Count();
    dir = &Techno_Fire_Direction(this_ptr);

    if (anim_count == 8) {

        index = Dir_To_8(*dir);
        anim = weapon->Anim[index % FACING_COUNT];

    } else if (anim_count == 16) {

        index = Dir_To_16(*dir);
        anim = weapon->Anim[index % 16];

    } else if (anim_count == 32) {

        index = Dir_To_32(*dir);
        anim = weapon->Anim[index % 32];

    } else if (anim_count == 64) {

        index = Dir_To_64(*dir);
        anim = weapon->Anim[index % 64];

    } else {

        index = 0;
        anim = nullptr;

    }

    _asm { mov edx, anim }
    JMP(0x006310A6);
}


/**
 *  #issue-356
 * 
 *  Custom cloaking sound for TechnoTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Do_Cloak_Cloak_Sound_Patch)
{
    GET_REGISTER_STATIC(Coordinate *, coord, eax);
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    static TechnoTypeClass *technotype;
    static TechnoTypeClassExtension *technotypeext;
    static VocType voc;

    technotype = this_ptr->Techno_Type_Class();

    /**
     *  Fetch the default cloaking sound.
     */
    voc = Rule->CloakSound;

    /**
     *  Fetch the class extension if it exists.
     */
    technotypeext = TechnoTypeClassExtensions.find(technotype);
    if (technotypeext) {

        /**
         *  Does this object have a custom cloaking sound? If so, use it.
         */
        if (technotypeext->CloakSound != VOC_NONE) {
            voc = technotypeext->CloakSound;
        }
    }

    /**
     *  Play the sound effect at the objects location.
     */
    Sound_Effect(voc, *coord);

    JMP(0x00633C8B);
}


/**
 *  #issue-356
 * 
 *  Custom uncloaking sound for TechnoTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Do_Uncloak_Uncloak_Sound_Patch)
{
    GET_REGISTER_STATIC(Coordinate *, coord, eax);
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    static TechnoTypeClass *technotype;
    static TechnoTypeClassExtension *technotypeext;
    static VocType voc;

    technotype = this_ptr->Techno_Type_Class();

    /**
     *  Fetch the default cloaking sound.
     */
    voc = Rule->CloakSound;

    /**
     *  Fetch the class extension if it exists.
     */
    technotypeext = TechnoTypeClassExtensions.find(technotype);
    if (technotypeext) {

        /**
         *  Does this object have a custom decloaking sound? If so, use it.
         */
        if (technotypeext->UncloakSound != VOC_NONE) {
            voc = technotypeext->UncloakSound;
        }
    }

    /**
     *  Play the sound effect at the objects location.
     */
    Sound_Effect(voc, *coord);

    JMP(0x00633BE7);
}


/**
 *  Main function for patching the hooks.
 */
void TechnoClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    TechnoClassExtension_Init();

    Patch_Jump(0x00633C78, &_TechnoClass_Do_Cloak_Cloak_Sound_Patch);
    Patch_Jump(0x00633BD4, &_TechnoClass_Do_Uncloak_Uncloak_Sound_Patch);
    Patch_Jump(0x0063105C, &_TechnoClass_Fire_At_Weapon_Anim_Patch);
}
