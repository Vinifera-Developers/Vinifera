/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended message input function.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "msglistext_hooks.h"

#include "asserthandler.h"
#include "hooker.h"


/**
 *  Main function for patching the hooks.
 */
void MessageListClassExtension_Hooks()
{
    // Replace the message format to add a space after the semicolon after the message author's name.
    Patch_Dword(0x00573161 + 1, reinterpret_cast<uintptr_t>(&"%s: %s"));
}
