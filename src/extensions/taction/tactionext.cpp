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

        case TACTION_CREDITS:
            return "Give credits to house...";

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

        case TACTION_CREDITS:
            return "Gives credits to the specified house.";

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

        case TACTION_CREDITS:
            success = TAction_Give_Credits_To_House(this_ptr, house, object, trigger, cell);
            break;

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


/**
 *  #issue-158
 * 
 *  Give credits to the specified house.
 * 
 *  Expected scenario INI format;
 *  NAME = [Action Count], 106, 0, [PARAM1 = House ID], [PARAM2 = Credits], 0, 0, 0
 * 
 *  For FinalSun;
 *  106=Give credits to house... [Requires Vinifera],0,2,5,0,0,0,0,0,0,Gives credits to the specified house.,1,0,106
 * 
 *  @author: CCHyper
 */
bool TActionClassExtension::TAction_Give_Credits_To_House(TActionClass *this_ptr, HouseClass *house, ObjectClass *object, TriggerClass *trigger, Cell *cell)
{
    if (!this_ptr || !house) {
        return false;
    }

#ifndef NDEBUG
    DEV_DEBUG_INFO("Executing \"%s\"\n", Action_Name(this_ptr->Action));
#endif

    /**
     *  Fetch the param values.
     */
    int param_one = this_ptr->Data.Value;
    int param_two = this_ptr->Bounds.X;
    int param_three = this_ptr->Bounds.Y;
    int param_four = this_ptr->Bounds.Width;
    int param_five = this_ptr->Bounds.Height;
#ifndef NDEBUG
    DEV_DEBUG_INFO("  param_one = %d\n", param_one);
    DEV_DEBUG_INFO("  param_two = %d\n", param_two);
    DEV_DEBUG_INFO("  param_three = %d\n", param_three);
    DEV_DEBUG_INFO("  param_four = %d\n", param_four);
    DEV_DEBUG_INFO("  param_five = %d\n", param_five);
#endif

    /**
     *  Fetch the house pointer and credit amount.
     */
    HouseClass *hptr = HouseClass::As_Pointer(HousesType(param_one));
    int amount = param_two;

    if (!hptr) {
        DEBUG_WARNING("TActionExtention: Invalid house!\n");
        return false;
    }
    if (amount == 0) {
        DEBUG_WARNING("TActionExtention: Amount is zero!\n");
    }

    /**
     *  If positive, grant the cash bonus.
     *  If negative, take money from the house.
     */
    if (amount != 0) {
        if (amount < 0) {
#ifndef NDEBUG
            DEV_DEBUG_INFO("  Reducing money for house \"%s\" by %d.\n", hptr->Name(), std::abs(amount));
#endif
            hptr->Spend_Money(std::abs(amount));
        } else {
#ifndef NDEBUG
            DEV_DEBUG_INFO("  Increasing money for house \"%s\" by %d.\n", hptr->Name(), amount);
#endif
            hptr->Refund_Money(amount);
        }
    }

    return true;
}
