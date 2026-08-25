/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Hooks that route non-gameplay UI audio (MS engine text/font, score
 *          screen, World Domination Tour, map select) through the new audio
 *          engine.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_ui_hooks.h"

#include "audio_manager.h"
#include "audio_util.h"
#include "ccfile.h"
#include "hooker.h"
#include "msengine.h"
#include "syringe.h"
#include "tibsun_globals.h"
#include "vector.h"

#include <algorithm>


class ScoreSfxEntryExt;
struct ScoreClassExt;
class MSFont;

static std::string Resolve_Text_Sound_Name(int index)
{
    static const std::string TextSoundNames[3] = {
        "TEXT1.AUD",
        "TEXT2.AUD",
        "TEXT3.AUD",
    };

    if (index < 0 || index >= 3) {
        return TextSoundNames[0];
    }

    return TextSoundNames[index];
}

struct ScoreClassExt {
    int XPos;
    int YPos;
    void* SurfacePtr;
    void* FullFont;
    void* BigFont;
    void* Drawer;
    int Color;
    DynamicVectorClass<void*> ScoreObjs;
    DynamicVectorClass<ScoreSfxEntryExt*> ScoreSnds;

    void DoSound(const char* name, int volume) const;
};
static_assert(offsetof(ScoreClassExt, ScoreSnds) == 0x34, "ScoreClassExt::ScoreSnds offset mismatch.");


DEFINE_HOOK(0x0056D035, _MSWordAnim_Play_Sample_Patch, 0)
{
    Audio_Play_UI_Sample("BLEEP1.AUD", 10, 64);
    return R->Origin() + 0x2F;
}
DEFINE_HOOK_AGAIN(0x0056D13B, _MSWordAnim_Play_Sample_Patch, 0);


DEFINE_HOOK(0x0057254C, _MSFont_Play_Sample_Patch, 0)
{
    GET(int, index, EDX);

    Audio_Play_UI_Sample(Resolve_Text_Sound_Name(index), 10, 64);
    return 0x00572576;
}


DEFINE_HOOK(0x005E3549, _ScoreClass_Presentation_Wipe_Sound_Patch, 0)
{
    GET(ScoreClassExt *, this_ptr, EBP);

    this_ptr->DoSound("Wipe", 256);
    return 0x005E35BA;
}


DEFINE_HOOK(0x005E3695, _ScoreClass_Presentation_Efficiency_Sound_Patch, 0)
{
    GET(ScoreClassExt *, this_ptr, EBP);

    this_ptr->DoSound("Efficiency", 128);
    return 0x005E3706;
}


DEFINE_HOOK(0x005E3DFA, _ScoreClass_Presentation_Emblem_Sound_Patch, 0)
{
    GET(ScoreClassExt *, this_ptr, EBP);

    this_ptr->DoSound("Emblem", 256);
    return 0x005E3E6B;
}


DEFINE_HOOK(0x005E43C6, _ScoreClass_Presentation_BestBox_Sound_Patch, 0)
{
    GET(ScoreClassExt *, this_ptr, EBP);

    this_ptr->DoSound("BestBox", 256);
    return 0x005E4437;
}


DEFINE_HOOK(0x005E57B2, _ScoreClass_Do_Graph_BarGraph_Sound_Patch, 0)
{
    GET(ScoreClassExt *, this_ptr, ESI);

    this_ptr->DoSound("BarGraph", 96);
    return 0x005E5835;
}


DEFINE_HOOK(0x005E5F83, _ScoreClass_Input_Name_Back_Sound_Patch, 0)
{
    GET(ScoreClassExt *, this_ptr, ESI);

    this_ptr->DoSound("Back", 192);
    return 0x005E61EB;
}


DEFINE_HOOK(0x005E6155, _ScoreClass_Input_Name_Type_Sound_Patch, 0)
{
    GET(ScoreClassExt *, this_ptr, ESI);

    this_ptr->DoSound("Type", 128);
    return 0x005E61C6;
}


DEFINE_HOOK(0x005E6B97, _ScoreFontClass_PrintChar_Sound_Patch, 0)
{
    GET(int, index, EDX);

    Audio_Play_UI_Sample(Resolve_Text_Sound_Name(index), 255, 128);
    return 0x005E6BC4;
}


class ScoreSfxEntryExt
{
public:
    ScoreSfxEntryExt(const char *name, const char *filename);
    ~ScoreSfxEntryExt();

    bool Matches_Name(const char *name) const;
    void Play(int volume) const;

    ScoreSfxEntryExt *CTOR_Proxy(const char *name, const char *filename);
    void DTOR_Proxy();

private:
    char *mName = nullptr;
    char *mFileName = nullptr;
    bool mAllocated = false;
    char mPad[3] {};
};
static_assert(sizeof(ScoreSfxEntryExt) == 0x0C, "ScoreSfxEntryExt must be 0x0C in size!");


void ScoreClassExt::DoSound(const char *name, int volume) const
{
    if (name != nullptr && AudioManager.Is_Available()) {
        for (int index = 0; index < ScoreSnds.Count(); ++index) {
            ScoreSfxEntryExt* snd = ScoreSnds[index];
            if (snd != nullptr && snd->Matches_Name(name)) {
                snd->Play(volume);
            }
        }
    }
}


ScoreSfxEntryExt::ScoreSfxEntryExt(const char *name, const char *filename)
{
    mName = strdup(name != nullptr ? name : "");
    mFileName = strdup(filename != nullptr ? filename : "");
    mAllocated = (mName != nullptr || mFileName != nullptr);
}


ScoreSfxEntryExt::~ScoreSfxEntryExt()
{
    if (mName != nullptr) {
        free(mName);
    }

    if (mFileName != nullptr) {
        free(mFileName);
    }
}


bool ScoreSfxEntryExt::Matches_Name(const char *name) const
{
    return mName != nullptr && name != nullptr && !strcasecmp(mName, name);
}


void ScoreSfxEntryExt::Play(int volume) const
{
    Audio_Play_UI_Sample(mFileName, 255, volume);
}


ScoreSfxEntryExt *ScoreSfxEntryExt::CTOR_Proxy(const char *name, const char *filename)
{
    new (this) ScoreSfxEntryExt(name, filename);
    return this;
}


void ScoreSfxEntryExt::DTOR_Proxy()
{
    this->~ScoreSfxEntryExt();
}


class WDTVoiceSampleExt
{
public:
    WDTVoiceSampleExt(const char *name, int volume);
    ~WDTVoiceSampleExt();

    bool Playing() const;
    void Start();
    void Stop();

    WDTVoiceSampleExt *CTOR_Proxy(const char *name, int volume);
    void DTOR_Proxy();

private:
    int mVolume = 255;
    AudioInstanceHandle mSoundHandle = INVALID_AUDIO_INSTANCE_HANDLE;
    char *mFileName = nullptr;
    char mPad[4] {};
};
static_assert(sizeof(WDTVoiceSampleExt) == 0x10, "WDTVoiceSampleExt must be 0x10 in size!");


WDTVoiceSampleExt::WDTVoiceSampleExt(const char *name, int volume)
{
    mVolume = volume;

    if (name != nullptr && CCFileClass(name).Is_Available()) {
        mFileName = strdup(name);
    }
}


WDTVoiceSampleExt::~WDTVoiceSampleExt()
{
    Stop();

    if (mFileName != nullptr) {
        free(mFileName);
    }
}


bool WDTVoiceSampleExt::Playing() const
{
    return mSoundHandle != INVALID_AUDIO_INSTANCE_HANDLE && AudioManager.Query_Is_Active(mSoundHandle);
}


void WDTVoiceSampleExt::Start()
{
    if (mSoundHandle == INVALID_AUDIO_INSTANCE_HANDLE && mFileName != nullptr) {
        AudioInstanceHandle handle = Audio_Play_UI_File(mFileName, AUDIO_TYPE_AUD, 255, mVolume);
        if (handle != INVALID_AUDIO_INSTANCE_HANDLE) {
            mSoundHandle = handle;
        }
    }
}


void WDTVoiceSampleExt::Stop()
{
    if (mSoundHandle != INVALID_AUDIO_INSTANCE_HANDLE) {
        AudioManager.Request_Stop(mSoundHandle);
        mSoundHandle = INVALID_AUDIO_INSTANCE_HANDLE;
    }
}


WDTVoiceSampleExt *WDTVoiceSampleExt::CTOR_Proxy(const char *name, int volume)
{
    new (this) WDTVoiceSampleExt(name, volume);
    return this;
}


void WDTVoiceSampleExt::DTOR_Proxy()
{
    this->~WDTVoiceSampleExt();
}


DEFINE_HOOK(0x0067FCE5, _WorldDominationTour_Voices_Advance_Start_Patch, 0)
{
    GET(void *, this_ptr, ESI);

    // Hack to get around having to make a WDT::Voices layout
    auto *sample = reinterpret_cast<WDTVoiceSampleExt *>(*reinterpret_cast<void **>(reinterpret_cast<uintptr_t>(this_ptr) + 0x40C));
    if (sample != nullptr) {
        sample->Start();
    }

    return 0x0067FD1A;
}


class MSSfxExt
{
public:
    MSSfxExt(char const* name, char const* fileName);
    ~MSSfxExt();
    void Play(int volume);

    MSSfxExt* CTOR_Proxy(char const* name, char const* fileName);
    void DTOR_Proxy();

private:
    char* mName = nullptr;
    char* mFileName = nullptr;
    char mPad[4] {};
};
static_assert(sizeof(MSSfxExt) == 0xC, "MSSfxExt must be 0xC in size!");


MSSfxExt::MSSfxExt(char const* name, char const* fileName)
{
    if (name != nullptr && fileName != nullptr) {
        mName = strdup(name);
        mFileName = strdup(fileName);
    }
}


MSSfxExt::~MSSfxExt()
{
    if (mName != nullptr) {
        free(mName);
    }
    if (mFileName != nullptr) {
        free(mFileName);
    }
}


void MSSfxExt::Play(int volume)
{
    if (AudioManager.Is_Available() && mFileName != nullptr) {
        Audio_Play_UI_Sample(mFileName, 255, volume);
    }
}


MSSfxExt* MSSfxExt::CTOR_Proxy(char const* name, char const* fileName)
{
    new (this) MSSfxExt(name, fileName);
    return this;
}


void MSSfxExt::DTOR_Proxy()
{
    this->~MSSfxExt();
}


class MSSfxEntryExt
{
public:
    MSSfxEntryExt(char const* name, char* string);
    ~MSSfxEntryExt();
    void Play();

    MSSfxEntryExt* CTOR_Proxy(char const* name, char* string);
    void DTOR_Proxy();

private:
    char* mName = nullptr;
    char* mFileName = nullptr;
    int mVolume = 255;
    char mPad[4] {};
};
static_assert(sizeof(MSSfxEntryExt) == 0x10, "MSSfxEntry must be 0x10 in size!");


MSSfxEntryExt::MSSfxEntryExt(char const* name, char* string)
{
    if (name != nullptr) {
        char* fileName = std::strtok(string, ",");

        char* volumeStr = std::strtok(nullptr, ",");
        if (volumeStr != nullptr) {
            mVolume = std::atoi(volumeStr);
            mVolume = std::clamp(mVolume, 0, 100);
            mVolume = 255 * mVolume / 100;
        }

        if (mVolume > 0 && name != nullptr && fileName != nullptr) {
            mName = strdup(name);
            mFileName = strdup(fileName);
        }
    }
}


MSSfxEntryExt::~MSSfxEntryExt()
{
    if (mName != nullptr) {
        free(mName);
    }
    if (mFileName != nullptr) {
        free(mFileName);
    }
}


void MSSfxEntryExt::Play()
{
    if (AudioManager.Is_Available() && mFileName != nullptr) {
        Audio_Play_UI_Sample(mFileName, 255, mVolume);
    }
}


MSSfxEntryExt* MSSfxEntryExt::CTOR_Proxy(char const* name, char* string)
{
    new (this) MSSfxEntryExt(name, string);
    return this;
}


void MSSfxEntryExt::DTOR_Proxy()
{
    this->~MSSfxEntryExt();
}


class MapChoiceExt
{
private:
    DynamicVectorClass<void*> mStages;
    Rect mTextRect;
    char const* mAnimPaletteName;
    DynamicVectorClass<void*> mAnimEntries;
    DynamicVectorClass<void*> mSoundEntries;
};
static_assert(sizeof(MapChoiceExt) == 0x5C, "MapChoiceExt must be 0x5C in size!");


class MapSelectExt : public MSEngine
{
public:
    void DoIdle();
    void DoVoiceOver(char const* name, int delay);
    void PlayVoiceOver(char const* name);
    void StopVoiceOver(bool fade);

private:
    int mXoffset;
    int mYoffset;
    MapChoiceExt mChoices;
    ConvertClass* mDrawer;
    ConvertClass* mOverlayDrawer;
    MSFont* mFont;
    Rect mTextRect;
    char const* mQueuedVoiceOver;
    CDTimerClass<SystemTimerClass> mVoiceOverTimer;
    AudioInstanceHandle mPlayingVoiceOver;
    Surface* mClickMap;
};
static_assert(sizeof(MapSelectExt) == 0xE0, "MapSelectExt must be 0xE0 in size!");


void MapSelectExt::DoIdle()
{
    if (mQueuedVoiceOver != nullptr && mVoiceOverTimer == 0) {
        PlayVoiceOver(mQueuedVoiceOver);
    }
}


void MapSelectExt::DoVoiceOver(char const* name, int delay)
{
    StopVoiceOver(true);
    mQueuedVoiceOver = name;
    mVoiceOverTimer = delay;
}


void MapSelectExt::PlayVoiceOver(char const* name)
{
    if (mPlayingVoiceOver != INVALID_AUDIO_INSTANCE_HANDLE) {
        AudioManager.Request_Stop(mPlayingVoiceOver);
    }

    /**
     *  The map selection voiceover is EVA speech, so it belongs to the speech
     *  group, which already applies the player's voice volume. Playing it in the
     *  UI group instead attenuated it by the sound effect volume on top of that.
     */
    mPlayingVoiceOver = Audio_Play_UI_Sample(name, 255, 255, AUDIO_GROUP_SPEECH);

    if (mPlayingVoiceOver != INVALID_AUDIO_INSTANCE_HANDLE) mQueuedVoiceOver = nullptr;

    mVoiceOverTimer = 0;
}


void MapSelectExt::StopVoiceOver(bool fade)
{
    if (mPlayingVoiceOver != INVALID_AUDIO_INSTANCE_HANDLE) {
        if (fade == true) {
            AudioManager.Request_Stop(mPlayingVoiceOver, 0.33f);
        } else {
            AudioManager.Request_Stop(mPlayingVoiceOver);
        }

        mPlayingVoiceOver = INVALID_AUDIO_INSTANCE_HANDLE;
    }

    mQueuedVoiceOver = nullptr;
    mVoiceOverTimer = 0;
}


DEFINE_HOOK(0x00554228, _MapSelect_UserInput_StopVoiceover_Patch1, 0)
{
    GET(MapSelectExt*, this_ptr, ESI);
    this_ptr->StopVoiceOver(true);
    return 0x00554277;
}


DEFINE_HOOK(0x005544B9, _MapSelect_UserInput_StopVoiceover_Patch2, 0)
{
    GET(MapSelectExt*, this_ptr, ESI);
    this_ptr->StopVoiceOver(false);
    return 0x005544F6;
}


DEFINE_HOOK(0x0055441D, _MapSelect_UserInput_DoVoiceover_Patch, 0)
{
    GET(MapSelectExt*, this_ptr, ESI);
    GET(char const*, voice_over, EBX);
    this_ptr->DoVoiceOver(voice_over, TIMER_SECOND / 2);
    return 0x00554493;
}


/**
 *  Main function for patching the hooks.
 */
void Audio_UI_Hooks()
{
    Patch_Jump(0x005E6770, &ScoreClassExt::DoSound);
    Patch_Jump(0x005E72F0, &ScoreSfxEntryExt::CTOR_Proxy);
    Patch_Jump(0x005E73B0, &ScoreSfxEntryExt::DTOR_Proxy);

    Patch_Jump(0x0067FF90, &WDTVoiceSampleExt::CTOR_Proxy);
    Patch_Jump(0x00680040, &WDTVoiceSampleExt::DTOR_Proxy);
    Patch_Jump(0x00680080, &WDTVoiceSampleExt::Playing);
    Patch_Jump(0x006800A0, &WDTVoiceSampleExt::Start);
    Patch_Jump(0x006800E0, &WDTVoiceSampleExt::Stop);

    Patch_Jump(0x00574BE0, &MSSfxExt::CTOR_Proxy);
    Patch_Jump(0x00574CA0, &MSSfxExt::DTOR_Proxy);
    Patch_Jump(0x00574CE0, &MSSfxExt::Play);

    Patch_Jump(0x0056F270, &MSSfxEntryExt::CTOR_Proxy);
    Patch_Jump(0x0056F3B0, &MSSfxEntryExt::DTOR_Proxy);
    Patch_Jump(0x0056F3F0, &MSSfxEntryExt::Play);

    Patch_Jump(0x00553FF0, &MapSelectExt::DoIdle);
    Patch_Jump(0x00554550, &MapSelectExt::DoVoiceOver);
    Patch_Jump(0x005545D0, &MapSelectExt::PlayVoiceOver);
    Patch_Jump(0x00554640, &MapSelectExt::StopVoiceOver);
    Patch_Dword(0x00553F38 + 6, INVALID_AUDIO_INSTANCE_HANDLE.ID); // Initialize mPlayingVoiceOver to INVALID_AUDIO_INSTANCE_HANDLE instead of -1
}
