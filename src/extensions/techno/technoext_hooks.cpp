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
#include "technoext.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_inline.h"
#include "weapontype.h"
#include "weapontypeext.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "house.h"
#include "housetype.h"
#include "houseext.h"
#include "rules.h"
#include "rulesext.h"
#include "uicontrol.h"
#include "aircraft.h"
#include "aircraftext.h"
#include "aircrafttypeext.h"
#include "infantry.h"
#include "infantrytype.h"
#include "infantrytypeext.h"
#include "unittype.h"
#include "unittypeext.h"
#include "voc.h"
#include "vinifera_util.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


 /**
  *  #issue-977
  *
  *  Adds check for IsLegalTargetComputer when evaluating a target for attacking.
  *
  *  @author: CCHyper
  */
DECLARE_PATCH(_TechnoClass_Evaluate_Object_Is_Legal_Target_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, edi);
    GET_REGISTER_STATIC(const TechnoClass *, object, esi); // The target object being evaluated.
    static TechnoClassExtension *this_technoext;
    static TechnoClassExtension *object_technoext;
    static const TechnoTypeClass *object_tclass;
    static const TechnoTypeClassExtension *object_tclassext;

    //this_technoext = Extension::Fetch<TechnoClassExtension>(this_ptr);
    //object_technoext = Extension::Fetch<TechnoClassExtension>(object);

    object_tclass = object->Techno_Type_Class();
    object_tclassext = Extension::Fetch<TechnoTypeClassExtension>(object_tclass);

    /**
     *  Determine if the target is theoretically allowed to be a target.
     */
    if (!object_tclass->IsLegalTarget) {
        goto return_false;
    }

    /**
     *  Now, determine if "we" are owned by a non-human house and the target is not theoretically allowed to be a target.
     */
    if (!this_ptr->House->Is_Human_Control() && !object_tclassext->IsLegalTargetComputer) {
        goto return_false;
    }

    /**
     *  Target object passed the "theoretical" check, for now...
     */
continue_eval:
    JMP_REG(edx, 0x0062D4D8);

    /**
     *  Target object is not a "theoretically legal" target.
     */
return_false:
    JMP_REG(edx, 0x0062D8C0);
}


/**
 *  #issue-594
 * 
 *  Implements IsCanRetaliate for TechnoTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Is_Allowed_To_Retaliate_Can_Retaliate_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    static TechnoTypeClassExtension *technotypeext;

    technotypeext = Extension::Fetch<TechnoTypeClassExtension>(this_ptr->Techno_Type_Class());

    /**
     *  If this unit is flagged as no being allowed to retaliate to attacks, return false.
     */
    if (!technotypeext->IsCanRetaliate) {
        goto return_FALSE;
    }

    /**
     *  Stolen bytes/code.
     */
    if (this_ptr->House->Is_Human_Control() && this_ptr->TarCom) {
        goto return_FALSE;
    }

continue_checks:
    JMP(0x00636F26);

return_FALSE:
    JMP(0x006371E7);
}


/**
 *  #issue-357
 * 
 *  Creates an instance of the electric bolt from the firing techno to the target.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Fire_At_Electric_Bolt_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(WeaponTypeClass const *, weapon, ebx);
    GET_STACK_STATIC(TARGET, target, ebp, 0x8);
    static WeaponTypeClassExtension *weapontypeext;
    static TechnoClassExtension *technoext;

    /**
     *  Spawn the electric bolt.
     */
    weapontypeext = Extension::Fetch<WeaponTypeClassExtension>(weapon);
    if (weapontypeext->IsElectricBolt) {

        technoext = Extension::Fetch<TechnoClassExtension>(this_ptr);
        technoext->Electric_Bolt(target);

    /**
     *  Spawn the laser.
     */
    } else if (weapon->IsLaser) {
        goto is_laser;
    }

    JMP(0x006312CD);

is_laser:
    JMP_REG(edi, 0x00631231);
}


/**
 *  #issue-579
 * 
 *  Implements the Suicide (death on firing) logic for technos.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Fire_At_Suicide_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(WeaponTypeClass *, weap, ebx);
    GET_REGISTER_STATIC(BulletTypeClass *, bullet, edx);
    GET_STACK_STATIC(TARGET, target, ebp, 0x8);
    static WeaponTypeClassExtension *weapontypeext;
    static int damage;

    /**
     *  Stolen bytes/code.
     */
    if (!target) {
        goto return_null;
    }

    /**
     *  Fetch the extension instance for the firing weapon.
     */
    weapontypeext = Extension::Fetch<WeaponTypeClassExtension>(weap);

    /**
     *  Firing unit must be active in the game world when performing suicide.
     */
    if (this_ptr->IsActive && !this_ptr->IsInLimbo) {

        /**
         *  Explicitly delete the unit from the game world at this very moment.
         *  This is legacy behavior similar to that of Red Alert.
         */
        if (weapontypeext->IsSuicide && weapontypeext->IsDeleteOnSuicide) {
            DEV_DEBUG_INFO("Deleted: %s\n", this_ptr->Name());
            this_ptr->entry_E4();

        /**
         *  Deal full damage to the firing unit. The removal of the unit will
         *  go though the normal damage handling code.
         */
        } else if (weapontypeext->IsSuicide) {

            /**
             *  #TODO:
             *  We have to skip aircraft as they crash the game because
             *  they do not get removed correctly after taking full damage.
             *  
             *  This same crash happens in Red Alert 2 also, possible engine bug.
             */
            if (this_ptr->What_Am_I() == RTTI_AIRCRAFT) {
                goto limpet_check;
            }

            damage = this_ptr->Techno_Type_Class()->MaxStrength;
            this_ptr->Take_Damage(damage, 0, Rule->C4Warhead, nullptr, true, false);
        }

    }

    /**
     *  Continue checks.
     */
limpet_check:
    _asm { mov edi, target }    // Restore EDI to expected pointer.
    JMP(0x0063039B);

    /**
     *  Return null (didn't fire, no bullet returned).
     */
return_null:
    _asm { mov edi, target }
    JMP(0x006304D2);
}


/**
 *  Handle the player assigned mission and play the respective voice response.
 * 
 *  @author: CCHyper
 */
static void Techno_Player_Assign_Mission_Response_Switch(TechnoClass *this_ptr, MissionType mission)
{
    if (!this_ptr) {
        return;
    }

    if (!AllowVoice) {
        return;
    }

    TechnoClassExtension *technoext = Extension::Fetch<TechnoClassExtension>(this_ptr);

    switch (mission) {

        default:
        case MISSION_MOVE:
            this_ptr->Response_Move();
            break;

        case MISSION_ATTACK:
            this_ptr->Response_Attack();
            break;

        /**
         *  #issue-574
         * 
         *  Implements VoiceCapture, VoiceEnter, VoiceDeploy and VoiceHarvest.
         */
        case MISSION_CAPTURE:
            technoext->Response_Capture();
            break;

        case MISSION_ENTER:
            technoext->Response_Enter();
            break;

        case MISSION_UNLOAD:
            technoext->Response_Deploy();
            break;

        case MISSION_HARVEST:
            technoext->Response_Harvest();
            break;
    }
}


/**
 *  #issue-574
 * 
 *  Patch to allow additional voice responses.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Player_Assign_Mission_Response_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(MissionType, mission, edi);

    Techno_Player_Assign_Mission_Response_Switch(this_ptr, mission);

    JMP(0x0063167E);
}


/**
 *  #issue-434
 * 
 *  Implements Soylent value (refund amount override) for technos.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Refund_Amount_Soylent_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    static TechnoTypeClassExtension *technotypext;
    static TechnoTypeClass *technotype;
    static int cost;

    /**
     *  Stolen bytes/code.
     */
    technotype = this_ptr->Techno_Type_Class();

    /**
     *  Fetch the extension instance.
     */
    technotypext = Extension::Fetch<TechnoTypeClassExtension>(technotype);

    /**
     *  If the object has a soylent value defined, return this.
     */
    if (technotypext->SoylentValue > 0) {
        cost = technotypext->SoylentValue;
        goto return_amount;
    }

continue_function:
    _asm { mov eax, technotype }    // restore EAX pointer.
    JMP_REG(ecx, 0x0063809D);

return_amount:
    _asm { mov edi, [cost] }
    JMP(0x006380DC);
}


/**
 *  #issue-226
 * 
 *  Ensures infantry with IsMechanic only auto-target units and aircraft.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Greatest_Threat_Infantry_Mechanic_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(InfantryClass *, infantry_this_ptr, esi);
    GET_REGISTER_STATIC(ThreatType, method, ebx);
    static InfantryTypeClassExtension *infantrytypeext;

    /**
     *  #NOTE: This case is already within a infantry check.
     */

    method = (method & (THREAT_RANGE|THREAT_AREA));
    
    /**
     *  The following;
     *  - If this is a dual healer: Then infantry, vehicles and aircraft are valid targets.
     *  - If this is a mechanic: Then only consider vehicles and aircraft as valid targets.
     *  - Otherwise, we assume this is a medic and they can only consider other infantry to be a threat.
     * 
     *  #NOTE: Removed THREAT_AIR for IsMechanic and IsOmniHealer infantry and it causes
     *         them to chase down damaged friendly aircraft in the air.
     */
    infantrytypeext = Extension::Fetch<InfantryTypeClassExtension>(infantry_this_ptr->Class);
    if (infantrytypeext->IsOmniHealer) {
        method = method|(THREAT_INFANTRY|THREAT_VEHICLES/*|THREAT_AIR*/|THREAT_4000);
    } else if (infantrytypeext->IsMechanic) {
        method = method|(THREAT_VEHICLES/*|THREAT_AIR*/|THREAT_4000);
    } else {
        method = method|(THREAT_INFANTRY|THREAT_4000);
    }

    _asm { mov ebx, method }
    JMP(0x0062DDB1);
}


/**
 *  #issue-541
 * 
 *  Allow customisation of the infantry health bar draw position.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Draw_Health_Bars_Infantry_Draw_Pos_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, ebx);
    static int x_pos;
    static int y_pos;

    x_pos = UIControls->InfantryHealthBarDrawPos.X;
    y_pos = UIControls->InfantryHealthBarDrawPos.Y;

    _asm { mov ecx, [x_pos] }
    _asm { mov eax, [y_pos] }

    JMP_REG(esi, 0x0062C565);
}


/**
 *  #issue-541
 * 
 *  Allow customisation of the unit health bar draw position.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Draw_Health_Bars_Unit_Draw_Pos_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, ebx);
    static int x_pos;
    static int y_pos;

    x_pos = UIControls->UnitHealthBarDrawPos.X;
    y_pos = UIControls->UnitHealthBarDrawPos.Y;

    _asm { mov ecx, [x_pos] }
    _asm { mov eax, [y_pos] }

    JMP_REG(esi, 0x0062C5DF);
}


/**
 *  #issue-411
 * 
 *  Implements IsAffectsAllies for WarheadTypes.
 * 
 *  @note: This patch does not replace "stolen" code as per our implementation
 *         rules, this is because the call to ObjectClass::Take_Damage that follows
 *         is too much of a risk to not have correctly implemented.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Take_Damage_IsAffectsAllies_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_STACK_STATIC(int *, damage, esp, 0xEC);
    GET_STACK_STATIC(int, distance, esp, 0xF0);
    GET_STACK_STATIC(const WarheadTypeClass *, warhead, esp, 0xF4);
    GET_STACK_STATIC(TechnoClass *, source, esp, 0xF8);
    GET_STACK_STATIC8(bool, forced, esp, 0xFC);
    GET_STACK_STATIC(int, a6, esp, 0x100);
    static WarheadTypeClassExtension *warheadtypeext;
    static ResultType result;

    if (warhead) {

        /**
         *  Is the warhead that hit us one that affects units allied with its firing owner?
         */
        warheadtypeext = Extension::Fetch<WarheadTypeClassExtension>(warhead);
        if (!warheadtypeext->IsAffectsAllies) {

            /**
             *  If the source of the damage is an ally of ours, then reset
             *  the damage amount and return that we took no damage.
             */
            if (source && source->House->Is_Ally(this_ptr->House)) {
                *damage = 0;
                goto return_RESULT_NONE;
            }

        }

    }

    /**
     *  Stolen bytes/code.
     */
    _asm { mov ecx, a6 }

    /**
     *   Restore a few registers to be safe.
     */
    _asm { mov ebx, source }
    //_asm { mov edi, damage }
    JMP_REG(edx, 0x006328E5);

    /**
     *  Function returns RESULT_NONE.
     */
return_RESULT_NONE:
    JMP_REG(edi, 0x00632882);
}


/**
 *  #issue-404
 * 
 *  A object with "CloakStop" set has no effect on the cloaking behavior.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Is_Ready_To_Uncloak_Cloak_Stop_BugFix_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(bool, cloaked_by_house, al);

    /**
     *  Is this object unable to recloak or is it disabled by an EMP?
     */
    //if (!this_ptr->Is_Allowed_To_Recloak() && !this_ptr->IsCloakable || this_ptr->entry_2A4()) { // Original code.
    if (!this_ptr->Is_Allowed_To_Recloak() || !this_ptr->IsCloakable || this_ptr->entry_2A4()) {
        goto continue_check;
    }

    /**
     *  Object is not allowed to un-cloak at this time.
     */
return_false:
    JMP_REG(ecx, 0x0062F746);

    /**
     *  Continue checks.
     */
continue_check:
    _asm { mov bl, cloaked_by_house }
    JMP_REG(ecx, 0x0062F6DD);
}


/**
 *  #issue-391
 * 
 *  Extends the firing animation effect to support more facings.
 * 
 *  @author: CCHyper
 */
static AnimTypeClass *Techno_Get_Firing_Anim(TechnoClass *this_ptr, WeaponTypeClass *weapon)
{
    AnimTypeClass *anim = nullptr;

    int index = 0;
    int anim_count = weapon->Anim.Count();
    DirStruct dir = this_ptr->Fire_Direction();

    if (anim_count == 8) {

        index = Dir_To_8(dir);
        anim = weapon->Anim[index % FACING_COUNT];

    } else if (anim_count == 16) {

        index = Dir_To_16(dir);
        anim = weapon->Anim[index % 16];

    } else if (anim_count == 32) {

        index = Dir_To_32(dir);
        anim = weapon->Anim[index % 32];

    } else if (anim_count == 64) {

        index = Dir_To_64(dir);
        anim = weapon->Anim[index % 64];

    } else if (anim_count > 0) {

        index = 0;
        anim = weapon->Anim.Fetch_Head();

    } else {

        index = 0;
        anim = nullptr;

    }

    return anim;
}

DECLARE_PATCH(_TechnoClass_Fire_At_Weapon_Anim_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(WeaponTypeClass *, weapon, ebx);
    static AnimTypeClass *anim;

    anim = Techno_Get_Firing_Anim(this_ptr, weapon);

    _asm { mov edi, anim }
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
     *  Fetch the extension instance.
     */
    technotypeext = Extension::Fetch<TechnoTypeClassExtension>(technotype);

    /**
     *  Does this object have a custom cloaking sound? If so, use it.
     */
    if (technotypeext->CloakSound != VOC_NONE) {
        voc = technotypeext->CloakSound;
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
     *  Fetch the extension instance.
     */
    technotypeext = Extension::Fetch<TechnoTypeClassExtension>(technotype);

    /**
     *  Does this object have a custom decloaking sound? If so, use it.
     */
    if (technotypeext->UncloakSound != VOC_NONE) {
        voc = technotypeext->UncloakSound;
    }

    /**
     *  Play the sound effect at the objects location.
     */
    Sound_Effect(voc, *coord);

    JMP(0x00633BE7);
}


/**
 *  A patch that adds debug logging on null house pointers in TechnoClass::Owner().
 * 
 *  This is a common crash observed by mod developers and map creators, and
 *  aims to assist tracking down the offending object.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Null_House_Warning_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, ecx);
    static HouseClass *house;
    static int id;
    
    house = this_ptr->House;
    if (!house) {
        DEBUG_WARNING("Techno \"%s\" has an invalid house!", this_ptr->Name());
        Vinifera_DeveloperMode_Warning_WWMessageBox("Techno \"%s\" has an invalid house!", this_ptr->Name());
        Fatal("Null house pointer in TechnoClass::Owner!\n");

    } else {
        id = house->ID;
    }
    
    _asm { mov eax, id }
    _asm { ret }
}


/**
 *  #issue-356
 *
 *  Enables the deploy keyboard command to work for units that
 *  transform into a different unit on deploy.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_TechnoClass_2A0_Is_Allowed_To_Deploy_Unit_Transform_Patch)
{
    GET_REGISTER_STATIC(UnitTypeClass*, unittype, eax);
    static UnitTypeClassExtension* unittypeext;

    /**
     *  Stolen bytes/code.
     */
    if (unittype->DeploysInto != nullptr) {
        goto has_deploy_ability;

    } else if (unittype->MaxPassengers > 0) {
        goto has_deploy_ability;

    } else if (unittype->IsMobileEMP) {
        goto has_deploy_ability;
    }

    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);

    if (unittypeext->TransformsInto != nullptr) {
        goto has_deploy_ability;
    }

    /**
     *  The unit has no ability that allows it to deploy / unload.
     *  Mark that and continue function after the check.
     */
has_no_deploy_ability:
    _asm { mov eax, unittype }
    JMP_REG(ecx, 0x006320E0);

    /**
     *  The unit has some kind of an ability that allows it to deploy / unload.
     *  Continue function after the check.
     */
has_deploy_ability:
    _asm { mov eax, unittype }
    JMP_REG(ecx, 0x006320E5);
}


/**
 *  Accumulates killed value for house that killed that our unit.
 *  If they have gathered enough value, then strengthen that house.
 * 
 *  @author: Rampastring
 */
DECLARE_PATCH(_TechnoClass_Record_The_Kill_Strengthen_Killer_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, source, edi);
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(int, cost, ebx);
    static HouseClassExtension *houseext;
    static int value;

    /**
     *  Stolen bytes / code.
     *  Mark the damage source's house as the house that last hurt our owner.
     */
    this_ptr->House->WhoLastHurtMe = (HousesType)source->Owner();

    if (RuleExtension->IsStrengtheningEnabled) {
        houseext = Extension::Fetch<HouseClassExtension>(source->House);
        value = cost;

        // Buildings have a multiplier to their value.
        if (this_ptr->What_Am_I() == RTTI_BUILDING) {
            value = value * RuleExtension->StrengthenBuildingValueMultiplier;
        }

        houseext->StrengthenDestroyedCost += value;

        /**
         *  Strengthen the house if they have exceeded the threshold.
         */
        while (houseext->StrengthenDestroyedCost > RuleExtension->StrengthenDestroyedValueThreshold) {
            source->House->FirepowerBias += 0.01;
            source->House->ArmorBias += 0.01;

            houseext->StrengthenDestroyedCost -= RuleExtension->StrengthenDestroyedValueThreshold;
        }
    }

    /**
     *  Continue the kill recording process from the point where the
     *  game updates the points for the owner of the damage source.
     */
    JMP(0x0063382D);
}


void Create_Aircraft(TechnoClass* creator, WeaponTypeClassExtension* weaponext) 
{
    /**
     *  Spawn an aircraft and make it attack the same target as our creator.
     */
    AircraftClass *aircraft = reinterpret_cast<AircraftClass*>(weaponext->AircraftTypeToSpawn->Create_One_Of(creator->House));

    if (aircraft == nullptr) {
        return;
    }

    aircraft->PrimaryFacing.Set(creator->PrimaryFacing.Current());
    aircraft->Coord = creator->Coord;
    aircraft->FirepowerBias = creator->FirepowerBias;
    aircraft->ArmorBias = creator->ArmorBias;

    /**
     *  Use ScenarioInit to allow the aircraft to spawn on the same cell
     *  with its creator.
     */
    ScenarioInit++;

    if (aircraft->Unlimbo(aircraft->Coord, creator->PrimaryFacing.Current().Get_Dir())) {

        /**
         *  If the creator of the aircraft was targeting something, then assign its
         *  target as the target of the spawned aircraft.
         */
        if (creator->TarCom) {
            aircraft->Assign_Target(creator->TarCom);
            aircraft->Assign_Mission(MISSION_ATTACK);
            aircraft->Commence();
        }

        AircraftClassExtension *aircraftext = Extension::Fetch<AircraftClassExtension>(aircraft);
        aircraftext->Spawner = creator;
    }

    ScenarioInit--;
}


/**
 *  Allows a weapon to spawn aircraft when fired.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_TechnoClass_Fire_At_Spawn_Aircraft_Patch)
{
    GET_REGISTER_STATIC(TARGET, target, edi);
    GET_REGISTER_STATIC(WeaponTypeClass *, weapontype, ebx);
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    static WeaponTypeClassExtension *weaponext;
    static bool isbright;

    weaponext = Extension::Fetch<WeaponTypeClassExtension>(weapontype);

    if (!weaponext->IsSpawnAircraft) {
        goto original_code;
    }

    Create_Aircraft(this_ptr, weaponext);

    /**
     *  Exit from function.
     *  Actually it's best to just carry over, spawning the projectile
     *  seems to do some beneficial stuff (like setting the ROF timer).
     */
    // _asm { xor  eax, eax }
    // JMP_REG(edi, 0x006313C2);

    /**
     *  Launch the weapon as normal.
     */
original_code:

    isbright = weapontype->IsBright;

    /**
     *  Stolen bytes / code.
     */
    _asm { mov dl, [isbright] }
    JMP(0x006306BB);
}


/**
 *  #issue-1032
 *
 *  Fixes a bug where the Medic indicator is drawn for a medic that has
 *  not been discovered by the player.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_TechnoClass_Draw_Pips_No_Medic_Indicator_In_Shroud_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);

    if (!this_ptr->IsDiscoveredByPlayer || this_ptr->Combat_Damage() >= 0) {
        goto no_indicator;
    }

    /**
     *  Draw the medic indicator pip.
     */
display_indicator:
    JMP(0x00637B90);

    /**
     *  Continue the function, but skip drawing the medic indicator pip.
     */
no_indicator:
    JMP(0x00637BD2);
}


/**
 *  Main function for patching the hooks.
 */
void TechnoClassExtension_Hooks()
{
    Patch_Jump(0x00633C78, &_TechnoClass_Do_Cloak_Cloak_Sound_Patch);
    Patch_Jump(0x00633BD4, &_TechnoClass_Do_Uncloak_Uncloak_Sound_Patch);
    Patch_Jump(0x0063105C, &_TechnoClass_Fire_At_Weapon_Anim_Patch);
    Patch_Jump(0x0062F6B7, &_TechnoClass_Is_Ready_To_Uncloak_Cloak_Stop_BugFix_Patch);
    Patch_Jump(0x0062E6F0, &_TechnoClass_Null_House_Warning_Patch);
    Patch_Jump(0x006328DE, &_TechnoClass_Take_Damage_IsAffectsAllies_Patch);
    Patch_Jump(0x0062C5D5, &_TechnoClass_Draw_Health_Bars_Unit_Draw_Pos_Patch);
    Patch_Jump(0x0062C55B, &_TechnoClass_Draw_Health_Bars_Infantry_Draw_Pos_Patch);
    Patch_Jump(0x0062DD70, &_TechnoClass_Greatest_Threat_Infantry_Mechanic_Patch);
    Patch_Jump(0x00638095, &_TechnoClass_Refund_Amount_Soylent_Patch);
    Patch_Jump(0x00631661, &_TechnoClass_Player_Assign_Mission_Response_Patch);
    Patch_Jump(0x00630390, &_TechnoClass_Fire_At_Suicide_Patch);
    Patch_Jump(0x00631223, &_TechnoClass_Fire_At_Electric_Bolt_Patch);
    Patch_Jump(0x00636F09, &_TechnoClass_Is_Allowed_To_Retaliate_Can_Retaliate_Patch);
    Patch_Jump(0x006320C2, &_TechnoClass_2A0_Is_Allowed_To_Deploy_Unit_Transform_Patch);
    Patch_Jump(0x0063381A, &_TechnoClass_Record_The_Kill_Strengthen_Killer_Patch);
    Patch_Jump(0x006306B5, &_TechnoClass_Fire_At_Spawn_Aircraft_Patch);
    Patch_Jump(0x00637B83, &_TechnoClass_Draw_Pips_No_Medic_Indicator_In_Shroud_Patch);
    Patch_Jump(0x0062D4CA, &_TechnoClass_Evaluate_Object_Is_Legal_Target_Patch);
}
