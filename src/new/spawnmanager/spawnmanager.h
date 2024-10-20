/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SPAWNMANAGER.H
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
#pragma once

#include "vinifera_defines.h"
#include "foot.h"
#include "ttimer.h"

class SpawnManagerClass;
class AircraftTypeClass;
class TechnoClass;
class FrameTimerClass;
class AircraftClass;

enum class SpawnManagerStatus {
    Idle = 0,		// no target or out of range
    Launching = 1,	// a launch in progress
    Cooldown = 2	// waiting for launch to complete
};

enum class SpawnControlStatus {
    Idle = 0,		// docked, waiting for target
    Takeoff = 1,	// missile tilting and launch
    Preparing = 2,	// gathering, waiting
    Attacking = 3,	// attacking until no ammo
    Returning = 4,	// return to carrier
    //Unused_5,		// not used
    Reloading = 6,	// docked, reloading ammo and health
    Dead = 7		// respawning
};


class DECLSPEC_UUID(CLSID_SPAWN_MANAGER_CLASS)
    SpawnManagerClass : public AbstractClass
{
public:
    struct SpawnControl
    {
        AircraftClass* Spawnee;
        SpawnControlStatus Status;
        CDTimerClass<FrameTimerClass> ReloadTimer;
        bool IsSpawnedMissile;
    };

    /**
    *  IPersist
    */
    IFACEMETHOD(GetClassID)(CLSID* pClassID);

    /**
    *  IPersistStream
    */
    IFACEMETHOD(Load)(IStream* pStm);
    IFACEMETHOD(Save)(IStream* pStm, BOOL fClearDirty);

public:
    SpawnManagerClass();
    SpawnManagerClass(TechnoClass* owner, const AircraftTypeClass* spawns, int spawn_count, int regen_rate, int reload_rate, int spawn_rate, int logic_rate);
    SpawnManagerClass(const NoInitClass& noinit) : SpawnControls(noinit), LogicTimer(noinit), SpawnTimer(noinit) {}
    virtual ~SpawnManagerClass() override;

    /**
     *  AbstractClass
     */
    virtual RTTIType Kind_Of() const override;
    virtual int Size_Of(bool firestorm = false) const override;
    virtual void Compute_CRC(WWCRCEngine& crc) const override;
    virtual void AI() override;

    void Detach_Spawns();
    void Queue_Target(TARGET target);
    void Abandon_Target();
    bool Next_Target();
    void Detach(TARGET target);
    int Active_Count();
    int Docked_Count();
    int Preparing_Count();

    static void Clear_All();

    SpawnManagerClass(const SpawnManagerClass&) = delete;
    SpawnManagerClass& operator= (const SpawnManagerClass&) = delete;

public:
    /**
     *  The Techno that owns this spawn manager.
     */
    TechnoClass* Owner;

    /**
     *  The AircraftType that this spawn manager spawns.
     */
    const AircraftTypeClass* SpawnType;

    /**
     *  How many spawns this spawn manager can have.
     */
    int SpawnCount;

    /**
     *  How long it takes for a spawn to regenerate.
     */
    int RegenRate;

    /**
     *  How long it takes for a spawn to reload.
     */
    int ReloadRate;

    /**
     *  How often does the spawn manager spawn a new spawn.
     */
    int SpawnRate;

    /**
     *  How often does the spawn manager process its logic.
     */
    int LogicRate;

    /**
     *  This vector holds the spawn manager.
     */
    DynamicVectorClass<SpawnControl*> SpawnControls;

    /**
     *  The timer that controls how often the spawn manager should execute its AI function.
     */
    CDTimerClass<FrameTimerClass> LogicTimer;

    /**
     *  The timer that controls how often the spawn manager should spawn a new spawn.
     */
    CDTimerClass<FrameTimerClass> SpawnTimer;

    /**
     *  The spawn manager's target.
     */
    TARGET Target;

    /**
     *  The next target the spawn manager should attack.
     */
    TARGET QueuedTarget;

    /**
     *  The current status of the spawn manager.
     */
    SpawnManagerStatus Status;
};