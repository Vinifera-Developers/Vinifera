/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended CCFileClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "ccfileext_hooks.h"

#include "ccfile.h"
#include "cd.h"
#include "debughandler.h"
#include "hooker.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class CCFileClassExt : public CCFileClass
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
void CCFileClassExt::_Error(FileErrorType error, bool can_retry, const char *filename)
{
    /**
     *  File system is failled as local, no need to check if required cd is available.
     */
    if (CD::IsOverrideSwap()) {
        CDFileClass::Error(error, can_retry, filename);

    } else {

        /**
         *  If the file was not found, its possible we have the wrong disk inserted
         *  so prompt the user to insert the correct disk.
         */
        if (!CD::ForceAvailable()) {

            DEV_DEBUG_ERROR("File - Error, CD '{}' not found!", (int)CD::RequiredCD);

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
    Patch_Jump(0x00449820, &CCFileClassExt::_Error);
}
