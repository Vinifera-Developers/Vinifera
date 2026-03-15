/*******************************************************************************
/*                  O P E N  S O U R C E -- V I N I F E R A                   **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_NEWVOC.H
 *
 *  @author        CCHyper, tomsons26
 *
 *  @brief         Sound effect (VOC) audio management.
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
#include "voc.h"
#include "wstring.h"


class CCINIClass;
class AudioEventHandleClass;


class AudioVocClass
{
    friend class AudioEventHandleClass;

    friend static void Voc_Calculate_Pan_And_Volume(AudioVocClass &voc, Coord &coord, float &pan_result, float &volume_result);
    friend static bool Voc_Play(AudioVocClass &voc, Coord &coord, int a2, float volume);

    public:
        AudioVocClass(const char *name);
        ~AudioVocClass();

        void Read_INI(CCINIClass &ini);
        bool Can_Play() const;

        int Play(float volume, int a2);
        int Play(float volume);
        int Play(Coord &coord);

        bool Update_Position(Coord &coord);
        bool Stop();

        static int Play(VocType voc, int a2, float volume = 1.0f);
        static int Play(VocType voc, float volume = 1.0f);
        static int Play(VocType voc, Coord &coord);

        static int Play(const char *filename, float volume = 1.0f);

        static void Scan();
        static void ScanAsync();
        static void Preload();
        static void Process(CCINIClass &ini);
        static void Clear();

        static VocType VocType_From_Voc(AudioVocClass *voc);
        static VocType From_Name(const char *name);
        static AudioVocClass *Voc_From_Name(const char *name);
        static const char *INI_Name_From(VocType type);

        static const AudioVocClass *As_Pointer(VocType type)
        {
            return type != VOC_NONE && type < Vocs.Count() ? (AudioVocClass *)Vocs[type] : nullptr;
        }

        //static bool Update_Audio_Event(AudioEventHandleClass &event, Coord &coord);
        //static bool Stop_Audio_Event(AudioEventHandleClass &event, Coord &coord);

        static void Set_Volume(int volume);

    private:
        /**
         *  Handle to current sound.
         */
        AudioHandleID Handle;

        /**
         *  Name of the sound event (up to 31 characters).
         */
        std::string Name;

        /**
         *  The file type of this sound.
         */
        AudioFileType FileType;

        /**
         *  #NEW: Full filename of the sound effect.
         */
        std::string FileName;

        /**
         *  #NEW: Is the sound available?
         */
        bool Available;

        /**
         *  #NEW: Is a special "event"? eg. Ambient sounds
         */
        bool IsSpecialEvent;

        /**
         *  Up to 32 sound files can be associated with an audio event. Do not specify
         *  path or extension of the file, just the name of the file. The file name must
         *  not be more that 31 characters, and do not use spaces in the filename!
         */
        DynamicVectorClass<std::string> Sounds;
        
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
         *  Limit specifies the maximum number of instances of an audio event type 
         *  that can be played SIMULTANEOUSLY. Limit of one, along with IMMEDIATE 
         *  control, can be used to achieve monaural sounds.
         */
        int Limit;
        
        /**
         *  Volume level playback for audio event. We assume that all sounds are 
         *  normalized. Use this attribute to set the mixing levels for audio.
         *  The value specified is the percentage of full volume. e.g. 0.25 means 
         *  playback at one quarter of full volume.
         */
        float Volume;

        /**
         *  The minimum volume for the GLOBAL Type event (ignored for all other types).
         */
        float MinVolume;

        // Not loaded from the INI!
        float MaxVolume;

        /**
         *  The FShift attribute alows the playback frequency to be randomly changed
         *  in order to give some variance to repetitive sounds. The engine uses a
         *  percentage value to represent the frequency of a sound. 100 percent
         *  means normal frequency. 50 percent means half the frequency. 200 percent
         *  means twice the frequency, and so on.
         *
         *  The minDelta and maxDelta values describe a range relative to 100 percent
         *  frequency. For example:
         *
         *     Volume = 80
         *     VShift = -5, 5     ; vary "Volume" between 75 and 85.
         */
        TPoint2D<int> VolumeShift;

        /**
         *  The FShift attribute alows the playback frequency to be randomly changed
         *  in order to give some variance to repetitive sounds. The engine uses a
         *  percentage value to represent the frequency of a sound. 100 percent
         *  means normal frequency. 50 percent means half the frequency. 200 percent
         *  means twice the frequency, and so on.
         * 
         *  The minDelta and maxDelta values describe a range relative to 100 percent
         *  frequency. For example:
         * 
         *     FShift = -5, 5      ; vary the frequency between (100 - 5) and (100 + 5)
         *     FShift = 5, 10      ; vary the frequency between 105% and 110% of original
         *     FShift = -50, 0     ; vary the frequency between 50% and 100% of original
         */
        TPoint2D<int> FrequencyShift;

        /**
         *  Specifies the audible range of a sound in game cells.
         */
        int Range;

        /**
         *  Type information allows the game engine to modify event behaviour.
         */
        AudioSoundType Type;

        /**
         *  The control attributes can be used in any combination to achieve desired
         *  playback effects. The default control behaviour is to play the first sound
         *  in the sound list just once.
         */
        AudioControlType Control;

        /**
         *  Do not touch!
         */
        AudioGroupType Group;
};
