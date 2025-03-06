/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WARHEADTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended WarheadTypeClass class.
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
#include "warheadtypeext.h"
#include "warheadtype.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "armortype.h"
#include "rules.h"
#include "ccini.h"
#include "wwcrc.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "miscutil.h"
#include "verses.h"
#include "vinifera_saveload.h"


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
    CraterChance(0.0f),
    CellAnimChance(0.0f),
    CellAnim(),
    InfantryModifier(1.0f),
    VehicleModifier(1.0f),
    AircraftModifier(1.0f),
    BuildingModifier(1.0f),
    TerrainModifier(1.0f),
    IsVolumetric(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("WarheadTypeClassExtension::WarheadTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::WarheadTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
WarheadTypeClassExtension::~WarheadTypeClassExtension()
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::~WarheadTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    WarheadTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT WarheadTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    CellAnim.Clear();

    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return hr;
    }

    new (this) WarheadTypeClassExtension(NoInitClass());

    CellAnim.Load(pStm);

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
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    CellAnim.Save(pStm);

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int WarheadTypeClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void WarheadTypeClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void WarheadTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    crc(CraterChance);
    crc(CellAnimChance);
    crc(CellAnim.Count());
    crc(InfantryModifier);
    crc(VehicleModifier);
    crc(AircraftModifier);
    crc(BuildingModifier);
    crc(TerrainModifier);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper, ZivDero
 */
bool WarheadTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("WarheadTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    char buffer[256];

    const char *ini_name = Name();

    IsWallAbsoluteDestroyer = ini.Get_Bool(ini_name, "WallAbsoluteDestroyer", IsWallAbsoluteDestroyer);
    IsAffectsAllies = ini.Get_Bool(ini_name, "AffectsAllies", IsAffectsAllies);
    CombatLightSize = ini.Get_Float_Clamp(ini_name, "CombatLightSize", 0.0f, 1.0f, CombatLightSize);
    ShakePixelYHi = ini.Get_Int(ini_name, "ShakeYhi", ShakePixelYHi);
    ShakePixelYLo = ini.Get_Int(ini_name, "ShakeYlo", ShakePixelYLo);
    ShakePixelXHi = ini.Get_Int(ini_name, "ShakeXhi", ShakePixelXHi);
    ShakePixelXLo = ini.Get_Int(ini_name, "ShakeXlo", ShakePixelXLo);

    WarheadType warheadtype = static_cast<WarheadType>(WarheadTypes.ID(This()));

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
        if (ini.Is_Present(ini_name, key_name)) {
            Verses::Set_Modifier(armor, warheadtype, ini.Get_Double(ini_name, key_name, Verses::Get_Modifier(armor, warheadtype)));
        }

        std::snprintf(key_name, sizeof(key_name), "ForceFire.%s", armor_name);
        if (ini.Is_Present(ini_name, key_name)) {
            Verses::Set_ForceFire(armor, warheadtype, ini.Get_Bool(ini_name, key_name, Verses::Get_ForceFire(armor, warheadtype)));
        }

        std::snprintf(key_name, sizeof(key_name), "PassiveAcquire.%s", armor_name);
        if (ini.Is_Present(ini_name, key_name)) {
            Verses::Set_PassiveAcquire(armor, warheadtype, ini.Get_Bool(ini_name, key_name, Verses::Get_PassiveAcquire(armor, warheadtype)));
        }

        std::snprintf(key_name, sizeof(key_name), "Retaliate.%s", armor_name);
        if (ini.Is_Present(ini_name, key_name)) {
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
    CraterChance = ini.Get_Float(ini_name, "CraterChance", CraterChance);
    CraterChance = std::clamp(CraterChance, 0.0f, 1.0f);
    CellAnimChance = ini.Get_Float(ini_name, "CellAnimChance", CellAnimChance);
    CellAnimChance = std::clamp(CellAnimChance, 0.0f, 1.0f);
    CellAnim = ini.Get_Anims(ini_name, "CellAnim", CellAnim);

    InfantryModifier = ini.Get_Float(ini_name, "InfantryModifier", InfantryModifier);
    VehicleModifier = ini.Get_Float(ini_name, "VehicleModifier", VehicleModifier);
    AircraftModifier = ini.Get_Float(ini_name, "AircraftModifier", AircraftModifier);
    BuildingModifier = ini.Get_Float(ini_name, "BuildingModifier", BuildingModifier);
    TerrainModifier = ini.Get_Float(ini_name, "TerrainModifier", TerrainModifier);

    IsVolumetric = ini.Get_Bool(ini_name, "Volumetric", IsVolumetric);

    IsInitialized = true;

    return true;
}
