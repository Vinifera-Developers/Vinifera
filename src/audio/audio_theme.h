/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Music theme playback and management.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "audio_defines.h"
#include "tibsun_defines.h"
#include "vector.h"


class CCINIClass;
class AudioSampleClass;


/**
 *  A reimplementation of ThemeClass to use with the new audio driver interface.
 */
class AudioThemeClass
{
    friend void Audio_Hooks();

public:
    AudioThemeClass();
    ~AudioThemeClass();

    ThemeType From_Name(const char* name) const;
    ThemeType Next_Song(ThemeType theme) const;
    ThemeType What_Is_Playing() const { return Score; }
    bool Is_Allowed(ThemeType theme) const;
    bool Is_Regular(ThemeType theme) const;
    bool Is_Playable(ThemeType theme) const;
    const char* Base_Name(ThemeType theme) const;
    const char* INI_Name(ThemeType theme) const;
    const char* Full_Name(ThemeType theme) const;

    int Max_Themes() const { return Themes.Count(); }

    bool Play_Song(ThemeType theme);
    bool Still_Playing() const;
    int Track_Length(ThemeType theme) const;
    void AI();
    void Fade_Out();
    void Queue_Song(ThemeType theme);
    void Stop(bool fade = false);
    bool Suspend();
    bool Resume();
    bool Is_Paused() const;

    void Set_Volume(int volume);

    int Process(CCINIClass const& ini);

    void Set_Theme_Data(ThemeType theme, int scenario, SideType owners);

    void Set_Shuffle(int on) { IsShuffle = on; }
    void Set_Repeat(int on) { IsRepeat = on; }

    void Scan();
    void Preload();

    bool Init_Themes(CCINIClass const& ini);
    void Free_Themes();

private:
    bool Is_Valid_Theme(ThemeType theme) const { return theme >= THEME_FIRST && theme < Themes.Count(); }

    /**
     *  Handle to current score.
     */
    AudioInstanceHandle ScoreHandle = INVALID_AUDIO_INSTANCE_HANDLE;

    /**
     *  Score number currently being played.
     */
    ThemeType Score = THEME_NONE;

    /**
     *  Score to play next.
     */
    ThemeType Pending = THEME_NONE;

    /**
     *  Score should repeat?
     */
    bool IsRepeat = false;

    /**
     *  Score list should shuffle?
     */
    bool IsShuffle = false;

public:
    struct ThemeControl {
        explicit ThemeControl(std::string name) : Name(std::move(name)) {}

        ~ThemeControl() = default;

        bool Fill_In(CCINIClass const& ini);

        /**
         *  The file type of this score.
         */
        AudioFileType FileType = AUDIO_TYPE_AUD;

        /**
         *  Full filename of the score.
         */
        std::string FileName;

        /**
         *  Filename of the score.
         */
        std::string Name;

        /**
         *  Full score name.
         */
        std::string Fullname;

        /**
         *  The artist name.
         */
        std::string Artist;

        /**
         *  Scenario when it first becomes available.
         */
        int Scenario = 0;

        /**
         *  Duration of theme in seconds.
         */
        float Duration = 0.0f;

        /**
         *  Allowed in normal game play?
         */
        bool Normal = true;

        /**
         *  Always repeat this score?
         */
        bool Repeat = false;

        /**
         *  Is the score available?
         */
        bool Available = false;

        /**
         *  What houses are allowed to play this theme (bit field)?
         */
        int Owner = -1;

        /**
         *  The addon required to be active for this theme to be available.
         */
        AddonType RequiredAddon = ADDON_BASE_GAME;

        /**
         *  User defined filename.
         */
        std::string Sound;

        /**
         *  Volume control for this theme.
         */
        float Volume = 1.0f;
    };

    /**
     *  List of all registered theme control entries.
     */
    DynamicVectorClass<ThemeControl*> Themes;

private:
    /**
     *  Duration in seconds for theme fade-out transitions.
     */
    static float FadeOutSeconds;

    /**
     *  Whether cross-fading between themes is enabled, and its duration.
     */
    static bool CrossFade;
    static float CrossFadeSeconds;
};
