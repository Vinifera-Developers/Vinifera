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
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "rules.h"
#include "voc.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


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
    Patch_Jump(0x00633C78, &_TechnoClass_Do_Cloak_Cloak_Sound_Patch);
    Patch_Jump(0x00633BD4, &_TechnoClass_Do_Uncloak_Uncloak_Sound_Patch);
}
