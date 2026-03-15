/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_SAMPLE.H
 *
 *  @author        CCHyper
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
#include "wstring.h"
#include "audio_defines.h"
#include "debughandler.h"
#include "asserthandler.h"


struct ma_sound;
struct ma_decoder;
typedef ma_sound ma_sound_group;


/**
 *  
 */
class AudioSampleBase
{
    public:
        AudioSampleBase() {}
        virtual ~AudioSampleBase() {}
};


/**
 *  
 */
class AudioSampleClass : public AudioSampleBase
{
    friend class AudioManagerClass;

    public:
        AudioSampleClass();
        virtual ~AudioSampleClass();

        std::string Get_FileName() const { return FileName; }

        AudioGroupType Get_Group() const { return Group; }
        AudioPriorityType Get_Priority() const { return Priority; }
        AudioControlType Get_Control() const { return Control; }
        AudioSoundType Get_Type() const { return Type; }
        int Get_Limit() const { return ConcurrentLimit; }

        void Set_Group(AudioGroupType group) { Group = group; }
        void Set_Priority(AudioPriorityType priority) { Priority = priority; }
        void Set_Control(AudioControlType control) { Control = control; }
        void Set_Type(AudioSoundType type) { Type = type; }
        void Set_Limit(int limit) { ConcurrentLimit = std::clamp(0, AUDIO_MAX_CONCURRENT_LIMIT, limit);; }

        bool Is_Available() const;

    protected:
        /**
         *  Original source filename.
         */
        std::string FileName;
        
        /**
         *  The filetype/ext of this file.
         */
        AudioFileType FileType;

        /**
         *  Sound group classification.
         */
        AudioGroupType Group;

        /**
         *  Playback priority.
         */
        AudioPriorityType Priority;

        /**
         *  Control flags or rules.
         */
        AudioControlType Control;

        /**
         *  Sound type (sample/stream/etc).
         */
        AudioSoundType Type;

        /**
         *  Max simultaneous plays of this sample at one time.
         */
        int ConcurrentLimit;

    private:
        // Disable copy semantics
        AudioSampleClass(const AudioSampleClass & other);
        AudioSampleClass & operator = (const AudioSampleClass &) = delete;
};
