/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEAMTYPEEXT.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Extended TeamTypeClass class.
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
#include "teamtypeext.h"
#include "teamtype.h"
#include "ccini.h"
#include "wwcrc.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: Rampastring
 */
TeamTypeClassExtension::TeamTypeClassExtension(const TeamTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("TeamTypeClassExtension::TeamTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TeamTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: Rampastring
 */
TeamTypeClassExtension::TeamTypeClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("TeamTypeClassExtension::TeamTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: Rampastring
 */
TeamTypeClassExtension::~TeamTypeClassExtension()
{
    //EXT_DEBUG_TRACE("TeamTypeClassExtension::~TeamTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TeamTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: Rampastring
 */
HRESULT TeamTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("TeamTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: Rampastring
 */
HRESULT TeamTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("TeamTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TeamTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: Rampastring
 */
HRESULT TeamTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("TeamTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: Rampastring
 */
int TeamTypeClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("TeamTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: Rampastring
 */
void TeamTypeClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("TeamTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: Rampastring
 */
void TeamTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TeamTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: Rampastring
 */
bool TeamTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("TeamTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    const char *ini_name = Name();

    IsInitialized = true;
    
    return true;
}
