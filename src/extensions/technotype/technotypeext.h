/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TechnoTypeClass class.
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

#include "objecttypeext.h"
#include "technotype.h"
#include "typelist.h"
#include "tibsun_defines.h"


class AircraftTypeClass;
class BSurface;


class TechnoTypeClassExtension : public ObjectTypeClassExtension
{
public:
    /**
     *  IPersistStream
     */
    IFACEMETHOD(Load)(IStream *pStm);
    IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);
    IFACEMETHOD_(LONG, GetSizeMax)(ULARGE_INTEGER *pcbSize);

public:
    TechnoTypeClassExtension(const TechnoTypeClass *this_ptr);
    TechnoTypeClassExtension(const NoInitClass &noinit);
    virtual ~TechnoTypeClassExtension();

    virtual void Detach(AbstractClass * target, bool all = true) override;
    virtual void Object_CRC(CRCEngine &crc) const override;

    virtual TechnoTypeClass *This() const override { return reinterpret_cast<TechnoTypeClass *>(ObjectTypeClassExtension::This()); }
    virtual const TechnoTypeClass *This_Const() const override { return reinterpret_cast<const TechnoTypeClass *>(ObjectTypeClassExtension::This_Const()); }

    virtual bool Read_INI(CCINIClass &ini) override;

public:
    /**
     *  This is the sound effect to play when the unit is cloaking.
     */
    VocType CloakSound;

    /**
     *  This is the sound effect to play when the unit is decloaking.
     */
    VocType UncloakSound;

    /**
     *  Can this object shake the screen when it is destroyed?
     *  (Must meet the rules as specified by Rule->ShakeScreen.
     */
    bool IsShakeScreen;

    /**
     *  Is this object immune to EMP (electromagnetic pulse) effects?
     *  Powered buildings, vehicles and cyborgs are typically disabled
     *  by EMP, unless this is set for them.
     */
    bool IsImmuneToEMP;

    /**
     *  Can this object acquire targets that are within its weapons range
     *  and attack them automatically?
     */
    bool IsCanPassiveAcquire;

    /**
     *  Can this object retaliate when hit by enemy fire?
     */
    bool IsCanRetaliate;

    /**
     *  Can this object be the target of an attack or move command by the computer?
     *  Typically, only objects that take damage or can be destroyed are allow to be
     *  a target. This flag is also subject to "IsLegalTarget" being "true".
     */
    bool IsLegalTargetComputer;

    /**
     *  These values are used to shake the screen when the object is destroyed.
     */
    unsigned ShakePixelYHi;
    unsigned ShakePixelYLo;
    unsigned ShakePixelXHi;
    unsigned ShakePixelXLo;

    /**
     *  The graphic class to switch to when this unit is unloading (e.g., at a refinery).
     */
    const TechnoTypeClass *UnloadingClass;

    /**
     *  The refund value for the unit when it is sold at a Service Depot.
     */
    unsigned SoylentValue;

    /**
     *  This is the sound effect to play when a passenger enters this unit.
     */
    VocType EnterTransportSound;

    /**
     *  This is the sound effect to play when a passenger leaves this unit.
     */
    VocType LeaveTransportSound;

    /**
     *  List of voices to use when giving this object a capture order.
     */
    TypeList<VocType> VoiceCapture;

    /**
     *  List of voices to use when giving this object an enter order (ie, transport, infiltrate building).
     */
    TypeList<VocType> VoiceEnter;

    /**
     *  List of voices to use when giving this object a unload order.
     */
    TypeList<VocType> VoiceDeploy;

    /**
     *  List of voices to use when giving this object a harvest order.
     */
    TypeList<VocType> VoiceHarvest;

    /**
     *  Custom index of a pip to be drawn (like the medic pip).
     */
    int SpecialPipIndex;

    /**
     *  If set to a value greater than 0, this many ammo pips will be drawn, wrapping around incrementing the frame index each time.
     */
    int PipWrap;

    /**
     *  The rate at which this unit animates when it is standing idle (not moving).
     */
    unsigned IdleRate;

    /**
     *  Pointer to the cameo image surface.
     */
    BSurface *CameoImageSurface;

    /**
     *  Should this be considered a base defense when sorting cameos on the sidebar?
     */
    bool IsSortCameoAsBaseDefense;

    /**
     *  Bitfield of houses that can build this type.
     *  If `RequiredHouses != -1`, only these houses can build it.
     */
    long RequiredHouses;

    /**
     *  Bitfield of houses that cannot build this type.
     *  If `ForbiddenHouses != -1`, these houses cannot build it under any circumstances.
     */
    long ForbiddenHouses;

    /**
     *  Description for the extended sidebar tooltip.
     */
    char Description[200];

    /**
     *  If this property is set to true, this object will not be selected when band box selecting
     *  if any objects in the selection have it set to false (e. g., harvesters and MCVs won't be selected with tanks).
     */
    bool IsFilterFromBandBoxSelection;

    /**
     *  How many crew members should exit this object when it is destroyed?
     */
    int CrewCount;

    /**
     *  If this is a spawned unit, is it a missile?
     */
    bool IsMissileSpawn;

    /**
     *  If this is a spawner (rocket launcher or aircraft carrier), this is the type of object it spawns.
     */
    const AircraftTypeClass* Spawns;

    /**
     *  The rate at which this spawner's spawned object reload (how much time it takes before they can attack again).
     */
    int SpawnReloadRate;

    /**
     *  The rate at which the spawner replenishes its destroyed spawned objects.
     */
    int SpawnRegenRate;

    /**
     *  The rate at which the spawner spawns objects.
     */
    int SpawnSpawnRate;

    /**
     *  The rate at which the spawner processes its logic.
     */
    int SpawnLogicRate;

    /**
     *  How many objects can this spawner spawn?
     */
    int SpawnsNumber;

    /**
     *  If it can spawn two missiles at once (like the Boomer submarine), this is an extra offset of the second spawn relative to the first.
     */
    TPoint3D<int> SecondSpawnOffset;

    /**
     *  If the spawn location is to be randomized, by how much?
     */
    int MaxRandomSpawnOffset;

    /**
     *  Should this unit not be scored, and its loss be counted in trackers?
     */
    bool IsDontScore;

    /**
     *  Is this meant to be spawned by something else (a spawner, or perhaps, off-map like a paradrop plane)?
     */
    bool IsSpawned;

    /**
     *  Optional override for the cost that is used for determining the techno's build time.
     */
    int BuildTimeCost;

    /**
     *  Defines how the techno treats targets outside of its zone when scanning for targets.
     */
    TargetZoneScanType TargetZoneScan;

    /**
     *  Does this object need to decloak before firing?
     */
    bool IsDecloakToFire;

private:
    int _JumpjetTurnRate;
    int _JumpjetSpeed;
    double _JumpjetClimb;
    int _JumpjetCruiseHeight;
    double _JumpjetAcceleration;
    double _JumpjetWobblesPerSecond;
    int _JumpjetWobbleDeviation;
    int _JumpjetCloakDetectionRadius;

public:
    int Get_Jumpjet_Turn_Rate() const;
    __declspec(property(get = Get_Jumpjet_Turn_Rate)) int JumpjetTurnRate;

    int Get_Jumpjet_Speed() const;
    __declspec(property(get = Get_Jumpjet_Speed)) int JumpjetSpeed;

    double Get_Jumpjet_Climb() const;
    __declspec(property(get = Get_Jumpjet_Climb)) double JumpjetClimb;

    int Get_Jumpjet_Cruise_Height() const;
    __declspec(property(get = Get_Jumpjet_Cruise_Height)) int JumpjetCruiseHeight;

    double Get_Jumpjet_Acceleration() const;
    __declspec(property(get = Get_Jumpjet_Acceleration)) double JumpjetAcceleration;

    double Get_Jumpjet_Wobbles_Per_Second() const;
    __declspec(property(get = Get_Jumpjet_Wobbles_Per_Second)) double JumpjetWobblesPerSecond;

    int Get_Jumpjet_Wobble_Deviation() const;
    __declspec(property(get = Get_Jumpjet_Wobble_Deviation)) int JumpjetWobbleDeviation;

    int Get_Jumpjet_Cloak_Detection_Radius() const;
    __declspec(property(get = Get_Jumpjet_Cloak_Detection_Radius)) int JumpjetCloakDetectionRadius;
};
