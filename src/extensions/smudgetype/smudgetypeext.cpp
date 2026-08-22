/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SmudgeTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "smudgetypeext.h"

#include "ccini.h"
#include "extension.h"
#include "smudgetype.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
SmudgeTypeClassExtension::SmudgeTypeClassExtension(const SmudgeTypeClass *this_ptr) :
    ObjectTypeClassExtension(this_ptr)
{
    SmudgeTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
SmudgeTypeClassExtension::SmudgeTypeClassExtension(const NoInitClass &noinit) :
    ObjectTypeClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SmudgeTypeClassExtension::~SmudgeTypeClassExtension()
{
    SmudgeTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT SmudgeTypeClassExtension::GetClassID(CLSID *lpClassID)
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
HRESULT SmudgeTypeClassExtension::Load(IStream *pStm)
{
    HRESULT hr = ObjectTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) SmudgeTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT SmudgeTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = ObjectTypeClassExtension::Save(pStm, fClearDirty);
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
int SmudgeTypeClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void SmudgeTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool SmudgeTypeClassExtension::Read_INI(CCINIClass &ini)
{
    if (!ObjectTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    IsInitialized = true;
    
    return true;
}
