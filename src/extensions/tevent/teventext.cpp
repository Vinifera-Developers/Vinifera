/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEVENTEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TEventClass class.
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
#include "teventext.h"
#include "tevent.h"
#include "house.h"
#include "techno.h"
#include "technotype.h"
#include "tibsun_globals.h"
#include "extension_globals.h"
#include "vinifera_defines.h"
#include "scenario.h"
#include "wwcrc.h"
#include "asserthandler.h"
#include "debughandler.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TEventClassExtension::TEventClassExtension(const TEventClass *this_ptr) :
    AbstractClassExtension(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("TEventClassExtension::TEventClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TEventExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TEventClassExtension::TEventClassExtension(const NoInitClass &noinit) :
    AbstractClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("TEventClassExtension::TEventClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TEventClassExtension::~TEventClassExtension()
{
    //EXT_DEBUG_TRACE("TEventClassExtension::~TEventClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TEventExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT TEventClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("TEventClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT TEventClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("TEventClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TEventClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TEventClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("TEventClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
int TEventClassExtension::Size_Of() const
{
    //EXT_DEBUG_TRACE("TEventClassExtension::Size_Of - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TEventClassExtension::Detach(TARGET target, bool all)
{
    //EXT_DEBUG_TRACE("TEventClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TEventClassExtension::Compute_CRC(WWCRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TEventClassExtension::Compute_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Returns the name of the TEventType.
 * 
 *  @author: CCHyper
 */
const char *TEventClassExtension::Event_Name(int action)
{
    if (action < TEVENT_COUNT) {
        return TEventClass::Event_Name(TEventType(action));
    }

    switch (action) {
        case TEVENT_SPY_AS_HOUSE:
            return "Spy entering as House...";
        case TEVENT_SPY_AS_INFANTRY:
            return "Spy entering as Infantry...";
        case TEVENT_DESTROYED_UNITS_NAVAL:
            return "Destroyed, Units, Naval...";
        case TEVENT_DESTROYED_UNITS_LAND:
            return "Destroyed, Units, Land...";
        case TEVENT_BUILDING_DOES_NOT_EXIST:
            return "Building does not exist";
        case TEVENT_POWER_FULL:
            return "Power Full...";
        case TEVENT_ENTERED_OR_OVERFLOWN_BY:
            return "Entered or Overflown By...";
        case TEVENT_TECHTYPE_EXISTS:
        case TEVENT_TECHTYPE_EXISTS_NAME:
            return "TechType Exists";
        case TEVENT_TECHTYPE_DOESNT_EXIST:
        case TEVENT_TECHTYPE_DOESNT_EXIST_NAME:
            return "TechType does not Exist";
        case TEVENT_POWER_LESS_THAN:
            return "Power Less Than...";
        case TEVENT_POWER_GREATER_THAN:
            return "Power Greater Than...";
        case TEVENT_INFANTRY_DESTROYED:
            return "Destroyed, Infantry, All...";
        case TEVENT_CONSTRUCTION_YARD:
            return "Construction yard exists";
        case TEVENT_MISSION_TIMER_LESS_THAN:
            return "Mission Timer Less Than";
        case TEVENT_MISSION_TIMER_GREATER_THAN:
            return "Mission Timer Greater Than";
        case TEVENT_MISSION_TIMER_EQUALS:
            return "Mission Timer Equals";
        default:
            return "<invalid>";
    }
}


/**
 *  Returns the description of the TEventType.
 * 
 *  @author: CCHyper
 */
const char *TEventClassExtension::Event_Description(int action)
{
    if (action < TEVENT_COUNT) {
        return TEventClass::Event_Description(TEventType(action));
    }

    switch (action) {
        case TEVENT_SPY_AS_HOUSE:
            return "Triggers if a spy disguised as house specified enters this.";
        case TEVENT_SPY_AS_INFANTRY:
            return "Triggers if a spy disguised as this type of infantry enters.";
        case TEVENT_DESTROYED_UNITS_NAVAL:
            return "Triggers when all naval units of the specified house have been destroyed. Typically used for end of game conditions.";
        case TEVENT_DESTROYED_UNITS_LAND:
            return "Triggers when all land units of the specified house have been destroyed. Typically used for end of game conditions.";
        case TEVENT_BUILDING_DOES_NOT_EXIST:
            return "Triggers when the building (owned by the house of this trigger) specified does not exist on the map.";
        case TEVENT_POWER_FULL:
            return "Triggers if the specified house's power is at 100%.";
        case TEVENT_ENTERED_OR_OVERFLOWN_BY:
            return "Triggers when unit, infantry, or aircraft move over this cell. <THEM = House of entering unit>.";
        case TEVENT_TECHTYPE_EXISTS:
        case TEVENT_TECHTYPE_EXISTS_NAME:
            return "True if there are at least this many of this type, belonging to anyone.";
        case TEVENT_TECHTYPE_DOESNT_EXIST:
        case TEVENT_TECHTYPE_DOESNT_EXIST_NAME:
            return "True if there are none of these on the map at all. Number doesn't mean anything.";
        case TEVENT_POWER_LESS_THAN:
            return "Triggers if the specified house's power is less than or equal to the value specified.";
        case TEVENT_POWER_GREATER_THAN:
            return "Triggers if the specified house's power is greater than or equal to the value specified.";
        case TEVENT_INFANTRY_DESTROYED:
            return "Triggers when all infantry of the specified side have been destroyed. Typically used for end of game conditions.";
        case TEVENT_CONSTRUCTION_YARD:
            return "Triggers if the specified house has one or more construction yards.";
        case TEVENT_MISSION_TIMER_LESS_THAN:
            return "Triggers when the mission timer drops below the time value specified.";
        case TEVENT_MISSION_TIMER_GREATER_THAN:
            return "Triggers when the mission timer is above the time value specified.";
        case TEVENT_MISSION_TIMER_EQUALS:
            return "Triggers when the mission timer equals time value specified.";
        default:
            return "<invalid>";
    }
}


/**
 *  Action operator to see if the "new" event is satisfied.
 * 
 *  This routine is called to see if the event has been satisfied. Typically, when the
 *  necessary trigger events have been satisfied, then the trigger is sprung. For some
 *  events, the act of calling this routine is tacit proof enough that the event has
 *  occurred. For most other events, the success condition must be explicitly checked.
 * 
 *  event   --  The event that has occurred according to the context from which this
 *              routine was called. In the case of no specific event having occurred,
 *              then TEVENT_ANY will be passed in.
 * 
 *  house   --  The house that this event is tied to.
 * 
 *  object  --  The object that this event might apply to. For object triggering
 *              events, this will point to the perpetrator object.
 * 
 *  td      --  This holds the changable data that is associated with an event as
 *              it relates to a trigger.
 * 
 *  tripped --  If this event has been triggered by something that is temporal, then
 *              this flag will be set to true so that subsequent trigger examination
 *              will return a successful event trigger flag.
 * 
 *  source  --  The object that was responsible for this event occuring.
 * 
 *  @return: Was this event satisfied? A satisfied event will probably spring the
 *           trigger it is attached to.
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Satisfied(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    bool success = false;

    switch ((ExtTEventType)this_ptr->Event) {
        case TEVENT_SPY_AS_HOUSE:
            //success = Spy_As_House(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_SPY_AS_INFANTRY:
            //success = Spy_As_Infantry(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_DESTROYED_UNITS_NAVAL:
            //success = Destroyed_Units_Naval(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_DESTROYED_UNITS_LAND:
            success = Destroyed_Units_Land(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_BUILDING_DOES_NOT_EXIST:
            success = Building_Does_Not_Exist(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_POWER_FULL:
            success = Power_Full(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_ENTERED_OR_OVERFLOWN_BY:
            //success = Entered_Or_Overflown_By(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_TECHTYPE_EXISTS:
            success = TechType_Exists(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_TECHTYPE_DOESNT_EXIST:
            success = TechType_Does_Not_Exist(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_TECHTYPE_EXISTS_NAME:
            //success = TechType_Exists(this_ptr, event, house, object, td, tripped, source, true);
            break;
        case TEVENT_TECHTYPE_DOESNT_EXIST_NAME:
            //success = TechType_Does_Not_Exist(this_ptr, event, house, object, td, tripped, source, true);
            break;
        case TEVENT_INFANTRY_DESTROYED:
            success = Infantry_Destroyed(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_MISSION_TIMER_LESS_THAN:
            success = Mission_Timer_Less_Than(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_MISSION_TIMER_GREATER_THAN:
            success = Mission_Timer_Greater_Than(this_ptr, event, house, object, td, tripped, source);
            break;
        case TEVENT_MISSION_TIMER_EQUALS:
            success = Mission_Timer_Equal_To(this_ptr, event, house, object, td, tripped, source);
            break;

        /**
         *  Unexpected TEventType.
         */
        default:
            DEV_DEBUG_WARNING("Invalid event type!\n");
            break;
    };

    return success;
}


/**
 *  Helper info for writing new event.
 *  
 *  Example event line from a scenario file;
 * 
 *  [Events]
 *  NAME = [Count], [TEventType], [TEventFormatType1], [PARAM1], ...
 *  
 *  To allow the use of TEventClass::Data (PARAM1), you must have the TEventFormatType set
 *  to "0", otherwise this param is "1" is interpreted as a TeamType!
 * 
 * 
 *  For producing FinalSun [Event] entries;
 *  NOTE: For available ParamTypes, see the [ParamTypes] section in FSData.INI.
 * 
 *  [Events]
 *  TEventType = [Name], [PARAM1_TYPE], [PARAM2_TYPE], 0, 0, [Description], 1, 0, [TEventType]
 */


/**
 *  Evaluation function for "TEVENT_SPY_AS_HOUSE".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Spy_As_House(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
#if 0
    if (object) {
        if (this_ptr->Data.House != HOUSE_NONE && HouseClass::As_Pointer(this_ptr->Data.House)) {
            HouseClass *hptr = object->Show_As(true);
            if (hptr->Fetch_ID() == HouseClass::As_Pointer(this_ptr->Data.House)->Fetch_ID()) {
                tripped = true;
                return true;
            }
        }
    }
#endif

    return false;
}


/**
 *  Evaluation function for "TEVENT_SPY_AS_INFANTRY".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Spy_As_Infantry(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
#if 0
    if (object) {
        TechnoTypeClass *disguise = object->Disguise_Type();
        if (disguise->What_Am_I() == RTTI_INFANTRYTYPE) {
            if (this_ptr->Data.Value != -1 && this_ptr->Data.Infantry == disguise->Get_Heap_ID()) {
                tripped = true;
                return true;
            }
        }
    }
#endif

    return false;
}


/**
 *  Evaluation function for "TEVENT_DESTROYED_UNITS_NAVAL".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Destroyed_Units_Naval(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
#if 0
    if (house->CurShips <= 0) {
        return true;
    }
#endif

    return false;
}


/**
 *  Evaluation function for "TEVENT_DESTROYED_UNITS_LAND".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Destroyed_Units_Land(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    if (house->CurUnits <= 0 && house->CurInfantry <= 0) {
        return true;
    }

    return false;
}


/**
 *  Evaluation function for "TEVENT_BUILDING_DOES_NOT_EXIST".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Building_Does_Not_Exist(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    if (this_ptr->Data.Structure != BUILDING_NONE) {
        if (house->ActiveBQuantity.Count_Of(this_ptr->Data.Structure) <= 0) {
            tripped = true;
            return true;
        }
    }

    return false;
}


/**
 *  Evaluation function for "TEVENT_POWER_FULL".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Power_Full(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    if (house->Power_Fraction() >= 1.0f) {
        return true;
    }

    return false;
}


/**
 *  Evaluation function for "TEVENT_ENTERED_OR_OVERFLOWN_BY".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Entered_Or_Overflown_By(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    // TODO: Requires TEventClass extension.
#if 0
    if (this_ptr->Data.House != HOUSE_NONE || HouseClass::As_Pointer(this_ptr->Data.House)) {
        if (object) {
            if (this_ptr->Data.House != HOUSE_NONE || object->Owner() == HouseClass::As_Pointer(this_ptr->Data.House)->Fetch_ID()) {
                tripped = true;
                this_ptr->House = object->Owning_House();
                return true;
            }
        }
    }
#endif

    return false;
}


/**
 *  Query if a TechnoType exists from the name string.
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::TechnoType_Exists_From_Name(const char *name)
{
    int v18 = -1;

    for (int i = Technos.Count()-1; i >= 0; --i) {
        if (std::strcmp(TechnoTypes[i]->Name(), name) == 0) {
            break;
        }
        v18 = i;
    }

    if (v18 >= TechnoTypes.Count() || v18 < 0) {
        return false;
    }

    for (int i = Technos.Count()-1; i >= 0; --i) {
        if (Technos[i]->Techno_Type_Class() == TechnoTypes[v18]) {
            return false;
        }
    }

    return true;
}


/**
 *  Query if a TechnoType exists from the input packed type list index.
 * 
 *  The input value will have its offset removed depending on the offset amount.
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::TechnoType_Exists_From_Value(const int data_value)
{
    enum {
        AIRCRAFT = 0x0,
        INFANTRY = 0x200,
        UNIT = 0x400,
        BUILDING = 0x600,
    };

    const TechnoTypeClass *ttype = nullptr;

    if (data_value >= BUILDING && (data_value - BUILDING) < BuildingTypes.Count()) {
        return (TechnoTypeClass *)BuildingTypes[data_value - BUILDING];

    } else if (data_value >= UNIT && (data_value - UNIT) < UnitTypes.Count()) {
        return (TechnoTypeClass *)UnitTypes[data_value - UNIT];

    } else if (data_value >= INFANTRY && (data_value - INFANTRY) < InfantryTypes.Count()) {
        return (TechnoTypeClass *)InfantryTypes[data_value - INFANTRY];

    } else if (data_value >= AIRCRAFT && (data_value - AIRCRAFT) < AircraftTypes.Count()) {
        return (TechnoTypeClass *)AircraftTypes[data_value - AIRCRAFT];
    }

    if (!ttype) {
        return false;
    }

    return TechnoType_Exists_From_Name(ttype->Name());
}


/**
 *  Evaluation function for "TEVENT_TECHTYPE_EXISTS".
 * 
 *  @note: This implementation works slightly different to how it works in YR, in which
 *         it adds a new member to TEventClass which holds a string.
 * 
 *         For this implementation, we use a Type list index, added with a value
 *         to tell us which Type list we should check.
 * 
 *         TODO: Use "by_name" argument to check by string value once TEventClass extension is implemented.
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::TechType_Exists(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source, bool by_name)
{
    bool exists = false;

    if (by_name) {
        // TODO: Fetch TEventClass extension.
        return false; //exists = TechnoType_Exists_From_Name(this_ptr->Data.String);
    } else {
        exists = TechnoType_Exists_From_Value(this_ptr->Data.Value);
    }

    return exists == true;
}


/**
 *  Evaluation function for "TEVENT_TECHTYPE_DOESNT_EXIST".
 * 
 *  @note: This implementation works slightly different to how it works in YR, in which
 *         it adds a new member to TEventClass which holds a string.
 * 
 *         For this implementation, we use a Type list index, added with a value
 *         to tell us which Type list we should check.
 * 
 *         TODO: Use "by_name" argument to check by string value once TEventClass extension is implemented.
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::TechType_Does_Not_Exist(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source, bool by_name)
{
    bool exists = false;

    if (by_name) {
        // TODO: Fetch TEventClass extension.
        return false; //exists = TechnoType_Exists_From_Name(this_ptr->Data.String);
    } else {
        exists = TechnoType_Exists_From_Value(this_ptr->Data.Value);
    }

    return exists == false;
}


/**
 *  Evaluation function for "TEVENT_POWER_LESS_THAN".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Power_Less_Than(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    if (house->Power_Fraction() <= this_ptr->Data.Float) {
        return true;
    }

    return false;
}


/**
 *  Evaluation function for "TEVENT_POWER_GREATER_THAN".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Power_Greater_Than(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    if (house->Power_Fraction() >= this_ptr->Data.Float) {
        return true;
    }

    return false;
}


/**
 *  Evaluation function for "TEVENT_INFANTRY_DESTROYED".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Infantry_Destroyed(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    if (house->CurInfantry <= 0) {
        return true;
    }

    return false;
}


/**
 *  Evaluation function for "TEVENT_CONSTRUCTION_YARD".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::House_Has_Construction_Yard(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    if (house->ConstructionYards.Count() > 0) {
        return true;
    }

    return false;
}


/**
 *  Evaluation function for "TEVENT_MISSION_TIMER_LESS_THAN".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Mission_Timer_Less_Than(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    if (Scen->MissionTimer.Is_Active() && Scen->MissionTimer <= this_ptr->Data.Value) {
        return true;
    }

    return false;
}


/**
 *  Evaluation function for "TEVENT_MISSION_TIMER_GREATER_THAN".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Mission_Timer_Greater_Than(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    if (Scen->MissionTimer.Is_Active() && Scen->MissionTimer >= this_ptr->Data.Value) {
        return true;
    }

    return false;
}


/**
 *  Evaluation function for "TEVENT_MISSION_TIMER_EQUALS".
 * 
 *  @author: CCHyper
 */
bool TEventClassExtension::Mission_Timer_Equal_To(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source)
{
    if (Scen->MissionTimer.Is_Active() && Scen->MissionTimer == this_ptr->Data.Value) {
        return true;
    }

    return false;
}
