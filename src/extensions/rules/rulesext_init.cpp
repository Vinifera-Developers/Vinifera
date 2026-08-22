/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended RulesClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension.h"
#include "extension_globals.h"
#include "hooker.h"
#include "rules.h"
#include "rulesext.h"
#include "syringe.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005C59A1, _RulesClass_Constructor_Patch, 5)
{
    GET(RulesClass *, this_ptr, ESI); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    RuleExtension = Extension::Singleton::Make<RulesClass, RulesClassExtension>(this_ptr);

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
DEFINE_HOOK(0x005C6120, _RulesClass_Destructor_Patch, 5)
{
    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<RulesClass, RulesClassExtension>(RuleExtension);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members when processing rules data.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005C6A4D, _RulesClass_Process_Patch, 7)
{
    GET(CCINIClass*, ini, ESI);

    RuleExtension->Process(*ini);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members when processing the MPlayer section.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005CC3BF, _RulesClass_MPlayer_Patch, 7)
{
    GET(CCINIClass*, ini, EDI);

    RuleExtension->MPlayer(*ini);

original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void RulesClassExtension_Init()
{

}
