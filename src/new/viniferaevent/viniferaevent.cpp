/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERAEVENT.CPP
 *
 *  @author        ZivDero, Belonit
 *
 *  @brief         Class that mimics vanilla EventClass to allow the creation
 *				   of new events in Vinifera.
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

#include "viniferaevent.h"

#include "asserthandler.h"
#include "protocolzero.h"

unsigned char ViniferaEventLength[VEVENT_COUNT - EVENT_COUNT]
{
	sizeof(ViniferaEventClass::Data.ResponseTime2)
};

const char* ViniferaEventNames[VEVENT_COUNT - EVENT_COUNT]
{
	"RESPONSE_TIME_2"
};


void ViniferaEventClass::Execute()
{
	switch (Type)
	{
	case VEVENT_RESPONSE_TIME_2:
		ProtocolZero::Handle_Response_Time(this);
		break;

	default:
		break;
	}
}

unsigned char ViniferaEventClass::Event_Length(ViniferaEventType type)
{
	ASSERT(type >= 0 && type < VEVENT_COUNT);

	if (type < EVENT_COUNT)
		return EventClass::Event_Length(static_cast<EventType>(type));

	return ViniferaEventLength[type - EVENT_COUNT];
}

const char* ViniferaEventClass::Event_Name(ViniferaEventType type)
{
	ASSERT(type >= 0 && type < VEVENT_COUNT);

	if (type < EVENT_COUNT)
		return EventClass::Event_Name(static_cast<EventType>(type));

	return ViniferaEventNames[type - EVENT_COUNT];
}

bool ViniferaEventClass::Is_Vinifera_Event(ViniferaEventType type)
{
	return (type >= VEVENT_FIRST && type < VEVENT_COUNT);
}
