/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INFANTRYEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended InfantryClass.
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
#include "infantryext_hooks.h"

#include "animtype.h"
#include "infantryext_init.h"
#include "infantry.h"
#include "infantrytype.h"
#include "infantrytypeext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "building.h"
#include "buildingtype.h"
#include "tagtype.h"
#include "house.h"
#include "housetype.h"
#include "target.h"
#include "voc.h"
#include "tibsun_globals.h"
#include "extension.h"
#include "options.h"
#include "rules.h"
#include "wwkeyboard.h"
#include "tiberium.h"
#include "tiberiumext.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "tibsun_inline.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "sideext.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(InfantryClass)
{
public:
    const ShapeSet* _Get_Image_Data() const;
};


/**
 *  Fetches the image data for this infantry unit.
 *
 *  The image data for the infantry differs from normal if this is a spy. A spy always
 *  appears like a regular infantry to the non-owning players.
 *
 *  Infantry currently in webs also display a different image.
 *
 *  @author: ZivDero
 */
const ShapeSet* InfantryClassExt::_Get_Image_Data() const
{
    if (Doing == DO_STRUGGLE && Rule->WebbedInfantry) {
        return Rule->WebbedInfantry->Get_Image_Data();
    }

    if (!IsOwnedByPlayer && Class->IsDisguised) {

        const auto disguise = SideClassExtension::Get_Disguise(House);
        if (disguise) {
            return disguise->Image;
        }
    }

    return ObjectClass::Get_Image_Data();
};


/**
 *  #issue-635
 * 
 *  Fixes a bug where EngineerDamage was not used to calculate the engineer damage.
 * 
 *  @author: CCHyper
 */
static int Get_Engineer_Damage(TechnoClass *tech)
{
    float damage = Rule->EngineerDamage;    // Was "Rule->ConditionRed * 0.5f"
    return std::min((tech->TClass->MaxStrength * damage), (float)(tech->Strength-1));
}


/** 
 *  Is the target buildings health low enough to be captured? 
 * 
 *  @author: CCHyper
 */
static bool Health_Low_Enough_To_Capture(TechnoClass *tech)
{
    /**
     *  #issue-633
     * 
     *  Changed to use Rule->EngineerCaptureLevel.
     * 
     *  @author: CCHyper
     */
    return tech->HealthRatio <= Rule->EngineerCaptureLevel;
}


/**
 *  Patch to intercept the engineer capture checks.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_Per_Cell_Process_Engineer_Capture_Damage_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TechnoClass *, tech, edi);      // From "cellptr->Cell_Building()".
    GET_REGISTER_STATIC(bool, iscapturable, bl);
    static int damage;

    /**
     *  If the target buildings health is low enough, go ahead and capture it.
     */
    if (Health_Low_Enough_To_Capture(tech)) {
        goto capture;
    }

    /**
     *  Health is still not low enough, go ahead and apply some more damage to it.
     */
    damage = Get_Engineer_Damage(tech);
    tech->Take_Damage(damage, 0, Rule->C4Warhead, this_ptr, true);

    /**
     *  Spring the DESTROYED_BY_ANYTHING event and remove this infantry.
     */
spring_and_delete:
    JMP(0x004D378D);

    /**
     *  Processing capturing of the target building.
     */
capture:
    JMP(0x004D36E1);
}


/**
 *  #issue-264
 * 
 *  Implements EnterTransportSound for infantry when they enter a transport.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_Per_Cell_Process_Transport_Attach_Sound_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, esi);
    GET_REGISTER_STATIC(TechnoClass *, techno, edi);        // Radio contact
    static TechnoTypeClassExtension *radio_technotypeext;

    /**
     *  Stolen bytes/code.
     */
    techno->Cargo.Attach(this_ptr);

    /**
     *  If this transport we are entering has a passenger entering sound, play it now.
     */
    radio_technotypeext = Extension::Fetch(techno->TClass);
    if (radio_technotypeext->EnterTransportSound != VOC_NONE) {
        Static_Sound(radio_technotypeext->EnterTransportSound, techno->Position);
    }

    JMP(0x004D3A87);
}


/**
 *  #issue-226
 * 
 *  Implements IsMechanic for infantry when searching for targets.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_Firing_AI_Mechanic_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, ebp);
    GET_REGISTER_STATIC(ObjectClass *, targ, esi);      // TarCom as ObjectClass.
    static InfantryTypeClassExtension *infantrytypeext;

    infantrytypeext = Extension::Fetch(this_ptr->Class);

    /**
     *  Is this infantry a "dual healer" (can it heal both infantry and units)?
     */
    if (infantrytypeext->IsOmniHealer) {

        /**
         *  Is the target being queried a unit, aircraft or infantry? If so, make
         *  sure this infantry is a mechanic before allowing it to heal the unit.
         */
        if (targ->RTTI == RTTI_UNIT || (targ->RTTI == RTTI_AIRCRAFT && !targ->In_Air()) || targ->RTTI == RTTI_INFANTRY) {
            goto health_ratio_check;
        }

    /**
     *  Is this infantry a mechanic?
     */
    } else if (infantrytypeext->IsMechanic) {

        /**
         *  Is the target being queried a unit or aircraft? If so, make sure this
         *  infantry is a mechanic before allowing it to heal the unit.
         */
        if (targ->RTTI == RTTI_UNIT || (targ->RTTI == RTTI_AIRCRAFT && !targ->In_Air())) {
            goto health_ratio_check;
        }

    /**
     *  Original code.
     */
    } else if (targ->RTTI == RTTI_INFANTRY) {
        goto health_ratio_check;
    }

assign_NULL_target:
    JMP(0x004D8824);

    /**
     *  Check the targets health ratio.
     */
health_ratio_check:
    JMP(0x004D87F5);
}


/**
 *  #issue-226
 * 
 *  Implements IsMechanic and IsDualHealer for infantry when deciding what action to perform.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_What_Action_Mechanic_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, edi);
    GET_REGISTER_STATIC(/*const */ObjectClass *, object, esi);  // target
    static InfantryTypeClassExtension *infantrytypeext;

    infantrytypeext = Extension::Fetch(this_ptr->Class);

    /**
     *  Is this infantry a "dual healer" (can it heal both infantry and units)?
     */
    if (infantrytypeext->IsOmniHealer) {

        /**
         *  If the mouse is over ourself, show the guard area cursor.
         */
        if (object == this_ptr) {
            goto guard_area;
        }

        /**
         *  Is the target being queried a unit, aircraft or infantry? If so, make
         *  sure this infantry is a mechanic before allowing it to heal the unit.
         */
        if (object->RTTI == RTTI_UNIT || object->RTTI == RTTI_AIRCRAFT || object->RTTI == RTTI_INFANTRY) {

            /**
             *  If we are force-moving into an Transport, don't try to heal it!
             */
            if (object->TClass->MaxPassengers > 0) {
                if (WWKeyboard->Down(Options.KeyForceMove1) || WWKeyboard->Down(Options.KeyForceMove2)) {
                    goto next_check;
                }
            }

            /**
             *  Before return ACTION_HEAL, check the targets health.
             */
            goto health_ratio_check;
        }

    /**
     *  Is this infantry a mechanic?
     */
    } else if (infantrytypeext->IsMechanic) {

        /**
         *  If the mouse is over ourself, show the guard area cursor.
         */
        if (object == this_ptr) {
            goto guard_area;
        }

        /**
         *  Is the target being queried a unit or aircraft? If so, make sure this
         *  infantry is a mechanic before allowing it to heal the unit.
         */
        if (object->RTTI == RTTI_UNIT || object->RTTI == RTTI_AIRCRAFT) {

            /**
             *  If we are force-moving into an Transport, don't try to heal it!
             */
            if (object->TClass->MaxPassengers > 0) {
                if (WWKeyboard->Down(Options.KeyForceMove1) || WWKeyboard->Down(Options.KeyForceMove2)) {
                    goto next_check;
                }
            }

            /**
             *  Before return ACTION_HEAL, check the targets health.
             */
            goto health_ratio_check;
        }

    /**
     *  Original code.
     */
    } else if (object->RTTI == RTTI_INFANTRY) {

        /**
         *  If the mouse is over ourself, show the guard area cursor.
         */
        if (object == this_ptr) {
            goto guard_area;
        }

        /**
         *  Before return ACTION_HEAL, check the targets health.
         */
        goto health_ratio_check;
    }

next_check:
    JMP(0x004D71B0);

    /**
     *  Show the guard area mouse cursor over us.
     */
guard_area:
    JMP(0x004D71A1);

    /**
     *  Check the targets health ratio.
     */
health_ratio_check:
    JMP(0x004D7178);
}


/**
 *  #issue-226
 * 
 *  Allow all foot objects to be valid targets when this infantry deals negative damage.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_Can_Fire_Target_Check_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, esi);
    GET_STACK_STATIC(AbstractClass *, target, esp, 0x10);
    GET_STACK_STATIC(int, which, esp, 0x14);
    static FootClass *targ;

    targ = Target_As_Foot(target);
    if (targ == nullptr) {
        goto return_FIRE_ILLEGAL;
    }

health_ratio_check:
    JMP_REG(ecx, 0x004D5ACF);

return_FIRE_ILLEGAL:
    JMP(0x004D5AE8);
}


/**
 *  #issue-80
 * 
 *  Fixes the bug where the Jumpjet uses the wrong DoType when idle on the
 *  ground. This was because the original code did not check if the infantry
 *  was actually in the air or not and always assumed it was on the ground, as
 *  a result it was always setting DO_STAND_READY.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_Doing_AI_JumpJet_Idle_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, esi);
    static const InfantryTypeClass *infantrytype;

    infantrytype = reinterpret_cast<const InfantryTypeClass *>(this_ptr->Class_Of());

    /**
     *  Stolen code.
     * 
     *  If infantry is prone, set DO_PRONE.
     */
    if (this_ptr->IsProne) {
        JMP(0x004D8B12);
    }

    if (infantrytype->IsJumpJet) {

        /**
         *  This is a Jumpjet infantry, make sure its in the air before
         *  assigning the hover graphic sequence.
         */
        if (this_ptr->In_Air()) {
            this_ptr->Do_Action(DO_HOVER, true);

        } else {
            this_ptr->Do_Action(DO_STAND_READY, true);
        }

    /**
     *  Handle normal infantry.
     */
    } else {
        this_ptr->Do_Action(DO_STAND_READY, true);
    }

    JMP(0x004D8CA1);
}


/**
 *  #issue-80
 * 
 *  Fixes the bug where the Jumpjet uses the wrong DoType when on the
 *  ground and in between firing rounds. This was because the original code
 *  did not check if the infantry was actually in the air and assumed it always
 *  is, thus setting DO_STAND_READY.
 * 
 *  @warning: This patch is within a branch that has already checked if the
 *            infantry is firing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_AI_JumpJet_Idle_Between_Firing_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, esi);
    static const InfantryTypeClass *infantrytype;

    infantrytype = reinterpret_cast<const InfantryTypeClass *>(this_ptr->Class_Of());

    if (infantrytype->IsJumpJet) {

        /**
         *  This is a Jumpjet infantry, make sure its in the air before
         *  assigning the hover graphic sequence.
         */
        if (this_ptr->In_Air()) {
            this_ptr->Do_Action(DO_HOVER);

        } else {
            this_ptr->Do_Action(DO_STAND_READY);
        }

    /**
     *  Handle normal infantry.
     */
    } else {
        this_ptr->Do_Action(DO_STAND_READY);
    }

    /**
     *  Stolen code.
     * 
     *  Clear the firing flag.
     */
    this_ptr->IsFiring = false;

    JMP(0x004D50E0);
}


/**
 *  #issue-80
 * 
 *  Fixes the bug where the Jumpjet uses the wrong DoType when not moving
 *  but actually in the air.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_Movement_AI_JumpJet_Not_Moving_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, ebp);
    static const InfantryTypeClass *infantrytype;

    infantrytype = reinterpret_cast<const InfantryTypeClass *>(this_ptr->Class_Of());

    if (infantrytype->IsJumpJet) {

        /**
         *  This is a Jumpjet infantry, make sure its in the air before
         *  assigning the hover graphic sequence.
         */
        if (this_ptr->In_Air()) {
            this_ptr->Do_Action(DO_HOVER);

        } else {
            this_ptr->Do_Action(DO_STAND_READY);
        }

    /**
     *  Handle normal infantry.
     */
    } else {
        this_ptr->Do_Action(DO_STAND_READY);
    }

    JMP(0x004D9087);
}


/**
 *  #issue-81
 * 
 *  Fixes the bug where the Jumpjet uses the wrong DoType when firing on
 *  the ground. This was because the original code did not check if the infantry
 *  was actually in the air and assumed it always is, thus setting DO_FIREFLY.
 * 
 *  @warning: This patch is within a branch that has already checked if the
 *            infantry is controlled by the Jumpjet locomotor!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_InfantryClass_Firing_AI_JumpJet_In_Air_Patch)
{
    GET_REGISTER_STATIC(InfantryClass *, this_ptr, ebp);

    /**
     *  Make sure its in the air before assigning the hover firing graphic sequence.
     */
    if (this_ptr->In_Air()) {
        this_ptr->Do_Action(DO_FIREFLY);
    } else {
        this_ptr->Do_Action(DO_FIRE_WEAPON);
    }

    JMP(0x004D8933);
}


/**
 *  Uses a new extension value as the damage Tiberium deals to infantry.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_InfantryClass_Per_Cell_Process_Tiberium_Damage_Patch)
{
    GET_REGISTER_STATIC(int, tib_id, eax);

    static int damage;
    damage = Extension::Fetch(Tiberiums[tib_id])->DamageToInfantry;

    _asm mov eax, damage
    _asm mov [esp + 0x10], eax

    JMP(0x004D3F8E);
}


/**
 *  #issue-1080
 *
 *  Fixes an out-of-bounds DoControls read when an infantry is "doing nothing".
 *
 *  @author: Rampastring
 */
void _Set_Infantry_Facing_After_Doing_Check_For_Do_Nothing(InfantryClass* this_ptr)
{
    if (this_ptr->Doing == DO_NOTHING) {
        return;
    }

    FacingType facing = this_ptr->Class->DoControls[this_ptr->Doing].Finish;
    if (facing == FACING_NONE) {
        return;
    }

    Dir256 dirtype = Facing_Dir(facing);
    DirType ds = DirType(dirtype);
    this_ptr->PrimaryFacing.Set(ds);
}


DECLARE_PATCH(_InfantryClass_Doing_AI_Fix_Invalid_Facing_Set)
{
    GET_REGISTER_STATIC(InfantryClass*, inf, esi);
    _Set_Infantry_Facing_After_Doing_Check_For_Do_Nothing(inf);
    JMP(0x004D8C14);
}


/**
 *  Main function for patching the hooks.
 */
void InfantryClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    InfantryClassExtension_Init();

    Patch_Jump(0x004D88FA, &_InfantryClass_Firing_AI_JumpJet_In_Air_Patch);
    Patch_Jump(0x004D8C83, &_InfantryClass_Doing_AI_JumpJet_Idle_Patch);
    Patch_Jump(0x004D50C9, &_InfantryClass_AI_JumpJet_Idle_Between_Firing_Patch);
    Patch_Jump(0x004D9076, &_InfantryClass_Movement_AI_JumpJet_Not_Moving_Patch);
    Patch_Jump(0x004D5AB4, &_InfantryClass_Can_Fire_Target_Check_Patch);
    Patch_Jump(0x004D7168, &_InfantryClass_What_Action_Mechanic_Patch);
    Patch_Jump(0x004D87E9, &_InfantryClass_Firing_AI_Mechanic_Patch);
    Patch_Jump(0x004D3A7B, &_InfantryClass_Per_Cell_Process_Transport_Attach_Sound_Patch);
    Patch_Jump(0x004D35F9, &_InfantryClass_Per_Cell_Process_Engineer_Capture_Damage_Patch);
    Patch_Jump(0x004D3F5D, &_InfantryClass_Per_Cell_Process_Tiberium_Damage_Patch);
    Patch_Jump(0x004D8BE4, &_InfantryClass_Doing_AI_Fix_Invalid_Facing_Set);

    Patch_Jump(0x004D90B0, &InfantryClassExt::_Get_Image_Data);
}
