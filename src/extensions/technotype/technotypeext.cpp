/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOTYPEEXT.CPP
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
#include "technotypeext.h"
#include "technotype.h"
#include "ccini.h"
#include "swizzle.h"
#include "tibsun_globals.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all TechnoTypeClass extension instances.
 */
ExtensionMap<TechnoTypeClass, TechnoTypeClassExtension> TechnoTypeClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::TechnoTypeClassExtension(TechnoTypeClass *this_ptr) :
    Extension(this_ptr),
    CloakSound(VOC_NONE),
    UncloakSound(VOC_NONE),
    IsShakeScreen(false),
    IsImmuneToEMP(false),
    ShakePixelYHi(0),
    ShakePixelYLo(0),
    ShakePixelXHi(0),
    ShakePixelXLo(0),
    UnloadingClass(nullptr),
    SoylentValue(0),
    EnterTransportSound(VOC_NONE),
    LeaveTransportSound(VOC_NONE),
    IsSpawnsTiberiumOnDeath(true)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TechnoTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("TechnoTypeClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::TechnoTypeClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class deconstructor.
 *  
 *  @author: CCHyper
 */
TechnoTypeClassExtension::~TechnoTypeClassExtension()
{
    //DEV_DEBUG_TRACE("TechnoTypeClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("TechnoTypeClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoTypeClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TechnoTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TechnoTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TechnoTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TechnoTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    SwizzleManager.Swizzle((void **)UnloadingClass);

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int TechnoTypeClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TechnoTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TechnoTypeClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TechnoTypeClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TechnoTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TechnoTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    crc(IsShakeScreen);
    crc(IsImmuneToEMP);
    crc(ShakePixelYHi);
    crc(ShakePixelYLo);
    crc(ShakePixelXHi);
    crc(ShakePixelXLo);
    crc(SoylentValue);
    crc(IsSpawnsTiberiumOnDeath);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool TechnoTypeClassExtension::Read_INI(CCINIClass &ini)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("TechnoTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("TechnoTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    const char *ini_name = ThisPtr->Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }
    
    CloakSound = ini.Get_VocType(ini_name, "CloakSound", CloakSound);
    UncloakSound = ini.Get_VocType(ini_name, "UncloakSound", UncloakSound);
    IsShakeScreen = ini.Get_Bool(ini_name, "CanShakeScreen", IsShakeScreen);
    IsImmuneToEMP = ini.Get_Bool(ini_name, "ImmuneToEMP", IsImmuneToEMP);
    ShakePixelYHi = ini.Get_Int(ini_name, "ShakeYhi", ShakePixelYHi);
    ShakePixelYLo = ini.Get_Int(ini_name, "ShakeYlo", ShakePixelYLo);
    ShakePixelXHi = ini.Get_Int(ini_name, "ShakeXhi", ShakePixelXHi);
    ShakePixelXLo = ini.Get_Int(ini_name, "ShakeXlo", ShakePixelXLo);
    UnloadingClass = ini.Get_Techno(ini_name, "UnloadingClass", UnloadingClass);
    SoylentValue = ini.Get_Int(ini_name, "Soylent", SoylentValue);
    EnterTransportSound = ini.Get_VocType(ini_name, "EnterTransportSound", EnterTransportSound);
    LeaveTransportSound = ini.Get_VocType(ini_name, "LeaveTransportSound", LeaveTransportSound);
    IsSpawnsTiberiumOnDeath = ini.Get_Bool(ini_name, "TiberiumOnDeath", IsSpawnsTiberiumOnDeath);

    return true;
}
