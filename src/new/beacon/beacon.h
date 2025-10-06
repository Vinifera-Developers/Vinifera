/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BEACON.H
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
#pragma once

#include "coord.h"
#include "point.h"
#include "rect.h"

#include <string>
#include <vector>

class ShapeSet;
class Surface;
enum HousesType;

class BeaconClass
{
public:
    BeaconClass();

    void Draw(Surface* surface, Rect cliprect) const;
    void Set(Coord coord, HousesType owner);
    void Select(bool selected);
    void Disown();
    void Set_Text(char const* text);
    void Draw_On_Radar(Surface* surface, Rect cliprect, bool is_doer);
    bool Is_Visible_To_Player() const;

    Coord Position;
    bool HasOwner;
    bool IsSelected;
    std::string Text;
    HousesType Owner;
};

class BeaconManagerClass
{
    enum {
        BEACONS_PER_PLAYER = 3
    };

public:
    BeaconManagerClass();
    ~BeaconManagerClass();

    void Reset();
    void Load_Art();
    void Draw(Surface* surface, Rect cliprect);
    void Place_Beacon(HousesType house, Coord coord, int beacon_id = -1);
    bool Select_Beacon(Coord coord);
    void Unselect_All_Beacons();
    BeaconClass* Beacon_At(Coord coord);
    BeaconClass* Find_Selected_Beacon(HousesType house);
    void Delete_Beacon(HousesType house, int beacon_id);
    void Delete_Owned_Beacons(HousesType house);
    void Set_Beacon_Text(char const* text, HousesType house, int beacon_id, bool is_doer = false);
    void Draw_On_Radar(Surface* surface, Rect cliprect);
    bool Is_To_Redraw_Radar();

    static void Send_Beacon_Place(Coord coord, HousesType house, int beacon_id);
    static void Send_Beacon_Delete(HousesType house, int beacon_id);
    static void Send_Set_Beacon_Text(char const* text, HousesType house, int beacon_id);

    std::vector<std::unique_ptr<BeaconClass>> Beacons[MAX_PLAYERS];
    int BeaconWidth;
    int BeaconHeight;
    int BeaconFrameCount;
    int RadarBeaconWidth;
    int RadarBeaconHeight;
    int RadarBeaconFrameCount;
    int RadarBeaconAnimPeriod;

public:
    static ShapeSet const* BeaconArt;
    static ShapeSet const* RadarBeaconArt;
};

extern BeaconManagerClass BeaconManager;