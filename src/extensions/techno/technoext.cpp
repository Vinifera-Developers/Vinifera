/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TechnoClass class.
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
#include "technoext.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "house.h"
#include "housetype.h"
#include "building.h"
#include "buildingtype.h"
#include "rules.h"
#include "voc.h"
#include "ebolt.h"
#include "tactical.h"
#include "tibsun_inline.h"
#include "tibsun_globals.h"
#include "wwcrc.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "saveload.h"
#include "vinifera_saveload.h"
#include "storageext.h"
#include "spawnmanager.h"
#include "weapontype.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TechnoClassExtension::TechnoClassExtension(const TechnoClass *this_ptr) :
    RadioClassExtension(this_ptr),
    ElectricBolt(nullptr),
    Storage(Tiberiums.Count()),
    SpawnManager(nullptr),
    SpawnOwner(nullptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("TechnoClassExtension::TechnoClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    for (int i = 0; i < Tiberiums.Count(); i++)
    {
        Storage[i] = 0;
    }

    if (this_ptr)
    {
        new ((StorageClassExt*)&(this_ptr->Storage)) StorageClassExt(&Storage);

        const auto ttypeext = Extension::Fetch(this_ptr->Techno_Type_Class());
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
    Storage(noinit)
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::TechnoClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TechnoClassExtension::~TechnoClassExtension()
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::~TechnoClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (ElectricBolt)
    {
        delete ElectricBolt;
        ElectricBolt = nullptr;
    }

    if (SpawnManager)
    {
        delete SpawnManager;
        SpawnManager = nullptr;
    }
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = RadioClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    Load_Primitive_Vector(pStm, Storage, "Storage");

    ElectricBolt = nullptr;

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(SpawnManager, "SpawnManager");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(SpawnOwner, "SpawnOwner");
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = RadioClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    Save_Primitive_Vector(pStm, Storage, "Storage");

    return hr;
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TechnoClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    RadioClassExtension::Detach(target, all);

    if (SpawnManager) {
        SpawnManager->Detach(target);
    }

    if (target == SpawnOwner) {
        SpawnOwner = nullptr;
    }
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TechnoClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    RadioClassExtension::Object_CRC(crc);

    if (SpawnOwner)
        crc(SpawnOwner->Fetch_Heap_ID());
}


/**
 *  Creates a electric bolt zap from the firing techno to the target.
 * 
 *  @author: CCHyper
 */
EBoltClass * TechnoClassExtension::Electric_Zap(AbstractClass * target, int which, const WeaponTypeClass *weapontype, Coordinate &source_coord)
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Electric_Zap - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    EBoltClass *ebolt = new EBoltClass;
    if (!ebolt) {
        return nullptr;
    }

    int z_adj = 0;

    if (Is_Target_Building(target)) {
        Coordinate source = This()->Render_Coord();

        Point2D p1 = TacticalMap->func_60F150(source);
        Point2D p2 = TacticalMap->func_60F150(source_coord);

        z_adj = p2.Y - p1.Y;

        if (z_adj > 0) {
            z_adj = 0;
        }
    }

    Coordinate target_coord = Is_Target_Object(target) ?
        reinterpret_cast<ObjectClass *>(target)->Target_Coord() : // #TODO: Should be Target_As_Object.
        target->entry_5C();

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
    //EXT_DEBUG_TRACE("TechnoClassExtension::Electric_Bolt - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    WeaponSlotType which = This()->What_Weapon_Should_I_Use(target);
    const WeaponTypeClass *weapontype = This()->Get_Weapon(which)->Weapon;
    Coordinate fire_coord = This()->Fire_Coord(which);

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
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Capture - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Enter - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Deploy - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("TechnoClassExtension::Response_Harvest - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("TechnoClassExtension::Can_Passive_Acquire - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    /**
     *  IsCanPassiveAcquire defaults to true to copy original behaviour, so all units can passive acquire unless told otherwise.
     */
    return Techno_Type_Class_Ext()->IsCanPassiveAcquire;
}



/**
 *  Determines the time it would take to build this object.
 * 
 *  @author: CCHyper
 */
int TechnoClassExtension::Time_To_Build() const
{
    //EXT_DEBUG_TRACE("TechnoClassExtension::Time_To_Build - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TechnoTypeClassExtension* technotypeext = Extension::Fetch< TechnoTypeClassExtension>(Techno_Type_Class());

    int time = Techno_Type_Class()->Time_To_Build();

    /**
     *  Adjust the time based on the houses build speed bonus.
     */
    time *= This()->House->BuildSpeedBias;

    /**
     *  Adjust the time to build based on the power output of the owning house.
     */
    double power = This()->House->Power_Fraction();
    double scale = 1.0f;
    if (power > 1.0) {
        scale = 1.0;

    } else if (power < 1.0 && power > 0.75) {
        scale = 0.75;

    } else if (power < 0.5) {
        scale = 0.5;

    }
    if (power <= Rule->MinProductionSpeed) {
        scale = Rule->MinProductionSpeed;
    }
    time /= scale;

    /**
     *  Calculate the bonus based on the current factory count.
     */
    int divisor = This()->House->Factory_Count(This()->Kind_Of());
#if 0
    /**
     *  Original code for "MultipleFactory".
     */
    if (Rule->MultipleFactory > 0.0 && divisor > 1) {
        time = (double)(1.0 / ((double)(divisor-1) * Rule->MultipleFactory) * (double)time);
    }
#else
    /**
     *  #issue-106
     * 
     *  "MultipleFactory" calculation back ported from Red Alert 2.
     * 
     *  @author: CCHyper
     */
    if (Rule->MultipleFactory > 0.0 && (divisor-1) > 0) {
        while (divisor) {
            time *= Rule->MultipleFactory;
            --divisor;
        }
    }
#endif

    /**
     *  Walls have a coefficient as they are really cheap.
     */
    if (This()->What_Am_I() == RTTI_BUILDING && reinterpret_cast<const BuildingTypeClass *>(This()->Techno_Type_Class())->IsWall) {
        time *= Rule->WallBuildSpeedCoefficient;
    }

    return time;
}


/**
 *  Determines the coordinate where bullets appear.
 *  Contains an additional argument to add an offset to the firing coordinate,
 *  used by the spawn manager.
 *
 *  @author: ZivDero
 */
Coordinate TechnoClassExtension::Fire_Coord(WeaponSlotType which, TPoint3D<int> offset) const
{
    const TechnoTypeClass *ttype = This()->Techno_Type_Class();
    const auto weaponinfo = This()->Get_Weapon(which);

    Matrix3D matrix;
    matrix.Make_Identity();

    float theta = This()->Turret_Facing().Get_Radian<32>();
    matrix.Rotate_Z(theta);

    const TPoint3D<int> flh = weaponinfo->FireFLH + offset;

    const float trans_x = static_cast<float>(flh.X + ttype->TurretOffset);
    const float trans_y = static_cast<float>(flh.Y * (This()->CurrentBurstIndex % 2 == 0 ? 1 : -1));
    const float trans_z = static_cast<float>(flh.Z + weaponinfo->BarrelThickness);
    matrix.Translate(trans_x, trans_y, trans_z);

    theta = -This()->BarrelFacing.Current().Get_Radian<32>();
    matrix.Rotate_Y(theta);

    matrix.Translate(static_cast<float>(weaponinfo->BarrelLength), 0, 0);

    const Vector3 fire_coord = matrix * Vector3(0, 0, 0);
    Coordinate render_coord = This()->Render_Coord();

    return { render_coord.X + static_cast<int>(fire_coord.X), render_coord.Y - static_cast<int>(fire_coord.Y), render_coord.Z + static_cast<int>(fire_coord.Z) };
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
    return reinterpret_cast<TechnoClass *>(This())->Techno_Type_Class();
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
