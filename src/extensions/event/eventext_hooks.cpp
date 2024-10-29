/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EVENTEXT_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the extended EventClass.
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

#include "eventext_hooks.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "viniferaevent.h"
#include "techno.h"
#include "building.h"
#include "foot.h"
#include "team.h"
#include "mouse.h"
#include "cell.h"
#include "fetchres.h"
#include "language.h"
#include "rules.h"
#include "unit.h"
#include "session.h"
#include "anim.h"
#include "extension.h"
#include "protocolzero.h"
#include "spawner.h"
#include "technoext.h"
#include "spawnmanager.h"
#include "tibsun_functions.h"
#include "vinifera_globals.h"


/**
  *  A fake class for implementing new member functions which allow
  *  access to the "this" pointer of the intended class.
  *
  *  @note: This must not contain a constructor or destructor!
  *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
  */
class EventClassExt final : public EventClass
{
public:
    void _Execute();

    void _Event_PowerOn();
    void _Event_PowerOff();
    void _Event_Ally();
    void _Event_MegaMission();
    void _Event_Idle();
    void _Event_Scatter();
    void _Event_Destruct();
    void _Event_Deploy();
    void _Event_Place();
    void _Event_Options();
    void _Event_GameSpeed();
    void _Event_Produce();
    void _Event_Suspend();
    void _Event_Abandon();
    void _Event_Primary();
    void _Event_Special_Place();
    void _Event_Exit();
    void _Event_Animation();
    void _Event_Repair();
    void _Event_Sell();
    void _Event_SellCell();
    void _Event_Special();
    void _Event_Response_Time();
    void _Event_SaveGame();
    void _Event_Archive();
    void _Event_AddPlayer();
    void _Event_Timing();
    void _Event_Process_Time();
    void _Event_PageUser();
    void _Event_RemovePlayer();
    void _Event_LatencyFudge();

};


void EventClassExt::_Event_PowerOn()
{
    BuildingClass* building = Data.Target.Whom.As_Building();
    if (building && building->IsActive && !building->IsInLimbo && !building->Class->IsFirestormWall) {
        building->Turn_On();
    }
}


void EventClassExt::_Event_PowerOff()
{
    BuildingClass* building = Data.Target.Whom.As_Building();
    if (building && building->IsActive && !building->IsInLimbo && !building->Class->IsFirestormWall) {
        building->Turn_Off();
    }
}


void EventClassExt::_Event_Ally()
{
    HouseClass* house = Houses[ID];
    if (house->Is_Ally(Houses[static_cast<HousesType>(Data.General.Value)])) {
        house->Make_Enemy(static_cast<HousesType>(Data.General.Value));
    }
    else {
        house->Make_Ally(static_cast<HousesType>(Data.General.Value));
    }
}


void EventClassExt::_Event_MegaMission()
{
    TechnoClass* techno = Data.MegaMission.Whom.As_Techno();

    if (techno != nullptr && techno->IsActive && techno->Strength > 0 && !techno->IsInLimbo) {

        /**
         *  Fetch a pointer to the object of the mission. If there is an error with
         *  this object, such as it is dead, then bail.
         */
        ObjectClass* object = nullptr;
        if (Data.MegaMission.Target.Is_Valid()) {
            object = Data.MegaMission.Target.As_Object();
            if (object != nullptr && (!object->IsActive || object->Strength == 0 || object->IsInLimbo)) {
                return;
            }
        }

        /**
         *  If the destination target is invalid because the object is dead, then
         *  bail from processing this mega mission.
         */
        if (Data.MegaMission.Destination.Is_Valid()) {
            ObjectClass* destination = Data.MegaMission.Destination.As_Object();
            if (destination != nullptr && (!destination->IsActive || destination->Strength == 0 || destination->IsInLimbo)) {
                return;
            }
        }

        /**
         *  If the destination target is a building that we're already docking with, then
         *  bail from processing this mega mission.
         */
        if (Data.MegaMission.Mission == MISSION_ENTER && techno->Get_Mission() == MISSION_ENTER && techno->Is_Foot()) {
            AbstractClass* destination = Data.MegaMission.Destination.As_Abstract();
            if (destination != nullptr && destination->What_Am_I() == RTTI_BUILDING && techno->Contact_With_Whom() == destination) {
                return;
            }
        }

        /**
         *  Break any existing tether or team contact, since it is now invalid.
         */
        if (!techno->IsTethered) {
            techno->Transmit_Message(RADIO_OVER_OUT);
        }
        else {
            if (techno->In_Radio_Contact() && techno->Contact_With_Whom()->IsActive) {
                BuildingClass* ref = dynamic_cast<BuildingClass*>(techno->Contact_With_Whom());

                if (ref && ref->Class->IsRefinery) {
                    techno->Transmit_Message(RADIO_OVER_OUT);
                    techno->IsTethered = false;
                }
            }
        }

        if (techno->field_20C) {
            techno->field_20C = nullptr;
        }

        if (techno->Is_Foot()) {
            if (static_cast<FootClass*>(techno)->Team && Data.MegaMission.Mission != MISSION_UNLOAD) {
                static_cast<FootClass*>(techno)->Team->Remove(static_cast<FootClass*>(techno));
            }
        }

        if (object != nullptr) {

            if (PlayerPtr->Is_Ally(techno)) {
                object->Clicked_As_Target();
            }
        }

        /**
         *  Test to see if the navigation target should really be queued rather
         *  than assigned to the object. This would be the case if this is a
         *  special queued move mission and there is already a valid navigation
         *  target for this unit.
         */
        const bool q = Data.MegaMission.Mission == MISSION_QMOVE;

        techno->Assign_Mission(Data.MegaMission.Mission);
        if (techno->Is_Foot()) {
            static_cast<FootClass*>(techno)->SuspendedNavCom = nullptr;
        }
        techno->SuspendedTarCom = nullptr;

        /**
         *  Guard area mode is handled with care. The specified target is actually
         *  assigned as the location that should be guarded. In addition, the
         *  movement destination is immediately set to this new location.
         */
        if (Data.MegaMission.Mission == MISSION_GUARD_AREA && techno->Is_Foot()) {
            techno->Assign_Target(nullptr);
            techno->Assign_Destination(Data.MegaMission.Target.As_Abstract());
            techno->Assign_Archive_Target(Data.MegaMission.Target.As_Abstract());
        }
        else {
            if (techno->Is_Foot()) {
                techno->Assign_Archive_Target(nullptr);
            }

            if (q && techno->Is_Foot()) {
                static_cast<FootClass*>(techno)->Queue_Navigation_List(Data.MegaMission.Destination.As_Abstract());
            }
            else {
                if (techno->Is_Foot()) {
                    ((FootClass*)techno)->Clear_Navigation_List();
                }
                techno->Assign_Target(Data.MegaMission.Target.As_Abstract());
                techno->Assign_Destination(Data.MegaMission.Destination.As_Abstract());
            }
        }
    }
}


void EventClassExt::_Event_Idle()
{
    TechnoClass* techno = Data.Target.Whom.As_Techno();

    if (techno != nullptr && techno->IsActive && !techno->IsInLimbo && !techno->IsTethered
        && techno->Get_Mission() != MISSION_CONSTRUCTION && techno->Get_Mission() != MISSION_DECONSTRUCTION) {

        if (techno->IsOnBridge || Map[techno->Get_Coord()].Ramp || !techno->Is_On_Elevation()) {

            if (techno->Is_Foot()) {
                static_cast<FootClass*>(techno)->Clear_Navigation_List();
                static_cast<FootClass*>(techno)->field_220 = -1;
                static_cast<FootClass*>(techno)->field_33E = 0;
                static_cast<FootClass*>(techno)->field_224 = Cell();
            }

            techno->Transmit_Message(RADIO_OVER_OUT);
            techno->Assign_Destination(nullptr);
            techno->Assign_Target(nullptr);

            const auto extension = Extension::Fetch<TechnoClassExtension>(techno);
            if (extension->SpawnManager)
                extension->SpawnManager->Abandon_Target();

            if (techno->What_Am_I() == RTTI_UNIT && static_cast<UnitClass*>(techno)->Class->IsToHarvest &&
                (techno->Get_Mission() == MISSION_HARVEST || techno->Get_Mission() == MISSION_RETURN)) {
                techno->Assign_Mission(MISSION_GUARD);
                techno->Commence();
            }
        }
    }
}


void EventClassExt::_Event_Scatter()
{
    TechnoClass* techno = Data.Target.Whom.As_Techno();

    if (techno != nullptr && techno->Is_Foot() && techno->IsActive && !techno->IsInLimbo && !techno->IsTethered) {
        static_cast<FootClass*>(techno)->IsScattering = true;
        techno->Scatter(Coordinate(), true, false);
    }
}


void EventClassExt::_Event_Destruct()
{
    Houses[ID]->Flag_To_Die();
}


void EventClassExt::_Event_Deploy()
{
    TechnoClass* techno = Data.Target.Whom.As_Techno();

    if (techno != nullptr && techno->IsActive && !techno->IsInLimbo && !techno->IsTethered && techno->EMPFramesRemaining == 0) {

        if (techno->IsOnBridge || Map[techno->Get_Coord()].Ramp || !techno->Is_On_Elevation()) {

            if (techno->Get_Mission() != MISSION_CONSTRUCTION && techno->Get_Mission() != MISSION_DECONSTRUCTION && techno->What_Am_I() != RTTI_AIRCRAFT)
            {
                Cell cell = techno->Get_Cell();
                if (cell == Cell() || Map[cell].Cell_Building() == nullptr || !Map[cell].Cell_Building()->Class->IsWeaponsFactory)
                {
                    techno->Transmit_Message(RADIO_OVER_OUT);
                    techno->Assign_Destination(nullptr);
                    techno->Assign_Target(nullptr);
                    techno->Assign_Mission(MISSION_UNLOAD);
                }
            }
        }
    }
}


void EventClassExt::_Event_Place()
{
    Cell cell(Data.Place.Cell);
    Houses[ID]->Place_Object(Data.Place.Type, cell);
}


void EventClassExt::_Event_Options()
{
    if (!Session.Play) {
        SpecialDialog = SDLG_OPTIONS;
    }
}


void EventClassExt::_Event_GameSpeed()
{
    char txt[256];

    Options.GameSpeed = Data.General.Value;

    HouseClass* house = Houses[ID];
    if (house != PlayerPtr && house != nullptr) {
        const char* text = Fetch_String(TXT_PLAYER_CHANGED_SPEED);

        if (text && std::strlen(text)) {
            std::snprintf(txt, std::size(txt), text, house->IniName);
            Session.Messages.Add_Message(nullptr, 0, txt, house->RemapColor, TPF_USE_GRAD_PAL | TPF_FULLSHADOW | TPF_6PT_GRAD, Rule->MessageDelay * TICKS_PER_MINUTE);
        }
    }
}


void EventClassExt::_Event_Produce()
{
    Houses[ID]->Begin_Production(Data.Specific.Type, Data.Specific.ID, false);
}


void EventClassExt::_Event_Suspend()
{
    Houses[ID]->Suspend_Production(Data.Specific.Type);
}


void EventClassExt::_Event_Abandon()
{
    Houses[ID]->Abandon_Production(Data.Specific.Type, Data.Specific.ID);
}


void EventClassExt::_Event_Primary()
{
    BuildingClass* building = Data.Target.Whom.As_Building();
    if (building && building->IsActive) {
        building->Toggle_Primary();
    }
}


void EventClassExt::_Event_Special_Place()
{
    Houses[ID]->Place_Special_Blast(static_cast<SpecialWeaponType>(Data.Special.ID), Data.Special.Cell);
}


void EventClassExt::_Event_Exit()
{
    PlayerAborts = true;
}


void EventClassExt::_Event_Animation()
{
    Coordinate coord = Coordinate(Data.Anim.Where.X, Data.Anim.Where.Y, 0);
    coord.Z = Map.Get_Cell_Height(coord);
    if (Map[coord].Bit2_16) {
        coord.Z += BridgeCellHeight;
    }

    if (Data.Anim.What != ANIM_NONE) {
        new AnimClass(AnimTypes[Data.Anim.What], coord);
    }
    else {
        new AnimClass(Rule->MoveFlash, coord);
    }
}


void EventClassExt::_Event_Repair()
{
    TechnoClass* techno = Data.Target.Whom.As_Techno();

    if (techno && techno->IsActive) {
        techno->Repair(-1);
    }
}


void EventClassExt::_Event_Sell()
{
    TechnoClass* techno = Data.Target.Whom.As_Techno();

    if (techno && techno->IsActive && techno->House->Get_Heap_ID() == ID) {
        if (techno->What_Am_I() == RTTI_BUILDING || ((techno->What_Am_I() == RTTI_UNIT || techno->What_Am_I() == RTTI_AIRCRAFT) && Map[techno->Center_Coord()].Cell_Building() != nullptr)) {
            techno->Sell_Back(-1);
        }
    }
}


void EventClassExt::_Event_SellCell()
{
    Cell cell(Data.SellCell.Cell);
    Houses[ID]->Sell_Wall(cell, false);
}


void EventClassExt::_Event_Special()
{
    char txt[256];
    HouseClass* house = Houses[ID];

    if (house) {
        Special = Data.Options.Data;
        Scen->SpecialFlags = Data.Options.Data;
        std::snprintf(txt, std::size(txt), Fetch_String(TXT_SPECIAL_WARNING), house->IniName);
        Session.Messages.Add_Message(nullptr, 0, txt, house->RemapColor, TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, 1200);
        Map.Flag_To_Redraw(false);
    }
}


void EventClassExt::_Event_Response_Time()
{
    Session.MaxAhead = Data.FrameInfo.Delay;
}


void EventClassExt::_Event_SaveGame()
{
    /**
     *  Mark that we'd like to save the game.
     */
    Vinifera_DoSave = true;
}


void EventClassExt::_Event_Archive()
{
    TechnoClass* techno = Data.NavCom.Whom.As_Techno();
    if (techno && techno->IsActive && techno->Get_Mission() != MISSION_DECONSTRUCTION) {
        techno->Assign_Archive_Target(Data.NavCom.Where.As_Abstract());
    }
}


void EventClassExt::_Event_AddPlayer()
{
    if (ID != PlayerPtr->Get_Heap_ID()) {
        delete[] Data.Variable.Pointer;
    }
}


void EventClassExt::_Event_Timing()
{
    if (Vinifera_SpawnerActive && !ProtocolZero::Enable)
        Data.Timing.MaxAhead -= Scen->SpecialFlags.IsFogOfWar ? 10 : 0;

    /**
     *  If MaxAhead is about to increase, we're vulnerable to a Packet-
     *  Received-Too-Late error, if any system generates an event after
     *  this TIMING event, but before it executes.  So, record the
     *  period of vulnerability's frame start & end values, so we
     *  can reschedule these events to execute after it's over.
     */
    if (Data.Timing.MaxAhead > Session.MaxAhead || Data.Timing.FrameSendRate > Session.FrameSendRate) {
        NewMaxAheadFrame1 = Frame;
        NewMaxAheadFrame2 = Data.Timing.FrameSendRate * ((Data.Timing.FrameSendRate + Data.Timing.MaxAhead + Frame - 1) / Data.Timing.FrameSendRate);
    }
    else {
        NewMaxAheadFrame1 = 0;
        NewMaxAheadFrame2 = 0;
    }

    Session.DesiredFrameRate = Data.Timing.DesiredFrameRate;
    Session.MaxAhead = Data.Timing.MaxAhead;
    Session.MaxMaxAhead = std::max(Session.MaxMaxAhead, Session.MaxAhead);
    Session.FrameSendRate = Data.Timing.FrameSendRate;
}


void EventClassExt::_Event_Process_Time()
{
    for (int i = 0; i < Session.Players.Count(); i++) {
        if (ID == Session.Players[i]->Player.ID) {
            Session.Players[i]->Player.ProcessTime = Data.ProcessTime.AverageTicks;
            break;
        }
    }
}


void EventClassExt::_Event_PageUser()
{
    if (!Session.Play) {
        SpecialDialog = SDLG_WOL_OPTIONS;
    }
}


void EventClassExt::_Event_RemovePlayer()
{
    DEBUG_INFO("Executing REMOVEPLAYER event. Frame is %d\n", Frame);
    HouseClass* house = Houses[Data.General.Value];

    if ((Session.Type == GAME_INTERNET && PlanetWestwoodTournament) || (Vinifera_SpawnerActive && Session.Type == GAME_IPX && Vinifera_SpawnerConfig->AutoSurrender)) {
        house->Flag_To_Die();
    }
    else if (house->Is_Human_Control()) {
        house->AI_Takeover();
    }
}


void EventClassExt::_Event_LatencyFudge()
{
    char txt[256];

    DEBUG_INFO("Executing LATENCYFUDGE event. Frame is %d\n", Frame);
    Session.LatencyFudge = Data.General.Value;
    DEBUG_INFO("LatencyFudge is %d\n", Session.LatencyFudge);

    HouseClass* house = Houses[ID];
    if (house != PlayerPtr && house != nullptr) {
        const char* text = Fetch_String(TXT_PLAYER_CHANGED_LATENCY);

        if (text && std::strlen(text)) {
            std::snprintf(txt, std::size(txt), text, house->IniName);
            Session.Messages.Add_Message(nullptr, 0, txt, house->RemapColor, TPF_USE_GRAD_PAL | TPF_FULLSHADOW | TPF_6PT_GRAD, Rule->MessageDelay * TICKS_PER_MINUTE);
        }
    }
}


void EventClassExt::_Execute()
{
    /**
     *  If it's one of our events, hand it over to our class to execute.
     */
    if (ViniferaEventClass::Is_Vinifera_Event(static_cast<ViniferaEventType>(Type)))
    {
        reinterpret_cast<ViniferaEventClass*>(this)->Execute();
        return;
    }

    switch (Type) {

        /**
         *  Turn a building's power on.
         */
    case EVENT_POWERON:
        _Event_PowerOn();
        break;

        /**
         *  Turn a building's power off.
         */
    case EVENT_POWEROFF:
        _Event_PowerOff();
        break;

        /**
         *  Make or break alliance.
         */
    case EVENT_ALLY:
        _Event_Ally();
        break;

        /**
         *  This is the general purpose mission control event. Most player
         *  action routes through this event. It sets a unit's mission, TarCom,
         *  and NavCom to the values specified.
         */
    case EVENT_MEGAMISSION_F:
    case EVENT_MEGAMISSION:
        _Event_MegaMission();
        break;

        /**
         *  Request that the unit/infantry/aircraft go into idle mode.
         */
    case EVENT_IDLE:
        _Event_Idle();
        break;

        /**
         *  Request that the unit/infantry/aircraft scatter from its current location.
         */
    case EVENT_SCATTER:
        _Event_Scatter();
        break;

        /**
         *  Special self destruct action requested. This is active in the multiplayer mode.
         */
    case EVENT_DESTRUCT:
        _Event_Destruct();
        break;

        /**
         *  Request that the unit deploys.
         */
    case EVENT_DEPLOY:
        _Event_Deploy();
        break;

        /**
         *  This event will place the specified object at the specified location.
         *  The event is used to place newly constructed buildings down on the map. The
         *  object type is specified. From this object type, the house can determine the
         *  exact factory and real object pointer to use.
         */
    case EVENT_PLACE:
        _Event_Place();
        break;

        /**
         *  Process the options menu, unless we're playing back a recording.
         */
    case EVENT_OPTIONS:
        _Event_Options();
        break;

        /**
         *  Process the options Game Speed
         */
    case EVENT_GAMESPEED:
        _Event_GameSpeed();
        break;

        /**
         *  This event starts production of the specified object type. The house can
         *  determine from the type and ID value, what object to begin production on and
         *  what factory to use.
         */
    case EVENT_PRODUCE:
        _Event_Produce();
        break;

        /**
         *  This event is generated when the player puts production on hold. From the
         *  object type, the factory can be inferred.
         */
    case EVENT_SUSPEND:
        _Event_Suspend();
        break;

        /**
         *  This event is generated when the player cancels production of the specified
         *  object type. From the object type, the exact factory can be inferred.
         */
    case EVENT_ABANDON:
        _Event_Abandon();
        break;

        /**
         *  Toggles the primary factory state of the specified building.
         */
    case EVENT_PRIMARY:
        _Event_Primary();
        break;

        /**
         *  If we are placing down the ion cannon blast then lets take
         *  care of it.
         */
    case EVENT_SPECIAL_PLACE:
        _Event_Special_Place();
        break;

        /**
         *  Exit the game.
         */
    case EVENT_EXIT:
        _Event_Exit();
        break;

        /**
         *  This even is used to trigger an animation that is generated as a direct
         *  result of player intervention.
         */
    case EVENT_ANIMATION:
        _Event_Animation();
        break;

        /**
         *  Starts or stops repair on the specified object. This event is triggered by the
         *  player clicking the repair wrench on a building.
         */
    case EVENT_REPAIR:
        _Event_Repair();
        break;

        /**
         *  Tells a building/unit to sell. This event is triggered by the player clicking the
         *  sell animating cursor over the building or unit.
         */
    case EVENT_SELL:
        _Event_Sell();
        break;

        /**
         *  Tells the wall at the specified location to sell off.
         */
    case EVENT_SELLCELL:
        _Event_SellCell();
        break;

        /**
         *  Update the special control flags. This is necessary so that in a multiplay
         *  game, all machines will agree on the rules. If these options change during
         *  game play, then all players are informed that options have changed.
         */
    case EVENT_SPECIAL:
        _Event_Special();
        break;

        /**
         *  Adjust connection timing for multiplayer games
         */
    case EVENT_RESPONSE_TIME:
        _Event_Response_Time();
        break;

        /**
         *  Save a multiplayer game (this event is only generated in multiplayer mode)
         */
    case EVENT_SAVEGAME:
        _Event_SaveGame();
        break;

        /**
         *  Update the archive target for this building.
         */
    case EVENT_ARCHIVE:
        _Event_Archive();
        break;

        /**
         *  Add a new player to the game:
         *  - Form a network connection to him
         *  - Add his name, ID, House etc to our list of players
         *  - Re-sort the ID array
         *  - Place his units on the map
         */
    case EVENT_ADDPLAYER:
        _Event_AddPlayer();
        break;

        /**
         *  This event tells all systems to use new timing values. It's like
         *  RESPONSE_TIME, only it works. It's only used with the
         *  COMM_MULTI_E_COMP protocol.
         */
    case EVENT_TIMING:
        _Event_Timing();
        break;

        /**
         *  This event tells all systems what the other systems' process
         *  timing requirements are; it's used to compute a desired frame rate
         *  for the game.
         */
    case EVENT_PROCESS_TIME:
        _Event_Process_Time();
        break;

        /**
         *  Opens some WOL dialog, perhaps an in-game chat?
         */
    case EVENT_PAGEUSER:
        _Event_PageUser();
        break;

        /**
         *  Remove a play from the game.
         */
    case EVENT_REMOVEPLAYER:
        _Event_RemovePlayer();
        break;

        /**
         *  Change network latency fudge.
         */
    case EVENT_LATENCYFUDGE:
        _Event_LatencyFudge();
        break;

        /**
         *  Default: do nothing.
         */
    case EVENT_FRAMESYNC:
    case EVENT_MESSAGE:
    case EVENT_FRAMEINFO:
    default:
        break;
    }
}


/**
 *  Patch event length in Add_Compressed_Events.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_Add_Compressed_Events_ViniferaEvent_Length)
{
    GET_REGISTER_STATIC(unsigned char, eventtype, cl);
    static unsigned char eventlength;

    _asm pushad

    if (ViniferaEventClass::Is_Vinifera_Event(static_cast<ViniferaEventType>(eventtype)))
    {
        eventlength = ViniferaEventClass::Event_Length(static_cast<ViniferaEventType>(eventtype));
    }
    else
    {
        eventlength = EventClass::Event_Length(static_cast<EventType>(eventtype));
    }

    if (eventtype == EVENT_ADDPLAYER)
    {
        _asm popad
        _asm mov bl, eventlength
        JMP_REG(esi, 0x005B45EA);
    }
    else
    {
        _asm popad
        _asm mov bl, eventlength
        JMP_REG(esi, 0x005B45F3);
    }
}


/**
 *  Extract_Compressed_Events -- extracts events from a packet.
 *
 *  @author: 11/21/1995 DRD - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
static int Vinifera_Extract_Compressed_Events(void* buf, int bufsize)
{
    int pos = 0;                    // current buffer parsing position
    int leftover = bufsize;         // # bytes left to process
    EventClass* event;              // event ptr for parsing buffer
    int count = 0;                  // # events processed
    int datasize = 0;               // size of data to copy
    EventClass eventdata;           // stores Frame, ID, etc
    unsigned char numunits = 0;     // # units stored in compressed MegaMissions

    /**
     *  Clear work event structure.
     */
    std::memset(&eventdata, 0, sizeof(EventClass));

    /**
     *  Assume the first event is a FRAMEINFO event
     *  Init 'datasize' to the amount of data to copy, minus the EventType value
     *  For the 1st packet only, this will include all info before the Data
     *  union, plus the size of the FrameInfo structure, minus the EventType size.
     */
    datasize = (offsetof(EventClass, Data) + sizeof(EventClass::Data.FrameInfo)) - sizeof(EventType);
    event = reinterpret_cast<EventClass*>(static_cast<char*>(buf) + pos);

    while (leftover >= datasize + (int)sizeof(EventType))
    {
        /**
         *  Add event to the DoList, only if it's not a FRAMESYNC
         *  (but FRAMEINFO's do get added.)
         */
        if (event->Type != EVENT_FRAMESYNC)
        {
            /**
             *  Initialize the common data from the FRAMEINFO event.
             *  keeping IsExecuted 0
             */
            if (event->Type == EVENT_FRAMEINFO)
            {
                eventdata.Frame = event->Frame;
                eventdata.ID = event->ID;

                /**
                 *  Adjust position past the common data.
                 */
                pos += offsetof(EventClass, Data) - sizeof(EventType);
                leftover -= offsetof(EventClass, Data) - sizeof(EventType);
            }

            /**
             *  If MEGAMISSION event get the number of units (events to generate).
             */
            else if (event->Type == EVENT_MEGAMISSION)
            {
                numunits = *(static_cast<unsigned char*>(buf) + pos + sizeof(eventdata.Type));
                pos += sizeof(numunits);
                leftover -= sizeof(numunits);
            }

            /**
             *  Clear the union data portion of the event.
             */
            memset(&eventdata.Data, 0, sizeof(eventdata.Data));
            eventdata.Type = event->Type;
            datasize = ViniferaEventClass::Event_Length(eventdata.Type);

            switch (eventdata.Type)
            {
            case EVENT_RESPONSE_TIME:
                memcpy(&eventdata.Data.FrameInfo.Delay, static_cast<char*>(buf) + pos + sizeof(EventType), datasize);
                break;

            case EVENT_ADDPLAYER:
                memcpy(&eventdata.Data.Variable.Size, static_cast<char*>(buf) + pos + sizeof(EventType), datasize);

                eventdata.Data.Variable.Pointer = new char[eventdata.Data.Variable.Size];
                memcpy(eventdata.Data.Variable.Pointer, static_cast<char*>(buf) + pos + sizeof(EventType) + datasize, eventdata.Data.Variable.Size);

                pos += eventdata.Data.Variable.Size;
                leftover -= eventdata.Data.Variable.Size;

                break;

            case EVENT_MEGAMISSION:
                memcpy(&eventdata.Data.MegaMission, static_cast<char*>(buf) + pos + sizeof(EventType), datasize);

                if (numunits > 1)
                {
                    pos += datasize + sizeof(EventType);
                    leftover -= datasize + sizeof(EventType);
                    datasize = sizeof(eventdata.Data.MegaMission.Whom);

                    while (numunits)
                    {
                        if (!DoList.Add(eventdata))
                            return -1;

                        /**
                         *  Keep count of how many events we add to the queue.
                         */
                        count++;
                        numunits--;
                        memcpy(&eventdata.Data.MegaMission.Whom, static_cast<char*>(buf) + pos, datasize);

                        /**
                         *  If one unit left fall thru to normal code.
                         */
                        if (numunits == 1)
                        {
                            datasize -= sizeof(EventType);
                            break;
                        }
                        else
                        {
                            pos += datasize;
                            leftover -= datasize;
                        }
                    }
                }
                break;

            default:
                memcpy(&eventdata.Data, static_cast<char*>(buf) + pos + sizeof(EventType), datasize);
                break;
            }

            if (!DoList.Add(eventdata))
            {
                if (eventdata.Type == EVENT_ADDPLAYER)
                    delete[] eventdata.Data.Variable.Pointer;

                return -1;
            }

            /**
             *  Keep count of how many events we add to the queue.
             */
            count++;

            pos += datasize + sizeof(EventType);
            leftover -= datasize + sizeof(EventType);

            if (leftover)
            {
                event = reinterpret_cast<EventClass*>(static_cast<char*>(buf) + pos);
                datasize = ViniferaEventClass::Event_Length(event->Type);
                if (event->Type == EVENT_MEGAMISSION)
                    datasize += sizeof(numunits);
            }
        }
        /**
         *  FRAMESYNC event: This >should< be the only event in the buffer,
         *  and it will be uncompressed.
         */
        else
        {
            pos += datasize + sizeof(EventType);
            leftover -= datasize + sizeof(EventType);
            event = reinterpret_cast<EventClass*>(static_cast<char*>(buf) + pos);

            /**
             *  Size of FRAMESYNC event - EventType size.
             */
            datasize = offsetof(EventClass, Data) + sizeof(EventClass::Data.FrameInfo) - sizeof(EventType);
        }
    }

    return count;

}


/**
 *  Main function for patching the hooks.
 */
void EventClassExtension_Hooks()
{
    Patch_Jump(0x00494280, &EventClassExt::_Execute);
    Patch_Jump(0x005B45D5, &_Add_Compressed_Events_ViniferaEvent_Length);
    Patch_Jump(0x005B4A40, &Vinifera_Extract_Compressed_Events);
}
