/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TechnoClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "technoext.h"

#include "anim.h"
#include "buildingtype.h"
#include "debughandler.h"
#include "ebolt.h"
#include "extension.h"
#include "extension_globals.h"
#include "house.h"
#include "houseext.h"
#include "rules.h"
#include "rulesext.h"
#include "saveload.h"
#include "spawnmanager.h"
#include "storageext.h"
#include "tactical.h"
#include "team.h"
#include "teamtype.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "unit.h"
#include "vinifera_saveload.h"
#include "voc.h"
#include "wwcrc.h"

#include <algorithm>


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
TechnoClassExtension::TechnoClassExtension(const TechnoClass *this_ptr) :
    RadioClassExtension(this_ptr),
    Vinifera::Detach::Listener<TechnoClass>(),
    Vinifera::Detach::Listener<AnimClass>(),
    ElectricBolt(nullptr),
    Storage(Tiberiums.Count()),
    SpawnManager(nullptr),
    SpawnOwner(nullptr),
    HasOpportunityFireTarget(false),
    LastTargetFrame(Frame),
    IsToResetBurst(false),
    BurstResetTimer(),
    LastVeterancy(RANK_NONE),
    IdleWakeAnim(nullptr),
    IronCurtainTimer()
{
    for (int i = 0; i < Tiberiums.Count(); i++)
    {
        Storage[i] = 0;
    }

    if (this_ptr)
    {
        new ((StorageClassExt*)&(this_ptr->Storage)) StorageClassExt(&Storage);

        const auto ttypeext = Extension::Fetch(this_ptr->TClass);
        if (ttypeext->Spawns)
            SpawnManager = new SpawnManagerClass(const_cast<TechnoClass*>(this_ptr), ttypeext->Spawns, ttypeext->SpawnsNumber, ttypeext->SpawnRegenRate, ttypeext->SpawnReloadRate, ttypeext->SpawnSpawnRate, ttypeext->SpawnLogicRate);
    }
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TechnoClassExtension::TechnoClassExtension(const NoInitClass &noinit) :
    RadioClassExtension(noinit),
    Vinifera::Detach::Listener<TechnoClass>(noinit),
    Vinifera::Detach::Listener<AnimClass>(noinit),
    Storage(noinit),
    BurstResetTimer(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TechnoClassExtension::~TechnoClassExtension()
{
    if (ElectricBolt) {
        delete ElectricBolt;
        ElectricBolt = nullptr;
    }

    if (SpawnManager) {
        delete SpawnManager;
        SpawnManager = nullptr;
    }

    if (IdleWakeAnim) {
        delete IdleWakeAnim;
        IdleWakeAnim = nullptr;
    }
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoClassExtension::Load(IStream *pStm)
{
    HRESULT hr = RadioClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    Load_Primitive_Vector(pStm, Storage);

    ElectricBolt = nullptr;

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(SpawnManager, "SpawnManager");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(SpawnOwner, "SpawnOwner");

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(IdleWakeAnim, "IdleWakeAnim");
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = RadioClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    Save_Primitive_Vector(pStm, Storage);

    return hr;
}


/**
 *  Clears SpawnOwner if it pointed at the destroyed techno.
 *  SpawnManager (when present) is itself an Abstract listener and handles
 *  its own pointer cleanup via the registry.
 */
void TechnoClassExtension::On_Detach(TechnoClass *target, bool all)
{
    if (target == SpawnOwner) {
        SpawnOwner = nullptr;
    }
}


/**
 *  Clears IdleWakeAnim if it pointed at the destroyed anim.
 */
void TechnoClassExtension::On_Detach(AnimClass *target, bool all)
{
    if (target == IdleWakeAnim) {
        IdleWakeAnim = nullptr;
    }
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TechnoClassExtension::Object_CRC(CRCEngine &crc) const
{
    RadioClassExtension::Object_CRC(crc);

    if (SpawnOwner) {
        crc(SpawnOwner->Fetch_Heap_ID());
    }
}


/**
 *  Creates a electric bolt zap from the firing techno to the target.
 * 
 *  @author: CCHyper
 */
EBoltClass * TechnoClassExtension::Electric_Zap(AbstractClass * target, int which, const WeaponTypeClass *weapontype, Coord &source_coord)
{
    EBoltClass *ebolt = new EBoltClass;
    if (!ebolt) {
        return nullptr;
    }

    int z_adj = 0;

    if (Is_Target_Building(target)) {
        Coord source = This()->Render_Coord();

        Point2D p1 = TacticalMap->func_60F150(source);
        Point2D p2 = TacticalMap->func_60F150(source_coord);

        z_adj = p2.Y - p1.Y;
        z_adj = std::min(z_adj, 0);
    }

    Coord target_coord = Is_Target_Object(target) ?
        reinterpret_cast<ObjectClass *>(target)->Target_Coord() : target->entry_5C();

    /**
     *  Spawn the electric bolt.
     */
    ebolt->Create(source_coord, target_coord, z_adj);

    return ebolt;
}


/**
 *  Creates an instance of the electric bolt from the firing techno to the target.
 * 
 *  @author: CCHyper
 */
EBoltClass * TechnoClassExtension::Electric_Bolt(AbstractClass * target)
{
    WeaponSlotType which = This()->What_Weapon_Should_I_Use(target);
    const WeaponTypeClass *weapontype = This()->Get_Weapon(which)->Weapon;
    Coord fire_coord = This()->Fire_Coord(which);

    EBoltClass *ebolt = Electric_Zap(target, which, weapontype, fire_coord);
    if (ebolt) {
        if (This()->IsActive) {
            /**
             *  Remove existing electric bolt from the object.
             */
            if (ElectricBolt) {
                ElectricBolt->Flag_To_Delete();
                ElectricBolt = nullptr;
            }

            if (!ElectricBolt) {
                ElectricBolt = ebolt;
                ElectricBolt->Set_Properties(This(), weapontype, which);
            }
        }
    }

    return ebolt;
}


/**
 *  Handles the voice response when given capture order.
 * 
 *  @author: CCHyper
 */
void TechnoClassExtension::Response_Capture()
{
    if (!AllowVoice) {
        return;
    }

    //if (!This()->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    const TechnoTypeClass *technotype = Techno_Type_Class();
    const TechnoTypeClassExtension *technotypeext = Techno_Type_Class_Ext();
    if (technotypeext->VoiceCapture.Count() > 0) {

        response = technotypeext->VoiceCapture[Sim_Random_Pick(0, technotypeext->VoiceCapture.Count()-1)];

    } else if (technotype->VoiceMove.Count() > 0) {
        
        response = technotype->VoiceMove[Sim_Random_Pick(0, technotype->VoiceMove.Count()-1)];
    
    }

    Sound_Effect(response);
}


/**
 *  Handles the voice response when given enter order.
 * 
 *  @author: CCHyper
 */
void TechnoClassExtension::Response_Enter()
{
    if (!AllowVoice) {
        return;
    }

    //if (!This()->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    const TechnoTypeClass *technotype = Techno_Type_Class();
    const TechnoTypeClassExtension *technotypeext = Techno_Type_Class_Ext();
    if (technotypeext->VoiceEnter.Count() > 0) {

        response = technotypeext->VoiceEnter[Sim_Random_Pick(0, technotypeext->VoiceEnter.Count()-1)];

    } else if (technotype->VoiceMove.Count() > 0) {
        
        response = technotype->VoiceMove[Sim_Random_Pick(0, technotype->VoiceMove.Count()-1)];
    
    }

    Sound_Effect(response);
}


/**
 *  Handles the voice response when given deploy order.
 * 
 *  @author: CCHyper
 */
void TechnoClassExtension::Response_Deploy()
{
    if (!AllowVoice) {
        return;
    }

    //if (!This()->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    const TechnoTypeClass *technotype = Techno_Type_Class();
    const TechnoTypeClassExtension *technotypeext = Techno_Type_Class_Ext();
    if (technotypeext->VoiceDeploy.Count() > 0) {

        response = technotypeext->VoiceDeploy[Sim_Random_Pick(0, technotypeext->VoiceDeploy.Count()-1)];

    } else if (technotype->VoiceMove.Count() > 0) {
        
        response = technotype->VoiceMove[Sim_Random_Pick(0, technotype->VoiceMove.Count()-1)];
    
    }

    Sound_Effect(response);
}


/**
 *  Handles the voice response when given harvest order.
 * 
 *  @author: CCHyper
 */
void TechnoClassExtension::Response_Harvest()
{
    if (!AllowVoice) {
        return;
    }

    //if (!This()->House->Is_Player_Control()) {
    //    return;
    //}

    VocType response = VOC_NONE;

    const TechnoTypeClass *technotype = Techno_Type_Class();
    const TechnoTypeClassExtension *technotypeext = Techno_Type_Class_Ext();
    if (technotypeext->VoiceHarvest.Count() > 0) {

        response = technotypeext->VoiceHarvest[Sim_Random_Pick(0, technotypeext->VoiceHarvest.Count()-1)];

    } else if (technotype->VoiceMove.Count() > 0) {
        
        response = technotype->VoiceMove[Sim_Random_Pick(0, technotype->VoiceMove.Count()-1)];
    
    }

    Sound_Effect(response);
}


/**
 *  Returns if this object can acquire targets that are within range and attack them automatically.
 * 
 *  @author: CCHyper
 */
bool TechnoClassExtension::Can_Passive_Acquire() const
{
    if ((!This()->Is_Renovator() || !This()->House->Is_Human_Player()) && This()->Is_Weapon_Equipped()) {
        /**
         *  IsCanPassiveAcquire defaults to true to copy original behaviour, so all units can passive acquire unless told otherwise.
         */
        return Techno_Type_Class_Ext()->IsCanPassiveAcquire;
    }

    return false;
}


/**
 *  Returns the sight range of this techno after calculations.
 *  Takes into account veterancy bonuses as well height bonuses, if any.
 *
 *  @author: JoyfulShush
 */
int TechnoClassExtension::Get_Sight_Range() const
{
    auto techno_class_ext = Techno_Type_Class_Ext();

    int sight_range = This()->TClass->SightRange;
    if (This()->Crew.IsElite) {
        if (techno_class_ext->EliteSightRange > 0) {
            sight_range = techno_class_ext->EliteSightRange;
        } else if (techno_class_ext->VeteranSightRange > 0) {
            sight_range = techno_class_ext->VeteranSightRange;
        }
    } else if (This()->Crew.IsVeteran) {
        if (techno_class_ext->VeteranSightRange > 0) {
            sight_range = techno_class_ext->VeteranSightRange;
        }
    }

    sight_range *= (This()->SightIncrease * 0.01 + 1.0);
    if (This()->Has_Ability(ABILITY_SIGHT) && Rule->VeteranSight != 0.0) {
        sight_range *= Rule->VeteranSight + 1;
    }

    return sight_range;
}


/**
 *  Determines the time it would take to build this object.
 * 
 *  @author: CCHyper, ZivDero
 */
int TechnoClassExtension::Time_To_Build() const
{
    const TechnoTypeClassExtension* technotypeext = Techno_Type_Class_Ext();

    int time = Techno_Type_Class()->Time_To_Build();

    /**
     *  Adjust the time based on the house's build speed bonus.
     */
    time *= This()->House->BuildSpeedBias;

    /**
     *  #issue-657
     * 
     *  Implements BuildTimeMultiplier for TechnoTypes.
     * 
     *  @author: CCHyper
     */
    time *= technotypeext->BuildTimeMultiplier;

    /**
     *  Adjust the time to build based on the power output of the owning house.
     */
    double power = This()->House->Power_Fraction();

    /**
     *  #issue-656
     * 
     *  Implements LowPowerPenaltyModifier for RulesClass.
     * 
     *  @author: CCHyper
     */
    double scale = 1.0f - (1.0f - power) * RuleExtension->LowPowerPenaltyModifier;

    /**
     *  #issue-658
     *
     *  Restores the affect of "WorstLowPowerBuildRateCoefficient".
     *
     *  @author: CCHyper
     */
    if (scale <= Rule->WorstLowPowerBuildRateCoefficient) scale = Rule->WorstLowPowerBuildRateCoefficient;

    /**
     *  #issue-658
     *
     *  Restores the affect of "BestLowPowerBuildRateCoefficient".
     *
     *  @author: CCHyper
     */
    if (power < 1.0 && scale >= Rule->BestLowPowerBuildRateCoefficient) scale = Rule->BestLowPowerBuildRateCoefficient; // Was "0.75"

    /**
     *  Ensure we don't end up doing division by zero.
     */
    if (scale == 0.0) scale = 0.01;

    scale = std::max(scale, Rule->MinProductionSpeed);

    time /= scale;

    /**
     *  Calculate the bonus based on the current factory count.
     */
    int divisor = Extension::Fetch(This()->House)->Factory_Count(This()->RTTI, TechnoTypeClassExtension::Get_Production_Flags(This())) - 1;

    /**
     *  #issue-106
     * 
     *  "MultipleFactory" calculation back ported from Red Alert 2.
     * 
     *  @author: CCHyper
     */
    if (Rule->MultipleFactory > 0.0 && divisor > 0) {

        /**
         *  #issue-659
         * 
         *  Implements MultipleFactoryCap for RulesClass.
         * 
         *  @author: CCHyper
         */
        if (RuleExtension->MultipleFactoryCap > 0) {
            divisor = RuleExtension->MultipleFactoryCap - 1;
        }

        while (divisor) {
            time *= Rule->MultipleFactory;
            divisor--;
        }
    }

    /**
     *  Walls have a coefficient as they are really cheap.
     */
    if (This()->RTTI == RTTI_BUILDING && reinterpret_cast<const BuildingTypeClass *>(This()->TClass)->IsWall) {
        time *= Rule->WallBuildSpeedCoefficient;
    }

    return time;
}


/**
 *  Can this unit opportunity fire?
 *
 *  @author: ZivDero
 */
bool TechnoClassExtension::Can_Opportunity_Fire() const
{
    if (This()->TarCom != nullptr && !This()->House->Is_Human_Player() && This()->Is_Foot()) {
        FootClass* foot = static_cast<FootClass*>(This());
        if (foot->Team != nullptr && !foot->Team->Class->IsSuicide && foot->Team->Class->IsAggressive && foot->CurrentMission == MISSION_MOVE) {
            return true;
        }
    }

    if (!Can_Passive_Acquire()) {
        return false;
    }

    if (Techno_Type_Class_Ext()->IsOpportunityFire) {
        return true;
    }

    return false;
}


/**
 *  Perform opportunity fire.
 *
 *  @author: ZivDero
 */
bool TechnoClassExtension::Opportunity_Fire()
{
    if (Can_Opportunity_Fire() && (This()->TarCom == nullptr || HasOpportunityFireTarget)) {
        AbstractClass* old_target = This()->TarCom;
        bool result = This()->Target_Something_Nearby(This()->Center_Coord(), THREAT_RANGE);
        if (result && This()->TarCom != old_target) {
            HasOpportunityFireTarget = true;
        }
        return result;
    }

    return false;
}


/**
 *  Determines the coordinate where bullets appear.
 *  Contains an additional argument to add an offset to the firing coordinate,
 *  used by the spawn manager.
 *
 *  @author: ZivDero
 */
Coord TechnoClassExtension::Fire_Coord(WeaponSlotType which, TPoint3D<int> offset) const
{
    const TechnoTypeClass *ttype = This()->TClass;
    const auto weaponinfo = This()->Get_Weapon(which);

    Matrix3D matrix;
    matrix.Make_Identity();

    float theta = This()->Turret_Facing().Get_Radian<32>();
    matrix.Rotate_Z(theta);

    const TPoint3D<int> flh = weaponinfo->FireFLH + offset;

    const float trans_x = static_cast<float>(flh.X + ttype->TurretOffset);
    const float trans_y = static_cast<float>(flh.Y * (This()->BurstIndex % 2 == 0 ? 1 : -1));
    const float trans_z = static_cast<float>(flh.Z + weaponinfo->BarrelThickness);
    matrix.Translate(trans_x, trans_y, trans_z);

    theta = -This()->BarrelFacing.Current().Get_Radian<32>();
    matrix.Rotate_Y(theta);

    matrix.Translate(static_cast<float>(weaponinfo->BarrelLength), 0, 0);

    const Vector3 fire_coord = matrix * Vector3(0, 0, 0);
    Coord render_coord = This()->Render_Coord();

    return { render_coord.X + static_cast<int>(fire_coord.X), render_coord.Y - static_cast<int>(fire_coord.Y), render_coord.Z + static_cast<int>(fire_coord.Z) };
}


/**
 *  Applies Iron Curtain to the unit. Can optionally skip legality checks.
 *
 *  @author: Rampastring
 */
bool TechnoClassExtension::Iron_Curtain_Me(bool forced)
{
    if (!forced) {
        HouseClassExtension* houseext = Extension::Fetch(This()->House);

        if (!houseext->Can_Use_Iron_Curtain()) {
            return false;
        }
    }

    IronCurtainTimer = RuleExtension->IronCurtainDuration;
    Static_Sound(RuleExtension->IronCurtainSound, This()->Center_Coord());
    return true;
}


/**
 *  Puts pointers to the storage extension into the storage class.
 *
 *  @author: ZivDero
 */
void TechnoClassExtension::Put_Storage_Pointers()
{
    new (reinterpret_cast<StorageClassExt*>(&This()->Storage)) StorageClassExt(&Storage);
}


/**
 *  Provides access to the TechnoTypeClass instance for this extension. 
 * 
 *  @author: CCHyper
 */
const TechnoTypeClass *TechnoClassExtension::Techno_Type_Class() const
{
    return reinterpret_cast<TechnoClass *>(This())->TClass;
}


/**
 *  Provides access to the TechnoTypeClass extension instance for this extension.
 *
 *  @author: CCHyper
 */
const TechnoTypeClassExtension *TechnoClassExtension::Techno_Type_Class_Ext() const
{
    return Extension::Fetch(Techno_Type_Class());
}
