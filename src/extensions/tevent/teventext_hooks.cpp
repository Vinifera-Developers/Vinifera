/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEVENTEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TEventClass.
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
#include "teventext_hooks.h"
#include "teventext.h"
#include "tevent.h"
#include "tibsun_defines.h"
#include "vinifera_defines.h"
#include "house.h"
#include "housetype.h"
#include "techno.h"
#include "technotype.h"
#include "buildingtype.h"
#include "unittype.h"
#include "infantrytype.h"
#include "aircrafttype.h"
#include "teamtype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Depending on the event type, return the "Data" field as represented as
 *  the correct string format.
 * 
 *  @author: CCHyper
 */
static const char * Get_TEvent_Data_String(TEventClass *event)
{
    static char _buffer[128];
    
    switch (event->Event) {
        case TEVENT_BUILDING_EXISTS:
        case TEVENT_BUILD:
            std::snprintf(_buffer, sizeof(_buffer), "Building - %s", BuildingTypeClass::Name_From(event->Data.Structure));
            break;
        case TEVENT_BUILD_UNIT:
            std::snprintf(_buffer, sizeof(_buffer), "Unit - %s", UnitTypeClass::Name_From(event->Data.Unit));
            break;
        case TEVENT_BUILD_INFANTRY:
            std::snprintf(_buffer, sizeof(_buffer), "Infantry - %s", InfantryTypeClass::Name_From(event->Data.Infantry));
            break;
        case TEVENT_BUILD_AIRCRAFT:
            std::snprintf(_buffer, sizeof(_buffer), "Aircraft - %s", AircraftTypeClass::Name_From(event->Data.Aircraft));
            break;
        case TEVENT_PLAYER_ENTERED:
        case TEVENT_CROSS_HORIZONTAL:
        case TEVENT_CROSS_VERTICAL:
        case TEVENT_ENTERS_ZONE:
        case TEVENT_THIEVED:
        case TEVENT_HOUSE_DISCOVERED:
        case TEVENT_BUILDINGS_DESTROYED:
        case TEVENT_UNITS_DESTROYED:
        case TEVENT_ALL_DESTROYED:
        case TEVENT_LOW_POWER:
            std::snprintf(_buffer, sizeof(_buffer), "House - %s", HouseTypeClass::Name_From(event->Data.House));
            break;
        //case X:
        //    std::snprintf(_buffer, sizeof(_buffer), "Float - %f", event->Data.Float);
        //    break;
        default:
        case TEVENT_TIME:
        case TEVENT_GLOBAL_SET:
        case TEVENT_GLOBAL_CLEAR:
        case TEVENT_LOCAL_SET:
        case TEVENT_LOCAL_CLEARED:
        case TEVENT_CREDITS:
        case TEVENT_NBUILDINGS_DESTROYED:
        case TEVENT_NUNITS_DESTROYED:
            std::snprintf(_buffer, sizeof(_buffer), "Value - %d", event->Data.Value);
            break;
        case TEVENT_LEAVES_MAP:
            std::snprintf(_buffer, sizeof(_buffer), "Team - %d", event->Data.Value);
            break;
            
    }

    return _buffer;
}


/**
 *  This patch extends the TEventClass operator.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TEventClass_Operator_Extend_Switch_Patch)
{
    /**
     *  Stolen bytes/code.
     */
    _asm { sub esp, 0x18 }
    _asm { push ebx }
    _asm { push ebp }
    _asm { push esi }
    _asm { push edi }

    GET_REGISTER_STATIC(TEventClass *, this_ptr, ecx);

    /**
     *  Function arguments
     */
    GET_STACK_STATIC(TEventType, event, esp, 0x2C);        // The event that has occurred according to the context from which this routine was called.
    GET_STACK_STATIC(HouseClass *, house, esp, 0x30);      // The house that this event is tied to.
    GET_STACK_STATIC(ObjectClass *, object, esp, 0x34);    // The object that this event might apply to. For object triggering events, this will point to the perpetrator object.
    GET_STACK_STATIC(TDEventClass *, td, esp, 0x38);       // This holds the changable data that is associated with an event as it relates to a trigger.
    GET_STACK_STATIC(bool *, tripped, esp, 0x3C);          // If this event has been triggered by something that is temporal, then this flag will be set to true so that subsequent trigger examination will return a successful event trigger flag.
    GET_STACK_STATIC(TechnoClass *, source, esp, 0x40);    // 

    static bool satisfied; // Was this event satisfied? A satisfied event will probably spring the trigger it is attached to.
    satisfied = false;

#if 0
    /**
     *  Helper info for debugging when adding new events.
     */
    DEV_DEBUG_INFO("TEventClass:\n");
    DEV_DEBUG_INFO("  Querying: \"%s\"\n", TEventClassExtension::Event_Name(this_ptr->Event)); // Use extension Event_Name so new events are referenced correctly.
    DEV_DEBUG_INFO("  ID: %d\n", this_ptr->ID);
    if (this_ptr->Next) DEV_DEBUG_INFO("  Next: \"%s\"\n", TEventClassExtension::Event_Name(this_ptr->Next->Event)); // Use extension Event_Name so new events are referenced correctly.
    if (this_ptr->Team) DEV_DEBUG_INFO("  Team: \"%s\"\n", this_ptr->Team->Name());
    DEV_DEBUG_INFO("  Data: %s\n", Get_TEvent_Data_String(this_ptr));
    if (house) {
        DEV_DEBUG_INFO("  House: \"%s\"\n", house->Class->Name());
        if (house->Is_Player()) {
            DEV_DEBUG_INFO("    IsPlayer: true\n");
        } else {
            DEV_DEBUG_INFO("    IsPlayer: false\n");
        }
    }
    DEV_DEBUG_INFO("  TData: %d\n", td->Timer.Value());
    if (object) {
        DEV_DEBUG_INFO("  Object: \"%s\"\n", object->Name());
        DEV_DEBUG_INFO("    Coord: %s\n", object->Get_Coord().As_String());
    }
    if (source) {
        DEV_DEBUG_INFO("  Source: \"%s\"\n", source->Name());
        DEV_DEBUG_INFO("    Coord: %s\n", source->Get_Coord().As_String());
    }
#endif

    /**
     *  Skip null actions.
     */
    if (this_ptr->Event == TEVENT_NONE) {
        satisfied = false;
        goto return_satisfied;
    }

    /**
     *  Handle the original TEventTypes.
     */
    if (this_ptr->Event < TEVENT_COUNT) {
        goto tevent_switch;
    }

    /**
     *  Evaluate the new trigger event. The map ini format must be greater than
     *  or equal to the value of 5. This allows us to make sure new events are
     *  not queried by mistake!
     */
    if (NewINIFormat >= 5) {
        if (this_ptr->Event < EXT_TEVENT_COUNT) {
            satisfied = TEventClassExtension::Satisfied(this_ptr, event, house, object, *td, *tripped, source);
            goto return_satisfied;
        }
    }

    /**
     *  The default case, return satisfied.
     */
return_satisfied:
    _asm { xor eax, eax }
    _asm { mov al, satisfied }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x18 }
    _asm { retn 0x18 }

    /**
     *  The switch case for the original TEventTypes
     */
tevent_switch:
    _asm { mov ebp, this_ptr }
    JMP_REG(eax, 0x00642319);
}


/**
 *  Main function for patching the hooks.
 */
void TEventClassExtension_Hooks()
{
    Patch_Jump(0x00642310, &_TEventClass_Operator_Extend_Switch_Patch);
}
