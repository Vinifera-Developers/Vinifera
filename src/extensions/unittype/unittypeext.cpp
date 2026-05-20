/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended UnitTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "unittypeext.h"

#include "ccini.h"
#include "debughandler.h"
#include "extension.h"
#include "findmake.h"
#include "tibsun_globals.h"
#include "unittype.h"
#include "vinifera_saveload.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
UnitTypeClassExtension::UnitTypeClassExtension(const UnitTypeClass *this_ptr) :
    TechnoTypeClassExtension(this_ptr),
    IsTotable(true),
    StartTurretFrame(-1),
    TurretFacings(32),		// Must default to 32 as all Tiberian Sun units have 32 facings for turrets.,
    StartIdleFrame(0),
    IdleFrames(0),
    TransformsInto(nullptr),
    IsTransformRequiresFullCharge(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("UnitTypeClassExtension::UnitTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    UnitTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
UnitTypeClassExtension::UnitTypeClassExtension(const NoInitClass &noinit) :
    TechnoTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::UnitTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
UnitTypeClassExtension::~UnitTypeClassExtension()
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::~UnitTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    UnitTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT UnitTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT UnitTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = TechnoTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) UnitTypeClassExtension(NoInitClass());
    
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(TransformsInto, "TransformsInto");

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT UnitTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = TechnoTypeClassExtension::Save(pStm, fClearDirty);
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
int UnitTypeClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void UnitTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool UnitTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("UnitTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (!TechnoTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    const char *ini_name = Name();
    const char *graphic_name = This()->Graphic_Name();
    
    //if (!ArtINI.Is_Present(graphic_name)) {
    //    return false;
    //}

    IsTotable = ini.Get_Bool(ini_name, "Totable", IsTotable);
    TransformsInto = TGet_Class(ini, ini_name, "TransformsInto", TransformsInto);
    IsTransformRequiresFullCharge = ini.Get_Bool(ini_name, "TransformRequiresFullCharge", IsTransformRequiresFullCharge);

    StartTurretFrame = ArtINI.Get_Int(graphic_name, "StartTurretFrame", StartTurretFrame);
    TurretFacings = ArtINI.Get_Int(graphic_name, "TurretFacings", TurretFacings);

    /**
     *  Set the defaults to walk frames (this ensures IdleRate by itself works as expected).
     */
    StartIdleFrame = This()->StartWalkFrame;
    IdleFrames = This()->WalkFrames;

    StartIdleFrame = ArtINI.Get_Int(graphic_name, "StartIdleFrame", StartIdleFrame);
    IdleFrames = ArtINI.Get_Int(graphic_name, "IdleFrames", IdleFrames);

    IsInitialized = true;

    return true;
}
