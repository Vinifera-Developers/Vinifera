/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SessionClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sessionext.h"

#include "ccini.h"
#include "noinit.h"
#include "vinifera_saveload.h"
#include "voc.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
SessionClassExtension::SessionClassExtension(const SessionClass *this_ptr) :
    GlobalExtensionClass(this_ptr),
    ExtOptions(),
    IsChatToAllies(false),
    MessageRecipientName("")
{
   /**
     *  Initialises the default game options.
     */
    ExtOptions.IsAutoDeployMCV = false;
    ExtOptions.IsPrePlacedConYards = false;
    ExtOptions.IsBuildOffAlly = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
SessionClassExtension::SessionClassExtension(const NoInitClass &noinit) :
    GlobalExtensionClass(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SessionClassExtension::~SessionClassExtension()
{
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT SessionClassExtension::Load(IStream *pStm)
{
    HRESULT hr = GlobalExtensionClass::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) SessionClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT SessionClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = GlobalExtensionClass::Save(pStm, fClearDirty);
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
int SessionClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void SessionClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void SessionClassExtension::Read_MultiPlayer_Settings()
{
}


/**
 *  Saves the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void SessionClassExtension::Write_MultiPlayer_Settings()
{
}
