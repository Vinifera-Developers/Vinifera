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
#include "extension_globals.h"
#include "vinifera_defines.h"
#include "wwcrc.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TActionClassExtension::TActionClassExtension(const TActionClass *this_ptr) :
    AbstractClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("TActionClassExtension::TActionClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TActionExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TActionClassExtension::TActionClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("TActionClassExtension::TActionClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TActionClassExtension::~TActionClassExtension()
{
    //EXT_DEBUG_TRACE("TActionClassExtension::~TActionClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TActionExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT TActionClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("TActionClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT TActionClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("TActionClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TActionClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TActionClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("TActionClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Save(pStm, fClearDirty);
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
int TActionClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("TActionClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TActionClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("TActionClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TActionClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TActionClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


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
 *  Performs the "new" action that this object does.
 * 
 *  This routine is called when the action associated with this action object must be
 *  performed. Typically, this occurs when a trigger has "sprung" and now it must take
 *  effect. The action object is what carries out this effect.
 * 
 *  house   --   The owner of this action. This information is necessary since some
 *               actions depend on who the trigger was owned by.
 * 
 *  object  --   Pointer to the object that the springing trigger was attached to. If
 *               this parameter is null, then the trigger wasn't attached to any object.
 * 
 *  trigger --   Trigger object (only if forced) otherwise null.
 * 
 *  cell    --   The cell this trigger is attached to (if any).
 * 
 * 
 *  @return: Was this action able to perform what it needed to do? Failure could be
 *           because a reinforcement couldn't be generated, for example.
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

