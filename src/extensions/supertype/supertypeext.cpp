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
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SuperWeaponTypeClassExtension::~SuperWeaponTypeClassExtension()
{
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
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void SuperWeaponTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool SuperWeaponTypeClassExtension::Read_INI(CCINIClass &ini)
{
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
