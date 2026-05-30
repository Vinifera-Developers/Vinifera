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
#include "vinifera_saveload.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
AircraftTypeClassExtension::AircraftTypeClassExtension(const AircraftTypeClass *this_ptr) :
    TechnoTypeClassExtension(this_ptr),
    IsCurleyShuffle(std::nullopt),
    ReloadRate(std::nullopt)
{
    AircraftTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
AircraftTypeClassExtension::AircraftTypeClassExtension(const NoInitClass &noinit) :
    TechnoTypeClassExtension(noinit),
    IsCurleyShuffle(std::nullopt),
    ReloadRate(std::nullopt)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
AircraftTypeClassExtension::~AircraftTypeClassExtension()
{
    AircraftTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT AircraftTypeClassExtension::GetClassID(CLSID *lpClassID)
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
HRESULT AircraftTypeClassExtension::Load(IStream *pStm)
{
    HRESULT hr = TechnoTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) AircraftTypeClassExtension(NoInitClass());

    if (FAILED(hr = Read_Optional(pStm, IsCurleyShuffle))) return hr;
    if (FAILED(hr = Read_Optional(pStm, ReloadRate))) return hr;

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT AircraftTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = TechnoTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    if (FAILED(hr = Put_Optional(pStm, IsCurleyShuffle))) return hr;
    if (FAILED(hr = Put_Optional(pStm, ReloadRate))) return hr;

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int AircraftTypeClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void AircraftTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.
 *
 *  @author: CCHyper
 */
bool AircraftTypeClassExtension::Read_INI(CCINIClass &ini)
{
    if (!TechnoTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    if (ini.Is_Present(ini_name, "CurleyShuffle")) {
        IsCurleyShuffle = ini.Get_Bool(ini_name, "CurleyShuffle", Get_IsCurleyShuffle());
    }
    if (ini.Is_Present(ini_name, "ReloadRate")) {
        ReloadRate = ini.Get_Float(ini_name, "ReloadRate", Get_ReloadRate());
    }

    IsInitialized = true;

    return true;
}
