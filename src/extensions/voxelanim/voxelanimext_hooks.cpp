/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended VoxelAnimClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "voxelanimext_hooks.h"

#include "asserthandler.h"
#include "debughandler.h"
#include "extension.h"
#include "hooker.h"
#include "voc.h"
#include "voxelanim.h"
#include "voxelanimtype.h"
#include "voxelanimtypeext.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(VoxelAnimClass)
{
    public:
        void _Delete_Me();
};


/**
 *  Implementation of Delete_Me() for VoxelAnimClass.
 */
void VoxelAnimClassExt::_Delete_Me()
{
    /**
     *  #issue-474
     * 
     *  Implements StopSound for VoxelAnimTypes.
     * 
     *  @author: CCHyper
     */
    VoxelAnimTypeClassExtension* voxelanimtypeext = Extension::Fetch(Class);
    if (voxelanimtypeext) {

        /**
         *  Play the StopSound if one has been defined.
         */
        if (voxelanimtypeext->StopSound != VOC_NONE) {
            Static_Sound(voxelanimtypeext->StopSound, Center_Coord());
        }
    }

    ObjectClass::Delete_Me();
}


/**
 *  Main function for patching the hooks.
 */
void VoxelAnimClassExtension_Hooks()
{
    Change_Virtual_Address(0x006D9134, Get_Func_Address(&VoxelAnimClassExt::_Delete_Me));
}
