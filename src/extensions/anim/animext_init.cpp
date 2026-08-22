/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended AnimClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "anim.h"
#include "animext.h"
#include "animtype.h"
#include "animtypeext.h"
#include "debughandler.h"
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
DEFINE_HOOK(0x00413C79, _AnimClass_Constructor_Patch, 7)
{
    GET(AnimClass *, this_ptr, ESI); // Current "this" pointer.
    AnimClassExtension *ext_ptr = nullptr;

    /**
     *  If we are performing a load operation, the Windows API will invoke the
     *  constructors for us as part of the operation, so we can skip our hook here.
     */
    if (Vinifera_PerformingLoad) {
        goto original_code;
    }

    /**
     *  This following code was moved here due to a patching address conflict
     *  in animext_hooks.cpp and is not the normal approach when extending
     *  a game class. - CCHyper
     */

    /**
     *  #BUGFIX:
     * 
     *  This check was observed in Red Alert 2, so there must be an edge case
     *  where anims are created with a null type instance. So lets do that
     *  here and also report a warning to the debug log.
     */
    if (!this_ptr->Class) {
        goto destroy_anim;
    }

    /**
     *  Create an extended class instance.
     */
    ext_ptr = Extension::Make<AnimClassExtension>(this_ptr);

    /**
     *  In multiplayer, the move flash anim is transferred out of the Anims heap
     *  into the local-only MoveFlashes list right after construction (see
     *  FootClass::Active_Click_With), identified by an ID of -2. Keep the
     *  extension attached to the anim (hooks fetch it through the object), but
     *  keep it out of AnimExtensions so the list always mirrors the Anims heap.
     */
    if (ext_ptr != nullptr && this_ptr->Fetch_ID() == -2) {
        AnimExtensions.Delete(ext_ptr);
    }

    /**
     *  #issue-561
     *
     *  Implements ZAdjust override for Anims. This will only have an effect
     *  if the anim is created with a z-adjustment value of "0" (default value).
     * 
     *  @author: CCHyper
     */
    if (!this_ptr->ZAdjust) {
        this_ptr->ZAdjust = Extension::Fetch(this_ptr->Class)->ZAdjust;
    }

original_code:
    return 0;

    /**
     *  Report that the anim type instance was invalid.
     */
destroy_anim:
    DEBUG_WARNING("Anim: Invalid anim type instance!\n");

    /**
     *  Remove the anim from the game world.
     */
    this_ptr->Delete_Me();
    
    return 0x00414157;
}


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004142A6, _AnimClass_Default_Constructor_Patch, 5)
{
    GET(AnimClass *, this_ptr, ESI); // Current "this" pointer.

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
    Extension::Make<AnimClassExtension>(this_ptr);

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
DEFINE_HOOK(0x0041436D, _AnimClass_Destructor_Patch, 6)
{
    GET(AnimClass *, this_ptr, ESI);

    /**
     *  If this anim instance was destoryed because it has a NULL class type, then
     *  it would not have created an extension instance, so we can skip the destroy
     *  call here.
     */
    if (!this_ptr->Class) {
        goto original_code;
    }

    /**
     *  Remove the extended class from the global index.
     */
    Extension::Destroy<AnimClassExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void AnimClassExtension_Init()
{
    Patch_Jump(0x0041441F, 0x00414475); // This jump goes from duplicate code in the destructor to our patch, removing the need for two hooks.
    Patch_Jump(0x00413C89, 0x00413D3E); // Skip part of the AnimClass constructor that's re-implemented in the extension constructor.
}
