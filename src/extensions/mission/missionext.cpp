/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended Mission class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "missionext.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
MissionClassExtension::MissionClassExtension(const MissionClass *this_ptr) :
    ObjectClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("MissionClassExtension::MissionClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
MissionClassExtension::MissionClassExtension(const NoInitClass &noinit) :
    ObjectClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("MissionClassExtension::MissionClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
MissionClassExtension::~MissionClassExtension()
{
    //EXT_DEBUG_TRACE("MissionClassExtension::~MissionClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT MissionClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("MissionClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Load(pStm);
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
HRESULT MissionClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("MissionClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Save(pStm, fClearDirty);
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
void MissionClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("MissionClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ObjectClassExtension::Object_CRC(crc);
}
