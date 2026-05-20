/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended Radio class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "radioext.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
RadioClassExtension::RadioClassExtension(const RadioClass *this_ptr) :
    MissionClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("RadioClassExtension::RadioClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
RadioClassExtension::RadioClassExtension(const NoInitClass &noinit) :
    MissionClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("RadioClassExtension::RadioClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
RadioClassExtension::~RadioClassExtension()
{
    //EXT_DEBUG_TRACE("RadioClassExtension::~RadioClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT RadioClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("RadioClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = MissionClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT RadioClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("RadioClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = MissionClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void RadioClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("RadioClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    MissionClassExtension::Object_CRC(crc);
}
