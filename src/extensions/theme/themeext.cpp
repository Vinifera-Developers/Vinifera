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
 *  Provides the map for all ThemeControlClass extension instances.
 */
ExtensionMap<ThemeClass::ThemeControl, ThemeControlExtension> ThemeControlExtensions;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
ThemeControlExtension::ThemeControlExtension(ThemeClass::ThemeControl *this_ptr) :
    Extension(this_ptr),
    RequiredAddon(ADDON_NONE)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("ThemeControlExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name, (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("ThemeControlExtension constructor - Name: %s (0x%08X)\n", ThisPtr->Name, (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
ThemeControlExtension::ThemeControlExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
ThemeControlExtension::~ThemeControlExtension()
{
    //EXT_DEBUG_TRACE("ThemeControlExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name, (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("ThemeControlExtension destructor - Name: %s (0x%08X)\n", ThisPtr->Name, (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int ThemeControlExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("ThemeControlExtension::Size_Of - Name: %s (0x%08X)\n", ThisPtr->Name, (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool ThemeControlExtension::Read_INI(CCINIClass &ini)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("ThemeControlExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name, (uintptr_t)(ThisPtr));
    EXT_DEBUG_WARNING("ThemeControlExtension::Read_INI - Name: %s (0x%08X)\n", ThisPtr->Name, (uintptr_t)(ThisPtr));

    const char *ini_name = ThisPtr->Name;

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    RequiredAddon = (AddonType)ini.Get_Int(ini_name, "RequiredAddon", RequiredAddon);
    
    return true;
}
