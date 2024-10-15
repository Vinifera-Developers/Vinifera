/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OVERLAYEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended OverlayClass class.
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
#include "overlayext.h"
#include "overlay.h"
#include "overlaytype.h"
#include "overlaytypeext.h"
#include "wwcrc.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
OverlayClassExtension::OverlayClassExtension(const OverlayClass *this_ptr) :
    ObjectClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("OverlayClassExtension::OverlayClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    OverlayExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
OverlayClassExtension::OverlayClassExtension(const NoInitClass &noinit) :
    ObjectClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("OverlayClassExtension::OverlayClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
OverlayClassExtension::~OverlayClassExtension()
{
    //EXT_DEBUG_TRACE("OverlayClassExtension::~OverlayClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    OverlayExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT OverlayClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("OverlayClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT OverlayClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("OverlayClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) OverlayClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT OverlayClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("OverlayClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Save(pStm, fClearDirty);
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
int OverlayClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("OverlayClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void OverlayClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("OverlayClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ObjectClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void OverlayClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("OverlayClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}
