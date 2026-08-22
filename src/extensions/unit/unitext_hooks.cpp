/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended UnitClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "unitext_hooks.h"

#include "asserthandler.h"
#include "ccini.h"
#include "debughandler.h"
#include "extension.h"
#include "findmake.h"
#include "hooker.h"
#include "house.h"
#include "housetype.h"
#include "housetypeext.h"
#include "infantry.h"
#include "iomap.h"
#include "options.h"
#include "rules.h"
#include "rulesext.h"
#include "spawnmanager.h"
#include "syringe.h"
#include "tacticalext.h"
#include "tag.h"
#include "target.h"
#include "technotype.h"
#include "technotypeext.h"
#include "house.h"
#include "warheadtype.h"
#include "weapontype.h"
#include "weapontypeext.h"
#include "tag.h"
#include "tagtype.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "unit.h"
#include "unitext.h"
#include "unitext_init.h"
#include "unittype.h"
#include "unittypeext.h"
#include "verses.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "voc.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "weapontype.h"
#include "weapontypeext.h"

#include <algorithm>


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(UnitClass)
{
public:
    void _Firing_AI();
    void _Draw_Voxel(unsigned int frame, int key, Rect& rect, Point2D& point, const Matrix3D& other_matrix, int color, int flags);
    int _Do_MISSION_HUNT();
    void _Rotation_AI();
    void _Approach_Target();
    static void _Read_INI(CCINIClass & ini);
};


/**
 *  Handles firing logic for this unit.
 *
 *  @author: 07/30/1996 JLB - Created
 *           ZivDero - Adjustments for Tiberian Sun
 */
void UnitClassExt::_Firing_AI()
{
    if (TarCom != nullptr && Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon)
    {
        /**
         *  Determine which weapon can fire. First check for the primary weapon. If that weapon
         *  cannot fire, then check any secondary weapon. If neither weapon can fire, then the
         *  failure code returned is that from the primary weapon.
         */
        WeaponSlotType primary = What_Weapon_Should_I_Use(TarCom);
        FireErrorType ok = Can_Fire(TarCom, primary);
        const WeaponTypeClass* weapon = Get_Weapon(primary)->Weapon;

        if (weapon && weapon->WarheadPtr && weapon->WarheadPtr->IsWebby && TarCom->RTTI == RTTI_INFANTRY)
        {
            InfantryClass* inf = reinterpret_cast<InfantryClass*>(TarCom);
            if (inf->ProneStruggleTimer.Value() > weapon->WarheadPtr->WebDuration / 4)
            {
                Assign_Target(nullptr);
                Assign_Mission(MISSION_GUARD);
                ok = FIRE_CANT;
            }
        }

        if ((ok == FIRE_OK || ok == FIRE_FACING) && Deploy_To_Fire())
        {
            Assign_Mission(MISSION_UNLOAD);
            return;
        }

        static int visceroid_stages[8] = { 0x64, 0x69, 0x6E, 0x73, 0x78, 0x7D, 0x5A, 0x5F };
        UnitClassExtension* ext;

        switch (ok)
        {
        case FIRE_OK:
            if (!Class->IsFireAnim)
                IsFiring = false;

            if (Class->IsLargeVisceroid || Class->IsSmallVisceroid)
            {
                Set_Stage(visceroid_stages[Dir_Facing(Direction(TarCom).Get_Dir())]);
                Set_Rate(5);
            }

            if (primary != WEAPON_SLOT_SECONDARY && weapon)
            {
                const int firing_sync = BurstIndex % weapon->Burst;
                if (firing_sync < 2 && Class->FiringSyncFrame[firing_sync] != -1)
                {
                    if (FiringSyncDelay == -1)
                        FiringSyncDelay = 2 * Class->FiringFrames - 1;
                    else if (FiringSyncDelay != Class->FiringSyncFrame[firing_sync])
                        return;
                }
            }

            Fire_At(TarCom, primary);
            break;

        case FIRE_FACING:
            if (Class->IsLockTurret || !Class->IsTurretEquipped)
            {
                if (NavCom == nullptr && !Locomotion->Is_Moving()) {
                    PrimaryFacing.Set_Desired(Direction(TarCom));
                    SecondaryFacing.Set_Desired(PrimaryFacing.Desired());
                }
            }
            else
            {
                SecondaryFacing.Set_Desired(Direction(TarCom));
            }
            break;

        case FIRE_ILLEGAL:
            if (Combat_Damage(primary) < 0)
            {
                if (!Is_Object(TarCom) || TarCom->RTTI != RTTI_UNIT || static_cast<ObjectClass*>(TarCom)->HealthRatio >= Rule->ConditionGreen)
                {
                    Assign_Target(nullptr);
                }
            }
            break;

        case FIRE_CANT:
            ext = Extension::Fetch(this);
            if (ext->SpawnManager)
                ext->SpawnManager->Abandon_Target();
            break;
            

        case FIRE_RANGE:
        case FIRE_MUST_DEPLOY:
            IsFiring = false;
            Approach_Target();
            break;

        case FIRE_CLOAKED:
            IsFiring = false;
            Do_Uncloak();
            break;

        default:
            return;
        }
    }
}


/**
 *  Draws the voxel model for this unit.
 *
 *  @author: ZivDero
 */
void UnitClassExt::_Draw_Voxel(unsigned int frame, int key, Rect& rect, Point2D& point, const Matrix3D& other_matrix, int color, int flags)
{
    Matrix3D matrix;
    Matrix3D::Multiply(Get_Voxel_Draw_Matrix(), other_matrix, &matrix);
    const auto typeext = Extension::Fetch(Class);
    const auto ext = Extension::Fetch(this);

    VoxelObject* voxel = nullptr;
    VoxelIndexClass* cache = nullptr;

    if (typeext->WaterAlt && Map[PositionCoord].Land_Type() == LAND_WATER && !IsOnBridge && HeightAGL < LEVEL_LEPTON_H) {
        voxel = &typeext->WaterVoxel;
        cache = &typeext->WaterVoxelIndex;
    } else if (typeext->NoSpawnAlt && ext->SpawnManager && !ext->SpawnManager->Docked_Count()) {
        voxel = &typeext->NoSpawnVoxel;
        cache = &typeext->NoSpawnVoxelIndex;
    } else {
        voxel = &Class->Voxel;
        cache = &Class->VoxelIndex;
    }

    Draw_Voxel(*voxel, frame, key, cache, rect, point, matrix, color, flags);
}


/**
 *  Patch that replaces the call to draw the voxel model to allow us to
 *  chose which voxel to draw.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x006527B1, _UnitClass_Draw_Voxel_Patch, 0)
{
    GET_STACK(unsigned int, frame, 0x58);
    GET_STACK(int, key, 0x40);
    LEA_STACK(Rect*, rect, 0x68);
    LEA_STACK(Point2D*, point, 0x50);
    LEA_STACK(Matrix3D*, matrix, 0x90);
    GET_STACK(int, color, 0x17C);
    GET_STACK(int, flags, 0x4C);
    GET(UnitClassExt*, this_ptr, EBP);

    this_ptr->_Draw_Voxel(frame, key, *rect, *point, *matrix, color, flags);

    R->EBX(color);

    return 0x006528E9;
}


/**
 *  #issue-550
 * 
 *  Implements IsOmniFire for units.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00656F99, _UnitClass_Can_Fire_IsOmniFire_Patch, 6)
{
    GET(WeaponTypeClass *, weapon, EBX);

    auto weapontypeext = Extension::Fetch(weapon);

    /**
     *  Do we need to perform a turn to face the target before firing?
     */
    if (weapontypeext->IsOmniFire) {
        return 0x00657030;
    }

    /**
     *  Continue the other normal checks.
     */
    return 0;
}


/**
 *  UnitClass::Rotation_AI re-implementation.
 *
 *  @author: ZivDero
 */
void UnitClassExt::_Rotation_AI()
{
    if (TarCom != nullptr && !IsRotating) {
        DirType dir = Direction(TarCom);

        if (Class->IsTurretEquipped) {

            /**
             *  #issue-550
             *
             *  Implements IsOmniFire for units.
             *
             *  @author: CCHyper
             */
            WeaponTypeClass* primary = PrimaryWeapon;
            if (primary != nullptr && !Extension::Fetch(primary)->IsOmniFire) {
                SecondaryFacing.Set_Desired(dir);
            }
            
        } else {

            /*
            **  Non turret equipped vehicles will rotate their body to face the target only
            **  if the vehicle isn't currently moving or facing the correct direction. This
            **  applies only to tracked vehicles. Wheeled vehicles never rotate to face the
            **  target, since they aren't maneuverable enough.
            */
            if (Class->Speed == SPEED_TRACK && NavCom == nullptr && !Locomotion->Is_Moving() && PrimaryFacing.Current() == dir) {
                PrimaryFacing.Set_Desired(dir);
            }
        }
    }

    if (Class->IsTurretSpins) {
        SecondaryFacing.Set(DirType(static_cast<Dir256>(SecondaryFacing.Current().Get_Facing<256>() + 8)));
    } else {

        IsRotating = false;
        if (Class->IsTurretEquipped) {

            if (SecondaryFacing.Is_Rotating()) {

                /*
                **  If no further rotation is necessary, flag that the rotation
                **  has stopped.
                */
                if (!Class->IsTurretSpins) {
                    IsRotating = SecondaryFacing.Is_Rotating();
                }
            } else {
                if (TarCom == nullptr) {
                    if (NavCom == nullptr) {
                        SecondaryFacing.Set_Desired(PrimaryFacing.Current());
                    }
                    else {
                        SecondaryFacing.Set_Desired(Direction(NavCom));
                    }
                }
            }
        }
    }
}


/**
 *  UnitClass::Approach_Target re-implementation.
 *
 *  @author: ZivDero
 */
void UnitClassExt::_Approach_Target()
{
    /**
     *  Only if there is a legal target should the approach check occur.
     */
    if (!House->Is_Human_Player() && TarCom != nullptr && NavCom == nullptr) {

        /**
         *  Special case:
         *  If this is for a unit that can crush infantry, and the target is
         *  infantry, AND the infantry is pretty darn close, then just try
         *  to drive over the infantry instead of firing on it.
         */
        TechnoClass* target = ::As_Techno(TarCom);
        if ((Class->IsCrusher || Has_Ability(ABILITY_CRUSHER)) && Distance(TarCom) < Rule->CrushDistance && target && target->Class_Of()->IsCrushable && (Class->IsAutoCrush || !House->Is_Human_Player())) {

            /**
             *  Don't allow units to try to crush opportunity fire targets.
             */
            if (!House->IsHuman || !Extension::Fetch(this)->HasOpportunityFireTarget) {
                Assign_Destination(TarCom);
            }
            return;
        }
    }

    /**
     *  In the other cases, uses the more complex "get to just within weapon range"
     *  algorithm.
     */
    FootClass::Approach_Target();
}


/**
 *  #issue-177
 *
 *  Reaplces UnitClass::Do_MISSION_HUNT to consider the entire BuildConst vector.
 *
 *  @author: ZivDero
 */
int UnitClassExt::_Do_MISSION_HUNT()
{
    if (Class->DeploysInto && (Rule->BuildConst.Is_Present(Class->DeploysInto) || TarCom != nullptr || House->Is_Human_Player())) {
        enum {
            FIND_SPOT,
            WAITING
        };

        switch (Status) {

        /**
         *  This stage handles locating a convenient spot, rotating to face the correct
         *  direction and then commencing the deployment operation.
         */
        case FIND_SPOT:
            if (Goto_Clear_Spot()) {
                if (Try_To_Deploy()) {
                    Status = WAITING;
                }
            }
            break;

        /**
         *  This stage watchdogs the deployment operation and if for some reason, the deployment
         *  is aborted (the IsDeploying flag becomes false), then it reverts back to hunting for
         *  a convenient spot to deploy.
         */
        case WAITING:
            if (!IsDeploying) {
                Status = FIND_SPOT;
            }
            break;
        }
    } else {
        return FootClass::Mission_Hunt();
    }
    return Current_Mission_Control().Normal_Delay() + Random_Pick(0, 2);
}


/**
 *  #issue-510
 * 
 *  This patch fixes the bug where the user could action a harvester to 
 *  target tiberium that was 'hidden' below a bridge by forcing any orders
 *  on a bridge with tiberium below it to be move orders.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00656623, _UnitClass_What_Action_ACTION_HARVEST_Block_On_Bridge_Patch, 0)
{
    GET(Cell *, cell, ESI);
    CellClass *cellptr;
    ActionType action;

    /**
     *  Code before here assumes we are a harvester of some kind.
     */
    action = ACTION_HARVEST;

    /**
     *  If the user has the mouse over a cell that contains tiberium, but also
     *  contains a bridge above it, force the MOVE action.
     */
    cellptr = &Map[*cell];
    if (cellptr && cellptr->Is_Bridge_Here()) {
        action = ACTION_MOVE;
    }

    R->EAX(action);
    return 0x006566CC;
}
DEFINE_HOOK_AGAIN(0x0065665D, _UnitClass_What_Action_ACTION_HARVEST_Block_On_Bridge_Patch, 0)


/**
 *  #issue-421
 * 
 *  Implements IdleRate for UnitTypes.
 *
 *  This replaces the WalkFrames branch as we needed to move FiringSyncDelay
 *  before so it has the highest priority over what shape frame to use.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00653114, _UnitClass_Draw_Shape_IdleRate_Patch, 0)
{
    GET(UnitClass *, this_ptr, ESI);
    GET(int, facing, EBX);
    int frame = 0;

    auto unittype = reinterpret_cast<const UnitTypeClass *>(this_ptr->TClass);
    auto unittypeext = Extension::Fetch(unittype);

    if (!this_ptr->Locomotion->Is_Moving()) {
        if (this_ptr->FiringSyncDelay >= 0) {
            frame = this_ptr->FiringSyncDelay/2
                + this_ptr->Class->StartFiringFrame
                + this_ptr->Class->FiringFrames * facing;

            goto continue_to_draw;
        }
    }

    if (!this_ptr->Locomotion->Is_Moving()) {
        if (this_ptr->DeathCounter >= 0) {

            int death_frame = this_ptr->DeathCounter / unittype->DeathFrameRate;
            death_frame = std::min(death_frame, unittype->DeathFrames - 1);
            frame = death_frame + unittype->StartDeathFrame;

            goto continue_to_draw;
        }
    }

    if (this_ptr->Locomotion->Is_Moving()) {
        frame = unittype->StartWalkFrame
            + this_ptr->TotalFramesWalked % unittype->WalkFrames
            + unittype->WalkFrames * facing;

        goto continue_to_draw;
    }

    /**
     *  Unit is not moving, so if the unit has a idle animation rate, use this.
     */
    if (!this_ptr->Locomotion->Is_Moving() && unittypeext->IdleRate > 0) {
        frame = unittypeext->StartIdleFrame
            + this_ptr->TotalFramesWalked % unittypeext->IdleFrames
            + unittypeext->IdleFrames * facing;

        goto continue_to_draw;
    }

    if (this_ptr->field_34D) {
        if (unittype->StandingFrames > 0) {
            frame = unittype->StartStandFrame + facing * unittype->StandingFrames;

        } else {
            frame = unittype->StartWalkFrame + facing * unittype->WalkFrames;
        }

        goto continue_to_draw;
    }

    /**
     *  Continue to the shape drawing.
     */
continue_to_draw:
    R->EBX(frame);
    return 0x006531FB;
}


/**
 *  #issue-264
 * 
 *  Implements LeaveTransportSound for this unit is unloading its passengers.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00654399, _UnitClass_Mission_Unload_Transport_Detach_Sound_Patch, 6)
{
    GET(UnitClass *, this_ptr, ESI);

    /**
     *  Do we have a sound to play when passengers leave us? If so, play it now.
     */
    TechnoTypeClassExtension* radio_technotypeext = Extension::Fetch(this_ptr->TClass);
    if (radio_technotypeext->LeaveTransportSound != VOC_NONE) {
        Static_Sound(radio_technotypeext->LeaveTransportSound, this_ptr->Position);
    }

    return 0;
}


/**
 *  #issue-188
 * 
 *  Adds support for custom (per-type) unloading class when a harvester is unloading at a refinery.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00653D7F, _UnitClass_Draw_It_Unloading_Harvester_Patch, 0)
{
    GET(UnitClass *, this_ptr, ESI);
    GET(UnitTypeClass *, unittype, EAX);

    /**
     *  Are we currently unloading at a refinery?
     */
    if (this_ptr->IsDumping) {

        /**
         *  Is this unit some type of harvester that is unloading?
         * 
         *  The original code only checked for "IsToHarvest".
         */
        if (unittype->IsToHarvest || unittype->IsToVeinHarvest) {
            const UnitTypeClass* unloading_class = nullptr;

            /**
             *  Fetch the default unloading class.
             * 
             *  If this is a weed harvester that is unloading, then they need
             *  a special case to ensure they do not switch unless defined as
             *  they do not have a unloading graphics switch in the original
             *  Tiberian Sun when they enter the facility.
             */
            if (!unittype->IsToVeinHarvest) {
                unloading_class = Rule->UnloadingHarvester;
            }

            /**
             *  Fetch the unloading class from the extended class instance if it exists.
             */
            const UnitTypeClassExtension* unittypeext = Extension::Fetch(unittype);
            if (unittypeext->UnloadingClass) {
                if (unittypeext->UnloadingClass->RTTI == RTTI_UNITTYPE) {
                    unloading_class = reinterpret_cast<const UnitTypeClass *>(unittypeext->UnloadingClass);
                }
            }

            /**
             *  Only switch the graphic if the unloading class is valid.
             */
            if (unloading_class) {
                this_ptr->Class = const_cast<UnitTypeClass *>(unloading_class);
            }
        }
    }

    return 0x00653DA5;
}


/**
 *  Returns the graphic shape number based on the input current facing and desired facing count.
 * 
 *  @author: CCHyper
 */
static int Facing_To_Frame_Number(FacingClass &facing, int facing_count)
{
    int shape_number = 0;

    /**
     *  Fetch the current facing value in the required units.
     */
    switch (facing_count) {

    case 8:
        shape_number = (facing.Current().Get_Facing<8>() + 1) % 8;
        break;

    case 16:
        shape_number = (facing.Current().Get_Facing<16>() + 2) % 16;
        break;

    case 32:
        shape_number = (facing.Current().Get_Facing<32>() + 4) % 32;
        break;

    case 64:
        shape_number = (facing.Current().Get_Facing<64>() + 8) % 64;
        break;

    default:
        shape_number = 0;
        break;
    }

    return shape_number;
}


/**
 *  #issue-#214
 * 
 *  Support for additional facings counts for units with shape graphics.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x006530EB, _UnitClass_Draw_Shape_Primary_Facing_Patch, 0)
{
    GET(UnitClass *, this_ptr, EBP);
    GET(const UnitTypeClass *, unittype, EAX);

    /**
     *  Fetch the frame index for current turret facing.
     */
    int shape_number = Facing_To_Frame_Number(this_ptr->PrimaryFacing, unittype->Facings);

    /**
     *  EBX == desired shape number.
     */
    R->EBX(shape_number);

    return 0x00653114;
}


/**
 *  #issue-#214
 * 
 *  Support for additional facings counts for units with shape graphics.
 * 
 *  This function replaces and reimplements the call to Draw_Object();
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x006537A8, _UnitClass_Draw_Shape_Turret_Facing_Patch, 0)
{
    GET(UnitClass *, this_ptr, EBP);

    const UnitTypeClass* unittype = static_cast<const UnitTypeClass*>(this_ptr->TClass);
    
    /**
     *  All turrets have 32 facings in Tiberian Sun.
     */
    int turret_facings = 32;

    /**
     *  Turret frames start directly after the facing frames.
     */
    int start_turret_frame = unittype->Facings * unittype->WalkFrames;

    const UnitTypeClassExtension* unittypeext = Extension::Fetch(unittype);

    /**
     *  #issue-393
     * 
     *  Allow the custom turret facings.
     * 
     *  @author: CCHyper
     */
    turret_facings = unittypeext->TurretFacings;

    /**
     *  Fetch the frame index for current turret facing.
     */
    int shape_number = Facing_To_Frame_Number(this_ptr->SecondaryFacing, turret_facings);

    /**
     *  Now adjust the frame index based on the units walk frames.
     */

    /**
     *  #issue-389
     * 
     *  Allow the starting turret frame index to be defined.
     * 
     *  @author: CCHyper
     */
    int frame_number = 0;
    if (unittypeext && unittypeext->StartTurretFrame != -1) {
        frame_number = unittypeext->StartTurretFrame + shape_number % turret_facings;
    } else {
        frame_number = start_turret_frame + shape_number % turret_facings;
    }

    /**
     *  The location we jump back to pushes EAX into the stack for
     *  the call to Draw_Object().
     */
    R->EAX(frame_number);

    return 0x006537AE;
}


/**
 *  #issue-334
 * 
 *  Fixes a division by zero crash when Rule->ShakeScreen is zero
 *  and a unit dies/explodes.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0065B554, _UnitClass_Explode_ShakeScreen_Division_BugFix_Patch, 0)
{
    GET(UnitClass *, this_ptr, EDI);

    /**
     *  Fetch the extension instance.
     */
    UnitTypeClassExtension* unittypeext = Extension::Fetch(static_cast<const UnitTypeClass*>(this_ptr->TClass));

    /**
     *  #issue-414
     *
     *  Can this unit shake the screen when it is destroyed?
     *
     *  @author: CCHyper
     */
    if (unittypeext->IsShakeScreen) {

        /**
         *  If this unit has screen shake values defined, then set the blitter
         *  offset values. GScreenClass::Blit will handle the rest for us.
         */
        if (unittypeext->ShakePixelXLo > 0 || unittypeext->ShakePixelXHi > 0 || unittypeext->ShakePixelYLo > 0 || unittypeext->ShakePixelYHi > 0) {

            if (unittypeext->ShakePixelXLo > 0 || unittypeext->ShakePixelXHi > 0) {
                Map.ScreenX = Sim_Random_Pick(unittypeext->ShakePixelXLo, unittypeext->ShakePixelXHi);
            }
            if (unittypeext->ShakePixelYLo > 0 || unittypeext->ShakePixelYHi > 0) {
                Map.ScreenY = Sim_Random_Pick(unittypeext->ShakePixelYLo, unittypeext->ShakePixelYHi);
            }

        } else {

            /**
             *  Very strong units that have an explosion will also rock the
             *  screen when they are destroyed.
             */
            if (this_ptr->Class->MaxStrength > Rule->ShakeScreen) {

                /**
                 *  Make sure both the screen shake factor and the units strength
                 *  are valid before performing the division.
                 */
                if (Rule->ShakeScreen > 0 && this_ptr->Class->MaxStrength > 0) {

                    int shakes = std::min<int>(this_ptr->Class->MaxStrength / (Rule->ShakeScreen / 2), 6);

                    /**
                     *  #issue-414
                     *
                     *  Restores the vertical screen shake when a strong unit is destroyed.
                     *
                     *  @author: CCHyper
                     */
                    Map.ScreenY = shakes;

                    // Shake_The_Screen(shakes);
                }
            }
        }
    }

    /**
     *  Return from the function.
     */
function_return:
    return 0x0065B581;
}


/**
 *  #issue-#6
 * 
 *  A "quality of life" patch for harvesters so they auto harvest
 *  when they have just been kicked out of the war factory.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x006517BE, _UnitClass_Per_Cell_Process_AutoHarvest_Assign_Harvest_Mission_Patch, 0)
{
    GET(UnitClass *, this_ptr, EBP);
    GET(AbstractClass *, target, ESI);
    BuildingClass *building_contact;
    UnitTypeClass *unittype;

    /**
     *  Is the unit we are processing a harvester?
     */
    unittype = (UnitTypeClass *)this_ptr->Class_Of();
    if (unittype->IsToHarvest || unittype->IsToVeinHarvest) {

        /**
         *  Order the unit to harvest.
         */
        this_ptr->Assign_Mission(MISSION_HARVEST);

        goto continue_check_scatter;
    }

    /**
     *  Stolen bytes/code from here on, continues function flow.
     */

    /**
     *  Find out if the target is a building. (flagged to not use dynamic_cast).
     */
continue_function:
    building_contact = Target_As_Building(target, false);

    /**
     *  This is real ugly, but we replace the dynamic_cast in the original
     *  location and we need to return to just after its stack fixup.
     */
    R->ECX(this_ptr->House);
    R->EAX(building_contact);

    return 0x006517DB;

continue_check_scatter:
    return 0x0065194E;
}


/**
 *  Replaces Verses (Modifier) of the Warhead with the one from the extension.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0064F2BE, _UnitClass_Jellyfish_AI_Armor_Patch, 0)
{
    GET(TechnoClass*, target, ESI);
    GET(UnitClass*, this_ptr, EBP);
    GET_STACK(WeaponTypeClass*, weapon, 0x20);
    GET_STACK(WarheadTypeClass*, warhead, 0x14);

    int damage = weapon->Attack * Verses::Get_Modifier(target->TClass->Armor, warhead);
    target->Take_Damage(damage, 0, warhead, this_ptr, false, false);

    return 0x0064F2FA;
}


/**
 *  Helper function.
 *  Creates a unit based on an already existing unit.
 *  Returns the new unit if successful, otherwise null.
 *
 *  @author: Rampastring
 */
UnitClass* Create_Transform_Unit(UnitClass* this_ptr) {

    UnitTypeClassExtension* unittypeext = Extension::Fetch(this_ptr->Class);

    UnitClass* newunit = reinterpret_cast<UnitClass*>(unittypeext->TransformsInto->Create_One_Of(this_ptr->House));
    if (newunit == nullptr) {

        /**
         *  Creating the new unit failed! Re-mark our occupation bits and return false.
         */
        return nullptr;
    }

    // Try_To_Deploy copies the tag this way at 0x0065112C
    if (this_ptr->Tag != nullptr) {
        newunit->Attach_Tag(this_ptr->Tag);
        this_ptr->Tag->AttachCount--;
        this_ptr->Tag = nullptr;
    }

    newunit->ActLike = this_ptr->ActLike;
    newunit->LimpetSpeedFactor = this_ptr->LimpetSpeedFactor;
    newunit->LimpetType = this_ptr->LimpetType; // also copied at 0x00650F4E
    newunit->Crew.From_Integer(this_ptr->Crew.To_Integer());
    newunit->Group = this_ptr->Group;
    newunit->BarrelFacing.Set(this_ptr->BarrelFacing.Current());
    newunit->BarrelFacing.Set_Desired(this_ptr->BarrelFacing.Desired());
    newunit->PrimaryFacing.Set(this_ptr->PrimaryFacing.Current());
    newunit->PrimaryFacing.Set_Desired(this_ptr->PrimaryFacing.Desired());
    newunit->SecondaryFacing.Set(this_ptr->SecondaryFacing.Current());
    newunit->SecondaryFacing.Set_Desired(this_ptr->SecondaryFacing.Desired());
    newunit->Strength = (int)(this_ptr->HealthRatio * (int)newunit->Class->MaxStrength);
    newunit->ArmorBias = this_ptr->ArmorBias;
    newunit->FirepowerBias = this_ptr->FirepowerBias;
    newunit->SpeedBias = this_ptr->SpeedBias;
    newunit->Position = this_ptr->Position;
    newunit->EMPFramesRemaining = this_ptr->EMPFramesRemaining;
    newunit->Ammo = this_ptr->Ammo;


    if (newunit->Unlimbo(newunit->Position, this_ptr->PrimaryFacing.Current().Get_Dir())) {

        /**
         *  Unlimbo successful, select our new unit and return it
         */

        if (PlayerPtr == newunit->Owner_HouseClass()) {
            newunit->Select();
        }

        if (this_ptr->TarCom) {
            newunit->Assign_Target(this_ptr->TarCom);
            newunit->Assign_Mission(MISSION_ATTACK);
            newunit->Commence();
        }

        return newunit;
    }

    /**
     *  Unlimboing the new unit failed! Delete the new unit and return false.
     */
    delete newunit;
    return nullptr;
}


enum TransformReturnValue {
    OriginalCode = 0x00650BC2,
    NotEnoughCharge = 0x006511A0,
    TransformSucceeded = 0x0065114C,
    TransformFailed = 0x00651168
};


/**
 *  #issue-715
 *
 *  Transforms a unit to another unit when a transformable unit deploys.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00650BAE, _UnitClass_Try_To_Deploy_Transform_To_Vehicle_Patch, 0)
{
    GET(UnitClass*, this_ptr, ESI);

    /**
     *  Stolen bytes/code.
     */
    if (this_ptr->Class->DeploysInto != nullptr) {

        /**
         *  This unit is deployable rather than transformable, check whether it can deploy.
         */
        return OriginalCode;
    }

    UnitTypeClassExtension* unittypeext = Extension::Fetch(this_ptr->Class);

    if (unittypeext->TransformsInto != nullptr) {

        /**
         *  Use custom "transform to vehicle" logic if we don't need charge or we have enough of it.
         */

        if (unittypeext->IsTransformRequiresFullCharge && this_ptr->CurrentCharge < this_ptr->Class->MaxCharge) {

            /**
             *  We don't have enough charge, return false
             */
            return NotEnoughCharge;
        }

        this_ptr->Mark(MARK_UP);
        this_ptr->Locomotor_Ptr()->Mark_All_Occupation_Bits(MARK_UP);

        UnitClass* newunit = Create_Transform_Unit(this_ptr);

        if (newunit != nullptr) {

            /**
             *  Creating transformed unit succeeded, erase the original unit and force function to return true
             */
            return TransformSucceeded;
        } else {

            /**
             *  Creating transformed unit failed. Re-mark our occupation bits and return false.
             */
            return TransformFailed;
        }
    }

    /**
     *  Continue to deployability check.
     */
    return OriginalCode;
}


/**
 *  #issue-715
 *
 *  Hack to display the the correct cursor for transformable units
 *  upon ACTION_SELF.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00656017, _UnitClass_What_Action_Self_Check_For_Vehicle_Transform_Patch, 0)
{
    GET(UnitClass*, this_ptr, ESI);

    auto unittype = this_ptr->Class;
    auto unittypeext = Extension::Fetch(unittype);

    /**
     *  Stolen bytes/code.
     *  If the unit can deploy into a building, check whether it's currently allowed.
     */
    if (unittype->DeploysInto != nullptr) {
        return 0x0065602B;
    }

    /**
     *  Check if this unit is able to transform into another unit.
     *  If not, we don't have anything else to do here.
     */
    if (unittypeext->TransformsInto == nullptr) {
        R->EAX(unittype);
        return 0x00656344;
    }

    /**
     *  If this unit is able to transform to a different unit, check if it requires charge for it.
     *  If it does, then check whether we have enough charge.
     */
    ActionType action;
    if (unittypeext->IsTransformRequiresFullCharge && this_ptr->CurrentCharge < this_ptr->Class->MaxCharge) {

        /**
         *  We don't have enough charge!
         */
        action = ACTION_NO_DEPLOY;
    }
    else if (this_ptr->Is_Immobilized()) {

        /**
         *  The unit is dying or under an EMP effect, don't allow it to transform.
         */
        action = ACTION_NO_DEPLOY;
    }
    else {
        action = ACTION_SELF;
    }

    R->EAX(action);
    return 0x0065648F;
}


/**
 *  #issue-715
 *
 *  Check whether the unit is able to transform into another unit
 *  when performing the "Unload" mission.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x006543DB, _UnitClass_Mission_Unload_Transform_To_Vehicle_Patch, 0)
{
    GET(UnitTypeClass*, unittype, EAX);

    /**
     *  Stolen bytes/code.
     */
    if (unittype->IsToHarvest || unittype->IsToVeinHarvest) {
harvester_process:
        return 0x006545A5;
    }

    UnitTypeClassExtension* unittypeext = Extension::Fetch(unittype);
    if (unittype->DeploysInto != nullptr || unittypeext->TransformsInto != nullptr) {
deployable_process:
        return 0x00654403;
    }

mobile_emp_process:
    return 0x00654545;
}


/**
 *  Finds the nearest docking bay for a specific unit.
 *
 *  @author: Rampastring
 */
void UnitClassExtension_Find_Nearest_Refinery(UnitClass* this_ptr, BuildingClass** building_addr, int* distance_addr, bool include_reserved)
{
    int nearest_refinery_distance = INT_MAX;
    BuildingClass* nearest_refinery = nullptr;

    /**
     *  Find_Docking_Bay looks also through occupied docking bays if ScenarioInit is set
     */
    if (include_reserved) {
        ScenarioInit++;
    }

    for (int i = 0; i < this_ptr->Class->Dock.Count(); i++) {
        BuildingTypeClass* dockbuildingtype = this_ptr->Class->Dock[i];

        BuildingClass* dockbuilding = this_ptr->Find_Docking_Bay(dockbuildingtype, false, false);
        if (dockbuilding == nullptr)
            continue;

        int distance = this_ptr->Distance(dockbuilding);

        if (distance < nearest_refinery_distance) {
            nearest_refinery_distance = distance;
            nearest_refinery = dockbuilding;
        }
    }

    if (include_reserved) {
        ScenarioInit--;
    }

    *building_addr = nearest_refinery;
    *distance_addr = nearest_refinery_distance;
}


/**
 *  #issue-201
 *
 *  A "quality of life" patch for harvesters so they don't discriminate against dock
 *  buildings that are not the first on their Dock= list. Also makes harvesters
 *  smarter by making them prefer queuing for nearby occupied refineries instead
 *  of wandering to distant free refineries.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00654EEE, _UnitClass_Mission_Harvest_FINDHOME_Find_Nearest_Refinery_Patch, 0)
{
    /**
     *  Enum for MISSION_HARVEST status constants.
     */
    enum {
        LOOKING,
        HARVESTING,
        FINDHOME,
        HEADINGHOME,
        GOINGTOIDLE,
    };


    GET(UnitClass*, harvester, ESI);
    RadioMessageType response;
    UnitClassExtension* unitext;
    int free_refinery_distance_bias;
    BuildingClass* nearest_free_refinery;
    int nearest_free_refinery_distance;
    BuildingClass* nearest_possibly_occupied_refinery;
    int nearest_possibly_occupied_refinery_distance;
    bool reserve_free_refinery;

    /**
     *  Find the nearest refinery that is not occupied.
     */
    UnitClassExtension_Find_Nearest_Refinery(harvester, &nearest_free_refinery, &nearest_free_refinery_distance, false);

    /**
     *  Find the nearest refinery, regardless of whether it's occupied.
     */
    UnitClassExtension_Find_Nearest_Refinery(harvester, &nearest_possibly_occupied_refinery, &nearest_possibly_occupied_refinery_distance, true);

    reserve_free_refinery = true;

    if (nearest_free_refinery == nullptr) {

        /**
         *  There was no free refinery, check if there was an occupied one.
         */
        if (nearest_possibly_occupied_refinery == nullptr) {

            /**
             *  No refinery existed at all! We have nothing to do here.
             */
            goto set_mission_delay_and_return;
        }

        /**
         *  There was an occupied refinery, queue for it instead.
         */
        reserve_free_refinery = false;
    }
    else if (nearest_free_refinery != nearest_possibly_occupied_refinery) {

        /**
         *  There was a free refinery as well as an occupied one.
         *  Check if the occupied refinery is significantly closer to us than the free refinery.
         */

        free_refinery_distance_bias = RuleExtension->MaxFreeRefineryDistanceBias;

        if (nearest_free_refinery_distance >
            nearest_possibly_occupied_refinery_distance + Cell_To_Lepton(free_refinery_distance_bias)) {

            reserve_free_refinery = false;
        }
    }

    unitext = Extension::Fetch(harvester);

    if (reserve_free_refinery) {

        /**
         *  We want to contact the free refinery, send a radio message to it.
         */
        response = harvester->Transmit_Message(RADIO_HELLO, nearest_free_refinery);

        /**
         *  Check if the refinery answered as expected. If not, we'll queue for it instead.
         */
        if (response == RADIO_ROGER) {

            /**
             *  The refinery accepted us! Change mission status to HEADINGHOME and jump to original code.
             */
            harvester->Status = HEADINGHOME;

            unitext->LastDockedBuilding = nearest_free_refinery;

            goto set_mission_delay_and_return;
        }
    }


    /**
     *  Re-use the original game's code for queueing to an occupied refinery.
     *  The game expects the occupied refinery pointer to be in edi.
     */
queue_to_occupied:

    unitext->LastDockedBuilding = nearest_possibly_occupied_refinery;
    R->EDI(nearest_possibly_occupied_refinery);
    return 0x00654FAA;


    /**
     *  Set mission delay and return from function.
     */
set_mission_delay_and_return:
    return 0x00655226;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0064E0D7, _UnitClass_AI_BuildConst_Patch, 0)
{
    GET(UnitTypeClass*, unittype, EDX);

    if (Rule->BuildConst.Is_Present(unittype->DeploysInto)) {
        return 0x0064E0EC;
    }

    return 0x0064E134;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0065607A, _UnitClass_What_Action_BuildConst, 0)
{
    GET(BuildingTypeClass*, buildingtype, EBP);

    if (Rule->BuildConst.Is_Present(buildingtype)) {
        return 0x00656084;
    }

    return 0x006560A3;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00656751, _UnitClass_Mission_Guard_BuildConst, 0)
{
    GET(UnitClass*, unit, ESI);

    if (Rule->BuildConst.Is_Present(unit->Class->DeploysInto)) {
        return 0x00656770;
    }

    return 0x006567FD;
}

/**
 *  Prevents deploying hijacked units that have a build limit.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x0065601D, _UnitClass_What_Action_ACTION_SELF_Prevent_Deploying_Hijacked_Build_Limited_Vehicles, 0)
{
    GET(UnitClass*, this_ptr, ESI);
    GET(UnitTypeClass*, unittype, EAX);

    // Stolen bytes / code.
    if (unittype->DeploysInto == nullptr) {
        return 0x00656344;
    }

    // Do not allow deploying if this unit has been hijacked and it would deploy into a build-limited unit.
    if (unittype->BuildLimit < INT_MAX && this_ptr->EnteredByInfType != INFANTRY_NONE) {
        R->Stack(0x28, ACTION_NO_DEPLOY);
        return 0x0065618E;
    }

    // Continue deployability checks.
    return 0x0065602B;
}


/**
 *  Patches UnitClass::Take_Damage right before iterating on the cargo.
 *  Fixes an issue where units that have cargo (such as APCs) do not spawn their cargo if they are destroyed
 *  while moving from one cell to another, resulting in the cargo getting erased instead.
 * 
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0064FDB4, _Unit_Class_Take_Damage_Cargo_Hold_Patch, 6)
{
    GET(UnitClass*, this_ptr, ESI);

    if (this_ptr->Cargo.Is_Something_Attached()) {
        this_ptr->Stop_Driver();
        if (this_ptr->Locomotion) {
            this_ptr->Locomotion->Mark_All_Occupation_Bits(MARK_UP);
        }
    }
    
    return 0;
}


/**
 *  Patches UnitClass::Take_Damage not to alert of harvesters receiving damage
 *  if the player was just recently told about it, or the damage was dealt
 *  by a passive damage source.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x0065019F, _UnitClass_Take_Damage_Throttle_Harvester_Under_Attack_Patch, 6)
{
    GET(ObjectClass*, source, EBX);

    /**
     *  Damage without an owner most likely means incidental damage rather than "we're under attack".
     */
    if (source == nullptr) {
        return 0x006501F3;
    }

    /**
     *  If EVA has just recently alerted about a harvester being under attack, there might be no need to repeat it yet.
     */
    if (RuleExtension->HarvesterUnderAttackThrottleTime > 0 &&
        Frame < TacticalMapExtension->LastHarvesterUnderAttackFrame + Options.Normalize_Delay(RuleExtension->HarvesterUnderAttackThrottleTime))
    {
        return 0x006501F3;
    }

    TacticalMapExtension->LastHarvesterUnderAttackFrame = Frame;
    return 0;
}


/**
 *  Patches UnitClass::What_Action at the part where MRVs (AKA units with negative combat damage)
 *  are evaluated whether they should commit to their action or switch to ACTION_SELECT.
 *
 *  Fixes an issue where ACTION_TOGGLE_SELECT (selecting units while shift is held) was converted into ACTION_SELECT,
 *  causing the game to instead unselect the currently selected units if the MRV was the "best object" in the current selection.
 *  For MRVs, this only triggers while trying to add infantry to your selection.
 *
 *  Causes What_Action to return with the ACTION_TOGGLE_SELECT action if this is the current mission.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x006563FD, _UnitClass_What_Action_MRV_Toggle_Select_Patch, 7) 
{
    GET_STACK(ActionType, action, 0x28);

    if (action == ACTION_TOGGLE_SELECT) {
        return 0x0065648B;
    }

	return 0;
}


TagClass* Find_Or_Make_Tag(TagTypeClass* type)
{
    for (int index = 0; index < Tags.Count(); index++) {
        TagClass* tag = Tags[index];
        if (tag->Class == type) {
            return (tag);
        }
    }

    return (new TagClass(type));
}

void Decrement_Followers(DynamicVectorClass<int> &followers, int index)
{
    for (int fid = 0; fid < followers.Count(); fid++) {
        if (followers[fid] >= index) {
            followers[fid] = followers[fid] - 1;
        }
    }
}

/**
 *  Replacement for UnitClass::Read_INI for the multiplayer spawner.
 *
 *  @author: Rampastring
 */
void UnitClassExt::_Read_INI(CCINIClass& ini)
{
    UnitClass* unit;    // Working unit pointer.
    HousesType inhouse; // Unit house.
    UnitType classid;   // Unit class.
    char buf[128];
    DynamicVectorClass<int> followers;

    char const* const INI_NAME = "Units";

    int len = ini.Entry_Count(INI_NAME);

    for (int index = 0; index < len; index++) {
        char const* entry = ini.Get_Entry(INI_NAME, index);

        ini.Get_String(INI_NAME, entry, nullptr, buf, sizeof(buf));

        char* housename = strtok(buf, ",");
        inhouse = HouseTypeClassExtension::House_From_Name(housename);
        HouseClass* inhousep = House_From_HousesType(inhouse);

        if (inhousep == nullptr) {
            if (Session.Type == GAME_NORMAL || inhouse < EXT_HOUSE_SPAWN1) {
                Vinifera_Log_And_Show_WWMessageBox("Unable to find house %s while reading units!", housename);
                Decrement_Followers(followers, index);
                continue;
            } else {

                DEBUG_INFO("Ignoring unit placed for {} because the house is not present\n", housename);

                // This unit's owner is likely a Spawn house that is not present.
                // Go through the followers and for each one with ID above the current unit ID, decrement it by one.
                Decrement_Followers(followers, index);
                continue;
            }
        }

        char* unittypename = strtok(nullptr, ",");
        classid = UnitTypeClass::From_Name(unittypename);

        if (classid == UNIT_NONE) {
            Vinifera_Log_And_Show_WWMessageBox("Unable to find UnitType %s while reading units!", unittypename);
            Decrement_Followers(followers, index);
            continue;
        }

        unit = new UnitClass(UnitTypes[classid], inhousep);
        if (unit != nullptr) {

            /*
            **	Read the raw data.
            */
            int strength = atoi(strtok(nullptr, ","));

            Cell cell;
            Coord coord;
            if (NewINIFormat >= 4) {
                unsigned short x = atoi(strtok(nullptr, ","));
                unsigned short y = atoi(strtok(nullptr, ","));
                cell = Cell(x, y);
            } else {
                int c = atoi(strtok(nullptr, ","));
                cell = Cell(c % 128, c / 128);
            }

            coord = cell.As_Coord();

            Dir256 dir = (Dir256)atoi(strtok(nullptr, ","));
            MissionType mission = MissionClass::Mission_From_Name(strtok(nullptr, ","));

            TagType tagtype = TagTypeClass::From_Name(strtok(nullptr, ","));
            if (tagtype != TAG_NONE) {
                TagTypeClass* tp = TagTypes[tagtype];
                if (tp != nullptr) {
                    TagClass* tt = Find_Or_Make_Tag(tp);
                    if (tt != nullptr) {
                        unit->Attach_Tag(tt);
                    }
                }
            }

            char* token = strtok(nullptr, ",");
            if (token != nullptr) {
                unit->Crew.From_Integer(atoi(token));
            }

            token = strtok(nullptr, ",");
            if (token != nullptr) {
                unit->Group = atoi(token);
            }

            token = strtok(nullptr, ",");
            if (token != nullptr) {
                unit->IsOnBridge = atoi(token) != 0;
                if (unit->IsOnBridge) {
                    coord.Z = Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT;
                }
            }

            token = strtok(nullptr, ",");
            if (token != nullptr) {
                followers.Add(atoi(token));
            }

            token = strtok(nullptr, ",");
            if (token != nullptr) {
                unit->field_205 = atoi(token) != 0;
            }

            token = strtok(nullptr, ",");
            if (token != nullptr) {
                unit->field_206 = atoi(token) != 0;
            }

            if (unit->Unlimbo(coord, dir)) {
                unit->Strength = unit->Class->MaxStrength * (double)strength / 256.0;
                if (unit->Strength > unit->Class->MaxStrength - 3) unit->Strength = unit->Class->MaxStrength;
                if (unit->Strength == 0) unit->Strength = 1;
                if (Session.Type == GAME_NORMAL || unit->House->Is_Human_Player()) {
                    unit->Assign_Mission(mission);
                    if (unit->Ready_To_Commence()) {
                        unit->Commence();
                    }
                } else {
                    unit->Enter_Idle_Mode();
                }

            } else {

                /*
                **	If the unit could not be unlimboed, then this is a catastrophic error
                **	condition. Delete the unit.
                */
                Vinifera_Log_And_Show_WWMessageBox("Failed to unlimbo unit %s at %d,%d while reading units!", unit->Class->IniName.c_str(), cell.X, cell.Y);
                delete unit;
            }
        }
    }

    for (int i = 0; i < Units.Count(); i++) {
        unsigned followerid = followers[i];
        UnitClass* unit = Units[i];
        if ((UnitType)followerid != UNIT_NONE && followerid < Units.Count()) {
            UnitClass* follower = Units[followerid];
            unit->FollowingMe = follower;
            follower->IsFollowing = true;
        } else {
            unit->FollowingMe = nullptr;
        }
    }
}


/**
 *  Patches UnitClass::Try_To_Deploy at the very end of the process, after the new building to be deployed into has been created.
 *  Typically, the unit is first stunned, which removes all associations it has with the game. If the unit belongs to an AI house,
 *  then this includes the unit's tag; otherwise, the tag persists. Then, the unit is checked for a tag to be assigned to the new building.
 *  This behavior causes AI units to lose the tag before they can attach it to the building.
 * 
 *  This swaps the order of operations: first, check the tag and assign it to the new building, and only then stun the unit,
 *  removing the tag from it. Once done, jump to the next statement.
 * 
 *  Only applies if the new PersistTagsOnAIDeploy key is set, for backwards compatibility.
 * 
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x00651122, Unit_Class_Try_To_Deploy_AI_Persist_Tag_Patch, 10)
{
    GET(UnitClass*, this_ptr, ESI);
    GET(ObjectClass*, new_building, EDI);

    if (!RuleExtension->PersistTagsOnAIDeploy) {
        return 0;
    }

    if (this_ptr->Tag != nullptr) {
        new_building->Attach_Tag(this_ptr->Tag);
        this_ptr->Tag->AttachCount--;
        this_ptr->Tag = nullptr;
    }

    this_ptr->Stun();

    return 0x0065114C;
}


/**
 *  Main function for patching the hooks.
 */
void UnitClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    UnitClassExtension_Init();

    Patch_Jump(0x0064E920, &UnitClassExt::_Firing_AI);
    Patch_Jump(0x00655270, &UnitClassExt::_Do_MISSION_HUNT);
    Patch_Jump(0x0064E560, &UnitClassExt::_Rotation_AI);
    Patch_Jump(0x006571E0, &UnitClassExt::_Approach_Target);
    Patch_Jump(0x006585C0, &UnitClassExt::_Read_INI);

    Patch_Byte(0x00658961, 0xEB); // Allow pre-placed units to have missions in multiplayer, change JZ to JMP
    /*
    *  Patches the MISSION_HARVEST logic that handles harvesters becoming idle due to being unable to find tiberium in its area.
    *  It does this by removing the MISSION_GUARD assignment inside the function and instead sets it to MISSION_GUARD_AREA,
    *  which naturally seeks out tiberium to harvest.
    */  
    Patch_Byte(0x0065521C + 1, (unsigned char)MISSION_GUARD_AREA); 
}
