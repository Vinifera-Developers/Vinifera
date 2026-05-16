/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended ScenarioClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension.h"
#include "extension_globals.h"
#include "hooker.h"
#include "scenario.h"
#include "scenarioext.h"
#include "syringe.h"
#include "tibsun_globals.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005DADDE, _ScenarioClass_Constructor_Patch, 9)
{
    GET(ScenarioClass *, this_ptr, EBP); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    ScenExtension = Extension::Singleton::Make<ScenarioClass, ScenarioClassExtension>(this_ptr);

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
DEFINE_HOOK(0x006023CC, _ScenarioClass_Destructor_Patch, 6)
{
    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<ScenarioClass, ScenarioClassExtension>(ScenExtension);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members when initialsing the scenario data.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x005DB166, _ScenarioClass_Init_Clear_Patch, 7)
{
    /**
     *  This is a odd case; ScenarioClass::Init_Clear is called within the class
     *  constructor, so the first time this patch is called, ScenExtension is NULL.
     *  The ScenarioClassExtension calls it's Init_Clear to mirror this behaviour
     *  so we can just check if the extension has be created first to catch this.
     */
    if (ScenExtension) {
        ScenExtension->Init_Clear();
    }

original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void ScenarioClassExtension_Init()
{

}
