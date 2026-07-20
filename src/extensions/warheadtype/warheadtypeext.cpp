/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended WarheadTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "warheadtypeext.h"

#include "animtype.h"
#include "armortype.h"
#include "asserthandler.h"
#include "ccini.h"
#include "debughandler.h"
#include "extension.h"
#include "findmake.h"
#include "miscutil.h"
#include "tibsun_globals.h"
#include "verses.h"
#include "vinifera_globals.h"
#include "vinifera_saveload.h"
#include "warheadtype.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
WarheadTypeClassExtension::WarheadTypeClassExtension(const WarheadTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    IsWallAbsoluteDestroyer(false),
    IsAffectsAllies(true),
    CombatLightSize(0.0f),
    ShakePixelYHi(0),
    ShakePixelYLo(0),
    ShakePixelXHi(0),
    ShakePixelXLo(0),
    MinDamage(-1),
    CellSpread(-1.0f),
    PercentAtMax(1.0f),
    ScorchChance(0.0f),
    ScorchPercentAtMax(1.0f),
    CraterChance(0.0f),
    CraterPercentAtMax(1.0f),
    CellAnimChance(0.0f),
    CellAnimPercentAtMax(1.0f),
    CellAnim(),
    InfantryModifier(1.0f),
    VehicleModifier(1.0f),
    AircraftModifier(1.0f),
    BuildingModifier(1.0f),
    TerrainModifier(1.0f),
    IsVolumetric(false),
    IsSnapToCellCenter(false)
{
    WarheadTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
WarheadTypeClassExtension::WarheadTypeClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit),
    CellAnim(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
WarheadTypeClassExtension::~WarheadTypeClassExtension()
{
    WarheadTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT WarheadTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT WarheadTypeClassExtension::Load(IStream *pStm)
{
    CellAnim.Clear();

    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return hr;
    }

    new (this) WarheadTypeClassExtension(NoInitClass());

    CellAnim.Load_Self(pStm);

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(CellAnim, "CellAnim");
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT WarheadTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = AbstractTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    CellAnim.Save_Self(pStm);

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int WarheadTypeClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void WarheadTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    crc(IsWallAbsoluteDestroyer);
    crc(IsAffectsAllies);
    crc(CombatLightSize);
    crc(ShakePixelYHi);
    crc(ShakePixelYLo);
    crc(ShakePixelXHi);
    crc(ShakePixelXLo);
    crc(CellSpread);
    crc(PercentAtMax);
    crc(ScorchChance);
    crc(ScorchPercentAtMax);
    crc(CraterChance);
    crc(CraterPercentAtMax);
    crc(CellAnimChance);
    crc(CellAnimPercentAtMax);
    crc(CellAnim.Count());
    crc(InfantryModifier);
    crc(VehicleModifier);
    crc(AircraftModifier);
    crc(BuildingModifier);
    crc(TerrainModifier);
    crc(IsVolumetric);
    crc(IsSnapToCellCenter);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper, ZivDero
 */
bool WarheadTypeClassExtension::Read_INI(CCINIClass &ini)
{
    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    char buffer[256];

    const char *ini_name = Name();

    IsWallAbsoluteDestroyer = ini.Get_Bool(ini_name, "WallAbsoluteDestroyer", IsWallAbsoluteDestroyer);
    IsAffectsAllies = ini.Get_Bool(ini_name, "AffectsAllies", IsAffectsAllies);
    CombatLightSize = ini.Get_Float(ini_name, "CombatLightSize", CombatLightSize);
    CombatLightSize = std::clamp(CombatLightSize, 0.0, 1.0);
    ShakePixelYHi = ini.Get_Int(ini_name, "ShakeYhi", ShakePixelYHi);
    ShakePixelYLo = ini.Get_Int(ini_name, "ShakeYlo", ShakePixelYLo);
    ShakePixelXHi = ini.Get_Int(ini_name, "ShakeXhi", ShakePixelXHi);
    ShakePixelXLo = ini.Get_Int(ini_name, "ShakeXlo", ShakePixelXLo);

    WarheadType warheadtype = static_cast<WarheadType>(Warheads.ID(This()));

    /**
     *  Reload the legacy version Verses, ForceFire, PassiveAcquire, Retaliate entries into the new Modifier array.
     */
    if (ini.Get_String(ini_name, "Verses", nullptr, buffer, sizeof(buffer)) > 0) {
        char *token = std::strtok(buffer, ",");
        for (ArmorType armor = ARMOR_NONE; armor < ArmorTypes.Count() && token; armor++, token = std::strtok(nullptr, ",")) {
            if (std::strchr(token, '%')) {
                Verses::Set_Modifier(armor, warheadtype, std::atoi(token) * 0.01);
            } else {
                Verses::Set_Modifier(armor, warheadtype, std::atof(token));
            }
        }
    }

    if (ini.Get_String(ini_name, "ForceFire", nullptr, buffer, sizeof(buffer)) > 0) {
        char* token = std::strtok(buffer, ",");
        for (ArmorType armor = ARMOR_NONE; armor < ArmorTypes.Count() && token; armor++, token = std::strtok(nullptr, ",")) {
            Verses::Set_ForceFire(armor, warheadtype, Parse_Boolean(token, Verses::Get_ForceFire(armor, warheadtype)));
        }
    }

    if (ini.Get_String(ini_name, "PassiveAcquire", nullptr, buffer, sizeof(buffer)) > 0) {
        char* token = std::strtok(buffer, ",");
        for (ArmorType armor = ARMOR_NONE; armor < ArmorTypes.Count() && token; armor++, token = std::strtok(nullptr, ",")) {
            Verses::Set_PassiveAcquire(armor, warheadtype, Parse_Boolean(token, Verses::Get_PassiveAcquire(armor, warheadtype)));
        }
    }

    if (ini.Get_String(ini_name, "Retaliate", nullptr, buffer, sizeof(buffer)) > 0) {
        char* token = std::strtok(buffer, ",");
        for (ArmorType armor = ARMOR_NONE; armor < ArmorTypes.Count() && token; armor++, token = std::strtok(nullptr, ",")) {
            Verses::Set_Retaliate(armor, warheadtype, Parse_Boolean(token, Verses::Get_Retaliate(armor, warheadtype)));
        }
    }

    /**
     *  Read the new Modifier, ForceFire, PassiveAcquire, Retaliate per-armor keys.
     */
    for (ArmorType armor = ARMOR_FIRST; armor < ArmorTypes.Count(); armor++)
    {
        static char key_name[256];
        const char* armor_name = ArmorTypeClass::Name_From(armor);

        std::snprintf(key_name, sizeof(key_name), "Modifier.%s", armor_name);
        if (ini.Get_String(ini_name, key_name, nullptr, buffer, sizeof(buffer)) > 0) {
            Verses::Set_Modifier(armor, warheadtype, ini.Get_Float(ini_name, key_name, Verses::Get_Modifier(armor, warheadtype)));
        }

        std::snprintf(key_name, sizeof(key_name), "ForceFire.%s", armor_name);
        if (ini.Get_String(ini_name, key_name, nullptr, buffer, sizeof(buffer)) > 0) {
            Verses::Set_ForceFire(armor, warheadtype, ini.Get_Bool(ini_name, key_name, Verses::Get_ForceFire(armor, warheadtype)));
        }

        std::snprintf(key_name, sizeof(key_name), "PassiveAcquire.%s", armor_name);
        if (ini.Get_String(ini_name, key_name, nullptr, buffer, sizeof(buffer)) > 0) {
            Verses::Set_PassiveAcquire(armor, warheadtype, ini.Get_Bool(ini_name, key_name, Verses::Get_PassiveAcquire(armor, warheadtype)));
        }

        std::snprintf(key_name, sizeof(key_name), "Retaliate.%s", armor_name);
        if (ini.Get_String(ini_name, key_name, nullptr, buffer, sizeof(buffer)) > 0) {
            Verses::Set_Retaliate(armor, warheadtype, ini.Get_Bool(ini_name, key_name, Verses::Get_Retaliate(armor, warheadtype)));
        }
    }

    if (!IsInitialized) {
        This()->IsOrganic = Verses::Get_Modifier(ARMOR_STEEL, warheadtype) == 0.0;
    }

    /**
     *  Allow overriding IsOrganic.
     */
    This()->IsOrganic = ini.Get_Bool(ini_name, "Organic", This()->IsOrganic);

    MinDamage = ini.Get_Int(ini_name, "MinDamage", MinDamage);

    CellSpread = ini.Get_Float(ini_name, "CellSpread", CellSpread);
    PercentAtMax = ini.Get_Float(ini_name, "PercentAtMax", PercentAtMax);

    ScorchChance = ini.Get_Float(ini_name, "ScorchChance", ScorchChance);
    ScorchChance = std::clamp(ScorchChance, 0.0f, 1.0f);
    ScorchPercentAtMax = ini.Get_Float(ini_name, "ScorchPercentAtMax", ScorchPercentAtMax);

    CraterChance = ini.Get_Float(ini_name, "CraterChance", CraterChance);
    CraterChance = std::clamp(CraterChance, 0.0f, 1.0f);
    CraterPercentAtMax = ini.Get_Float(ini_name, "CraterPercentAtMax", CraterPercentAtMax);

    CellAnimChance = ini.Get_Float(ini_name, "CellAnimChance", CellAnimChance);
    CellAnimChance = std::clamp(CellAnimChance, 0.0f, 1.0f);
    CellAnimPercentAtMax = ini.Get_Float(ini_name, "CellAnimPercentAtMax", CellAnimPercentAtMax);
    CellAnim = TGet_TypeList(ini, ini_name, "CellAnim", CellAnim);

    InfantryModifier = ini.Get_Float(ini_name, "InfantryModifier", InfantryModifier);
    VehicleModifier = ini.Get_Float(ini_name, "VehicleModifier", VehicleModifier);
    AircraftModifier = ini.Get_Float(ini_name, "AircraftModifier", AircraftModifier);
    BuildingModifier = ini.Get_Float(ini_name, "BuildingModifier", BuildingModifier);
    TerrainModifier = ini.Get_Float(ini_name, "TerrainModifier", TerrainModifier);

    IsVolumetric = ini.Get_Bool(ini_name, "Volumetric", IsVolumetric);
    IsSnapToCellCenter = ini.Get_Bool(ini_name, "SnapToCellCenter", IsSnapToCellCenter);

    IsInitialized = true;

    return true;
}


/**
 *  Returns the damage modifier for this type of object.
 *
 *  @author: ZivDero
 */
float WarheadTypeClassExtension::Fetch_Type_Modifier(RTTIType type) const
{
    switch (type) {
    case RTTI_INFANTRY:
    case RTTI_INFANTRYTYPE:
        return InfantryModifier;
    case RTTI_UNIT:
    case RTTI_UNITTYPE:
        return VehicleModifier;
    case RTTI_AIRCRAFT:
    case RTTI_AIRCRAFTTYPE:
        return AircraftModifier;
    case RTTI_BUILDING:
    case RTTI_BUILDINGTYPE:
        return BuildingModifier;
    case RTTI_TERRAIN:
    case RTTI_TERRAINTYPE:
        return TerrainModifier;
    default:
        break;
    }
    return 1.0f;
}
