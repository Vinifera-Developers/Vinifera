/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BEACON.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         New classes adding Beacons to TS.
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
#include "beacon.h"
#include "always.h"
#include "debughandler.h"
#include "drawshape.h"
#include "dsurface.h"
#include "extension_globals.h"
#include "house.h"
#include "ipxmgr.h"
#include "miscutil.h"
#include "mixfile.h"
#include "mouse.h"
#include "radarevent.h"
#include "rawfile.h"
#include "rules.h"
#include "rulesext.h"
#include "session.h"
#include "shapeset.h"
#include "tactical.h"
#include "tacticalext.h"
#include "tibsun_functions.h"
#include "uicontrol.h"
#include "vinifera_defines.h"
#include "voc.h"
#include "vox.h"
#include "wwfont.h"
#include <cstring>

BeaconManagerClass BeaconManager;
ShapeSet const* BeaconManagerClass::BeaconArt = nullptr;
ShapeSet const* BeaconManagerClass::RadarBeaconArt = nullptr;


BeaconClass::BeaconClass() :
    Position(COORD_NONE),
    HasOwner(false),
    IsSelected(false),
    Owner(HOUSE_NONE)
{
}


void BeaconClass::Draw(Surface* surface, Rect cliprect) const
{
    ColorScheme* scheme = ColorSchemes[Houses[Owner]->Scheme];

    Point2D drawpoint;
    if (TacticalMap->Coord_To_Pixel(Position, drawpoint)) {
        int shapenum = Get_Shape_Frame();

        Draw_Shape(*surface, *scheme->Converter, BeaconManagerClass::BeaconArt, shapenum, drawpoint, cliprect, SHAPE_CENTER | SHAPE_WIN_REL);
        if (!Text.empty()) {
            std::string text = Text;

            if (IsSelected && Session.Messages.Is_Edit()) {
                if (timeGetTime() % 500 < 250) {
                    char* underscore = std::strrchr(&text[0], '_');
                    if (underscore && std::strlen(underscore) == 1) {
                        *underscore = ' ';
                    }
                }
            }

            if (!IsSelected || !Session.Messages.Is_Edit()) {
                char* underscore = std::strrchr(&text[0], '_');
                if (underscore && std::strlen(underscore) == 1) {
                    *underscore = '\0';
                    text.resize(std::strlen(&text[0]));
                } else {
                    char* space = std::strrchr(&text[0], ' ');
                    if (space && std::strlen(space) == 1) {
                        *space = '\0';
                        text.resize(std::strlen(&text[0]));
                    }
                }
            }

            TacticalMapExtension->Draw_Beacon_Text(text, *scheme, drawpoint, cliprect, true, UIControls->BeaconTextOffset);
        }
    }
}


void BeaconClass::Set(Coord coord, HousesType owner)
{
    if (coord != COORD_NONE) {
        Position = coord;
    }

    if (owner < MAX_PLAYERS) {
        Owner = owner;
        HasOwner = true;
    }
}


void BeaconClass::Select(bool selected)
{
    IsSelected = selected;
}


void BeaconClass::Disown()
{
    HasOwner = false;
}


void BeaconClass::Set_Text(char const* text)
{
    Text.clear();
    if (text != nullptr) {
        Text = text;
    }
}


void BeaconClass::Draw_On_Radar(Surface* surface, Rect cliprect, bool is_doer)
{
    int shapenum = BeaconManager.Get_Radar_Shape_Frame();

    if (shapenum < BeaconManager.RadarBeaconFrameCount + 1 || is_doer) {
        Point2D drawpoint = Map.Coord_To_Radar_Pixel(Position, true);
        if (shapenum < BeaconManager.RadarBeaconFrameCount && !is_doer) {
            Draw_Shape(*surface, *ColorSchemes[Houses[Owner]->Scheme]->Converter, BeaconManagerClass::RadarBeaconArt, shapenum, drawpoint, cliprect, SHAPE_CENTER | SHAPE_WIN_REL);
        }

        Point2D size(BeaconManager.RadarBeaconWidth, BeaconManager.RadarBeaconHeight);
        Point2D topleft = drawpoint - size / 2;
        Map.LastDrawRect = Intersect(Union(Map.LastDrawRect, Rect(topleft + Map.RadarRect.TopLeft, BeaconManager.RadarBeaconWidth, BeaconManager.RadarBeaconHeight)), Map.RadarRect);

        for (int y = topleft.Y; y < topleft.Y + size.Y; y++) {
            for (int x = topleft.X; x < topleft.X + size.X; x++) {
                Map.Radar_Pixel(Point2D(x, y));
            }
        }
    }
}


bool BeaconClass::Is_Visible_To_Player() const
{
    if (HasOwner && PlayerPtr->Is_Ally(Owner) && Houses[Owner]->Is_Ally(PlayerPtr) && !Houses[Owner]->IsDefeated) {
        return true;
    }
    return false;
}


int BeaconClass::Get_Shape_Frame() const
{
    float rate = static_cast<float>(UIControls->BeaconAnimFramesPerSecond);
    int frame = static_cast<int>(static_cast<float>(timeGetTime()) * rate / 1000.0f) % (BeaconManager.BeaconFrameCount / 2);
    int shapenum = frame + (IsSelected ? BeaconManager.BeaconFrameCount / 2 : 0);
    return shapenum;
}


BeaconManagerClass::BeaconManagerClass() :
    BeaconWidth(0),
    BeaconHeight(0),
    BeaconFrameCount(0),
    RadarBeaconWidth(0),
    RadarBeaconHeight(0),
    RadarBeaconFrameCount(0),
    RadarBeaconAnimPeriod(1)
{
}


BeaconManagerClass::~BeaconManagerClass()
{
    Reset();
}


void BeaconManagerClass::Reset()
{
    for (auto& array : Beacons) {
        array.clear();
    }
}


void BeaconManagerClass::Load_Art()
{
    RawFileClass pbeacon("PBEACON.SHP");
    BeaconArt = static_cast<ShapeSet const*>(Load_Alloc_Data(pbeacon));
    if (BeaconArt == nullptr) {
        BeaconArt = static_cast<ShapeSet const*>(MFCD::Retrieve("PBEACON.SHP"));
    }
    if (BeaconArt != nullptr) {
        BeaconWidth = BeaconArt->Get_Width();
        BeaconHeight = BeaconArt->Get_Height();
        BeaconFrameCount = BeaconArt->Get_Count();
    } else {
        DEBUG_INFO("Failed to load BeaconArt");
    }

    RawFileClass rdrbeacon("RDRBEACN.SHP");
    RadarBeaconArt = static_cast<ShapeSet const*>(Load_Alloc_Data(rdrbeacon));
    if (RadarBeaconArt == nullptr) {
        RadarBeaconArt = static_cast<ShapeSet const*>(MFCD::Retrieve("RDRBEACN.SHP"));
    }
    if (RadarBeaconArt != nullptr) {
        RadarBeaconWidth = RadarBeaconArt->Get_Width();
        RadarBeaconHeight = RadarBeaconArt->Get_Height();
        RadarBeaconFrameCount = RadarBeaconArt->Get_Count();
        RadarBeaconAnimPeriod = 4 * RadarBeaconFrameCount;
    } else {
        DEBUG_INFO("Failed to load RadarBeaconArt");
    }
}


void BeaconManagerClass::Draw(Surface* surface, Rect cliprect)
{
    for (auto& array : Beacons) {
        for (auto& beacon : array) {
            if (beacon->Is_Visible_To_Player()) {
                beacon->Draw(surface, cliprect);
            }
        }
    }
}


void BeaconManagerClass::Place_Beacon(HousesType house, Coord coord, int beacon_id, char const* text)
{
    BeaconClass* beacon = new BeaconClass;

    if (beacon_id != -1) {
        Beacons[house].erase(Beacons[house].begin() + beacon_id);
        Beacons[house].emplace(Beacons[house].begin() + beacon_id, beacon);
    } else {
        if (RuleExtension->MaxBeacons > 0 && Beacons[house].size() >= RuleExtension->MaxBeacons) {
            Delete_Beacon(house, 0);
        }
        Beacons[house].emplace_back(beacon);
    }

    DEBUG_INFO("Placing beacon: (%d, %d, %d)\n", coord.X, coord.Y, coord.Z);
    beacon->Set(coord, house);

    if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
        if (house == PlayerPtr->HeapID) {
            //Session.Messages.Add_Message(nullptr, 0, "Select a beacon and hit 'enter' key to enter text", PlayerPtr->Scheme, TPF_USE_GRAD_PAL | TPF_FULLSHADOW | TPF_6PT_GRAD, 225);
            //Session.Messages.Add_Message(nullptr, 0, "Select a beacon and hit 'delete' key to remove", PlayerPtr->Scheme, TPF_USE_GRAD_PAL | TPF_FULLSHADOW | TPF_6PT_GRAD, 225);
            Speak(RuleExtension->PlaceBeaconVoice);
            Sound_Effect(RuleExtension->PlaceBeaconSound);
            Send_Beacon_Place(coord, house, beacon_id);
        } else if (beacon->Is_Visible_To_Player()) {
            if (Submit_Radar_Event(RADAREVENT_DROPZONE, coord.As_Cell())) { // abusing RADAREVENT_DROPZONE for beacons
                Speak(RuleExtension->DetectBeaconVoice);
            }
        }
    }

    if (text != nullptr) {
        Find_Beacon(beacon, house, beacon_id);
        Set_Beacon_Text(text, house, beacon_id, house == PlayerPtr->HeapID);
    }
}


bool BeaconManagerClass::Select_Beacon(Coord coord)
{
    BeaconClass* beacon = Beacon_At(coord);
    if (beacon != nullptr) {
        Unselect_All();
        beacon->Select(true);
        return true;
    }
    return false;
}


void BeaconManagerClass::Unselect_All_Beacons()
{
    for (auto& array : Beacons) {
        for (auto& beacon : array) {
            beacon->Select(false);
        }
    }
}


BeaconClass* BeaconManagerClass::Beacon_At(Coord coord)
{
    for (auto& array : Beacons) {
        for (auto& beacon : array) {
            if (coord.Distance_To(beacon->Position) < 128) {
                return beacon.get();
            }
        }
    }

    return nullptr;
}

bool BeaconManagerClass::Find_Beacon(BeaconClass const* beacon, HousesType& house, int& beacon_id)
{
    for (HousesType i = HOUSE_FIRST; i < std::size(Beacons); i++) {
        auto& vec = Beacons[i];
        auto it = std::find_if(vec.begin(), vec.end(), [beacon](const std::unique_ptr<BeaconClass>& ptr) { return ptr.get() == beacon; });
        if (it != vec.end()) {
            house = i;
            beacon_id = it - vec.begin();
            return true;
        }
    }

    return false;
}


BeaconClass* BeaconManagerClass::Find_Selected_Beacon(HousesType house)
{
    for (auto& array : Beacons) {
        for (auto& beacon : array) {
            if (beacon->IsSelected && (house == HOUSE_NONE || house == beacon->Owner)) {
                return beacon.get();
            }
        }
    }

    return nullptr;
}


void BeaconManagerClass::Delete_Beacon(HousesType house, int beacon_id)
{
    BeaconClass* beacon = nullptr;
    bool is_local_action = false;

    if (house == HOUSE_NONE && beacon_id == -1) {
        is_local_action = true;
        beacon = Find_Selected_Beacon(house);
    } else {
        beacon = Beacons[house][beacon_id].get();
    }

    if (beacon != nullptr) {

        if (beacon->Owner != PlayerPtr->HeapID && is_local_action) {
            beacon->Disown();
            beacon->Select(false);
        } else {
            if (beacon->Is_Visible_To_Player()) {
                beacon->Draw_On_Radar(Map.RadarSurface, Map.RadarSurface->Get_Rect(), true);
            }

            if (Find_Beacon(beacon, house, beacon_id)) {
                Beacons[house].erase(Beacons[house].begin() + beacon_id);
                if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
                    if (house == PlayerPtr->HeapID && is_local_action) {
                        Send_Beacon_Delete(house, beacon_id);
                    }
                }
            }
        }
    }
}


void BeaconManagerClass::Delete_Owned_Beacons(HousesType house)
{
    while (!Beacons[house].empty()) {
        Delete_Beacon(house, 0);
    }
}


void BeaconManagerClass::Set_Beacon_Text(char const* text, HousesType house, int beacon_id, bool send)
{
    BeaconClass* beacon = nullptr;
    bool is_local_action = false;

    if (house == HOUSE_NONE && beacon_id == -1) {
        is_local_action = true;
        beacon = Find_Selected_Beacon(house);
    } else {
        beacon = Beacons[house][beacon_id].get();
    }

    if (beacon != nullptr) {
        beacon->Set_Text(text);
        if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
            if (is_local_action && beacon->Owner == PlayerPtr->HeapID && send) {
                for (int i = 0; i < Beacons[PlayerPtr->HeapID].size(); i++) {
                    if (Beacons[PlayerPtr->HeapID][i].get() == beacon) {
                        beacon_id = i;
                        break;
                    }
                }
                Send_Set_Beacon_Text(text, house, beacon_id);
            } else if (beacon->Is_Visible_To_Player()) {
                Submit_Radar_Event(RADAREVENT_DROPZONE, beacon->Position.As_Cell()); // abusing RADAREVENT_DROPZONE for beacons
            }
        }
    }
}


void BeaconManagerClass::Draw_On_Radar(Surface* surface, Rect cliprect)
{
    if (Get_Radar_Shape_Frame() < BeaconManager.RadarBeaconFrameCount + 1) {
        for (auto& array : Beacons) {
            for (auto& beacon : array) {
                if (beacon->Is_Visible_To_Player()) {
                    beacon->Draw_On_Radar(surface, cliprect, false);
                }
            }
        }
    }
}


bool BeaconManagerClass::Is_To_Redraw_Radar()
{
    bool visible = false;
    for (auto& array : Beacons) {
        for (auto& beacon : array) {
            if (beacon->Is_Visible_To_Player()) {
                visible = true;
                goto breakout;
            }
        }
    }

    breakout:
    if (visible && Get_Radar_Shape_Frame() < BeaconManager.RadarBeaconFrameCount + 1) {
        return true;
    }
    return false;
}


void BeaconManagerClass::Send_Beacon_Place(Coord coord, HousesType house, int beacon_id)
{
    ExtGlobalPacketType packet;
    packet.Command = EXT_NET_BEACON_PLACE;
    strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name));
    packet.PlaceBeacon.Number = beacon_id;
    packet.PlaceBeacon.Position = coord;
    packet.PlaceBeacon.House = house;
    for (int i = 1; i < Session.Players.Count(); i++) {
        DEBUG_INFO("Sending beacon placement to %s\n", static_cast<const char*>(Session.Players[i]->Name));
        Ipx.Send_Global_Message(&packet, sizeof(packet), true, &Session.Players[i]->Address);
    }
}


void BeaconManagerClass::Send_Beacon_Delete(HousesType house, int beacon_id)
{
    ExtGlobalPacketType packet;
    packet.Command = EXT_NET_BEACON_DELETE;
    strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name));
    packet.DeleteBeacon.House = house;
    packet.DeleteBeacon.Number = beacon_id;
    for (int i = 1; i < Session.Players.Count(); i++) {
        DEBUG_INFO("Sending beacon delete to %s\n", static_cast<const char*>(Session.Players[i]->Name));
        Ipx.Send_Global_Message(&packet, sizeof(packet), true, &Session.Players[i]->Address);
    }
}


void BeaconManagerClass::Send_Set_Beacon_Text(char const * text, HousesType house, int beacon_id)
{
    ExtGlobalPacketType packet;
    packet.Command = EXT_NET_BEACON_TEXT;
    strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name));
    if (text != nullptr) {
        std::strncpy(packet.BeaconText.Text, text, std::size(packet.BeaconText.Text));
    } else {
        memset(packet.BeaconText.Text, '\0', sizeof(packet.BeaconText.Text));
    }

    packet.BeaconText.Number = beacon_id;
    packet.BeaconText.House = PlayerPtr->HeapID;

    if (beacon_id != -1) {
        for (int i = 1; i < Session.Players.Count(); i++) {
            DEBUG_INFO("Sending beacon text to %s\n", static_cast<const char*>(Session.Players[i]->Name));
            Ipx.Send_Global_Message(&packet, sizeof(packet), true, &Session.Players[i]->Address);
        }
    }
}


ActionType BeaconManagerClass::Pick_Beacon_Placement_Action()
{
    const bool altdown = Key_Down(Options.KeyForceMove1) || Key_Down(Options.KeyForceMove2);
    const bool ctrldown = Key_Down(Options.KeyForceAttack1) || Key_Down(Options.KeyForceAttack2);
    const bool shiftdown = Key_Down(Options.KeySelect1) || Key_Down(Options.KeySelect2);

    ExtActionType action = EXT_ACTION_PLACE_BEACON;

    if (ctrldown && altdown) {
        action = EXT_ACTION_PLACE_DEFEND_BEACON;
    } else if (ctrldown) {
        action = EXT_ACTION_PLACE_ATTACK_BEACON;
    } else if (altdown) {
        action = EXT_ACTION_PLACE_MOVE_BEACON;
    } else if (shiftdown) {
        action = EXT_ACTION_PLACE_EXPAND_BEACON;
    }

    return static_cast<ActionType>(action);
}


bool BeaconManagerClass::Is_Beacon_Placement_Action(ActionType action)
{
    switch (action) {
    case EXT_ACTION_PLACE_BEACON:
    case EXT_ACTION_PLACE_ATTACK_BEACON:
    case EXT_ACTION_PLACE_DEFEND_BEACON:
    case EXT_ACTION_PLACE_MOVE_BEACON:
    case EXT_ACTION_PLACE_EXPAND_BEACON:
        return true;
    default:
        return false;
    }
}


char const* BeaconManagerClass::Beacon_Text(ActionType action)
{
    switch (action) {
    case EXT_ACTION_PLACE_ATTACK_BEACON:
        return "Attack";
    case EXT_ACTION_PLACE_DEFEND_BEACON:
        return "Defend";
    case EXT_ACTION_PLACE_MOVE_BEACON:
        return "Move";
    case EXT_ACTION_PLACE_EXPAND_BEACON:
        return "Expand";
    case EXT_ACTION_PLACE_BEACON:
    default:
        return nullptr;
    }
}


int BeaconManagerClass::Get_Radar_Shape_Frame() const
{
    float rate = static_cast<float>(UIControls->RadarBeaconAnimFramesPerSecond);
    int shapenum = static_cast<int>(static_cast<float>(timeGetTime()) * rate / 1000.0f) % RadarBeaconAnimPeriod;
    return shapenum;
}