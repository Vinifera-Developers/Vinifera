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
    //if (this_ptr) EXT_DEBUG_TRACE("UnitClassExtension::UnitClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("UnitClassExtension::UnitClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
UnitClassExtension::~UnitClassExtension()
{
    //EXT_DEBUG_TRACE("UnitClassExtension::~UnitClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    UnitExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT UnitClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("UnitClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("UnitClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("UnitClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("UnitClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("UnitClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    crc(LastDockedBuilding != nullptr ? LastDockedBuilding->Fetch_ID() : 0);
}
