/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_MANAGER.CPP
 *
 *  @author        CCHyper
 * 
 *  @contributions mackron (miniaudio developer)
 *
 *  @brief         Installable MiniAudio audio driver.
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
 ******************************************************************************/

#include "audio_sample.h"
#include "audio_manager.h"
#include "audio_debug.h"
#include "ccfile.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
AudioSampleClass::AudioSampleClass() :
    AudioSampleBase(),
    FileName(),
    Group(AUDIO_GROUP_NONE),
    Priority(AUDIO_PRIORITY_NORMAL),
    Control(AUDIO_CONTROL_NORMAL),
    Type(AUDIO_SOUND_NORMAL),
    ConcurrentLimit(AUDIO_MAX_CONCURRENT_LIMIT/4) // Users should increase this manually if they want it more prominent
{
}


/**
 *  Class destructor.
 *
 *  @author: CCHyper
 */
AudioSampleClass::~AudioSampleClass()
{
}


/**
 *  Checks whether the sample's source file exists and is accessible.
 *
 *  @author: CCHyper
 */
bool AudioSampleClass::Is_Available() const
{
    if (!CCFileClass(FileName.c_str()).Is_Available()) {
        AUDIO_DEBUG_MSG(LEVEL_ERROR, TYPE_SAMPLE, "AudioSample::Is_Available - Unable to find \"%s\"!\n", FileName.c_str());
        return false;
    }

    return true;
}
