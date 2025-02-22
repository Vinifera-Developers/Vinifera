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
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Target, "SuspendedTarget");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(QueuedTarget, "Target");

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
    Target(nullptr),
    QueuedTarget(nullptr),
    Status(SpawnManagerStatus::Idle)
{
    SpawnManagers.Add(this);
}


/**
 *  Basic constructor for the SpawnManagerClass.
 *
 *  @author: ZivDero
 */
SpawnManagerClass::SpawnManagerClass(TechnoClass* owner, const AircraftTypeClass* spawns, int spawn_count, int regen_rate, int reload_rate, int spawn_rate, int logic_rate) :
    Owner(owner),
    SpawnType(spawns),
    SpawnCount(spawn_count),
    RegenRate(regen_rate),
    ReloadRate(reload_rate),
    SpawnRate(spawn_rate),
    LogicRate(logic_rate),
    Target(nullptr),
    QueuedTarget(nullptr),
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

    if (QueuedTarget != nullptr)
        crc(QueuedTarget->Get_Heap_ID());

    if (Target != nullptr)
        crc(Target->Get_Heap_ID());

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
    /**
     *  The spawner only does logic every 10 frames.
     */
    if (!LogicTimer.Expired())
        return;

    LogicTimer = LogicRate;

    /**
     *  No spawning if the spawner is EMP'ed.
     */
    if (Owner->EMPFramesRemaining)
        Owner->Assign_Target(nullptr);

    /**
     *  Iterate all the controls.
     */
    for (int i = 0; i < SpawnControls.Count(); i++)
    {
        SpawnControl* control = SpawnControls[i];
        AircraftClass* spawnee = control->Spawnee;
        const auto owner_ext = Extension::Fetch<TechnoClassExtension>(Owner);
        const auto owner_type_ext = Extension::Fetch<TechnoTypeClassExtension>(Owner->Techno_Type_Class());

        switch (control->Status)
        {
            /**
             *  The spawn is currently idle.
             */
        case SpawnControlStatus::Idle:
            {
                /**
                 *  If we don't have a target, no need to do anything about it.
                 */
                if (!Target)
                    continue;

                /**
                 *  If it's not yet time to respawn, skip.
                 */
                if (!SpawnTimer.Expired())
                    continue;

                /**
                 *  If we're on cooldown.
                 */
                if (Status == SpawnManagerStatus::Cooldown)
                    continue;

                /**
                 *  No spawning during an Ion Storm.
                 */
                if (IonStorm_Is_Active())
                    continue;

                /**
                 *  No spawning if the spawner is EMP'ed.
                 */
                if (Owner->EMPFramesRemaining)
                    continue;

                /**
                 *  If the spawner can move (i. e. is not a building), don't allow spawning while it's on the move.
                 */
                if (control->IsSpawnedMissile && Owner->Is_Foot())
                {
                    if (static_cast<FootClass*>(Owner)->Locomotion->Is_Moving() || static_cast<FootClass*>(Owner)->Locomotion->Is_Moving_Now())
                        continue;
                }

                /**
                 *  Not quite sure what's up here.
                 *  Maybe should check the missile instead, huh?
                 */
                SpawnTimer = SpawnRate;

                /**
                 *  We can spawn 2 missiles using the burst logic.
                 */
                const auto weapon = Owner->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon;
                if (control->IsSpawnedMissile && weapon->Burst > 1 && i < weapon->Burst)
                    Owner->CurrentBurstIndex = i;
                else
                    Owner->CurrentBurstIndex = 0;

                /**
                 *  Update our status.
                 */
                control->Status = SpawnControlStatus::Preparing;

                WeaponSlotType weapon_slot = Extension::Fetch<WeaponTypeClassExtension>(Owner->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon)->IsSpawner ? WEAPON_SLOT_PRIMARY : WEAPON_SLOT_SECONDARY;

                /**
                 *  Apply SecondSpawnOffset if this is the second missile in a burst.
                 */
                Coordinate fire_coord;
                if (Owner->CurrentBurstIndex % 2 == 0)
                    fire_coord = owner_ext->Fire_Coord(weapon_slot);
                else
                    fire_coord = owner_ext->Fire_Coord(weapon_slot, owner_type_ext->SecondSpawnOffset);

                Coordinate spawn_coord = Coordinate(fire_coord.X, fire_coord.Y, fire_coord.Z + 10);

                /**
                 *  Randomize the horizontal position a bit if requested.
                 */
                if (owner_type_ext->MaxRandomSpawnOffset > 0)
                    spawn_coord += Coordinate(Random_Pick(0, owner_type_ext->MaxRandomSpawnOffset), Random_Pick(0, owner_type_ext->MaxRandomSpawnOffset), 0);

                /**
                 *  Place the spawn in the world.
                 */
                DirStruct dir = Owner->PrimaryFacing.Current();
                spawnee->Unlimbo(spawn_coord, dir.Get_Dir());

                const auto rocket = RocketTypeClass::From_AircraftType(SpawnType);

                /**
                 *  Cruise missiles spawn their takeoff animation.
                 */
                if (rocket && rocket->IsCruiseMissile && rocket->TakeoffAnim)
                    new AnimClass(rocket->TakeoffAnim, spawnee->Coord, 2, 1, SHAPE_WIN_REL | SHAPE_CENTER, -10);

                /**
                 *  Reset burst since if we're done with this volley.
                 */
                if (i == SpawnControls.Count() - 1)
                    Owner->CurrentBurstIndex = 0;

                /**
                 *  Missiles only take a destination once, so they go straight to the target.
                 */
                if (control->IsSpawnedMissile)
                {
                    Next_Target();
                    spawnee->Assign_Destination(Target);
                    spawnee->Assign_Mission(MISSION_MOVE);
                }
                /**
                 *  Aircraft first "organize" next to the spawner.
                 */
                else
                {
                    CellClass* owner_cell = Owner->Get_Cell_Ptr();
                    CellClass* adjacent_cell = &owner_cell->Adjacent_Cell(FACING_S);
                    spawnee->Assign_Destination(adjacent_cell);
                    spawnee->Assign_Mission(MISSION_MOVE);
                }

                break;
            }

            /**
             *  The rocket is taking off (handled by the locomotor), so wait until it's done, then let it go.
             */
        case SpawnControlStatus::Takeoff:
            {
                if (control->ReloadTimer.Expired())
                    Detach(spawnee);
                break;
            }

            /**
             *  The aircraft is preparing to attack.
             */
        case SpawnControlStatus::Preparing:
            {
                /**
                 *  Missiles don't do this.
                 */
                if (control->IsSpawnedMissile)
                    break;

                /**
                 *  If there's no target, return to base.
                 */
                Next_Target();
                if (!Target)
                {
                    spawnee->Assign_Destination(Owner);
                    spawnee->Assign_Target(nullptr);
                    spawnee->Assign_Mission(MISSION_MOVE);
                    spawnee->Commence();
                    control->Status = SpawnControlStatus::Returning;
                }
                /**
                 *  Send the aircraft to attack.
                 */
                else
                {
                    CellClass* adjacent_cell = &Owner->Get_Cell_Ptr()->Adjacent_Cell(FACING_S);
                    spawnee->Assign_Destination(adjacent_cell);
                    spawnee->Assign_Mission(MISSION_MOVE);
                }
                break;
            }

            /**
             *  The aircraft is currently attacking.
             */
        case SpawnControlStatus::Attacking:
            {
                /**
                 *  If there's still ammo and a target, attack.
                 */
                Next_Target();
                if (spawnee->Ammo > 0 && Target)
                {
                    spawnee->Assign_Target(Target);
                    spawnee->Assign_Mission(MISSION_ATTACK);
                }
                /**
                 *  Otherwise, return to base.
                 */
                else
                {
                    spawnee->Assign_Destination(Owner);
                    spawnee->Assign_Target(nullptr);
                    spawnee->Assign_Mission(MISSION_MOVE);
                    control->Status = SpawnControlStatus::Returning;
                }
                break;
            }

            /**
             *  The aircraft is retuning back to the spawner.
             */
        case SpawnControlStatus::Returning:
            {
                /**
                 *  Check if we've got ammo and there's a target now.
                 *  If so, attack it.
                 */
                Next_Target();
                if (spawnee->Ammo > 0 && Target)
                {
                    control->Status = SpawnControlStatus::Attacking;
                    spawnee->Assign_Target(Target);
                    spawnee->Assign_Mission(MISSION_ATTACK);
                    break;
                }

                /**
                 *  If we've arrived at the spawner, "land" (despawn).
                 *  Otherwise, keep going towards the spawner.
                 */
                Cell owner_coord = Owner->Get_Cell();
                Cell spawnee_coord = spawnee->Get_Cell();

                if (owner_coord == spawnee_coord && std::abs(spawnee->Coord.Z - Owner->Coord.Z) < 20)
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

            /**
             *  The aircraft has expended its ammo and is reloading.
             */
        case SpawnControlStatus::Reloading:
            {
                /**
                 *  Wait until the reload timer expires.
                 */
                if (!control->ReloadTimer.Expired())
                    break;

                /**
                 *  Then reset the spawn to max ammo and health.
                 */
                control->Status = SpawnControlStatus::Idle;
                spawnee->Ammo = spawnee->Class->MaxAmmo;
                spawnee->Strength = spawnee->Class->MaxStrength;
                break;
            }

            /**
             *  The spawn has been destroyed and is respawning.
             */
        case SpawnControlStatus::Dead:
            {
                /**
                 *  Wait until the reload timer expires.
                 */
                if (!control->ReloadTimer.Expired())
                    break;

                /**
                 *  Create a new spawn and set it to idle.
                 */
                control->Spawnee = static_cast<AircraftClass*>(SpawnType->Create_One_Of(Owner->Owning_House()));
                control->IsSpawnedMissile = RocketTypeClass::From_AircraftType(SpawnType) != nullptr;
                control->Spawnee->Limbo();
                Extension::Fetch<AircraftClassExtension>(control->Spawnee)->SpawnOwner = Owner;
                control->Status = SpawnControlStatus::Idle;
                break;
            }
        }
    }

    /**
     *  If the spawner is currently idling, check if there's a target.
     *  If there is one, and it's in range, attack it.
     */
    if (Status == SpawnManagerStatus::Idle)
    {
        Next_Target();
        if (Target)
        {
            WeaponSlotType weapon = Owner->What_Weapon_Should_I_Use(Target);
            if (Owner->In_Range_Of(Target, weapon))
                Status = SpawnManagerStatus::Launching;
            else
                Abandon_Target();
        }
    }
    else if (Status == SpawnManagerStatus::Launching)
    {
        /**
         *  If we're launching spawns, but there isn't a target anymore, stop it.
         */
        if (Target == nullptr)
        {
            Abandon_Target();
            return;
        }

        /**
         *  Check to make sure all of our spawns are currently preparing to launch.
         *  This should only happen when the spawns are missiles, I believe.
         */
        for (int i = 0; i < SpawnControls.Count(); i++)
        {
            const SpawnControl* control = SpawnControls[i];
            if (control->Status != SpawnControlStatus::Preparing && control->Status != SpawnControlStatus::Dead)
                return;
        }

        /**
         *  Process all our missiles.
         */
        bool is_missile_launcher = false;
        for (int i = 0; i < SpawnControls.Count(); i++)
        {
            SpawnControl* control = SpawnControls[i];
            AircraftClass* spawnee = control->Spawnee;

            /**
             *  Don't process dead spawns.
             */
            if (control->Status == SpawnControlStatus::Preparing)
            {
                /**
                 *  If the spawn is a missile, add it to the kamikaze tracker and set it to take off.
                 *  Also set the reload timer to the missile's takeoff time.
                 */
                if (Extension::Fetch<AircraftTypeClassExtension>(spawnee->Techno_Type_Class())->IsMissileSpawn)
                {
                    is_missile_launcher = true;
                    KamikazeTracker->Add(spawnee, Target);
                    KamikazeTracker->UpdateTimer = 2;

                    if (control->IsSpawnedMissile)
                    {
                        control->Status = SpawnControlStatus::Takeoff;
                        const auto atype = control->Spawnee->Class;
                        const RocketTypeClass* rocket = RocketTypeClass::From_AircraftType(atype);
                        control->ReloadTimer = rocket->PauseFrames + rocket->TiltFrames;
                    }
                    else
                    {
                        Detach(spawnee);
                    }
                }
                /**
                 *  On the off chance it's not a missile, just set it to attack.
                 */
                else
                {
                    control->Status = SpawnControlStatus::Attacking;
                    spawnee->Assign_Target(Target);
                    spawnee->Assign_Mission(MISSION_ATTACK);
                }
            }
        }

        /**
         *  If this is a missile launcher,
         *  abandon the target.
         */
        if (is_missile_launcher)
            Abandon_Target();

        /**
         *  Phew, time to go on cooldown.
         */
        Status = SpawnManagerStatus::Cooldown;
    }
    /**
     *  If we're on cooldown, check if any of the spawns are currently attacking or returning.
     *  If they are, change the status to idle instead.
     */
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

    /**
     *  Iterate all the spawns.
     */
    for (int i = SpawnControls.Count() - 1; i >= 0; i--)
    {
        /**
         *  Don't need to do anything about dead spawns.
         */
        SpawnControl* control = SpawnControls[i];
        if (control->Status != SpawnControlStatus::Dead)
        {
            /**
             *  If it's currently docked, just kill it off. It's already limboed.
             */
            if (control->Status != SpawnControlStatus::Idle && control->Status != SpawnControlStatus::Reloading)
            {
                /**
                 *  If it's a rocket taking off, detach it and remove it from the world.
                 */
                if (control->Status == SpawnControlStatus::Takeoff)
                {
                    KamikazeTracker->Detach(control->Spawnee);
                    control->Status = SpawnControlStatus::Dead;
                    control->Spawnee->Remove_This();
                }
                /**
                 *  Otherwise it's probably currently in flight, so just detach it.
                 */
                else
                {
                    control->Status = SpawnControlStatus::Dead;
                    KamikazeTracker->Add(control->Spawnee, Target);
                }

                control->Spawnee = nullptr;
            }
            else
            {
                control->Status = SpawnControlStatus::Dead;
                control->Spawnee->Remove_This();
                control->Spawnee = nullptr;
            }

            /**
             *  Set the spawn to regenerate.
             */
            control->ReloadTimer = regen_rate;
        }
    }
}


/**
 *  Assigns a new target.
 *
 *  @author: ZivDero
 */
void SpawnManagerClass::Queue_Target(TARGET target)
{
    if (target != Target)
        QueuedTarget = target;
}


/**
 *  Managers the kamikaze spawns.
 *
 *  @author: ZivDero
 */
void SpawnManagerClass::Abandon_Target()
{
    /**
     *  Loop all the spawns. If any of them are currently preparing to attack, drop them.
     */
    for (int i = 0; i < SpawnControls.Count(); ++i)
    {
        SpawnControl* control = SpawnControls[i];
        if (control->Status == SpawnControlStatus::Preparing)
        {
            const auto extension = Extension::Fetch<AircraftTypeClassExtension>(control->Spawnee->Techno_Type_Class());
            if (extension->IsMissileSpawn)
            {
                KamikazeTracker->Add(control->Spawnee, Target);
                KamikazeTracker->UpdateTimer = 2;
                Detach(control->Spawnee);
            }
        }
    }

    /**
     *  Set the status to idle and remove any targets.
     */
    Status = SpawnManagerStatus::Idle;
    QueuedTarget = nullptr;
    Target = nullptr;
}


/**
 *  Suspends the current target.
 *
 *  @author: ZivDero
 */
bool SpawnManagerClass::Next_Target()
{
    if (QueuedTarget == nullptr)
        return false;

    Target = QueuedTarget;
    QueuedTarget = nullptr;
    return true;
}


/**
 *  Detaches an object from the SpawnManager.
 *
 *  @author: ZivDero
 */
void SpawnManagerClass::Detach(TARGET target)
{
    /**
     *  If it's the suspended target, remove it.
     *  If we don't have any more targets, stop attacking.
     */
    if (Target == target)
    {
        Target = nullptr;

        if (!QueuedTarget)
            Abandon_Target();
    }
    /**
     *  If it's the current target, remove it.
     */
    else if (QueuedTarget == target)
    {
        QueuedTarget = nullptr;
    }
    else
    {
        /**
         *  Check if it's one of the spawns. If so, remove it.
         */
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

        /**
         *  lastly, check if it's maybe the owner of the spawner itself.
         */
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
