/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended AbstractClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "abstractext_init.h"

#include "abstract.h"
#include "asserthandler.h"
#include "extension.h"
#include "hooker.h"
#include "syringe.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(AbstractClass)
{
public:
    IFACEMETHOD_(LONG, IsDirty)();
};


/**
 *  This patch forces AbstractClass::IsDirty() to return true.
 * 
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE AbstractClassExt::IsDirty()
{
    return TRUE;
}


/**
 *  This patch clears the DWORD at 0x10 (0x10 is "bool IsDirty") to use the space
 *  for storing a pointer to the extension class instance for this AbstractClass.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00405B61, _AbstractClass_Constructor_Extension, 6)
{
    GET(AbstractClass*, this_ptr, EAX);

    // IsDirty, now reused as an extension pointer, so we need to clear the whole DWORD.
    reinterpret_cast<int&>(this_ptr->Dirty) = 0;

    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void AbstractClassExtension_Init()
{
    Patch_Jump(0x00405E00, &AbstractClassExt::Is_Dirty);

    /**
     *  Removes the branch from AbstractClass::Internal_Save which clears IsDirty.
     */
    Patch_Byte_Range(0x00405CF8, 0x90, 12);
}
