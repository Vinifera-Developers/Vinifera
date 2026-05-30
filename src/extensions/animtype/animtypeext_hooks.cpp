/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended AnimTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "animtypeext_hooks.h"

#include "animtype.h"
#include "animtypeext.h"
#include "animtypeext_init.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "extension.h"
#include "hooker.h"
#include "supertype.h"
#include "syringe.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(AnimTypeClass)
{
public:
    void _Load_Image(TheaterType theater);
};


/**
 *  Reimplementation of AnimTypeClass::Load_Image.
 *
 *  @author: ZivDero
 */
void AnimTypeClassExt::_Load_Image(TheaterType theater)
{
    if (!IsDemandLoad && Image == nullptr) {
        if (IsTheater) {
            Fetch_Normal_Image();
        } else {
            char fullname[_MAX_FNAME + _MAX_EXT];
            _makepath(fullname, nullptr, nullptr, Graphic_Name(), ".SHP");
            Theater_Naming_Convention(fullname, theater);
            Image = static_cast<ShapeSet const*>(MixFileClass::Retrieve(fullname));
        }
    }

    /**
     *  The game would calculate Stages and LoopEnd now, set them to -1
     *  instead to be calcalated in AnimClass::AI.
     */
    if (Stages == 0) {
        Stages = -1;
    }
    if (LoopEnd == 0) {
        LoopEnd = -1;
    }

    /**
     *  No longer important as we use the MiddleFrames type list now.
     */
    Biggest = -1;
}


/**
 *  Main function for patching the hooks.
 */
void AnimTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    AnimTypeClassExtension_Init();

    Patch_Jump(0x00418A70, &AnimTypeClassExt::_Load_Image);
}
