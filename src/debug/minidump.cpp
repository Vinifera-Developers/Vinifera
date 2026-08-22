/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Creates a mini dump for analysis.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "minidump.h"

#include "critsection.h"
#include "debughandler.h"
#include "debughlp.h"
#include "miscutil.h"
#include "vinifera_globals.h"

#include <dbghelp.h>
#include <mutex>
#include <windows.h>
#include <tlhelp32.h> // Must be after windows.h!


// Link dbhhelp.lib!
// #pragma comment(lib, "dbghelp.lib")


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
bool Create_Mini_Dump(struct _EXCEPTION_POINTERS *e_info, const char *app_name, const char *path, DWORD crashed_tid)
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
        std::snprintf((char *)MinidumpFilename, sizeof(MinidumpFilename), "%s\\MINIDUMP_%s_%02u-%02u-%04u_%02u-%02u-%02u.DMP",
            Vinifera_DebugDirectory.c_str(), strupr((char *)app_name), Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);

    } else {

        /**
         *  Create a unique filename for the crash dump based on the current time and module name.
         */
        std::snprintf((char *)MinidumpFilename, sizeof(MinidumpFilename), "%s\\CRASHDUMP_%s_%02u-%02u-%04u_%02u-%02u-%02u.DMP",
            Vinifera_DebugDirectory.c_str(), strupr((char *)app_name), Execute_Day, Execute_Month, Execute_Year, Execute_Hour, Execute_Min, Execute_Sec);

    }

    HANDLE dump_file = CreateFile(MinidumpFilename, GENERIC_WRITE, FILE_SHARE_WRITE|FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_FLAG_WRITE_THROUGH, nullptr);
    if (dump_file == INVALID_HANDLE_VALUE) {
        DEBUG_FATAL("Failed to create minidump file with filename \"{}\"! (error {}).", MinidumpFilename, GetLastError());
        return false;
    }

    Init_Symbol_Info();

    MINIDUMP_TYPE flags = MINIDUMP_TYPE(MiniDumpNormal
                                       |MiniDumpWithDataSegs
                                       |MiniDumpWithIndirectlyReferencedMemory);
    if (GenerateFullCrashDump) {
        flags = MINIDUMP_TYPE(flags | MiniDumpWithFullMemory);
    }

    MINIDUMP_EXCEPTION_INFORMATION md_e_info;
    ZeroMemory(&md_e_info, sizeof(MINIDUMP_EXCEPTION_INFORMATION));
    /**
     *  ThreadId must be the CRASHING thread. When the dump runs on the dumper
     *  thread (see exceptionhandler.cpp) that isn't the current thread, so the
     *  caller passes the TID; 0 means "use the calling thread".
     */
    md_e_info.ThreadId = (crashed_tid != 0) ? crashed_tid : GetCurrentThreadId();
    md_e_info.ExceptionPointers = e_info; // Exception data is optional and can be NULL.
    md_e_info.ClientPointers = FALSE;

    //DEBUG_WARNING("Create_Mini_Dump() - About to call MiniDumpWriteDump.\n");

    {
        std::scoped_lock dbghelp_lock(DbgHelpMutex);
        MiniDumpWriteDump(GetCurrentProcess(),
            GetCurrentProcessId(),
            dump_file,
            flags,
            e_info != nullptr ? &md_e_info : nullptr,
            nullptr,
            nullptr);
    }

    CloseHandle(dump_file);

    DEBUG_WARNING("Minidump generated: \"{}\".\n", MinidumpFilename);

    return true;
}
