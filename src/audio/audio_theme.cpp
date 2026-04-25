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
 */
const char * AudioThemeClass::Full_Name(ThemeType theme) const
{
    if (Is_Valid_Theme(theme)) {
        return Themes[theme]->Fullname.c_str();
    }
    return nullptr;
}


/**
 *  Process the theme engine and restart songs.
 */
void AudioThemeClass::AI()
{
    /**
     *  If there is no sound driver, no score file present, or sounds have been
     *  specifically turned off then abort.
     */
    if (!AudioManager.Is_Available() || !ScoresPresent || Debug_Quiet) {
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
        DEBUG_INFO("Theme::AI - Next_Song returned \"%s\".\n", Themes[Pending]->Name.c_str());
    }
    
    /**
     *  Start the song playing.
     */
    DEBUG_INFO("Theme::AI - About to call Play_Song with \"%s\".\n", Themes[Pending]->Name.c_str());
    Play_Song(Pending);

    /**
     *  Now flag it so that a new song will be picked when this one ends.
     */
    Pending = THEME_PICK_ANOTHER;
}


/**
 *  Calculates the next song number to play.
 */
ThemeType AudioThemeClass::Next_Song(ThemeType theme) const
{
    /**
     *  If the current theme is set to repeat, return it again.
     */
    if (Is_Valid_Theme(theme) && theme > THEME_FIRST && (Themes[theme]->Repeat || IsRepeat)) {
        return theme;
    }

    if (IsShuffle) {

        /**
         *  Shuffle the theme, but never pick the same theme that was just
         *  playing.
         */
        int tries = 0;
        bool maxed = false;
        ThemeType newtheme;
        while (tries++ <= 1000) {
            newtheme = Sim_Random_Pick(THEME_FIRST, (ThemeType)Themes.Count()-1);
            theme = newtheme;
            maxed = tries == 1000;
            if (newtheme != theme || Is_Allowed(newtheme)) {
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
        for (int i = Themes.Count()+1; i > 0; --i) {
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
 */
void AudioThemeClass::Queue_Song(ThemeType theme)
{
    /**
     *  If there is no sound driver, no score file present, or sounds have been
     *  specifically turned off then abort.
     */
    if (!AudioManager.Is_Available() || !ScoresPresent || Debug_Quiet) {
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
            DEBUG_INFO("Theme::Queue_Song - Queued \"%s\".\n", Themes[theme]->Name.c_str());
        } else {
            DEBUG_INFO("Theme::Queue_Song - Pending %d, theme %d.\n", Pending, theme);
        }

        Pending = theme;

        if (Still_Playing()) {
            //DEBUG_INFO("Theme::Queue_Song - Fading out \"%s\"...\n", Themes[Score]->Name.c_str());
            DEBUG_INFO("Theme::Queue_Song - Fading out current score...\n");
            AudioManager.Request_Stop(ScoreHandle, CrossFade ? CrossFadeSeconds : 0.0f);
            //ScoreHandle->Stop(FadeOutSeconds, true);
        }

    }
}


/**
 *  Starts the specified song play NOW.
 */
bool AudioThemeClass::Play_Song(ThemeType theme)
{
    /**
     *  If there is no sound driver, no score file present, or sounds have been
     *  specifically turned off then abort.
     */
    if (!AudioManager.Is_Available() || !ScoresPresent || Debug_Quiet) {
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
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Play_Song - About to call AudioManager.Play with \"%s\".\n", tctrl->FileName.c_str());
    AudioHandleID handle = AudioManager.Request_Play(tctrl->FileName, AUDIO_GROUP_MUSIC, tctrl->Volume, 1.0f, 0.0f, AUDIO_PRIORITY_HIGH, 1, CrossFade ? CrossFadeSeconds : 0.0f);

    if (handle == INVALID_AUDIO_HANDLE_ID) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_THEME, "Theme::Play_Song - Failed to play \"%s\"!\n", Themes[theme]->Name.c_str());
        return false;
    }

    /**
     *  Stop the previously playing theme if one was active.
     */
    if (ScoreHandle && current > THEME_NONE && current < Themes.Count()) {
        AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Play_Song - Stopping handle for \"%s\"\n", Themes[current]->Name.c_str());
        AudioManager.Request_Stop(ScoreHandle, CrossFade ? CrossFadeSeconds : 0.0f);
        //ScoreHandle->Stop(CrossFade ? CrossFadeSeconds : 0.0f, CrossFade);
    }

    /**
     *  Store the new handle as the active score.
     */
    ScoreHandle = handle;
    
    /**
     *  If this theme is flagged to repeat, set pending.
     */
    if (tctrl->Repeat || IsRepeat) {
        DEBUG_INFO("Theme::Play_Song - Playing \"%s\" (Repeating)\n", Themes[theme]->Name.c_str());
        Pending = theme;

    } else {
        DEBUG_INFO("Theme::Play_Song - Playing \"%s\" (Normal)\n", Themes[theme]->Name.c_str());
    }

    return true;
}


/**
 *  Calculates the length of the song (in seconds).
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
    AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Fade_Out - About to call Queue_Song with THEME_QUIET.\n");
    Queue_Song(THEME_QUIET);
}


/**
 *  Stops the current theme from playing.
 * 
 *  @author: CCHyper
 */
void AudioThemeClass::Stop(bool fade)
{
    /**
     *  If there is no sound driver, no score file present, or sounds have been
     *  specifically turned off then abort.
     */
    if (!AudioManager.Is_Available() || !ScoresPresent || Debug_Quiet) {
        return;
    }

    if (Score == THEME_NONE) {
        //AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "Theme::Stop - Score is null, nothing to stop.\n");
        return;
    }

    if (ScoreHandle == INVALID_AUDIO_HANDLE_ID) {
        //AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "Theme::Stop - Handle is null, nothing to stop.\n");
        return;
    }

    if (Still_Playing()) {
        if (fade) {
            DEBUG_INFO("Theme::Stop - Fading out \"%s\"...\n", Themes[Score]->Name.c_str());
            AudioManager.Request_Stop(ScoreHandle, FadeOutSeconds);
            //ScoreHandle->Stop(FadeOutSeconds, true);

        } else {
            DEBUG_INFO("Theme::Stop - Forced \"%s\" to stop.\n", Themes[Score]->Name.c_str());
            AudioManager.Request_Stop(ScoreHandle);
            //ScoreHandle->Stop();
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
    /**
     *  If there is no sound driver, no score file present, or sounds have been
     *  specifically turned off then abort.
     */
    if (!AudioManager.Is_Available() || !ScoresPresent || Debug_Quiet) {
        return false;
    }

    if (!Is_Valid_Theme(What_Is_Playing())) {
        return false;
    }

    //if (ScoreHandle == INVALID_AUDIO_HANDLE_ID) {
    //    return false;
    //}

    DEBUG_INFO("Theme::Suspend - Suspending score \"%s\"\n", Themes[Score]->Name.c_str());
    
    return AudioManager.Request_Pause(ScoreHandle);
}


/**
 *  Resumes playback of a previously suspended theme.
 *
 *  @author: CCHyper
 */
bool AudioThemeClass::Resume()
{
    /**
     *  If there is no sound driver, no score file present, or sounds have been
     *  specifically turned off then abort.
     */
    if (!AudioManager.Is_Available() || !ScoresPresent || Debug_Quiet) {
        return false;
    }

    if (!Is_Valid_Theme(What_Is_Playing())) {
        return false;
    }

    //if (ScoreHandle == INVALID_AUDIO_HANDLE_ID) {
    //    return false;
    //}

    DEBUG_INFO("Theme::Resume - Resuming score \"%s\"\n", Themes[Score]->Name.c_str());
    
    return AudioManager.Request_Resume(ScoreHandle);
}


/**
 *  Checks if the current theme is paused.
 *
 *  @author: CCHyper
 */
bool AudioThemeClass::Is_Paused() const
{
    //if (ScoreHandle == INVALID_AUDIO_HANDLE_ID) {
    //    return false;
    //}

    return AudioManager.Query_Is_Paused(ScoreHandle);
}


/**
 *  Deletes all theme control entries and clears the theme list.
 *
 *  @author: CCHyper
 */
void AudioThemeClass::Free_Themes()
{
    //Themes.Delete_All();

    while (Themes.Count() > 0) {
        int index = Themes.Count()-1;
        delete Themes[index];
        Themes.Delete(index);
    }
}


/**
 *  Sets the music group volume, converting from 0-255 range to 0.0-1.0.
 *
 *  @author: CCHyper
 */
void AudioThemeClass::Set_Volume(int volume)
{
    float volf = std::clamp(float(volume/255.0f), 0.0f, 1.0f);
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

    char buffer[32];

    if (ini.Is_Present(GENERAL)) {
        FadeOutSeconds = ini.Get_Float(GENERAL, "FadeOutSeconds", FadeOutSeconds);
        CrossFade = ini.Get_Bool(GENERAL, "CrossFading", CrossFade);
        CrossFadeSeconds = ini.Get_Float(GENERAL, "CrossFadeSeconds", CrossFadeSeconds);
    }

    int count = ini.Entry_Count(THEMES);
    for (int index = 0; index < count; ++index) {

        if (ini.Get_String(THEMES, ini.Get_Entry(THEMES, index), "", buffer, sizeof(buffer)-1) > 0) {
            ThemeType theme = From_Name(buffer);

            ThemeControl *ctrl = nullptr;
            if (theme == THEME_NONE) {
                ctrl = new ThemeControl(buffer);
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

    //if (ScoreHandle == INVALID_AUDIO_HANDLE_ID) {
    //    return false;
    //}

    return AudioManager.Query_Is_Playing(ScoreHandle);
}


/**
 *  Checks to see if the specified theme is legal.
 * 
 *  @author: CCHyper
 */
bool AudioThemeClass::Is_Allowed(ThemeType theme) const
{
    //ASSERT(index < Themes.Count()); // Removed as Next_Song goes out of bounds (by design).

    if (theme == THEME_QUIET || theme == THEME_PICK_ANOTHER) {
        return true;
    }

#if 0
    /**
     *  Is the required theme within the available range?
     * 
     *  #NOTE: Removed as Next_Song goes out of bounds (by design).
     */
    if (theme >= Themes.Count()) {
        return false;
    }

    /**
     *  If the theme is not present, then it certainly isn't allowed.
     */
    if (!Themes[theme]->Available) {
        return false;
    }

    /**
     *  Only normal themes (playable during battle) are considered allowed.
     */
    if (!Themes[theme]->Normal) {
        return false;
    }
#else
    /**
     *  Only normal themes (playable during battle) are considered allowed.
     */
    if (!Is_Playable(theme)) {
        return false;
    }
#endif

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
            //AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "Theme::Is_Allowed - \"%s\" is only available for addon %d.\n", Themes[index]->Name.c_str(), addon);
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
        //AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "Theme::Is_Allowed - \"%s\" is not yet available for this scenario (\"%d\").\n", Themes[index]->Name.c_str(), Scen->Scenario);
        return false;
    }

    //AUDIO_DEBUG_MSG(LEVEL_INFO, TYPE_THEME, "Theme::Is_Allowed - \"%s\" is allowed to be played!\n", Themes[index]->Name.c_str());

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

    std::string sname = name;
    string_to_upper(sname);

    /**
     *  First search for an exact name match with the filename
     *  of the theme. This is guaranteed to be unique.
     */
    for (ThemeType theme = THEME_FIRST; theme < Themes.Count(); ++theme) {
        if (Themes[theme]->Name == sname) {
            return theme;
        }
    }

    /**
     *  If the filename scan failed to find a match, then scan for
     *  a substring within the full name of the score. This might
     *  yield a match, but is not guaranteed to be unique.
     */
    for (ThemeType theme = THEME_FIRST; theme < Themes.Count(); ++theme) {
        if (std::strstr(Themes[theme]->Fullname.c_str(), sname.c_str()) != nullptr) {
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

        std::string name = tctrl->Name;

        if (!tctrl->Sound.empty()) {
            name = tctrl->Sound;
        }

        if (!AudioManager.Is_File_Available(name)) {
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "Theme::Scan - File \"%s\" was not found in any supported formats!\n", name.c_str());
            continue;
        }

        tctrl->Available = true;

        /**
         *  Retrieve the file type and full filename for this theme.
         */
        AudioManager.Get_File_Info(name,  tctrl->FileType, tctrl->FileName);
    }

#if 0//#ifndef NDEBUG
    AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "Theme dump...\n");
    for (int index = 0; index < Themes.Count(); ++index) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_THEME, "  %03d  %s\n", index, Themes[index]->Name.c_str());
    }
#endif

    // Call preload here to reduce patches.
    Preload();
}


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
            AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_VOC, "Theme::Preload - File \"%s\" has already been submitted to the audio manager!\n", tctrl->Name.c_str());
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
    RequiredAddon = (AddonType)ini.Get_Int(name, "RequiredAddon", RequiredAddon);

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

    char tmp[512];

    ini.Get_String(name, "Sound", "", tmp, sizeof(tmp));
    Sound = tmp;

    ini.Get_String(name, "Name", "", tmp, sizeof(tmp));
    Fullname = tmp;

    ini.Get_String(name, "Artist", "", tmp, sizeof(tmp));
    Artist = tmp;

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

        ThemeControl *tctrl = Themes[theme];

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
