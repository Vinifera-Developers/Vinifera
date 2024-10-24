/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_SAVEVER.H
 *
 *  @authors       tomsons26, ZivDero
 *
 *  @brief         Vinifera replacement of the save file header.
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

#include <objidl.h>
#include <minwindef.h>

class ViniferaSaveVersionInfo
{
public:
    enum {
        ID_SCENARIO_DESCRIPTION = 2,
        ID_PLAYER_HOUSE = 3,
        ID_VERSION = 9,
        ID_INTERNAL_VERSION = 16,
        ID_START_TIME = 12,
        ID_LAST_SAVE_TIME = 13,
        ID_PLAY_TIME = 10,
        ID_EXECUTABLE_NAME = 18,
        ID_PLAYER_NAME = 4,
        ID_PLAYER_NAME2 = 8,
        ID_SCENARIO_NUMBER = 100,
        ID_CAMPAIGN = 101,
        ID_GAMETYPE = 102,

        ID_VINIFERA_VERSION = 105,
        ID_VINIFERA_COMMIT_HASH = 106,
        ID_SESSION_ID = 107,
    };

public:
    ViniferaSaveVersionInfo();

    void Set_Version(int num);
    int Get_Version() const;

    void Set_Internal_Version(int num);
    int Get_Internal_Version() const;

    void Set_Scenario_Description(const char * desc);
    const char * Get_Scenario_Description() const;

    void Set_Player_House(const char * name);
    const char * Get_Player_House() const;

    void Set_Campaign_Number(int num);
    int Get_Campaign_Number() const;

    void Set_Scenario_Number(int num);
    int Get_Scenario_Number() const;

    void Set_Unknown_String(const char * name);
    const char * Get_Unknown_String() const;

    void Set_Player_Name(const char * name);
    const char * Get_Player_Name() const;

    void Set_Executable_Name(const char * name);
    const char * Get_Executable_Name() const;

    void Set_Start_Time(FILETIME &time);
    FILETIME Get_Start_Time() const;

    void Set_Play_Time(FILETIME &time);
    FILETIME Get_Play_Time() const;

    void Set_Last_Time(FILETIME &time);
    FILETIME Get_Last_Time() const;

    void Set_Game_Type(int id);
    int Get_Game_Type() const;

    void Set_Vinifera_Version(int num);
    int Get_Vinifera_Version() const;

    void Set_Vinifera_Commit_Hash(const char* hash);
    const char* Get_Vinifera_Commit_Hash() const;

    void Set_Session_ID(int num);
    int Get_Session_ID() const;

    HRESULT Save(IStorage *storage);
    HRESULT Load(IStorage *storage);

private:
    HRESULT Load_String(IStorage *storage, int id, char *string);
    HRESULT Load_String_Set(IPropertySetStorage *storageset, int id, char *string);

    HRESULT Load_Int(IStorage *storage, int id, int *integer);
    HRESULT Load_Int_Set(IPropertySetStorage *storageset, int id, int *integer);

    HRESULT Save_String(IStorage *storage, int id, char *string);
    HRESULT Save_String_Set(IPropertySetStorage *storageset, int id, const char *string);

    HRESULT Save_Int(IStorage *storage, int id, int integer);
    HRESULT Save_Int_Set(IPropertySetStorage *storageset, int id, int integer);

    HRESULT Load_Time(IStorage *storage, int id, FILETIME *time);
    HRESULT Load_Time_Set(IPropertySetStorage *storageset, int id, FILETIME *time);

    HRESULT Save_Time(IStorage *storage, int id, FILETIME *time);
    HRESULT Save_Time_Set(IPropertySetStorage *storageset, int id, FILETIME *time);

private:
    int InternalVersion;
    int Version;
    char ScenarioDescription[128];
    char PlayerHouse[64];
    int CampaignNumber;
    int ScenarioNumber;
    char UnknownString[260];
    char PlayerName[64];
    char ExecutableName[260];
    FILETIME StartTime;
    FILETIME PlayTime;
    FILETIME LastSaveTime;
    int GameType;

    /**
     *  New Vinifera fields.
     */
    int ViniferaVersion;
    char ViniferaCommitHash[40];
    int SessionID;
};

const WCHAR* Vinifera_Stream_Name_From_ID(int id);
bool Vinifera_Get_Savefile_Info(char const* name, ViniferaSaveVersionInfo& info);
