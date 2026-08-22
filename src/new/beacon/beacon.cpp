/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  New classes adding Beacons to TS.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "beacon.h"

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


/**
 *  Beacon manager instance.
 */
BeaconManagerClass BeaconManager;

/**
 *  Beacon art.
 */
ShapeSet const* BeaconManagerClass::BeaconArt = nullptr;
ShapeSet const* BeaconManagerClass::RadarBeaconArt = nullptr;


/**
 *  BeaconClass constructor.
 *
 *  @authors: ZivDero, tomsons26
 */
BeaconClass::BeaconClass() :
    ID(-1),
    Position(COORD_NONE),
    IsVisible(false),
    IsSelected(false),
    House(HOUSE_NONE)
{
}


/**
 *  Sets a beacon's position and house.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconClass::Set(Coord const& coord, HousesType house)
{
    if (coord != COORD_NONE) {
        Position = coord;
    }

    if (house < MAX_PLAYERS) {
        House = house;
        IsVisible = true;
    }
}


/**
 *  Selects/unselects a beacon.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconClass::Select(bool selected)
{
    IsSelected = selected;
}


/**
 *  Hides the beacon.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconClass::Hide()
{
    IsVisible = false;
}


/**
 *  Updates the displayed beacon text.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconClass::Set_Text(char const* text)
{
    Text.clear();
    if (text != nullptr) {
        Text = text;
    }
}


/**
 *  Checks whether the beacon is visible to the local player.
 *
 *  @authors: ZivDero, tomsons26
 */
bool BeaconClass::Is_Visible_To_Player() const
{
    if (IsVisible && PlayerPtr->Is_Ally(House) && Houses[House]->Is_Ally(PlayerPtr) && !Houses[House]->IsDefeated) {
        return true;
    }
    return false;
}


/**
 *  Returns the current animation frame index for rendering on the tactical map.
 *
 *  @authors: ZivDero, tomsons26
 */
int BeaconClass::Get_Shape_Frame() const
{
    float rate = static_cast<float>(UIControls->BeaconAnimFramesPerSecond);
    int frame = static_cast<int>(static_cast<float>(timeGetTime()) * rate / 1000.0f) % (BeaconManager.BeaconFrameCount / 2);
    int shapenum = frame + (IsSelected ? BeaconManager.BeaconFrameCount / 2 : 0);
    return shapenum;
}


/**
 *  Draws a beacon on the tactical map.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconClass::Draw(Surface* surface, Rect const& cliprect) const
{
    ColorScheme* scheme = ColorSchemes[Houses[House]->Scheme];

    Point2D drawpoint;
    if (TacticalMap->Coord_To_Pixel(Position, drawpoint)) {
        int shapenum = Get_Shape_Frame();

        /**
         *  Draw main beacon animation frame.
         */
        Draw_Shape(*surface, *scheme->Converter, BeaconManagerClass::BeaconArt, shapenum, drawpoint, cliprect, SHAPE_CENTER | SHAPE_WIN_REL);

        if (!Text.empty()) {
            std::string text = Text;

            /**
             *  Blinking underscore effect for text being edited.
             */
            if (IsSelected && Session.Messages.Is_Edit()) {
                if (timeGetTime() % 500 < 250) {
                    char* underscore = std::strrchr(&text[0], '_');
                    if (underscore && std::strlen(underscore) == 1) {
                        *underscore = ' ';
                    }
                }
            }

            /**
             *  Remove trailing underscore or space when not editing.
             */
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

            /**
             *  Draw label text below the beacon icon.
             */
            TacticalMapExtension->Draw_Beacon_Text(text, *scheme, drawpoint, cliprect, true, UIControls->BeaconTextOffset);
        }
    }
}


/**
 *  Draws this beacon on the radar.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconClass::Draw_On_Radar(Surface* surface, Rect const& cliprect, bool removed) const
{
    int shapenum = BeaconManager.Get_Radar_Shape_Frame();

    /**
     *  Only render the radar beacon if within visible animation range
     *  or if the beacon's been removed.
     */
    if (shapenum < BeaconManager.RadarBeaconFrameCount + 1 || removed) {
        Point2D drawpoint = Map.Coord_To_Radar_Pixel(Position, true);
        if (shapenum < BeaconManager.RadarBeaconFrameCount && !removed) {
            Draw_Shape(*surface, *ColorSchemes[Houses[House]->Scheme]->Converter, BeaconManagerClass::RadarBeaconArt, shapenum, drawpoint, cliprect, SHAPE_CENTER | SHAPE_WIN_REL);
        }

        /**
         *  Update radar redraw region to include beacon area.
         */
        Point2D size(BeaconManager.RadarBeaconWidth, BeaconManager.RadarBeaconHeight);
        Point2D topleft = drawpoint - size / 2;
        Map.LastDrawRect = Intersect(Union(Map.LastDrawRect, Rect(topleft + Map.RadarRect.TopLeft, BeaconManager.RadarBeaconWidth, BeaconManager.RadarBeaconHeight)), Map.RadarRect);

        /**
         *  Touch radar pixels for visual update.
         */
        for (int y = topleft.Y; y < topleft.Y + size.Y; y++) {
            for (int x = topleft.X; x < topleft.X + size.X; x++) {
                Map.Radar_Pixel(Point2D(x, y));
            }
        }
    }
}


/**
 *  BeaconManagerClass constructor.
 *
 *  @authors: ZivDero, tomsons26
 */
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


/**
 *  Clears all existing beacons.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Reset()
{
    for (auto& map : Beacons) {
        map.clear();
    }
}



/**
 *  Loads beacon art assets.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Load_Art()
{
    RawFileClass pbeacon("PBEACON.SHP");
    BeaconArt = static_cast<ShapeSet const*>(Load_Alloc_Data(pbeacon));
    if (BeaconArt == nullptr) {
        BeaconArt = static_cast<ShapeSet const*>(MFCD::Retrieve("PBEACON.SHP"));
    }

    /**
     *  Initialize tactical beacon metrics if loaded successfully.
     */
    if (BeaconArt != nullptr) {
        BeaconWidth = BeaconArt->Get_Width();
        BeaconHeight = BeaconArt->Get_Height();
        BeaconFrameCount = BeaconArt->Get_Count();
    } else {
        DEBUG_WARNING("Failed to load BeaconArt\n");
    }

    RawFileClass rdrbeacon("RDRBEACN.SHP");
    RadarBeaconArt = static_cast<ShapeSet const*>(Load_Alloc_Data(rdrbeacon));
    if (RadarBeaconArt == nullptr) {
        RadarBeaconArt = static_cast<ShapeSet const*>(MFCD::Retrieve("RDRBEACN.SHP"));
    }

    /**
     *  Initialize radar beacon metrics and animation timing.
     */
    if (RadarBeaconArt != nullptr) {
        RadarBeaconWidth = RadarBeaconArt->Get_Width();
        RadarBeaconHeight = RadarBeaconArt->Get_Height();
        RadarBeaconFrameCount = RadarBeaconArt->Get_Count();
        RadarBeaconAnimPeriod = 4 * RadarBeaconFrameCount;
    } else {
        DEBUG_WARNING("Failed to load RadarBeaconArt\n");
    }
}


/**
 *  Returns if beacons are currently enabled.
 *
 *  @authors: ZivDero
 */
bool BeaconManagerClass::Are_Beacons_Enabled()
{
    if (!RuleExtension->IsBeaconsEnabled) {
        return false;
    }

    /**
     *  Skirmish supports beacons, but they should be enabled separately if the modder
     *  thinks they're necessary.
     */
    if (Session.Type == GAME_SKIRMISH && !RuleExtension->IsSPBeacons) {
        return false;
    }

    /**
     *  Campaign, on the other hand, probably won't work well with how
     *  the beacon manager is designed, so disallow it.
     */
    if (Session.Type == GAME_NORMAL) {
        return false;
    }

    return true;
}


/**
 *  Draws all visible beacons on the tactical map.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Draw(Surface* surface, Rect const& cliprect) const
{
    for (auto& map : Beacons) {
        for (auto& pair : map) {
            auto& beacon = pair.second;
            if (beacon->Is_Visible_To_Player()) {
                beacon->Draw(surface, cliprect);
            }
        }
    }
}


/**
 *  Draws all visible beacons on the radar map.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Draw_On_Radar(Surface* surface, Rect const& cliprect) const
{
    if (Get_Radar_Shape_Frame() < RadarBeaconFrameCount + 1) {
        for (auto& map : Beacons) {
            for (auto& pair : map) {
                auto& beacon = pair.second;
                if (beacon->Is_Visible_To_Player()) {
                    beacon->Draw_On_Radar(surface, cliprect, false);
                }
            }
        }
    }
}


/**
 *  Determines if the radar needs to be redrawn for beacon animation.
 *
 *  @authors: ZivDero, tomsons26
 */
bool BeaconManagerClass::Is_To_Redraw_Radar() const
{
    bool visible = false;
    for (auto& map : Beacons) {
        for (auto& pair : map) {
            auto& beacon = pair.second;
            if (beacon->Is_Visible_To_Player()) {
                visible = true;
                goto breakout;
            }
        }
    }

breakout:
    if (visible && Get_Radar_Shape_Frame() < RadarBeaconFrameCount + 1) {
        return true;
    }
    return false;
}


/**
 *  Returns the current animation frame index for radar beacon rendering.
 *
 *  @authors: ZivDero, tomsons26
 */
int BeaconManagerClass::Get_Radar_Shape_Frame() const
{
    float rate = static_cast<float>(UIControls->RadarBeaconAnimFramesPerSecond);
    int shapenum = static_cast<int>(static_cast<float>(timeGetTime()) * rate / 1000.0f) % RadarBeaconAnimPeriod;
    return shapenum;
}


/**
 *  Creates and places a new beacon for a house at a given coordinate.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Place_Beacon(HousesType house, Coord const& coord, int beacon_id, char const* text)
{
    if (house < HOUSE_FIRST || house >= Session.Players.Count()) {
        return;
    }

    BeaconClass* beacon = new BeaconClass;

    if (beacon_id != -1) {

        /**
         *  Store the beacon at the given ID.
         */
        Beacons[house][beacon_id] = std::unique_ptr<BeaconClass>(beacon);
        beacon->ID = beacon_id;

    } else {

        /**
         *  Enforce per-house beacon limit.
         */
        if (RuleExtension->MaxBeacons > 0 && Beacons[house].size() >= RuleExtension->MaxBeacons) {
            Delete_Beacon(house, Beacons[house].begin()->second->ID);
        }

        /**
         *  Store the beacon with the current frame number as its ID.
         */
        Beacons[house].emplace(Frame, beacon);
        beacon->ID = Frame;
    }

    DEBUG_INFO("Placing beacon: ({}, {}, {})\n", coord.X, coord.Y, coord.Z);
    beacon->Set(coord, house);

    if (beacon->House == PlayerPtr->HeapID) {
        AudioVoxClass::Speak(RuleExtension->PlaceBeaconVoice);
        Sound_Effect(RuleExtension->PlaceBeaconSound);
    }

    if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {

        /**
         *  Send update to network peers if this is a local edit.
         */
        if (beacon->House == PlayerPtr->HeapID) {
            Send_Beacon_Place(beacon->Position, beacon->House, beacon->ID);
        }

        /**
         *  Otherwise, trigger a radar event for visibility.
         */
        else if (beacon->Is_Visible_To_Player()) {
            if (Submit_Radar_Event(RADAREVENT_DROPZONE, coord.As_Cell())) {
                AudioVoxClass::Speak(RuleExtension->DetectBeaconVoice);
            }
        }
    }

    /**
     *  Assign initial label text if provided.
     */
    if (text != nullptr) {
        Set_Beacon_Text(text, beacon->House, beacon->ID, house == PlayerPtr->HeapID);
    }
}


/**
 *  Deletes a beacon, locally or across the network.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Delete_Beacon(HousesType house, int beacon_id)
{
    BeaconClass* beacon = nullptr;
    bool is_local_action = false;

    /**
     *  Determine target beacon (direct or selected).
     */
    if (house == HOUSE_NONE && beacon_id == -1) {
        is_local_action = true;
        beacon = Find_Selected_Beacon(house);
    } else if (house >= HOUSE_FIRST && house < Session.Players.Count() && Beacons[house].find(beacon_id) != Beacons[house].end()) {
        beacon = Beacons[house][beacon_id].get();
    }

    if (beacon != nullptr) {

        /**
         *  If deleting remotely owned beacon, just hide it.
         */
        if (beacon->House != PlayerPtr->HeapID && is_local_action) {
            beacon->Hide();
            beacon->Select(false);
        } else {

            /**
             *  Redraw on the radar one last time before removing before removal.
             */
            if (beacon->Is_Visible_To_Player()) {
                beacon->Draw_On_Radar(Map.RadarSurface, Map.RadarSurface->Get_Rect(), true);
            }

            /**
             *  Remove beacon from container and propagate deletion.
             */
            house = beacon->House;
            beacon_id = beacon->ID;
            Beacons[house].erase(beacon_id);
            if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {
                if (house == PlayerPtr->HeapID && is_local_action) {
                    Send_Beacon_Delete(house, beacon_id);
                }
            }
        }
    }
}


/**
 *  Deletes all beacons owned by a given house.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Delete_Owned_Beacons(HousesType house)
{
    while (!Beacons[house].empty()) {
        Delete_Beacon(house, Beacons[house].begin()->second->ID);
    }
}


/**
 *  Sets or updates a beacon's text.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Set_Beacon_Text(char const* text, HousesType house, int beacon_id, bool send)
{
    BeaconClass* beacon = nullptr;

    if (house == HOUSE_NONE && beacon_id == -1) {
        beacon = Find_Selected_Beacon(house);
    } else if (house >= HOUSE_FIRST && house < Session.Players.Count() && Beacons[house].find(beacon_id) != Beacons[house].end()) {
        beacon = Beacons[house][beacon_id].get();
    }

    if (beacon != nullptr) {

        /**
         *  Set the text to the beacon.
         */
        beacon->Set_Text(text);

        if (Session.Type != GAME_NORMAL && Session.Type != GAME_SKIRMISH) {

            /**
             *  If it's the player setting the text for a beacon, send the update to other players.
             */
            if (beacon->House == PlayerPtr->HeapID && send) {
                Send_Set_Beacon_Text(text, beacon->House, beacon->ID);
            }

            /**
             *  Otherwise, we must've received an updated, so emit a radar event.
             */
            else if (beacon->House != PlayerPtr->HeapID && beacon->Is_Visible_To_Player()) {
                Submit_Radar_Event(RADAREVENT_DROPZONE, beacon->Position.As_Cell()); // abusing RADAREVENT_DROPZONE for beacons
            }
        }
    }
}



/**
 *  Selects a beacon located at the specified coordinate.
 *
 *  @authors: ZivDero, tomsons26
 */
bool BeaconManagerClass::Select_Beacon(Coord const& coord) const
{
    BeaconClass* beacon = Beacon_At(coord);
    if (beacon != nullptr) {
        Unselect_All();
        beacon->Select(true);
        return true;
    }
    return false;
}


/**
 *  Unselects all active beacons.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Unselect_All_Beacons()
{
    /**
     *  If we're editing a beacon, then clear its text and exit editing mode.
     */
    if (TacticalMapExtension->IsEditingBeaconText) {
        Set_Beacon_Text(nullptr, HOUSE_NONE, -1, true);
        Session.Messages.Remove_Edit();
        TacticalMapExtension->IsEditingBeaconText = false;
    }

    for (auto& map : Beacons) {
        for (auto& pair : map) {
            auto& beacon = pair.second;
            beacon->Select(false);
        }
    }
}


/**
 *  Finds the beacon instance located at a given coordinate.
 *
 *  @authors: ZivDero, tomsons26
 */
BeaconClass* BeaconManagerClass::Beacon_At(Coord const& coord) const
{
    for (auto& map : Beacons) {
        for (auto& pair : map) {
            auto& beacon = pair.second;
            if (coord.Distance_To(beacon->Position) < 128 && beacon->Is_Visible_To_Player()) {
                return beacon.get();
            }
        }
    }

    return nullptr;
}


/**
 *  Returns the currently selected beacon, optionally filtered by house.
 *
 *  @authors: ZivDero, tomsons26
 */
BeaconClass* BeaconManagerClass::Find_Selected_Beacon(HousesType house) const
{
    for (auto& map : Beacons) {
        for (auto& pair : map) {
            auto& beacon = pair.second;
            if (beacon->IsSelected && (house == HOUSE_NONE || house == beacon->House)) {
                return beacon.get();
            }
        }
    }

    return nullptr;
}


/**
 *  Handles input for the beacon message.
 *
 *  @authors: ZivDero
 */
void BeaconManagerClass::Input(KeyNumType input, bool finalize)
{
    BeaconClass* beacon = BeaconManager.Find_Selected_Beacon(PlayerPtr->HeapID);

    /**
     *  No beacon - nothing to edit.
     */
    if (beacon == nullptr) {
        return;
    }

    /**
     *  Escape always clears the beacon and de-selects it.
     */
    if (input == KN_ESC) {
        Set_Beacon_Text(nullptr, HOUSE_NONE, -1, true);
        TacticalMapExtension->IsEditingBeaconText = false;
        beacon->Select(false);
        return;
    }

    /**
     *  Otherwise, we need to update the beacon's text.
     */
    std::string buffer;
    if (auto edit = Session.Messages.Get_Edit_Buf()) {
        buffer.assign(edit);

        /**
         *  If we're done editing the text, it will contain a space at the end (just how
         *  MessageListClass works), remove it.
         */
        if (finalize && !buffer.empty()) {
            buffer.pop_back();
        }
    }

    /**
     *  If we're aren't done editing the text, then add an undescore
     *  (it will be make to blink by the rendering function).
     */
    if (!finalize) {
        buffer.push_back('_');
    }

    /**
     *  Set the text.
     */
    Set_Beacon_Text(buffer.c_str(), HOUSE_NONE, -1, finalize);

    /**
     *  If we're done with this beacon - de-select it and mark that we're done.
     */
    if (finalize) {
        beacon->Select(false);
        TacticalMapExtension->IsEditingBeaconText = false;
    }
}


/**
 *  Sends a network packet to notify other players of a new beacon.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Send_Beacon_Place(Coord const& coord, HousesType house, int beacon_id)
{
    ExtGlobalPacketType packet {};
    packet.Command = EXT_NET_BEACON_PLACE;
    std::strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name) - 1);
    packet.PlaceBeacon.Number = beacon_id;
    packet.PlaceBeacon.Position = coord;
    packet.PlaceBeacon.House = house;
    for (int i = 1; i < Session.Players.Count(); i++) {
        DEBUG_INFO("Sending beacon placement to {}\n", static_cast<const char*>(Session.Players[i]->Name));
        Ipx.Send_Global_Message(&packet, sizeof(packet), true, &Session.Players[i]->Address);
    }
}


/**
 *  Sends a network packet to delete a beacon remotely.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Send_Beacon_Delete(HousesType house, int beacon_id)
{
    ExtGlobalPacketType packet {};
    packet.Command = EXT_NET_BEACON_DELETE;
    std::strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name) - 1);
    packet.DeleteBeacon.House = house;
    packet.DeleteBeacon.Number = beacon_id;
    for (int i = 1; i < Session.Players.Count(); i++) {
        DEBUG_INFO("Sending beacon delete to {}\n", static_cast<const char*>(Session.Players[i]->Name));
        Ipx.Send_Global_Message(&packet, sizeof(packet), true, &Session.Players[i]->Address);
    }
}


/**
 *  Sends a network packet to update beacon text.
 *
 *  @authors: ZivDero, tomsons26
 */
void BeaconManagerClass::Send_Set_Beacon_Text(char const * text, HousesType house, int beacon_id)
{
    ExtGlobalPacketType packet {};
    packet.Command = EXT_NET_BEACON_TEXT;
    std::strncpy(packet.Name, Session.Players[0]->Name, sizeof(packet.Name) - 1);
    if (text != nullptr) {
        std::strncpy(packet.BeaconText.Text, text, std::size(packet.BeaconText.Text) - 1);
    }

    packet.BeaconText.Number = beacon_id;
    packet.BeaconText.House = house;

    if (beacon_id != -1) {
        for (int i = 1; i < Session.Players.Count(); i++) {
            DEBUG_INFO("Sending beacon text to {}\n", static_cast<const char*>(Session.Players[i]->Name));
            Ipx.Send_Global_Message(&packet, sizeof(packet), true, &Session.Players[i]->Address);
        }
    }
}


/**
 *  Determines which beacon placement action should be used based on key state.
 *
 *  @authors: ZivDero
 */
ActionType BeaconManagerClass::Pick_Beacon_Placement_Action()
{
    const bool altdown = Key_Down(Options.KeyForceMove1) || Key_Down(Options.KeyForceMove2);
    const bool ctrldown = Key_Down(Options.KeyForceAttack1) || Key_Down(Options.KeyForceAttack2);
    const bool shiftdown = Key_Down(Options.KeySelect1) || Key_Down(Options.KeySelect2);

    ExtActionType action = EXT_ACTION_PLACE_BEACON;

    if (!ctrldown && !altdown && !shiftdown) {
        action = EXT_ACTION_PLACE_BEACON; // none
    } else if (shiftdown && !ctrldown && !altdown) {
        action = EXT_ACTION_PLACE_BEACON_1; // shift
    } else if (ctrldown && !altdown && !shiftdown) {
        action = EXT_ACTION_PLACE_BEACON_2; // ctrl
    } else if (altdown && !ctrldown && !shiftdown) {
        action = EXT_ACTION_PLACE_BEACON_3; // alt
    } else if (ctrldown && shiftdown && !altdown) {
        action = EXT_ACTION_PLACE_BEACON_4; // ctrl + shift
    } else if (altdown && shiftdown && !ctrldown) {
        action = EXT_ACTION_PLACE_BEACON_5; // alt + shift
    } else if (ctrldown && altdown && !shiftdown) {
        action = EXT_ACTION_PLACE_BEACON_6; // ctrl + alt
    } else if (ctrldown && altdown && shiftdown) {
        action = EXT_ACTION_PLACE_BEACON_7; // ctrl + alt + shift
    }

    return static_cast<ActionType>(action);
}



/**
 *  Checks if an action type corresponds to a beacon placement.
 *
 *  @authors: ZivDero
 */
bool BeaconManagerClass::Is_Beacon_Placement_Action(ActionType action)
{
    switch (action) {
    case EXT_ACTION_PLACE_BEACON:
    case EXT_ACTION_PLACE_BEACON_1:
    case EXT_ACTION_PLACE_BEACON_2:
    case EXT_ACTION_PLACE_BEACON_3:
    case EXT_ACTION_PLACE_BEACON_4:
    case EXT_ACTION_PLACE_BEACON_5:
    case EXT_ACTION_PLACE_BEACON_6:
    case EXT_ACTION_PLACE_BEACON_7:
        return true;
    default:
        return false;
    }
}


/**
 *  Returns a preset text label for a beacon placement action.
 *
 *  @authors: ZivDero
 */
char const* BeaconManagerClass::Beacon_Text(ActionType action)
{
    if (action >= EXT_ACTION_PLACE_BEACON_1 && action <= EXT_ACTION_PLACE_BEACON_7) {
        if (!UIControls->BeaconText[action - EXT_ACTION_PLACE_BEACON_1].empty()) {
            return UIControls->BeaconText[action - EXT_ACTION_PLACE_BEACON_1].c_str();
        }
    }

    return nullptr;
}

/**
 *  Returns a preset preview text label for a beacon placement action.
 *
 *  @authors: ZivDero
 */
char const* BeaconManagerClass::Beacon_Preview_Text(ActionType action)
{
    if (action >= EXT_ACTION_PLACE_BEACON_1 && action <= EXT_ACTION_PLACE_BEACON_7) {
        if (!UIControls->BeaconPreviewText[action - EXT_ACTION_PLACE_BEACON_1].empty()) {
            return UIControls->BeaconPreviewText[action - EXT_ACTION_PLACE_BEACON_1].c_str();
        }
    }

    return nullptr;
}
