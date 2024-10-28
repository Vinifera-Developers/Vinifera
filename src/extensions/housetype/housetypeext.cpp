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
#include "housetypeext.h"
#include "housetype.h"
#include "ccini.h"
#include "extension.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
HouseTypeClassExtension::HouseTypeClassExtension(const HouseTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    LoadingScreens { }
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
    AbstractTypeClassExtension(noinit),
    LoadingScreens { }
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

    LoadingScreens[0].Clear();
    LoadingScreens[1].Clear();
    LoadingScreens[2].Clear();

    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) HouseTypeClassExtension(NoInitClass());

    /**
     *  We don't need loading screens during the game so we don't bother saving and loading them.
     */
    new (&LoadingScreens[0]) DynamicVectorClass<Wstring>();
    new (&LoadingScreens[1]) DynamicVectorClass<Wstring>();
    new (&LoadingScreens[2]) DynamicVectorClass<Wstring>();
    
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
int HouseTypeClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void HouseTypeClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void HouseTypeClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("HouseTypeClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
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

    if (!IsInitialized) {

        char buffer[12];

        LoadingScreens[0].Clear();
        LoadingScreens[1].Clear();
        LoadingScreens[2].Clear();

        for (int i = 0; i < 2; i++)
        {
            const char letter_count = 26;
            char letter;

            if (This()->House == HOUSE_GDI) {
                letter = 'C' + i;
            }
            else if (This()->House == HOUSE_NOD) {
                letter = 'A' + i;
            }
            else {
                letter = 'A' + ((static_cast<char>(This()->House) * 2) % letter_count) + i;
            }

            std::sprintf(buffer, "LOAD400%c", letter);
            LoadingScreens[0].Add(buffer);

            std::sprintf(buffer, "LOAD480%c", letter);
            LoadingScreens[1].Add(buffer);

            std::sprintf(buffer, "LOAD600%c", letter);
            LoadingScreens[2].Add(buffer);
        }
    }

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    LoadingScreens[0] = ini.Get_Strings(ini_name, "LoadingScreens400", LoadingScreens[0]);
    LoadingScreens[1] = ini.Get_Strings(ini_name, "LoadingScreens480", LoadingScreens[1]);
    LoadingScreens[2] = ini.Get_Strings(ini_name, "LoadingScreens600", LoadingScreens[2]);

    IsInitialized = true;
    
    return true;
}
