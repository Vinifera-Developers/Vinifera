/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended TeamTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension.h"
#include "hooker.h"
#include "syringe.h"
#include "teamtype.h"
#include "teamtypeext.h"
#include "vinifera_globals.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00627ED4, _TeamTypeClass_Constructor_Patch, 5)
{
    GET(TeamTypeClass *, this_ptr, ESI); // "this" pointer.

    /**
     *  If we are performing a load operation, the Windows API will invoke the
     *  constructors for us as part of the operation, so we can skip our hook here.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }

    /**
     *  Create an extended class instance.
     */
    Extension::Make<TeamTypeClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00627EF8, _TeamTypeClass_Destructor_Patch, 6)
{
    GET(TeamTypeClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<TeamTypeClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00629298, _TeamTypeClass_Scalar_Destructor_Patch, 6)
{
    GET(TeamTypeClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<TeamTypeClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void TeamTypeClassExtension_Init()
{

}
