/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SmudgeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "smudgeext.h"

#include "extension.h"
#include "smudge.h"
#include "smudgetype.h"
#include "smudgetypeext.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
SmudgeClassExtension::SmudgeClassExtension(const SmudgeClass *this_ptr) :
    ObjectClassExtension(this_ptr)
{
    SmudgeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
SmudgeClassExtension::SmudgeClassExtension(const NoInitClass &noinit) :
    ObjectClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SmudgeClassExtension::~SmudgeClassExtension()
{
    SmudgeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT SmudgeClassExtension::GetClassID(CLSID *lpClassID)
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
HRESULT SmudgeClassExtension::Load(IStream *pStm)
{
    HRESULT hr = ObjectClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) SmudgeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT SmudgeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
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
int SmudgeClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void SmudgeClassExtension::Object_CRC(CRCEngine &crc) const
{
}
