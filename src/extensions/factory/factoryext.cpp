/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended FactoryClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "factoryext.h"

#include "extension.h"
#include "factory.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
FactoryClassExtension::FactoryClassExtension(const FactoryClass *this_ptr) :
    AbstractClassExtension(this_ptr),
    IsHoldingExit(false),
    HasSpoken(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("FactoryClassExtension::FactoryClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    FactoryExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
FactoryClassExtension::FactoryClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::FactoryClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
FactoryClassExtension::~FactoryClassExtension()
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::~FactoryClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    FactoryExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT FactoryClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT FactoryClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) FactoryClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT FactoryClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
int FactoryClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void FactoryClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("FactoryClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}
