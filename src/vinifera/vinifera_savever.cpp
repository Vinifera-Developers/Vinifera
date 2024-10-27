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

#include "vinifera_savever.h"

#include <comdef.h>

#include "savever.h"
#include "debughandler.h"


/**
 *  Basic constructor for ViniferaSaveVersionInfo.
 *
 *  @author: tomsons26
 */
ViniferaSaveVersionInfo::ViniferaSaveVersionInfo() :
    InternalVersion(0),
    Version(0),
    ScenarioDescription{ "" },
    PlayerHouse{ "" },
    CampaignNumber(-1),
    ScenarioNumber(0),
    UnknownString{ "" },
    PlayerName{ "" },
    ExecutableName{ "" },
    GameType(0),
    ViniferaVersion(0),
    ViniferaCommitHash{ "" },
    PlaythroughID(0)
{
    StartTime.dwLowDateTime = 0;
    StartTime.dwHighDateTime = 0;

    PlayTime.dwLowDateTime = 0;
    PlayTime.dwHighDateTime = 0;

    LastSaveTime.dwLowDateTime = 0;
    LastSaveTime.dwHighDateTime = 0;
}


/**
 *  Sets the save version field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Version(int num)
{
    Version = num;
}


/**
 *  Gets the save version field.
 *
 *  @author: tomsons26
 */
int ViniferaSaveVersionInfo::Get_Version() const
{
    return Version;
}


/**
 *  Sets the game version field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Internal_Version(int num)
{
    InternalVersion = num;
}


/**
 *  Gets the game version field.
 *
 *  @author: tomsons26
 */
int ViniferaSaveVersionInfo::Get_Internal_Version() const
{
    return InternalVersion;
}


/**
 *  Sets the scenario description field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Scenario_Description(const char * desc)
{
    ScenarioDescription[sizeof(ScenarioDescription) - 1] = 0;
    strncpy(ScenarioDescription, desc, sizeof(ScenarioDescription) - 1);
}


/**
 *  Gets the scenario description field.
 *
 *  @author: tomsons26
 */
const char * ViniferaSaveVersionInfo::Get_Scenario_Description() const
{
    return ScenarioDescription;
}


/**
 *  Sets the player house name field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Player_House(const char * name)
{
    PlayerHouse[sizeof(PlayerHouse) - 1] = 0;
    strncpy(PlayerHouse, name, sizeof(PlayerHouse) - 1);
}


/**
 *  Gets the player house name field.
 *
 *  @author: tomsons26
 */
const char * ViniferaSaveVersionInfo::Get_Player_House() const
{
    return PlayerHouse;
}


/**
 *  Sets the campaign number field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Campaign_Number(int num)
{
    CampaignNumber = num;
}


/**
 *  Gets the campaign number field.
 *
 *  @author: tomsons26
 */
int ViniferaSaveVersionInfo::Get_Campaign_Number() const
{
    return CampaignNumber;
}


/**
 *  Sets the scenario number field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Scenario_Number(int num)
{
    ScenarioNumber = num;
}


/**
 *  Gets the scenario number field.
 *
 *  @author: tomsons26
 */
int ViniferaSaveVersionInfo::Get_Scenario_Number() const
{
    return ScenarioNumber;
}


/**
 *  Sets the unknown field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Unknown_String(const char * str)
{
    UnknownString[sizeof(UnknownString) - 1] = 0;
    strncpy(UnknownString, str, sizeof(UnknownString) - 1);
}


/**
 *  Gets the unknown field.
 *
 *  @author: tomsons26
 */
const char * ViniferaSaveVersionInfo::Get_Unknown_String() const
{
    return UnknownString;
}


/**
 *  Sets the player name field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Player_Name(const char * name)
{
    PlayerName[sizeof(PlayerName) - 1] = 0;
    strncpy(PlayerName, name, sizeof(PlayerName) - 1);
}


/**
 *  Gets the player name field.
 *
 *  @author: tomsons26
 */
const char * ViniferaSaveVersionInfo::Get_Player_Name() const
{
    return PlayerName;
}


/**
 *  Sets the executable name field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Executable_Name(const char * name)
{
    ExecutableName[sizeof(ExecutableName) - 1] = 0;
    strncpy(ExecutableName, name, sizeof(ExecutableName) - 1);
}


/**
 *  Gets the executable name field.
 *
 *  @author: tomsons26
 */
const char * ViniferaSaveVersionInfo::Get_Executable_Name() const
{
    return ExecutableName;
}


/**
 *  Sets the start time field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Start_Time(FILETIME &time)
{
    StartTime = time;
}


/**
 *  Gets the start time field.
 *
 *  @author: tomsons26
 */
FILETIME ViniferaSaveVersionInfo::Get_Start_Time() const
{
    return StartTime;
}


/**
 *  Sets the play time field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Play_Time(FILETIME &time)
{
    PlayTime = time;
}


/**
 *  Gets the play time field.
 *
 *  @author: tomsons26
 */
FILETIME ViniferaSaveVersionInfo::Get_Play_Time() const
{
    return PlayTime;
}


/**
 *  Sets the last save time field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Last_Time(FILETIME &time)
{
    LastSaveTime = time;
}


/**
 *  Gets the last save time field.
 *
 *  @author: tomsons26
 */
FILETIME ViniferaSaveVersionInfo::Get_Last_Time() const
{
    return LastSaveTime;
}


/**
 *  Sets the session type field.
 *
 *  @author: tomsons26
 */
void ViniferaSaveVersionInfo::Set_Game_Type(int type)
{
    GameType = type;
}


/**
 *  Gets the session type field.
 *
 *  @author: tomsons26
 */
int ViniferaSaveVersionInfo::Get_Game_Type() const
{
    return GameType;
}


/**
 *  Sets the Vinifera version field.
 *
 *  @author: ZivDero
 */
void ViniferaSaveVersionInfo::Set_Vinifera_Version(int num)
{
    ViniferaVersion = num;
}


/**
 *  Gets the Vinifera version field.
 *
 *  @author: ZivDero
 */
int ViniferaSaveVersionInfo::Get_Vinifera_Version() const
{
    return ViniferaVersion;
}


/**
 *  Sets the Vinifera commit hash field.
 *
 *  @author: ZivDero
 */
void ViniferaSaveVersionInfo::Set_Vinifera_Commit_Hash(const char* hash)
{
    ViniferaCommitHash[sizeof(ViniferaCommitHash) - 1] = 0;
    strncpy(ViniferaCommitHash, hash, sizeof(ViniferaCommitHash) - 1);
}


/**
 *  Gets the Vinifera commit hash field.
 *
 *  @author: ZivDero
 */
const char* ViniferaSaveVersionInfo::Get_Vinifera_Commit_Hash() const
{
    return ViniferaCommitHash;
}


/**
 *  Sets the unique sesssion ID field.
 *
 *  @author: ZivDero
 */
void ViniferaSaveVersionInfo::Set_Playthrough_ID(int num)
{
    PlaythroughID = num;
}


/**
 *  Gets the unique sesssion ID field.
 *
 *  @author: ZivDero
 */
int ViniferaSaveVersionInfo::Get_Playthrough_ID() const
{
    return PlaythroughID;
}


/**
 *  Sets the difficulty field.
 *
 *  @author: ZivDero
 */
void ViniferaSaveVersionInfo::Set_Difficulty(int num)
{
    Difficulty = num;
}


/**
 *  Gets the difficulty field.
 *
 *  @author: ZivDero
 */
int ViniferaSaveVersionInfo::Get_Difficulty() const
{
    return Difficulty;
}


/**
 *  Sets the total play time field.
 *
 *  @author: ZivDero
 */
void ViniferaSaveVersionInfo::Set_Total_Play_Time(int num)
{
    TotalPlayTime = num;
}


/**
 *  Gets the total play time field.
 *
 *  @author: ZivDero
 */
int ViniferaSaveVersionInfo::Get_Total_Play_Time() const
{
    return TotalPlayTime;
}


/**
 *  Saves the version information to the storage.
 *
 *  @author: tomsons26, ZivDero
 */
HRESULT ViniferaSaveVersionInfo::Save(IStorage *storage)
{
    if (storage == nullptr) {
        return E_POINTER;
    }

    DEBUG_INFO("Attempting to obtain PropertySetStorage interface\n");

    IPropertySetStoragePtr storageset;
    HRESULT res = storage->QueryInterface(IID_IPropertySetStorage, (void **)&storageset);

    if (SUCCEEDED(res)) {
    
        DEBUG_INFO("Saving version information the new way.\n");

        res = Save_String_Set(storageset, ID_SCENARIO_DESCRIPTION, ScenarioDescription);
        if (FAILED(res)) {
            return res;
        }

        res = Save_String_Set(storageset, ID_PLAYER_HOUSE, PlayerHouse);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Int_Set(storageset, ID_VERSION, Version);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Int_Set(storageset, ID_INTERNAL_VERSION, InternalVersion);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Time_Set(storageset, ID_START_TIME, &StartTime);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Time_Set(storageset, ID_LAST_SAVE_TIME, &LastSaveTime);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Time_Set(storageset, ID_PLAY_TIME, &PlayTime);
        if (FAILED(res)) {
            return res;
        }

        res = Save_String_Set(storageset, ID_EXECUTABLE_NAME, ExecutableName);
        if (FAILED(res)) {
            return res;
        }

        res = Save_String_Set(storageset, ID_PLAYER_NAME, PlayerName);
        if (FAILED(res)) {
            return res;
        }

        res = Save_String_Set(storageset, ID_PLAYER_NAME2, PlayerName);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Int_Set(storageset, ID_SCENARIO_NUMBER, ScenarioNumber);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Int_Set(storageset, ID_CAMPAIGN, CampaignNumber);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Int_Set(storageset, ID_GAMETYPE, GameType);
        if (FAILED(res)) {
            return res;
        }

        /**
         *  New Vinifera fields.
         */
        res = Save_Int_Set(storageset, ID_VINIFERA_VERSION, ViniferaVersion);
        if (FAILED(res)) {
            return res;
        }

        res = Save_String_Set(storageset, ID_VINIFERA_COMMIT_HASH, ViniferaCommitHash);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Int_Set(storageset, ID_PLAYTHROUGH_ID, PlaythroughID);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Int_Set(storageset, ID_DIFFICULTY, Difficulty);
        if (FAILED(res)) {
            return res;
        }

        res = Save_Int_Set(storageset, ID_TOTAL_PLAY_TIME, TotalPlayTime);
        if (FAILED(res)) {
            return res;
        }

        //return S_OK;
    }
    else {
        DEBUG_INFO("Failed to save the new way!\n");
    }

    DEBUG_INFO("Saving version information the old way.\n");

    res = Save_String(storage, ID_SCENARIO_DESCRIPTION, ScenarioDescription);
    if (FAILED(res)) {
        return res;
    }

    res = Save_String(storage, ID_PLAYER_HOUSE, PlayerHouse);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, ID_VERSION, Version);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, ID_INTERNAL_VERSION, InternalVersion);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Time(storage, ID_START_TIME, &StartTime);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Time(storage, ID_LAST_SAVE_TIME, &LastSaveTime);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Time(storage, ID_PLAY_TIME, &PlayTime);
    if (FAILED(res)) {
        return res;
    }

    res = Save_String(storage, ID_EXECUTABLE_NAME, ExecutableName);
    if (FAILED(res)) {
        return res;
    }

    res = Save_String(storage, ID_PLAYER_NAME, PlayerName);
    if (FAILED(res)) {
        return res;
    }

    res = Save_String(storage, ID_PLAYER_NAME2, PlayerName);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, ID_SCENARIO_NUMBER, ScenarioNumber);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, ID_CAMPAIGN, CampaignNumber);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, ID_GAMETYPE, GameType);
    if (FAILED(res)) {
        return res;
    }

    /**
     *  New Vinifera fields.
     */
    res = Save_Int(storage, ID_VINIFERA_VERSION, ViniferaVersion);
    if (FAILED(res)) {
        return res;
    }

    res = Save_String(storage, ID_VINIFERA_COMMIT_HASH, ViniferaCommitHash);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, ID_PLAYTHROUGH_ID, PlaythroughID);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, ID_DIFFICULTY, Difficulty);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, ID_TOTAL_PLAY_TIME, TotalPlayTime);
    if (FAILED(res)) {
        return res;
    }

    return S_OK;
}


/**
 *  Loads the version information from the storage.
 *
 *  @author: tomsons26, ZivDero
 */
HRESULT ViniferaSaveVersionInfo::Load(IStorage *storage)
{
    char buffer[256];

    if (storage == nullptr) {
        return E_POINTER;
    }

    DEBUG_INFO("Attempting to obtain PropertySetStorage interface\n");

    IPropertySetStoragePtr storageset;
    HRESULT res = storage->QueryInterface(IID_IPropertySetStorage, (void**)&storageset);

    if (SUCCEEDED(res)) {
    
        DEBUG_INFO("Loading version information.\n");

        res = Load_String_Set(storageset, ID_SCENARIO_DESCRIPTION, buffer);
        if (FAILED(res)) {
            return res;
        }

        strcpy(ScenarioDescription, buffer);

        res = Load_String_Set(storageset, ID_PLAYER_HOUSE, buffer);
        if (FAILED(res)) {
            return res;
        }

        strcpy(PlayerHouse, buffer);

        res = Load_Int_Set(storageset, ID_VERSION, &Version);
        if (FAILED(res)) {
            return res;
        }

        res = Load_Int_Set(storageset, ID_INTERNAL_VERSION, &InternalVersion);
        if (FAILED(res)) {
            return res;
        }

        res = Load_Time_Set(storageset, ID_START_TIME, &StartTime);
        if (FAILED(res)) {
            return res;
        }

        res = Load_Time_Set(storageset, ID_LAST_SAVE_TIME, &LastSaveTime);
        if (FAILED(res)) {
            return res;
        }

        res = Load_Time_Set(storageset, ID_PLAY_TIME, &PlayTime);
        if (FAILED(res)) {
            return res;
        }

        res = Load_String_Set(storageset, ID_EXECUTABLE_NAME, buffer);
        if (FAILED(res)) {
            return res;
        }

        strcpy(ExecutableName, buffer);

        res = Load_String_Set(storageset, ID_PLAYER_NAME, buffer);
        if (FAILED(res)) {
            return res;
        }
    
        strcpy(PlayerName, buffer);

        res = Load_Int_Set(storageset, ID_SCENARIO_NUMBER, &ScenarioNumber);
        if (FAILED(res)) {
            return res;
        }

        res = Load_Int_Set(storageset, ID_CAMPAIGN, &CampaignNumber);
        if (FAILED(res)) {
            return res;
        }

        res = Load_Int_Set(storageset, ID_GAMETYPE, &GameType);
        if (FAILED(res)) {
            return res;
        }

        /**
         *  New Vinifera fields.
         */
        res = Load_Int_Set(storageset, ID_VINIFERA_VERSION, &ViniferaVersion);
        if (FAILED(res)) {
            return res;
        }

        res = Load_String_Set(storageset, ID_VINIFERA_COMMIT_HASH, buffer);
        if (FAILED(res)) {
            return res;
        }

        strcpy(ViniferaCommitHash, buffer);

        res = Load_Int_Set(storageset, ID_PLAYTHROUGH_ID, &PlaythroughID);
        if (FAILED(res)) {
            return res;
        }

        res = Load_Int_Set(storageset, ID_DIFFICULTY, &Difficulty);
        if (FAILED(res)) {
            return res;
        }

        res = Load_Int_Set(storageset, ID_TOTAL_PLAY_TIME, &TotalPlayTime);
        if (FAILED(res)) {
            return res;
        }

        //return S_OK;
    }
    else {
        DEBUG_INFO("Failed to load the new way!\n");
    }

    DEBUG_INFO("Loading version information the old way.\n");

    res = Load_String(storage, ID_SCENARIO_DESCRIPTION, buffer);
    if (FAILED(res)) {
        return res;
    }

    strcpy(ScenarioDescription, buffer);

    res = Load_String(storage, ID_PLAYER_HOUSE, buffer);
    if (FAILED(res)) {
        return res;
    }

    strcpy(PlayerHouse, buffer);

    res = Load_Int(storage, ID_VERSION, &Version);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, ID_INTERNAL_VERSION, &InternalVersion);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Time(storage, ID_START_TIME, &StartTime);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Time(storage, ID_LAST_SAVE_TIME, &LastSaveTime);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Time(storage, ID_PLAY_TIME, &PlayTime);
    if (FAILED(res)) {
        return res;
    }

    res = Load_String(storage, ID_EXECUTABLE_NAME, buffer);
    if (FAILED(res)) {
        return res;
    }

    strcpy(ExecutableName, buffer);

    res = Load_String(storage, ID_PLAYER_NAME, buffer);
    if (FAILED(res)) {
        return res;
    }

    strcpy(PlayerName, buffer);

    res = Load_Int(storage, ID_SCENARIO_NUMBER, &ScenarioNumber);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, ID_CAMPAIGN, &CampaignNumber);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, ID_GAMETYPE, &GameType);
    if (FAILED(res)) {
        return res;
    }

    /**
     *  New Vinifera fields.
     */
    res = Load_Int(storage, ID_VINIFERA_VERSION, &ViniferaVersion);
    if (FAILED(res)) {
        return res;
    }

    res = Load_String(storage, ID_VINIFERA_COMMIT_HASH, buffer);
    if (FAILED(res)) {
        return res;
    }

    strcpy(ViniferaCommitHash, buffer);

    res = Load_Int(storage, ID_PLAYTHROUGH_ID, &PlaythroughID);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, ID_DIFFICULTY, &Difficulty);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, ID_TOTAL_PLAY_TIME, &TotalPlayTime);
    if (FAILED(res)) {
        return res;
    }

    return S_OK;
}


/**
 *  Loads a string from the storage.
 *
 *  @author: tomsons26, ZivDero
 */
HRESULT ViniferaSaveVersionInfo::Load_String(IStorage *storage, int id, char *string)
{
    HRESULT res;
    IStreamPtr stm;

    *string = '\0';

    res = storage->OpenStream(Vinifera_Stream_Name_From_ID(id), nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    int i = 0;
    WCHAR buffer[128];

    do {
        res = stm->Read(&buffer[i], sizeof(buffer[i]), nullptr);
        if (FAILED(res)) {
            return res;
        }

    } while (buffer[i++]);

    WideCharToMultiByte(CP_ACP, 0, buffer, -1, string, std::size(buffer) - 1, nullptr, nullptr);
    return res;
}


/**
 *  Loads a string from the storage set.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Load_String_Set(IPropertySetStorage *storageset, int id, char *string)
{
    HRESULT res;
    IPropertyStoragePtr storage;

    *string = '\x0';

    res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
    if (FAILED(res)) {
        return res;
    }

    PROPSPEC propsec;
    propsec.ulKind = PRSPEC_PROPID;
    propsec.propid = id;
    PROPVARIANT propvar;

    res = storage->ReadMultiple(1, &propsec, &propvar);
    if (FAILED(res)) {
        return res;
    }

    if (propvar.vt == VT_LPWSTR) { 
        WideCharToMultiByte(CP_ACP, 0, propvar.pwszVal, -1, string, 64, nullptr, nullptr);
    }

    return res;
}


/**
 *  Loads an integer from the storage.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Load_Int(IStorage *storage, int id, int *integer)
{
    HRESULT res;
    IStreamPtr stm;

    *integer = 0;

    res = storage->OpenStream(Vinifera_Stream_Name_From_ID(id), nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    res = stm->Read(integer, sizeof(*integer), nullptr);
    if (FAILED(res)) {
        return res;
    }

    return res;
}


/**
 *  Loads an integer from the storage set.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Load_Int_Set(IPropertySetStorage *storageset, int id, int *integer)
{
    HRESULT res;
    IPropertyStoragePtr storage;

    *integer = 0;

    res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
    if (FAILED(res)) {
        return res;
    }

    PROPSPEC propsec;
    propsec.ulKind = PRSPEC_PROPID;
    propsec.propid = id;
    PROPVARIANT propvar;

    res = storage->ReadMultiple(1, &propsec, &propvar);
    if (FAILED(res)) {
        return res;
    }

    if (propvar.vt == VT_I4) {
        *integer = propvar.lVal;
    }

    return res;
}


/**
 *  Saves a string to the storage.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Save_String(IStorage *storage, int id, char *string)
{
    WCHAR buffer[128];

    MultiByteToWideChar(CP_ACP, 0, string, -1, buffer, std::size(buffer) - 1);

    IStreamPtr stm(nullptr);

    HRESULT res = storage->CreateStream(Vinifera_Stream_Name_From_ID(id), STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    res = stm->Write(buffer, sizeof(WCHAR) * wcslen(buffer) + 2, nullptr);
    if (FAILED(res)) {
        return res;
    }
    res = stm->Commit(STGC_DEFAULT);
    if (FAILED(res)) {
        return res;
    }

    return res;
}


/**
 *  Saves a string to the storage set.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Save_String_Set(IPropertySetStorage *storageset, int id, const char *string)
{
    WCHAR buffer[128];

    MultiByteToWideChar(CP_ACP, 0, string, -1, buffer, std::size(buffer) - 1);

    HRESULT res;
    IPropertyStoragePtr storage;

    res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
    if (FAILED(res)) {
        res = storageset->Create(FMTID_SummaryInformation, nullptr, PROPSETFLAG_DEFAULT, STGM_SHARE_EXCLUSIVE|STGM_READWRITE|STGM_CREATE, &storage);
        if (FAILED(res)) {
            return res;
        }
    }

    PROPSPEC propsec;
    propsec.ulKind = PRSPEC_PROPID;
    propsec.propid = id;
    PROPVARIANT propvar;

    propvar.vt = VT_LPWSTR;
    propvar.pwszVal = buffer;

    res = storage->WriteMultiple(1, &propsec, &propvar, 2);
    if (FAILED(res)) {
        return res;
    }

    return res;
}


/**
 *  Saves an integer to the storage.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Save_Int(IStorage *storage, int id, int integer)
{
    IStreamPtr stm(nullptr);

    HRESULT res = storage->CreateStream(Vinifera_Stream_Name_From_ID(id), STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    res = stm->Write(&integer, sizeof(integer), nullptr);
    if (FAILED(res)) {
        return res;
    }
    res = stm->Commit(STGC_DEFAULT);
    if (FAILED(res)) {
        return res;
    }

    return res;
}


/**
 *  Saves an integer to the storage set.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Save_Int_Set(IPropertySetStorage *storageset, int id, int integer)
{
    HRESULT res;
    IPropertyStoragePtr storage;

    res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
    if (FAILED(res)) {
        res = storageset->Create(FMTID_SummaryInformation, nullptr, PROPSETFLAG_DEFAULT, STGM_SHARE_EXCLUSIVE|STGM_READWRITE|STGM_CREATE, &storage);
        if (FAILED(res)) {
            return res;
        }
    }

    PROPSPEC propsec;
    propsec.ulKind = PRSPEC_PROPID;
    propsec.propid = id;
    PROPVARIANT propvar;

    propvar.vt = VT_I4;
    propvar.lVal = integer;

    res = storage->WriteMultiple(1, &propsec, &propvar, 2);
    if (FAILED(res)) {
        return res;
    }

    return res;
}


/**
 *  Load a FILETIME from the storage.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Load_Time(IStorage *storage, int id, FILETIME *time)
{
    HRESULT res;
    IStreamPtr stm;

    time->dwLowDateTime = 0;
    time->dwHighDateTime = 0;

    res = storage->OpenStream(Vinifera_Stream_Name_From_ID(id), nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    res = stm->Read(time, sizeof(*time), nullptr);
    if (FAILED(res)) {
        return res;
    }

    return res;
}


/**
 *  Load a FILETIME from the storage set.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Load_Time_Set(IPropertySetStorage *storageset, int id, FILETIME *time)
{
    HRESULT res;
    IPropertyStoragePtr storage;

    time->dwLowDateTime = 0;
    time->dwHighDateTime = 0;


    res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
    if (FAILED(res)) {
        return res;
    }

    PROPSPEC propsec;
    propsec.ulKind = PRSPEC_PROPID;
    propsec.propid = id;
    PROPVARIANT propvar;

    res = storage->ReadMultiple(1, &propsec, &propvar);
    if (FAILED(res)) {
        return res;
    }

    if (propvar.vt == VT_FILETIME) {
        *time = propvar.filetime;
    }

    return res;
}


/**
 *  Save a FILETIME to the storage.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Save_Time(IStorage *storage, int id, FILETIME *time)
{
    IStreamPtr stm(nullptr);

    HRESULT res = storage->CreateStream(Vinifera_Stream_Name_From_ID(id), STGM_SHARE_EXCLUSIVE|STGM_READWRITE, 0, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    res = stm->Write(time, sizeof(*time), nullptr);
    if (FAILED(res)) {
        return res;
    }
    res = stm->Commit(STGC_DEFAULT);
    if (FAILED(res)) {
        return res;
    }

    return res;
}


/**
 *  Save a FILETIME to the storage set.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Save_Time_Set(IPropertySetStorage *storageset, int id, FILETIME *time)
{
    HRESULT res;
    IPropertyStoragePtr storage;

    res = storageset->Open(FMTID_SummaryInformation, STGM_SHARE_EXCLUSIVE|STGM_READWRITE, &storage);
    if (FAILED(res)) {
        res = storageset->Create(FMTID_SummaryInformation, nullptr, PROPSETFLAG_DEFAULT, STGM_SHARE_EXCLUSIVE|STGM_READWRITE|STGM_CREATE, &storage);
        if (FAILED(res)) {
            return res;
        }
    }

    PROPSPEC propsec;
    propsec.ulKind = PRSPEC_PROPID;
    propsec.propid = id;
    PROPVARIANT propvar;

    propvar.vt = VT_FILETIME;
    propvar.filetime = *time;

    res = storage->WriteMultiple(1, &propsec, &propvar, 2);
    if (FAILED(res)) {
        return res;
    }

    return res;
}


/**
 *  Get the name of the stream from the ID.
 *
 *  @author: tomsons26, ZivDero
 */
const WCHAR *Vinifera_Stream_Name_From_ID(int id)
{
    struct StreamID {
        int ID;
        WCHAR const *Name;
    };

    static StreamID _ids[] = {
        { ViniferaSaveVersionInfo::ID_SCENARIO_DESCRIPTION,  L"Scenario Description" },
        { ViniferaSaveVersionInfo::ID_PLAYER_HOUSE,          L"Player House" },
        { ViniferaSaveVersionInfo::ID_VERSION,               L"Version" },
        { ViniferaSaveVersionInfo::ID_INTERNAL_VERSION,      L"Internal Version" },
        { ViniferaSaveVersionInfo::ID_START_TIME,            L"Start Time" },
        { ViniferaSaveVersionInfo::ID_LAST_SAVE_TIME,        L"Last Save Time" },
        { ViniferaSaveVersionInfo::ID_PLAY_TIME,             L"Play Time" },
        { ViniferaSaveVersionInfo::ID_EXECUTABLE_NAME,       L"Executable Name" },
        { ViniferaSaveVersionInfo::ID_PLAYER_NAME,           L"Player Name" },
        { ViniferaSaveVersionInfo::ID_PLAYER_NAME2,          L"Player Name2" },
        { ViniferaSaveVersionInfo::ID_SCENARIO_NUMBER,       L"Scenario Number" },
        { ViniferaSaveVersionInfo::ID_CAMPAIGN,              L"Campaign" },
        { ViniferaSaveVersionInfo::ID_GAMETYPE,              L"GameType" },

        { ViniferaSaveVersionInfo::ID_VINIFERA_VERSION,      L"Vinifera Version" },
        { ViniferaSaveVersionInfo::ID_VINIFERA_COMMIT_HASH,  L"Vinifera Commit Hash" },
        { ViniferaSaveVersionInfo::ID_PLAYTHROUGH_ID,        L"Playthrough ID" },
        { ViniferaSaveVersionInfo::ID_DIFFICULTY,            L"Difficulty" },
        { ViniferaSaveVersionInfo::ID_TOTAL_PLAY_TIME,       L"Total Play Time" },
    };

    for (int i = 0; i < std::size(_ids); i++) {
        if (_ids[i].ID == id) {
            return _ids[i].Name;
        }
    }

    return nullptr;
}


/**
 *  Read the save version info from a save file.
 *
 *  @author: tomsons26
 */
bool Vinifera_Get_Savefile_Info(char const* name, ViniferaSaveVersionInfo& info)
{
    IStoragePtr storage;
    WCHAR wname[PATH_MAX];

    MultiByteToWideChar(0, 0, name, -1, wname, std::size(wname));

    HRESULT result = StgOpenStorage(wname, nullptr, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, nullptr, 0, &storage);
    if (FAILED(result)) {
        return false;
    }

    result = info.Load(storage);
    if (FAILED(result)) {
        return false;
    }

    return true;
}
