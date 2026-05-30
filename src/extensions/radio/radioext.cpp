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
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
RadioClassExtension::RadioClassExtension(const NoInitClass &noinit) :
    MissionClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
RadioClassExtension::~RadioClassExtension()
{
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT RadioClassExtension::Load(IStream *pStm)
{
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
    MissionClassExtension::Object_CRC(crc);
}
