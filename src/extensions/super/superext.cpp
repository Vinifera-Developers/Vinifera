/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SuperClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "superext.h"

#include "extension.h"
#include "super.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
SuperClassExtension::SuperClassExtension(const SuperClass *this_ptr) :
    AbstractClassExtension(this_ptr),
    FlashTimeEnd(0),
    TimerFlashState(false)
{
    SuperExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
SuperClassExtension::SuperClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SuperClassExtension::~SuperClassExtension()
{
    SuperExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT SuperClassExtension::GetClassID(CLSID *lpClassID)
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
HRESULT SuperClassExtension::Load(IStream *pStm)
{
    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) SuperClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT SuperClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
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
int SuperClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void SuperClassExtension::Object_CRC(CRCEngine &crc) const
{
}
