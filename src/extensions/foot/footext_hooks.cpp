/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FOOTEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended FootClass.
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
#include "footext_hooks.h"
#include "footext_functions.h"
#include "foot.h"
#include "technoext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "unit.h"
#include "unittype.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


 /**
  *  A fake class for implementing new member functions which allow
  *  access to the "this" pointer of the intended class.
  *
  *  @note: This must not contain a constructor or deconstructor!
  *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
  */
static class FootClassFake final : public FootClass
{
public:
    Cell _Search_For_Tiberium(int rad, bool a2);
};


/**
 * #issue-203
 *
 * Enables smarter harvester tiberium-seeking algorithm.
 *
 * Author: Rampastring
 */
Cell FootClassFake::_Search_For_Tiberium(int rad, bool a2)
{
    return Vinifera_FootClass_Search_For_Tiberium(this, rad, a2);
}


/**
 *  #issue-593
 * 
 *  Implements IsCanPassiveAcquire for TechnoTypes when the unit is in MISSION_MOVE.
 * 
 *  @author: CCHyper
 */
static bool Foot_Target_Something_Nearby_Coord(FootClass *this_ptr, ThreatType threat) { return this_ptr->Target_Something_Nearby(this_ptr->Get_Coord(), threat); }
DECLARE_PATCH(_FootClass_Mission_Move_Can_Passive_Acquire_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, esi);
    static TechnoClassExtension *technoclassext;

    technoclassext = Extension::Fetch<TechnoClassExtension>(this_ptr);

    /**
     *  Can this unit passively acquire new targets?
     */
    if (!technoclassext->Can_Passive_Acquire()) {
        goto finish_mission_process;
    }

    /**
     *  Find a fresh target within my range.
     */
    Foot_Target_Something_Nearby_Coord(this_ptr, THREAT_RANGE);

finish_mission_process:
    JMP(0x004A104B);
}


/**
 *  #issue-593
 * 
 *  Implements IsCanPassiveAcquire for TechnoTypes when the unit is in MISSION_GUARD.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_FootClass_Mission_Guard_Can_Passive_Acquire_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, esi);
    static TechnoClassExtension *technoclassext;

    technoclassext = Extension::Fetch<TechnoClassExtension>(this_ptr);

    /**
     *  Can this unit passively acquire new targets?
     */
    if (!technoclassext->Can_Passive_Acquire()) {
        goto continue_check;
    }

    /**
     *  Find a fresh target within my range.
     */
    if (!Foot_Target_Something_Nearby_Coord(this_ptr, THREAT_RANGE)) {
        goto random_animate;
    }

continue_check:
    JMP(0x004A1AD6);

random_animate:
    JMP(0x004A1ACC);
}


/**
 *  #issue-593
 * 
 *  Implements IsCanPassiveAcquire for TechnoTypes when the unit is in MISSION_GUARD_AREA.
 * 
 *  @author: CCHyper
 */
static bool Foot_Target_Something_Nearby_Archive(FootClass *this_ptr, ThreatType threat) { return this_ptr->Target_Something_Nearby(this_ptr->ArchiveTarget->Center_Coord(), threat); }
DECLARE_PATCH(_FootClass_Mission_Guard_Area_Can_Passive_Acquire_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, esi);
    static TechnoClassExtension *technoclassext;

    technoclassext = Extension::Fetch<TechnoClassExtension>(this_ptr);

    /**
     *  Can this unit passively acquire new targets?
     */
    if (!technoclassext->Can_Passive_Acquire()) {
        goto tarcom_check;
    }

    /**
     *  Find a fresh target in my area using the backup target.
     */
    Foot_Target_Something_Nearby_Archive(this_ptr, THREAT_AREA);

tarcom_check:
    JMP(0x004A2C04);
}


/**
 *  #issue-421
 * 
 *  Implements IdleRate for TechnoTypes.
 * 
 *  @author: CCHyper
 */
static bool Locomotion_Is_Moving_Now(FootClass *this_ptr) { return this_ptr->Locomotion->Is_Moving_Now(); }
DECLARE_PATCH(_FootClass_AI_IdleRate_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, esi);
    GET_REGISTER_STATIC(ILocomotion *, loco, edi);
    static TechnoTypeClassExtension *technotypeext;

    technotypeext = Extension::Fetch<TechnoTypeClassExtension>(this_ptr->Techno_Type_Class());

    /**
     *  Stolen bytes/code.
     * 
     *  If the object is currently moving, check to see if its time to update its walk frame.
     */
    if (Locomotion_Is_Moving_Now(this_ptr) && !(Frame % this_ptr->Techno_Type_Class()->WalkRate)) {
        ++this_ptr->TotalFramesWalked;

    /**
     *  Otherwise, if the object is not currently moving, check to see if its time to update its idle frame.
     */
    } else if (technotypeext->IdleRate > 0) {
        if (!Locomotion_Is_Moving_Now(this_ptr) && !(Frame % technotypeext->IdleRate)) {
            ++this_ptr->TotalFramesWalked;
        }
    }

    _asm { mov edi, loco }      // Restore EDI register.

    JMP_REG(edx, 0x004A5A12);
}


/**
 *  #issue-404
 * 
 *  A object with "CloakStop" set has no effect on the cloaking behavior.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_FootClass_Is_Allowed_To_Recloak_Cloak_Stop_BugFix_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TechnoTypeClass *, technotype, eax);
    static ILocomotion *loco;

    /**
     *  Is this unit flagged to only re-cloak when not moving?
     */
    if (technotype->CloakStop) {

        loco = this_ptr->Locomotor_Ptr();

        /**
         *  If the object is currently moving, then return false.
         * 
         *  The original code here called Is_Moving_Now, which returned
         *  false when the locomotor was on a slope or rotating, which
         *  breaks the CloakStop mechanic.
         */
        if (loco->Is_Moving()) {
            goto return_false;
        }
    }

    /**
     *  The unit can re-cloak.
     */
return_true:
    JMP_REG(ecx, 0x004A6897);

    /**
     *  The unit is not allowed to re-cloak.
     */
return_false:
    JMP_REG(ecx, 0x004A689B);
}


/**
 *  #issue-192
 * 
 *  IsInsignificant is not checked on FootClass objects.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_FootClass_Death_Announcement_IsInsignifcant_Patch)
{
    GET_REGISTER_STATIC(FootClass *, this_ptr, ecx);
    static const TechnoTypeClass *technotype;

    /**
     *  Stolen bytes/code here.
     */
    _asm { sub esp, 0x10 }

    /**
     *  Don't announce the death of objects we don't own.
     */
    if (!this_ptr->IsOwnedByPlayer) {
        goto function_return;
    }

    /**
     *  If this object is marked as "Insignificant", then the user
     *  should not hear any EVA notification when it is killed.
     */
    technotype = this_ptr->Techno_Type_Class();
    if (technotype->IsInsignificant) {
        goto function_return;
    }

    /**
     *  Continues to the Speak() call.
     */
continue_function:
    _asm { mov ecx, this_ptr }
    JMP(0x004A4D6D);

    /**
     *  Return from function.
     */
function_return:
    JMP(0x004A4DB5);
}


/**
 *  Main function for patching the hooks.
 */
void FootClassExtension_Hooks()
{
    Patch_Jump(0x004A4D60, &_FootClass_Death_Announcement_IsInsignifcant_Patch);
    Patch_Jump(0x004A6866, &_FootClass_Is_Allowed_To_Recloak_Cloak_Stop_BugFix_Patch);
    Patch_Jump(0x004A59E1, &_FootClass_AI_IdleRate_Patch);
    Patch_Jump(0x004A2BE7, &_FootClass_Mission_Guard_Area_Can_Passive_Acquire_Patch);
    Patch_Jump(0x004A1AAE, &_FootClass_Mission_Guard_Can_Passive_Acquire_Patch);
    Patch_Jump(0x004A102F, &_FootClass_Mission_Move_Can_Passive_Acquire_Patch);
    Patch_Jump(0x004A76F0, &FootClassFake::_Search_For_Tiberium);
}
