/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MINIDUMP.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Creates a mini dump for analysis.
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
#include "minidump.h"
#include "winutil.h"
#include "miscutil.h"
#include "critsection.h"
#include "debughlp.h"
#include "debughandler.h"
#include "vinifera_globals.h"
#include <Windows.h>
#include <dbghelp.h>
#include <tlhelp32.h> // Must be after Windows.h!
#include <time.h>


// Link dbhhelp.lib!
//#pragma comment(lib, "dbghelp.lib")


extern int Execute_Day;
extern int Execute_Month;
extern int Execute_Year;
extern int Execute_Hour;
extern int Execute_Min;
extern int Execute_Sec;


bool GenerateFullCrashDump = false;
bool NonFatalMinidump = false;
bool MinidumpUseCurrentTime = false;

char MinidumpFilename[PATH_MAX] = { '\0' };


/**
 *  Creates a new file and dumps the exception info into it.
 */
bool Create_Mini_Dump(struct _EXCEPTION_POINTERS *e_info, const char *app_name, const char *path)
{
    static FastCriticalSectionClass MiniDumpCriticalSection;;
    FastCriticalSectionClass::LockClass critsection(MiniDumpCriticalSection);

    DEBUG_WARNING("Generating Minidump...\n");

    /**
     *  Clear the filename buffer.
     */
    MinidumpFilename[0] = '\0';

    if (MinidumpUseCurrentTime) {

        /**
         *  Get the current timestamp.
         */
        int day = 0;
        int month = 0;
        int year = 0;
        int hour = 0;
        int min = 0;
        int sec = 0;
        Get_Full_Time(day, month, year, hour, min, sec);

        /**
         *  Create a unique filename for the crash dump based on the current time and module name.
         */
        std::snprintf((char *)MinidumpFilename, sizeof(MinidumpFilename), ".\\%s\\MINIDUMP_%s_%02u-%02u-%04u_%02u-%02u-%02u.DMP",
            Vinifera_DebugDirectory, strupr((char *)app_name), Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);

    } else {

        /**
         *  Create a unique filename for the crash dump based on the current time and module name.
         */
        std::snprintf((char *)MinidumpFilename, sizeof(MinidumpFilename), ".\\%s\\CRASHDUMP_%s_%02u-%02u-%04u_%02u-%02u-%02u.DMP",
            Vinifera_DebugDirectory, strupr((char *)app_name), Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);

    }

    HANDLE dump_file = CreateFile(MinidumpFilename, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, nullptr);
    if (dump_file == INVALID_HANDLE_VALUE) {
        DEBUG_FATAL("Failed to create minidump file with filename \"%s\"! (error %d).", MinidumpFilename, GetLastError());
        return false;
    }

    Init_Symbol_Info();

    MINIDUMP_TYPE flags = MINIDUMP_TYPE(MiniDumpWithDataSegs
                                       |MiniDumpWithIndirectlyReferencedMemory);
    if (GenerateFullCrashDump) {
        flags = MINIDUMP_TYPE(flags | MiniDumpWithFullMemory);
    }

    MINIDUMP_EXCEPTION_INFORMATION md_e_info;
    ZeroMemory(&md_e_info, sizeof(MINIDUMP_EXCEPTION_INFORMATION));
    md_e_info.ThreadId = GetCurrentThreadId();
    md_e_info.ExceptionPointers = e_info; // Exception data is optional and can be NULL.
    md_e_info.ClientPointers = FALSE;

    //DEBUG_WARNING("Create_Mini_Dump() - About to call MiniDumpWriteDump.\n");

    MiniDumpWriteDump(GetCurrentProcess(),
        GetCurrentProcessId(),
        dump_file,
        flags,
        &md_e_info,
        nullptr,
        nullptr);

    CloseHandle(dump_file);

    DEBUG_WARNING("Minidump generated: \"%s\".\n", MinidumpFilename);

    return true;
}
