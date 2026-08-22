/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Vinifera replacement of the save file header.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "vinifera_savever.h"

#include "debughandler.h"
#include "tibsun_defines.h"

#include <comdef.h>
#include <iterator>


/**
 *  Saves the version information to the storage.
 *
 *  @author: tomsons26, ZivDero
 */
HRESULT ViniferaSaveVersionInfo::Save(IStorage* storage) const
{
    if (storage == nullptr) {
        return E_POINTER;
    }

    DEBUG_INFO("Saving version information.\n");

    HRESULT res = Save_String(storage, L"Scenario Description", ScenarioDescription);
    if (FAILED(res)) {
        return res;
    }

    res = Save_String(storage, L"Player House", PlayerHouse);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, L"Internal Version", InternalVersion);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, L"Scenario Number", ScenarioNumber);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, L"Campaign", CampaignNumber);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, L"GameType", GameType);
    if (FAILED(res)) {
        return res;
    }

    /**
     *  New Vinifera fields.
     */
    res = Save_Int(storage, L"Vinifera Version", ViniferaVersion);
    if (FAILED(res)) {
        return res;
    }

    res = Save_String(storage, L"Vinifera Commit Hash", ViniferaCommitHash);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, L"Playthrough ID", PlaythroughID);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, L"Difficulty", Difficulty);
    if (FAILED(res)) {
        return res;
    }

    res = Save_Int(storage, L"Elapsed Time", ElapsedTime);
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
HRESULT ViniferaSaveVersionInfo::Load(IStorage* storage)
{
    if (storage == nullptr) {
        return E_POINTER;
    }

    DEBUG_INFO("Loading version information.\n");

    HRESULT res = Load_String(storage, L"Scenario Description", ScenarioDescription);
    if (FAILED(res)) {
        return res;
    }

    res = Load_String(storage, L"Player House", PlayerHouse);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, L"Internal Version", InternalVersion);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, L"Scenario Number", ScenarioNumber);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, L"Campaign", CampaignNumber);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, L"GameType", GameType);
    if (FAILED(res)) {
        return res;
    }

    /**
     *  New Vinifera fields.
     */
    res = Load_Int(storage, L"Vinifera Version", ViniferaVersion);
    if (FAILED(res)) {
        return res;
    }

    res = Load_String(storage, L"Vinifera Commit Hash", ViniferaCommitHash);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, L"Playthrough ID", PlaythroughID);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, L"Difficulty", Difficulty);
    if (FAILED(res)) {
        return res;
    }

    res = Load_Int(storage, L"Elapsed Time", ElapsedTime);
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
HRESULT ViniferaSaveVersionInfo::Load_String(IStorage* storage, const WCHAR* name, std::string& string)
{
    static constexpr std::size_t MAX_HEADER_STRING_LENGTH = 4096;

    string.clear();

    IStreamPtr stm;
    HRESULT res = storage->OpenStream(name, nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    std::wstring buffer;
    bool terminated = false;
    for (std::size_t i = 0; i < MAX_HEADER_STRING_LENGTH; ++i) {
        WCHAR ch = L'\0';
        ULONG bytes_read = 0;
        res = stm->Read(&ch, sizeof(ch), &bytes_read);
        if (FAILED(res) || bytes_read != sizeof(ch)) {
            return STG_E_READFAULT;
        }

        if (ch == L'\0') {
            terminated = true;
            break;
        }

        buffer.push_back(ch);
    }

    if (!terminated) {
        return STG_E_READFAULT;
    }

    int len = WideCharToMultiByte(CP_ACP, 0, buffer.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (len > 1) {
        string.resize(len);
        if (WideCharToMultiByte(CP_ACP, 0, buffer.c_str(), -1, string.data(), len, nullptr, nullptr) == 0) {
            string.clear();
            return HRESULT_FROM_WIN32(GetLastError());
        }
        string.pop_back();
    }

    return S_OK;
}


/**
 *  Loads an integer from the storage.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Load_Int(IStorage* storage, const WCHAR* name, int& integer)
{
    integer = 0;

    IStreamPtr stm;
    HRESULT res = storage->OpenStream(name, nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    ULONG bytes_read = 0;
    res = stm->Read(&integer, sizeof(integer), &bytes_read);
    if (FAILED(res) || bytes_read != sizeof(integer)) {
        return STG_E_READFAULT;
    }

    return res;
}


/**
 *  Loads a boolean from the storage.
 *
 *  @author: Rampastring
 */
HRESULT ViniferaSaveVersionInfo::Load_Bool(IStorage* storage, const WCHAR* name, bool& boolean)
{
    boolean = false;

    IStreamPtr stm;
    HRESULT res = storage->OpenStream(name, nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    ULONG bytes_read = 0;
    res = stm->Read(&boolean, sizeof(boolean), &bytes_read);
    if (FAILED(res) || bytes_read != sizeof(boolean)) {
        return STG_E_READFAULT;
    }

    return res;
}


/**
 *  Saves a string to the storage.
 *
 *  @author: tomsons26, ZivDero
 */
HRESULT ViniferaSaveVersionInfo::Save_String(IStorage* storage, const WCHAR* name, const std::string& string)
{
    int len = MultiByteToWideChar(CP_ACP, 0, string.c_str(), -1, nullptr, 0);
    std::wstring buffer(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, string.c_str(), -1, buffer.data(), len);

    IStreamPtr stm(nullptr);
    HRESULT res = storage->CreateStream(name, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    res = stm->Write(buffer.c_str(), sizeof(WCHAR) * len, nullptr);
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
 *  Saves an integer to the storage.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Save_Int(IStorage* storage, const WCHAR* name, int integer)
{
    IStreamPtr stm(nullptr);
    HRESULT res = storage->CreateStream(name, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
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
 *  Saves a boolean to the storage.
 *
 *  @author: Rampastring
 */
HRESULT ViniferaSaveVersionInfo::Save_Bool(IStorage* storage, const WCHAR* name, bool boolean)
{
    IStreamPtr stm(nullptr);
    HRESULT res = storage->CreateStream(name, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    res = stm->Write(&boolean, sizeof(boolean), nullptr);
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
 *  Load a FILETIME from the storage.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Load_Time(IStorage* storage, const WCHAR* name, FILETIME& time)
{
    time.dwLowDateTime = 0;
    time.dwHighDateTime = 0;

    IStreamPtr stm;
    HRESULT res = storage->OpenStream(name, nullptr, STGM_SHARE_EXCLUSIVE, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    ULONG bytes_read = 0;
    res = stm->Read(&time, sizeof(time), &bytes_read);
    if (FAILED(res) || bytes_read != sizeof(time)) {
        return STG_E_READFAULT;
    }

    return res;
}


/**
 *  Save a FILETIME to the storage.
 *
 *  @author: tomsons26
 */
HRESULT ViniferaSaveVersionInfo::Save_Time(IStorage* storage, const WCHAR* name, const FILETIME& time)
{
    IStreamPtr stm(nullptr);
    HRESULT res = storage->CreateStream(name, STGM_SHARE_EXCLUSIVE | STGM_READWRITE, 0, 0, &stm);
    if (FAILED(res)) {
        return res;
    }

    res = stm->Write(&time, sizeof(time), nullptr);
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
 *  Read the save version info from a save file.
 *
 *  @author: tomsons26
 */
bool Vinifera_Get_Savefile_Info(std::string_view name, ViniferaSaveVersionInfo& info)
{
    IStoragePtr storage;
    WCHAR wname[PATH_MAX];

    int len = MultiByteToWideChar(CP_ACP, 0, name.data(), (int)name.size(), wname, std::size(wname) - 1);
    wname[len] = L'\0';

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
