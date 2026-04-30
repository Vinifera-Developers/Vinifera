/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended InfantryClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "infantryext_hooks.h"

#include "animtype.h"
#include "asserthandler.h"
#include "building.h"
#include "buildingtype.h"
#include "extension.h"
#include "fetchres.h"
#include "hooker.h"
#include "house.h"
#include "infantry.h"
#include "infantryext_init.h"
#include "infantrytype.h"
#include "infantrytypeext.h"
#include "language.h"
#include "options.h"
#include "rules.h"
#include "sideext.h"
#include "syringe.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tiberium.h"
#include "tiberiumext.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "voc.h"
#include "wwkeyboard.h"


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
    const char* _Full_Name(void) const;
};


/**
 *  Fetches the image data for this infantry unit.
 *
 *  The image data for the infantry differs from normal if this is a spy. A spy always
 *  appears like a regular infantry to the non-owning players.
 *
 *  Infantry currently in webs also display a different image.
 *
 *  This implementation adds support for side-specific disguises and also fixes
 *  a bug in the original game where friendly Spies were shown as disguised.
 *
 *  @author: ZivDero, Rampastring
 */
const ShapeSet* InfantryClassExt::_Get_Image_Data() const
{
    if (Doing == DO_STRUGGLE && Rule->WebbedInfantry) {
        return Rule->WebbedInfantry->Get_Image_Data();
    }

    if (!House->Is_Ally(PlayerPtr) && Class->IsDisguised) {

        const auto disguise = SideClassExtension::Get_Disguise(House);
        if (disguise) {
            return disguise->Image;
        }
    }

    return ObjectClass::Get_Image_Data();
};


/**
 *  Fetches the full name for this infantry unit.
 *
 *  This implementation adds support for side-specific disguises and also fixes
 *  a bug in the original game where friendly Spies were shown as disguised.
 *
 *  @author: tomsons26/ZivDero, Rampastring
 */
const char* InfantryClassExt::_Full_Name(void) const
{
    assert(IsActive);

    if (IsTechnician) {
        return Fetch_String(TXT_TECHNICIAN);
    }

    if (Class->IsDisguised && !House->Is_Ally(PlayerPtr) && Rule->Disguise != NULL) {
        const auto disguise = SideClassExtension::Get_Disguise(House);
        if (disguise) {
            return disguise->GivenName.c_str();
        }

        return Rule->Disguise->GivenName.c_str();
    }

    return Class->GivenName.c_str();
}


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
    return std::min(tech->TClass->MaxStrength * damage, (float)(tech->Strength-1));
}


/**
 *  Patch to intercept the engineer capture checks.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D35F9, _InfantryClass_Per_Cell_Process_Engineer_Capture_Damage_Patch, 0)
{
    GET(InfantryClass *, this_ptr, ESI);
    GET(TechnoClass *, tech, EDI);      // From "cellptr->Cell_Building()".

    /**
     *  If the target buildings health is low enough, go ahead and capture it.
     *  Changed to use Rule->EngineerCaptureLevel.
     * 
     *  @author: CCHyper
     */
    if (tech->HealthRatio <= Rule->EngineerCaptureLevel) {
        goto capture;
    }

    /**
     *  Health is still not low enough, go ahead and apply some more damage to it.
     */
    int damage = Get_Engineer_Damage(tech);
    tech->Take_Damage(damage, 0, Rule->C4Warhead, this_ptr, true);

    /**
     *  Spring the DESTROYED_BY_ANYTHING event and remove this infantry.
     */
spring_and_delete:
    return 0x004D378D;

    /**
     *  Processing capturing of the target building.
     */
capture:
    return 0x004D36E1;
}


/**
 *  #issue-264
 * 
 *  Implements EnterTransportSound for infantry when they enter a transport.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D3A7B, _InfantryClass_Per_Cell_Process_Transport_Attach_Sound_Patch, 0)
{
    GET(InfantryClass *, this_ptr, ESI);
    GET(TechnoClass *, techno, EDI);        // Radio contact

    /**
     *  Stolen bytes/code.
     */
    techno->Cargo.Attach(this_ptr);

    /**
     *  If this transport we are entering has a passenger entering sound, play it now.
     */
    auto radio_technotypeext = Extension::Fetch(techno->TClass);
    if (radio_technotypeext->EnterTransportSound != VOC_NONE) {
        Static_Sound(radio_technotypeext->EnterTransportSound, techno->Position);
    }

    return 0x004D3A87;
}


/**
 *  #issue-226
 * 
 *  Implements IsMechanic for infantry when searching for targets.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D87E9, _InfantryClass_Firing_AI_Mechanic_Patch, 0)
{
    GET(InfantryClass *, this_ptr, EBP);
    GET(ObjectClass *, targ, ESI);      // TarCom as ObjectClass.

    auto infantrytypeext = Extension::Fetch(this_ptr->Class);

    /**
     *  Is this infantry a "dual healer" (can it heal both infantry and units)?
     */
    if (infantrytypeext->IsOmniHealer) {

        /**
         *  Is the target being queried a unit, aircraft or infantry? If so, make
         *  sure this infantry is a mechanic before allowing it to heal the unit.
         */
        if (targ->RTTI == RTTI_UNIT || 
            (targ->RTTI == RTTI_AIRCRAFT && !targ->In_Air()) || 
            targ->RTTI == RTTI_INFANTRY || 
            (targ->RTTI == RTTI_BUILDING && targ->TClass->UndeploysInto != nullptr && !reinterpret_cast<BuildingClass*>(targ)->Class->IsConstructionYard)) {
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
        if (targ->RTTI == RTTI_UNIT || 
            (targ->RTTI == RTTI_AIRCRAFT && !targ->In_Air()) || 
            (targ->RTTI == RTTI_BUILDING && targ->TClass->UndeploysInto != nullptr && !reinterpret_cast<BuildingClass*>(targ)->Class->IsConstructionYard)) {
            goto health_ratio_check;
        }

    /**
     *  Original code.
     */
    } else if (targ->RTTI == RTTI_INFANTRY) {
        goto health_ratio_check;
    }

assign_NULL_target:
    return 0x004D8824;

    /**
     *  Check the targets health ratio.
     */
health_ratio_check:
    return 0x004D87F5;
}


/**
 *  #issue-226
 * 
 *  Implements IsMechanic and IsDualHealer for infantry when deciding what action to perform.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D7168, _InfantryClass_What_Action_Mechanic_Patch, 0)
{
    GET(InfantryClass *, this_ptr, EDI);
    GET(/*const */ObjectClass *, object, ESI);  // target

    auto infantrytypeext = Extension::Fetch(this_ptr->Class);

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
        if (object->RTTI == RTTI_UNIT ||
            object->RTTI == RTTI_AIRCRAFT || 
            object->RTTI == RTTI_INFANTRY || 
            (object->RTTI == RTTI_BUILDING && object->TClass->UndeploysInto != nullptr && !reinterpret_cast<BuildingClass*>(object)->Class->IsConstructionYard)) {

            /**
             *  If we are force-moving into an Transport, don't try to heal it!
             */
            if (object->TClass->MaxPassengers > 0) {
                if (Keyboard->Down(Options.KeyForceMove1) || Keyboard->Down(Options.KeyForceMove2)) {
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
        if (object->RTTI == RTTI_UNIT ||
            object->RTTI == RTTI_AIRCRAFT || (object->RTTI == RTTI_BUILDING && object->TClass->UndeploysInto != nullptr && !reinterpret_cast<BuildingClass*>(object)->Class->IsConstructionYard)) {

            /**
             *  If we are force-moving into an Transport, don't try to heal it!
             */
            if (object->TClass->MaxPassengers > 0) {
                if (Keyboard->Down(Options.KeyForceMove1) || Keyboard->Down(Options.KeyForceMove2)) {
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
    return 0x004D71B0;

    /**
     *  Show the guard area mouse cursor over us.
     */
guard_area:
    return 0x004D71A1;

    /**
     *  Check the targets health ratio.
     */
health_ratio_check:
    return 0x004D7178;
}


/**
 *  #issue-226
 * 
 *  Allow all foot objects to be valid targets when this infantry deals negative damage.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D5AB4, _InfantryClass_Can_Fire_Target_Check_Patch, 0)
{
    GET_STACK(AbstractClass *, target, 0x10);

    TechnoClass* techno = dynamic_cast<TechnoClass*>(target);
    R->EAX(techno);

    return 0x004D5ACB;
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
DEFINE_HOOK(0x004D8C83, _InfantryClass_Doing_AI_JumpJet_Idle_Patch, 0)
{
    GET(InfantryClass *, this_ptr, ESI);

    auto infantrytype = reinterpret_cast<const InfantryTypeClass *>(this_ptr->Class_Of());

    /**
     *  Stolen code.
     * 
     *  If infantry is prone, set DO_PRONE.
     */
    if (this_ptr->IsProne) {
        return 0x004D8B12;
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

    return 0x004D8CA1;
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
DEFINE_HOOK(0x004D50C9, _InfantryClass_AI_JumpJet_Idle_Between_Firing_Patch, 0)
{
    GET(InfantryClass *, this_ptr, ESI);

    auto infantrytype = reinterpret_cast<const InfantryTypeClass *>(this_ptr->Class_Of());

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

    return 0x004D50E0;
}


/**
 *  #issue-80
 * 
 *  Fixes the bug where the Jumpjet uses the wrong DoType when not moving
 *  but actually in the air.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D9076, _InfantryClass_Movement_AI_JumpJet_Not_Moving_Patch, 0)
{
    GET(InfantryClass *, this_ptr, EBP);

    auto infantrytype = reinterpret_cast<const InfantryTypeClass *>(this_ptr->Class_Of());

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

    return 0x004D9087;
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
DEFINE_HOOK(0x004D88FA, _InfantryClass_Firing_AI_JumpJet_In_Air_Patch, 0)
{
    GET(InfantryClass *, this_ptr, EBP);

    /**
     *  Make sure its in the air before assigning the hover firing graphic sequence.
     */
    if (this_ptr->In_Air()) {
        this_ptr->Do_Action(DO_FIREFLY);
    } else {
        this_ptr->Do_Action(DO_FIRE_WEAPON);
    }

    return 0x004D8933;
}


/**
 *  Uses a new extension value as the damage Tiberium deals to infantry.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004D3F5D, _InfantryClass_Per_Cell_Process_Tiberium_Damage_Patch, 0)
{
    GET(int, tib_id, EAX);

    int damage = Extension::Fetch(Tiberiums[tib_id])->DamageToInfantry;

    R->EAX(damage);
    R->Stack(0x10, damage);

    return 0x004D3F8E;
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


DEFINE_HOOK(0x004D8BE4, _InfantryClass_Doing_AI_Fix_Invalid_Facing_Set, 0)
{
    GET(InfantryClass*, inf, ESI);
    _Set_Infantry_Facing_After_Doing_Check_For_Do_Nothing(inf);
    return 0x004D8C14;
}


/**
 *  Fixes an exploit where hijackers are able to hijack vehicles of their allies.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x004D7267, _InfantryClass_What_Action_Prevent_Hijacking_Allied_Vehicles, 0)
{
    GET(TechnoClass*, target, ESI);
    GET(InfantryClass*, this_ptr, EDI);

    if (this_ptr->House->Is_Ally(target)) {
        // The target is allied to the hijacker. Move on.
        return 0x004D72B7;
    }

    // Move to harvester truce check.
    return 0x004D7277;
}

/*
*  Reimplements InfantryClass::What_Action when the identified object is an armory.
*  No longer transmits a message to the armory requesting it to establish radio connection (which only allows one unit at a time).
*  Instead, simply checks for the armory's conditions (unit's veterancy and armory's ammo) to determine if the unit can go in or not.
*  This also allows additional units to be ordered into the armory even if other unit(s) are already on their way to it.
* 
*  @author: JoyfulShush
*/
DEFINE_HOOK(0x004D7355, _InfantryClass_What_Action_Armory_Action_Patch, 0)
{
    GET(BuildingClass*, object, ESI);
    GET(InfantryClass*, this_ptr, EDI);

    ActionType action = ACTION_ENTER;

    if (this_ptr->Crew.Is_Elite()) {
        action = ACTION_NO_ENTER;
    }

    if (object->Ammo <= 0) {
        action = ACTION_NO_ENTER;
    }

    R->EBX(action);

    return 0x004D738E;
}

/*
 *  Reimplements InfantryClass::What_Action when the identified object is a hospital.
 *  No longer transmits a message to the hospital requesting it to establish radio connection (which only allows one unit at a time).
 *  Instead, simply checks for the hospital's conditions (unit's health and hospital's ammo) to determine if the unit can go in or not.
 *  This also allows additional units to be ordered into the hospital even if other unit(s) are already on their way to it.
 * 
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D72F2, _InfantryClass_What_Action_Hospital_Action_Patch, 0)
{
    GET(BuildingClass*, object, ESI);
    GET(InfantryClass*, this_ptr, EDI);

    ActionType action = ACTION_ENTER;

    if (this_ptr->Strength >= this_ptr->Class->MaxStrength) {
        action = ACTION_NO_ENTER;
    }

    if (object->Ammo <= 0) {
        action = ACTION_NO_ENTER;
    }

    R->EBX(action);

    return 0x004D731A;
}

/*
 *  Reimplements InfantryClass::Assign_Destination at the part that identifies units with mission "MISSION_ENTER",
 *  at a part where units are communicating with a building that has a radio buddy.
 *  For hospitals and armories, this typically means that a unit got the permission to dock into the hospital/armory.
 *  Other units will then go near the hospital/armory (due to being assigned MISSION_MOVE) and will constantly contact the hospital/armory to try to enter it.
 *  Once the hospital/armory finish with the unit inside them, they dismiss contact with them and are available to accept requests from another unit.
 * 
 *  All other buildings preserve the original behavior that they had before.
 * 
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D4251, _Assign_Destination_Hospital_Armory_Queue_Patch, 9)
{
    GET(InfantryClass*, this_ptr, EBP);
    GET(BuildingClass*, object, EBX);    
    
    if (object->Class->IsHospital || object->Class->IsArmory) {
        if (object->Ammo >= 0) {
            this_ptr->Assign_Archive_Target(object);
            this_ptr->field_20C = object; // in IDA: "__ObjectCloseToMe"; in practice, seems to be the object that becomes the "target" of this unit.
            this_ptr->Assign_Mission(MISSION_MOVE);
            goto success;
        }
    }

    // original behavior
    this_ptr->Assign_Archive_Target(nullptr);
    
    success:
    return 0x004D425A;
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

    Patch_Jump(0x004D90B0, &InfantryClassExt::_Get_Image_Data);
    Patch_Jump(0x004D77A0, &InfantryClassExt::_Full_Name);
}
