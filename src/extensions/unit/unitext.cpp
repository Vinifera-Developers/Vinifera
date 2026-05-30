/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended UnitClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "unitext.h"

#include "building.h"
#include "debughandler.h"
#include "extension.h"
#include "unit.h"
#include "vinifera_saveload.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
UnitClassExtension::UnitClassExtension(const UnitClass *this_ptr) :
    FootClassExtension(this_ptr),
    Vinifera::Detach::Listener<BuildingClass>(),
    LastDockedBuilding(nullptr)
{
    UnitExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
UnitClassExtension::UnitClassExtension(const NoInitClass &noinit) :
    FootClassExtension(noinit),
    Vinifera::Detach::Listener<BuildingClass>(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
UnitClassExtension::~UnitClassExtension()
{
    UnitExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT UnitClassExtension::GetClassID(CLSID *lpClassID)
{
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
HRESULT UnitClassExtension::Load(IStream *pStm)
{
    HRESULT hr = FootClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) UnitClassExtension(NoInitClass());
    
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(LastDockedBuilding, "LastDockedBuilding");

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT UnitClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
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
int UnitClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}


/**
 *  Removes the specified building from any targeting and reference trackers.
 */
void UnitClassExtension::On_Detach(BuildingClass *target, bool all)
{
    if (LastDockedBuilding == target) {
        LastDockedBuilding = nullptr;
    }
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void UnitClassExtension::Object_CRC(CRCEngine &crc) const
{
    crc(LastDockedBuilding != nullptr ? LastDockedBuilding->Fetch_ID() : 0);
}
