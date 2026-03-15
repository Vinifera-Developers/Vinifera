/*******************************************************************************
/*                  O P E N  S O U R C E -- V I N I F E R A                   **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_VOX.H
 *
 *  @author        CCHyper, tomsons26
 *
 *  @brief         EVA speech/voice audio management.
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
#pragma once

#include "always.h"
#include "tibsun_defines.h"
#include "audio_manager.h"
#include "vector.h"
#include "wstring.h"


class CCINIClass;


/**
 *  A reimplementation of Vox interface for use with the new audio driver interface.
 */
class AudioVoxClass
{
    public:
        AudioVoxClass(std::string name);
        ~AudioVoxClass();

        void Read_INI(CCINIClass &ini);

        static void One_Time();

        static bool Process(CCINIClass &ini);
        static void Scan();
        static void ScanAsync();
        static void Preload();
        static void Clear();

        static void Speak(VoxType voice, bool force = false);
        static void AI();
        static void Stop_Speaking();
        static bool Is_Speaking();
        static void Set_Speech_Volume(int vol);

#ifndef NDEBUG
        static bool Write_Default_Speech_INI(CCINIClass &ini);
#endif

        static VoxType From_Name(const char *name);
        static const char *Name_From(VoxType type);

        static void Set_Speech_Allowed(bool set);
        static bool Is_Speech_Allowed();

    private:
        /**
         *  The file type of this speech.
         */
        AudioFileType FileType;

        /**
         *  The resolved filename for this speech audio file.
         */
        std::string FileName;

        /**
         *  Is the speech available?
         */
        bool Available;

        /**
         *  Name of this speech event (up to 31 characters).
         */
        std::string Name;
        
        /**
         *  Text description for this speech line, for debugging purposes only.
         */
        std::string DescriptionText;

        /**
         *  The name override of this speech event (up to 31 characters).
         */
        std::string Sound;

        /**
         *  Priority and Limit are the most important attributes of them all. While
         *  possibly hundreds of audio events want to trigger every frame, only a few
         *  will be chosen. It is vitally important to ensure that important events
         *  do not get dropped. The audio engine uses the priority of the audio event
         *  when choosing which events to drop. So make priorities are set correctly
         *  for all events.
         */
        int Priority;
        
        /**
         *  The playback volume for this speech entry.
         */
        float Volume;

        /**
         *  The minimum and maximum volume bounds for this speech entry.
         */
        float MinVolume;
        float MaxVolume;

        /**
         *  Delay in seconds before this speech begins playback.
         */
        float Delay;

        /**
         *  Pitch multiplier applied to this speech during playback.
         */
        float FrequencyShift;

        /**
         *  The sound type flags for this speech entry.
         */
        AudioSoundType Type;

        /**
         *  The playback control flags for this speech entry.
         */
        AudioControlType Control;

    private:
        static bool IsSpeechAllowed;
};
