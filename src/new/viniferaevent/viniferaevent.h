/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERAEVENT.H
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
#pragma once

#include <cstddef>
#include <stdint.h>

#include "event.h"
#include "tibsun_defines.h"

enum ViniferaEventType : unsigned char
{
	VEVENT_RESPONSE_TIME = EVENT_COUNT, // Start after the last vanilla event

	VEVENT_COUNT,
	VEVENT_FIRST = VEVENT_RESPONSE_TIME
};


class ViniferaEventClass 
{
#pragma pack(push, 1)
public:
	ViniferaEventType Type;
	unsigned Frame;
	bool IsExecuted;
	int ID;

	union
	{
		char DataBuffer[36];

		struct ResponseTime2
		{
			char MaxAhead;
			uint8_t LatencyLevel;
		} ResponseTime2;

	} Data;

	void Execute();
	EventClass& As_Event() { return *reinterpret_cast<EventClass*>(this); }

	static unsigned char Event_Length(ViniferaEventType type);
	static unsigned char Event_Length(EventType type) { return Event_Length(static_cast<ViniferaEventType>(type)); };
	static bool Is_Vinifera_Event(ViniferaEventType type);
#pragma pack(pop)
};

static_assert(sizeof(ViniferaEventClass) == sizeof(EventClass), "ViniferaEventClass doesn't match EventClass in size!");
static_assert(sizeof(ViniferaEventClass::Data) == sizeof(EventClass::Data), "ViniferaEventClass::Data doesn't match EventClass::Data in size!");
static_assert(offsetof(ViniferaEventClass, Data) == offsetof(EventClass, Data), "ViniferaEventClass Data is misplaced!");
