/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended AircraftTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "aircrafttypeext.h"

#include "aircrafttype.h"
#include "ccini.h"
#include "extension.h"
#include "rules.h"
#include "tibsun_globals.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
AircraftTypeClassExtension::AircraftTypeClassExtension(const AircraftTypeClass *this_ptr) :
    TechnoTypeClassExtension(this_ptr),
    IsCurleyShuffle(false), // Same as the RulesClass default.
    ReloadRate(0.05f) // Same as the RulesClass default.
{
    //if (this_ptr) EXT_DEBUG_TRACE("AircraftTypeClassExtension::AircraftTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    AircraftTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
AircraftTypeClassExtension::AircraftTypeClassExtension(const NoInitClass &noinit) :
    TechnoTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("AircraftTypeClassExtension::AircraftTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
AircraftTypeClassExtension::~AircraftTypeClassExtension()
{
    //EXT_DEBUG_TRACE("AircraftTypeClassExtension::~AircraftTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    AircraftTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT AircraftTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("AircraftTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT AircraftTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("AircraftTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = TechnoTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) AircraftTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT AircraftTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("AircraftTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = TechnoTypeClassExtension::Save(pStm, fClearDirty);
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
int AircraftTypeClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("AircraftTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void AircraftTypeClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("AircraftTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TechnoTypeClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void AircraftTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("AircraftTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool AircraftTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("AircraftTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!TechnoTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    IsCurleyShuffle = ini.Get_Bool(ini_name, "CurleyShuffle", Rule->IsCurleyShuffle);
    ReloadRate = ini.Get_Float(ini_name, "ReloadRate", Rule->ReloadRate);

    IsInitialized = true;

    return true;
}
