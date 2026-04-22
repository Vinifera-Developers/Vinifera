/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended CampaignClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "campaign.h"
#include "campaignext.h"
#include "extension.h"
#include "hooker.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00448AC4, _CampaignClass_Constructor_Patch, 6)
{
    GET(CampaignClass *, this_ptr, EBP); // "this" pointer.

    // Campaign's are not saved to file, so this case is not required.
#if 0
    /**
     *  If we are performing a load operation, the Windows API will invoke the
     *  constructors for us as part of the operation, so we can skip our hook here.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }
#endif

    /**
     *  Create an extended class instance.
     */
    Extension::Make<CampaignClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for removing the inlined constructor and replacing it with a direct call.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00448CD0, _CampaignClass_Process_Patch, 0)
{
    GET(CampaignClass*, this_ptr, EBP);
    LEA_STACK(char const*, ini_name, 0x18);

    new (this_ptr) CampaignClass(ini_name);

    R->ECX(this_ptr);

    return 0x00448D86;
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00448EF8, _CampaignClass_Scalar_Destructor_Patch, 6)
{
    GET(CampaignClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<CampaignClassExtension>(this_ptr);

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
DEFINE_HOOK(0x00448C1E, _CampaignClass_Read_INI_Patch, 5)
{
    GET(CampaignClass *, this_ptr, ESI);
    GET(CCINIClass *, ini, EBX);

    /**
     *  Fetch the extension instance.
     */
    auto exttype_ptr = Extension::Fetch(this_ptr);

    /**
     *  Read type class ini.
     */
    exttype_ptr->Read_INI(*ini);

    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void CampaignClassExtension_Init()
{

}
