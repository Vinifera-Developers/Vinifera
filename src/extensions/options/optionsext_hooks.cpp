/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended OptionsClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "optionsext_hooks.h"
#include "optionsext_init.h"
#include "optionsext.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "hooker.h"
#include "optionsext_init.h"
#include "rawfile.h"
#include "syringe.h"
#include "audio_voc.h"

/**
 *  Patches Hotkey_Dialog_Proc to use RawFileClass when deleting Keyboard.INI to ensure only
 *  the file in the game's root directory is deleted.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0058AA18, _Hotkey_Dialog_Proc_Keyboard_INI_RawFileClass_Patch, 0)
{
    RawFileClass keyboard_ini("Keyboard.ini");
    keyboard_ini.Delete();
    return 0x0058AA21;
}


/**
 *  x
 * 
 *  @author: CCHyper
 */
static int Options_SoundVolume_To_Int() { return Options.SoundVolume * 255; }
DECLARE_PATCH(_OptionsClass_Set_Sound_Volume_Patch)
{
    AudioVocClass::Set_Volume(Options_SoundVolume_To_Int());

    /**
     *  Stolen bytes/code.
     */
    _asm { pop esi }
    _asm { ret 8 }
}


/**
 *  Main function for patching the hooks.
 */
void OptionsClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    OptionsClassExtension_Init();

    Patch_Jump(0x00589B68, &_OptionsClass_Set_Sound_Volume_Patch);
}
