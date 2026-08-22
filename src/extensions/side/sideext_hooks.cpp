/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended SideClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "sideext_hooks.h"

#include "sideext.h"
#include "sideext_init.h"

#include "extension.h"
#include "ownrdraw.h"
#include "rgb.h"
#include "syringe.h"


/**
 *  Patches OwnerDraw initialization to set menu color based on the player's side.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x00591491, _OwnerDraw_Set_Colors_Text_Color_Patch, 6)
{
    RGBClass rgb;

    if (PlayerPtr == nullptr) {
        rgb = OPTIONS_MENU_TEXT_DEFAULT_COLOR;
    } else {
        SideClassExtension* sideext = Extension::Fetch(Sides[PlayerPtr->Class->Side]);
        rgb = sideext->OptionsMenuTextColor;
    }

    OwnerDraw::TextColor1 = RGB(rgb.Get_Red(), rgb.Get_Green(), rgb.Get_Blue());

    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void SideClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    SideClassExtension_Init();
}
