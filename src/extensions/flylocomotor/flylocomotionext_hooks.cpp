/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended FlyLocomotionClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "anim.h"
#include "extension.h"
#include "flylocomotion.h"
#include "foot.h"
#include "hooker.h"
#include "house.h"
#include "object.h"
#include "rules.h"
#include "tibsun_globals.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(FlyLocomotionClass)
{
public: 
    int STDMETHODCALLTYPE _Apparent_Speed(void);
};

int STDMETHODCALLTYPE FlyLocomotionClassExt::_Apparent_Speed(void)
{
    // 'this' is currently shifted by 4 bytes due to the engine's vtable optimization.
    // We cast to char* to do byte-level arithmetic, subtract 4, and cast back.
    FlyLocomotionClass* trueThis = reinterpret_cast<FlyLocomotionClass*>(reinterpret_cast<char*>(this) - 4);
    FootClass* foot = trueThis->LinkedTo;

    int speed = foot->Get_Max_Speed() * foot->House->AirspeedBias * foot->SpeedBias;
    if (foot->Has_Ability(ABILITY_FASTER)) {
        speed *= Rule->VeteranSpeed + 1.0;
    }

    return speed * trueThis->CurrentSpeed;
}


/**
 *  Main function for patching the hooks.
 */
void FlyLocomotionClassExtension_Hooks() 
{
    Patch_Jump(0x0049CEC0, &FlyLocomotionClassExt::_Apparent_Speed);
}
