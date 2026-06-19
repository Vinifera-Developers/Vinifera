/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TechnoTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "technotypeext.h"

#include "aircrafttype.h"
#include "animtype.h"
#include "bsurface.h"
#include "ccini.h"
#include "debughandler.h"
#include "findmake.h"
#include "rules.h"
#include "technotype.h"
#include "tibsun_globals.h"
#include "unittype.h"
#include "vinifera_saveload.h"
#include "vinifera_util.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
TechnoTypeClassExtension::TechnoTypeClassExtension(const TechnoTypeClass *this_ptr) :
    ObjectTypeClassExtension(this_ptr),
    CloakSound(VOC_NONE),
    UncloakSound(VOC_NONE),
    IsShakeScreen(false),
    IsImmuneToEMP(false),
    IsCanPassiveAcquire(true),
    IsCanRetaliate(true),
    IsLegalTargetComputer(true),
    ShakePixelYHi(0),
    ShakePixelYLo(0),
    ShakePixelXHi(0),
    ShakePixelXLo(0),
    UnloadingClass(nullptr),
    SoylentValue(-1),
    EnterTransportSound(VOC_NONE),
    LeaveTransportSound(VOC_NONE),
    VoiceCapture(),
    VoiceEnter(),
    VoiceDeploy(),
    VoiceHarvest(),
    SpecialPipIndex(-1),
    PipWrap(0),
    IdleRate(0),
    CameoImageSurface(nullptr),
    IsSortCameoAsBaseDefense(false),
    Description(""),
    IsFilterFromBandBoxSelection(false),
    CrewCount(1),
    IsMissileSpawn(false),
    Spawns(nullptr),
    SpawnReloadRate(0),
    SpawnRegenRate(0),
    SpawnSpawnRate(20),
    SpawnLogicRate(10),
    SpawnsNumber(0),
    SecondSpawnOffset(0, 0, 0),
    MaxRandomSpawnOffset(0),
    IsDontScore(false),
    IsSpawned(false),
    RequiredHouses(-1),
    ForbiddenHouses(-1),
    TargetZoneScan(TZST_SAME),
    IsDecloakToFire(true),
    _JumpjetTurnRate(std::numeric_limits<int>::min()),
    _JumpjetSpeed(std::numeric_limits<int>::min()),
    _JumpjetClimb(-std::numeric_limits<double>::max()),
    _JumpjetCruiseHeight(std::numeric_limits<int>::min()),
    _JumpjetAcceleration(-std::numeric_limits<double>::max()),
    _JumpjetWobblesPerSecond(-std::numeric_limits<double>::max()),
    _JumpjetWobbleDeviation(std::numeric_limits<int>::min()),
    _JumpjetCloakDetectionRadius(std::numeric_limits<int>::min()),
    JumpjetNoWobbles(false),
    IsNaval(false),
    BuiltAt(),
    BuildTimeMultiplier(1.0f),
    IsOpportunityFire(false),
    WakeAnim(nullptr),
    WakeAnimRate(10),                   // Default DriveLocomotion value.
    IdleWakeAnim(nullptr),
    IsHideWakeWhenCloaked(false),
    SelfHealingCap(-1),
    SelfHealingRate(-1),
    IsDetectDisguise(false),
    IronCurtainPriorityTarget(false),
    EscortRange(-1),
    AbandonTargetEscortRange(-1),
    VeteranSightRange(-1),
    EliteSightRange(-1)
{
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::TechnoTypeClassExtension(const NoInitClass &noinit) :
    ObjectTypeClassExtension(noinit),
    VoiceCapture(noinit),
    VoiceEnter(noinit),
    VoiceDeploy(noinit),
    VoiceHarvest(noinit),
    BuiltAt(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::~TechnoTypeClassExtension()
{
    delete CameoImageSurface;
    CameoImageSurface = nullptr;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoTypeClassExtension::Load(IStream *pStm)
{
    VoiceCapture.Clear();
    VoiceEnter.Clear();
    VoiceDeploy.Clear();
    VoiceHarvest.Clear();
    BuiltAt.Clear();

    HRESULT hr = ObjectTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    VoiceCapture.Load_Self(pStm);
    VoiceEnter.Load_Self(pStm);
    VoiceDeploy.Load_Self(pStm);
    VoiceHarvest.Load_Self(pStm);
    BuiltAt.Load_Self(pStm);

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(UnloadingClass, "UnloadingClass");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Spawns, "Spawns");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(WakeAnim, "WakeAnim");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(IdleWakeAnim, "IdleWakeAnim");

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(BuiltAt, "BuiltAt");

    /**
     *  We need to reload the "Cameo" key because TechnoTypeClass does
     *  not store the entry value. 
     */
    const char *ini_name = IniName.c_str();
    const char* graphic_name = GraphicName.c_str();

    char cameo_buffer[32];
    
    ArtINI.Get_String(ini_name, "Cameo", "XXICON", cameo_buffer, sizeof(cameo_buffer));
    if (std::string_view(cameo_buffer) != "XXICON") {
        ArtINI.Get_String(graphic_name, "Cameo", "XXICON", cameo_buffer, sizeof(cameo_buffer));

        /**
         *  Fetch the cameo image surface if it exists.
         */
        BSurface *imagesurface = Vinifera_Get_Image_Surface(cameo_buffer);
        if (imagesurface) {
            CameoImageSurface = imagesurface;
        }

    }
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = ObjectTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    VoiceCapture.Save_Self(pStm);
    VoiceEnter.Save_Self(pStm);
    VoiceDeploy.Save_Self(pStm);
    VoiceHarvest.Save_Self(pStm);
    BuiltAt.Save_Self(pStm);

    return hr;
}


/**
 *  Retrieves the size of the stream needed to save the object.
 * 
 *  @author: CCHyper, tomsons26
 */
LONG TechnoTypeClassExtension::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    if (!pcbSize) {
        return E_POINTER;
    }

    pcbSize->LowPart += VoiceCapture.Count() * sizeof(uint32_t);
    pcbSize->LowPart += VoiceEnter.Count() * sizeof(uint32_t);
    pcbSize->LowPart += VoiceDeploy.Count() * sizeof(uint32_t);
    pcbSize->LowPart += VoiceHarvest.Count() * sizeof(uint32_t);

    return S_OK;
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TechnoTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    crc(IsShakeScreen);
    crc(IsImmuneToEMP);
    crc(ShakePixelYHi);
    crc(ShakePixelYLo);
    crc(ShakePixelXHi);
    crc(ShakePixelXLo);
    crc(SoylentValue);
    crc(IsLegalTargetComputer);
    crc(Spawns->Fetch_Heap_ID());
    crc(SpawnRegenRate);
    crc(SpawnReloadRate);
    crc(SpawnsNumber);
    crc(TargetZoneScan);
    crc(IsDecloakToFire);
    crc(_JumpjetTurnRate);
    crc(_JumpjetSpeed);
    crc(_JumpjetClimb);
    crc(_JumpjetCruiseHeight);
    crc(_JumpjetAcceleration);
    crc(_JumpjetWobblesPerSecond);
    crc(_JumpjetWobbleDeviation);
    crc(_JumpjetCloakDetectionRadius);
    crc(JumpjetNoWobbles);
    crc(IsNaval);
    crc(BuiltAt.Count());
    crc(IsOpportunityFire);
    crc(WakeAnimRate);
    crc(IsHideWakeWhenCloaked);
    crc(SelfHealingCap);
    crc(SelfHealingRate);
    crc(IsDetectDisguise);
    crc(IronCurtainPriorityTarget);
    crc(EscortRange);
    crc(AbandonTargetEscortRange);
    crc(VeteranSightRange);
    crc(EliteSightRange);
}


/**
 *  #issue-1161
 *
 *  Fetches the target zone scan type from the INI database.
 *
 *  @author: Rampastring
 */
TargetZoneScanType _Get_TargetZoneScanType(CCINIClass& ini, const char* section, const char* entry, const TargetZoneScanType defvalue)
{
    char buffer[1024];

    if (ini.Get_String(section, entry, nullptr, buffer, sizeof(buffer)) > 0) {
        if (std::strncmp("Same", buffer, sizeof("Same")) == 0) {
            return TZST_SAME;
        }

        if (std::strncmp("Any", buffer, sizeof("Any")) == 0) {
            return TZST_ANY;
        }

        if (std::strncmp("InRange", buffer, sizeof("InRange")) == 0) {
            return TZST_INRANGE;
        }
    }

    return defvalue;
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool TechnoTypeClassExtension::Read_INI(CCINIClass &ini)
{
    if (!ObjectTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();
    const char *graphic_name = Graphic_Name();

    //if (!ArtINI.Is_Present(graphic_name)) {
    //    return false;
    //}

    /**
     *  #issue-407
     * 
     *  Allow WalkRate to be optionally loaded from ART.INI image entries. This
     *  will also override any value set on the RULES.INI section.
     * 
     *  @author: CCHyper
     */
    This()->WalkRate = ArtINI.Get_Int(graphic_name, "WalkRate", This()->WalkRate);
    
    CloakSound = ini.Get_VocType(ini_name, "CloakSound", CloakSound);
    UncloakSound = ini.Get_VocType(ini_name, "UncloakSound", UncloakSound);
    IsShakeScreen = ini.Get_Bool(ini_name, "CanShakeScreen", IsShakeScreen);
    IsImmuneToEMP = ini.Get_Bool(ini_name, "ImmuneToEMP", IsImmuneToEMP);
    IsCanPassiveAcquire = ini.Get_Bool(ini_name, "CanPassiveAcquire", IsCanPassiveAcquire);
    IsCanRetaliate = ini.Get_Bool(ini_name, "CanRetaliate", IsCanRetaliate);
    IsLegalTargetComputer = ini.Get_Bool(ini_name, "AILegalTarget", IsLegalTargetComputer);
    ShakePixelYHi = ini.Get_Int(ini_name, "ShakeYhi", ShakePixelYHi);
    ShakePixelYLo = ini.Get_Int(ini_name, "ShakeYlo", ShakePixelYLo);
    ShakePixelXHi = ini.Get_Int(ini_name, "ShakeXhi", ShakePixelXHi);
    ShakePixelXLo = ini.Get_Int(ini_name, "ShakeXlo", ShakePixelXLo);
    UnloadingClass = TGet_Class(ini, ini_name, "UnloadingClass", UnloadingClass);
    SoylentValue = ini.Get_Int(ini_name, "Soylent", SoylentValue);
    EnterTransportSound = ini.Get_VocType(ini_name, "EnterTransportSound", EnterTransportSound);
    LeaveTransportSound = ini.Get_VocType(ini_name, "LeaveTransportSound", LeaveTransportSound);
    VoiceCapture = Get_VocTypes(ini, ini_name, "VoiceCapture", VoiceCapture);
    VoiceEnter = Get_VocTypes(ini, ini_name, "VoiceEnter", VoiceEnter);
    VoiceDeploy = Get_VocTypes(ini, ini_name, "VoiceDeploy", VoiceDeploy);
    VoiceHarvest = Get_VocTypes(ini, ini_name, "VoiceHarvest", VoiceHarvest);
    SpecialPipIndex = ini.Get_Int(ini_name, "SpecialPipIndex", SpecialPipIndex);
    PipWrap = ini.Get_Int(ini_name, "PipWrap", PipWrap);

    if (ini.Is_Present(ini_name, "Description")) {
        ini.Get_String(ini_name, "Description", "", Description, std::size(Description));
    }

    IdleRate = ini.Get_Int(ini_name, "IdleRate", IdleRate);
    IdleRate = ArtINI.Get_Int(graphic_name, "IdleRate", IdleRate);

    BuildTimeMultiplier = ini.Get_Float(ini_name, "BuildTimeMultiplier", BuildTimeMultiplier);

    /**
     *  Fetch the cameo image surface if it exists.
     */
    BSurface* imagesurface = Vinifera_Get_Image_Surface(This()->CameoFilename.c_str());
    if (imagesurface) {
        CameoImageSurface = imagesurface;
    }

    IsSortCameoAsBaseDefense = ini.Get_Bool(ini_name, "SortCameoAsBaseDefense", IsSortCameoAsBaseDefense);
    IsFilterFromBandBoxSelection = ini.Get_Bool(ini_name, "FilterFromBandBoxSelection", IsFilterFromBandBoxSelection);
    CrewCount = ini.Get_Int(ini_name, "CrewCount", CrewCount);

    IsMissileSpawn = ini.Get_Bool(ini_name, "MissileSpawn", IsMissileSpawn);
    Spawns = TGet_Class(ini, ini_name, "Spawns", Spawns);
    SpawnReloadRate = ini.Get_Int(ini_name, "SpawnReloadRate", SpawnReloadRate);
    SpawnRegenRate = ini.Get_Int(ini_name, "SpawnRegenRate", SpawnRegenRate);
    SpawnSpawnRate = ini.Get_Int(ini_name, "SpawnSpawnRate", SpawnSpawnRate);
    SpawnLogicRate = ini.Get_Int(ini_name, "SpawnLogicRate", SpawnLogicRate);
    SpawnsNumber = ini.Get_Int(ini_name, "SpawnsNumber", SpawnsNumber);
    SecondSpawnOffset = ArtINI.Get_Point(graphic_name, "SecondSpawnOffset", SecondSpawnOffset);
    MaxRandomSpawnOffset = ini.Get_Int(ini_name, "MaxRandomSpawnOffset", MaxRandomSpawnOffset);

    IsDontScore = ini.Get_Bool(ini_name, "DontScore", IsDontScore);
    IsSpawned = ini.Get_Bool(ini_name, "Spawned", IsSpawned);

    RequiredHouses = ini.Get_Owners(ini_name, "RequiredHouses", RequiredHouses);
    ForbiddenHouses = ini.Get_Owners(ini_name, "ForbiddenHouses", ForbiddenHouses);

    TargetZoneScan = _Get_TargetZoneScanType(ini, ini_name, "TargetZoneScan", TargetZoneScan);
    IsDecloakToFire = ini.Get_Bool(ini_name, "DecloakToFire", IsDecloakToFire);

    _JumpjetTurnRate = ini.Get_Int(ini_name, "JumpjetTurnRate", _JumpjetTurnRate);
    _JumpjetSpeed = ini.Get_Int(ini_name, "JumpjetSpeed", _JumpjetSpeed);
    _JumpjetClimb = ini.Get_Float(ini_name, "JumpjetClimb", _JumpjetClimb);
    _JumpjetCruiseHeight = ini.Get_Int(ini_name, "JumpjetCruiseHeight", _JumpjetCruiseHeight);
    _JumpjetAcceleration = ini.Get_Float(ini_name, "JumpjetAcceleration", _JumpjetAcceleration);
    _JumpjetWobblesPerSecond = ini.Get_Float(ini_name, "JumpjetWobblesPerSecond", _JumpjetWobblesPerSecond);
    _JumpjetWobbleDeviation = ini.Get_Int(ini_name, "JumpjetWobbleDeviation", _JumpjetWobbleDeviation);
    _JumpjetCloakDetectionRadius = ini.Get_Int(ini_name, "JumpjetCloakDetectionRadius", _JumpjetCloakDetectionRadius);
    JumpjetNoWobbles = ini.Get_Bool(ini_name, "JumpjetNoWobbles", JumpjetNoWobbles);

    IsNaval = ini.Get_Bool(ini_name, "Naval", IsNaval);

    BuiltAt = TGet_TypeList(ini, ini_name, "BuiltAt", BuiltAt);
    IsOpportunityFire = ini.Get_Bool(ini_name, "OpportunityFire", IsOpportunityFire);

    WakeAnim = TGet_Class(ArtINI, graphic_name, "WakeAnim", WakeAnim);
    WakeAnimRate = ArtINI.Get_Int(graphic_name, "WakeAnimRate", WakeAnimRate);
    IdleWakeAnim = TGet_Class(ArtINI, graphic_name, "IdleWakeAnim", IdleWakeAnim);
    IsHideWakeWhenCloaked = ArtINI.Get_Bool(graphic_name, "HideWakeWhenCloaked", IsHideWakeWhenCloaked);

    SelfHealingCap = ini.Get_Float(ini_name, "SelfHealingCap", SelfHealingCap);
    SelfHealingRate = ini.Get_Float(ini_name, "SelfHealingRate", SelfHealingRate);

    IsDetectDisguise = ini.Get_Bool(ini_name, "DetectDisguise", IsDetectDisguise);

    IronCurtainPriorityTarget = ini.Get_Bool(ini_name, "IronCurtainPriorityTarget", IronCurtainPriorityTarget);

    EscortRange = ini.Get_Lepton(ini_name, "EscortRange", EscortRange);
    AbandonTargetEscortRange = ini.Get_Lepton(ini_name, "AbandonTargetEscortRange", AbandonTargetEscortRange);

    VeteranSightRange = ini.Get_Int(ini_name, "VeteranSight", VeteranSightRange);
    EliteSightRange = ini.Get_Int(ini_name, "EliteSight", EliteSightRange);

    return true;
}


/**
 *  Gets the production flags for this object type.
 *
 *  @author: ZivDero
 */
ProductionFlags TechnoTypeClassExtension::Get_Production_Flags(RTTIType type, int id)
{
    const TechnoTypeClass* ttype = Fetch_Techno_Type(type, id);
    if (ttype != nullptr) {
        return Get_Production_Flags(ttype);
    }
    return PRODFLAG_NONE;
}


/**
 *  Gets the production flags for this object type.
 *
 *  @author: ZivDero
 */
ProductionFlags TechnoTypeClassExtension::Get_Production_Flags(const TechnoTypeClassExtension* ttype_ext)
{
    ProductionFlags flags = PRODFLAG_NONE;

    if (ttype_ext->IsNaval) {
        flags = static_cast<ProductionFlags>(flags | PRODFLAG_NAVAL);
    }

    return flags;
}


int TechnoTypeClassExtension::Get_Jumpjet_Turn_Rate() const
{
    if (_JumpjetTurnRate != std::numeric_limits<int>::min()) {
        return _JumpjetTurnRate;
    }
    return Rule->JumpjetTurnRate;
}


int TechnoTypeClassExtension::Get_Jumpjet_Speed() const
{
    if (_JumpjetSpeed != std::numeric_limits<int>::min()) {
        return _JumpjetSpeed;
    }
    return Rule->JumpjetSpeed;
}


double TechnoTypeClassExtension::Get_Jumpjet_Climb() const
{
    if (_JumpjetClimb != -std::numeric_limits<double>::max()) {
        return _JumpjetClimb;
    }
    return Rule->JumpjetClimb;
}


int TechnoTypeClassExtension::Get_Jumpjet_Cruise_Height() const
{
    if (_JumpjetCruiseHeight != std::numeric_limits<int>::min()) {
        return _JumpjetCruiseHeight;
    }
    return Rule->JumpjetCruiseHeight;
}


double TechnoTypeClassExtension::Get_Jumpjet_Acceleration() const
{
    if (_JumpjetAcceleration != -std::numeric_limits<double>::max()) {
        return _JumpjetAcceleration;
    }
    return Rule->JumpjetAcceleration;
}


double TechnoTypeClassExtension::Get_Jumpjet_Wobbles_Per_Second() const
{
    if (_JumpjetWobblesPerSecond != -std::numeric_limits<double>::max()) {
        return _JumpjetWobblesPerSecond;
    }
    return Rule->JumpjetWobblesPerSecond;
}


int TechnoTypeClassExtension::Get_Jumpjet_Wobble_Deviation() const
{
    if (_JumpjetWobbleDeviation != std::numeric_limits<int>::min()) {
        return _JumpjetWobbleDeviation;
    }
    return Rule->JumpjetWobbleDeviation;
}


int TechnoTypeClassExtension::Get_Jumpjet_Cloak_Detection_Radius() const {
    if (_JumpjetCloakDetectionRadius != std::numeric_limits<int>::min()) {
        return _JumpjetCloakDetectionRadius;
    }
    return Rule->JumpjetCloakDetectionRadius;
}
