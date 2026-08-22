/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended SuperClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "super.h"
#include "superext.h"
#include "syringe.h"
#include "vinifera_globals.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0060B352, _SuperClass_Default_Constructor_Patch, 4)
{
    GET(SuperClass *, this_ptr, ESI); // Current "this" pointer.

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
    Extension::Make<SuperClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0060B4AB, _SuperClass_Constructor_Patch, 7)
{
    GET(SuperClass *, this_ptr, ESI); // Current "this" pointer.

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
    Extension::Make<SuperClassExtension>(this_ptr);

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
DEFINE_HOOK(0x0060B51A, _SuperClass_Destructor_Patch, 6)
{
    GET(SuperClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<SuperClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0060CC2A, _SuperClass_Scalar_Destructor_Patch, 6)
{
    GET(SuperClass *, this_ptr, ESI);

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<SuperClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  This patch fixes the incorrect constructor being used in SuperClass::Load, so
 *  this is technically a bug fix, while also allowing the extended class to operate
 *  correctly.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SuperClass_Load_Patch)
{
    _asm { lea ecx, [esp+0x0C] }
    _asm { push ecx }
    _asm { mov ecx, esi }
    _asm { mov eax, 0x00405B70 } // AbstractClass::AbstractClass(const NoInitClass &)
    _asm { call eax }

    JMP(0x0060C7AF);
}


/**
 *  Main function for patching the hooks.
 */
void SuperClassExtension_Init()
{
    Patch_Jump(0x0060C7A8, &_SuperClass_Load_Patch);
}
