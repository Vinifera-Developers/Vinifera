/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended
 *          SuperWeaponTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension.h"
#include "hooker.h"
#include "supertype.h"
#include "supertypeext.h"
#include "syringe.h"
#include "vinifera_globals.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0060D04A, _SuperWeaponTypeClass_Constructor_Patch, 5)
{
    GET(SuperWeaponTypeClass *, this_ptr, EBP); // "this" pointer.

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
    Extension::Make<SuperWeaponTypeClassExtension>(this_ptr);

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
DEFINE_HOOK(0x0060D0EA, _SuperWeaponTypeClass_Destructor_Patch, 0)
{
    GET(SuperWeaponTypeClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<SuperWeaponTypeClassExtension>(this_ptr);

original_code:
    this_ptr->AbstractTypeClass::~AbstractTypeClass();
    return 0x0060D0F1;
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0060D87A, _SuperWeaponTypeClass_Scalar_Destructor_Patch, 0)
{
    GET(SuperWeaponTypeClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<SuperWeaponTypeClassExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    this_ptr->AbstractTypeClass::~AbstractTypeClass();
    return 0x0060D881;
}


/**
 *  Main function for patching the hooks.
 */
void SuperWeaponTypeClassExtension_Init()
{

}
