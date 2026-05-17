/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended WaveClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "waveext.h"

#include "extension.h"
#include "wave.h"
#include "wwcrc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
WaveClassExtension::WaveClassExtension(const WaveClass *this_ptr) :
    ObjectClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("WaveClassExtension::WaveClassExtension - 0x%08X\n", (uintptr_t)(This()));

    WaveExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
WaveClassExtension::WaveClassExtension(const NoInitClass &noinit) :
    ObjectClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("WaveClassExtension::WaveClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
WaveClassExtension::~WaveClassExtension()
{
    //EXT_DEBUG_TRACE("WaveClassExtension::~WaveClassExtension - 0x%08X\n", (uintptr_t)(This()));

    WaveExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT WaveClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("WaveClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT WaveClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("WaveClassExtension::Load - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) WaveClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT WaveClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("WaveClassExtension::Save - 0x%08X\n", (uintptr_t)(This()));

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
int WaveClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("WaveClassExtension::Get_Object_Size - 0x%08X\n", (uintptr_t)(This()));

    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void WaveClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("WaveClassExtension::Object_CRC - 0x%08X\n", (uintptr_t)(This()));
}
