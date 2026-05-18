/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SuperWeaponTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "supertypeext.h"

#include "bsurface.h"
#include "ccini.h"
#include "extension.h"
#include "supertype.h"
#include "vinifera_util.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
SuperWeaponTypeClassExtension::SuperWeaponTypeClassExtension(const SuperWeaponTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    SidebarImage(),
    IsShowTimer(false),
    CameoImageSurface(nullptr),
    ActionOutOfRange(ACTION_EMPULSE_RANGE),
    VoxMissileLaunched(VOX_MISSILE_LAUNCHED),
    Description("")
{
    //if (this_ptr) EXT_DEBUG_TRACE("SuperWeaponTypeClassExtension::SuperWeaponTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    SuperWeaponTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
SuperWeaponTypeClassExtension::SuperWeaponTypeClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit),
    SidebarImage(noinit)
{
    //EXT_DEBUG_TRACE("SuperWeaponTypeClassExtension::SuperWeaponTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SuperWeaponTypeClassExtension::~SuperWeaponTypeClassExtension()
{
    //EXT_DEBUG_TRACE("SuperWeaponTypeClassExtension::~SuperWeaponTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    delete CameoImageSurface;
    CameoImageSurface = nullptr;

    SuperWeaponTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT SuperWeaponTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("SuperWeaponTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT SuperWeaponTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("SuperWeaponTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) SuperWeaponTypeClassExtension(NoInitClass());

    /**
     *  Fetch the cameo image surface if it exists.
     */
    BSurface *imagesurface = Vinifera_Get_Image_Surface(SidebarImage.c_str());
    if (imagesurface) {
        CameoImageSurface = imagesurface;
    }
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT SuperWeaponTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("SuperWeaponTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    /**
     *  Store the graphic name strings as raw data, these are used by the load operation.
     */
    SidebarImage = This()->SidebarImage;

    HRESULT hr = AbstractTypeClassExtension::Save(pStm, fClearDirty);
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
int SuperWeaponTypeClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("SuperWeaponTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void SuperWeaponTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("SuperWeaponTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool SuperWeaponTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("SuperWeaponTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();

    IsShowTimer = ini.Get_Bool(ini_name, "ShowTimer", IsShowTimer);

    /**
     *  Fetch the cameo image surface if it exists.
     */
    BSurface *imagesurface = Vinifera_Get_Image_Surface(This()->SidebarImage.c_str());
    if (imagesurface) {
        CameoImageSurface = imagesurface;
    }

    ActionOutOfRange = ini.Get_ActionType(ini_name, "ActionOutOfRange", ActionOutOfRange);
    VoxMissileLaunched = ini.Get_VoxType(ini_name, "MissileLaunchedVoice", VoxMissileLaunched);

    if (ini.Is_Present(ini_name, "Description")) ini.Get_String(ini_name, "Description", "", Description, std::size(Description));

    IsInitialized = true;
    
    return true;
}
