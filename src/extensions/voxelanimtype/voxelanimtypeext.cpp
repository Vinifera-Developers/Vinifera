/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended VoxelAnimTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "voxelanimtypeext.h"

#include "ccini.h"
#include "extension.h"
#include "voxelanimtype.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
VoxelAnimTypeClassExtension::VoxelAnimTypeClassExtension(const VoxelAnimTypeClass *this_ptr) :
    ObjectTypeClassExtension(this_ptr),
    StopSound(VOC_NONE)
{
    VoxelAnimTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
VoxelAnimTypeClassExtension::VoxelAnimTypeClassExtension(const NoInitClass &noinit) :
    ObjectTypeClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
VoxelAnimTypeClassExtension::~VoxelAnimTypeClassExtension()
{
    VoxelAnimTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT VoxelAnimTypeClassExtension::GetClassID(CLSID *lpClassID)
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
HRESULT VoxelAnimTypeClassExtension::Load(IStream *pStm)
{
    HRESULT hr = ObjectTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) VoxelAnimTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT VoxelAnimTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
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
int VoxelAnimTypeClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void VoxelAnimTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    crc(StopSound);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool VoxelAnimTypeClassExtension::Read_INI(CCINIClass &ini)
{
    if (!ObjectTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    StopSound = ini.Get_VocType(ini_name, "StopSound", StopSound);

    IsInitialized = true;
    
    return true;
}
