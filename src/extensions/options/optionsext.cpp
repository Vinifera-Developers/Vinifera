/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OPTIONSEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended OptionsClass class.
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
#include "optionsext.h"
#include "options.h"
#include "tibsun_globals.h"
#include "noinit.h"
#include "options.h"
#include "ccini.h"
#include "rawfile.h"
#include "asserthandler.h"
#include "debughandler.h"


OptionsClassExtension *OptionsExtension = nullptr;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
OptionsClassExtension::OptionsClassExtension(OptionsClass *this_ptr) :
    Extension(this_ptr)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OptionsClassExtension constructor - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("OptionsClassExtension constructor - 0x%08X\n", (uintptr_t)(ThisPtr));

    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
OptionsClassExtension::OptionsClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class deconstructor.
 *  
 *  @author: CCHyper
 */
OptionsClassExtension::~OptionsClassExtension()
{
    //EXT_DEBUG_TRACE("OptionsClassExtension deconstructor - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("OptionsClassExtension deconstructor - 0x%08X\n", (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int OptionsClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OptionsClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Load_Settings()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OptionsClassExtension::Load_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("OptionsClassExtension::Load_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
    
    RawFileClass file("SUN.INI");
}


/**
 *  Fetches the extension data from the INI database at game init.  
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Load_Init_Settings()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OptionsClassExtension::Load_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("OptionsClassExtension::Load_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
    
    RawFileClass file("SUN.INI");
}


/**
 *  Saves the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Save_Settings()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OptionsClassExtension::Save_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("OptionsClassExtension::Save_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
    
    RawFileClass file("SUN.INI");
}


/**
 *  Sets any options based on current settings.
 *  
 *  @author: CCHyper
 */
void OptionsClassExtension::Set()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("OptionsClassExtension::Save_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("OptionsClassExtension::Save_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));

}
