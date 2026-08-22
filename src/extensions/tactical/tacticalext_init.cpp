/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for initialising the extended Tactical class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "extension.h"
#include "hooker.h"
#include "syringe.h"
#include "tactical.h"
#include "tacticalext.h"
#include "vinifera_globals.h"


/**
 *  Patch for including the extended class members in the creation process.
 *
 *  @warning: Do not touch this unless you know what you are doing!
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0060F08A, _Tactical_Constructor_Patch, 5)
{
    GET(Tactical *, this_ptr, ESI); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    TacticalMapExtension = Extension::Singleton::Make<Tactical, TacticalExtension>(this_ptr);

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper, Rampastring
 */
DEFINE_HOOK(0x0060F0DD, _Tactical_Destructor_Patch, 10)
{
    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<Tactical, TacticalExtension>(TacticalMapExtension);
    TacticalMapExtension = nullptr;

original_code:
    return 0;
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper, Rampastring
 */
DEFINE_HOOK(0x00618020, _Tactical_Scalar_Destructor_Patch, 10)
{
    GET(Tactical *, this_ptr, ESI);

    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<Tactical, TacticalExtension>(TacticalMapExtension);
    TacticalMapExtension = nullptr;

original_code:
    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void TacticalExtension_Init()
{

}
