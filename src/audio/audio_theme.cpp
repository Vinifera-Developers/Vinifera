/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Music theme playback and management.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "audio_theme.h"

#include "addon.h"
#include "audio_debug.h"
#include "audio_manager.h"
#include "ccini.h"
#include "debughandler.h"
#include "house.h"
#include "housetype.h"
#include "session.h"
#include "side.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "vinifera_util.h"


/**
 *  Instance of the new theme engine (extern where needed).
 */
AudioThemeClass AudioTheme;


float AudioThemeClass::FadeOutSeconds = 1.5f;
bool AudioThemeClass::CrossFade = false;
float AudioThemeClass::CrossFadeSeconds = 10.0f;


/**
 *  Default constructor for the theme manager class.
 * 
 *  @author: CCHyper
 */
AudioThemeClass::AudioThemeClass()
{
    Themes.Clear();
}


/**
 *  Class destructor
 * 
 *  @author: CCHyper
 */
AudioThemeClass::~AudioThemeClass()
{
    Free_Themes();
}


/**
 *  Is this a normal theme that was flaged as available and can be played? 
 * 
 *  @author: CCHyper
 */
bool AudioThemeClass::Is_Playable(ThemeType theme) const
{
    return Is_Regular(theme) && Themes[theme]->Available;
}


/**
 *  Is this a normal theme that can be played?
 * 
 *  @author: CCHyper
 */
bool AudioThemeClass::Is_Regular(ThemeType theme) const
{
    return Is_Valid_Theme(theme) && Themes[theme]->Normal;
}


/**
 *  Fetches the base filename for the theme specified.
 * 
 *  @author: CCHyper
 */
const char * AudioThemeClass::Base_Name(ThemeType theme) const
{
    if (Is_Valid_Theme(theme)) {
        return Themes[theme]->Name.c_str();
    }
    return "No theme";
}


/**
 *  Fetches the base filename for use in INI lookups, returning "none" for invalid themes.
 *
 *  @author: CCHyper
 */
const char * AudioThemeClass::INI_Name(ThemeType theme) const
{
    if (Is_Valid_Theme(theme)) {
        return Themes[theme]->Name.c_str();
    }
    return "none";
}


/**
 *  Retrieves the full score name.
 *
 *  @author: CCHyper
 */
const char * AudioThemeClass::Full_Name(ThemeType theme) const
{
    if (Is_Valid_Theme(theme)) {
        return Themes[theme]->Fullname.c_str();
    }
    return nullptr;
}


/**
 *  Are scores available for playback?
 *
 *  @author: ZivDero
 */
bool AudioThemeClass::Scores_Available()
{
    return AudioManager.Is_Available() && ScoresPresent && !Debug_Quiet;
}


/**
 *  Process the theme engine and restart songs.
 *
 *  @author: CCHyper, ZivDero
 */
void AudioThemeClass::AI()
{
    if (!Scores_Available()) {
        return;
    }

    if (AudioManager.Get_Group_Volume(AUDIO_GROUP_MUSIC) <= 0.0f) {
        return;
    }

    if (ScenarioInit) {
        return;
    }

    if (Still_Playing()) {
        return;
    }

    if (Pending == THEME_NONE || Pending == THEME_QUIET) {
        return;
    }
    
    DEBUG_INFO("Theme::AI - Time to play next song...\n");
    
    /**
     *  If the pending song needs to be picked, then pick it now.
     */
    if (Pending == THEME_PICK_ANOTHER) {
        Pending = Next_Song(Score);
        DEBUG_INFO("Theme::AI - Next_Song returned \"{}\".\n", Themes[Pending]->Name);
    }
    
    /**
     *  Start the song playing.
     */
    DEBUG_INFO("Theme::AI - About to call Play_Song with \"{}\".\n", Themes[Pending]->Name);
    Play_Song(Pending);

    /**
     *  Now flag it so that a new song will be picked when this one ends.
     */
    Pending = THEME_PICK_ANOTHER;
}


/**
 *  Calculates the next song number to play.
 *
 *  @author: CCHyper, ZivDero
 */
ThemeType AudioThemeClass::Next_Song(ThemeType theme) const
{
    /**
     *  If the current theme is set to repeat, return it again.
     */
    if (Is_Valid_Theme(theme) && (Themes[theme]->Repeat || IsRepeat)) {
        return theme;
    }

    if (IsShuffle) {

        /**
         *  Shuffle the theme, but never pick the same theme that was just
         *  playing.
         */
        const ThemeType previous = theme;
        int tries = 0;
        bool maxed = false;
        while (tries++ <= 1000) {
            ThemeType newtheme = Sim_Random_Pick(THEME_FIRST, static_cast<ThemeType>(Themes.Count() - 1));
            maxed = tries == 1000;
            if (newtheme != previous && Is_Allowed(newtheme)) {
                theme = newtheme;
                break;
            }
        }
        if (maxed) {
            theme = THEME_FIRST;
        }

    } else {

        /**
         *  Sequential score playing.
         */
        for (int i = Themes.Count() + 1; i > 0; --i) {
            if (++theme >= Themes.Count()) {
                theme = THEME_FIRST;
            }
            if (Is_Allowed(theme)) {
                return theme;
            }
        }

        theme = THEME_FIRST;
    }

    return theme;
}


/**
 *  Queues the song to the play queue.
 *
 *  @author: CCHyper, ZivDero
 */
void AudioThemeClass::Queue_Song(ThemeType theme)
{
    if (!Scores_Available()) {
        return;
    }

    /**
     *  If the current score volume is set to silent, then there is no need to
     *  play the specified theme.
     */
    if (AudioManager.Get_Group_Volume(AUDIO_GROUP_MUSIC) <= 0.0f) {
        return;
    }

    /**
     *  If the pending theme is available to be set and the specified theme is valid, then
     *  set the queued theme accordingly.
     */
    if (Pending == THEME_NONE || Pending == THEME_PICK_ANOTHER || theme == THEME_NONE || theme == THEME_QUIET) {

        if (Is_Valid_Theme(theme)) {
            DEBUG_INFO("Theme::Queue_Song - Queued \"{}\".\n", Themes[theme]->Name);
        } else {
            DEBUG_INFO("Theme::Queue_Song - Pending {}, theme {}.\n", (int)Pending, (int)theme);
        }

        Pending = theme;

        if (Still_Playing()) {
            DEBUG_INFO("Theme::Queue_Song - Fading out current score...\n");
            AudioManager.Request_Stop(ScoreHandle, CrossFade ? CrossFadeSeconds : 0.0f);
        }

    }
}


/**
 *  Starts the specified song play NOW.
 *
 *  @author: CCHyper, ZivDero
 */
bool AudioThemeClass::Play_Song(ThemeType theme)
{
    if (!Scores_Available()) {
        return false;
    }

    /**
     *  Stop any theme currently playing in a abrupt manner.
     */
    Stop();

    /**
     *  If the current score volume is set to silent, then there is no need to
     *  play the specified theme.
     */
    if (AudioManager.Get_Group_Volume(AUDIO_GROUP_MUSIC) <= 0.0f) {
        return false;
    }

    /**
     *  Bail if the theme is not a valid playable index.
     */
    if (theme == THEME_PICK_ANOTHER) {
        Pending = theme;
        return false;
    }

    if (!Is_Valid_Theme(theme)) {
        return false;
    }

    ThemeControl *tctrl = Themes[theme];

    /**
     *  #BUGFIX:
     *  Check for availability of the theme before attempting to play it.
     *  This stops the theme handler from spamming attempts to the audio engine.
     */
    if (!tctrl->Available) {
        return false;
    }

    ThemeType current = Score;
    Score = theme;

    /**
     *  Request the audio manager to begin playing the theme.
     */
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Play_Song - About to call AudioManager.Request_Play with \"%s\".\n", tctrl->FileName.c_str());
    AudioInstanceHandle handle = AudioManager.Request_Play(tctrl->FileName, AUDIO_GROUP_MUSIC, tctrl->Volume, 1.0f, 0.0f, AUDIO_PRIORITY_HIGH, 1, CrossFade ? CrossFadeSeconds : 0.0f);

    if (handle == INVALID_AUDIO_INSTANCE_HANDLE) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THEME, "Theme::Play_Song - Failed to play \"%s\"!\n", Themes[theme]->Name.c_str());
        return false;
    }

    /**
     *  Stop the previously playing theme if one was active.
     */
    if (ScoreHandle.Is_Valid() && current > THEME_NONE && current < Themes.Count()) {
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Play_Song - Stopping handle for \"%s\"\n", Themes[current]->Name.c_str());
        AudioManager.Request_Stop(ScoreHandle, CrossFade ? CrossFadeSeconds : 0.0f);
    }

    /**
     *  Store the new handle as the active score.
     */
    ScoreHandle = handle;
    
    /**
     *  If this theme is flagged to repeat, set pending.
     */
    if (tctrl->Repeat || IsRepeat) {
        DEBUG_INFO("Theme::Play_Song - Playing \"{}\" (Repeating)\n", Themes[theme]->Name);
        Pending = theme;

    } else {
        DEBUG_INFO("Theme::Play_Song - Playing \"{}\" (Normal)\n", Themes[theme]->Name);
    }

    return true;
}


/**
 *  Calculates the length of the song (in seconds).
 *
 *  @author: CCHyper
 */
int AudioThemeClass::Track_Length(ThemeType theme) const
{
    if (Is_Valid_Theme(theme)) {
        return Themes[theme]->Duration * TIMER_SECOND;
    }
    return 0;
}


/**
 *  Queues the current theme to fade out and stop.
 *
 *  @author: CCHyper
 */
void AudioThemeClass::Fade_Out()
{
    Queue_Song(THEME_QUIET);
}


/**
 *  Stops the current theme from playing.
 * 
 *  @author: CCHyper
 */
void AudioThemeClass::Stop(bool fade)
{
    if (!Scores_Available()) {
        return;
    }

    if (Score == THEME_NONE) {
        return;
    }

    if (ScoreHandle == INVALID_AUDIO_INSTANCE_HANDLE) {
        return;
    }

    if (Still_Playing()) {
        if (fade) {
            DEBUG_INFO("Theme::Stop - Fading out \"{}\"...\n", Themes[Score]->Name);
            AudioManager.Request_Stop(ScoreHandle, FadeOutSeconds);
        } else {
            DEBUG_INFO("Theme::Stop - Forced \"{}\" to stop.\n", Themes[Score]->Name);
            AudioManager.Request_Stop(ScoreHandle);
        }
    }

    Score = THEME_NONE;
    Pending = THEME_NONE;
}


/**
 *  Suspends (pauses) the currently playing theme.
 *
 *  @author: CCHyper
 */
bool AudioThemeClass::Suspend()
{
    if (!Scores_Available()) {
        return false;
    }

    if (!Is_Valid_Theme(What_Is_Playing())) {
        return false;
    }

    DEBUG_INFO("Theme::Suspend - Suspending score \"{}\"\n", Themes[Score]->Name);
    
    return AudioManager.Request_Pause(ScoreHandle);
}


/**
 *  Resumes playback of a previously suspended theme.
 *
 *  @author: CCHyper
 */
bool AudioThemeClass::Resume()
{
    if (!Scores_Available()) {
        return false;
    }

    if (!Is_Valid_Theme(What_Is_Playing())) {
        return false;
    }

    DEBUG_INFO("Theme::Resume - Resuming score \"{}\"\n", Themes[Score]->Name);
    
    return AudioManager.Request_Resume(ScoreHandle);
}


/**
 *  Checks if the current theme is paused.
 *
 *  @author: CCHyper
 */
bool AudioThemeClass::Is_Paused() const
{
    return AudioManager.Query_Is_Paused(ScoreHandle);
}


/**
 *  Deletes all theme control entries and clears the theme list.
 *
 *  @author: CCHyper
 */
void AudioThemeClass::Free_Themes()
{
    while (Themes.Count() > 0) {
        delete Themes[0];
        Themes.Delete(0);
    }
}


/**
 *  Sets the music group volume, converting from 0-255 range to 0.0-1.0.
 *
 *  @author: CCHyper
 */
void AudioThemeClass::Set_Volume(int volume)
{
    float volf = std::clamp(AudioManagerClass::iVolume_To_fVolume(volume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    AudioManager.Set_Group_Volume(AUDIO_GROUP_MUSIC, volf);
}


/**
 *  Reads theme definitions from the INI database and populates the theme list.
 *
 *  @author: CCHyper
 */
int AudioThemeClass::Process(CCINIClass const &ini)
{
    static char const * const GENERAL = "General";
    static char const * const THEMES = "Themes";

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Process(enter): Themes.Count = %d\n", Themes.Count());

    if (ini.Is_Present(GENERAL)) {
        FadeOutSeconds = ini.Get_Float(GENERAL, "FadeOutSeconds", FadeOutSeconds);
        CrossFade = ini.Get_Bool(GENERAL, "CrossFading", CrossFade);
        CrossFadeSeconds = ini.Get_Float(GENERAL, "CrossFadeSeconds", CrossFadeSeconds);
    }

    int count = ini.Entry_Count(THEMES);

    for (int index = 0; index < count; ++index) {
        std::string name = ini.Get_String(THEMES, ini.Get_Entry(THEMES, index), "");
        if (!name.empty()) {
            ThemeType theme = From_Name(name.c_str());

            ThemeControl *ctrl = nullptr;
            if (theme == THEME_NONE) {
                ctrl = new ThemeControl(name);
                AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Process: Creating new Theme %s, processing.\n", ctrl->Name.c_str());
                Themes.Add(ctrl);
            } else {
                ctrl = Themes[theme];
                AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Process: Found existing Theme %s, updating.\n", ctrl->Name.c_str());
            }
            ctrl->Fill_In(ini);
        }
    }

    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Process(exit): Themes.Count = %d\n", Themes.Count());

    return count;
}


/**
 *  Determines if music is still playing.
 * 
 *  @author: CCHyper
 */
bool AudioThemeClass::Still_Playing() const
{
    if (!AudioManager.Is_Available() || Debug_Quiet) {
        return false;
    }

    return AudioManager.Query_Is_Playing(ScoreHandle);
}


/**
 *  Checks to see if the specified theme is legal.
 * 
 *  @author: CCHyper
 */
bool AudioThemeClass::Is_Allowed(ThemeType theme) const
{
    if (theme == THEME_QUIET || theme == THEME_PICK_ANOTHER) {
        return true;
    }

    /**
     *  Only normal themes (playable during battle) are considered allowed.
     */
    if (!Is_Playable(theme)) {
        return false;
    }

    /**
     *  #issue-764
     * 
     *  If this theme requires an addon, make sure that addon is active.
     * 
     *  @author: CCHyper
     */
    AddonType addon = Themes[theme]->RequiredAddon;
    if (addon != ADDON_BASE_GAME) {
        if (!Addon_Installed(addon)) {
            return false;
        }
    }

    /**
     *  If the theme is not allowed to be played by the player's house, then don't allow
     *  it. If the player's house hasn't yet been determined, then presume this test
     *  passes.
     */
    if (PlayerPtr != nullptr && Themes[theme]->Owner != -1 && ((1 << PlayerPtr->Class->Side) & Themes[theme]->Owner) == 0) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "Theme::Is_Allowed - Side \"%s\" not allowed to play %s!\n", SideClass::Name_From(PlayerPtr->Class->Side), Themes[theme]->Name.c_str());
        return false;
    }

    /**
     *  If the scenario doesn't allow this theme yet, then return the failure flag. The
     *  scenario check only makes sense for solo play.
     */
    if (Session.Type == GAME_NORMAL && Scen->Scenario < Themes[theme]->Scenario) {
        return false;
    }

    /**
     *  Since all tests passed, return with the "is allowed" flag.
     */
    return true;
}


/**
 *  Determines theme number from specified name.
 * 
 *  @author: CCHyper
 */
ThemeType AudioThemeClass::From_Name(const char * name) const
{
    if (name == nullptr || std::strlen(name) <= 0) {
        return THEME_NONE;
    }

    /**
     *  First search for an exact name match with the filename
     *  of the theme. This is guaranteed to be unique.
     */
    for (ThemeType theme = THEME_FIRST; theme < Themes.Count(); ++theme) {
        if (strcasecmp(Themes[theme]->Name.c_str(), name) == 0) {
            return theme;
        }
    }

    /**
     *  If the filename scan failed to find a match, then scan for
     *  a substring within the full name of the score. This might
     *  yield a match, but is not guaranteed to be unique.
     */
    const std::string needle = name;
    for (ThemeType theme = THEME_FIRST; theme < Themes.Count(); ++theme) {
        if (string_icontains(Themes[theme]->Fullname, needle)) {
            return theme;
        }
    }

    return THEME_NONE;
}


/**
 *  Scans all scores for availability.
 *
 *  @author: CCHyper
 */
void AudioThemeClass::Scan()
{
    if (!AudioManager.Is_Available() || Debug_Quiet || !ScoresPresent) {
        return;
    }

    for (ThemeType theme = THEME_FIRST; theme < Themes.Count(); ++theme) {

        ThemeControl *tctrl = Themes[theme];

        tctrl->Available = false;
        tctrl->FileType = AUDIO_TYPE_NONE;
        tctrl->FileName.clear();

        std::string name = tctrl->Name;

        if (!tctrl->Sound.empty()) {
            name = tctrl->Sound;
        }

        if (!AudioManager.Get_File_Info(name, tctrl->FileType, tctrl->FileName, true)) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "Theme::Scan - File \"%s\" was not found in any supported formats!\n", name.c_str());
            continue;
        }

        tctrl->Available = true;
    }

    Preload();
}


/**
 *  Preloads all available themes into the audio manager.
 *
 *  @author: CCHyper
 */
void AudioThemeClass::Preload()
{
    if (!AudioManager.Is_Available() || Debug_Quiet || !ScoresPresent) {
        return;
    }

    for (ThemeType theme = THEME_FIRST; theme < Themes.Count(); ++theme) {

        ThemeControl *tctrl = Themes[theme];

        if (!tctrl->Available) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "Theme::Preload - File \"%s\" was not found in any supported formats!\n", tctrl->Name.c_str());
            continue;
        }

        if (AudioManager.Has_Been_Submitted(tctrl->FileName, AUDIO_GROUP_MUSIC)) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "Theme::Preload - File \"%s\" has already been submitted to the audio manager!\n", tctrl->Name.c_str());
            continue;
        }

        /**
         *  Submit the theme sample to the audio manager for preloading.
         */
        bool submitted = AudioManager.Submit_Sample(
            tctrl->FileName,
            tctrl->FileType,
            AUDIO_GROUP_MUSIC,
            AUDIO_PRIORITY_CRITICAL,
            AUDIO_CONTROL_NORMAL,
            AUDIO_SOUND_NORMAL,
            1);

        if (submitted) {
            AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Preload - Submitted \"%s\" to audio manager.\n", tctrl->FileName.c_str());
        } else {
            AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THEME, "Theme::Preload - Failed to submit \"%s\" to audio manager!\n", tctrl->FileName.c_str());
        }
    }
}


/**
 *  Set the theme data for scenario and owner.
 *
 *  @author: CCHyper
 */
void AudioThemeClass::Set_Theme_Data(ThemeType theme, int scenario, SideType owners)
{
    if (Is_Valid_Theme(theme)) {
        Themes[theme]->Normal = true;
        Themes[theme]->Scenario = scenario;
        Themes[theme]->Owner = owners;
    }
}


/**
 *  Reads and populates this theme's properties from the INI database.
 *
 *  @author: CCHyper
 */
bool AudioThemeClass::ThemeControl::Fill_In(CCINIClass const &ini)
{
    if (!ini.Is_Present(Name.c_str())) {
        return false;
    }

    char buffer[256];
    const char* name = Name.c_str();

    Scenario = ini.Get_Int(name, "Scenario", Scenario);
    Duration = ini.Get_Float(name, "Length", Duration);
    Normal = ini.Get_Bool(name, "Normal", Normal);
    Repeat = ini.Get_Bool(name, "Repeat", Repeat);

    /**
     *  #issue-764
     *
     *  Read the required addon for this theme to be available.
     *
     *  @author: CCHyper
     */
    RequiredAddon = static_cast<AddonType>(ini.Get_Int(name, "RequiredAddon", RequiredAddon));

    /**
     *  Parse the comma-separated side ownership list for this theme.
     *
     *  @author: CCHyper, ZivDero
     */
    if (ini.Get_String(name, "Side", "", buffer, sizeof(buffer)) > 0) {
        const char * token = std::strtok(buffer, ",");
        while (token) {
            SideType side = SideClass::From_Name(token);
            if (side != SIDE_NONE) {
                if (Owner == -1) {
                    Owner = 0;
                }
                Owner |= 1 << side;
                AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme: Setting side %s for %s.\n", token, name);
            }
            token = std::strtok(nullptr, ",");
        }
    }

    Volume = std::clamp<float>(ini.Get_Float(name, "Volume", Volume), AUDIO_VOLUME_MIN, AUDIO_VOLUME_MAX);
    Sound = ini.Get_String(name, "Sound", Sound);
    Fullname = ini.Get_String(name, "Name", Fullname);
    Artist = ini.Get_String(name, "Artist", Artist);

    return true;
}


/**
 *  Reads and applies side ownership overrides for all themes from the INI database.
 *
 *  @author: CCHyper
 */
bool AudioThemeClass::Init_Themes(CCINIClass const &ini)
{
    for (ThemeType theme = THEME_FIRST; theme < Themes.Count(); ++theme) {

        ThemeControl* tctrl = Themes[theme];

        if (!ini.Is_Present(tctrl->Name.c_str())) {
            continue;
        }

        char buffer[256];
        const char * name = tctrl->Name.c_str();

        /**
         *  Parse the comma-separated side ownership list for this theme.
         *
         *  @author: CCHyper
         */
         //Owner = ini.Get_SideType(name, "Side", Owner);
        if (ini.Get_String(name, "Side", "", buffer, sizeof(buffer)) > 0) {
            const char * token = std::strtok(buffer, ",");
            while (token) {
                SideType side = SideClass::From_Name(token);
                if (side != SIDE_NONE) {
                    if (tctrl->Owner == -1) tctrl->Owner = 0;
                    tctrl->Owner |= 1 << side;
                    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme: Setting side %s for %s.\n", token, name);
                }
                token = std::strtok(nullptr, ",");
            }
        }

    }

    return true;
}
