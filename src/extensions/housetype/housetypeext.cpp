/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOUSETYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended HouseTypeClass class.
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

#include "always.h"

#include "housetypeext.h"

#include "ccini.h"
#include "extension.h"
#include "housetype.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
HouseTypeClassExtension::HouseTypeClassExtension(const HouseTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("HouseTypeClassExtension::HouseTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HouseTypeExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
HouseTypeClassExtension::HouseTypeClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::HouseTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
HouseTypeClassExtension::~HouseTypeClassExtension()
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::~HouseTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HouseTypeExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT HouseTypeClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT HouseTypeClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) HouseTypeClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT HouseTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
int HouseTypeClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void HouseTypeClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void HouseTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool HouseTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    const char* ini_name = Name();

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    IsInitialized = true;
    
    return true;
}


/**
 *  Fetch house pointer from its name.
 *  Also takes care of ts-patches spawn houses.
 *
 *  @warning: Do not use the raw output if you expect spawn houses!
 *
 *  @author: ZivDero
 */
HousesType HouseTypeClassExtension::House_From_Name(char const* name)
{
    if (Session.Type != GAME_NORMAL) {
        int spawn_number;

        /**
         *  Try to read the house name as a spawn house name and extract its number.
         */
        if (std::sscanf(name, "Spawn%d", &spawn_number) == 1) {
            spawn_number--;
            if (spawn_number >= 0 && spawn_number < MAX_PLAYERS) {
                return static_cast<HousesType>(spawn_number + 50);
            }
        }
    }

    if (name != nullptr) {
        for (int house = HOUSE_FIRST; house < HouseTypes.Count(); house++) {
            HouseTypeClass* ptr = HouseTypes[house];
            if (ptr->GivenName == name || ptr->IniName == name) {
                return ptr->House;
            }
        }
    }

    return HOUSE_NONE;
}
