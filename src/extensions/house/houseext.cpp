/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOUSEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended HouseClass class.
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
#include "houseext.h"
#include "house.h"
#include "ccini.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "storage/storageext.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
HouseClassExtension::HouseClassExtension(const HouseClass *this_ptr) :
    AbstractClassExtension(this_ptr),
    TiberiumStorage(Tiberiums.Count()),
    WeedStorage(Tiberiums.Count())
{
    //if (this_ptr) EXT_DEBUG_TRACE("HouseClassExtension::HouseClassExtension - 0x%08X\n", (uintptr_t)(This()));

    if (this_ptr)
    {
        new ((StorageClassExt*)&(this_ptr->Tiberium)) StorageClassExt(&TiberiumStorage);
        new ((StorageClassExt*)&(this_ptr->Weed)) StorageClassExt(&WeedStorage);
    }

    HouseExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
HouseClassExtension::HouseClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit),
    TiberiumStorage(noinit),
    WeedStorage(noinit)
{
    //EXT_DEBUG_TRACE("HouseClassExtension::HouseClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
HouseClassExtension::~HouseClassExtension()
{
    //EXT_DEBUG_TRACE("HouseClassExtension::~HouseClassExtension - 0x%08X\n", (uintptr_t)(This()));

    HouseExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT HouseClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("HouseClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT HouseClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("HouseClassExtension::Load - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) HouseClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT HouseClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("HouseClassExtension::Save - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Save(pStm, fClearDirty);
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
int HouseClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("HouseClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void HouseClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("HouseClassExtension::Detach - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void HouseClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("HouseClassExtension::Compute_CRC - 0x%08X\n", (uintptr_t)(This()));
}

void HouseClassExtension::Put_Storage_Pointers()
{
    new ((StorageClassExt*)&(This()->Tiberium)) StorageClassExt(&TiberiumStorage);
    new ((StorageClassExt*)&(This()->Weed)) StorageClassExt(&WeedStorage);
}
