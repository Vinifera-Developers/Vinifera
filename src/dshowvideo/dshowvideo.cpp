/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          DSHOWVIDEO.CPP
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
#include "dshowvideo.h"
#include "dshowrenderer.h"
#include "dshowutil.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "vinifera_util.h"
#include "options.h"
#include "ccfile.h"
#include "ini.h"
#include "winutil.h"
#include "wwkeyboard.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <string>
#include <new>


/**
 *  Default constructor.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
DirectShowVideoPlayer::DirectShowVideoPlayer() :
    hWnd(nullptr),
    State(STATE_NO_GRAPH),
    Format(FORMAT_NONE),
    SeekCaps(0),
    IsAudioStream(false),
    CurrentVolume(MAX_VOLUME),
    IsMuted(false),
    PlaybackRate(1.0),
    IsBreakoutAllowed(false),
    IsScalingAllowed(false),
    Filename(nullptr),
    FilenameNoExt(nullptr),
    VideoWidth(NORMAL_VIDEO_WIDTH),
    VideoHeight(NORMAL_VIDEO_HEIGHT),
    Graph(nullptr),
    Video(nullptr),
    Control(nullptr),
    Event(nullptr),
    Seek(nullptr),
    Position(nullptr),
    Audio(nullptr),
    FrameStep(nullptr),
    VideoRenderer(nullptr)
{
}


/**
 *  Constructor that takes window handle and filename.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
DirectShowVideoPlayer::DirectShowVideoPlayer(HWND hWnd, const char *pszFileName) :
    hWnd(nullptr),
    State(STATE_NO_GRAPH),
    Format(FORMAT_NONE),
    SeekCaps(0),
    IsAudioStream(false),
    CurrentVolume(MAX_VOLUME),
    IsMuted(false),
    PlaybackRate(1.0),
    IsBreakoutAllowed(false),
    IsScalingAllowed(false),
    Filename(nullptr),
    FilenameNoExt(nullptr),
    VideoWidth(NORMAL_VIDEO_WIDTH),
    VideoHeight(NORMAL_VIDEO_HEIGHT),
    Graph(nullptr),
    Video(nullptr),
    Control(nullptr),
    Event(nullptr),
    Seek(nullptr),
    Position(nullptr),
    Audio(nullptr),
    FrameStep(nullptr),
    VideoRenderer(nullptr)
{
    Open_File(pszFileName);
}


/**
 *  
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
DirectShowVideoPlayer::~DirectShowVideoPlayer()
{
    Reset();
}


/**
 *  Reset the video player instance. 
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
void DirectShowVideoPlayer::Reset()
{
    Tear_Down_Graph();

    hWnd = nullptr;
    State = STATE_NO_GRAPH;
    Format = FORMAT_NONE;
    SeekCaps = 0;
    IsAudioStream = false;
    CurrentVolume = MAX_VOLUME;
    IsMuted = false;
    PlaybackRate = 1.0;
    IsBreakoutAllowed = false;
    IsScalingAllowed = false;
    Filename = nullptr;
    FilenameNoExt = nullptr;
    VideoWidth = NORMAL_VIDEO_WIDTH;
    VideoHeight = NORMAL_VIDEO_HEIGHT;
    Graph = nullptr;
    Video = nullptr;
    Control = nullptr;
    Event = nullptr;
    Seek = nullptr;
    Position = nullptr;
    Audio = nullptr;
    FrameStep = nullptr;
    VideoRenderer = nullptr;
}


/**
 *  Open a media file ready for playback.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Open_File(const char *pszFileName)
{
    IBaseFilter *pSource = nullptr;
    std::string dest_filename;
    HRESULT hr;

    /**
     *  Create a new filter graph (This also closes the old one).
     */
    hr = Initialize_Graph();
    if (FAILED(hr)) {
        goto done;
    }

    WCHAR wFileName[MAX_PATH];

    /**
     *  Check to see if the file exists.
     */
    if (!Is_File_Available(pszFileName, &dest_filename)) {
        goto done;
    }

    MultiByteToWideChar(CP_ACP, 0, dest_filename.c_str(), -1, wFileName, MAX_PATH);
    
    /**
     *  Add the source filter to the graph.
     */
    hr = Graph->AddSourceFilter(wFileName, nullptr, &pSource);

    /**
     *  If the media file was not found, inform the user.
     */
    if (hr == VFW_E_NOT_FOUND) {
        DEBUG_ERROR("DirectShow: Media file not found! Error code: 0x%08x.\n", hr);
        goto done;
    }

    if (FAILED(hr)) {
        DEBUG_ERROR("DirectShow: Could not add source filter to graph! Error code: 0x%08x.\n", hr);
        goto done;
    }

    /**
     *  Store the filename.
     */
    Filename = dest_filename.c_str();
    FilenameNoExt = pszFileName;

    /**
     *  Try to render the streams.
     */
    hr = Render_Streams(pSource);
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Get the seeking capabilities.
     */
    hr = Seek->GetCapabilities(&SeekCaps);
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Set the volume.
     */
    hr = Update_Volume();
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Update our state.
     */
    State = STATE_STOPPED;

done:
    //if (FAILED(hr)) {
    //    Tear_Down_Graph();
    //}

    SafeRelease(&pSource);

    return hr;
}


/**
 *  Respond to a graph event.
 * 
 *  The owning window should call this method when it receives the window
 *  message that the application specified when it called SetEventWindow.
 * 
 *  pfnOnGraphEvent:
 *      Pointer to the GraphEventCallback callback, implemented by 
 *      the application. This callback is invoked once for each event
 *      in the queue.
 * 
 *  @warning: Do not tear down the graph from inside the callback.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Handle_Graph_Event(GraphEventCallback *pfnOnGraphEvent)
{
    if (!pfnOnGraphEvent) {
        return E_UNEXPECTED;
    }

    if (!Event) {
        return E_UNEXPECTED;
    }

    long evCode = 0;
    LONG_PTR param1 = 0;
    LONG_PTR param2 = 0;

    HRESULT hr = S_OK;
    
    /**
     *  Get the events from the queue.
     */
    while (SUCCEEDED(Event->GetEvent(&evCode, &param1, &param2, 0))) {

        /**
         *  Invoke the callback.
         */
        pfnOnGraphEvent->OnGraphEvent(evCode, param1, param2);
        
        /**
         *  Free the event data.
         */
        hr = Event->FreeEventParams(evCode, param1, param2);
        if (FAILED(hr)) {
            break;
        }
    }

    return hr;
}


/**
 *  Start playing the video.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Play()
{
    if (State != STATE_PAUSED && State != STATE_STOPPED) {
        return VFW_E_WRONG_STATE;
    }

    ASSERT(Graph); // If state is correct, the graph should exist.

    /**
     *  Run the graph to play the media file.
     */
    HRESULT hr = Control->Run();
    if (SUCCEEDED(hr)) {
        State = STATE_RUNNING;
    }

    return hr;
}


/**
 *  
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Play_Callback()
{
    Play();

    WWKeyboard->Clear();

    /*if (IsScalingAllowed) {
        InvalidateRect(hWnd, nullptr, false);
    }*/

    PAINTSTRUCT ps;
    HDC hdc;

    //hdc = BeginPaint(hWnd, &ps);

    bool process = true;

    while (process) {

        /**
         *  Are there any Windows messages to handle?
         */
        if (!VQA_Movie_Message_Loop()) {
            break;
        }

        /**
         *  Handle window focus loss and restore.
         */
        if (!GameInFocus && State == STATE_PAUSED) {
            DEBUG_INFO("DirectShow: Focus Restore.\n");
            Play();

        } else if (!GameInFocus && State == STATE_RUNNING) {
            DEBUG_INFO("DirectShow: Focus Loss.\n");
            Pause();
        }

        /**
         *  Has the video finished? If so, stop it.
         */
        if (Is_Finished()) {
            DEV_DEBUG_INFO("DirectShow: Video finished.\n");
            State = STATE_STOPPED;
            //break;
        }
    
        /**
         *  HAve we lost the video?
         */
        if (!Has_Video()) {
            DEV_DEBUG_INFO("DirectShow: Video lost, ending playback.\n");
            break;
        }
    
        /**
         *  If we have been flagged as finished, we are done.
         */
        if (State == STATE_STOPPED) {
            DEV_DEBUG_INFO("DirectShow: Video stopped, ending playback.\n");
            Stop_Events();
            break;
        }

        /**
         *  Are we still playing? Draw the next frame.
         */
        if (State == STATE_RUNNING) {

            /**
             *  The player has video, so ask the player to repaint.
             */
            //Repaint(hdc);

        } else {

            /**
             *  We are paused. we don't need to redraw every tick, so add a little wait to take
             *  the stress away from the CPU, because you know, it has a hard life...
             */
            //Repaint(hdc);
            Sleep(33); // Sleep for 33 msec.
        }

        /**
         *  Check for any keyboard input.
         */
        if (WWKeyboard->Check()) {

            switch (WWKeyboard->Get()) {

                /**
                 *  Debug only: Space bar toggles pause.
                 */
                case (KN_RLSE_BIT|KN_SPACE):
                    if (Vinifera_DeveloperMode) {
                        if (State == STATE_PAUSED) {
                            DEV_DEBUG_INFO("DirectShow: RESUME\n");
                            Play();
                        } else {
                            DEV_DEBUG_INFO("DirectShow: PAUSE\n");
                            Pause();
                        }
                    }
                    break;

                /**
                 *  Debug only: Display the debug overlay.
                 */
                case (KN_RLSE_BIT|KN_D):
                    if (Vinifera_DeveloperMode) {
                    }
                    break;

                /**
                 *  Debug only: Restart playback.
                 */
                case (KN_RLSE_BIT|KN_R):
                    if (Vinifera_DeveloperMode && Can_Seek()) {
                        DEV_DEBUG_INFO("DirectShow: Restarting playback.\n");
                        Set_Position(0);
                    }
                    break;

                /**
                 *  Debug only: Mute volume.
                 */
                case (KN_RLSE_BIT|KN_M):
                    if (Vinifera_DeveloperMode) {
                        if (Is_Muted()) {
                            DEV_DEBUG_INFO("DirectShow: Mute.\n");
                            Mute(false);
                        } else {
                            DEV_DEBUG_INFO("DirectShow: Unmute.\n");
                            Mute(true);
                        }
                    }
                    break;

                /**
                 *  Debug only: Adjust playback rate.
                 */
                case (KN_RLSE_BIT|KN_MINUS):
                    if (Vinifera_DeveloperMode) {
                        DEV_DEBUG_INFO("DirectShow: Rate down.\n");
                        Set_Rate(PlaybackRate-0.10f);
                    }
                    break;
                case (KN_RLSE_BIT|KN_EQUAL):
                    if (Vinifera_DeveloperMode) {
                        DEV_DEBUG_INFO("DirectShow: Rate up.\n");
                        Set_Rate(PlaybackRate+0.10f);
                    }
                    break;

                /**
                 *  Debug only: Adjust volume.
                 */
                case (KN_RLSE_BIT|KN_KEYPAD_MINUS):
                    if (Vinifera_DeveloperMode) {
                        DEV_DEBUG_INFO("DirectShow: Volume down.\n");
                        Set_Volume(CurrentVolume-0.10f);
                    }
                    break;
                case (KN_RLSE_BIT|KN_KEYPAD_PLUS):
                    if (Vinifera_DeveloperMode) {
                        DEV_DEBUG_INFO("DirectShow: Volume up.\n");
                        Set_Volume(CurrentVolume+0.10f);
                    }
                    break;

                /**
                 *  Debug only: Frame step.
                 */
                case (KN_RLSE_BIT|KN_F1):
                {
                    if (Vinifera_DeveloperMode && Can_Seek()) {
                        DEV_DEBUG_INFO("DirectShow: Frame step.\n");
                        Step_One_Frame();
                    }
                    break;
                }

                /**
                 *  Debug only: Previous and next frame. The payback must be
                 *              paused for these keys to work.
                 */
                case (KN_RLSE_BIT|KN_COMMA):
                {
                    if (Vinifera_DeveloperMode && Can_Seek()) {
                        DEV_DEBUG_INFO("DirectShow: Seek back .5 second.\n");
                        LONGLONG current;
                        Get_Current_Position(&current);
                        Set_Position(current - HALF_SEC);
                    }
                    break;
                }
                case (KN_RLSE_BIT|KN_PERIOD):
                    if (Vinifera_DeveloperMode && Can_Seek()) {
                        DEV_DEBUG_INFO("DirectShow: Seek forward .5 second.\n");
                        LONGLONG current;
                        Get_Current_Position(&current);
                        Set_Position(current + HALF_SEC);
                    }
                    break;

                /**
                 *  Check if the ESC key has been pressed. If so, break out
                 *  and stop all frame updates.
                 */
                case (KN_RLSE_BIT|KN_ESC):
                    if (IsBreakoutAllowed) {
                        DEBUG_INFO("DirectShow: Breakout.\n");
                        Stop();
                        UpdateWindow(hWnd);
                        //State = STATE_STOPPED;
                        //process = false;
                    }
                    break;
            };

        }
    
    }

    //EndPaint(hWnd, &ps);

    /*if (IsScalingAllowed) {
        InvalidateRect(hWnd, nullptr, false);
    }*/

    /**
     *  Clear the keyboard buffers.
     */
    WWKeyboard->Clear();

    return S_OK;
}


/**
 *  
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Pause()
{
    if (State != STATE_RUNNING) {
        return VFW_E_WRONG_STATE;
    }

    ASSERT(Graph); // If state is correct, the graph should exist.

    HRESULT hr = Control->Pause();
    if (SUCCEEDED(hr)) {
        State = STATE_PAUSED;
    }

    return hr;
}


/**
 *  
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Stop()
{
    if (State != STATE_RUNNING && State != STATE_PAUSED) {
        return VFW_E_WRONG_STATE;
    }

    ASSERT(Graph); // If state is correct, the graph should exist.

    HRESULT hr = Control->Stop();
    if (SUCCEEDED(hr)) {
        State = STATE_STOPPED;
    }

    return hr;
}


/**
 *  
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
BOOL DirectShowVideoPlayer::Is_File_Available(const char *pszFileName, std::string *dest)
{
    for (int format = 0; format < FORMAT_COUNT; ++format) {
        
        std::string filename;
        std::string tmp;

        /**
         *  Reset filename.
         */
        filename = pszFileName;

        switch (format) {
            case FORMAT_MP4:
                filename += ".MP4";
                break;
            case FORMAT_WMV:
                filename += ".WMV";
                break;
            case FORMAT_MPG:
                filename += ".MPG";
                break;
            default:
            case FORMAT_AVI:
                filename += ".AVI";
                break;
        };

        /**
         *  Load from the file system. We use CDFileClass so local files
         *  have priority over any video files in the mix files.
         */
        CDFileClass localfile(filename.c_str());
        if (localfile.Is_Available()) {
            if (dest) *dest = localfile.File_Name();
            DEBUG_INFO("DirectShow: Found \"%s\".\n", localfile.File_Name());
            return true;
        }

        /**
         *  #TODO: Mix file support.
         */
#if 0
        /**
         *  Lastly check in the games mix files.
         */
        DEBUG_INFO("DirectShow: Search for \"%s\" in mix files.\n", filename.c_str());
        if (CCFileClass(filename.c_str()).Is_Available()) {
            if (dest) *dest = filename;
            DEBUG_INFO("DirectShow: Found \"%s\".\n", filename.c_str());
            return true;
        }
#endif

    }

    return false;
}


/**
 *  Has the video player lost the renderer or ran out of frames to play?
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
BOOL DirectShowVideoPlayer::Has_Video() const 
{
#if 0
    if (!VideoRenderer) {
        DEBUG_ERROR("DirectShow: VideoRenderer is null!\n");
        return false;
    }
    if (!VideoRenderer->HasVideo()) {
        DEBUG_ERROR("DirectShow: HasVideo returned false!\n");
        return false;
    }
#endif
    return VideoRenderer && VideoRenderer->HasVideo(); 
}


/**
 *  Sets the destination rectangle for the video.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
BOOL DirectShowVideoPlayer::Update_Video_Window(const LPRECT prc)
{
    if (!VideoRenderer) {
        DEBUG_WARNING("DirectShow: Unable to update window, VideoRenderer is null!\n");
        return S_OK;
    }

    bool fixup_default_scale = false;

    /**
     *  Get the size of the current window.
     */
    RECT rect;

    /**
     *  This is a workaround to make sure that we stretch the video to the size set by
     *  a custom DDRAW dll, even if the size is larger than the game resolution.
     */
    if (RawFileClass("DDRAW.DLL").Is_Available()) {
        RawFileClass ddrawfile("DDRAW.INI");
        INIClass ddrawini;

        ddrawini.Load(ddrawfile);

        GetClientRect(hWnd, &rect);

        int ddraw_width = ddrawini.Get_Int("ddraw", "width", rect.right);
        int ddraw_height = ddrawini.Get_Int("ddraw", "height", rect.bottom);

        if ((ddraw_width > 0 && ddraw_height > 0)
          && (Options.ScreenWidth != ddraw_width || Options.ScreenHeight != ddraw_height)) {
            fixup_default_scale = true;
            rect.right = ddraw_width;
            rect.bottom = ddraw_height;
            DEV_DEBUG_WARNING("DirectShow: Possible resolution mismatch in ddraw.ini!\n");
        }
    }

    /**
     *  We detected a possible resolution mis-match, attempt to fix it.
     */
    if (fixup_default_scale) {

        /**
         *  #TODO:
         *  For now we have to assume that the user set the resolution
         *  in DDRAW.INI to x2 of the game resolution.
         */
        rect.left = prc->left*2;
        rect.top = prc->top*2;
        rect.right = prc->right*2;
        rect.bottom = prc->bottom*2;

    } else {
        rect.left = prc->left;
        rect.top = prc->top;
        rect.right = prc->right;
        rect.bottom = prc->bottom;
    }

    if (VideoRenderer) {
        return VideoRenderer->UpdateVideoWindow(hWnd, &rect) == S_OK;
    }

    return false;
}


/**
 *  Set the desired video window handle.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Set_Video_Window(HWND new_hWnd)
{
    if (!VideoRenderer) {
        DEBUG_WARNING("DirectShow: Unable to set window, VideoRenderer is null!\n");
        return S_OK;
    }

    /*if (!Video) {
        DEBUG_WARNING("DirectShow: Unable to set window, Video is null!\n");
        return S_OK;
    }*/

    bool fixup_default_scale = false;

    /**
     *  Get the size of the current window.
     */
    RECT rect;
    GetClientRect(new_hWnd, &rect);

    /**
     *  This is a workaround to make sure that we stretch the video to the size set by
     *  a custom DDRAW dll, even if the size is larger than the game resolution.
     */
    if (RawFileClass("DDRAW.DLL").Is_Available()) {
        RawFileClass ddrawfile("DDRAW.INI");
        INIClass ddrawini;

        ddrawini.Load(ddrawfile);

        int ddraw_width = ddrawini.Get_Int("ddraw", "width", rect.right);
        int ddraw_height = ddrawini.Get_Int("ddraw", "height", rect.bottom);

        if ((ddraw_width > 0 && ddraw_height > 0)
          && (Options.ScreenWidth != ddraw_width || Options.ScreenHeight != ddraw_height)) {
            fixup_default_scale = true;
            rect.right = ddraw_width;
            rect.bottom = ddraw_height;
            DEV_DEBUG_WARNING("DirectShow: Possible resolution mismatch in ddraw.ini!\n");
        }
    }

    /**
     *  Store the screen surface size.
     */
    int surface_width = rect.right;
    int surface_height = rect.bottom;

    /**
     *  Stretch (while maintaining aspect ratio) the movie to the window size.
     */
    Options.StretchMovies = true;
    if (IsScalingAllowed && Options.StretchMovies) {

        /**
         *  Fetch the video dimensions.
         */
        /*if (Video) {
            HRESULT hr = Video->GetVideoSize(&VideoWidth, &VideoHeight);
            if (FAILED(hr)) {
                return hr;
            }
        }*/

        /**
         *  Set the full window size, DirectShow will do the scaling for us.
         */
        rect.left = 0;
        rect.top = 0;
        rect.right = surface_width;
        rect.bottom = surface_height;

    /**
     *  We detected a possible resolution mis-match, attempt to fix it.
     */
    } else if (fixup_default_scale) {
    
        /**
         *  #TODO:
         *  For now we have to assume that the user set the resolution
         *  in DDRAW.INI to x2 of the game resolution.
         */
        rect.left = (surface_width-(NORMAL_VIDEO_WIDTH*2))/2;
        rect.top = (surface_height-(NORMAL_VIDEO_HEIGHT*2))/2;
        rect.right = (rect.left+(NORMAL_VIDEO_WIDTH*2));
        rect.bottom = (rect.top+(NORMAL_VIDEO_HEIGHT*2));

    } else {

        /**
         *  Even if the video is larger than 640x400, we still
         *  retain the original expectations of the video playback
         *  and tell DirectShow to scale the video down.
         */
        rect.left = (surface_width-(NORMAL_VIDEO_WIDTH))/2;
        rect.top = (surface_height-(NORMAL_VIDEO_HEIGHT))/2;
        rect.right = (rect.left+(NORMAL_VIDEO_WIDTH));
        rect.bottom = (rect.top+(NORMAL_VIDEO_HEIGHT));
    }

    DEBUG_INFO("DirectShow: %d,%d,%d,%d\n", rect.left, rect.top, rect.right, rect.bottom);

    return VideoRenderer->UpdateVideoWindow(new_hWnd, &rect);
}


/**
 *  Repaints the video. Call this method when the application receives WM_PAINT.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Repaint(HDC hdc)
{
    if (VideoRenderer) {
        return VideoRenderer->Repaint(hWnd, hdc);
    }

    return S_OK;
}


/**
 *  Notifies the video renderer that the display mode changed.
 *  Call this method when the application receives WM_DISPLAYCHANGE.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Display_Mode_Changed()
{
    if (VideoRenderer) {
        return VideoRenderer->DisplayModeChanged();
    }

    return S_OK;
}


/**
 *  Returns TRUE if the current file is seekable.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
BOOL DirectShowVideoPlayer::Can_Seek() const
{
    const DWORD caps = AM_SEEKING_CanSeekAbsolute | AM_SEEKING_CanGetDuration;
    return ((SeekCaps & caps) == caps);
}


/**
 *  Seeks to a new position.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Set_Position(REFERENCE_TIME pos)
{
    if (Control == nullptr || Seek == nullptr) {
        return E_UNEXPECTED;
    }

    HRESULT hr = S_OK;

    hr = Seek->SetPositions(&pos, AM_SEEKING_AbsolutePositioning,
        nullptr, AM_SEEKING_NoPositioning);

    if (SUCCEEDED(hr)) {

        /**
         *  If playback is stopped, we need to put the graph into the paused
         *  state to update the video renderer with the new frame, and then stop 
         *  the graph again. The IMediaControl::StopWhenReady does this.
         */
        if (State == STATE_STOPPED) {
            hr = Control->StopWhenReady();
        }
    }

    return hr;
}


/**
 *  Gets the duration of the current file.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Get_Duration(LONGLONG *pDuration)
{
    if (Seek == nullptr) {
        return E_UNEXPECTED;
    }

    return Seek->GetDuration(pDuration);
}


/**
 *  Gets the current playback position.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Get_Current_Position(LONGLONG *pTimeNow)
{
    if (Seek == nullptr) {
        return E_UNEXPECTED;
    }

    return Seek->GetCurrentPosition(pTimeNow);
}


/**
 *  Has the video finished playing?
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
BOOL DirectShowVideoPlayer::Is_Finished()
{
    HRESULT hr = S_OK;

    LONGLONG current = 0;
    LONGLONG duration = 0;

    hr = Get_Current_Position(&current);
    if (FAILED(hr)) {
        return false;
    }

    hr = Get_Duration(&duration);
    if (FAILED(hr)) {
        return false;
    }

    return current >= duration;
}


/**
 *  Mutes or unmutes the audio.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT	DirectShowVideoPlayer::Mute(BOOL bMute)
{
    IsMuted = bMute;
    return Update_Volume();
}


/**
 *  Sets the media volume.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT	DirectShowVideoPlayer::Set_Volume(float lVolume)
{
    if (lVolume < 0.0 ) lVolume = 0.0;
    if (lVolume > 1.0 ) lVolume = 1.0; 
    // Volume has to be log corrected/converted.
    CurrentVolume = log10(lVolume) * 4000.0;
    return Update_Volume();
}


/**
 *  Update the volume after a call to Mute() or SetVolume().
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Update_Volume()
{
    HRESULT hr = S_OK;

    if (IsAudioStream && Audio) {

        /**
         *  If the audio is muted, set the minimum volume.
         */ 
        if (IsMuted) {
            hr = Audio->put_Volume(MIN_VOLUME);

        } else {

            /**
             *  Restore previous volume setting.
             */
            hr = Audio->put_Volume(CurrentVolume);
        }
    }

    return hr;
}


/**
 *  Stop the window from receiving any render events.
 * 
 *  @author: CCHyper
 */
void DirectShowVideoPlayer::Stop_Events()
{
    /**
     *  Clear open dialog remnants before calling RenderFile().
     */
    UpdateWindow(hWnd);

    /**
     *  Stop sending event messages.
     */
    if (Event) {
        Event->SetNotifyWindow((OAHWND)nullptr, 0, 0);
    }
}


/**
 *  Create a new filter graph. (Tears down the old graph.) 
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Initialize_Graph()
{
    Tear_Down_Graph();

    /**
     *  QueryInterface for DirectShow interfaces
     */

    /**
     *  Create the Filter Graph Manager.
     */
    HRESULT hr = CoCreateInstance(CLSID_FilterGraph, nullptr, 
        CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&Graph));
    if (FAILED(hr)) {
        return hr;
    }

    /**
     *  Query for graph interfaces.
     */

    hr = Graph->QueryInterface(IID_PPV_ARGS(&Control));
    if (FAILED(hr)) {
        return hr;
    }

    hr = Graph->QueryInterface(IID_PPV_ARGS(&Event));
    if (FAILED(hr)) {
        return hr;
    }

    hr = Graph->QueryInterface(IID_PPV_ARGS(&Seek));
    if (FAILED(hr)) {
        return hr;
    }

    hr = Graph->QueryInterface(IID_PPV_ARGS(&Audio));
    if (FAILED(hr)) {
        return hr;
    }

    hr = Graph->QueryInterface(IID_PPV_ARGS(&Video));
    if (FAILED(hr)) {
        return hr;
    }

    hr = Graph->QueryInterface(IID_PPV_ARGS(&Position));
    if (FAILED(hr)) {
        return hr;
    }

    /**
     *  Get the frame step interface, if supported.
     */
    IVideoFrameStep *pFSTest = nullptr;
    hr = Graph->QueryInterface(IID_PPV_ARGS(&pFSTest));
    if (FAILED(hr)) {
        return hr;
    }

    /**
     *  Check if this decoder can step.
     */
    hr = pFSTest->CanStep(false, nullptr);
    if (hr == S_OK) {
        FrameStep = pFSTest;

    } else {
        pFSTest->Release();
    }

    /**
     *  Set up event notification.
     *  Have the graph signal event via window callbacks for performance.
     */
    hr = Event->SetNotifyWindow((OAHWND)hWnd, WM_GRAPH_EVENT, 0);
    if (FAILED(hr)) {
        return hr;
    }

    State = STATE_STOPPED;

    return hr;
}


/**
 *  Tear down the filter graph and release resources.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
void DirectShowVideoPlayer::Tear_Down_Graph()
{
    /**
     *  Clear open dialog remnants before calling RenderFile().
     */
    UpdateWindow(hWnd);

    /**
     *  Stop sending event messages.
     */
    if (Event) {
        Event->SetNotifyWindow((OAHWND)nullptr, 0, 0);
    }

    SafeRelease(&Graph);
    SafeRelease(&Control);
    SafeRelease(&Event);
    SafeRelease(&Seek);
    SafeRelease(&Audio);
    SafeRelease(&Video);

    SAFE_DELETE(VideoRenderer);

    State = STATE_NO_GRAPH;
    SeekCaps = 0;

    IsAudioStream = false;
}


/**
 *  Create the DirectShow renderer.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Create_Video_Renderer()
{
    HRESULT hr = E_FAIL;

    enum { Try_EVR, Try_VMR9, Try_VMR7 };

    for (DWORD i = Try_EVR; i <= Try_VMR7; i++) {

        switch (i) {
            case Try_EVR:
                DEBUG_INFO("DirectShow: Using EVR.\n");
                VideoRenderer = new (std::nothrow) EVR();
                break;

            case Try_VMR9:
                DEBUG_INFO("DirectShow: Using VMR9.\n");
                VideoRenderer = new (std::nothrow) VMR9();
                break;

            case Try_VMR7:
                DEBUG_INFO("DirectShow: Using VMR7.\n");
                VideoRenderer = new (std::nothrow) VMR7();
                break;
        };

        if (VideoRenderer == nullptr) {
            hr = E_OUTOFMEMORY;
            break;
        }

        hr = VideoRenderer->AddToGraph(Graph, hWnd);
        if (SUCCEEDED(hr)) {
            DEV_DEBUG_INFO("DirectShow: VideoRenderer->AddToGraph passed.\n");
            break;
        }
        
        DEBUG_ERROR("DirectShow: AddToGraph failed!\n");

        SAFE_DELETE(VideoRenderer);

    }

    return hr;
}


/**
 *  Render the streams from a source filter.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Render_Streams(IBaseFilter *pSource)
{
    BOOL bRenderedAnyPin = FALSE;

    IFilterGraph2 *pGraph2 = nullptr;
    IEnumPins *pEnum = nullptr;
    IBaseFilter *pAudioRenderer = nullptr;

    HRESULT hr = Graph->QueryInterface(IID_PPV_ARGS(&pGraph2));
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Add the video renderer to the graph.
     */
    hr = Create_Video_Renderer();
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Add the DSound Renderer to the graph.
     */
    hr = AddFilterByCLSID(Graph, CLSID_DSoundRender, 
        &pAudioRenderer, L"Audio Renderer");
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Enumerate the pins on the source filter.
     */
    hr = pSource->EnumPins(&pEnum);
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Loop through all the pins.
     */
    IPin *pPin;
    while (S_OK == pEnum->Next(1, &pPin, nullptr)) {

        /**
         *  Try to render this pin.
         *  It's OK if we fail some pins, if at least one pin renders.
         */
        HRESULT hr2 = pGraph2->RenderEx(pPin, AM_RENDEREX_RENDERTOEXISTINGRENDERERS, nullptr);

        pPin->Release();
        if (SUCCEEDED(hr2)) {
            bRenderedAnyPin = TRUE;
        }
    }

    /**
     *  Remove un-used renderers.
     */

    /**
     *  Try to remove the VMR.
     */
    hr = VideoRenderer->FinalizeGraph(Graph);
    if (FAILED(hr)) {
        goto done;
    }

    /**
     *  Try to remove the audio renderer.
     */
    BOOL bRemoved = FALSE;
    hr = RemoveUnconnectedRenderer(Graph, pAudioRenderer, &bRemoved);
    if (bRemoved) {
        IsAudioStream = FALSE;
    } else {
        IsAudioStream = TRUE;
    }

done:
    SafeRelease(&pEnum);
    //SafeRelease(pVMR);
    SafeRelease(&pAudioRenderer);
    SafeRelease(&Audio);
    SafeRelease(&Video);
    SafeRelease(&pGraph2);

    /**
     *  If we succeeded to this point, make sure we rendered at least one
     *  stream.
     */
    if (SUCCEEDED(hr)) {
        if (!bRenderedAnyPin) {
            hr = VFW_E_CANNOT_RENDER;
        }
    }

    return hr;
}


/**
 *  Step a single frame forward.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Step_One_Frame()
{
    HRESULT hr = S_OK;

    /**
     *  If the Frame Stepping interface exists, use it to step one frame.
     */
    if (FrameStep) {

        /**
         *  The graph must be paused for frame stepping to work.
         */
        if (State != STATE_PAUSED) {
            Pause();
        }

        /**
         *  Step the requested number of frames, if supported.
         */
        hr = FrameStep->Step(1, nullptr);
    }

    return hr;
}


/**
 *  Skip 'n' frames forward.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Step_Frames(int nFramesToStep)
{
    HRESULT hr = S_OK;

    /**
     *  If the Frame Stepping interface exists, use it to step frames.
     */
    if (FrameStep) {

        /**
         *  The renderer may not support frame stepping for more than one
         *  frame at a time, so check for support. S_OK indicates that the
         *  renderer can step nFramesToStep successfully.
         */
        if ((hr = FrameStep->CanStep(nFramesToStep, nullptr)) == S_OK) {

            /**
             *  The graph must be paused for frame stepping to work.
             */
            if (State != STATE_PAUSED) {
                Pause();
            }

            /**
             *  Step the requested number of frames, if supported.
             */
            hr = FrameStep->Step(nFramesToStep, nullptr);
        }
    }

    return hr;
}


/**
 *  Modify the current playback rate by this delta.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Modify_Rate(double dRateAdjust)
{
    HRESULT hr = S_OK;
    double dRate;

    /**
     *  If the IMediaPosition interface exists, use it to set rate.
     */
    if ((Position) && (dRateAdjust != 0)) {
        if ((hr = Position->get_Rate(&dRate)) == S_OK) {

            /**
             *  Add current rate to adjustment value.
             */
            double dNewRate = dRate + dRateAdjust;
            hr = Position->put_Rate(dNewRate);

            /**
             *  Save new rate.
             */
            if (SUCCEEDED(hr)) {
                PlaybackRate = dNewRate;
            }
        }
    }

    return hr;
}


/**
 *  Set the desired playback rate.
 * 
 *  @author: CCHyper (based on Windows SDK sample code)
 */
HRESULT DirectShowVideoPlayer::Set_Rate(double dRate)
{
    HRESULT hr = S_OK;

    /**
     *  If the IMediaPosition interface exists, use it to set rate.
     */
    if (Position) {
        hr = Position->put_Rate(dRate);

        /**
         *  Save new rate
         */
        if (SUCCEEDED(hr)) {
            PlaybackRate = dRate;
        }
    }

    return hr;
}

#if 0
double getDurationInSeconds(){
    if( isLoaded() ){
        long long lDurationInNanoSecs = 0;
        m_pSeek->GetDuration(&lDurationInNanoSecs);
        double timeInSeconds = (double)lDurationInNanoSecs/10000000.0;

        return timeInSeconds;
    }
    return 0.0;
}

double getCurrentTimeInSeconds(){
    if( isLoaded() ){
        long long lCurrentTimeInNanoSecs = 0;
        m_pSeek->GetCurrentPosition(&lCurrentTimeInNanoSecs);
        double timeInSeconds = (double)lCurrentTimeInNanoSecs/10000000.0;

        return timeInSeconds;
    }
    return 0.0;
}
#endif
