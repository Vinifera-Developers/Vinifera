/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TechnoTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "extension.h"
#include "objecttypeext.h"
#include "techno.h"
#include "technotype.h"
#include "tibsun_defines.h"
#include "tibsun_functions.h"
#include "typelist.h"


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

    virtual void Object_CRC(CRCEngine &crc) const override;

    virtual TechnoTypeClass *This() const override { return reinterpret_cast<TechnoTypeClass *>(ObjectTypeClassExtension::This()); }
    virtual const TechnoTypeClass *This_Const() const override { return reinterpret_cast<const TechnoTypeClass *>(ObjectTypeClassExtension::This_Const()); }

    virtual bool Read_INI(CCINIClass &ini) override;

    static ProductionFlags Get_Production_Flags(RTTIType type, int id);
    static ProductionFlags Get_Production_Flags(const TechnoClass* techno) { return Get_Production_Flags(techno->TClass); }
    static ProductionFlags Get_Production_Flags(const TechnoTypeClass* ttype) { return Get_Production_Flags(Extension::Fetch(ttype)); }
    static ProductionFlags Get_Production_Flags(const TechnoTypeClassExtension* ttype_ext);

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
    const UnitTypeClass *UnloadingClass;

    /**
     *  The refund value for the unit when it is sold at a Service Depot.
     */
    int SoylentValue;

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
     *  Defines how the techno treats targets outside of its zone when scanning for targets.
     */
    TargetZoneScanType TargetZoneScan;

    /**
     *  Defines the health cap (in precentages) that this techno can self-heal up to
     */
    float SelfHealingCap;

    /**
     *  Defines the rate (in minutes) that this techno will self-heal
     */
    float SelfHealingRate;

    /**
     *  Define the amount of strength regenerated whenever this techno self-heals
     */
    int SelfHealingStep;

    /**
     *  Does this object need to decloak before firing?
     */
    bool IsDecloakToFire;

    /**
     *  Determines how far away, in leptons, an Area Guarding unit wait to its guard target before moving towards it again.
     */
    int EscortRange;

    /**
     *  Determines how far away, in leptons, an Area Guarding unit will keep engaging its target unit before abandoning it
     *  and going back to escort its guard target.
     */
    int AbandonTargetEscortRange;

private:

    /**
     *  These are backing fields for properties below.
     */
    int _JumpjetTurnRate;
    int _JumpjetSpeed;
    double _JumpjetClimb;
    int _JumpjetCruiseHeight;
    double _JumpjetAcceleration;
    double _JumpjetWobblesPerSecond;
    int _JumpjetWobbleDeviation;
    int _JumpjetCloakDetectionRadius;

public:

    /**
     *  Maximum turning rate of the jumpjet unit.
     */
    int Get_Jumpjet_Turn_Rate() const;
    __declspec(property(get = Get_Jumpjet_Turn_Rate)) int JumpjetTurnRate;

    /**
     *  Forward speed of the jumpjet unit.
     */
    int Get_Jumpjet_Speed() const;
    __declspec(property(get = Get_Jumpjet_Speed)) int JumpjetSpeed;

    /**
     *  Vertical climb rate of the jumpjet unit.
     */
    double Get_Jumpjet_Climb() const;
    __declspec(property(get = Get_Jumpjet_Climb)) double JumpjetClimb;

    /**
     *  Desired cruising height of the jumpjet unit.
     */
    int Get_Jumpjet_Cruise_Height() const;
    __declspec(property(get = Get_Jumpjet_Cruise_Height)) int JumpjetCruiseHeight;

    /**
     *  Acceleration of the jumpjet unit when gaining speed.
     */
    double Get_Jumpjet_Acceleration() const;
    __declspec(property(get = Get_Jumpjet_Acceleration)) double JumpjetAcceleration;

    /**
     *  Frequency of wobble oscillation per second for jumpjets.
     */
    double Get_Jumpjet_Wobbles_Per_Second() const;
    __declspec(property(get = Get_Jumpjet_Wobbles_Per_Second)) double JumpjetWobblesPerSecond;

    /**
     *  Maximum wobble deviation (in leptons) for jumpjet movement.
     */
    int Get_Jumpjet_Wobble_Deviation() const;
    __declspec(property(get = Get_Jumpjet_Wobble_Deviation)) int JumpjetWobbleDeviation;

    /**
     *  Radius at which the jumpjet unit can detect cloaked objects.
     */
    int Get_Jumpjet_Cloak_Detection_Radius() const;
    __declspec(property(get = Get_Jumpjet_Cloak_Detection_Radius)) int JumpjetCloakDetectionRadius;

    /**
     *  Whether the jumpjet unit doesn't wobble.
     */
    bool JumpjetNoWobbles;

    /**
     *  If this techno "naval"? For buildings, this usually means this is a naval yard,
     *  and for units - that this is a ship.
     */
    bool IsNaval;

    /**
     *  A list of factories that can produce this unit.
     */
    TypeList<BuildingTypeClass*> BuiltAt;
    
    /**
     *  This is an individual control of the build time for this object.
     */
    float BuildTimeMultiplier;

    /**
     *  Can this object pick up targets within its range automatically (opportunity fire)?
     */
    bool IsOpportunityFire;

    /**
     *  The wake graphic to show as the object moves across water.
     */
    const AnimTypeClass* WakeAnim;

    /**
     *  The rate at which this object creates the wake animation while moving.
     */
    int WakeAnimRate;

    /**
     *  The wake graphic to show when the object is staying still on water.
     */
    const AnimTypeClass* IdleWakeAnim;

    /**
     *  Should this unit not spawn wakes when it's cloaked? Usually useful for submarines.
     */
    bool IsHideWakeWhenCloaked;

    /**
     *  Specifies whether this unit can see through the disguise of disguised enemy units.
     */
    bool IsDetectDisguise;

    /**
     *  Specifies whether the AI should use the Iron Curtain to protect this object.
     */
    bool IronCurtainPriorityTarget;

    /**
     *  List of animations to be used as the explosion when scrap explosions are turned on.
     */
    TypeList<AnimTypeClass*> ScrapExplosion;

    /*
     *  Specifies the sight range that should be used when the techno is veteran.
     *  Falls back to vanilla SightRange when not provided.
     *  Used as a fall back if EliteSightRange is not provided.
     */
    int VeteranSightRange;

    /*
     *  Specifies the sight range that should be used when the techno is elite.
     *  Falls back to VeteranSightRange when not provided.
     */
    int EliteSightRange;
};
