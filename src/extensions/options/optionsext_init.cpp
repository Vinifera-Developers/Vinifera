/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended OptionsClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension.h"
#include "extension_globals.h"
#include "hooker.h"
#include "options.h"
#include "optionsext.h"
#include "syringe.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00589A12, _OptionsClass_Constructor_Patch, 1)
{
    GET(OptionsClass *, this_ptr, EAX); // "this" pointer.

    /**
     *  The OptionsClass constructor is actually called twice as there are
     *  are two instances; The Options and another temporary one for the
     *  display options. So we handle this by skipping that second call.
     */
    if (OptionsExtension) {
        goto original_code;
    }

    /**
     *  Create the extended class instance.
     */
    OptionsExtension = Extension::Singleton::Make<OptionsClass, OptionsClassExtension>(this_ptr);

original_code:

    /**
     *  #issue-244
     * 
     *  Changes the default value of "AllowHiResModes" to "true".
     * 
     *  @author: CCHyper
     */
    this_ptr->AllowHiResModes = true;

    /**
     *  #issue-212
     * 
     *  Changes the default value of "IsScoreShuffle" to "true".
     * 
     *  @author: CCHyper
     */
    this_ptr->IsScoreShuffle = true;

    return 0;
}


/**
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0058A132, _OptionsClass_Load_Settings_Patch, 2)
{
    /**
     *  Load ini.
     */
    OptionsExtension->Load_Settings();

original_code:
    return 0;
}


/**
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0060127E, _WinMain_Load_Init_Options_Settings_Patch, 5)
{
    /**
     *  Load ini.
     */
    OptionsExtension->Load_Init_Settings();

original_code:
    return 0;
}


/**
 *  Patch for saving the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0058A3C3, _OptionsClass_Save_Settings_Patch, 5)
{
    /**
     *  Save ini.
     */
    OptionsExtension->Save_Settings();

original_code:
    return 0;
}


/**
 *  Patch for saving the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0058A5E6, _OptionsClass_Set_Patch, 6)
{
    /**
     *  Set options.
     */
    OptionsExtension->Set();

original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void OptionsClassExtension_Init()
{

}
