/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INFANTRYEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended InfantryClass class.
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
#include "infantryext.h"
#include "infantry.h"
#include "wwcrc.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Provides the map for all InfantryClass extension instances.
 */
ExtensionMap<InfantryClass, InfantryClassExtension> InfantryClassExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
InfantryClassExtension::InfantryClassExtension(InfantryClass *this_ptr) :
    Extension(this_ptr)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("InfantryClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("InfantryClassExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
InfantryClassExtension::InfantryClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class deconstructor.
 *  
 *  @author: CCHyper
 */
InfantryClassExtension::~InfantryClassExtension()
{
    //DEV_DEBUG_TRACE("InfantryClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
    //DEV_DEBUG_WARNING("InfantryClassExtension deconstructor - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT InfantryClassExtension::Load(IStream *pStm)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("InfantryClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    HRESULT hr = Extension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) InfantryClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT InfantryClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("InfantryClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

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
int InfantryClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("InfantryClassExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void InfantryClassExtension::Detach(TARGET target, bool all)
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("InfantryClassExtension::Detach - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void InfantryClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    ASSERT(ThisPtr != nullptr);
    //DEV_DEBUG_TRACE("InfantryClassExtension::Compute_CRC - Name: %s (0x%08X)\n", ThisPtr->Name(), (uintptr_t)(ThisPtr));
}
