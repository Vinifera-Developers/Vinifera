/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BEACON.H
 *
 *  @author        ZivDero, tomsons26
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
#pragma once

#include "coord.h"
#include "point.h"
#include "rect.h"

#include <map>
#include <string>

class ShapeSet;
class Surface;
enum HousesType;
enum KeyNumType;

/**
 *  An individual beacon is represented by this class.
 */
class BeaconClass
{
public:
    BeaconClass();
    ~BeaconClass() = default;
    
    /**
     *  Rendering
     */
    void Draw(Surface* surface, Rect const& cliprect) const;
    void Draw_On_Radar(Surface* surface, Rect const& cliprect, bool removed) const;
    bool Is_Visible_To_Player() const;
    int Get_Shape_Frame() const;
    
    /**
     *  I/O
     */
    void Set(Coord const& coord, HousesType house);
    void Set_Text(char const* text);
    void Select(bool selected);
    void Hide();

public:
    int ID;
    Coord Position;
    bool IsVisible;
    bool IsSelected;
    std::string Text;
    HousesType House;
};


/**
 *  This class manages all the beacons currently in existence (and owns them).
 *  There is a list of beacons per player.
 */
class BeaconManagerClass
{
public:
    BeaconManagerClass();
    ~BeaconManagerClass() = default;

    /**
     *  Initialization & teardown
     */
    void Reset();
    void Load_Art();
    static bool Are_Beacons_Enabled();

    /**
     *  Rendering
     */
    void Draw(Surface* surface, Rect const& cliprect) const;
    void Draw_On_Radar(Surface* surface, Rect const& cliprect) const;
    bool Is_To_Redraw_Radar() const;
    int Get_Radar_Shape_Frame() const;

    /**
     *  Beacon lifecycle
     */
    void Place_Beacon(HousesType house, Coord const& coord, int beacon_id = -1, char const* text = nullptr);
    void Delete_Beacon(HousesType house, int beacon_id);
    void Delete_Owned_Beacons(HousesType house);
    void Set_Beacon_Text(char const* text, HousesType house, int beacon_id, bool send = false);

    /**
     *  I/O.
     */
    bool Select_Beacon(Coord const& coord) const;
    void Unselect_All_Beacons();
    BeaconClass* Beacon_At(Coord const& coord) const;
    BeaconClass* Find_Selected_Beacon(HousesType house) const;
    void Input(KeyNumType input, bool finalize = false);
    
    /**
     *  Networking
     */
    static void Send_Beacon_Place(Coord const& coord, HousesType house, int beacon_id);
    static void Send_Beacon_Delete(HousesType house, int beacon_id);
    static void Send_Set_Beacon_Text(char const* text, HousesType house, int beacon_id);

    /**
     *  Action helpers
     */
    static ActionType Pick_Beacon_Placement_Action();
    static bool Is_Beacon_Placement_Action(ActionType action);
    static char const* Beacon_Text(ActionType action);
    static char const* Beacon_Preview_Text(ActionType action);

public:
    /**
     *  Per-player beacon lists (sorted by creation frame).
     */
    std::map<int, std::unique_ptr<BeaconClass>> Beacons[MAX_PLAYERS];

    /**
     *  Dimensions of the beacon graphics.
     */
    int BeaconWidth;
    int BeaconHeight;
    int BeaconFrameCount;
    int RadarBeaconWidth;
    int RadarBeaconHeight;
    int RadarBeaconFrameCount;

    /**
     *  The radar beacon isn't constantly visible - it "blinks".
     *  This is how many frames the cycle is.
     */
    int RadarBeaconAnimPeriod;

public:
    /**
     *  Beacon graphics.
     */
    static ShapeSet const* BeaconArt;
    static ShapeSet const* RadarBeaconArt;
};

extern BeaconManagerClass BeaconManager;
