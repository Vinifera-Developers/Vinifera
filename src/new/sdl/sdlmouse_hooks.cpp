/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the SDLMouse class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "dsurface.h"
#include "hooker.h"
#include "sdlmouse.h"
#include "sdlsurface.h"
#include "wwmouse.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class WWMouseClassExt : public WWMouseClass
{
public:
    SDLMouseClass* CTOR_Proxy(Surface*, HWND);
};


/**
 *  A function imitating a constructor because we can't take the address of a constructor.
 *
 *  @author: ZivDero
 */
SDLMouseClass* WWMouseClassExt::CTOR_Proxy(Surface*, HWND)
{
    return new (reinterpret_cast<SDLMouseClass*>(this)) SDLMouseClass;
}


/**
 *  Main function for patching the hooks.
 */
void SDLMouse_Hooks()
{
    Patch_Byte(0x00601857, sizeof(SDLMouseClass));
    Patch_Call(0x00601877, &WWMouseClassExt::CTOR_Proxy);
}
