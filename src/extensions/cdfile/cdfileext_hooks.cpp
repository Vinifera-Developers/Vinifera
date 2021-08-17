/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CDFILEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended CDFileClass.
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
#include "cdfileext_hooks.h"
#include "cdfile.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class CDFileClassFake : public CDFileClass
{
    public:
        const char * _Set_Name(const char *filename);
        bool _Open(const char *filename, FileAccessType rights);
};


/**
 *  Performs a multiple directory scan to set the filename.
 * 
 *  @author: 10/18/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Vinifera.
 */
const char * CDFileClassFake::_Set_Name(const char *filename)
{
    /**
     *  Try to find the file in the current directory first. If it can be found, then
     *  just return with the normal file name setting process. Do the same if there is
     *  no multi-drive search path.
     */
    BufferIOFileClass::Set_Name(filename);
    if (IsDisabled || !First || BufferIOFileClass::Is_Available()) {
        return File_Name();
    }

    /**
     *  Attempt to find the file first. Check the current directory. If not found there, then
     *  search all the path specifications available. If it still can't be found, then just
     *  fall into the normal raw file filename setting system.
     */
    SearchDriveType * srch = First;

    while (srch) {
        char path[_MAX_PATH];

        /**
         *  Build a pathname to search for.
         */
        std::strcpy(path, srch->Path);
        std::strcat(path, filename);

        /**
         *  Check to see if the file could be found. The low level Is_Available logic will
         *  prompt if necessary when the CD-ROM drive has been removed. In all other cases,
         *  it will return false and the search process will continue.
         */
        BufferIOFileClass::Set_Name(path);
        if (BufferIOFileClass::Is_Available()) {
            DEV_DEBUG_INFO("CDFileClass::Set_Name - \"%s\" Found in: %s\n", filename, path);
            return File_Name();
        }

        /**
         *  It wasn't found, so try the next path entry.
         */
        srch = (SearchDriveType *)srch->Next;
    }

    /**
     *  At this point, all path searching has failed. Just set the file name to the
     *  plain text passed to this routine and be done with it.
     */
    BufferIOFileClass::Set_Name(filename);
    return File_Name();
}


/**
 *  Opens the file wherever it can be found.
 * 
 *  @author: 10/18/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Vinifera.
 */
bool CDFileClassFake::_Open(const char *filename, FileAccessType rights)
{
    CDFileClass::Close();

    /**
     *  Verify that there is a filename associated with this file object. If not, then this is a
     *  big error condition.
     */
    if (!filename) {
        Error(FILE_ERROR_NOENT, false);
    }

    /**
     *  If writing is requested, then multiple drive searching is not performed.
     */
    if (IsDisabled || rights == FILE_ACCESS_WRITE) {
        BufferIOFileClass::Set_Name(filename);
        return BufferIOFileClass::Open(rights);
    }

    /**
     *  Perform normal multiple drive searching for the filename and open
     *  using the normal procedure.
     */
    Set_Name(filename);
    if (BufferIOFileClass::Open(rights)) {
        DEV_DEBUG_INFO("CDFileClass::Open - Opened: %s\n", Filename);
        return true;
    }

    return false;
}


/**
 *  Main function for patching the hooks.
 */
void CDFileClassExtension_Hooks()
{
    Patch_Jump(0x00450AD0, &CDFileClassFake::_Open);
    Patch_Jump(0x004509D0, &CDFileClassFake::_Set_Name);
}
