/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTIONEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TActionClass class.
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
#include "tactionext.h"
#include "taction.h"
#include "house.h"
#include "object.h"
#include "vinifera_defines.h"
#include "wwcrc.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Returns the name of the TActionType.
 * 
 *  @author: CCHyper
 */
const char *TActionClassExtension::Action_Name(int action)
{
    if (action < TACTION_COUNT) {
        return TActionClass::Action_Name(TActionType(action));
    }

    switch (action) {

        default:
            return "<invalid>";
    }
}


/**
 *  Returns the description of the TActionType.
 * 
 *  @author: CCHyper
 */
const char *TActionClassExtension::Action_Description(int action)
{
    if (action < TACTION_COUNT) {
        return TActionClass::Action_Description(TActionType(action));
    }

    switch (action) {

        default:
            return "<invalid>";
    }
}


/**
 *  Executes the new trigger action.
 * 
 *  @author: CCHyper
 */
bool TActionClassExtension::Execute(TActionClass *this_ptr, HouseClass *house, ObjectClass *object, TriggerClass *trigger, Cell *cell)
{
    bool success = false;

    switch (this_ptr->Action) {

        /**
         *  Unexpected TActionType.
         */
        default:
            DEV_DEBUG_WARNING("Invalid action type!\n");
            break;
    };

    return success;
}


/**
 *  Helper info for writing new actions.
 * 
 *  TActionClass::Data                  = First Param (PARAM1)
 *  TActionClass::Bounds.X              = Second Param (PARAM2)
 *  TActionClass::Bounds.Y              = Third Param (PARAM3)
 *  TActionClass::Bounds.W              = Fourth Param (PARAM4)
 *  TActionClass::Bounds.H              = Fifth Param (PARAM5)
 * 
 *  (PARAM6) (OPTIONAL)
 *  if TActionFormatType == 4
 *    TActionClass::Data (overwrites PARAM1)
 *  else
 *    TActionClass::Location
 * 
 *  
 *  Example action line from a scenario file;
 * 
 *  [Actions]
 *  NAME = [Action Count], [TActionType], [TActionFormatType], [PARAM1], [PARAM2], [PARAM3], [PARAM4], [PARAM5], [PARAM6:OPTIONAL]
 *  
 *  To allow the use of TActionClass::Data (PARAM1), you must have the TActionFormatType set
 *  to "0", otherwise this param is ignored!
 * 
 * 
 *  For producing FinalSun [Action] entries;
 *  NOTE: For available ParamTypes, see the [ParamTypes] section in FSData.INI.
 *  NOTE: "DEF_PARAM1_VALUE" if negative (-ve), PARAM1 will be set to the absolute value of this number (filled in).
 * 
 *  [Actions]
 *  TActionType = [Name], [DEF_PARAM1_VALUE], [PARAM1_TYPE], [PARAM2_TYPE], [PARAM3_TYPE], [PARAM4_TYPE], [PARAM5_TYPE], [PARAM6_TYPE], [USE_WP], [USE_TAG], [Description], 1, 0, [TActionType]
 */

