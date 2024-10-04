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

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual const char *Name() const override { return "Scenario"; }
        virtual const char *Full_Name() const override { return "Scenario"; }

        void Init_Clear();
        bool Read_INI(CCINIClass &ini);

        bool Read_Tutorial_INI(CCINIClass &ini, bool log = false);

        Cell Waypoint_Cell(WAYPOINT wp) const;
        CellClass * Waypoint_CellClass(WAYPOINT wp) const;
        Coord Waypoint_Coord(WAYPOINT wp) const;

        void Set_Waypoint_Cell(WAYPOINT wp, Cell &cell);
        void Set_Waypoint_Coord(WAYPOINT wp, Coord &coord);

        bool Is_Waypoint_Valid(WAYPOINT wp) const;
        void Clear_Waypoint(WAYPOINT wp);

        void Clear_All_Waypoints();

        void Read_Waypoint_INI(CCINIClass &ini);
        void Write_Waypoint_INI(CCINIClass &ini);

        const char *Waypoint_As_String(WAYPOINT wp) const;

        bool Read_Global_INI(INIClass& ini);
        bool Read_Local_INI(INIClass& ini);
        bool Write_Local_INI(INIClass& ini);

        int Set_Global_To(int global, int value);
        int Set_Global_To(char const* name, int value);
        bool Get_Global_Value(int global, int& value);
        bool Get_Global_Value(char const* name, int& value);
        int Set_Local_To(int local, int value);
        int Set_Local_To(char const* name, int value);
        bool Get_Local_Value(int local, int& value);
        bool Get_Local_Value(char const* name, int& value);

        int Find_Global_Variable_Index(char const* name);
        int Find_Local_Variable_Index(char const* name);

        int Find_Free_Local() const;
        int Num_Locals() const;

        static std::string Substitute_Variable_Placeholders(std::string input);

        static bool Start_Scenario(char* name, bool briefing, CampaignType campaignid);
        static bool Read_Scenario_INI(const char* root, bool);
        static bool Load_Scenario(CCINIClass& ini, bool random = false);
        static void Init_Forced_Alliances();

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

        struct ScenarioFlagExtType {
            char VariableName[40];
            int Value;
        };
        ScenarioFlagExtType GlobalFlags[500];
        ScenarioFlagExtType LocalFlags[500];

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

int Vinifera_Scan_Place_Object(ObjectClass* obj, Cell cell, int min_dist, int max_dist, bool no_scatter);
