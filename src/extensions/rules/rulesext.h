/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended RulesClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "extension.h"
#include "rules.h"
#include "tibsun_defines.h"


class CCINIClass;


class RulesClassExtension final : public GlobalExtensionClass<RulesClass>
{
public:
    IFACEMETHOD(Load)(IStream* pStm);
    IFACEMETHOD(Save)(IStream* pStm, BOOL fClearDirty);

public:
    RulesClassExtension(const RulesClass* this_ptr);
    RulesClassExtension(const NoInitClass& noinit);
    virtual ~RulesClassExtension();

    virtual int Get_Object_Size() const override;
    virtual void Object_CRC(CRCEngine& crc) const override;

    virtual const char* Name() const override { return "Rule"; }
    virtual const char* Full_Name() const override { return "Rule"; }

    void Process(CCINIClass& ini);
    void Initialize(CCINIClass& ini);

    bool Objects(CCINIClass& ini);

    bool General(CCINIClass& ini);
    bool MPlayer(CCINIClass& ini);
    bool AudioVisual(CCINIClass& ini);
    bool CombatDamage(CCINIClass& ini);
    bool AI(CCINIClass& ini);
    bool Weapons(CCINIClass& ini);
    bool Armors(CCINIClass& ini);
    bool Rockets(CCINIClass& ini);
    bool Tiberiums(CCINIClass& ini);
    bool PrerequisiteGroups(CCINIClass& ini);

    static bool Set_Voxel_Light_Angle(float azimuth, float elevation, float offset);

private:
    void Check();
    void Fixups(CCINIClass& ini);

public:
    /**
     *  Should the MCV unit auto deploy on game start?
     */
    bool IsMPAutoDeployMCV;

    /**
     *  Are construction yards pre-placed on the map rather than a MCV given to the player?
     */
    bool IsMPPrePlacedConYards;

    /**
     *  Can players build their own structures adjacent to structures owned by their allies?
     */
    bool IsBuildOffAlly;

    /**
     *  Should active super weapons show their recharge timer display
     *  on the tactical view?
     */
    bool IsShowSuperWeaponTimers;

    /**
     *  Defines the strength of ice. Higher values make ice less likely
     *  to break from a shot.
     */
    int IceStrength;

    /**
     *  Storage pip used for weeds.
     */
    int WeedPipIndex;

    /**
     *  Customizable maximum counts for drawing different pips.
     */
    TypeList<int> MaxPips;

    /**
     *  When looking for refineries, harvesters will prefer a distant free
     *  refinery over a closer occupied refinery if the refineries' distance
     *  difference in cells is less than this.
     */
    int MaxFreeRefineryDistanceBias;

    /**
     *  Should prerequisites be rechecked when buildings are lost, making the player lose access to units/buildings?
     */
    bool IsRecheckPrerequisites;

    /**
     *  Should the game assume there is more than one MCV (that factions don't share their MCV?)
     */
    bool IsMultiMCV;

    /**
     *  The distance in cells the computer player can place their Naval Yard from their Construction Yard.
     */
    int AINavalYardAdjacency;

    /**
     *  Should the AI automatically repair buildings built as Base Nodes?
     */
    bool IsAIRepairBaseNodes;

    /**
     *  The "double penalty" or "half penalty". Multiply this by the power
     *  units you are short of to get the actual penalty to the build speed.
     */
    float LowPowerPenaltyModifier;

    /**
     *  The maximum number of factories that can be considered when calculating
     *  the multiple factory bonus on an object's build time.
     */
    int MultipleFactoryCap;

    /**
     *  Horizontal direction of the light source.
     */
    float VoxelLightAzimuth;

    /**
     *  Vertical angle of the light source.
     */
    float VoxelLightElevation;

    /**
     *  How much the shadow is offset from the unit.
     */
    float VoxelShadowOffset;

    /**
     *  Determines whether the Tiberium storage logic is enabled.
     */
    bool IsTiberiumStorage;

    /**
     *  Sounds played when a unit is promoted.
     */
    VocType UpgradeVeteranSound;
    VocType UpgradeEliteSound;

    /**
     *  EVA announcement when a unit is promoted.
     */
    VoxType VoxUnitPromoted;

    /**
     *  The number of frames that a newly elite unit will flash for.
     */
    int EliteFlashTimer;

    /**
     *  Controls for beacons.
     */
    bool IsBeaconsEnabled;
    bool IsSPBeacons;
    int MaxBeacons;
    VocType PlaceBeaconSound;
    VoxType PlaceBeaconVoice;
    VoxType DetectBeaconVoice;

    /**
     *  Defines the game-wide cap (in percentages) that technos can self-heal.
     *  This is the default used by technos that don't have this key explicitly specified for them.
     */
    double SelfHealingCap;

    /**
     *  Defines the game-wide rate (in minutes) that technos will self-heal.
     *  This is the default used by technos that don't have this key explicitly specified for them.
     */
    double SelfHealingRate;

    /**
     *  Is LandType Beach considered as "requires crushing" for passability purposes, as opposed to water?
     */
    bool IsBeachIsCrush;

    /**
     *  Defines for how many frames buildings do not get flames spawned on them on
     *  damage state change after once catching fire.
     */
    int BuildingFlameSpawnBlockFrames;

    /**
     *  List of buildings that enable the AI to use the Iron Curtain.
     */
    TypeList<BuildingTypeClass*> IronCurtains;

    /**
     *  Duration of the Iron Curtain effect in frames.
     */
    int IronCurtainDuration;

    /**
     *  Recharge time of a house's Iron Curtain in frames.
     */
    int IronCurtainRechargeTime;

    /**
     *  Flash rate of the Iron Curtain pulse effect.
     */
    int IronCurtainFlashRate;

    /**
     *  Intensity multiplier of the Iron Curtain pulse effect.
     */
    int IronCurtainFlashIntensityMultiplier;

    /**
     *  Brightness modifier table for the Iron Curtain pulse effect.
     */
    TypeList<int> IronCurtainPulseTable;

    VocType IronCurtainSound;

    /**
     *  Distance to consider "close enough" for TEVENT_NEAR_WAYPOINT.
     */
    int ComesNearWaypointDistance;

    /**
     *  Do AI-controlled units ignore disguise and automatically target disguised enemy units?
     */
    bool IsAIDetectDisguise;

    /**
     *  Determines how many harvesters the AI builds for each refinery on different difficulty levels.
     */
    TypeList<int> AIHarvestersPerRefinery;

    /**
     *  Determines whether the AI is limited to one harvester in singleplayer scenarios, like in original Tiberian Sun.
     */
    bool IsAIOneHarvesterInSingleplayer;

    /**
     *  Determines the wrench shape frame that should be used while repairs are paused.
     */
    int PausedRepairsFrame;

    /**
     *  Determines the distance, in leptons, that an escorting unit (Area Guarding unit assigned to another unit) can be separated from its guard target
     *  before it moves again to its guard target's position.
     */
    int EscortRange;

    /**
     *  Determines the distance, in leptons, that an escorting unit (Area Guarding unit assigned to another unit) will keep engaging its current target
     *  before abandoning it and going back to escort its guard target.
     */
    int AbandonTargetEscortRange;

	/*
    * The armor type used by bridges for damage calculation.
    */
    ArmorType BridgeArmor;
};
