/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VOXELANIMTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended VoxelAnimTypeClass class.
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
#include "voxelanimtypeext.h"
#include "voxelanimtype.h"
#include "ccini.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
VoxelAnimTypeClassExtension::VoxelAnimTypeClassExtension(const VoxelAnimTypeClass *this_ptr) :
    ObjectTypeClassExtension(this_ptr),
    StopSound(VOC_NONE)
{
    //if (this_ptr) EXT_DEBUG_TRACE("VoxelAnimTypeClassExtension::VoxelAnimTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("VoxelAnimTypeClassExtension::VoxelAnimTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
VoxelAnimTypeClassExtension::~VoxelAnimTypeClassExtension()
{
    //EXT_DEBUG_TRACE("VoxelAnimTypeClassExtension::~VoxelAnimTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    VoxelAnimTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT VoxelAnimTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("VoxelAnimTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("VoxelAnimTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("VoxelAnimTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("VoxelAnimTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void VoxelAnimTypeClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("VoxelAnimTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ObjectTypeClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void VoxelAnimTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("VoxelAnimTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    crc(StopSound);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool VoxelAnimTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("VoxelAnimTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
