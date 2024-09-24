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

#include "protocolzero.h"

void ViniferaEventClass::Execute()
{
	switch (Type)
	{
	case VEVENT_RESPONSE_TIME:
		ProtocolZero::Handle_Response_Time(this);
		break;

	default:
		break;
	}
}

unsigned char ViniferaEventClass::Event_Length(ViniferaEventType type)
{
	switch (type)
	{
	case VEVENT_RESPONSE_TIME:
		return sizeof(Data.ResponseTime2);

	default:
		break;
	}

	return 0;
}

bool ViniferaEventClass::Is_Vinifera_Event(ViniferaEventType type)
{
	return (type >= VEVENT_FIRST && type < VEVENT_COUNT);
}
