/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TerrainTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "terraintypeext.h"

#include "ccini.h"
#include "extension.h"
#include "terraintype.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
TerrainTypeClassExtension::TerrainTypeClassExtension(const TerrainTypeClass* this_ptr) :
    ObjectTypeClassExtension(this_ptr),
    IsLightEnabled(false),
    LightVisibility(5000),
    LightIntensity(0),
    LightRedTint(1000000),
    LightGreenTint(1000000),
    LightBlueTint(1000000),
    TiberiumSpawnRange(1),
    TiberiumSpawnStage(5, 5),
    TiberiumSpawnCount(1, 1),
    TiberiumSpawnStageFalloff(0),
    IsTiberiumScatterSpawn(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("TerrainTypeClassExtension::TerrainTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TerrainTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TerrainTypeClassExtension::TerrainTypeClassExtension(const NoInitClass &noinit) :
    ObjectTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("TerrainTypeClassExtension::TerrainTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TerrainTypeClassExtension::~TerrainTypeClassExtension()
{
    //EXT_DEBUG_TRACE("TerrainTypeClassExtension::~TerrainTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TerrainTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT TerrainTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("TerrainTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT TerrainTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("TerrainTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TerrainTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TerrainTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("TerrainTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int TerrainTypeClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("TerrainTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TerrainTypeClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("TerrainTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ObjectTypeClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TerrainTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TerrainTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    crc(IsLightEnabled);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool TerrainTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("TerrainTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!ObjectTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    IsLightEnabled = ini.Get_Bool(ini_name, "IsLightEnabled", IsLightEnabled);
    LightVisibility = ini.Get_Int(ini_name, "LightVisibility", LightVisibility);
    LightIntensity = ini.Get_Float(ini_name, "LightIntensity", (LightIntensity / 1000)) * 1000.0 + 0.1;
    LightRedTint = ini.Get_Float(ini_name, "LightRedTint", (LightRedTint / 1000)) * 1000.0 + 0.1;
    LightGreenTint = ini.Get_Float(ini_name, "LightGreenTint", (LightGreenTint / 1000)) * 1000.0 + 0.1;
    LightBlueTint = ini.Get_Float(ini_name, "LightBlueTint", (LightBlueTint / 1000)) * 1000.0 + 0.1;

    auto get_min_max = [](auto& ini, const char* section, const char* key, const Point2D& defval) {
        char buffer[128];
        int scan_min = 0, scan_max = 0;
        if (ini.Get_String(section, key, "", buffer, sizeof(buffer)) > 0) {
            int scanned = sscanf(buffer, "%d,%d", &scan_min, &scan_max);
            if (scanned > 0) {
                if (scanned == 1) {
                    return Point2D(scan_min, scan_min);
                } else if (scanned == 2) {
                    if (scan_max < scan_min) std::swap(scan_min, scan_max);
                    return Point2D(scan_min, scan_max);
                }
            }
        }
        return defval;
    };

    TiberiumSpawnRange = ini.Get_Int(ini_name, "SpawnsTiberiumRange", TiberiumSpawnRange);
    TiberiumSpawnCount = get_min_max(ini, ini_name, "SpawnsTiberiumCount", TiberiumSpawnCount);
    TiberiumSpawnStage = get_min_max(ini, ini_name, "SpawnsTiberiumStage", TiberiumSpawnStage);
    TiberiumSpawnStageFalloff = ini.Get_Float(ini_name, "SpawnsTiberiumStageFalloff", TiberiumSpawnStageFalloff);
    IsTiberiumScatterSpawn = ini.Get_Bool(ini_name, "SpawnsTiberiumScattered", IsTiberiumScatterSpawn);

    IsInitialized = true;
    
    return true;
}
