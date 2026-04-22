/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for Fetch_Resource and Fetch_String.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "fetchres_hooks.h"

#include "fetchres.h"
#include "hooker.h"
#include "vinifera_util.h"


extern HMODULE DLLInstance;


/**
 *  This intercept allows us to override default dialogs using a new id, while
 *  allowing the new dialogs to be overridden by LANGUAGE.DLL.
 * 
 *  @author: CCHyper
 */
static HGLOBAL Fetch_Resource_Intercept(const char *id, const char *type)
{
    HGLOBAL hGlobal = Fetch_Resource(id, type);
    if (hGlobal) {
        return hGlobal;
    }

    if (DLLInstance) {
        HGLOBAL v_hGlobal = FETCH_RESOURCE(DLLInstance, id, type);
        if (v_hGlobal) {
            return v_hGlobal;
        }
    }

    return nullptr;
}

 
/**
 *  Main function for patching the hooks.
 */
void FetchRes_Hooks()
{
    Patch_Call(0x005A0645, &Fetch_Resource_Intercept);
    Patch_Call(0x005A0C75, &Fetch_Resource_Intercept);
    Patch_Call(0x0068301B, &Fetch_Resource_Intercept);
}
