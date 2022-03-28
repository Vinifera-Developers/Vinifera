/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RAWFILEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended RawFileClass.
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
#include "rawfileext_hooks.h"
#include "rawfile.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class RawFileClassFake final : public RawFileClass
{
    public:
        long _Read(void *buffer, int length);
};


/**
 *  Reads the specified number of bytes into a memory buffer.
 * 
 *  @author: 10/18/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun, minor bug fix.
 */
long RawFileClassFake::_Read(void *buffer, int length)
{
    ASSERT(buffer != nullptr);
    ASSERT(length > 0);

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

    //DEV_DEBUG_INFO("File - Reading \"%s\".\n", Filename);

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
 *  Main function for patching the hooks.
 */
void RawFileClassExtension_Hooks()
{
    Patch_Jump(0x005BE560, &RawFileClassFake::_Read);
}
