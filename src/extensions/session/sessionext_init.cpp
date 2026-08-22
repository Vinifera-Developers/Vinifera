/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended SessionClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension.h"
#include "extension_globals.h"
#include "hooker.h"
#include "sessionext.h"
#include "syringe.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005ED1AA, _SessionClass_Constructor_Patch, 5)
{
    GET(SessionClass *, this_ptr, EBP); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    SessionExtension = Extension::Singleton::Make<SessionClass, SessionClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005ED465, _SessionClass_Destructor_Patch, 3)
{
    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<SessionClass, SessionClassExtension>(SessionExtension);

original_code:
    return 0;
}

/**
 *  Main function for patching the hooks.
 */
void SessionClassExtension_Init()
{

}
