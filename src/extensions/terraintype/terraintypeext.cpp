/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TERRAINTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TerrainTypeClass class.
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
#include "terraintypeext.h"
#include "terraintype.h"
#include "ccini.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all TerrainTypeClass extension instances.
 */
ExtensionMap<TerrainTypeClass, TerrainTypeClassExtension> TerrainTypeClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TerrainTypeClassExtension::TerrainTypeClassExtension(TerrainTypeClass *this_ptr) :
    Extension(this_ptr),
    IsLightEnabled(false),
    LightVisibility(5000),
    LightIntensity(0),
    LightRedTint(1000000),
    LightGreenTint(1000000),
    LightBlueTint(1000000),
    SpawnsTiberiumType(TIBERIUM_RIPARIUS)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TerrainTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("TerrainTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TerrainTypeClassExtension::TerrainTypeClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class deconstructor.
 *  
 *  @author: CCHyper
 */
TerrainTypeClassExtension::~TerrainTypeClassExtension()
{
    //DEV_DEBUG_TRACE("TerrainTypeClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("TerrainTypeClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT TerrainTypeClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TerrainTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
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
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TerrainTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Save(pStm, fClearDirty);
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
int TerrainTypeClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TerrainTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TerrainTypeClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TerrainTypeClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TerrainTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TerrainTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    crc(IsLightEnabled);
    crc(SpawnsTiberiumType);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool TerrainTypeClassExtension::Read_INI(CCINIClass &ini)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TerrainTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    DEV_DEBUG_WARNING("TerrainTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    const char *ini_name = ThisPtr->Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    IsLightEnabled = ini.Get_Bool(ini_name, "IsLightEnabled", IsLightEnabled);
    LightVisibility = ini.Get_Int(ini_name, "LightVisibility", LightVisibility);
    LightIntensity = ini.Get_Double(ini_name, "LightIntensity", (LightIntensity / 1000)) * 1000.0 + 0.1;
    LightRedTint = ini.Get_Double(ini_name, "LightRedTint", (LightRedTint / 1000)) * 1000.0 + 0.1;
    LightGreenTint = ini.Get_Double(ini_name, "LightGreenTint", (LightGreenTint / 1000)) * 1000.0 + 0.1;
    LightBlueTint = ini.Get_Double(ini_name, "LightBlueTint", (LightBlueTint / 1000)) * 1000.0 + 0.1;

    SpawnsTiberiumType = ini.Get_TiberiumType(ini_name, "SpawnsTiberiumType", SpawnsTiberiumType);
    
    return true;
}
