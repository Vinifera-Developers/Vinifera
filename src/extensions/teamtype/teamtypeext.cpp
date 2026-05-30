/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TeamTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "teamtypeext.h"

#include "ccini.h"
#include "extension.h"
#include "teamtype.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: Rampastring
 */
TeamTypeClassExtension::TeamTypeClassExtension(const TeamTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr)
{
    TeamTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: Rampastring
 */
TeamTypeClassExtension::TeamTypeClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: Rampastring
 */
TeamTypeClassExtension::~TeamTypeClassExtension()
{
    TeamTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: Rampastring
 */
HRESULT TeamTypeClassExtension::GetClassID(CLSID *lpClassID)
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
 *  @author: Rampastring
 */
HRESULT TeamTypeClassExtension::Load(IStream *pStm)
{
    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TeamTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: Rampastring
 */
HRESULT TeamTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = AbstractTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: Rampastring
 */
int TeamTypeClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: Rampastring
 */
void TeamTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: Rampastring
 */
bool TeamTypeClassExtension::Read_INI(CCINIClass &ini)
{
    const char *ini_name = Name();

    IsInitialized = true;
    
    return true;
}
