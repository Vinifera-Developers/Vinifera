/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the new audio driver interface.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_hooks.h"

#include "addon.h"
#include "audio_ahandle.h"
#include "audio_voc_handle.h"
#include "audio_static_sound.h"
#include "audio_theme.h"
#include "audio_util.h"
#include "audio_voc.h"
#include "audio_vox.h"
#include "ccini.h"
#include "credits.h"
#include "debughandler.h"
#include "hooker.h"
#include "rules.h"
#include "stimer.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "voc.h"
#include "vox.h"


/**
 *  Ion Storm ambient track hooks.
 *
 *  The system now plays this track independent of the music so we can do some cool stuff with it!
 *
 *  @author: CCHyper, ZivDero
 */
DEFINE_HOOK(0x004ECA1E, _IonStormClass_Ion_Storm_Begin_Ambient_Patch1, 5)
{
    // Skip stopping the theme if the ion ambient sound is present
    if (IonAmbient::Is_Available()) {
        R->Pop();
        return 0x004ECA28;
    }
    return 0;
}

DEFINE_HOOK(0x004ECC13, _IonStormClass_Ion_Storm_Begin_Ambient_Patch2, 5)
{
    // Play the ion ambient sound instead of the theme if it's available
    if (IonAmbient::Is_Available()) {
        IonAmbient::Start();
        return 0x004ECC2D;
    }
    return 0;
}

DEFINE_HOOK(0x004ED2B8, _IonStormClass_Ion_Storm_AI_Ambient_Patch, 6)
{
    // If there is an active ion storm but the ambient isn't playing
    // (e.g. we've loaded a save), start playing it.
    if (IonAmbient::Is_Available() && !IonAmbient::Is_Playing()) {
        IonAmbient::Start();
    }
    return 0;
}

DEFINE_HOOK(0x004ECE7F, _IonStormClass_Ion_Storm_End_Ambient_Patch, 6)
{
    // Stop the ambient sound now. Always take this path when the ambient is
    // available (mirroring the Begin patch), so the music volume duck is
    // restored even if the ambient already stopped playing on its own.
    if (IonAmbient::Is_Available()) {
        IonAmbient::Stop();
        return 0x004ECE90;
    }
    return 0;
}


/**
 *  Handy macros for defining the audio driver patches.
 * 
 *  @author: CCHyper
 */
#define AUDIO_ENGINE_INIT_PATCH(address, label, ret_addr) \
    DEFINE_HOOK(address, label, 0) \
    { \
        AudioManager.Init(MainWindow); \
        return ret_addr; \
    }

#define AUDIO_ENGINE_END_PATCH(address, label, ret_addr) \
    DEFINE_HOOK(address, label, 0) \
    { \
        AudioManager.End(); \
        return ret_addr; \
    }

#define AUDIO_ENGINE_IS_AVAILABLE_PATCH(address, label, return_false_addr, return_true_addr) \
    DEFINE_HOOK(address, label, 0) \
    { \
        return AudioManager.Is_Available() ? return_true_addr : return_false_addr; \
    }

#define AUDIO_ENGINE_FOCUS_LOSS_PATCH(address, label, ret_addr) \
    DEFINE_HOOK(address, label, 0) \
    { \
        AudioManager.Focus_Loss(); \
        return ret_addr; \
    }

#define AUDIO_ENGINE_FOCUS_RESTORE_PATCH(address, label, ret_addr) \
    DEFINE_HOOK(address, label, 0) \
    { \
        AudioManager.Focus_Restore(); \
        return ret_addr; \
    }

#define AUDIO_ENGINE_SOUND_CALLBACK_PATCH(address, label, ret_addr) \
    DEFINE_HOOK(address, label, 0) \
    { \
        AudioManager.Sound_Callback(); \
        return ret_addr; \
    }


/**
 *  The following definitions create intercept patches for various audio engine calls.
 */
AUDIO_ENGINE_INIT_PATCH(0x006013B0, _WinMain_Init_Audio_1_Patch, 0x006013C8);
AUDIO_ENGINE_INIT_PATCH(0x0060169B, _WinMain_Init_Audio_2_Patch, 0x006016B3);

AUDIO_ENGINE_END_PATCH(0x00601A12, _WinMain_End_Audio_Patch, 0x00601A1C);
AUDIO_ENGINE_END_PATCH(0x00602480, _Emergency_Exit_End_Audio_Patch, 0x0060248A);

AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x00462C6C, _Call_Back_Is_Available_Patch_1, 0x00462CA0, 0x00462C7E);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x004AAACB, _GameSettingsClass_Is_Available_Patch_1, 0x004AAAFB, 0x004AAADF);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x004E4615, _Init_Bulk_Data_Is_Available_Patch_1, 0x004E465F, 0x004E4626);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x00508ACD, _Main_Loop_Is_Available_Patch_1, 0x00508AF4, 0x00508ADF);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x0050A758, _MenuOptionsDialog_Is_Available_Patch_1, 0x0050A773, 0x0050A76C);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x005641A8, _MovieClass_Is_Available_Patch_1, 0x005641B4, 0x005641BE);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x0056E646, _MapSelect_Is_Available_Patch_1, 0x0056E7B8, 0x0056E65F);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x0056E99D, _MapChoice_Is_Available_Patch_1, 0x0056E9D3, 0x0056E9A8);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x0057146E, _MSEngine_Is_Available_Patch_1, 0x005714C5, 0x00571480);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x005721B7, _MSFont_Is_Available_Patch_1, 0x005722D7, 0x005721CF);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x00594261, _OwnerDraw_Is_Available_Patch_1, 0x00594295, 0x00594273);
DEFINE_HOOK(0x005FC53D, _SoundControlsClass_Is_Available_Patch_1, 0)
{
    R->Stack8(0x84, AudioManager.Is_Available());
    return 0x005FC55A;
}
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x00664A7E, _VocClass_Is_Available_Patch_1, 0x00664AEF, 0x00664A90);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x00664B28, _VocClass_Is_Available_Patch_2, 0x00664B93, 0x00664B3A);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x0067F77C, _WorldDominationTour_Is_Available_Patch_1, 0x0067F986, 0x0067F796);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x0067FA0C, _WorldDominationTour_Is_Available_Patch_2, 0x0067FBCE, 0x0067FA26);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x00685A12, _Focus_Restore_Is_Available_Patch_1, 0x00685A2B, 0x00685A1F);
AUDIO_ENGINE_IS_AVAILABLE_PATCH(0x006859A8, _Focus_Loss_Is_Available_Patch_1, 0x006859C4, 0x006859BA);
AUDIO_ENGINE_SOUND_CALLBACK_PATCH(0x00462C87, _Call_Back_Sound_Callback_Patch_1, 0x00462C91);
AUDIO_ENGINE_SOUND_CALLBACK_PATCH(0x0059427C, _OwnerDraw_Window_Procedure_Sound_Callback_Patch_1, 0x00594286);
AUDIO_ENGINE_SOUND_CALLBACK_PATCH(0x00643E65, _ThemeClass_AI_Sound_Callback_Patch_1, 0x00643E6F);
AUDIO_ENGINE_FOCUS_RESTORE_PATCH(0x00685A1F, _Focus_Restore_Start_Primary_Sound_Buffer_Patch_1, 0x00685A2B);
AUDIO_ENGINE_FOCUS_LOSS_PATCH(0x006859BA, _Focus_Loss_Stop_Primary_Sound_Buffer_Patch_1, 0x006859C4);

#if 0 // Sounds the same to me - ZivDero
/**
 *  Forces a playback limit on the credits tick sound, otherwise it runs wild, brother!
 * 
 *  @author: CCHyper
 */
constexpr int AUDIO_CREDIT_TICK_DELAY = 20;
DEFINE_HOOK(0x00471493, _CreditsClass_Graphic_Logic_Limit_Sfx_Patch, 0)
{
    GET(CreditClass *, this_ptr, EDI);
    static CDTimerClass<MSTimerClass> _delay = AUDIO_CREDIT_TICK_DELAY;

    if (_delay.Expired()) {
        if (this_ptr->IsUp) {
            Sound_Effect(Rule->CreditTicks[0], 0.5f);
        } else {
            Sound_Effect(Rule->CreditTicks[1], 0.5f);
        }
        _delay = AUDIO_CREDIT_TICK_DELAY;
        _delay.Start();
    }

    return 0x004714CC;
}
#endif


/**
 *  Patch to play the faction emblem sound effect when the owner-draw window is displayed.
 */
DEFINE_HOOK(0x00593DAC, _OwnerDraw_Window_Procedure_Play_EMBLEM_Patch, 0)
{
    /**
     *  Play the emblem sound at a reduced volume matching the original game's scaling.
     *  The sound effect volume is applied by the UI group, so it must not be folded in here.
     */
    Audio_Play_UI_Sample("EMBLEM", 10, 64);

    return 0x00593DF9;
}


/**
 *  Reads and processes sound definitions from SOUND.INI and SOUND01.INI.
 *
 *  @author: CCHyper
 */
static bool Read_Sound_Ini()
{
    CCFileClass file;
    CCINIClass ini;

    /**
     *  General note of warning here: TS Client packages removes SOUND.INI in
     *  favour of SOUND01.INI.
     */
    bool sound_ini_found = false;
    bool sound01_ini_found = false;

    AudioVocClass::Clear();

    DEBUG_INFO("Reading SOUND.INI\n");

    file.Set_Name("SOUND.INI");
    if (file.Is_Available()) {
        if (ini.Load(file, false)) {
            DEBUG_INFO("About to call VocClass::Process(SOUND.INI)...\n");
            AudioVocClass::Process(ini);
            sound_ini_found = true;
        } else {
            DEBUG_ERROR("Failed to load SOUND.INI!\n");
        }
    } else {
        DEBUG_ERROR("Failed to find SOUND.INI!\n");
    }

    if (Addon_Installed(ADDON_FIRESTORM)) {

        DEBUG_INFO("Reading SOUND01.INI\n");

        file.Set_Name("SOUND01.INI");
        if (file.Is_Available()) {
            if (ini.Load(file, false)) {
                DEBUG_INFO("About to call VocClass::Process(SOUND01.INI) as addition...\n");
                AudioVocClass::Process(ini);
                sound01_ini_found = true;
            } else {
                DEBUG_ERROR("Failed to load SOUND01.INI!\n");
            }
        } else {
            DEBUG_WARNING("Failed to find SOUND01.INI!\n");
        }
    }

    DEBUG_INFO("About to call VocClass::Scan()...\n");
    AudioVocClass::ScanAsync();

    return sound_ini_found || sound01_ini_found;
}

/**
 *  Reads and processes theme definitions from THEME.INI and THEME01.INI.
 *
 *  @author: CCHyper
 */
static bool Read_Theme_Ini()
{
    CCFileClass file;
    CCINIClass ini;

    DEBUG_INFO("Reading THEME.INI\n");

    file.Set_Name("THEME.INI");
    if (!file.Is_Available()) {
        DEBUG_ERROR("Failed to find THEME.INI!\n");
        return false;
    }
    if (!ini.Load(file, false)) {
        DEBUG_ERROR("Failed to load THEME.INI!\n");
        return false;
    }

    DEBUG_INFO("About to call Theme.Process(THEME.INI)...\n");
    AudioTheme.Free_Themes();
    AudioTheme.Process(ini);

    if (Addon_Installed(ADDON_FIRESTORM)) {

        DEBUG_INFO("Reading THEME01.INI\n");

        file.Set_Name("THEME01.INI");
        if (file.Is_Available()) {
            if (ini.Load(file, false)) {
                DEBUG_INFO("About to call Theme.Process(THEME01.INI) as addition...\n");
                AudioTheme.Process(ini);
            } else {
                DEBUG_ERROR("Failed to load THEME01.INI!\n");
            }
        } else {
            DEBUG_WARNING("Failed to find THEME01.INI!\n");
        }
    }

    DEBUG_INFO("About to call Theme.Scan()...\n");
    AudioTheme.Scan();

    return true;
}

/**
 *  Reads and processes speech definitions from EVA.INI.
 *
 *  @author: CCHyper
 */
static bool Read_Speech_Ini()
{
    /**
     *  Initialize the speech/voice audio subsystem from INI data.
     */
    DEBUG_INFO("Reading EVA.INI\n");
    CCFileClass file;
    CCINIClass ini;

    file.Set_Name("EVA.INI");
    if (file.Is_Available()) {
        ini.Load(file, false);
        if (!AudioVoxClass::Process(ini)) {
            DEV_DEBUG_WARNING("Failed to read EVA.INI!\n");
        }
    } else {
        DEV_DEBUG_WARNING("EVA.INI not found!\n");
    }

    /**
     *  If the user didn't provide a valid EVA.INI, create voxes from vanilla defaults.
     */
    if (Voxs.Count() <= 0) {
        DEBUG_INFO("About to call AudioVoxClass::One_Time...\n");
        AudioVoxClass::One_Time();
    }

    return true;
}


DEFINE_HOOK(0x004E133A, _Init_Game_Read_SOUND_THEME_INI_Patch, 6)
{
    GET(CCINIClass*, rules_ini, ESI);

    // stolen instruction
    RuleINI = rules_ini;

    /**
     *  Read the lists of houses types and sides from rules
     *  so that they are available when reading themes and speeches.
     */
    Rule->Houses(*RuleINI);
    Rule->Sides(*RuleINI);

    if (!Read_Sound_Ini()) {
        R->Pop();
        return 0x004E16AA;
    }
    if (!Read_Theme_Ini()) {
        R->Pop();
        return 0x004E16AA;
    }

    Read_Speech_Ini();

    return 0;
}

DEFINE_HOOK(0x005DC95F, _Clear_Static_Sounds_Patch, 5) // Do_Win
{
    Clear_Tracked_Static_Sounds();
    IonAmbient::Stop();
    return 0;
}
DEFINE_HOOK_AGAIN(0x005DCCAB, _Clear_Static_Sounds_Patch, 5); // Do_Lose
DEFINE_HOOK_AGAIN(0x005DCED4, _Clear_Static_Sounds_Patch, 5); // Do_Restart
DEFINE_HOOK_AGAIN(0x005DCFE8, _Clear_Static_Sounds_Patch, 5); // Do_Abort
DEFINE_HOOK_AGAIN(0x005059AF, _Clear_Static_Sounds_Patch, 7); // LoadOptionsClass::Load_File


/**
 *  Wrapper for the vanilla Speak function that plays hardcoded speeches by name.
 *
 *  #NOTE: Because of this, do NOT call vanilla Speak() if you want a speech by index.
 *  For this reason, we also re-implement the speech trigger action.
 *
 *  @author: ZivDero
 */
static void Speak_Wrapper(VoxType voice, bool now)
{
    if (voice >= VOX_FIRST && voice < std::size(EvaNames)) {
        AudioVoxClass::Speak(EvaNames[voice], now);
    }
}


/**
 *  Main function for patching the hooks.
 */
void Audio_Hooks()
{
    /**
     *  Remove the initialsation of the original DirectSound audio engine.
     */
    Patch_Byte(0x00487990, 0xC3); // patch "retn" in the dynamic initializer for DSAudio.
    Patch_Byte(0x004879B0, 0xC3); // patch "retn" in the dynamic initializer _atexit for DSAudio.

    /**
     *  Replace references to ThemeClass members with the new AudioThemeClass.
     */
    Patch_Dword(0x0044E877+2, (uintptr_t)&AudioTheme.Score); // CD::Insert_Disk
    Patch_Dword(0x0044E997+1, (uintptr_t)&AudioTheme.Score); // CD::Init_Swap
    Patch_Dword(0x004ECA19+1, (uintptr_t)&AudioTheme.Score); // IonStorm_Start
    Patch_Dword(0x00508ADF+2, (uintptr_t)&AudioTheme.Score); // Main_Loop
    Patch_Dword(0x005C0324+2, (uintptr_t)&AudioTheme.Score); // Restate_Mission
    Patch_Dword(0x005FC7E8+2, (uintptr_t)&AudioTheme.Score); // SoundControlsClass::Process
    Patch_Dword(0x00685994+1, (uintptr_t)&AudioTheme.Score); // Focus_Loss
    Patch_Dword(0x00687F2E+2, (uintptr_t)&AudioTheme.Score); // WOL
    Patch_Dword(0x00687F3D+2, (uintptr_t)&AudioTheme.Score); // WOL
    Patch_Dword(0x0068C09B+2, (uintptr_t)&AudioTheme.Score); // WOL start
    Patch_Dword(0x0068C0CE+2, (uintptr_t)&AudioTheme.Score); // WOL start

    Patch_Dword(0x00589A67+1, (uintptr_t)&AudioTheme.IsRepeat); // OptionsClass::Set_Repeat
    Patch_Dword(0x0058A019+1, (uintptr_t)&AudioTheme.IsRepeat); // OptionsClass::Load_Settings
    Patch_Dword(0x0058A59E+1, (uintptr_t)&AudioTheme.IsRepeat); // OptionsClass::Set
    Patch_Dword(0x0067C9D8+2, (uintptr_t)&AudioTheme.IsRepeat); // WDT init menu

    Patch_Dword(0x00589A37+1, (uintptr_t)&AudioTheme.IsShuffle); // OptionsClass::Set_Shuffle
    Patch_Dword(0x0058A058+1, (uintptr_t)&AudioTheme.IsShuffle); // OptionsClass::Load_Settings
    Patch_Dword(0x0058A5C5+1, (uintptr_t)&AudioTheme.IsShuffle); // OptionsClass::Set
    Patch_Dword(0x0068C99C+1, (uintptr_t)&AudioTheme.IsShuffle); // Join_WOL_Lobby
    Patch_Dword(0x0068C9A1+2, (uintptr_t)&AudioTheme.IsShuffle); // Join_WOL_Lobby
    Patch_Dword(0x0068CFBD+2, (uintptr_t)&AudioTheme.IsShuffle); // Join_WOL_Lobby

    // This voodoo is required as we don't have access to ActiveCount of DynamicVectorClass.
    Patch_Dword(0x005FC747+1, ((uintptr_t)&AudioTheme.Themes) + (sizeof(DynamicVectorClass<void*>)-8)); // SoundControlsClass::Process
    Patch_Dword(0x005FC7F7+1, ((uintptr_t)&AudioTheme.Themes) + (sizeof(DynamicVectorClass<void*>)-8)); // SoundControlsClass::Process

    /**
     *  Replace the speech handler with the new AudioVoxClass.
     */
    Patch_Jump(0x00665940, &AudioVoxClass::AI);
    Patch_Jump(0x00665AF0, &AudioVoxClass::Stop_Speaking);
    Patch_Jump(0x00665B20, &AudioVoxClass::Is_Speaking);
    Patch_Jump(0x00665B70, &AudioVoxClass::Set_Speech_Volume);
    Patch_Jump(0x00665BC0, &AudioVoxClass::Set_Speech_Allowed);
    Patch_Jump(0x00665BD0, &AudioVoxClass::Is_Speech_Allowed);

    // Speak() is generally patched to go through the wrapper for name-based speaking
    Patch_Jump(0x00665800, &Speak_Wrapper);
    // But TeamClass::TMission_PLAY_SPEECH and its inlined copy get passed through the normal play-by-index function
    // In TAction this is handled by re-implementing the action completely.
    Patch_Call(0x00622E75, static_cast<void (*)(VoxType, bool)>(&AudioVoxClass::Speak));
    Patch_Call(0x0062683C, static_cast<void (*)(VoxType, bool)>(&AudioVoxClass::Speak));

    /**
     *  Replace VocClass with the new AudioVocClass.
     */
    Patch_Jump(0x00664BA0, static_cast<int (*)(VocType, float, int)>(&AudioVocClass::Play));
    // Voice_Sound_Effect is not subject to Options.SoundVolume, so it goes through
    // a dedicated function that plays in the always-full-volume system group.
    Patch_Jump(0x00664C60, static_cast<int (*)(VocType, float)>(&AudioVocClass::Voice_Play));
    Patch_Jump(0x00664D10, static_cast<int (*)(VocType, Coord const &)>(&AudioVocClass::Play));
    Patch_Jump(0x00664EC0, &AudioVocClass::Process);
    Patch_Jump(0x00665080, &AudioVocClass::Clear);
    Patch_Jump(0x00665100, &AudioVocClass::From_Name);
    Patch_Jump(0x00665140, &AudioVocClass::Voc_From_Name);
    Patch_Jump(0x006651C0, &AudioVocClass::VocType_From_Voc);

    /**
     *  Replace audio driver interface for the VQA player (AHandle).
     */
    Patch_Jump(0x004072D0, &AudioHandleClass::Timer_Callback_Audio_Handler);
    Patch_Jump(0x00407450, &AudioHandleClass::Stream_Audio_Handler);
    Patch_Jump(0x00408200, &AudioHandleClass::Simple_Timer_Callback_Audio_Handler);
    Patch_Jump(0x004082B0, &AudioHandleClass::Lock_Audio_Handler);
    Patch_Jump(0x004082C0, &AudioHandleClass::Unlock_Audio_Handler);

    /**
     *  #BUGFIX:
     *  Fixes a bug where themes would restart after focus loss and regain. The
     *  new audio manager system handles the focus loss and gain, so this is no
     *  longer required.
     * 
     *  This patch removes calls to ThemeClass::Suspend and ThemeClass::Play_Song.
     */
    Patch_Jump(0x00685994, 0x006859A8); // Focus_Loss
    Patch_Jump(0x00685EA5, 0x00685EB9); // inlined Focus_Loss
    Patch_Jump(0x00685B84, 0x00685B95); // Focus_Restore

    /**
     *  Skip the original sound/theme initialization.
     *  This is now done mid-Init_Rules so that sides are available
     */
    Patch_Jump(0x004E08EE, 0x004E0AFF);

    /**
     *  Skips some file/INI destructors in Init_Game.
     */
    Patch_Jump(0x004E0B22, 0x004E0B4F);
    Patch_Jump(0x004E0BD8, 0x004E0C05);
}
