/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended RulesClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "rulesext.h"

#include "addon.h"
#include "aircrafttypeext.h"
#include "animtypeext.h"
#include "armortype.h"
#include "audio_util.h"
#include "audio_theme.h"
#include "asserthandler.h"
#include "buildingtype.h"
#include "buildingtypeext.h"
#include "bullettypeext.h"
#include "ccini.h"
#include "debughandler.h"
#include "extension.h"
#include "extension_globals.h"
#include "findmake.h"
#include "housetype.h"
#include "housetypeext.h"
#include "infantrytypeext.h"
#include "mission.h"
#include "noinit.h"
#include "overlaytypeext.h"
#include "particlesystypeext.h"
#include "particletypeext.h"
#include "prerequisitegroup.h"
#include "rockettype.h"
#include "rules.h"
#include "side.h"
#include "sideext.h"
#include "smudgetypeext.h"
#include "supertypeext.h"
#include "terraintypeext.h"
#include "tiberium.h"
#include "tiberiumext.h"
#include "unittypeext.h"
#include "verses.h"
#include "vinifera_saveload.h"
#include "voxelanimtypeext.h"
#include "voxelinit.h"
#include "warheadtypeext.h"
#include "weapontype.h"
#include "weapontypeext.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
RulesClassExtension::RulesClassExtension(const RulesClass* this_ptr) :
    GlobalExtensionClass(this_ptr),
    IsMPAutoDeployMCV(false),
    IsMPPrePlacedConYards(false),
    IsBuildOffAlly(true),
    IsShowSuperWeaponTimers(true),
    IceStrength(0),
    WeedPipIndex(1),
    MaxFreeRefineryDistanceBias(16),
    IsRecheckPrerequisites(false),
    IsMultiMCV(false),
    AINavalYardAdjacency(20),
    IsAIRepairBaseNodes(false),
    LowPowerPenaltyModifier(1.0f),
    MultipleFactoryCap(0),
    VoxelLightAzimuth(0),
    VoxelLightElevation(DEG_TO_RAD(45)),
    VoxelShadowOffset(6),
    IsTiberiumStorage(true),
    UpgradeVeteranSound(VOC_NONE),
    UpgradeEliteSound(VOC_NONE),
    VoxUnitPromoted(VOX_NONE),
    EliteFlashTimer(0),
    IsBeaconsEnabled(false),
    IsSPBeacons(false),
    MaxBeacons(-1),
    PlaceBeaconSound(VOC_NONE),
    PlaceBeaconVoice(VOX_NONE),
    DetectBeaconVoice(VOX_NONE),
    SelfHealingCap(-1),
    SelfHealingRate(-1),
    IsBeachIsCrush(false),
    BuildingFlameSpawnBlockFrames(0),
    IronCurtainDuration(675),
    IronCurtainRechargeTime(9900),
    IronCurtainFlashRate(8),
    IronCurtainFlashIntensityMultiplier(50),
    IronCurtainSound(VOC_NONE),
    ComesNearWaypointDistance(CELL_LEPTON_W * 5),
    IsAIDetectDisguise(true),
    IsAIOneHarvesterInSingleplayer(true),
    PausedRepairsFrame(6),
    EscortRange(-1),
    AbandonTargetEscortRange(-1),
    BaseUnit(),
    Diff(),
    PlayerNormal(),
    IsHasPlayerNormal(false)
{
    /**
     *  Due to the changes made when addressing issues #632, 633, and 635, we
     *  need change the default engineer capture values. These values are from
     *  Red Alert 1, and they match the expected hardcoded behavior of the
     *  Multi Engineer logic in the release version of Tiberian Sun.
     * 
     *  Fixing the default values here ensures Multi-Engineer works in Tiberian Sun
     *  without manually fixing up the ini data (which is required for Firestorm).
     */
    This()->EngineerDamage = 1.0f / 3;                    // Amount of damage an engineer does.
    This()->EngineerCaptureLevel = This()->ConditionRed;  // Building damage level before engineer can capture.

    MaxPips = TypeList<int>(5);
    MaxPips.Add(5);     // PIPSCALE_AMMO
    MaxPips.Add(5);     // PIPSCALE_TIBERIUM
    MaxPips.Add(5);     // PIPSCALE_PASSENGERS
    MaxPips.Add(10);    // PIPSCALE_POWER
    MaxPips.Add(8);     // PIPSCALE_CHARGE

    IronCurtains = TypeList<BuildingTypeClass*>(0);

    IronCurtainPulseTable = TypeList<int>(8);
    IronCurtainPulseTable.Add(-16);
    IronCurtainPulseTable.Add(-15);
    IronCurtainPulseTable.Add(-14);
    IronCurtainPulseTable.Add(-13);
    IronCurtainPulseTable.Add(-12);
    IronCurtainPulseTable.Add(-13);
    IronCurtainPulseTable.Add(-14);
    IronCurtainPulseTable.Add(-15);

    AIHarvestersPerRefinery = TypeList<int>(3);
    AIHarvestersPerRefinery.Add(2);
    AIHarvestersPerRefinery.Add(2);
    AIHarvestersPerRefinery.Add(1);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
RulesClassExtension::RulesClassExtension(const NoInitClass &noinit) :
    GlobalExtensionClass(noinit),
    MaxPips(noinit),
    IronCurtains(noinit),
    IronCurtainPulseTable(noinit),
    AIHarvestersPerRefinery(noinit),
    BaseUnit(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
RulesClassExtension::~RulesClassExtension()
{
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT RulesClassExtension::Load(IStream *pStm)
{
    MaxPips.Clear();
    IronCurtains.Clear();
    IronCurtainPulseTable.Clear();
    AIHarvestersPerRefinery.Clear();
    BaseUnit.Clear();

    HRESULT hr = GlobalExtensionClass::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) RulesClassExtension(NoInitClass());

    MaxPips.Load_Self(pStm);
    IronCurtains.Load_Self(pStm);
    IronCurtainPulseTable.Load_Self(pStm);
    AIHarvestersPerRefinery.Load_Self(pStm);
    BaseUnit.Load_Self(pStm);

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(BaseUnit, "BaseUnit");
    
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(IronCurtains, "IronCurtains");

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT RulesClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = GlobalExtensionClass::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    MaxPips.Save_Self(pStm);
    IronCurtains.Save_Self(pStm);
    IronCurtainPulseTable.Save_Self(pStm);
    AIHarvestersPerRefinery.Save_Self(pStm);
    BaseUnit.Save_Self(pStm);

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int RulesClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Object_CRC(CRCEngine &crc) const
{
    crc(IsMPAutoDeployMCV);
    crc(IsMPPrePlacedConYards);
    crc(IsBuildOffAlly);
    crc(IsShowSuperWeaponTimers);
    crc(IceStrength);
    crc(MaxFreeRefineryDistanceBias);
    crc(IsRecheckPrerequisites);
    crc(IsMultiMCV);
    crc(AINavalYardAdjacency);
    crc(IsAIRepairBaseNodes);
    crc(BuildingFlameSpawnBlockFrames);
    crc(IronCurtainDuration);
    crc(IronCurtainRechargeTime);
    crc(IronCurtains.Count());
    crc(ComesNearWaypointDistance);
    crc(IsAIDetectDisguise);
    crc(AIHarvestersPerRefinery.Count());
    crc(IsAIOneHarvesterInSingleplayer);
    crc(PausedRepairsFrame);
    crc(EscortRange);
    crc(AbandonTargetEscortRange);
    crc(BaseUnit.Count());
}


/**
 *  Fetch the bulk of the rule data from the control file.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Process(CCINIClass &ini)
{
    /**
     *  This function replaces the original rules process, so we need to duplicate
     *  the its behaviour here first.
     */

    This()->Colors(ini);
    This()->Houses(ini);
    This()->Sides(ini);
    This()->Overlays(ini);

    PrerequisiteGroups(ini);

    /**
     *  #issue-117
     * 
     *  Add reading of Weapons list from RULES.INI. This needs to be done before
     *  all weapon
     * 
     *  @author: CCHyper
     */
    Weapons(ini);

    /**
     *  Read the new ArmorTypes. This needs to happen before Technos and Warheads are read.
     *
     *  @author: ZivDero
     */
    Armors(ini);

    /**
     *  Read the new RocketTypes.
     *
     *  @author: ZivDero
     */
    Rockets(ini);

    This()->SuperWeapons(ini);
    This()->Warheads(ini);
    This()->Smudges(ini);
    This()->Terrains(ini);
    This()->Buildings(ini);
    This()->Vehicles(ini);
    This()->Aircraft(ini);
    This()->Infantry(ini);
    This()->Animations(ini);
    This()->VoxelAnims(ini);
    This()->Particles(ini);
    This()->ParticleSystems(ini);

    /**
     *  Read Tiberiums like all other types, instead of handling them separately.
     *
     *  @author: ZivDero
     */
    Tiberiums(ini);

    This()->JumpjetControls(ini);
    This()->MPlayer(ini);
    This()->AI(ini);
    This()->Powerups(ini);
    This()->Land_Types(ini);
    This()->IQ(ini);
    This()->General(ini);

    for (int index = 0; index < BuildingTypes.Count(); ++index) {

        BuildingTypeClass *btype = BuildingTypes[index];

        /**
         *  This is a edge case issue we exposed in the original RULES.INI where the
         *  Nod Radar (NARADR) has "IsNewTheater" set to false, and as a result, the
         *  new theater system ends up making this build show in the wrong drawing
         *  palette. To fix this, just before Read_INI() is called on all the
         *  BuildingTypes (see RulesClass::Objects()), we make sure NARADR has the
         *  default value of "IsNewTheater" set to true.
         */
        if (btype->IniName == "NARADR" && btype->IsNewTheater == false) {
            DEBUG_WARNING("Rules: Changing the default value of IsNewTheater for NARADR to 'true'!\n");
            DEBUG_WARNING("Rules: Please consider changing NewTheater on NARADR to 'yes'!\n");
            btype->IsNewTheater = true;
        }
    }

    /**
     *  Now that we know how many armors and warheads we have, resize the Verses arrays accordingly
     *  before reading the actual values.
     */
    Verses::Resize();

    /**
     *  Process the objects (extension classes).
     *  This includes all vanilla objects.
     */
    Objects(ini);

    This()->CrateRules(ini);
    This()->CombatDamage(ini);
    This()->AudioVisual(ini);
    This()->SpecialWeapons(ini);

    /**
     *  Now in case there are yet new warheads, resize the Verses arrays again.
     */
    Verses::Resize();

    /**
     *  Note: The game re-reads INI values for warheads at the end of
     *  SpecialWeapons(), so we do the same here for our extensions.
     */
    for (int i = 0; i < WarheadTypeExtensions.Count(); i++) {
        WarheadTypeExtensions[i]->Read_INI(ini);
    }

    //TiberiumClass::Process(ini);

    /**
     *  Process the rules extension.
     * 
     *  #NOTE: These must be performed last!
     */
    General(ini);
    Difficulty(ini);
    MPlayer(ini);
    AudioVisual(ini);
    CombatDamage(ini);
    AI(ini);

    /**
     *  Run some checks to ensure certain values are as expected.
     */
    Check();

    /**
     *  Fixup various inconsistencies in the original INI files.
     */
    Fixups(ini);

    /**
     *  Read the theme data from the INI.
     */
    AudioTheme.Init_Themes(ini);
}


/**
 *  Process and initialise rule data from the control file.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Initialize(CCINIClass &ini)
{
    Verses::Clear();
    ArmorTypeClass::One_Time();
    PrerequisiteGroupClass::One_Time();
}


/**
 *  Fetch all the object characteristic values.
 *  
 *  @author: CCHyper, ZivDero
 */
bool RulesClassExtension::Objects(CCINIClass &ini)
{
    /**
     *  Fetch the game object and extension values from the rules file.
     */

    DEBUG_INFO("Rules: Processing HouseTypes (Count: {})...\n", HouseTypes.Count());
    for (int index = 0; index < HouseTypes.Count(); ++index) {
        HouseTypes[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing HouseTypeExtensions (Count: {})...\n", HouseTypeExtensions.Count());
    for (int index = 0; index < HouseTypeExtensions.Count(); ++index) {
        HouseTypeExtensions[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing SuperWeaponTypes (Count: {})...\n", SuperWeaponTypes.Count());
    for (int index = 0; index < SuperWeaponTypes.Count(); ++index) {
        SuperWeaponTypes[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing SuperWeaponTypeExtensions (Count: {})...\n", SuperWeaponTypeExtensions.Count());
    for (int index = 0; index < SuperWeaponTypeExtensions.Count(); ++index) {
        SuperWeaponTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing AnimTypes (Count: {})...\n", AnimTypes.Count());
    for (int index = 0; index < AnimTypes.Count(); ++index) {
        AnimTypes[index]->Read_INI(ArtINI); // Animations are loaded explicitly from ArtINI.
    }
    
    DEBUG_INFO("Rules: Processing AnimTypeExtensions (Count: {})...\n", AnimTypeExtensions.Count());
    for (int index = 0; index < AnimTypeExtensions.Count(); ++index) {
        AnimTypeExtensions[index]->Read_INI(ArtINI); // Animations are loaded explicitly from ArtINI.
    }
    
    DEBUG_INFO("Rules: Processing BuildingTypes (Count: {})...\n", BuildingTypes.Count());
    for (int index = 0; index < BuildingTypes.Count(); ++index) {
        BuildingTypes[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing BuildingTypeExtensions (Count: {})...\n", BuildingTypeExtensions.Count());
    for (int index = 0; index < BuildingTypeExtensions.Count(); ++index) {
        BuildingTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing AircraftTypes (Count: {})...\n", AircraftTypes.Count());
    for (int index = 0; index < AircraftTypes.Count(); ++index) {
        AircraftTypes[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing AircraftTypeExtensions (Count: {})...\n", AircraftTypeExtensions.Count());
    for (int index = 0; index < AircraftTypeExtensions.Count(); ++index) {
        AircraftTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing UnitTypes (Count: {})...\n", UnitTypes.Count());
    for (int index = 0; index < UnitTypes.Count(); ++index) {
        UnitTypes[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing UnitTypeExtensions (Count: {})...\n", UnitTypeExtensions.Count());
    for (int index = 0; index < UnitTypeExtensions.Count(); ++index) {
        UnitTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing InfantryTypes (Count: {})...\n", InfantryTypes.Count());
    for (int index = 0; index < InfantryTypes.Count(); ++index) {
        InfantryTypes[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing InfantryTypeExtensions (Count: {})...\n", InfantryTypeExtensions.Count());
    for (int index = 0; index < InfantryTypeExtensions.Count(); ++index) {
        InfantryTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing WeaponTypes (Count: {})...\n", ::Weapons.Count());
    for (int index = 0; index < ::Weapons.Count(); ++index) {
        ::Weapons[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing WeaponTypeExtensions (Count: {})...\n", WeaponTypeExtensions.Count());
    for (int index = 0; index < WeaponTypeExtensions.Count(); ++index) {
        WeaponTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing BulletTypes (Count: {})...\n", BulletTypes.Count());
    for (int index = 0; index < BulletTypes.Count(); ++index) {
        BulletTypes[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing BulletTypeExtensions (Count: {})...\n", BulletTypeExtensions.Count());
    for (int index = 0; index < BulletTypeExtensions.Count(); ++index) {
        BulletTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing WarheadTypes (Count: {})...\n", Warheads.Count());
    for (int index = 0; index < Warheads.Count(); ++index) {
        Warheads[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing WarheadTypeExtensions (Count: {})...\n", WarheadTypeExtensions.Count());
    for (int index = 0; index < WarheadTypeExtensions.Count(); ++index) {
        WarheadTypeExtensions[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Calling WeaponTypeClass::Set_Speed (Count: {})...\n", ::Weapons.Count());
    for (int index = 0; index < ::Weapons.Count(); ++index) {
        ::Weapons[index]->Set_Speed();
    }

    DEBUG_INFO("Rules: Calling BuildingTypeClass::Set_Base_Defense_Values (Count: {})...\n", BuildingTypes.Count());
    for (int index = 0; index < BuildingTypes.Count(); ++index) {
        BuildingTypes[index]->Set_Base_Defense_Values();
    }
    
    DEBUG_INFO("Rules: Processing TerrainTypes (Count: {})...\n", TerrainTypes.Count());
    for (int index = 0; index < TerrainTypes.Count(); ++index) {
        TerrainTypes[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing TerrainTypeExtensions (Count: {})...\n", TerrainTypeExtensions.Count());
    for (int index = 0; index < TerrainTypeExtensions.Count(); ++index) {
        TerrainTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing SmudgeTypes (Count: {})...\n", SmudgeTypes.Count());
    for (int index = 0; index < SmudgeTypes.Count(); ++index) {
        SmudgeTypes[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing SmudgeTypeExtensions (Count: {})...\n", SmudgeTypeExtensions.Count());
    for (int index = 0; index < SmudgeTypeExtensions.Count(); ++index) {
        SmudgeTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing OverlayTypes (Count: {})...\n", OverlayTypes.Count());
    for (int index = 0; index < OverlayTypes.Count(); ++index) {
        OverlayTypes[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing OverlayTypeExtensions (Count: {})...\n", OverlayTypeExtensions.Count());
    for (int index = 0; index < OverlayTypeExtensions.Count(); ++index) {
        OverlayTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing ParticleTypes (Count: {})...\n", ParticleTypes.Count());
    for (int index = 0; index < ParticleTypes.Count(); ++index) {
        ParticleTypes[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing ParticleTypeExtensions (Count: {})...\n", ParticleTypeExtensions.Count());
    for (int index = 0; index < ParticleTypeExtensions.Count(); ++index) {
        ParticleTypeExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing ParticleSystemTypes (Count: {})...\n", ParticleSystemTypes.Count());
    for (int index = 0; index < ParticleSystemTypes.Count(); ++index) {
        ParticleSystemTypes[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing ParticleSystemTypeExtensions (Count: {})...\n", ParticleSystemTypeExtensions.Count());
    for (int index = 0; index < ParticleSystemTypeExtensions.Count(); ++index) {
        ParticleSystemTypeExtensions[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing Tiberiums (Count: {})...\n", ::Tiberiums.Count());
    for (int index = 0; index < ::Tiberiums.Count(); ++index) {
        ::Tiberiums[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing TiberiumExtensions (Count: {})...\n", TiberiumExtensions.Count());
    for (int index = 0; index < TiberiumExtensions.Count(); ++index) {
        TiberiumExtensions[index]->Read_INI(ini);
    }
    
    DEBUG_INFO("Rules: Processing VoxelAnimTypes (Count: {})...\n", VoxelAnimTypes.Count());
    for (int index = 0; index < VoxelAnimTypes.Count(); ++index) {
        VoxelAnimTypes[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing VoxelAnimTypeExtensions (Count: {})...\n", VoxelAnimTypeExtensions.Count());
    for (int index = 0; index < VoxelAnimTypeExtensions.Count(); ++index) {
        VoxelAnimTypeExtensions[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing MissionControlClasses (Count: {})...\n", (int)MISSION_COUNT);
    for (int mission = 0; mission < MISSION_COUNT; mission++) {
        MissionControl[mission].Mission = static_cast<MissionType>(mission);
        MissionControl[mission].Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing SideExtensions (Count: {})...\n", SideExtensions.Count());
    for (int index = 0; index < SideExtensions.Count(); ++index) {
        SideExtensions[index]->Read_INI(ini);
    }

    /**
     *  Fetch new Vinifera object values from the rules file.
     */

    DEBUG_INFO("Rules: Processing ArmorTypes (Count: {})...\n", ArmorTypes.Count());
    for (int index = 0; index < ArmorTypes.Count(); ++index) {
        ArmorTypes[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing RocketTypes (Count: {})...\n", RocketTypes.Count());
    for (int index = 0; index < RocketTypes.Count(); ++index) {
        RocketTypes[index]->Read_INI(ini);
    }

    DEBUG_INFO("Rules: Processing global PrerequisiteGroups...\n");
    PrerequisiteGroupClass::Read_Global_INI(ini);

    DEBUG_INFO("Rules: Processing PrerequisiteGroups (Count: {})...\n", ::PrerequisiteGroups.Count());
    for (int index = 0; index < ::PrerequisiteGroups.Count(); ++index) {
        ::PrerequisiteGroups[index]->Read_INI(ini);
    }

    return true;
}


/**
 *  Process the general main game rules.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::General(CCINIClass &ini)
{
    static char const * const GENERAL = "General";

    if (!ini.Is_Present(GENERAL)) {
        return false;
    }

    /**
     *  #issue-632
     *
     *  "EngineerDamage" was incorrectly loaded with "EngineerCaptureLevel", so
     *  load the value correctly.
     *
     *  @author: CCHyper
     */
    This()->EngineerDamage = ini.Get_Float(GENERAL, "EngineerDamage", This()->EngineerDamage);

    MaxFreeRefineryDistanceBias = ini.Get_Int(GENERAL, "MaxFreeRefineryDistanceBias", MaxFreeRefineryDistanceBias);
    IsRecheckPrerequisites = ini.Get_Bool(GENERAL, "RecheckPrerequisites", IsRecheckPrerequisites);
    IsMultiMCV = ini.Get_Bool(GENERAL, "MultiMCV", IsMultiMCV);
    LowPowerPenaltyModifier = ini.Get_Float(GENERAL, "LowPowerPenaltyModifier", LowPowerPenaltyModifier);
    MultipleFactoryCap = ini.Get_Int(GENERAL, "MultipleFactoryCap", MultipleFactoryCap);
    IsTiberiumStorage = ini.Get_Bool(GENERAL, "TiberiumStorage", IsTiberiumStorage);
    IsBeaconsEnabled = ini.Get_Bool(GENERAL, "BeaconsEnabled", IsBeaconsEnabled);
    IsSPBeacons = ini.Get_Bool(GENERAL, "SPBeacons", IsSPBeacons);
    MaxBeacons = ini.Get_Int(GENERAL, "MaxBeacons", MaxBeacons);
    SelfHealingCap = ini.Get_Float(GENERAL, "SelfHealingCap", SelfHealingCap);    
    SelfHealingRate = ini.Get_Float(GENERAL, "SelfHealingRate", SelfHealingRate);
    PausedRepairsFrame = ini.Get_Int(GENERAL, "PausedRepairsFrame", PausedRepairsFrame);
    EscortRange = ini.Get_Lepton(GENERAL, "EscortRange", EscortRange);
    AbandonTargetEscortRange = ini.Get_Lepton(GENERAL, "AbandonTargetEscortRange", AbandonTargetEscortRange);

    /**
     *  Allow replacing any signle movement zone with a copy of RA2's water MZone.
     */
    MZoneType mzone_water = ini.Get_MZoneType(GENERAL, "WaterMovementZoneOverride", MZONE_NORMAL);
    if (mzone_water != MZONE_NORMAL) {
        int water[7] = {2, 2, 2, 1, 2, 2, 3}; // LAND = NO, CRUSH = NO, BLOCKED = NO, WATER = YES, PARTIALLY_BLOCKED = NO, NO = NO, OUTSIDE = ILLEGAL
        std::copy(std::begin(water), std::end(water), MovementZonePassability[mzone_water]);
    }

    IsBeachIsCrush = ini.Get_Bool(GENERAL, "BeachIsCrush", IsBeachIsCrush);
    ComesNearWaypointDistance = ini.Get_Int(GENERAL, "ComesNearWaypointDistance", ComesNearWaypointDistance);

    IronCurtains = ::TGet_TypeList(ini, GENERAL, "IronCurtains", IronCurtains);
    IronCurtainDuration = ini.Get_Int(GENERAL, "IronCurtainDuration", IronCurtainDuration);

    float icrecharge = ini.Get_Float(GENERAL, "IronCurtainRechargeTime");
    if (icrecharge != 0.0) {
        IronCurtainRechargeTime = icrecharge * 900.0f;
    }

    /**
     *  Reload the BaseUnit entry and store the value in the new class extension.
     *  This allows us to expand the original BaseUnit logic without impacting
     *  the original behaviour of BaseUnit.
     */
    BaseUnit = TGet_TypeList(ini, GENERAL, "BaseUnit", BaseUnit);

    IsHasPlayerNormal = ini.Get_Bool(GENERAL, "HasPlayerNormal", IsHasPlayerNormal);

    return true;
}


/**
 *  Process the audio/visual game settings.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::AudioVisual(CCINIClass &ini)
{
    static char const * const AUDIOVISUAL = "AudioVisual";

    if (!ini.Is_Present(AUDIOVISUAL)) {
        return false;
    }

    IsShowSuperWeaponTimers = ini.Get_Bool(AUDIOVISUAL, "ShowSuperWeaponTimers", IsShowSuperWeaponTimers);
    WeedPipIndex = ini.Get_Int(AUDIOVISUAL, "WeedPipIndex", WeedPipIndex);
    MaxPips = ini.Get_IntList(AUDIOVISUAL, "MaxPips", MaxPips);

    VoxelLightAzimuth = DEG_TO_RADF(ini.Get_Float(AUDIOVISUAL, "VoxelLightAzimuth", RAD_TO_DEGF(VoxelLightAzimuth)));
    VoxelLightElevation = DEG_TO_RADF(ini.Get_Float(AUDIOVISUAL, "VoxelLightElevation", RAD_TO_DEGF(VoxelLightElevation)));
    VoxelShadowOffset = ini.Get_Float(AUDIOVISUAL, "VoxelShadowOffset", VoxelShadowOffset);

    Set_Voxel_Light_Angle(VoxelLightAzimuth, VoxelLightElevation, VoxelShadowOffset);

    UpgradeVeteranSound = ini.Get_VocType(AUDIOVISUAL, "UpgradeVeteranSound", UpgradeVeteranSound);
    UpgradeEliteSound = ini.Get_VocType(AUDIOVISUAL, "UpgradeEliteSound", UpgradeEliteSound);
    VoxUnitPromoted = ini.Get_VoxType(AUDIOVISUAL, "VoxUnitPromoted", VoxUnitPromoted);
    EliteFlashTimer = ini.Get_Int(AUDIOVISUAL, "EliteFlashTimer", EliteFlashTimer);

    PlaceBeaconSound = ini.Get_VocType(AUDIOVISUAL, "PlaceBeaconSound", PlaceBeaconSound);
    PlaceBeaconVoice = ini.Get_VoxType(AUDIOVISUAL, "PlaceBeaconVoice", PlaceBeaconVoice);
    DetectBeaconVoice = ini.Get_VoxType(AUDIOVISUAL, "DetectBeaconVoice", DetectBeaconVoice);

    IronCurtainFlashRate = ini.Get_Int(AUDIOVISUAL, "IronCurtainFlashRate", IronCurtainFlashRate);
    IronCurtainFlashIntensityMultiplier = ini.Get_Int(AUDIOVISUAL, "IronCurtainFlashIntensityMultiplier", IronCurtainFlashIntensityMultiplier);
    IronCurtainPulseTable = ini.Get_IntList(AUDIOVISUAL, "IronCurtainPulseTable", IronCurtainPulseTable);
    IronCurtainSound = ini.Get_VocType(AUDIOVISUAL, "IronCurtainSound", IronCurtainSound);

    return true;
}


/**
 *  Process the combat damage related game settings.
 *
 *  @author: Rampastring
 */
bool RulesClassExtension::CombatDamage(CCINIClass & ini)
{
    static char const * const COMBATDAMAGE = "CombatDamage";

    if (!ini.Is_Present(COMBATDAMAGE)) {
        return false;
    }

    IceStrength = ini.Get_Int(COMBATDAMAGE, "IceStrength", IceStrength);
    BuildingFlameSpawnBlockFrames = ini.Get_Int(COMBATDAMAGE, "BuildingFlameSpawnBlockFrames", BuildingFlameSpawnBlockFrames);

    return true;
}


/**
 *  Process the AI related game settings.
 *
 *  @author: ZivDero
 */
bool RulesClassExtension::AI(CCINIClass& ini)
{
    static char const* const AI = "AI";

    if (!ini.Is_Present(AI)) {
        return false;
    }

    AINavalYardAdjacency = ini.Get_Int(AI, "AINavalYardAdjacency", AINavalYardAdjacency);
    IsAIRepairBaseNodes = ini.Get_Bool(AI, "AIRepairBaseNodes", IsAIRepairBaseNodes);
    IsAIDetectDisguise = ini.Get_Bool(AI, "AIDetectDisguise", IsAIDetectDisguise);
    AIHarvestersPerRefinery = ini.Get_IntList(AI, "HarvestersPerRefinery", AIHarvestersPerRefinery);
    IsAIOneHarvesterInSingleplayer = ini.Get_Bool(AI, "AIOneHarvesterInSingleplayer", IsAIOneHarvesterInSingleplayer);

    return true;
}


/**
 *  Process the general main game rules.
 *  
 *  @author: CCHyper
 */
bool RulesClassExtension::MPlayer(CCINIClass &ini)
{
    static char const * const MPLAYER = "MultiplayerDefaults";

    if (!ini.Is_Present(MPLAYER)) {
        return false;
    }

    IsMPAutoDeployMCV = ini.Get_Bool(MPLAYER, "AutoDeployMCV", IsMPAutoDeployMCV);
    IsMPPrePlacedConYards = ini.Get_Bool(MPLAYER, "PrePlacedConYards", IsMPPrePlacedConYards);
    IsBuildOffAlly = ini.Get_Bool(MPLAYER, "BuildOffAlly", IsBuildOffAlly);

    return true;
}


/**
 *  Fetch all the weapon characteristic values.
 * 
 *  @author: CCHyper
 */
bool RulesClassExtension::Weapons(CCINIClass &ini)
{
    static const char * const WEAPONS = "Weapons";

    char buf[128];
    const WeaponTypeClass *weapontype;

    int counter = ini.Entry_Count(WEAPONS);
    for (int index = 0; index < counter; ++index) {
        const char *entry = ini.Get_Entry(WEAPONS, index);

        /**
         *  Get a weapon entry.
         */
        if (ini.Get_String(WEAPONS, entry, "", buf, sizeof(buf))) {

            /**
             *  Find or create a weapon of the name specified.
             */
            weapontype = WeaponTypeClass::Find_Or_Make(buf);
            if (weapontype) {
                DEV_DEBUG_INFO("Rules: Found WeaponType \"{}\".\n", buf);
            } else {
                DEV_DEBUG_WARNING("Rules: Error processing WeaponType \"{}\"!\n", buf);
            }

        }

    }

    return counter > 0;
}


/**
 *  Fetch all the armor characteristic values.
 *
 *  @author: CCHyper
 */
bool RulesClassExtension::Armors(CCINIClass &ini)
{
    static const char *const ARMORTYPES = "ArmorTypes";

    char buf[128];
    const ArmorTypeClass *armortype;

    int counter = ini.Entry_Count(ARMORTYPES);
    for (int index = 0; index < counter; ++index) {
        const char *entry = ini.Get_Entry(ARMORTYPES, index);

        /**
         *  Get a weapon entry.
         */
        if (ini.Get_String(ARMORTYPES, entry, "", buf, sizeof(buf))) {

            /**
             *  Find or create a weapon of the name specified.
             */
            armortype = ArmorTypeClass::Find_Or_Make(buf);
            if (armortype) {
                DEV_DEBUG_INFO("Rules: Found ArmorType \"{}\".\n", buf);
            } else {
                DEV_DEBUG_WARNING("Rules: Error processing ArmorType \"{}\"!\n", buf);
            }
        }
    }

    return counter > 0;
}


/**
 *  Fetch all the Rocket characteristic values.
 *
 *  @author: ZivDero
 */
bool RulesClassExtension::Rockets(CCINIClass &ini)
{
    static const char *const ROCKETTYPES = "RocketTypes";

    char buf[128];
    const RocketTypeClass *rockettype;

    int counter = ini.Entry_Count(ROCKETTYPES);
    for (int index = 0; index < counter; ++index) {
        const char *entry = ini.Get_Entry(ROCKETTYPES, index);

        /**
         *  Get a rocket entry.
         */
        if (ini.Get_String(ROCKETTYPES, entry, "", buf, sizeof(buf))) {

            /**
             *  Find or create a rocket of the name specified.
             */
            rockettype = RocketTypeClass::Find_Or_Make(buf);
            if (rockettype) {
                DEV_DEBUG_INFO("Rules: Found RocketType \"{}\".\n", buf);
            } else {
                DEV_DEBUG_WARNING("Rules: Error processing RocketType \"{}\"!\n", buf);
            }
        }
    }

    return counter > 0;
}


/**
 *  Reimplemented function to read Tiberiums like all other types,
 *  instead of handling them in a special way.
 *
 *  @author: ZivDero
 */
bool RulesClassExtension::Tiberiums(CCINIClass &ini)
{
    static const char * const TIBERIUMS = "Tiberiums";

    char buf[128];
    const TiberiumClass* tiberium;

    int counter = ini.Entry_Count(TIBERIUMS);
    for (int index = 0; index < counter; ++index) {
        const char *entry = ini.Get_Entry(TIBERIUMS, index);

        /**
         *  Get a Tiberium entry.
         */
        if (ini.Get_String(TIBERIUMS, entry, "", buf, sizeof(buf))) {

            /**
             *  Find or create a weapon of the name specified.
             */
            tiberium = TiberiumClass::Find_Or_Make(buf);
            if (tiberium) {
                DEV_DEBUG_INFO("Rules: Found Tiberium \"{}\".\n", buf);
            } else {
                DEV_DEBUG_WARNING("Rules: Error processing Tiberium \"{}\"!\n", buf);
            }

        }

    }

    return counter > 0;
}


/**
 *  Fetch all prerequisite group values.
 *
 *  @author: ZivDero
 */
bool RulesClassExtension::PrerequisiteGroups(CCINIClass& ini)
{
    static const char* const PREREQUISITE_GROUPS = "PrerequisiteGroups";

    char buf[128];
    const PrerequisiteGroupClass* group;

    int counter = ini.Entry_Count(PREREQUISITE_GROUPS);
    for (int index = 0; index < counter; ++index) {
        const char* entry = ini.Get_Entry(PREREQUISITE_GROUPS, index);

        /**
         *  Get a group entry.
         */
        if (ini.Get_String(PREREQUISITE_GROUPS, entry, "", buf, sizeof(buf)) > 0) {

            /**
             *  Find or create a group of the name specified.
             */
            group = PrerequisiteGroupClass::Find_Or_Make(entry);
            if (group) {
                DEV_DEBUG_INFO("Rules: Found PrerequisiteGroup \"{}\".\n", buf);
            } else {
                DEV_DEBUG_WARNING("Rules: Error processing PrerequisiteGroup \"{}\"!\n", buf);
            }
        }
    }

    return counter > 0;
}


/**
 *  Fetch the various difficulty group settings.
 *
 *  @author: Rampastring
 */
bool RulesClassExtension::Difficulty(CCINIClass& ini)
{
    Difficulty_Get(ini, Diff[DIFF_EASY], "Easy");
    Difficulty_Get(ini, Diff[DIFF_NORMAL], "Normal");
    Difficulty_Get(ini, PlayerNormal, "PlayerNormal");
    Difficulty_Get(ini, Diff[DIFF_HARD], "Difficult");
    Difficulty_Get(ini, Diff[EXT_DIFF_VERY_EASY], "VeryEasy");
    Difficulty_Get(ini, Diff[EXT_DIFF_BRUTALLY_EASY], "BrutallyEasy");
    Difficulty_Get(ini, Diff[EXT_DIFF_EXTREMELY_EASY], "ExtremelyEasy");
    Difficulty_Get(ini, Diff[EXT_DIFF_ULTIMATELY_EASY], "UltimatelyEasy");

    return true;
}


/**
 *  Performs checks on rules data to ensure values are as expected.
 *  
 *  @author: CCHyper
 */
void RulesClassExtension::Check()
{
    ASSERT_PRINT(This()->CreditTicks.Count() == 2, "CreditTicks must contain 2 valid entries!");
}


/**
 *  This function is for fixing up any erroneous rules data in the unmodded Tiberian Sun to
 *  ensure the original game works as expected with any new systems we implement.
 *
 *  @author: CCHyper
 */
void RulesClassExtension::Fixups(CCINIClass &ini)
{
    DEBUG_INFO("Rules::Fixups(enter)\n");

    /**
     *  These are the CRC values for the unmodified ini files, TS2.03EN.
     */
    static const int Unmodified_RuleINI_CRC = 0x9F3ECD2A;
    static const int Unmodified_FSRuleINI_CRC = 0xA0738E22;

    /**
     *  Constant values to change to.
     */
    /*static*/ const float CorrectEngineerDamage = 1.0f / 3;                    // Amount of damage an engineer does.
    /*static*/ const float CorrectEngineerCaptureLevel = This()->ConditionRed;  // Building damage level before engineer can capture.
    /*static*/ const float CorrectWorstLowPowerBuildRateCoefficient = 0.5f;     // Lowest the build rate can get for being low on power.

    /**
     *  Fetch the unique crc values for both rule databases.
     */
    int rule_crc = RuleINI->Get_Unique_ID();
    DEV_DEBUG_INFO("Rules: RuleINI CRC = {:X}\n", rule_crc);

    int fsrule_crc = FSRuleINI.Get_Unique_ID();
    if (Addon_Installed(ADDON_FIRESTORM)) {
        DEV_DEBUG_INFO("Rules: FSRuleINI CRC = {:X}\n", fsrule_crc);
    }

    /**
     *  Check to see if the ini files have been modified.
     */
    bool rule_unmodified = false;
    if (rule_crc == Unmodified_RuleINI_CRC) {
        DEBUG_INFO("Rules: RuleINI is unmodified (version 2.03).\n");
        rule_unmodified = true;
    }
    bool fsrule_unmodified = false;
    if (Addon_Installed(ADDON_FIRESTORM)) {
        if (fsrule_crc == Unmodified_FSRuleINI_CRC) {
            DEBUG_INFO("Rules: FSRuleINI is unmodified (version 2.03).\n");
            fsrule_unmodified = true;
        }
    }

    /**
     *  Detect which unmodified ini file we are currently processing.
     */
    bool is_ruleini = false;
    if (ini.Get_Unique_ID() == Unmodified_RuleINI_CRC) {
        DEV_DEBUG_INFO("Rules: Current INI is RuleINI.\n");
        is_ruleini = true;
    }
    bool is_fsruleini = false;
    if (Addon_Installed(ADDON_FIRESTORM) && ini.Get_Unique_ID() == Unmodified_FSRuleINI_CRC) {
        DEV_DEBUG_INFO("Rules: Current INI is FSRuleINI.\n");
        is_fsruleini = true;
    }

    /**
     *  Fix up the multi engineer values if we have possibly detected the original, unmodified ini databases.
     * 
     *  Match criteria;
     *   - Are we currently processing FSRuleINI?
     *   - EngineerCaptureLevel is "1.0"
     *   - EngineerDamage is "0.0"
     */
    if (is_fsruleini) {

        if (This()->EngineerCaptureLevel == 1.0f && This()->EngineerDamage == 0.0f) {

            DEBUG_WARNING("Rules: EngineerCaptureLevel is '{:.2f}', changing to '{:.2f}'!\n", This()->EngineerDamage, CorrectEngineerCaptureLevel);
            DEBUG_WARNING("Rules: Please consider changing EngineerCaptureLevel to {:.2f}!\n", CorrectEngineerCaptureLevel);
            This()->EngineerCaptureLevel = CorrectEngineerCaptureLevel;

            DEBUG_WARNING("Rules: EngineerDamage is '{:.2f}', changing to '{:.2f}'!\n", This()->EngineerDamage, CorrectEngineerDamage);
            DEBUG_WARNING("Rules: Please consider changing EngineerDamage to {:.2f}!\n", CorrectEngineerDamage);
            This()->EngineerDamage = CorrectEngineerDamage;

        }

    }

    /**
     *  Fix up the WorstLowPowerBuildRateCoefficient value if we have possibly detected the original, unmodified rule ini database.
     *
     *  Match criteria;
     *   - Are we currently processing RuleINI?
     *   - WorstLowPowerBuildRateCoefficient is "0.3"
     * 
     *  We don't need to check BestLowPowerBuildRateCoefficient as the value in the INI matches the original
     *  hardcoded value in TechnoClass::Time_To_Build.
     */
    if (is_ruleini) {

        /**
         *  The loaded value is 0.3, but gets stored as 0.333 (with 3 repeating until infinity), so
         *  we need to use a math utility function to do a "essentually equal" comparison.
         */
        if (WWMath::EssentiallyEqual(This()->WorstLowPowerBuildRateCoefficient, 0.3)) {
            DEBUG_WARNING("Rules: WorstLowPowerBuildRateCoefficient is '{:.2f}', changing to '{:.2f}'!\n", This()->WorstLowPowerBuildRateCoefficient, CorrectWorstLowPowerBuildRateCoefficient);
            DEBUG_WARNING("Rules: Please consider changing WorstLowPowerBuildRateCoefficient to {:.2f}!\n", CorrectWorstLowPowerBuildRateCoefficient);
            This()->WorstLowPowerBuildRateCoefficient = CorrectWorstLowPowerBuildRateCoefficient;
        }
    }

    /**
     *  Workaround because NOD has Side=GDI and Prefix=B in unmodded Tiberian Sun.
     *
     *  Match criteria;
     *   - Are we currently processing one of the unmodified rule INI's?
     */
    if (rule_unmodified || fsrule_unmodified) {

        /**
         *  Ensure at least two HouseTypes are defined before performing this fixup case.
         */
        HouseTypeClass *housetype = HouseTypes.Count() >= 2 ? HouseTypes[HOUSE_NOD] : nullptr;
        if (housetype && Sides.Count() >= 2) {

            /**
             *  #issue-903
             *
             *  Workaround because NOD has Side=GDI in unmodded Tiberian Sun.
             *
             *  Match criteria;
             *   - The HouseType's name is "Nod"
             *   - HouseType "Nod" is index 1
             *   - The HouseType's Side is GDI (index 0)
             *   - The HouseType's Side name is "GDI"
             *   - Side 1 name is "Nod"
             */
            if (housetype->IniName == "Nod"
                && housetype->Fetch_Heap_ID() == HOUSE_NOD
                && housetype->Side == SIDE_GDI
                && Sides[housetype->Side]->IniName == "GDI"
                && Sides[SIDE_NOD]->IniName == "Nod") {

                DEBUG_WARNING("Rules: House \"{}\" ({}) has \"Side=GDI\", changing Side to \"Nod\"!\n",
                    housetype->Name(), housetype->Fetch_Heap_ID());

                /**
                 *  We are pretty sure this house is NOD, force the Side to SIDE_NOD.
                 */
                housetype->Side = SIDE_NOD;

                DEBUG_WARNING("Rules: Please consider changing House \"{}\" to have \"Side=Nod\"!\n",
                    housetype->Name());
            }

            /**
             *  #issue-903
             * 
             *  Workaround because NOD has Prefix=B in unmodded Tiberian Sun.
             * 
             *  Match criteria;
             *   - The HouseType's name is "Nod"
             *   - HouseType "Nod" is index 1
             *   - HouseType "Nod" has Prefix=B
             */
            if (housetype->IniName == "Nod"
                && housetype->Fetch_Heap_ID() == HOUSE_NOD
                && housetype->Prefix == 'B') {

                DEBUG_WARNING("Rules: House \"{}\" ({}) has \"Prefix=B\", changing Prefix to \"N\"!\n",
                    housetype->Name(), housetype->Fetch_Heap_ID());

                /**
                 *  We are pretty sure this house is NOD, force the Prefix to the 'N' character.
                 */
                housetype->Prefix = 'N';

                DEBUG_WARNING("Rules: Please consider changing House \"{}\" to have \"Side=Nod\"!\n",
                    housetype->Name());
            }

        }
    }

    if (is_ruleini) {

    }

    DEBUG_INFO("Rules::Fixups(exit)\n");
}


/**
 *  Sets the properties of the voxel light.
 *
 *  @author: ZivDero
 */
bool RulesClassExtension::Set_Voxel_Light_Angle(float azimuth, float elevation, float offset)
{
    static float _last_azimuth = 0;
    static float _last_offset = 6;

    if (azimuth != _last_azimuth || elevation != VoxelLightAngle || offset != _last_offset) {
        Matrix3D mtx(true);
        VoxelLightAngle = elevation;
        _last_azimuth = azimuth;
        _last_offset = offset;
        mtx.Rotate_Z(azimuth);
        mtx.Rotate_Y(elevation);
        VoxelLightSource = mtx * Vector3(-1, 0, 0);
        VoxelShadowLightSource = Vector3(-offset * VoxelLightSource.X, -offset * VoxelLightSource.Y, 0);
        ObjectTypeClass::Clear_Voxel_Indexes();
        return true;
    }

    return false;
}
