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
	Idle = 0, // no target or out of range
	Launching = 1, // one launch in progress
	Cooldown = 2 // waiting for launch to complete
};

enum class SpawnControlStatus {
	Idle = 0, // docked, waiting for target
	Takeoff = 1, // missile tilting and launch
	Preparing = 2, // gathering, waiting
	Attacking = 3, // attacking until no ammo
	Returning = 4, // return to carrier
	//Unused_5, // not used
	Reloading = 6, // docked, reloading ammo and health
	Dead = 7 // respawning
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
	SpawnManagerClass(TechnoClass* owner, const AircraftTypeClass* spawns, int spawn_count, int regen_rate, int reload_rate);
	SpawnManagerClass(const NoInitClass& noinit) : SpawnControls(noinit), LogicTimer(noinit), SpawnTimer(noinit) {}
	virtual ~SpawnManagerClass() override;

	/**
	 *  AbstractClass
	 */
	virtual RTTIType Kind_Of() const override;
	virtual int Size_Of(bool firestorm = false) const override;
	virtual void Compute_CRC(WWCRCEngine& crc) const override;
	virtual void AI() override;

	void Manage();
	void Assign_Target(TARGET target);
	void Kamikaze_AI();
	bool Suspend_Target();
	void Detach2(TARGET target);
	int Active_Count();
	int Docked_Count();
	int Missile_Count();

	static void Clear_All();

	SpawnManagerClass(const SpawnManagerClass&) = delete;
	SpawnManagerClass& operator= (const SpawnManagerClass&) = delete;

public:
	TechnoClass* Owner;
	const AircraftTypeClass* SpawnType;
	int SpawnCount;
	int RegenRate;
	int ReloadRate;
	DynamicVectorClass<SpawnControl*> SpawnControls;
	CDTimerClass<FrameTimerClass> LogicTimer;
	CDTimerClass<FrameTimerClass> SpawnTimer;
	AbstractClass* SuspendedTarget;
	AbstractClass* Target;
	SpawnManagerStatus Status;
};