/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SUPERTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended SuperWeaponTypeClass class.
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
#include "supertypeext.h"
#include "supertype.h"
#include "vinifera_util.h"
#include "bsurface.h"
#include "ccini.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


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
    VoxMissileLaunched(VOX_MISSILE_LAUNCHED)
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
    AbstractTypeClassExtension(noinit)
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
    BSurface *imagesurface = Vinifera_Get_Image_Surface(SidebarImage);
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
    std::strncpy(SidebarImage, This()->SidebarImage, sizeof(SidebarImage));

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
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void SuperWeaponTypeClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("SuperWeaponTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
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
    BSurface *imagesurface = Vinifera_Get_Image_Surface(This()->SidebarImage);
    if (imagesurface) {
        CameoImageSurface = imagesurface;
    }

    ActionOutOfRange = ini.Get_ActionType(ini_name, "ActionOutOfRange", ActionOutOfRange);
    VoxMissileLaunched = ini.Get_VoxType(ini_name, "MissileLaunchedVoice", VoxMissileLaunched);

    IsInitialized = true;
    
    return true;
}
