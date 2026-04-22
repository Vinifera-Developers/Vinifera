/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended IsometricTileTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "isotiletypeext.h"

#include "ccini.h"
#include "debughandler.h"
#include "extension.h"
#include "findmake.h"
#include "isotiletype.h"
#include "scenario.h"
#include "smudgetype.h"
#include "theatertype.h"
#include "tiberium.h"
#include "tibsun_globals.h"
#include "vinifera_saveload.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
IsometricTileTypeClassExtension::IsometricTileTypeClassExtension(const IsometricTileTypeClass *this_ptr) :
    ObjectTypeClassExtension(this_ptr),
    TileSetName(""),
    AllowedTiberiums(),
    AllowedSmudges(),
    IsAllowVeins(true),
    IsWaterTunnel(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("IsometricTileTypeClassExtension::~IsometricTileTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    IsometricTileTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
IsometricTileTypeClassExtension::IsometricTileTypeClassExtension(const NoInitClass &noinit) :
    ObjectTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("IsometricTileTypeClassExtension::~IsometricTileTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
IsometricTileTypeClassExtension::~IsometricTileTypeClassExtension()
{
    //EXT_DEBUG_TRACE("IsometricTileTypeClassExtension::~IsometricTileTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    IsometricTileTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT IsometricTileTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("IsometricTileTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT IsometricTileTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("IsometricTileTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    AllowedTiberiums.Clear();
    AllowedSmudges.Clear();

    HRESULT hr = ObjectTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) IsometricTileTypeClassExtension(NoInitClass());

    AllowedTiberiums.Load_Self(pStm);
    AllowedSmudges.Load_Self(pStm);

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(AllowedTiberiums, "AllowedTiberiums");
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(AllowedSmudges, "AllowedSmudges");
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT IsometricTileTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("IsometricTileTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    AllowedTiberiums.Save_Self(pStm);
    AllowedSmudges.Save_Self(pStm);

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int IsometricTileTypeClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("IsometricTileTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void IsometricTileTypeClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("IsometricTileTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ObjectTypeClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void IsometricTileTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("IsometricTileTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool IsometricTileTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("IsometricTileTypeClassExtension::Read_INI - Name: %s, TileSetName %s (0x%08X)\n", Name(), TileSetName, (uintptr_t)(This()));

    //if (!ObjectTypeClassExtension::Read_INI(ini)) {   // not required for this and causes it to return early
    //    return false;                                 // because individual tiles don't have their own sections
    //}

    const char *ini_name = TileSetName;

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    char buffer[1024];
    if (ini.Get_String(ini_name, "AllowedTiberiums", "", buffer, sizeof(buffer)) > 0) {
        AllowedTiberiums.Clear();
        char* token = std::strtok(buffer, ",");
        while (token != nullptr) {
            TiberiumType tiberium = TiberiumClass::From_Name(token);
            if (tiberium != TIBERIUM_NONE) {
                AllowedTiberiums.Add(tiberium);
            }
            token = std::strtok(nullptr, ",");
        }
    }

    auto smudges = TGet_TypeList(ini, ini_name, "AllowedSmudges", TypeList<SmudgeTypeClass*>());
    if (smudges.Count() > 0) {
        AllowedSmudges.Clear();
        for (auto smudge : smudges) {
            AllowedSmudges.Add(smudge->HeapID);
        }
    }

    IsAllowVeins = ini.Get_Bool(ini_name, "AllowVeins", IsAllowVeins);
    IsWaterTunnel = ini.Get_Bool(ini_name, "WaterTunnel", IsWaterTunnel);

    IsInitialized = true;
    
    return true;
}


bool IsometricTileTypeClassExtension::Init(CCINIClass &ini)
{
    static const char *GENERAL = "General";

    DEV_DEBUG_INFO("IsometricTileTypeClassExtension::Init(%s)\n", TheaterTypeClass::Name_From(Scen->Theater));

    if (!ini.Is_Present(GENERAL)) {
        return false;
    }
    
    return true;
}
