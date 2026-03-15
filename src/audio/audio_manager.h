/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          AUDIO_MANAGER.H
 *
 *  @author        CCHyper
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
#pragma once

#include "always.h"
#include "wstring.h"
#include "vector.h"
#include "audio_defines.h"
#include "debughandler.h"
#include "asserthandler.h"

#include <unordered_map>
#include <unordered_set>

#include <memory>                // for std::shared_ptr
#include <atomic>
#include <chrono>                // for time tracking
#include <thread>                // for sleep_until
#include <mutex>                 // for std::mutex, std::lock_guard
#include <queue>                 // for std::queue
#include <condition_variable>


/**
 *  Forward declarations.
 */
struct ma_engine;
struct ma_device;
struct ma_sound;
typedef ma_sound ma_sound_group;
class AudioSampleClass;


/**
 *  Central audio manager that handles playback, sample management, and group control via miniaudio.
 */
class AudioManagerClass
{
    friend class AudioSampleClass;
    friend class AudioInstanceClass;
    friend class AudioStreamingClass;

    public:
        AudioManagerClass();
        virtual ~AudioManagerClass();

        /**
         *  Audio engine IO.
         */
        virtual bool Init(HWND hWnd);
        virtual void End();

        virtual bool Is_Available() const;
        virtual bool Is_Enabled() const;

        virtual void Enable();
        virtual void Disable();

        virtual bool Start_Engine(bool forced = false);
        virtual bool Stop_Engine();

        virtual void Focus_Loss();
        virtual void Focus_Restore();

        virtual void Sound_Callback() {}

        /**
         *  Sound playback control.
         */
        virtual AudioHandleID Request_Play(std::string filename, AudioGroupType group, float volume = 1.0f, float pitch = 1.0f, float pan = 0.0f, AudioPriorityType priority = AUDIO_PRIORITY_NORMAL, int limit = -1, float fade_in_seconds = 0.0f, float delay_in_seconds = 0.0f, bool start = true, bool looping = false);
        virtual bool Request_Stop(AudioHandleID handle, float fade_out = 0.0f);

        virtual bool Request_Pause(AudioHandleID handle);
        virtual bool Request_Resume(AudioHandleID handle);

        /**
         *  Query functions.
         */
        virtual bool Query_Is_Playing(AudioHandleID handle);
        virtual bool Query_Is_Paused(AudioHandleID handle);

        //virtual bool Query_Sample_Ready(AudioHandleID handle, AudioGroupType group);
        virtual bool Query_Sample_Ready(std::string name, AudioGroupType group);
        
        /**
         *  Set properties functions.
         */
        virtual bool Set_Volume(AudioHandleID handle, float volume);
        virtual bool Set_Pan(AudioHandleID handle, float pan);
        virtual bool Set_Pitch(AudioHandleID handle, float pitch);

        /**
         *  Submission functions.
         */
        virtual bool Submit_Sample(std::string &filename, AudioFileType filetype, AudioGroupType group, AudioPriorityType priority, AudioControlType control, AudioSoundType type, unsigned int limit);
        virtual bool Clear_Samples(AudioGroupType group = AUDIO_GROUP_NONE);
        virtual bool Has_Been_Submitted(std::string &filename, AudioGroupType group = AUDIO_GROUP_NONE);

        void Lock_Submissions() { SubmissionsLocked = true; }

        /**
         *  Master volume control.
         */
        virtual bool Set_Master_Volume(float volume) const;

        /**
         *  Audio group control.
         */
        virtual float Get_Group_Volume(AudioGroupType group);
        virtual bool Set_Group_Volume(AudioGroupType group, float volume);
        virtual bool Is_Group_Playing(AudioGroupType group) const;
        virtual bool Start_Group(AudioGroupType group) const;
        virtual bool Stop_Group(AudioGroupType group) const;
        virtual bool Stop_And_Fade_Out_Group(AudioGroupType group, float duration) const;

        /**
         *  DirectSound access.
         */
        void * Get_DirectSound_Object() const;
        void * Get_DirectSound_Primary_Buffer() const;
        void * Get_DirectSound_Buffer() const;

        /**
         *  Supported formats query functions.
         */
        virtual bool Is_File_Available(AudioFileType type, std::string name) const;
        virtual bool Is_File_Available(std::string name) const;
        bool Is_FileType_Supported(AudioFileType type) const;
        std::string Build_Filename_From_Type(AudioFileType type, std::string name);

        bool Get_File_Info(std::string name, AudioFileType &filetype, std::string &filename, bool ignore_error = false);

        /**
         *  Utility functions.
         */
        int AudioPriority_To_Priority(AudioPriorityType priority);
        AudioPriorityType Priority_To_AudioPriority(int priority);
        unsigned int fVolume_To_iVolume(float vol);
        float iVolume_To_fVolume(unsigned int vol);

        bool Is_Handle_Valid(AudioHandleID handle);

        /**
         *  VQ Audio streaming support
         */
        bool Open_VQ_Audio_Stream(const std::string& name, int sampleRate, int channels, int bitsPerSample, float volume = 1.0f);
        bool Push_VQ_Audio_Chunk(const void* data, size_t size);
        bool Play_VQ_Audio_Stream();
        bool Pause_VQ_Audio_Stream();
        bool Resume_VQ_Audio_Stream();
        bool Stop_VQ_Audio_Stream();
        bool Close_VQ_Audio_Stream();
        bool Is_VQ_Audio_Stream_Playing() const;

#ifndef NDEBUG
        bool Create_Debug_Window(/*HINSTANCE hInstance*/);
        bool Close_Debug_Window();
        void Debug_Window_Message_Handler();
        void Debug_Window_Loop();
#endif

    private:
        /**
         *  Active handle management
         */
        bool Add_Active_Handle(std::unique_ptr<AudioInstanceClass> handle);
        bool Add_Active_Handle_NoLock(std::unique_ptr<AudioInstanceClass> handle);
        bool Remove_Active_Handle(AudioHandleID id);
        bool Remove_Active_Handle_NoLock(AudioHandleID id);
        bool Clear_All_Active_Handles();

        AudioSampleClass * Find_Sample(const std::string & filename, AudioGroupType group);

    private:
        static unsigned __stdcall CleanupThreadFunction(void * context);

        /**
         *  Utility functions to handle unique handle id's
         */
        static AudioHandleID Generate_Unique_Audio_ID(AudioGroupType group);
        static bool Is_Valid_Audio_ID(AudioHandleID id, AudioGroupType group);
        static AudioInstanceClass * Find_Handle_By_ID(AudioHandleID id);
        static AudioInstanceClass * Find_Handle_By_ID_NoLock(AudioHandleID id);

    private:
        /**
         *  The miniaudio engine and device instances.
         */
        ma_engine *Engine;
        ma_device *Device;

        /**
         *  Sound groups for per-group volume and playback control.
         */
        ma_sound_group *SoundGroups[AUDIO_GROUP_COUNT];

        /**
         *  Has the audio engine been successfully initialized?
         */
        bool IsInitialized;

        /**
         *  When the window loses focus, the current engine volume is stored here
         *  so it can be restored when focus is restored.
         */
        float FocusRestoreVolume;

        /**
         *  Tracks all the currently playing sounds.
         */
        std::unordered_map<AudioHandleID, std::unique_ptr<AudioInstanceClass> > ActiveInstanceMap;
        
        /**
         *  Tracks all the currently playing sounds in their respective groups types. We can use raw
         *  pointers here and ActiveInstanceMap has ownership of the audio instances being added.
         */
        std::unordered_map<AudioGroupType, std::vector<AudioInstanceClass *>> GroupedActiveInstanceMap;
        
        /**
         *  Stores all available samples submitted to the manager.
         */
        std::unordered_map<AudioSampleKey, std::unique_ptr<AudioSampleClass> > SamplesMap;

        /**
         *  Lock down the manager to accepting any further submissions.
         */
        bool SubmissionsLocked;

    private:
        /**
         *  Types of audio requests that can be queued for processing.
         */
        typedef enum AudioRequestType {
            AUDIO_REQUEST_PLAY,
            AUDIO_REQUEST_STOP
        } AudioRequestType;

        /**
         *  Queued audio playback or stop request with all associated parameters.
         */
        typedef struct AudioRequest  {

            AudioRequestType Type;

            // For both Play and Stop
            AudioHandleID HandleID = INVALID_AUDIO_HANDLE_ID;

            // Only for Play
            std::string Filename;
            AudioGroupType Group = AUDIO_GROUP_NONE;
            float Volume = 1.0f;
            float Pitch = 1.0f;
            float Pan = 0.0f;
            AudioPriorityType Priority = AUDIO_PRIORITY_NORMAL;
            int Limit = -1;
            float FadeInSeconds = 0.0f;
            float DelayInSeconds = 0.0f;
            bool StartImmediately = true;
            bool Loops = false;

            // Only for Stop
            float FadeOutSeconds = 0.0f;

            AudioRequest() = default;

            // Constructor for Play
            AudioRequest(AudioHandleID id, std::string filename, AudioGroupType group,
                         float volume, float pitch, float pan,
                         AudioPriorityType priority, int limit,
                         float fadeIn, float delay, bool start, bool looping) :
                HandleID(id),
                Type(AudioRequestType::AUDIO_REQUEST_PLAY),
                Filename(std::move(filename)),
                Group(group),
                Volume(volume),
                Pitch(pitch),
                Pan(pan),
                Priority(priority),
                Limit(limit),
                FadeInSeconds(fadeIn),
                DelayInSeconds(delay),
                StartImmediately(start),
                Loops(looping)
            {
            }

            // Constructor for Stop
            AudioRequest(AudioHandleID id, float fade_out) :
                Type(AudioRequestType::AUDIO_REQUEST_STOP),
                HandleID(id),
                FadeOutSeconds(fade_out)
            {
            }

        } AudioRequest;

    private:
        /**
         *  Background thread and synchronization primitives for cleanup and request processing.
         */
        std::thread CleanupThread;
        std::atomic<bool> ThreadExitFlag { false };
        std::condition_variable_any ThreadWakeSignal;
        std::mutex ThreadMutex;

        std::queue<AudioRequest> RequestQueue;
        std::mutex RequestMutex;
        std::condition_variable_any RequestCV;

        /**
         *  IDs of requests submitted to RequestQueue but not yet processed by the worker.
         *  Allows Query_Is_Playing to return true for in-flight requests without polling.
         */
        std::unordered_set<AudioHandleID> PendingHandleIDs;
        std::mutex PendingMutex;

        /**
         *  Requests with AUDIO_CONTROL_QUEUE that were deferred due to concurrent limit.
         *  Worker-thread-only — no mutex required.
         */
        std::queue<AudioRequest> DeferredPlayQueue;

        std::mutex SubmissionMutex;
};


/**
 *  Global audio manager instance.
 */
extern AudioManagerClass AudioManager;
