/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ABSTRACTTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Base extension class for all type objects.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once

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
    FullName()
{
    //if (this_ptr) EXT_DEBUG_TRACE("AbstractTypeClassExtension::AbstractTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
AbstractTypeClassExtension::AbstractTypeClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit)
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
    std::strncpy(IniName, Name(), sizeof(IniName));
    std::strncpy(FullName, Full_Name(), sizeof(FullName));

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
