/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended BulletTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "bullettypeext.h"

#include "bullettype.h"
#include "ccini.h"
#include "extension.h"
#include "tibsun_globals.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
BulletTypeClassExtension::BulletTypeClassExtension(const BulletTypeClass *this_ptr) :
    ObjectTypeClassExtension(this_ptr),
    SpawnDelay(3),           // Default hardcoded value.
    IsTorpedo(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("BulletTypeClassExtension::BulletTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    BulletTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
BulletTypeClassExtension::BulletTypeClassExtension(const NoInitClass &noinit) :
    ObjectTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("BulletTypeClassExtension::BulletTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
BulletTypeClassExtension::~BulletTypeClassExtension()
{
    //EXT_DEBUG_TRACE("BulletTypeClassExtension::~BulletTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    BulletTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT BulletTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("BulletTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT BulletTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("BulletTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) BulletTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT BulletTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("BulletTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
int BulletTypeClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("BulletTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void BulletTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("BulletTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool BulletTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("BulletTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();
    const char *graphic_name = Graphic_Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }
    
    IsTorpedo = ini.Get_Bool(ini_name, "Torpedo", IsTorpedo);

    //if (!ArtINI.Is_Present(graphic_name)) {
    //    return false;
    //}

    /**
     *  The following keys are loaded from the ArtINI database.
     */
    SpawnDelay = ArtINI.Get_Int(graphic_name, "SpawnDelay", SpawnDelay);

    IsInitialized = true;
    
    return true;
}
