/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EMPULSEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended EMPulseClass.
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
#include "technotypeext.h"
#include "technotype.h"
#include "techno.h"
#include "building.h"
#include "foot.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"

 /**
  *  #issue-181
  *
  *  Implements optional EMP (electromagnetic pulse) immunity for buildings.
  *  Typically buildings are disabled by EMP. This patch allows
  *  making specific building types immune to the effect.
  *
  *  @author: Rampastring
  */
DECLARE_PATCH(_EMPulseClass_Create_Building_EMPImmune_Patch)
{
    GET_REGISTER_STATIC(BuildingTypeClass *, buildingtype, eax);
    static TechnoTypeClassExtension *exttype_ptr;

    exttype_ptr = TechnoTypeClassExtensions.find(buildingtype);
    if (exttype_ptr) {

        /**
         *  Is this building immune to EMP weapons?
         */
        if (exttype_ptr->IsImmuneToEMP) {
            goto loop_continue;
        }
    }

    /**
     *  Stolen bytes / code.
     */
original_code:
    
    if (buildingtype->IsInvisibleInGame) {
        goto loop_continue;
    }

    _asm {mov eax, buildingtype}
    JMP_REG(ecx, 0x00492C53);

    /**
     *  Continue looping through affected cells.
     */
loop_continue:
    JMP(0x00492F93);
}

/**
 *  #issue-181
 *
 *  Implements optional EMP (electromagnetic pulse) immunity for mobile TechnoTypes 
 *  (Technos that derive from FootClass).
 *  Typically vehicles and cyborgs are disabled by EMP. This patch
 *  allows making specific object types immune to the effect.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_EMPulseClass_Create_Foot_EMPImmune_Patch)
{
    GET_REGISTER_STATIC(FootClass *, foot, esi);
    static ILocomotion *loco;
    static TechnoTypeClassExtension *exttype_ptr;


    exttype_ptr = TechnoTypeClassExtensions.find(foot->Techno_Type_Class());
    if (exttype_ptr) {

        /**
         *  Is this object immune to EMP weapons?
         */
        if (exttype_ptr->IsImmuneToEMP) {
            goto loop_continue;
        }
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    loco = foot->Locomotor_Ptr();
    loco->Power_Off();

    JMP(0x00492EB8);

    /**
     *  Continue looping through the cell occupiers.
     */
loop_continue:
    JMP(0x00492F78);
}


/**
 *  Main function for patching the hooks.
 */
void EMPulseClassExtension_Hooks()
{
    /**
     *  EMP Immunity
     */
    Patch_Jump(0x00492E84, _EMPulseClass_Create_Foot_EMPImmune_Patch);
    Patch_Jump(0x00492C45, _EMPulseClass_Create_Building_EMPImmune_Patch);
}