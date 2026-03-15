/*******************************************************************************
/*                  O P E N  S O U R C E -- V I N I F E R A                    *
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_IONSTORM.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Ion storm ambient sound effect management.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 *  @note          This file contains heavily modified code from the source code
 *                 released by Electronic Arts for the C&C Remastered Collection
 *                 under the GPL3 license. Source:
 *                 https://github.com/ElectronicArts/CnC_Remastered_Collection
 *
 ******************************************************************************/

#include "audio_ambient.h"
#include "audio_manager.h"
#include "audio_util.h"
#include "audio_debug.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"


DynamicVectorClass<AudioAmbientClass *> AudioAmbients;


/**
 *  Constructor; initializes the ambient sound with the given name and registers it.
 *
 *  @author: CCHyper
 */
AudioAmbientClass::AudioAmbientClass(std::string name) :
	Handle(INVALID_AUDIO_HANDLE_ID),
    Name(name),
    FadeIn(true),
    FadeOut(true),
    FadeSeconds(1.0f)
{
    string_to_upper(Name);

    AudioAmbients.Add(this);
}


/**
 *  Destructor; unregisters this ambient sound from the global list.
 *
 *  @author: CCHyper
 */
AudioAmbientClass::~AudioAmbientClass()
{
    AudioAmbients.Delete(this);
}


/**
 *  Starts playback of the ambient sound with fade-in and a short initial delay.
 *
 *  @author: CCHyper
 */
bool AudioAmbientClass::Start()
{
    if (!AudioManager.Is_Available() || Debug_Quiet) {
        return false;
    }

    if (Handle != INVALID_AUDIO_HANDLE_ID) {
        return true;
    }

    /**
        *  Resolve the file type and full filename for this ambient sound.
        */
    AudioManager.Get_File_Info(Name, FileType, FileName);

    if (!IsAvailable) {
        AUDIO_DEBUG_MSG(LEVEL_WARNING, TYPE_AMBIENT, "Ambient: Unable to find \"%s\" ambient track!\n", Name.c_str());
        return false;
    }

    bool start = true;
    float delay = 1.0f; // So it does not start during the static screen effect.

    AudioHandleID handle = AudioManager.Request_Play(FileName, AUDIO_GROUP_AMBIENT, Volume, 1.0f, 0.0f, AUDIO_PRIORITY_CRITICAL, -1, FadeSeconds, delay, start);
    ASSERT(handle != INVALID_AUDIO_HANDLE_ID);

    Handle = handle;

    return true;
}


/**
 *  Stops playback of the ambient sound with a fade-out.
 *
 *  @author: CCHyper
 */
bool AudioAmbientClass::Stop()
{
    return AudioManager.Request_Stop(Handle, FadeSeconds); // Nice and smooth!
}


/**
 *  Pauses playback of the ambient sound.
 *
 *  @author: CCHyper
 */
bool AudioAmbientClass::Pause()
{
    return AudioManager.Request_Pause(Handle);
}


/**
 *  Resumes playback of a paused ambient sound.
 *
 *  @author: CCHyper
 */
bool AudioAmbientClass::Resume()
{
    return AudioManager.Request_Resume(Handle);
}


/**
 *  Checks whether the ambient sound is currently playing.
 *
 *  @author: CCHyper
 */
bool AudioAmbientClass::Is_Playing()
{
    return AudioManager.Query_Is_Playing(Handle);
}
