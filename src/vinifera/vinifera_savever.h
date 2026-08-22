/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Vinifera replacement of the save file header.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "tibsun_defines.h"

#include <minwindef.h>
#include <objidl.h>
#include <string>
#include <string_view>


class ViniferaSaveVersionInfo
{
public:
    void Set_Internal_Version(int num) { InternalVersion = num; }
    int Get_Internal_Version() const { return InternalVersion; }

    void Set_Scenario_Description(std::string_view desc) { ScenarioDescription.assign(desc); }
    const std::string& Get_Scenario_Description() const { return ScenarioDescription; }

    void Set_Player_House(std::string_view name) { PlayerHouse.assign(name); }
    const std::string& Get_Player_House() const { return PlayerHouse; }

    void Set_Campaign_Number(int num) { CampaignNumber = num; }
    int Get_Campaign_Number() const { return CampaignNumber; }

    void Set_Scenario_Number(int num) { ScenarioNumber = num; }
    int Get_Scenario_Number() const { return ScenarioNumber; }

    void Set_Game_Type(int type) { GameType = type; }
    int Get_Game_Type() const { return GameType; }

    void Set_Vinifera_Version(int num) { ViniferaVersion = num; }
    int Get_Vinifera_Version() const { return ViniferaVersion; }

    void Set_Vinifera_Commit_Hash(std::string_view hash) { ViniferaCommitHash.assign(hash); }
    const std::string& Get_Vinifera_Commit_Hash() const { return ViniferaCommitHash; }

    void Set_Playthrough_ID(int num) { PlaythroughID = num; }
    int Get_Playthrough_ID() const { return PlaythroughID; }

    void Set_Difficulty(int num) { Difficulty = num; }
    int Get_Difficulty() const { return Difficulty; }

    void Set_Elapsed_Time(int time) { ElapsedTime = time; }
    int Get_Elapsed_Time() const { return ElapsedTime; }

    HRESULT Save(IStorage* storage) const;
    HRESULT Load(IStorage* storage);

private:
    static HRESULT Load_String(IStorage* storage, const WCHAR* name, std::string& string);
    static HRESULT Save_String(IStorage* storage, const WCHAR* name, const std::string& string);

    static HRESULT Load_Int(IStorage* storage, const WCHAR* name, int& integer);
    static HRESULT Save_Int(IStorage* storage, const WCHAR* name, int integer);

    static HRESULT Load_Bool(IStorage* storage, const WCHAR* name, bool& boolean);
    static HRESULT Save_Bool(IStorage* storage, const WCHAR* name, bool boolean);

    static HRESULT Load_Time(IStorage* storage, const WCHAR* name, FILETIME& time);
    static HRESULT Save_Time(IStorage* storage, const WCHAR* name, const FILETIME& time);

private:
    int InternalVersion = 0;
    std::string ScenarioDescription;
    std::string PlayerHouse;
    int CampaignNumber = -1;
    int ScenarioNumber = 0;
    int GameType = 0;

    /**
     *  New Vinifera fields.
     */
    int ViniferaVersion = 0;
    std::string ViniferaCommitHash;
    int PlaythroughID = 0;
    int Difficulty = DIFF_NORMAL;
    int ElapsedTime = 0;
};

bool Vinifera_Get_Savefile_Info(std::string_view name, ViniferaSaveVersionInfo& info);
