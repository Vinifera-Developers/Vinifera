/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  SpawnManagerClass reimplementation from YR.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "detach_listener.h"
#include "foot.h"
#include "ttimer.h"
#include "vinifera_defines.h"

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


class DECLSPEC_UUID(UUID_SPAWN_MANAGER)
    SpawnManagerClass : public AbstractClass,
                        public Vinifera::Detach::Listener<TechnoClass>,
                        public Vinifera::Detach::Listener<AbstractClass>
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
    SpawnManagerClass(const NoInitClass& noinit) :
        Vinifera::Detach::Listener<TechnoClass>(noinit),
        Vinifera::Detach::Listener<AbstractClass>(noinit),
        SpawnControls(noinit),
        LogicTimer(noinit),
        SpawnTimer(noinit) {}
    virtual ~SpawnManagerClass() override;

    /**
     *  AbstractClass
     */
    virtual RTTIType Fetch_RTTI() const override;
    virtual int Get_Object_Size(bool firestorm = false) const override;
    virtual void Object_CRC(CRCEngine& crc) const override;
    virtual void AI() override;

    void Detach_Spawns();
    void Queue_Target(AbstractClass * target);
    void Abandon_Target();
    bool Next_Target();

    /**
     *  Detach listener callbacks. On_Detach(TechnoClass*) covers Owner and
     *  Spawnee references; On_Detach(AbstractClass*) covers Target/QueuedTarget.
     */
    void On_Detach(TechnoClass *target, bool all) override;
    void On_Detach(AbstractClass *target, bool all) override;

    int Active_Count();
    int Docked_Count();
    int Preparing_Count();

private:
    void Detach_Spawnee(AircraftClass *spawnee);

public:

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
    AbstractClass * Target;

    /**
     *  The next target the spawn manager should attack.
     */
    AbstractClass * QueuedTarget;

    /**
     *  The current status of the spawn manager.
     */
    SpawnManagerStatus Status;
};