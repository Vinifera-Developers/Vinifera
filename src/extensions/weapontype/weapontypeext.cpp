/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WEAPONTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended WeaponTypeClass class.
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
#include "weapontypeext.h"
#include "weapontype.h"
#include "ebolt.h"
#include "ccini.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all WeaponTypeClass extension instances.
 */
ExtensionMap<WeaponTypeClass, WeaponTypeClassExtension> WeaponTypeClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
WeaponTypeClassExtension::WeaponTypeClassExtension(WeaponTypeClass *this_ptr) :
    Extension(this_ptr),
    IsSuicide(false),
    IsDeleteOnSuicide(false),
    IsElectricBolt(false),
    ElectricBoltColor1(EBOLT_DEFAULT_COLOR_1),
    ElectricBoltColor2(EBOLT_DEFAULT_COLOR_2),
    ElectricBoltColor3(EBOLT_DEFAULT_COLOR_3),
    ElectricBoltSegmentCount(EBOLT_DEFAULT_LINE_SEGEMENTS),
    ElectricBoltLifetime(EBOLT_DEFAULT_LIFETIME),
    ElectricBoltIterationCount(EBOLT_DEFAULT_INTERATIONS),
    ElectricBoltDeviation(EBOLT_DEFAULT_DEVIATION)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("WeaponTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
WeaponTypeClassExtension::WeaponTypeClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class deconstructor.
 *  
 *  @author: CCHyper
 */
WeaponTypeClassExtension::~WeaponTypeClassExtension()
{
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("WeaponTypeClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT WeaponTypeClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) WeaponTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT WeaponTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

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
int WeaponTypeClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void WeaponTypeClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void WeaponTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    crc(IsElectricBolt);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool WeaponTypeClassExtension::Read_INI(CCINIClass &ini)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("WeaponTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    EXT_DEBUG_WARNING("WeaponTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    const char *ini_name = ThisPtr->Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }
    
    IsSuicide = ini.Get_Bool(ini_name, "Suicide", IsSuicide);
    IsDeleteOnSuicide = ini.Get_Bool(ini_name, "DeleteOnSuicide", IsDeleteOnSuicide);

    IsElectricBolt = ini.Get_Bool(ini_name, "IsElectricBolt", IsElectricBolt);
    ElectricBoltColor1 = ini.Get_RGB(ini_name, "EBoltColor1", ElectricBoltColor1);
    ElectricBoltColor2 = ini.Get_RGB(ini_name, "EBoltColor2", ElectricBoltColor2);
    ElectricBoltColor3 = ini.Get_RGB(ini_name, "EBoltColor3", ElectricBoltColor3);
    ElectricBoltSegmentCount = ini.Get_Int(ini_name, "EBoltSegmentCount", ElectricBoltSegmentCount);
    ElectricBoltLifetime = ini.Get_Int(ini_name, "EBoltLifetime", ElectricBoltLifetime);
    ElectricBoltIterationCount = ini.Get_Int(ini_name, "EBoltIterations", ElectricBoltIterationCount);
    ElectricBoltDeviation = ini.Get_Float(ini_name, "EBoltDeviation", ElectricBoltDeviation);
    //ElectricBoltSourceBoltParticleSys = ini.Get_ParticleSys(ini_name, "EBoltSourceParticleSys", ElectricBoltSourceBoltParticleSys);
    //ElectricBoltTargetBoltParticleSys = ini.Get_ParticleSys(ini_name, "EBoltTargetBoltParticleSys", ElectricBoltTargetBoltParticleSys);
    

    return true;
}
