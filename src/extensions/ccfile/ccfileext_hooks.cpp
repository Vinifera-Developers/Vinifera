/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CCFILEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended CCFileClass.
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
#include "ccfileext_hooks.h"
#include "ccfile.h"
#include "cd.h"
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
class CCFileClassFake final : public CCFileClass
{
    public:
        void _Error(FileErrorType error, bool can_retry = false, const char *filename = nullptr);
};


/**
 *  Handles displaying a file error message. 
 * 
 *  @author: 10/17/1994 JLB - Red Alert source code.
 *           CCHyper - Adjustments for Tiberian Sun, minor bug fix.
 */
void CCFileClassFake::_Error(FileErrorType error, bool can_retry, const char *filename)
{
    /**
     *  File system is failled as local, no need to check if required cd is available.
     */
    if (CD::IsFilesLocal) {
        CDFileClass::Error(error, can_retry, filename);

    } else {

        /**
         *  If the file was not found, its possible we have the wrong disk inserted
         *  so prompt the user to insert the correct disk.
         */
        if (!CD().Is_Available(CD::RequiredCD)) {

            DEV_DEBUG_ERROR("File - Error, CD '%d' not found!", CD::RequiredCD);

            /**
             *  If still not available, now let the low level file interface report the error.
             */
            CDFileClass::Error(error, can_retry, filename);
        }

    }
}



/**
 *  Main function for patching the hooks.
 */
void CCFileClassExtension_Hooks()
{
    Patch_Jump(0x00449820, &CCFileClassFake::_Error);
}
