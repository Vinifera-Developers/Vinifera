/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SCENARIOEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended ScenarioClass class.
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

#include "always.h"
#include "extension.h"
#include "scenario.h"
#include "wstring.h"


class ScenarioClassExtension final : public GlobalExtensionClass<ScenarioClass>
{
public:
    IFACEMETHOD(Load)(IStream *pStm);
    IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

public:
    ScenarioClassExtension(const ScenarioClass *this_ptr);
    ScenarioClassExtension(const NoInitClass &noinit);
    virtual ~ScenarioClassExtension();

    virtual int Size_Of() const override;
    virtual void Detach(TARGET target, bool all = true) override;
    virtual void Compute_CRC(WWCRCEngine &crc) const override;

    virtual const char *Name() const override { return "Scenario"; }
    virtual const char *Full_Name() const override { return "Scenario"; }

    void Init_Clear();
    bool Read_INI(CCINIClass &ini);

    bool Read_Tutorial_INI(CCINIClass &ini, bool log = false);

    Cell Get_Waypoint_Cell(WaypointType wp) const;
    CellClass * Get_Waypoint_CellPtr(WaypointType wp) const;
    Coordinate Get_Waypoint_Coord(WaypointType wp) const;
    Coordinate Get_Waypoint_Coord_Height(WaypointType wp) const;

    void Set_Waypoint_Cell(WaypointType wp, Cell &cell);
    void Set_Waypoint_Coord(WaypointType wp, Coordinate &coord);

    bool Is_Valid_Waypoint(WaypointType wp) const;
    void Clear_Waypoint(WaypointType wp);

    void Clear_All_Waypoints();

    void Read_Waypoint_INI(CCINIClass &ini);
    void Write_Waypoint_INI(CCINIClass &ini);

    const char *Waypoint_As_String(WaypointType wp) const;

    static bool Start_Scenario(char* name, bool briefing, CampaignType campaignid);
    static bool Read_Scenario_INI(const char* root, bool);
    static bool Load_Scenario(CCINIClass& ini, bool random = false);

    void Assign_Starting_Positions(bool official);
    static void Assign_Houses();
    static void Create_Units(bool official);
    bool Read_Loading_Screen_INI(const char* filename);

public:
    /**
     *  This is a vector of waypoints; each waypoint corresponds to a letter of
     *  the alphabet, and points to a cell position.
     * 
     *  The CellClass has a bit that tells if that cell has a waypoint attached to
     *  it; the only way to find which waypoint it is, is to scan this array. This
     *  shouldn't be needed often; usually, you know the waypoint & you want the "Cell".
     */
    VectorClass<Cell> Waypoint;

    /**
     *  Can ice get destroyed when hit by certain weapons?
     */
    bool IsIceDestruction;

    RGBStruct ScorePlayerColor;
    RGBStruct ScoreEnemyColor;

    /**
     *  The starting positions of the houses.
     *  StartingPositions[HousesType] = 0 .. 7
     */
    int StartingPositions[MAX_PLAYERS];
    Cell StartingPositionCells[MAX_PLAYERS];

    /**
     *  The side to use for the sidebar assets (singleplayer only).
     */
    SideType SidebarSide;

    /**
     *  Scenarios can override the loading screen with a custom variant, these
     *  define the filename to load and position overrides.
     */
    struct LoadingScreenData {
        Wstring Filename;
        TPoint2D<int> Position;
    };

    LoadingScreenData LoadingScreens[3];

    /**
     *  Should the AI use base nodes outside of campaign, instead of skirmish AI base building logic.
     */
    bool IsUseMPAIBaseNodes;
};
