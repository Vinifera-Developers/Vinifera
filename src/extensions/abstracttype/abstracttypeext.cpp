/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Base extension class for all type objects.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "abstracttypeext.h"

#include "abstracttype.h"
#include "ccini.h"
#include "extension.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
AbstractTypeClassExtension::AbstractTypeClassExtension(const AbstractTypeClass *this_ptr) :
    AbstractClassExtension(this_ptr),
    IniName(),
    GivenName(),
    IsInitialized(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("AbstractTypeClassExtension::AbstractTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
AbstractTypeClassExtension::AbstractTypeClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit),
    IniName(noinit),
    GivenName(noinit)
{
    //EXT_DEBUG_TRACE("AbstractTypeClassExtension::AbstractTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
AbstractTypeClassExtension::~AbstractTypeClassExtension()
{
    //EXT_DEBUG_TRACE("AbstractTypeClassExtension::~AbstractTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT AbstractTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("AbstractTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return hr;
    }
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT AbstractTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("AbstractTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
    
    /**
     *  Store the name strings as raw data, these are used by the load operation.
     */
    IniName = Name();
    GivenName = Full_Name();

    HRESULT hr = AbstractClassExtension::Internal_Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Returns the name of this object type.
 *  
 *  @author: CCHyper
 */
const char *AbstractTypeClassExtension::Name() const
{
    //EXT_DEBUG_TRACE("AbstractTypeClassExtension::Name - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    const char *name = reinterpret_cast<const AbstractTypeClass *>(This())->Name();
    //EXT_DEBUG_INFO("AbstractTypeClassExtension: Name -> %s.\n", name);
    return name;
}


/**
 *  Returns the full name of this object type.
 *  
 *  @author: CCHyper
 */
const char *AbstractTypeClassExtension::Full_Name() const
{
    //EXT_DEBUG_TRACE("AbstractTypeClassExtension::Full_Name - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    const char *name = reinterpret_cast<const AbstractTypeClass *>(This())->Full_Name();
    //EXT_DEBUG_INFO("AbstractTypeClassExtension: Full_Name -> %s.\n", name);
    return name;
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool AbstractTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("AbstractTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    const char *ini_name = Name();

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    return true;
}
