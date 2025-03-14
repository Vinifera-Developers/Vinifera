/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          THEMEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended ThemeClass class.
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
#include "themeext.h"
#include "theme.h"
#include "ccini.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
ThemeControlExtension::ThemeControlExtension(const ThemeClass::ThemeControl *this_ptr) :
    GlobalExtensionClass(this_ptr),
    RequiredAddon(ADDON_NONE)
{
    //EXT_DEBUG_TRACE("ThemeControlExtension::ThemeControlExtension - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
ThemeControlExtension::ThemeControlExtension(const NoInitClass &noinit) :
    GlobalExtensionClass(noinit)
{
    //EXT_DEBUG_TRACE("ThemeControlExtension::ThemeControlExtension(NoInitClass) - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
ThemeControlExtension::~ThemeControlExtension()
{
    //EXT_DEBUG_TRACE("ThemeControlExtension::~ThemeControlExtension - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT ThemeControlExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("ThemeControlExtension::Load - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = GlobalExtensionClass::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) ThemeControlExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT ThemeControlExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("ThemeControlExtension::Save - 0x%08X\n", (uintptr_t)(This()));

    HRESULT hr = GlobalExtensionClass::Save(pStm, fClearDirty);
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
int ThemeControlExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("ThemeControlExtension::Size_Of - 0x%08X\n", (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void ThemeControlExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("ThemeControlExtension::Detach - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void ThemeControlExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("ThemeControlExtension::Compute_CRC - 0x%08X\n", (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool ThemeControlExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("ThemeControlExtension::Read_INI - Name: %s (0x%08X)\n", This()->Name, (uintptr_t)(This()));

    const char *ini_name = This()->Name;

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    RequiredAddon = (AddonType)ini.Get_Int(ini_name, "RequiredAddon", RequiredAddon);

    IsInitialized = true;

    return true;
}
