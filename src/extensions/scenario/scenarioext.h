/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended ScenarioClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


#include "extension.h"
#include "scenario.h"
#include "uicontrol.h"
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
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual const char *Name() const override { return "Scenario"; }
        virtual const char *Full_Name() const override { return "Scenario"; }

        void Init_Clear();
        bool Read_INI(CCINIClass &ini);

        static bool Read_Tutorial_INI(CCINIClass const& ini);

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

        void Dump_Globals() const;

        static std::string Substitute_Variable_Placeholders(std::string input);

        static bool Start_Scenario(char* name, bool briefing, CampaignType campaignid);
        static bool Read_Scenario_INI(CCINIClass& ini, bool random = false);
        static void Init_Forced_Alliances();

        void Assign_Starting_Positions(bool official);
        static bool Assign_Random_Starting_Position(HouseClass* house);
        static void Assign_Houses();
        static void Create_Units(bool official);
        bool Read_Loading_Screen_INI(const char* filename);

        UIControlsClass::LoadingScreen const* Pick_Loading_Screen_Override(HousesType house) const;

    public:
        /**
         *  This is an array of waypoints; each waypoint corresponds to a letter of
         *  the alphabet, and points to a cell position.
         * 
         *  The CellClass has a bit that tells if that cell has a waypoint attached to
         *  it; the only way to find which waypoint it is, is to scan this array. This
         *  shouldn't be needed often; usually, you know the waypoint & you want the "Cell".
         */
        Cell Waypoint[NEW_WAYPOINT_COUNT];

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
         *  Should the AI use base nodes outside of campaign, instead of skirmish AI base building logic.
         */
        bool IsUseMPAIBaseNodes;

        /**
         *  Convenient property to access IsGDI as a HousesType.
         */
        HousesType Get_House() const { return static_cast<HousesType>(reinterpret_cast<unsigned char&>(This()->IsGDI)); }
        void Set_House(HousesType house) { reinterpret_cast<unsigned char&>(This()->IsGDI) = house; }
        __declspec(property(get = Get_House, put = Set_House)) HousesType House;
};

