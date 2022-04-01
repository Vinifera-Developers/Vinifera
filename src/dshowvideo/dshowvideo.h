/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DSHOWVIDEO.H
 *
 *  @author        CCHyper
 *
 *  @brief         DirectShow video player interface.
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
#include <windows.h>
#include <dshow.h>
#include <d3d9.h>
#include <control.h>
#include <string>


/**
 * 
 *  Direct show video player heavily based on Windows SDK sample code.
 * 
 */


class BaseVideoRenderer;


enum PlaybackState
{
    STATE_NO_GRAPH,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_STOPPED,
};


/**
 *  Supported video format types.
 */
enum FormatType
{
    FORMAT_MP4,
    FORMAT_WMV,
    FORMAT_MPG,
    FORMAT_AVI,
    FORMAT_COUNT,

    FORMAT_NONE = -1
};


/**
 *  Un-stretched video size.
 */
#define NORMAL_VIDEO_WIDTH 720
#define NORMAL_VIDEO_HEIGHT 400

const long MIN_VOLUME = -10000;
const long MAX_VOLUME = 0;

const LONG ONE_MSEC = 10000;                // The number of 100-ns in 1 msec.
const LONG HALF_SEC = (ONE_MSEC*1000)/2;
const LONG ONE_SEC = (ONE_MSEC*1000);

const UINT WM_GRAPH_EVENT = WM_APP + 1;


struct GraphEventCallback
{
	virtual void OnGraphEvent(long eventCode, LONG_PTR param1, LONG_PTR param2) = 0;
};


class DirectShowVideoPlayer
{
    public:
        DirectShowVideoPlayer();
        DirectShowVideoPlayer(HWND hWnd, const char *pszFileName);
        ~DirectShowVideoPlayer();

        HRESULT Open_File(const char *pszFileName);
        void Reset();

        static BOOL Is_File_Available(const char *pszFileName, std::string *dest = nullptr);

        /**
         *  VMR functionality
         */
        BOOL Has_Video() const;
        BOOL Update_Video_Window(const LPRECT prc);
        HRESULT Set_Video_Window(HWND hWnd);
        HRESULT Repaint(HDC hdc);
        HRESULT Display_Mode_Changed();

        /**
         *  Events
         */
        HRESULT Handle_Graph_Event(GraphEventCallback  *pfnOnGraphEvent);

        /**
         *  Streaming
         */
        HRESULT Play();
        HRESULT Play_Callback();
        HRESULT Pause();
        HRESULT Stop();

        /**
         *  Seeking
         */
        BOOL Can_Seek() const;
        HRESULT Set_Position(REFERENCE_TIME pos);
        HRESULT Get_Duration(LONGLONG *pDuration);
        HRESULT Get_Current_Position(LONGLONG *pTimeNow);

        /**
         *  Audio
         */
        HRESULT Mute(BOOL bMute);
        BOOL Is_Muted() const { return IsMuted; }
        HRESULT Set_Volume(float lVolume);
        float Get_Volume() const { return powf(10, (float)CurrentVolume/4000.0); }

        /**
         *  Frame stepping
         */
        HRESULT Step_One_Frame();
        HRESULT Step_Frames(int nFramesToStep);

        /**
         *  Playback
         */
        BOOL Is_Finished();

        HRESULT Modify_Rate(double dRateAdjust);
        HRESULT Set_Rate(double dRate);

        void Set_Breakout(bool breakout) { IsBreakoutAllowed = breakout; }
        void Set_Stretch(bool stretch) { IsScalingAllowed = stretch; }

        void Set_HWND(HWND new_hWnd) { hWnd = new_hWnd; }

        PlaybackState Get_State() const { return State; }
        FormatType Get_Format() const { return Format; }

        const char *Get_Filename() const { return Filename; }
        const char *Get_Filename_NoExt() const { return FilenameNoExt; }

    protected:
        HRESULT Initialize_Graph();
        void Tear_Down_Graph();
        HRESULT Create_Video_Renderer();
        HRESULT Render_Streams(IBaseFilter *pSource);
        HRESULT Update_Volume();
        void Stop_Events();

    private:
        HWND hWnd;              // Video window. This window also receives graph events.
        PlaybackState State;
        FormatType Format;
        DWORD SeekCaps;         // Caps bits for IMediaSeeking
        BOOL IsAudioStream;     // Is there an audio stream?
        long CurrentVolume;     // Current volume (unless muted)
        BOOL IsMuted;           // Is the volume muted?
        double PlaybackRate;
        BOOL IsBreakoutAllowed;
        BOOL IsScalingAllowed;
        const char *Filename;
        const char *FilenameNoExt;
        LONG VideoWidth;
        LONG VideoHeight;
        IGraphBuilder *Graph;
        IBasicVideo *Video;
        IMediaControl *Control;
        IMediaEventEx *Event;
        IMediaSeeking *Seek;
        IMediaPosition *Position;
        IBasicAudio *Audio;
        IVideoFrameStep *FrameStep;
        BaseVideoRenderer *VideoRenderer;
};
