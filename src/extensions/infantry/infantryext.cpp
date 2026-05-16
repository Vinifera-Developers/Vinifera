/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended InfantryClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "infantryext.h"

#include "extension.h"
#include "infantry.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
InfantryClassExtension::InfantryClassExtension(const InfantryClass *this_ptr) :
    FootClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("InfantryClassExtension::InfantryClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    InfantryExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
InfantryClassExtension::InfantryClassExtension(const NoInitClass &noinit) :
    FootClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("InfantryClassExtension::InfantryClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
InfantryClassExtension::~InfantryClassExtension()
{
    //EXT_DEBUG_TRACE("InfantryClassExtension::~InfantryClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    InfantryExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT InfantryClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("InfantryClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT InfantryClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("InfantryClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = FootClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) InfantryClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT InfantryClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("InfantryClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
int InfantryClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("InfantryClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void InfantryClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("InfantryClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}
