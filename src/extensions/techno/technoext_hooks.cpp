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
#include "rules.h"
#include "rulesext.h"
#include "uicontrol.h"
#include "infantry.h"
#include "infantrytype.h"
#include "infantrytypeext.h"
#include "voc.h"
#include "vinifera_util.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "drawshape.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "textprint.h"


/**
  *  A fake class for implementing new member functions which allow
  *  access to the "this" pointer of the intended class.
  *
  *  @note: This must not contain a constructor or destructor.
  *
  *  @note: All functions must not be virtual and must also be prefixed
  *         with "_" to prevent accidental virtualization.
  */
class TechnoClassExt : public TechnoClass
{
public:
    void _Draw_Pips(Point2D& bottomleft, Point2D& bottomright, Rect& rect) const;
};


/**
 *  Draw the pips of this Techno.
 *
 *  @author: 08/08/1995 JLB - Created.                       
 *           10/06/1995 JLB - Displays the team group number.
 *           09/10/1996 JLB - Medic hack for red pip.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
void TechnoClassExt::_Draw_Pips(Point2D& bottomleft, Point2D& bottomright, Rect& rect) const
{
    int drawx = bottomleft.X + 6;
    int drawy = bottomleft.Y - 1;
    int dx = 4;
    int dy = 2;

    const ShapeFileStruct* pip_shapes = Class_Of()->PipShapes;
    const ShapeFileStruct* pips1 = Class_Of()->PipShapes;
    const ShapeFileStruct* pips2 = Class_Of()->Pip2Shapes;

    if (What_Am_I() != RTTI_BUILDING)
    {
        drawx = bottomleft.X - 5;
        drawy = bottomleft.Y;
        dx = 4;
        dy = 0;
        pip_shapes = pips2;
    }

    /*
    **	Because of ts-patches we have to check if the object is selected,
    **  or we'll draw pips for unselected objects
    */
    if (IsSelected)
    {
        /*
        **	Transporter type objects have a different graphic representation for the pips. The
        **	pip color represents the type of occupant.
        */
        const bool carrying_passengers = Techno_Type_Class()->Max_Passengers() > 0;
        if (carrying_passengers)
        {
            ObjectClass const* object = Cargo.Attached_Object();
            for (int index = 0; index < Class_Of()->Max_Pips(); index++)
            {
                int pip = 0;

                if (object != nullptr)
                {
                    pip = 1;
                    if (object->What_Am_I() == RTTI_INFANTRY)
                    {
                        pip = ((InfantryClass*)object)->Class->Pip;
                    }
                    object = object->Next;
                }
                CC_Draw_Shape(TempSurface, NormalDrawer, pip_shapes, pip, &Point2D(drawx + dx * index, drawy + dy * index), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
            }

        }
        else
        {
            /*
            **	Display number of how many attached objects there are. This is also used
            **	to display the fullness rating for a harvester.
            */
            int pips = Pip_Count();

            /*
            ** Check if it's a harvester, to show the right type of pips for the
            ** various minerals it could have harvested.
            */
            if (What_Am_I() == RTTI_UNIT && Techno_Type_Class()->PipScale == PIP_TIBERIUM)
            {
                int greentib = Storage.Get_Amount(TIBERIUM_RIPARIUS);
                int bluetib = greentib - Storage.Get_Total_Amount();

                TechnoTypeClass* harvtype = Techno_Type_Class();

                double green_fraction = (double)greentib / harvtype->Storage;
                int greenpips = harvtype->Max_Pips() * green_fraction + 0.5;

                double blue_fraction = (double)bluetib / harvtype->Storage;
                int bluepips = harvtype->Max_Pips() * blue_fraction + 0.5;

                for (int index = 0; index < Class_Of()->Max_Pips(); index++)
                {
                    int shape = 0;
                    if (index < pips)
                    {
                        if (bluepips)
                        {
                            shape = 5;
                            bluepips--;
                        }
                        else
                        {
                            shape = 1;
                            greenpips--;
                        }
                    }
                    CC_Draw_Shape(TempSurface, NormalDrawer, pip_shapes, shape, &Point2D(drawx + dx * index, drawy + dy * index), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
                }
            }
            else if (Techno_Type_Class()->PipScale == PIP_AMMO)
            {
                for (int index = 0; index < Class_Of()->Max_Pips() && pips > 0; index++, pips--)
                {
                    CC_Draw_Shape(TempSurface, NormalDrawer, pips2, 6, &Point2D(drawx + dx * index, drawy + dy * index - 3), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
                }
            }
            else if (What_Am_I() == RTTI_BUILDING && Techno_Type_Class()->PipScale == PIP_TIBERIUM)
            {
                for (int index = 0; index < Class_Of()->Max_Pips(); index++)
                {
                    int shape = 0;
                    if (pips > 0)
                    {
                        shape = 1;
                        pips--;
                    }
                    CC_Draw_Shape(TempSurface, NormalDrawer, pip_shapes, shape, &Point2D(drawx + dx * index, drawy + dy * index), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
                }
            }
            else if (Techno_Type_Class()->PipScale == PIP_CHARGE)
            {
                for (int index = 0; index < Class_Of()->Max_Pips(); index++)
                {
                    CC_Draw_Shape(TempSurface, NormalDrawer, pip_shapes, index < pips ? 1 : 0, &Point2D(drawx + dx * index, drawy + dy * index), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
                }
            }
        }
    }

    /*
    **	Special hack to display a red pip on the medic.
    */
    if (What_Am_I() == RTTI_INFANTRY && Combat_Damage() < 0)
    {
        CC_Draw_Shape(TempSurface, NormalDrawer, pips1, 6, &Point2D(drawx, drawy - 8), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
    }

    /*
    **	Print the primary (IsLeader) text.
    **  Maybe a leftover of RA formations?
    */
    if (What_Am_I() != RTTI_BUILDING)
    {
        this->entry_338(Point2D(bottomleft.X - 10, bottomleft.Y + 10), bottomleft, rect);
    }

    /*
    **	Display a veterancy pip is the unit is promoted.
    */
    int veterancy_shape = -1;
    if (Veterancy.Is_Veteran())
    {
        veterancy_shape = 7;
    }

    if (Veterancy.Is_Elite())
    {
        veterancy_shape = 8;
    }

    if (Veterancy.Is_Dumbass())
    {
        veterancy_shape = 12;
    }

    if (veterancy_shape != -1)
    {
        Point2D drawpoint(bottomright.X + 5, bottomright.Y + 2);
        if (What_Am_I() != RTTI_UNIT)
        {
            drawpoint.X += 5;
            drawpoint.Y += 4;
        }
        CC_Draw_Shape(TempSurface, NormalDrawer, pip_shapes, veterancy_shape, &drawpoint, &rect, SHAPE_WIN_REL | SHAPE_CENTER);
    }

    if (IsSelected)
    {
        /*
        **	Display what group this unit belongs to. This corresponds to the team
        **	number assigned with the <CTRL> key.
        */
        if (Group < 10)
        {
            char buffer[12];

            int yval = -30 /*ts-patches, vanilla -1*/;
            int group = Group + 1;

            // disabled for ts-patches
            /*if (Class_Of()->Max_Pips())
                yval -= 5;*/

            if (group == 10)
                group = 0;

            sprintf(buffer, "%d", group >= 10 ? 0 : group);
            Plain_Text_Print(buffer, TempSurface, &rect, &Point2D(bottomleft.X - 8 /*ts-patches, vanilla 4*/, bottomleft.Y + yval - 3), COLOR_WHITE, COLOR_TBLACK, TPF_FULLSHADOW | TPF_EFNT, COLORSCHEME_NONE, 1);
        }
    }
}



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
    Patch_Jump(0x0062D4CA, &_TechnoClass_Evaluate_Object_Is_Legal_Target_Patch);
    Patch_Jump(0x00637540, &TechnoClassExt::_Draw_Pips);
}
