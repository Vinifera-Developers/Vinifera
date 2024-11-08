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
#include "saveload.h"
#include "vinifera_saveload.h"
#include "storageext.h"
#include "utracker.h"


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

    for (int i = 0; i < Tiberiums.Count(); i++)
    {
        TiberiumStorage[i] = 0;
        WeedStorage[i] = 0;
    }

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

    Load_Primitive_Vector(pStm, TiberiumStorage, "TiberiumStorage");
    Load_Primitive_Vector(pStm, WeedStorage, "WeedStorage");

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

    Save_Primitive_Vector(pStm, TiberiumStorage, "TiberiumStorage");
    Save_Primitive_Vector(pStm, WeedStorage, "WeedStorage");

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


/**
 *  Puts pointers to the storage extension into the storage class.
 *
 *  @author: ZivDero
 */
void HouseClassExtension::Put_Storage_Pointers()
{
    new (reinterpret_cast<StorageClassExt*>(&This()->Tiberium)) StorageClassExt(&TiberiumStorage);
    new (reinterpret_cast<StorageClassExt*>(&This()->Weed)) StorageClassExt(&WeedStorage);
}


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class UnitTrackerClassExt final : public UnitTrackerClass
{
public:
    HRESULT _Load(IStream* pStm);
    HRESULT _Save(IStream* pStm);
};


/**
 *  Saves a unit tracker's counts
 *
 *  @author: ZivDero
 */
HRESULT UnitTrackerClassExt::_Load(IStream* pStm)
{
    int count;
    HRESULT hr = pStm->Read(&count, sizeof(count), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    new (this) UnitTrackerClass(count);

    for (int i = 0; i < UnitCount; i++) {
        hr = pStm->Read(&UnitTotals[i], sizeof(UnitTotals[i]), nullptr);
        if (FAILED(hr)) {
            return hr;
        }

    }

    return S_OK;
}


/**
 *  Saves a unit tracker's counts.
 *
 *  @author: ZivDero
 */
HRESULT UnitTrackerClassExt::_Save(IStream* pStm)
{
    HRESULT hr = pStm->Write(&UnitCount, sizeof(UnitCount), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    for (int i = 0; i < UnitCount; i++) {
        HRESULT hr = pStm->Write(&UnitTotals[i], sizeof(UnitTotals[i]), nullptr);
        if (FAILED(hr)) {
            return hr;
        }

    }

    return S_OK;
}


/**
 *  Reads a house's unit trackers.
 *
 *  @author: ZivDero
 */
void HouseClassExtension::Load_Unit_Trackers(IStream* pStm)
{
    /**
     *  Trackers store their counts in a dynamically allocated array (AARGH WW!).
     *  Thus, we need to save/load them manually.
     *  But we can't do this in the extension because ThisPtr isn't remapped yet.
     */

    reinterpret_cast<UnitTrackerClassExt*>(This()->AircraftTotals)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->InfantryTotals)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->UnitTotals)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->BuildingTotals)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->DestroyedAircraft)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->DestroyedInfantry)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->DestroyedUnits)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->DestroyedBuildings)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->CapturedBuildings)->_Load(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->TotalCrates)->_Load(pStm);
}


/**
 *  Saves a house's unit trackers.
 *
 *  @author: ZivDero
 */
void HouseClassExtension::Save_Unit_Trackers(IStream* pStm)
{
    reinterpret_cast<UnitTrackerClassExt*>(This()->AircraftTotals)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->InfantryTotals)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->UnitTotals)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->BuildingTotals)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->DestroyedAircraft)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->DestroyedInfantry)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->DestroyedUnits)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->DestroyedBuildings)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->CapturedBuildings)->_Save(pStm);
    reinterpret_cast<UnitTrackerClassExt*>(This()->TotalCrates)->_Save(pStm);
}
