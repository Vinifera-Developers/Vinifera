/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended RawFileClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "rawfileext_hooks.h"

#include "asserthandler.h"
#include "debughandler.h"
#include "fatal.h"
#include "hooker.h"
#include "rawfile.h"
#include "vinifera_globals.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class RawFileClassExt : public RawFileClass
{
    public:
        long _Read(void *buffer, int length);
        void _Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr);
};


/**
 *  Reads the specified number of bytes into a memory buffer.
 * 
 *  @author: 10/18/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun, minor bug fix.
 */
long RawFileClassExt::_Read(void *buffer, int length)
{
    ASSERT_PRINT(buffer != nullptr, "Filename -> {}", Get_Safe_File_Name());
    ASSERT_PRINT(length > 0, "Filename -> {}", Get_Safe_File_Name());

    long bytesread = 0; // Running count of the number of bytes read into the buffer.
    int	opened = false; // Was the file opened by this routine?

    /**
     *  If the file isn't opened, open it. This serves as a convenience
     *  for the programmer.
     */
    if (!Is_Open()) {

        /**
         *  The error check here is moot. Open will never return unless it succeeded.
         */
        if (!Open(FILE_ACCESS_READ)) {
            return 0;
        }
        opened = true;
    }

    //DEV_DEBUG_INFO("File - Reading \"{}\".\n", Filename);

    /**
     *  A biased file has the requested read length limited to the bias length of
     *  the file.
     */
    if (BiasLength != -1) {
        int remainder = BiasLength - Seek(0);
        length = length < remainder ? length : remainder;
    }

    long total = 0;
    while (length > 0) {
        bytesread = 0;

        SetErrorMode(SEM_FAILCRITICALERRORS);
        if (!ReadFile(Handle, buffer, length, &(DWORD &)bytesread, nullptr)) {
            buffer = (unsigned char *)buffer + bytesread;
            length -= bytesread;
            total += bytesread;
            Error(FileErrorType(GetLastError()), true, Filename);
            //SetErrorMode(0);
        } else {
            //SetErrorMode(0);
            buffer = (unsigned char *)buffer + bytesread;
            length -= bytesread;
            total += bytesread;
            if (bytesread == 0) {
                break;
            }
        }
    }

    bytesread = total;

    /**
     *  Close the file if it was opened by this routine and return
     *  the actual number of bytes read into the buffer.
     */
    if (opened) {
        Close();
    }

    return total;
}


/**
 *  Handles displaying a file error message. 
 * 
 *  @author: 10/17/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun, minor bug fix.
 */
void RawFileClassExt::_Error(FileErrorType error, bool can_retry, const char *filename)
{
    static char buffer[2048];

    bool handled = false;

    std::snprintf(buffer, sizeof(buffer),
        "File - error:(%d) \"%s\"  can_retry:%s  filename:%s.\n",
        error, File_Error_To_String(error),
        can_retry ? "true" : "false",
        filename != nullptr ? filename : Filename);

    /**
     *  Output error to the debug system.
     */
    if (Vinifera_PrintFileErrors) {
        DEV_DEBUG_ERROR("{}", buffer);
    }

    /**
     *  If flagged, force exit the game.
     */
    if (!can_retry && Vinifera_FatalFileErrors) {
        Emergency_Exit(EXIT_FAILURE);
    }
    
    /**
     *  If flagged, trigger a fatal assert to the user.
     */
    if (Vinifera_AssertFileErrors) {
        ASSERT_FATAL_PRINT(can_retry, "{}", buffer);
    }
}


/**
 *  Main function for patching the hooks.
 */
void RawFileClassExtension_Hooks()
{
    Patch_Jump(0x005BE560, &RawFileClassExt::_Read);
    Patch_Jump(0x005BE300, &RawFileClassExt::_Error);
}
