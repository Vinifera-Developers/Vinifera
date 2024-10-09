/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SPAWNMANAGER.CPP
 *
 *  @authors       ZivDero
 *
 *  @brief         SpawnManagerClass reimplementation from YR.
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

#include "spawnmanager.h"

#include "aircraft.h"
#include "aircraftext.h"
#include "aircrafttype.h"
#include "aircrafttypeext.h"
#include "anim.h"
#include "animtype.h"
#include "cell.h"
#include "extension.h"
#include "ionstorm.h"
#include "kamikazetracker/kamikazetracker.h"
#include "tibsun_inline.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "weapontype.h"
#include "weapontypeext.h"
#include "rockettype.h"
#include "vinifera_saveload.h"


/**
  *  Retrieves the class identifier (CLSID) of the object.
  *
  *  @author: ZivDero
  */
IFACEMETHODIMP SpawnManagerClass::GetClassID(CLSID* pClassID)
{
    if (pClassID == nullptr) {
        return E_POINTER;
    }

    *pClassID = __uuidof(this);

    return S_OK;
}

/**
 *  Initializes an object from the stream where it was saved previously.
 *
 *  @author: ZivDero
 */
IFACEMETHODIMP SpawnManagerClass::Load(IStream* pStm)
{
    HRESULT hr = Abstract_Load(pStm);
    if (FAILED(hr))
        return hr;

    new (this) SpawnManagerClass(NoInitClass());

    /**
     *  Read the count of active spawn controls.
     */
    hr = pStm->Read(&SpawnCount, sizeof(SpawnCount), nullptr);
    if (FAILED(hr))
        return hr;

    new (&SpawnControls) DynamicVectorClass<SpawnControl*>();

    if (SpawnCount <= 0)
        return hr;

    /**
     *  Read each of the controls as a binary blob.
     */
    for (int index = 0; index < SpawnCount; ++index)
    {
        const auto control = new SpawnControl;
        hr = pStm->Read(control, sizeof(SpawnControl), nullptr);
        if (FAILED(hr))
            return hr;
        SpawnControls.Add(control);
    }

    for (int i = 0; i < SpawnCount; i++)
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(SpawnControls[i]->Spawnee, "Spawnee");

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Owner, "Owner");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(SpawnType, "SpawnType");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(SuspendedTarget, "SuspendedTarget");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Target, "Target");

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *
 *  @author: ZivDero
 */
IFACEMETHODIMP SpawnManagerClass::Save(IStream* pStm, BOOL fClearDirty)
{
    HRESULT hr = Abstract_Save(pStm, fClearDirty);
    if (FAILED(hr))
        return hr;

    /**
     *  Write the count of active spawn controls.
     */
    hr = pStm->Write(&SpawnCount, sizeof(SpawnCount), nullptr);
    if (FAILED(hr))
        return hr;

    if (SpawnCount <= 0)
        return hr;

    /**
     *  Write each of the controls as a binary blob.
     */
    for (int index = 0; index < SpawnCount; ++index)
    {
        hr = pStm->Write(SpawnControls[index], sizeof(SpawnControl), nullptr);
        if (FAILED(hr))
            return hr;
    }

    return hr;
}


/**
 *  Basic constructor for the SpawnManagerClass.
 *
 *  @author: ZivDero
 */
SpawnManagerClass::SpawnManagerClass() :
    Owner(nullptr),
    SpawnType(nullptr),
    SpawnCount(0),
    SuspendedTarget(nullptr),
    Target(nullptr),
    Status(SpawnManagerStatus::Idle)
{
    SpawnManagers.Add(this);
}


/**
 *  Basic constructor for the SpawnManagerClass.
 *
 *  @author: ZivDero
 */
SpawnManagerClass::SpawnManagerClass(TechnoClass* owner, const AircraftTypeClass* spawns, int spawn_count, int regen_rate, int reload_rate) :
    Owner(owner),
    SpawnType(spawns),
    SpawnCount(spawn_count),
    RegenRate(regen_rate),
    ReloadRate(reload_rate),
    SuspendedTarget(nullptr),
    Target(nullptr),
    Status(SpawnManagerStatus::Idle)
{
    for (int i = 0; i < SpawnCount; i++)
    {
        auto control = new SpawnControl();
        if (control == nullptr)
            break;

        auto spawnee = static_cast<AircraftClass*>(SpawnType->Create_One_Of(owner->Owning_House()));
        control->Spawnee = spawnee;

        if (spawnee != nullptr)
        {
            control->IsSpawnedMissile = RocketTypeClass::From_AircraftType(SpawnType) != nullptr;
            control->Spawnee->Limbo();
            Extension::Fetch<AircraftClassExtension>(control->Spawnee)->SpawnOwner = Owner;
            control->Status = SpawnControlStatus::Idle;
            control->ReloadTimer = 0;
            SpawnControls.Add(control);
        }
    }

    SpawnManagers.Add(this);
}


/**
 *  Basic destructor for the SpawnManagerClass.
 *
 *  @author: ZivDero
 */
SpawnManagerClass::~SpawnManagerClass()
{
    if (GameActive)
        Detach_Spawns();

    for (int i = SpawnControls.Count() - 1; i >= 0; i--)
    {
        if (SpawnControls[i] != nullptr)
            delete SpawnControls[i];
    }

    SpawnManagers.Delete(this);
}


/**
 *  Returns the RTTI type of the object.
 *
 *  @author: ZivDero
 */
RTTIType SpawnManagerClass::Kind_Of() const
{
    return static_cast<RTTIType>(RTTI_SPAWN_MANAGER);
}


/**
 *  Returns the size of the object.
 *
 *  @author: ZivDero
 */
int SpawnManagerClass::Size_Of(bool firestorm) const
{
    return sizeof(*this);
}


/**
 *  Computes the CRC of the object.
 *
 *  @author: ZivDero
 */
void SpawnManagerClass::Compute_CRC(WWCRCEngine& crc) const
{
    AbstractClass::Compute_CRC(crc);

    crc(static_cast<int>(Status));

    if (Target != nullptr)
        crc(Target->Get_Heap_ID());

    if (SuspendedTarget != nullptr)
        crc(SuspendedTarget->Get_Heap_ID());

    crc(SpawnTimer.Value());
    crc(LogicTimer.Value());
    crc(SpawnControls.Count());
    crc(SpawnCount);

    if (SpawnType != nullptr)
        crc(SpawnType->Get_Heap_ID());

    if (Owner != nullptr)
        crc(Owner->Get_Heap_ID());
}


/**
 *  Performs all the main SpawnManager logic.
 *
 *  @author: ZivDero
 */
void SpawnManagerClass::AI()
{
    if (!LogicTimer.Expired())
        return;

    LogicTimer = 10;

    for (int i = 0; i < SpawnControls.Count(); i++)
    {
        SpawnControl* control = SpawnControls[i];
        AircraftClass* spawnee = control->Spawnee;
        TechnoTypeClassExtension* owner_class_ext = Extension::Fetch<TechnoTypeClassExtension>(Owner->Techno_Type_Class());

        switch (control->Status)
        {
        case SpawnControlStatus::Idle:
            {
                if (!SuspendedTarget)
                    break;

                if (!SpawnTimer.Expired())
                    break;

                if (Status == SpawnManagerStatus::Cooldown || IonStorm_Is_Active())
                    break;

                if (control->IsSpawnedMissile)
                {
                    if (static_cast<FootClass*>(Owner)->Locomotion->Is_Moving() || static_cast<FootClass*>(Owner)->Locomotion->Is_Moving_Now())
                        continue;
                }

                // Maybe should check the missile instead, huh?
                SpawnTimer = owner_class_ext->IsMissileSpawn ? 9 : 20;

                bool burst = false;
                Coordinate fire_coord;
                if (control->IsSpawnedMissile &&
                    Owner->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon->Burst > 1)
                {
                    burst = true;
                    Owner->CurrentBurstIndex = i % 2;
                }

                control->Status = SpawnControlStatus::Preparing;

                WeaponSlotType weapon_slot = Extension::Fetch<WeaponTypeClassExtension>(Owner->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon)->IsSpawner ? WEAPON_SLOT_PRIMARY : WEAPON_SLOT_SECONDARY;

                // In the "yes" case should apply second spawn offset, not implemented
                if (Owner->CurrentBurstIndex > 0)
                {
                    fire_coord = Owner->Fire_Coord(weapon_slot);
                }
                else
                {
                    fire_coord = Owner->Fire_Coord(weapon_slot);
                }

                Coordinate spawn_coord = Coordinate(fire_coord.X, fire_coord.Y, fire_coord.Z + 10);

                const auto rocket = RocketTypeClass::From_AircraftType(SpawnType);
                if (rocket && rocket->IsCruiseMissile)
                {
                    spawn_coord.X -= 40;
                    spawn_coord.Y -= 40;
                }

                DirStruct dir = Owner->PrimaryFacing.Current();
                spawnee->Unlimbo(spawn_coord, dir.Get_Dir());

                if (rocket && rocket->IsCruiseMissile && rocket->TakeoffAnim)
                    new AnimClass(rocket->TakeoffAnim, spawnee->Coord, 2, 1, SHAPE_WIN_REL | SHAPE_CENTER, -10);

                if (burst)
                    Owner->CurrentBurstIndex = 0;

                if (control->IsSpawnedMissile)
                {
                    Suspend_Target();
                    spawnee->Assign_Destination(SuspendedTarget);
                    spawnee->Assign_Mission(MISSION_MOVE);
                }
                else
                {
                    CellClass* owner_cell = Owner->Get_Cell_Ptr();
                    CellClass* adjacent_cell = &owner_cell->Adjacent_Cell(FACING_S);
                    spawnee->Assign_Destination(adjacent_cell);
                    spawnee->Assign_Mission(MISSION_MOVE);
                }

                break;
            }

        case SpawnControlStatus::Takeoff:
            {
                if (control->ReloadTimer.Expired())
                    Detach(spawnee);
                break;
            }

        case SpawnControlStatus::Preparing:
            {
                if (control->IsSpawnedMissile)
                    break;

                Suspend_Target();
                if (SuspendedTarget != nullptr)
                {
                    spawnee->Assign_Destination(Owner);
                    spawnee->Assign_Target(nullptr);
                    spawnee->Assign_Mission(MISSION_MOVE);
                    spawnee->Commence();
                    control->Status = SpawnControlStatus::Returning;
                    break;
                }

                CellClass* owner_cell = Owner->Get_Cell_Ptr();
                CellClass* adjacent_cell = &owner_cell->Adjacent_Cell(FACING_S);
                spawnee->Assign_Destination(adjacent_cell);
                spawnee->Assign_Mission(MISSION_MOVE);
                break;
            }

        case SpawnControlStatus::Attacking:
            {
                Suspend_Target();
                if (spawnee->Ammo > 0 && SuspendedTarget)
                {
                    spawnee->Assign_Target(SuspendedTarget);
                    spawnee->Assign_Mission(MISSION_ATTACK);
                }
                else
                {
                    spawnee->Assign_Destination(Owner);
                    spawnee->Assign_Target(nullptr);
                    spawnee->Assign_Mission(MISSION_MOVE);
                    control->Status = SpawnControlStatus::Returning;
                }
                break;
            }

        case SpawnControlStatus::Returning:
            {
                Suspend_Target();
                if (spawnee->Ammo > 0 && SuspendedTarget)
                {
                    control->Status = SpawnControlStatus::Attacking;
                    spawnee->Assign_Target(SuspendedTarget);
                    spawnee->Assign_Mission(MISSION_ATTACK);
                    break;
                }

                Cell owner_coord = Owner->Get_Cell();
                Cell spawnee_coord = spawnee->Get_Cell();

                if (owner_coord == spawnee_coord && spawnee->Coord.Z - Owner->Coord.Z < 20)
                {
                    spawnee->Limbo();
                    control->Status = SpawnControlStatus::Reloading;
                    control->ReloadTimer = ReloadRate;
                }
                else
                {
                    spawnee->Assign_Destination(Owner);
                    spawnee->Assign_Target(nullptr);
                    spawnee->Assign_Mission(MISSION_MOVE);
                }

                break;
            }

        case SpawnControlStatus::Reloading:
            {
                if (!control->ReloadTimer.Expired())
                    break;

                control->Status = SpawnControlStatus::Idle;
                spawnee->Ammo = spawnee->Class->MaxAmmo;
                spawnee->Strength = spawnee->Class->MaxStrength;
                break;
            }

        case SpawnControlStatus::Dead:
            {
                if (!control->ReloadTimer.Expired())
                    break;

                control->Spawnee = static_cast<AircraftClass*>(SpawnType->Create_One_Of(Owner->Owning_House()));
                control->IsSpawnedMissile = RocketTypeClass::From_AircraftType(SpawnType) != nullptr;
                control->Spawnee->Limbo();
                Extension::Fetch<AircraftClassExtension>(control->Spawnee)->SpawnOwner = Owner;
                control->Status = SpawnControlStatus::Idle;
                break;
            }
        }
    }

    if (Status == SpawnManagerStatus::Idle)
    {
        Suspend_Target();
        if (SuspendedTarget != nullptr)
        {
            WeaponSlotType weapon = Owner->What_Weapon_Should_I_Use(SuspendedTarget);
            if (Owner->In_Range_Of(SuspendedTarget, weapon))
                Status = SpawnManagerStatus::Launching;
            else
                Abandon_Target();
        }
    }
    else if (Status == SpawnManagerStatus::Launching)
    {
        if (SuspendedTarget == nullptr)
        {
            Abandon_Target();
            return;
        }

        for (int i = 0; i < SpawnControls.Count(); i++)
        {
            const SpawnControl* control = SpawnControls[i];
            if (control->Status != SpawnControlStatus::Preparing && control->Status != SpawnControlStatus::Dead)
                return;
        }

        bool has_missiles = false;
        for (int i = 0; i < SpawnControls.Count(); i++)
        {
            SpawnControl* control = SpawnControls[i];
            AircraftClass* spawnee = control->Spawnee;
            if (control->Status == SpawnControlStatus::Preparing)
            {
                if (Extension::Fetch<AircraftTypeClassExtension>(spawnee->Techno_Type_Class())->IsMissileSpawn)
                {
                    has_missiles = true;
                    KamikazeTracker->Add(spawnee, SuspendedTarget);
                    KamikazeTracker->UpdateTimer = 2;

                    if (control->IsSpawnedMissile)
                    {
                        control->Status = SpawnControlStatus::Takeoff;
                        const auto atype = control->Spawnee->Class;
                        const RocketTypeClass* rocket = RocketTypeClass::From_AircraftType(atype);
                        control->ReloadTimer = rocket->IsCruiseMissile ? 0 : rocket->PauseFrames + rocket->TiltFrames;
                    }
                    else
                    {
                        Detach(spawnee);
                    }
                }
                else
                {
                    control->Status = SpawnControlStatus::Attacking;
                    spawnee->Assign_Target(SuspendedTarget);
                    spawnee->Assign_Mission(MISSION_ATTACK);
                }
            }
        }

        if (has_missiles)
            Abandon_Target();

        Status = SpawnManagerStatus::Cooldown;
    }
    else if (Status == SpawnManagerStatus::Cooldown)
    {
        bool is_idle = true;
        for (int i = 0; i < SpawnControls.Count(); i++)
        {
            SpawnControl* control = SpawnControls[i];
            if (control->Status == SpawnControlStatus::Attacking || control->Status == SpawnControlStatus::Returning)
            {
                is_idle = false;
                break;
            }
        }

        if (is_idle)
            Status = SpawnManagerStatus::Idle;
    }
}


/**
 *  Removes any spawns that are not currently in flight, and sends the rest into kamikaze mode.
 *
 *  @author: ZivDero
 */
void SpawnManagerClass::Detach_Spawns()
{
    int regen_rate = Owner->Strength > 0 && Owner->IsActive ? 0 : RegenRate;

    for (int i = 0; i < SpawnControls.Count(); i++)
    {
        SpawnControl* control = SpawnControls[i];
        if (control->Status == SpawnControlStatus::Dead)
            continue;

        if (control->Status == SpawnControlStatus::Idle || control->Status == SpawnControlStatus::Reloading)
        {
            control->Status = SpawnControlStatus::Dead;
            control->Spawnee->Remove_This();
        }
        else
        {
            if (control->Status == SpawnControlStatus::Takeoff)
            {
                KamikazeTracker->Detach(control->Spawnee);
                control->Status = SpawnControlStatus::Dead;
                control->Spawnee->Remove_This();
            }
            else
            {
                control->Status = SpawnControlStatus::Dead;
                KamikazeTracker->Add(control->Spawnee, SuspendedTarget);
            }
        }

        control->Spawnee = nullptr;
        control->ReloadTimer = regen_rate;
    }
}


/**
 *  Assigns a new target.
 *
 *  @author: ZivDero
 */
void SpawnManagerClass::Assign_Target(TARGET target)
{
    if (target != SuspendedTarget)
        Target = target;
}


/**
 *  Managers the kamikaze spawns.
 *
 *  @author: ZivDero
 */
void SpawnManagerClass::Abandon_Target()
{
    for (int i = 0; i < SpawnControls.Count(); ++i)
    {
        SpawnControl* control = SpawnControls[i];
        if (control->Status == SpawnControlStatus::Preparing)
        {
            AircraftTypeClassExtension* extension = Extension::Fetch<AircraftTypeClassExtension>(control->Spawnee->Techno_Type_Class());
            if (extension->IsMissileSpawn)
            {
                KamikazeTracker->Add(control->Spawnee, SuspendedTarget);
                KamikazeTracker->UpdateTimer = 2;
                Detach(control->Spawnee);
            }
        }
    }

    Status = SpawnManagerStatus::Idle;
    Target = nullptr;
    SuspendedTarget = nullptr;
}


/**
 *  Suspends the current target.
 *
 *  @author: ZivDero
 */
bool SpawnManagerClass::Suspend_Target()
{
    if (Target == nullptr)
        return false;

    SuspendedTarget = Target;
    Target = nullptr;
    return true;
}


/**
 *  Detaches an object from the SpawnManager.
 *
 *  @author: ZivDero
 */
void SpawnManagerClass::Detach(TARGET target)
{
    if (SuspendedTarget == target)
    {
        SuspendedTarget = nullptr;

        if (!Target)
            Abandon_Target();
    }
    else if (Target == target)
    {
        Target = nullptr;
    }
    else
    {
        for (int i = 0; i < SpawnControls.Count(); i++)
        {
            const auto control = SpawnControls[i];
            if (control->Spawnee == target)
            {
                if (control->Spawnee->Strength <= 0 || control->Spawnee->IsKamikaze || control->IsSpawnedMissile)
                {
                    control->Spawnee = nullptr;
                    control->Status = SpawnControlStatus::Dead;
                    control->ReloadTimer = RegenRate;
                }

                break;
            }
        }

        if (Owner == target)
        {
            Detach_Spawns();
            Abandon_Target();
        }
    }
}


/**
 *  Returns the count of currently active spawns.
 *
 *  @author: ZivDero
 */
int SpawnManagerClass::Active_Count()
{
    int count = 0;
    for (int i = 0; i < SpawnControls.Count(); i++)
    {
        if (SpawnControls[i]->Status != SpawnControlStatus::Dead)
            count++;
    }
    return count;
}


/**
 *  Returns the count of currently docked spawns.
 *
 *  @author: ZivDero
 */
int SpawnManagerClass::Docked_Count()
{
    int count = 0;
    for (int i = 0; i < SpawnControls.Count(); i++)
    {
        if (SpawnControls[i]->Status == SpawnControlStatus::Reloading ||
            SpawnControls[i]->Status == SpawnControlStatus::Idle)
            count++;
    }
    return count;
}


/**
 *  Returns the count of spawns currently preparing to take off.
 *
 *  @author: ZivDero
 */
int SpawnManagerClass::Preparing_Count()
{
    int count = 0;
    for (int i = 0; i < SpawnControls.Count(); i++)
    {
        if (SpawnControls[i]->Status == SpawnControlStatus::Takeoff)
            count++;

        if (SpawnControls[i]->Status == SpawnControlStatus::Preparing)
        {
            const AircraftClass* spawnee = SpawnControls[i]->Spawnee;
            if (spawnee && !spawnee->IsInLimbo
                && Extension::Fetch<AircraftTypeClassExtension>(spawnee->Techno_Type_Class())->IsMissileSpawn)
            {
                count++;
            }
        }
    }
    return count;
}

/**
 *  Removes all SpawnManagers from the game world.
 *
 *  @author: ZivDero
 */
void SpawnManagerClass::Clear_All()
{
    for (int i = 0; i < SpawnManagers.Count(); ++i)
        delete SpawnManagers[i];
    
    SpawnManagers.Clear();
}
