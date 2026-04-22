/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended ThemeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension.h"
#include "hooker.h"
#include "syringe.h"
#include "theme.h"
#include "themeext.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x006439E5, _ThemeClass_ThemeControl_Constructor_Patch, 1)
{
    GET(ThemeClass::ThemeControl *, this_ptr, EAX); // "this" pointer.

    /**
     *  Create an extended class instance.
     */
    Extension::List::Make<ThemeClass::ThemeControl, ThemeControlExtension>(this_ptr, ThemeControlExtensions);

original_code:
    return 0;
}


/**
 *  This patch replaces an inlined copy of the constructor with a call to the constructor.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00643B45, _ThemeClass_ThemeControl_Inlined_Constructor_Patch, 0)
{
    GET(ThemeClass::ThemeControl*, this_ptr, EAX);

    new (this_ptr) ThemeClass::ThemeControl;

    R->EBP(this_ptr);

    return 0x00643B7C;
}


/**
 *  Patch for reading the extended class members from the ini instance.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00643AAB, _ThemeClass_ThemeControl_Fill_In_Patch, 7)
{
    GET(ThemeClass::ThemeControl*, this_ptr, ESI);
    GET(CCINIClass*, ini, EDI);

    /**
     *  Find the extension instance.
     */
    auto exttype_ptr = Extension::List::Fetch<ThemeClass::ThemeControl, ThemeControlExtension>(this_ptr, ThemeControlExtensions);

    /**
     *  Read type class ini.
     */
    exttype_ptr->Read_INI(*ini);

original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void ThemeClassExtension_Init()
{

}
