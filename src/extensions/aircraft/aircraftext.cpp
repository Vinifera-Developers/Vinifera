/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AIRCRAFTEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended AircraftClass class.
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
#include "aircraftext.h"
#include "aircraft.h"
#include "wwcrc.h"
#include "extension.h"
#include "vinifera_saveload.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
AircraftClassExtension::AircraftClassExtension(const AircraftClass *this_ptr) :
    FootClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("AircraftClassExtension::AircraftClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    AircraftExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
AircraftClassExtension::AircraftClassExtension(const NoInitClass &noinit) :
    FootClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("AircraftClassExtension::AircraftClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
AircraftClassExtension::~AircraftClassExtension()
{
    //EXT_DEBUG_TRACE("AircraftClassExtension::~AircraftClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    AircraftExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT AircraftClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("AircraftClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT AircraftClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("AircraftClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = FootClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) AircraftClassExtension(NoInitClass());
    
    if (Spawner != nullptr) {
        VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(Spawner, "Spawner");
    }

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT AircraftClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("AircraftClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = FootClassExtension::Save(pStm, fClearDirty);
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
int AircraftClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("AircraftClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void AircraftClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("AircraftClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void AircraftClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("AircraftClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}
