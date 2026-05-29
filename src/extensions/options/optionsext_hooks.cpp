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

#include "audio_voc.h"
#include "ccini.h"
#include "hooker.h"
#include "optionsext.h"
#include "optionsext_init.h"
#include "rawfile.h"
#include "syringe.h"

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
 *  Sets the volume for the sound audio groups.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00589B68, _OptionsClass_Set_Sound_Volume_Patch, 4)
{
    AudioVocClass::Set_Volume(static_cast<int>(Options.SoundVolume * 255));

    return 0;
}


/**
 *  Replace inlined calls setting the volume on settings load with function calls.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00589EFE, _OptionsClass_Load_Settings_Volume_Patch, 0)
{
    GET(OptionsClass*, this_ptr, ESI);

    this_ptr->Set_Sound_Volume(ConfigINI.Get_Float("Audio", "SoundVolume", this_ptr->SoundVolume), false);
    this_ptr->Set_Voice_Volume(ConfigINI.Get_Float("Audio", "VoiceVolume", this_ptr->VoiceVolume), false);
    this_ptr->Set_Score_Volume(ConfigINI.Get_Float("Audio", "ScoreVolume", this_ptr->ScoreVolume), false);

    R->ESP(R->ESP() - 0xC); // stack fixup

    return 0x00589FFB;
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

}
