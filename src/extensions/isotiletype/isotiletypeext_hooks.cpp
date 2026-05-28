/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended IsometricTileTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "isotiletypeext_hooks.h"

#include "asserthandler.h"
#include "debughandler.h"
#include "extension.h"
#include "hooker.h"
#include "isotiletype.h"
#include "isotiletypeext.h"
#include "isotiletypeext_init.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(IsometricTileTypeClass)
{
public:
    const ShapeSet * _Get_Image_Data();
};


/**
 *  Reimplementation of IsometricTileTypeClass::Get_Image_Data with added assertion.
 * 
 *  @author: CCHyper
 */
const ShapeSet * IsometricTileTypeClassExt::_Get_Image_Data()
{
    if (Image) {
        return Image;
    }

    if (IsFileLoaded) {
        Load_Image_Data();
    }
    
    if (Image == nullptr) {
        DEBUG_WARNING("IsoTile {} has NULL image data!\n", Name());
    }

    return Image;
}


/**
 *  Main function for patching the hooks.
 */
void IsometricTileTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    IsometricTileTypeClassExtension_Init();

    Patch_Jump(0x004F3570, &IsometricTileTypeClassExt::_Get_Image_Data);
}
