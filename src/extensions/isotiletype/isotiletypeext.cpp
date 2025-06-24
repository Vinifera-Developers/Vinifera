/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ISOTILETYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended IsometricTileTypeClass class.
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
#include "isotiletypeext.h"
#include "isotiletype.h"
#include "tibsun_globals.h"
#include "scenario.h"
#include "theatertype.h"
#include "ccini.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "tiberium.h"
#include "vinifera_saveload.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
IsometricTileTypeClassExtension::IsometricTileTypeClassExtension(const IsometricTileTypeClass *this_ptr) :
    ObjectTypeClassExtension(this_ptr),
    TileSetName(""),
    AllowedTiberiums()
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

    HRESULT hr = ObjectTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) IsometricTileTypeClassExtension(NoInitClass());

    AllowedTiberiums.Load(pStm);

    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP_LIST(AllowedTiberiums, "AllowedTiberiums");
    
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

    AllowedTiberiums.Save(pStm);

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

    if (AllowedTiberiums.Is_Present(reinterpret_cast<TiberiumClass*>(target))) {
        AllowedTiberiums.Delete(reinterpret_cast<TiberiumClass*>(target));
    }
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
                AllowedTiberiums.Add(Tiberiums[tiberium]);
            }
            token = std::strtok(nullptr, ",");
        }
    }

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
