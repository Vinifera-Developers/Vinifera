/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SESSIONEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended SessionClass class.
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
#include "sessionext.h"
#include "tibsun_globals.h"
#include "noinit.h"
#include "options.h"
#include "ccini.h"
#include "rawfile.h"
#include "voc.h"
#include "rules.h"
#include "asserthandler.h"
#include "debughandler.h"


SessionClassExtension *SessionExtension = nullptr;


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
SessionClassExtension::SessionClassExtension(SessionClass *this_ptr) :
    Extension(this_ptr)
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("SessionClassExtension constructor - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("SessionClassExtension constructor - 0x%08X\n", (uintptr_t)(ThisPtr));

   /**
     *  Initialises the default game options.
     */
    ExtOptions.IsAutoDeployMCV = false;
    ExtOptions.IsPrePlacedConYards = false;
    ExtOptions.IsBuildOffAlly = true;
    
    IsInitialized = true;
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
SessionClassExtension::SessionClassExtension(const NoInitClass &noinit) :
    Extension(noinit)
{
    IsInitialized = false;
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
SessionClassExtension::~SessionClassExtension()
{
    //EXT_DEBUG_TRACE("SessionClassExtension destructor - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("SessionClassExtension destructor - 0x%08X\n", (uintptr_t)(ThisPtr));

    IsInitialized = false;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int SessionClassExtension::Size_Of() const
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("SessionClassExtension::Size_Of - 0x%08X\n", (uintptr_t)(ThisPtr));

    return sizeof(*this);
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void SessionClassExtension::Read_MultiPlayer_Settings()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("SessionClassExtension::Read_MultiPlayer_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("SessionClassExtension::Read_MultiPlayer_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
}


/**
 *  Saves the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
void SessionClassExtension::Write_MultiPlayer_Settings()
{
    ASSERT(ThisPtr != nullptr);
    //EXT_DEBUG_TRACE("SessionClassExtension::Write_MultiPlayer_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
    //EXT_DEBUG_WARNING("SessionClassExtension::Write_MultiPlayer_Settings - 0x%08X\n", (uintptr_t)(ThisPtr));
}
