/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MISCUTIL.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Misc utility functions for common tasks.
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
#include "miscutil.h"
#include "rawfile.h"
#include "ffactory.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <Windows.h>
#include <winver.h> // for GetFileVersionInfoSize, GetFileVersionInfo.
#include <tlhelp32.h> // Must be after Windows.h!
#include <shlwapi.h> // for PathFindExtension
#include <shlobj.h> // for SHGetKnownFolderPath
#include <dbghelp.h>
#include <string>
#include <locale>
#include <codecvt>

#include "ccfile.h"
#include "objectext.h"
#include "motionlib.h"
#include "voxellib.h"


const char *Get_Text_Time()
{
    time_t timer = std::time(nullptr);
    char *time = std::ctime(&timer);

    time[std::strlen(time) - 1] = '\0';
    
    return time;
}


void Seconds_To_Hms(float seconds, int &h, int &m, int &s)
{
    /**
     *  Do the hours first: there are 3600 seconds in an hour, so if we divide
     *  the total number of seconds by 3600 and throw away the remainder, we're
     *  left with the number of hours in those seconds.
     */
    h = (seconds / 3600.0);

    seconds -= (h * 3600);
    m = (seconds / 60.0);
    s -= (m * 60.0); 
}


void Get_Time(int &hour, int &min, int &sec)
{
    time_t raw;
    tm t;

    std::time(&raw);

    localtime_s(&t, &raw);

    hour = t.tm_hour;
    min = t.tm_min;
    sec = t.tm_sec;
}


void Get_Full_Time(int &day, int &month, int &year, int &hour, int &min, int &sec)
{
    time_t raw;
    std::time(&raw);

    struct tm t;
    localtime_s(&t, &raw);

    year = 1900 + t.tm_year; // struct tm holds years since 1900.
    month = t.tm_mon + 1; // month is zero based.
    day = t.tm_mday;
    hour = t.tm_hour;
    min = t.tm_min;
    sec = t.tm_sec;
}


const char *Get_Date_Time_String(bool filename_safe)
{
    static const char wday_name[7][4] = {
        "Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"
    };

    static const char month_name[12][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    static char buffer[64];

    time_t raw;
    std::time(&raw);

    struct tm t;
    localtime_s(&t, &raw);

    const char *format = "%.3s %.3s%3d %.2d:%.2d:%.2d %d\n";
    if (filename_safe) {
        format = "%.3s_%.3s%3d_%.2d_%.2d_%.2d_%d\n";
    }

    std::snprintf(buffer, sizeof(buffer),
            format,
            wday_name[t.tm_wday],
            month_name[t.tm_mon],
            t.tm_mday,
            t.tm_hour,
            t.tm_min,
            t.tm_sec,
            1900 + t.tm_year); // struct tm holds years since 1900.

    return buffer;
}


void Get_File_Id_String(const char *filename, char &str)
{
    char buffer[1024];
    IMAGE_FILE_HEADER f_hdr; // Temp file header.

    std::memset(&f_hdr, 0, sizeof(f_hdr));

    RawFileClass file(filename);
    size_t file_size = file.Size();

    Get_EXE_File_Header(filename, &f_hdr);

    /**
     *  Copy the filename to the buffer.
     */
    std::strcpy(buffer, filename);

    /**
     *  Convert the string to uppercase.
     */
    strupr(buffer);

    /**
     *  Track back through the string looking for the first path separator...
     */
    int i = 1;
    char *end = nullptr;
    for (end = &buffer[strlen(buffer)]; end > buffer; --end) {
        if (end[-1] == '\\' || end[-1] == '/') {
            break;
        }
        ++i;
    }

    /**
     *  ...then copy the end bit of the string over the buffer, including
     *  the null terminator it already had.
     */
    std::memmove(buffer, end, i);

    /**
     *  Get the time and date stamp.
     */
    int timedate = (int)f_hdr.TimeDateStamp;

    std::snprintf(buffer, sizeof(buffer), "%s %zd %d", buffer, file_size, timedate);

    /**
     *  Copy the final string to the input destination.
     */
    std::strcpy(&str, buffer);
}


bool Get_EXE_File_Header(const char *filename, IMAGE_FILE_HEADER *header)
{
    /**
     *  Get the file from the factory.
     */
    RawFileClass file(filename);
    /*FileClass *file = TheFileFactory->Get_File(filename);
    if (file != nullptr ) {
        return false;
    }*/

    /**
     *  Open the file for reading.
     */
    if (!file.Open(FILE_ACCESS_READ) || !file.Is_Available()) {
        return false;
    }

    /**
     *  Read the IMAGE_DOS_HEADER from the binary file.
     */
    IMAGE_DOS_HEADER hdr; // Temp dos file header.
    if (file.Read(&hdr, sizeof(IMAGE_DOS_HEADER)) != sizeof(IMAGE_DOS_HEADER)) {
        return false;
    }

    /**
     *  Fetch and seek to the offset of the IMAGE_FILE_HEADER.
     */
    int64_t file_hdr_offs = (hdr.e_lfanew + 4);
    file.Seek(file_hdr_offs, FILE_SEEK_START);

    /**
     *  Read the IMAGE_FILE_HEADER from the binary file.
     */
    if (file.Read(header, sizeof(IMAGE_FILE_HEADER)) != sizeof(IMAGE_FILE_HEADER) ) {
        return false;
    }

    /**
     *  Return the file to the factory.
     */
    //TheFileFactory->Return_File(file);

    /**
     *  Return if we successfully read the file header or not.
     */
    return true;
}


bool Get_EXE_File_Header_From_Instance(HINSTANCE h, IMAGE_FILE_HEADER *f_hdr)
{
    if (h == nullptr) {
        return false;
    }

    /**
     *  HINSTANCE in Win32 is a pointer to the first byte of the exe file
     *  that was loaded into memory, which is the first byte of the IMAGE_DOS_HEADER.
     */
    IMAGE_DOS_HEADER *dos_hdr = reinterpret_cast<IMAGE_DOS_HEADER *>(h);

    /**
     *  Offset of NT Header is found at 0x3C location in DOS header specified by e_lfanew.
     */
    long offs = (dos_hdr->e_lfanew + 4);

    std::memcpy(f_hdr, &offs, sizeof(IMAGE_FILE_HEADER));

    return true;
}


bool Get_File_Creation_Time(const char *filename, time_t & time)
{
    /**
     *  Make sure input filename is valid before we continue.
     */
    if (filename == nullptr) {
        return false;
    }

    /**
     *  Make sure we was parsed a valid reference to a time_t struct.
     */
    if (&time == nullptr) {
        return false;
    }

    /**
     *  Get the file from the factory.
     */
    RawFileClass file(filename);
    /*FileClass *file = TheFileFactory->Get_File(filename);
    if (file == nullptr) {
        return false;
    }*/

    /**
     *  Open the file for reading.
     */
    if (!file.Open(FILE_ACCESS_READ) || !file.Is_Available()) {
        return false;
    }

    /**
     *  Clear the destination time struct.
     */
    time = 0;

    /**
     *  Fetch the creation date and time.
     */
    if ((time = file.Get_Date_Time()) != 0) {
        return true;
    }

    return false;
}


int Compare_EXE_Version(HINSTANCE h, const char *filename)
{
    IMAGE_FILE_HEADER fhdr1; // Temp dos file headers.
    IMAGE_FILE_HEADER fhdr2;

    if (Get_EXE_File_Header_From_Instance(h, &fhdr1) && Get_EXE_File_Header(filename, &fhdr2)) {
        return fhdr1.TimeDateStamp - fhdr2.TimeDateStamp;
    }

    return false;

}


bool Get_Version_Info(const char *filename, VS_FIXEDFILEINFO *out_file_info)
{
    bool retval = false;

    DWORD handle = 0;

    /**
     *  Get the file version info size.
     */
    DWORD size = GetFileVersionInfoSize(filename, &handle);
    ASSERT_PRINT(size > 0, "Error in GetFileVersionInfoSize: %d", GetLastError());
    if (size <= 0) {
        return false;
    }

    /**
     *  Allocation of space for the version size.
     */
    BYTE *verinfo = new BYTE[size];

    BOOL info_obtained = GetFileVersionInfo(filename, handle, size, verinfo);
    ASSERT_PRINT(info_obtained, "Get_Version_Info() - Error in GetFileVersionInfo: %d", GetLastError());
    if (info_obtained) {

        UINT len = 0;
        VS_FIXEDFILEINFO *fileinfo = nullptr;

        BOOL query_success = VerQueryValue(verinfo, "\\", (LPVOID *)&fileinfo, &len);
        ASSERT_PRINT(query_success, "Get_Version_Info() - Error in VerQueryValue: %d", GetLastError());

        if (query_success) {

            if (fileinfo != nullptr) {
                std::memcpy(out_file_info, fileinfo, sizeof(VS_FIXEDFILEINFO));
                retval = true;
            }

        }

    }

    delete verinfo;

    return retval;
}


void HexPrint32(const uint32_t *data, size_t size)
{
    for (int i = 0; i < size; ++i) {
        if (!(i % 80)) {
            DEBUG_INFO("\n");
        }
        DEBUG_INFO("0x%04llX", data[i]);
    }
    DEBUG_INFO("\n");
}


void HexPrint64(const uint64_t *data, size_t size)
{
    for (int i = 0; i < size; ++i) {
        if (!(i % 80)) {
            DEBUG_INFO("\n");
        }
        DEBUG_INFO("0x%08llX", data[i]);
    }
    DEBUG_INFO("\n");
}


int Clock_Get_Time(clockid_t id, struct timespec *ts)
{
    FILETIME ftime;
    GetSystemTimeAsFileTime(&ftime);

    int64_t wintime = ftime.dwLowDateTime + ((int64_t)ftime.dwHighDateTime << 32);

    /**
     *  Convert to base unix time.
     *  
     *  FILETIME is a 64-bit unsigned integer representing
     *  the number of 100-nanosecond intervals since January 1, 1601
     *  UNIX timestamp is number of seconds since January 1, 1970
     *  116444736000000000 = 10000000 * 60 * 60 * 24 * 365 * 369 + 89 leap days
     */
    wintime -= 116444736000000000;
    ts->tv_sec = wintime / 10000000;
    ts->tv_nsec = wintime % 10000000 * 100;

    return 0;
}


bool Create_Directory(const char *name)
{
    return CreateDirectory(name, nullptr);
}


bool Directory_Exists(char const *name)
{
    DWORD ftyp = GetFileAttributes(name);
    if (ftyp == INVALID_FILE_ATTRIBUTES) {
        return false;
    }
    if ((ftyp & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        return true;
    }
    return false;
}


void Set_Working_Directory()
{
    char path[MAX_PATH];

    GetModuleFileName(GetModuleHandle(nullptr), path, sizeof(path));

    for (char *i = &path[std::strlen(path)]; i != path; --i) {
        if (*i == '\\' || *i == '/') {
            *i = '\0';
            break;
        }
    }

    SetCurrentDirectory(path);
}


int Get_Last_System_Error()
{
    return GetLastError();
}


bool Delete_File(const char *filename)
{
    return RawFileClass(filename).Delete();
}


bool Rename_File(const char *filename, const char *new_filename)
{
    return std::rename(filename, new_filename) == 0;
}


bool File_Exists(const char *filename)
{
    return RawFileClass(filename).Is_Available();
}


/**
 *  Checks if a file exists in the directory using the Windows API.
 */
bool WinAPI_File_Exists(const char *file)
{
    WIN32_FIND_DATA fileinfo;
    HANDLE handle = FindFirstFile(file, &fileinfo) ;
    bool found = (handle != INVALID_HANDLE_VALUE);
    if (found) {
        FindClose(handle);
    }
    return found;
}


bool Is_Full_Path(const char *path)
{
    if (path == nullptr) {
        return false;
    }

    return path[1] == ':' || (path[0] == '\\' && path[1] == '\\') || path[0] == '/';
}


const char *Get_User_Documents_Path()
{
    static char path[PATH_MAX];
    HRESULT hr;

    OSVERSIONINFO osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    GetVersionEx(&osvi);
	
	BOOL bIsWindowsVistaOrLater = (osvi.dwMajorVersion > 5);

    // Windows users can move the Documents folder to any location they want
    // SHGetKnownFolderPath/SHGetFolderPath knows how to find it.
    // *FLAG_CREATE = Flag to specify if the requested folder must be created if it didn't exist.

    if (bIsWindowsVistaOrLater) {
        PWSTR ppszPath;    // variable to receive the path memory block pointer.
        hr = SHGetKnownFolderPath(FOLDERID_Documents, KF_FLAG_DEFAULT|KF_FLAG_CREATE, nullptr, &ppszPath);
        if (FAILED(hr)) {
            return nullptr;
        }
        std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
        std::string str(converter.to_bytes(ppszPath), sizeof(path));
        std::strncpy(path, str.c_str(), std::strlen(str.c_str()));
        CoTaskMemFree(ppszPath);    // free up the path memory block.
        return path;

    } else {
        PSTR ppszPath;    // variable to receive the path memory block pointer.
        hr = SHGetFolderPath(NULL, CSIDL_MYDOCUMENTS|CSIDL_FLAG_CREATE, NULL, SHGFP_TYPE_CURRENT, ppszPath);
        if (FAILED(hr)) {
            return nullptr;
        }
        std::strncpy(path, ppszPath, sizeof(path));
        CoTaskMemFree(ppszPath);    // free up the path memory block.
        return path;
    }
}


const char *Filename_From_Path(const char *filename)
{
    static char path[_MAX_PATH];
    static char name[_MAX_FNAME];
    static char ext[_MAX_EXT];

    std::memset(path, 0, _MAX_PATH);

    /**
     *  Strip the drive and path (if present) off of the filename.
     */
    _splitpath(filename, nullptr, nullptr, name, ext);
    _makepath(path, nullptr, nullptr, name, ext);

    return path;
}


bool Parse_Boolean(const char* value, bool defval)
{
    while (*value == ' ') {
        value++;
    }

    switch (toupper(value[0])) {
    case '0':
    case 'F':
    case 'N':
        return false;
    case '1':
    case 'T':
    case 'Y':
        return true;
    default:
        return defval;
    }
}
