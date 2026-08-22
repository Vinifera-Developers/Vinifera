/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended ParticleSystemTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "particlesystypeext.h"

#include "ccini.h"
#include "extension.h"
#include "particlesystype.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
ParticleSystemTypeClassExtension::ParticleSystemTypeClassExtension(const ParticleSystemTypeClass *this_ptr) :
    ObjectTypeClassExtension(this_ptr)
{
    ParticleSystemTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
ParticleSystemTypeClassExtension::ParticleSystemTypeClassExtension(const NoInitClass &noinit) :
    ObjectTypeClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
ParticleSystemTypeClassExtension::~ParticleSystemTypeClassExtension()
{
    ParticleSystemTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT ParticleSystemTypeClassExtension::GetClassID(CLSID *lpClassID)
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
HRESULT ParticleSystemTypeClassExtension::Load(IStream *pStm)
{
    HRESULT hr = ObjectTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) ParticleSystemTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT ParticleSystemTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
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
int ParticleSystemTypeClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void ParticleSystemTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool ParticleSystemTypeClassExtension::Read_INI(CCINIClass &ini)
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
