/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WINUTIL.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Utility functions for interacting with the Windows API.
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
#include "winutil.h"
#include "debughandler.h"
#include <windows.h>
#include <Tlhelp32.h>
#include <shlwapi.h> // for PathFindExtension
#include <string>
#include <locale>
#include <codecvt>
#include <algorithm>


const char *Get_Module_File_Name()
{
    static char outbuff[PATH_MAX] = { '\0' };
    char buffer[PATH_MAX];

    if (outbuff[0] != '\0') {
        return outbuff;
    }

    /**
     *  Get the fully-qualified path of the executable.
     *  Output = "c:\folder\executable.exe"
     */
    if (GetModuleFileName(nullptr, buffer, sizeof(buffer)) == sizeof(buffer)) {
        DEBUG_ERROR("Error with GetModuleFileName in Get_Module_File_Name()!");
        return nullptr;
    }

    /**
     *  Go to the beginning of the file name.
     *  Output = "executable.exe"
     */
    char *out = PathFindFileName(buffer);

    /*
     *  Set the dot before the extension to 0 (terminate the string there).
     *  Output = "executable"
     */
    *(PathFindExtension(out)) = '\0';

    std::strcpy(outbuff, out);

    return outbuff;
}


const char *Get_Module_Directory()
{
    static char outbuff[PATH_MAX] = { '\0' };
    char buffer[PATH_MAX];

    if (outbuff[0] != '\0') {
        return outbuff;
    }

    /**
     *  Get the fully-qualified path of the executable.
     *  Output = "c:\folder\executable.exe"
     */
    if (GetModuleFileName(nullptr, buffer, sizeof(buffer)) == sizeof(buffer)) {
        DEBUG_ERROR("Error with GetModuleFileName in Get_Module_Directory()!");
        return nullptr;
    }

    /**
     *  Go to the beginning of the file name.
     *  Output = "executable.exe"
     */
    char *out = PathFindFileName(buffer);

    /*
     *  Insert a null character at the module name.
     */
    *(out) = '\0';

    std::strcpy(outbuff, buffer);

    return outbuff;
}


const char *Get_Module_File_Name_Ext()
{
    static char outbuff[PATH_MAX] = "";
    char buffer[PATH_MAX];

    /**
     *  Get the fully-qualified path of the executable.
     *  Output = "c:\folder\executable.exe"
     */
    if (GetModuleFileName(nullptr, buffer, sizeof(buffer)) == sizeof(buffer)) {
        DEBUG_ERROR("Error with GetModuleFileName in Get_Module_File_Name_Ext()!");
        return nullptr;
    }

    /**
     *  Go to the beginning of the file name.
     *  Output = "executable.exe"
     */
    char *out = PathFindFileName(buffer);

    std::strcpy(outbuff, out);

    return outbuff;
}


/**
 *  Returns the last Win32 error, in string format.
 */
// NOTE: Added FORMAT_MESSAGE_MAX_WIDTH_MASK to remove line breaks from result message buffer.
const char *Last_System_Error_As_String()
{
    static char buffer[1024];

    DWORD dw = GetLastError(); 

    FormatMessage(
        FORMAT_MESSAGE_MAX_WIDTH_MASK | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
        nullptr,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), 
        buffer,
        sizeof(buffer),
        nullptr);

    return buffer;
}


void Convert_System_Error_To_String(int id, char *buffer, int buf_len)
{
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, nullptr, id, MAKELANGID(LANG_NEUTRAL, SUBLANG_NEUTRAL), buffer, buf_len, nullptr);
}


// Source: http://blog.aaronballman.com/2011/12/string-resources/
int Load_String_Ex(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, INT nBufferMax, WORD wLanguage)
{
    /**
     *  Loading a string is a bit strange. Strings are grouped by blocks of 16
     *  items. However, the ID passed in does not reflect this. This means 
     *  that the resource itself does *not* have the same ID as what the user
     *  expects! Instead, the actual resource is a combination of the block
     *  number and index within the block. This can be calculated by 
     *  translating the ID.
     */
    UINT blockNumber = (uID >> 4) + 1;
    UINT indexNumber = uID % 16;
 
    // Now we can attempt to find the resource by block number
    HRSRC hResource = FindResourceEx(hInstance, RT_STRING, MAKEINTRESOURCE(blockNumber), wLanguage);
    if (hResource != nullptr) {

        /*
         *  Get the size of the block; we need to traverse it to find the index
         *  we are after. The strings are like Unicode versions of Pascal 
         *  strings; they are prefaced with a single byte denoting the number of
         *  characters in the string (or 0 if the string is empty).
         */
        DWORD size = SizeofResource(hInstance, hResource);
        HGLOBAL glob = LoadResource(hInstance, hResource);
        if (glob && size) {

            LPCWSTR buffer = static_cast<LPCWSTR>(LockResource(glob));
            LPCWSTR end = buffer + size;
            if (buffer) {

                INT idx = 0;
                while (buffer < end) {

                    /**
                     *  Get the length byte
                     */
                    WORD length = static_cast<WORD>(buffer[0]);
                    if (idx == indexNumber) {

                        /**
                         *  We are at the string we're after, so copy it into 
                         *  the buffer the caller passed. If the caller did not
                         *  pass a buffer (the buffer length is zero), then we
                         *  point the passed buffer to the start of the resource.
                         */
                        if (nBufferMax) {
                            if (std::memcpy(lpBuffer, &buffer[1], length * sizeof(wchar_t)) == 0) {

                                /**
                                 *  Return the number of characters copied into the buffer
                                 */
                                return std::min<int>(nBufferMax, length);
                            }
                        } else {
                            lpBuffer = const_cast<LPWSTR>(&buffer[1]);
                            return 0;    // This is the number of characters copied!
                        }
                    }
 
                    /**
                     *  Advance by the string length, plus one for the length byte itself
                     */
                    buffer += length + 1;
 
                    /**
                     *  Advance our index
                     */
                    ++idx;
                }
            }
        }
    }

    return 0;
}


/**
 *  #TODO: This is horrible, but it's the only way i see loading strings with specific languages...
 */
int Load_String_Ex(HINSTANCE hInstance, UINT uID, LPCSTR lpBuffer, INT nBufferMax, WORD wLanguage)
{
    wchar_t pwBuffer[2048];
    if (!Load_String_Ex(hInstance, uID, pwBuffer, sizeof(pwBuffer), wLanguage)) {
        DEBUG_INFO("Load_String_Ex() - Load_String_Ex(wide) failed. Error! %s\n", Last_System_Error_As_String());
        return 0;
    }

    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
    std::string str(converter.to_bytes(pwBuffer), sizeof(pwBuffer));
    std::strncpy((char *)lpBuffer, str.c_str(), nBufferMax);
    return lpBuffer[0] != '\0' && std::strlen(lpBuffer);
}


DWORD Find_Process_Id(const char *process_name)
{
    PROCESSENTRY32 processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));
    processInfo.dwSize = sizeof(processInfo);

    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (processesSnapshot == INVALID_HANDLE_VALUE) {
        return 0;
    }

    Process32First(processesSnapshot, &processInfo);
    if (!strcmp(process_name, processInfo.szExeFile)) {
        CloseHandle(processesSnapshot);
        return processInfo.th32ProcessID;
    }

    while (Process32Next(processesSnapshot, &processInfo)) {
        if (!strcmp(process_name, processInfo.szExeFile)) {
            CloseHandle(processesSnapshot);
            return processInfo.th32ProcessID;
        }
    }

    CloseHandle(processesSnapshot);

    return 0;
}


HANDLE Get_Process_by_Id(DWORD pId)
{
    PROCESSENTRY32 processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));
    processInfo.dwSize = sizeof(processInfo);

    // Create toolhelp snapshot.
    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    Process32First(processesSnapshot, &processInfo);
    if (processInfo.th32ProcessID == pId) {
        CloseHandle(processesSnapshot);
        return OpenProcess(PROCESS_ALL_ACCESS, FALSE, processInfo.th32ProcessID);
    }

    while (Process32Next(processesSnapshot, &processInfo)) {
        //DEBUG_INFO("Checking process %s %d...\n", processInfo.szExeFile, processInfo.th32ProcessID);
        if (processInfo.th32ProcessID == pId) {
            CloseHandle(processesSnapshot);
            return OpenProcess(PROCESS_ALL_ACCESS, FALSE, processInfo.th32ProcessID);
        }
    }

    CloseHandle(processesSnapshot);

    return nullptr;
}


HANDLE Get_Process_By_Name(const char *process_name)
{
    PROCESSENTRY32 processInfo;
    ZeroMemory(&processInfo, sizeof(processInfo));
    processInfo.dwSize = sizeof(processInfo);

    DWORD pid = 0;

    // Create toolhelp snapshot.
    HANDLE processesSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    Process32First(processesSnapshot, &processInfo);
    if (!strcmp(process_name, processInfo.szExeFile)) {
        CloseHandle(processesSnapshot);
        if (processInfo.th32ProcessID != 0) {
            return OpenProcess(PROCESS_ALL_ACCESS, FALSE, processInfo.th32ProcessID);
        }
    }

    while (Process32Next(processesSnapshot, &processInfo)) {
        if (!strcmp(process_name, processInfo.szExeFile)) {
            CloseHandle(processesSnapshot);
            if (processInfo.th32ProcessID != 0) {
                return OpenProcess(PROCESS_ALL_ACCESS, FALSE, processInfo.th32ProcessID);
            }
        }
    }

    CloseHandle(processesSnapshot);

    return nullptr;
}


/**
 *  Get the thread id of the main thread of a target process.
 */
DWORD Get_Process_Main_Thread_Id(DWORD pId)
{
    LPVOID lpThId;

    _asm
    {
        mov eax, fs:[18h]
        add eax, 36
        mov [lpThId], eax
    }

    HANDLE hProcess = OpenProcess(PROCESS_VM_READ, FALSE, pId);
    if (hProcess == nullptr) {
        return 0;
    }

    DWORD tId;
    if (ReadProcessMemory(hProcess, lpThId, &tId, sizeof(tId), nullptr) == FALSE) {
        CloseHandle(hProcess);
        return 0;
    }

    CloseHandle(hProcess);

    return tId;
}


/**
 *  Get a handle to the main thread of a target process.
 */
HANDLE Get_Thread_Handle(DWORD pId, DWORD dwDesiredAccess)
{
    DWORD tId = Get_Process_Main_Thread_Id(pId);
    if (tId == 0) {
        return nullptr;
    }
    return OpenThread(dwDesiredAccess, FALSE, tId);
}


/**
 *  Finds module handle from some address inside it.
 */
HMODULE Get_Module_From_Address(LPVOID address)
{
	HMODULE module;
	if (GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS|GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (char *)address, &module)) {
		return module;
    }
	return nullptr;
}


HICON LoadIconFromFile(const char *filename, int width, int height)
{
    HICON hIcon = (HICON)LoadImage(NULL,
                         filename, IMAGE_ICON,
                         width,
                         height,
                         (LR_LOADTRANSPARENT|LR_LOADFROMFILE));

    return hIcon;
}


/**
 *  Based off: https://stackoverflow.com/a/46203975
 */
bool DeleteFilesOlderThan(unsigned days, const char *in, const char *filename)
{
    #define FT_SECOND ((INT64) 10000000)
    #define FT_MINUTE (60 * FT_SECOND)
    #define FT_HOUR   (60 * FT_MINUTE)
    #define FT_DAY    (24 * FT_HOUR)

    if (days > 90 || !in || !filename) {
        return false;
    }

    /**
     *  Build full path.
     */
    std::string folder = /*std::string(".\\") +*/ std::string(in) + std::string("\\");

    /**
     *  Get date today @ 00:00:00:00 time, in UTC...
     */
    SYSTEMTIME stUTC = {0};
    GetSystemTime(&stUTC);
    stUTC.wHour = stUTC.wMinute = stUTC.wSecond =  stUTC.wMilliseconds = 0;

    /**
     *  Subtract age in days from it...
     */
    FILETIME ftDaysAgo = {0};
    ULARGE_INTEGER ul;
    SystemTimeToFileTime(&stUTC, &ftDaysAgo);
    ul.LowPart = ftDaysAgo.dwLowDateTime;
    ul.HighPart = ftDaysAgo.dwHighDateTime;
    ul.QuadPart -= (days * FT_DAY);
    ftDaysAgo.dwLowDateTime = ul.LowPart;
    ftDaysAgo.dwHighDateTime = ul.HighPart;

    /**
     *  Now search for files...
     */
    WIN32_FIND_DATA info;
    HANDLE hp = FindFirstFile((folder + filename).c_str(), &info);
    if (hp != INVALID_HANDLE_VALUE) {
        do {
            std::string composite_filename = (folder + info.cFileName);

            /**
             *  Process only files...
             */
            if ((info.dwFileAttributes == FILE_ATTRIBUTE_DIRECTORY)) {
                DEBUG_INFO("%s is dir\n", composite_filename.c_str());
                continue;
            }

            /**
             *  That have a last-write time...
             */
            if ((info.ftLastWriteTime.dwLowDateTime == 0) || (info.ftLastWriteTime.dwHighDateTime == 0)) {
                continue;
            }

            /**
             *  Older than 'X' days...
             */
            if (CompareFileTime(&(info.ftLastWriteTime), &ftDaysAgo) < 0) {

                DEBUG_INFO("  Deleting \"%s\".\n", composite_filename.c_str());

                /**
                 *  And delete!
                 */
                DeleteFile(composite_filename.c_str());
            }
        }
        while (FindNextFile(hp, &info));

        FindClose(hp);
    }

    return true;
}
