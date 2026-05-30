/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended InfantryTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "infantrytypeext.h"

#include "ccini.h"
#include "extension.h"
#include "infantrytype.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
InfantryTypeClassExtension::InfantryTypeClassExtension(const InfantryTypeClass *this_ptr) :
    TechnoTypeClassExtension(this_ptr),
    IsMechanic(false),
    IsOmniHealer(false)
{
    InfantryTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
InfantryTypeClassExtension::InfantryTypeClassExtension(const NoInitClass &noinit) :
    TechnoTypeClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
InfantryTypeClassExtension::~InfantryTypeClassExtension()
{
    InfantryTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT InfantryTypeClassExtension::GetClassID(CLSID *lpClassID)
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
HRESULT InfantryTypeClassExtension::Load(IStream *pStm)
{
    HRESULT hr = TechnoTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) InfantryTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT InfantryTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
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
int InfantryTypeClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void InfantryTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    crc(IsMechanic);
    crc(IsOmniHealer);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool InfantryTypeClassExtension::Read_INI(CCINIClass &ini)
{
    if (!TechnoTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    IsMechanic = ini.Get_Bool(ini_name, "Mechanic", IsMechanic);
    IsOmniHealer = ini.Get_Bool(ini_name, "OmniHealer", IsOmniHealer);

    IsInitialized = true;
    
    return true;
}
