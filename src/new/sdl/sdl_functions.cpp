/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SDL_FUNCTIONS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains functions for the SDL system.
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
#include "sdl_functions.h"
#include <algorithm>
#include "cctooltip.h"
#include "cdctrl.h"
#include "command.h"
#include "convert.h"
#include "debughandler.h"
#include "mouse.h"
#include "optionsext.h"
#include "playmovie.h"
#include "rect.h"
#include "sdlmouse.h"
#include "sdlsurface.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "windialog.h"
#include "wsproto.h"
#include "wwmouse.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_render.h"
#include "SDL3/SDL_video.h"
#include <windowsx.h>


/**
 *  Allocates all game surfaces with the given sizes.
 *
 *  @author: ZivDero, tomsons26
 */
bool SDL_Allocate_Surfaces(const Rect& hidden_rect, const Rect& composite_rect, const Rect& tile_rect, const Rect& sidebar_rect, bool hidden_first)
{
    DEBUG_INFO("Allocating new surfaces\n");

    if (AlternateSurface != nullptr) {
        DEBUG_INFO("Deleting AlternateSurface\n");
        delete AlternateSurface;
        AlternateSurface = nullptr;
    }

    if (HiddenSurface != nullptr) {
        DEBUG_INFO("Deleting HiddenSurface\n");
        delete HiddenSurface;
        HiddenSurface = nullptr;
    }

    if (CompositeSurface != nullptr) {
        DEBUG_INFO("Deleting CompositeSurface\n");
        delete CompositeSurface;
        CompositeSurface = nullptr;
    }

    if (TileSurface != nullptr) {
        DEBUG_INFO("Deleting TileSurface\n");
        delete TileSurface;
        TileSurface = nullptr;
    }

    if (SidebarSurface != nullptr) {
        DEBUG_INFO("Deleting SidebarSurface\n");
        delete SidebarSurface;
        SidebarSurface = nullptr;
    }

    if (hidden_first && hidden_rect.Is_Valid()) {
        HiddenSurface = new SDLSurface(hidden_rect.Width, hidden_rect.Height);
        HiddenSurface->Fill(0);
        DEBUG_INFO("HiddenSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
    }

    if (composite_rect.Is_Valid()) {
        CompositeSurface = new SDLSurface(composite_rect.Width, composite_rect.Height);
        CompositeSurface->Fill(0);
        DEBUG_INFO("CompositeSurface (%dx%d)\n", composite_rect.Width, composite_rect.Height);
    }

    if (tile_rect.Is_Valid()) {
        TileSurface = new SDLSurface(tile_rect.Width, tile_rect.Height);
        TileSurface->Fill(0);
        DEBUG_INFO("TileSurface (%dx%d)\n", tile_rect.Width, tile_rect.Height);
    }

    if (sidebar_rect.Is_Valid()) {
        SidebarSurface = new SDLSurface(sidebar_rect.Width, sidebar_rect.Height);
        SidebarSurface->Fill(0);
        DEBUG_INFO("SidebarSurface (%dx%d)\n", sidebar_rect.Width, sidebar_rect.Height);
    }

    if (!hidden_first && hidden_rect.Is_Valid()) {
        HiddenSurface = new SDLSurface(hidden_rect.Width, hidden_rect.Height);
        HiddenSurface->Fill(0);
        DEBUG_INFO("HiddenSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
    }

    if (hidden_rect.Is_Valid()) {
        AlternateSurface = new SDLSurface(hidden_rect.Width, hidden_rect.Height);
        AlternateSurface->Fill(0);
        DEBUG_INFO("AlternateSurface (%dx%d)\n", hidden_rect.Width, hidden_rect.Height);
    }

    return true;
}


/**
 *  Initializes the SDL presentation layer.
 *
 *  @author: ZivDero
 */
bool SDL_Set_Video_Mode(HWND, int width, int height, int bits_per_pixel)
{
    if (SDLWindow == nullptr) {
        DEBUG_ERROR("SDLWindow is null!\n");
        return false;
    }
    
    /**
     *  We need to delete the existing presentation layer first.
     */
    SDL_Reset_Video_Mode();
    
    /**
     *  Query the window's pixel format.
     */
    SDL_PixelFormat pixel_format = SDL_GetWindowPixelFormat(SDLWindow);
    if (pixel_format == SDL_PIXELFORMAT_UNKNOWN || SDL_BITSPERPIXEL(pixel_format) < 16) {
        DEBUG_ERROR("SDL3 window pixel format unsupported: %s (%d bpp)\n", SDL_GetPixelFormatName(pixel_format), SDL_BITSPERPIXEL(pixel_format));
        return false;
    }

    DEBUG_INFO("Pixel format: %s (%d bpp)\n", SDL_GetPixelFormatName(pixel_format), SDL_BITSPERPIXEL(pixel_format));

    /**
     *  Create the renderer for window.
     */
    SDLWindowRenderer = SDL_CreateRenderer(SDLWindow, nullptr);
    if (SDLWindowRenderer == nullptr) {
        DEBUG_ERROR("SDLWindowRenderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }
    DEBUG_INFO("SDLWindowRenderer created.\n");

    const char* driver_name = SDL_GetRendererName(SDLWindowRenderer);
    DEBUG_INFO("Renderer driver: %s\n", driver_name);

    /**
     *  Toggle VSync.
     */
    SDL_SetRenderVSync(SDLWindowRenderer, OptionsExtension->IsVSync ? 1 : 0);

    /**
     *  Set the scaling mode if specified.
     */
    if (OptionsExtension->ScaleMode != SDL_SCALEMODE_INVALID) {
        SDL_SetDefaultTextureScaleMode(SDLWindowRenderer, OptionsExtension->ScaleMode);
    }

    /**
     *  Create the window texture.
     */
    SDLWindowTexture = SDL_CreateTexture(SDLWindowRenderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, width, height);
    if (SDLWindowTexture == nullptr) {
        DEBUG_ERROR("SDLWindowTexture could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    DEBUG_INFO("SDLWindowTexture created.\n");

    /**
     *  Save video mode information.
     */
    VideoWidth = width;
    VideoHeight = height;
    VideoBitsPerPixel = bits_per_pixel;

    return true;
}


/**
 *  Resets video mode and deletes the SDL presentation layer.
 *
 *  @author: ZivDero
 */
void SDL_Reset_Video_Mode()
{
    /**
     *  Destroy the renderer.
     */
    SDL_DestroyRenderer(SDLWindowRenderer);
    SDLWindowRenderer = nullptr;

    /**
     *  Deallocate the texture.
     */
    SDL_DestroyTexture(SDLWindowTexture);
    SDLWindowTexture = nullptr;

    /**
     *  Clear video mode information.
     */
    VideoWidth = 0;
    VideoHeight = 0;
    VideoBitsPerPixel = 0;
}


/**
 *  Pointer to the window procedure set by SDL.
 */
static WNDPROC SDL_Proc = nullptr;

/**
 *  Replacement window procedure for the main window.
 *
 *  @author: tomsons26, ZivDero
 */
LRESULT CALLBACK SDL_Windows_Procedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    /*
    **  Scale mouse inputs before they are processed by SDL or the game.
    */
    switch (message) {
    case WM_MOUSEMOVE:
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_LBUTTONDBLCLK:
    case WM_RBUTTONDOWN:
    case WM_RBUTTONUP:
    case WM_RBUTTONDBLCLK:
    case WM_MBUTTONDOWN:
    case WM_MBUTTONUP:
    case WM_MBUTTONDBLCLK:
    case WM_MOUSEWHEEL:
    case WM_XBUTTONDOWN:
    case WM_XBUTTONUP:
        if (SDL_Should_Scale()) {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            x = static_cast<int>(x * SDL_XScale());
            y = static_cast<int>(y * SDL_YScale());

            lParam = MAKELPARAM(x, y);
        }
        break;
    default:
        break;
    }

    /*
    **  Pass on any messages intended for the winsock message handler.
    */
    if (PacketTransport) {
        if (message == (UINT)PacketTransport->Protocol_Event_Message()) {
            if (PacketTransport->Message_Handler(hwnd, message, wParam, lParam)) {
                return DefWindowProc(hwnd, message, wParam, lParam);
            } else {
                return 0;
            }
        }
    }

    Map.Message_Handler(hwnd, message, wParam, lParam);

    switch (message) {

        /*
        **  Refresh the window.
        */
    case WM_PAINT:
        if (MouseCursor != nullptr && VisibleSurface != nullptr && HiddenSurface != nullptr && CompositeSurface != nullptr) {
            if (ScenarioStarted == true) {
                Update_Visible_Surface(MouseCursor->Is_Captured(), CompositeSurface);
                Map.Blit_Sidebar(true);
            } else if (Movie_Is_Playing() == true) {
                Movie_Update_Visible_Surface();
            } else {
                Update_Visible_Surface(MouseCursor->Is_Captured(), HiddenSurface);
            }
        }

        /*
        **  Tell SDL that the window needs refreshing to simulate what it does itself.
        */
        SDL_Event event;
        event.type = SDL_EVENT_WINDOW_EXPOSED;
        event.window.windowID = SDL_GetWindowID(SDLWindow);
        event.window.data1 = 0;
        event.window.data2 = 0;
        SDL_PushEvent(&event);

        /*
        **  But don't let SDL handle this event, or it will break Win32 controls' drawing.
        */
        return DefWindowProc(hwnd, message, wParam, lParam);

    case WM_CLOSE:
        CDControl.Unlock_All_CD_Trays();
        break;

        /*
        **  Windoze message says we have to shut down. Try and do it cleanly.
        */
    case WM_DESTROY:
        if (ToolTips != nullptr) {
            delete ToolTips;
            ToolTips = nullptr;
        }
        CDControl.Unlock_All_CD_Trays();
        MainWindow = nullptr;

        /*
        **  If we are shutting down gracefully than flag that the message loop has finished.
        **  If this is a forced shutdown (ReadyToQuit == 0) then try and close down everything
        **  before we exit.
        */
        switch (ReadyToQuit) {
        default:
        case 1:
            ReadyToQuit = 2;
            break;

        case 0:
            break;
        }
        return 0;

    case WM_ACTIVATEAPP:
        if (hwnd == MainWindow && GameInFocus != (wParam != 0)) {
            GameInFocus = wParam != 0;
            if (GameInFocus) {
                Focus_Restore();

                /*
                **  Force all child controls to redraw when regaining focus.
                */
                EnumChildWindows(
                    hwnd,
                    [](HWND child, LPARAM) -> BOOL {
                        RedrawWindow(child, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
                        return TRUE;
                    },
                    0);

                RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
            } else {
                Focus_Loss();
            }
        }
        return 0;

    case WM_RBUTTONUP:

        /*
        **  Set some kind of scolling flag, perhaps "CanScroll".
        */
        Map.field_1D0C = false;
        break;

    case WM_MOVING:
        On_WM_MOVING(hwnd, wParam, lParam);
        return CallWindowProc(SDL_Proc, hwnd, message, wParam, lParam);

    case WM_MOUSEWHEEL:
        if (!_MouseWheel) {
            _MouseWheel = true;

            /**
             *  If we are not currently playing a scenario, no need to execute this command.
             */
            if (ScenarioStarted && TacticalViewActive) {
                if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) {
                    Do_Command("SidebarDown");
                } else {
                    Do_Command("SidebarUp");
                }
            }
            _MouseWheel = false;
        }
        break;

    case WM_SYSCOMMAND:
        switch (wParam) {

        case SC_CLOSE:
            CDControl.Unlock_All_CD_Trays();

#ifdef TS_CLIENT
            /*
            **  TS Client users are used to Alt+F4 aborting the game, which in turn closes the game
            **  because there is no main menu in the TS Client.
            */
            if (GameActive) {
                Queue_Exit();
            }
#endif
            /*
            **  Windows sent us a close message. Probably in response to Alt-F4. Ignore it by
            **  pretending to handle the message and returning true;
            */
            return 0;

        case SC_SCREENSAVE:

            /*
            **  Windoze is about to start the screen saver. If we just return without passing
            **  this message to DefWindowProc then the screen saver will not be allowed to start.
            */
            return 0;

        default:
            break;
        }
        break;

    default:
        break;
    }

    /*
    **  Pass this message through to the keyboard handler.
    */
    Keyboard->Message_Handler(hwnd, message, wParam, lParam);

    return CallWindowProc(SDL_Proc, hwnd, message, wParam, lParam);
}


/**
 *  Creates the main game window.
 *
 *  @author: ZivDero, CCHyper
 */
bool SDL_Create_Main_Window(HINSTANCE instance, int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        DEBUG_ERROR("SDL_Init failed! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_PropertiesID props = SDL_CreateProperties();

    if (WindowedMode) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
    } else {
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);
    }

    DWORD dwPid = GetProcessId(GetCurrentProcess());
    if (!dwPid) {
        DEBUG_ERROR("Create_Main_Window() - Failed to get the process id!\n");
        return false;
    }

    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, Vinifera_Get_Window_Title(dwPid));

    /**
     *  Create the window.
     */
    SDLWindow = SDL_CreateWindowWithProperties(props);
    if (SDLWindow == nullptr) {
        DEBUG_ERROR("SDLWindow could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    DEBUG_INFO("SDLWindow created.\n");
    
    /**
     *  Record the size that the window has been created at.
     */
    SDL_GetWindowSize(SDLWindow, &SDLWindowWidth, &SDLWindowHeight);
    DEBUG_INFO("SDLWindow size: %d X %d.\n", SDLWindowWidth, SDLWindowHeight);

    /**
     *  Save the window handle for the game to use.
     */
    props = SDL_GetWindowProperties(SDLWindow);
    MainWindow = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

    /**
     *  We draw Win32 child windows as part of the main window, so we need to disable clipping.
     *  Otherwise, we will see black boxes where child windows are.
     */
    LONG_PTR style = GetWindowLongPtr(MainWindow, GWL_STYLE);
    style &= ~(WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    SetWindowLongPtr(MainWindow, GWL_STYLE, style);

    /**
     *  Set the window to use our window procedure, save the one SDL set.
     */
    SDL_Proc = (WNDPROC)SetWindowLongPtr(MainWindow, GWLP_WNDPROC, (LONG_PTR)SDL_Windows_Procedure);

    /**
     *  Explicitly set input focus to the window.
     */
    SDL_RaiseWindow(SDLWindow);
    GameInFocus = true; // The SDL window needs this initially otherwise we need to alt-tab to gain focus.

    /**
     *  This used to happen on WM_CREATE but our proc is no longer the proc that's used when
     *  the window is created, so it never happens.
     */
    if (!ToolTips) {
        ToolTips = new CCToolTip(MainWindow);
        if (ToolTips) {
            ToolTips->Set_Timer_Delay(500);
        }
    }

    return true;
}


/**
 *  Destroys the main game window.
 *
 *  @author: CCHyper
 */
void SDL_Destroy_Main_Window()
{
    /**
     *  Destroy window.
     */
    SDL_DestroyWindow(SDLWindow);
    SDLWindow = nullptr;
}


/**
 *  Update the screen with any rendering performed since the previous call.
 *
 *  @author: ZivDero, CCHyper, tomsons26
 */
bool SDL_Update_Screen(Surface* surface)
{
    SDL_RenderClear(SDLWindowRenderer);

    /**
     *  Blit game's surface to SDL's window surface.
     */
    if (surface) {

        /**
         *  First, update the texture with the pixels from the game's surface.
         */
        if (void* pixels = surface->Lock()) {
#if 0
            void* tex_pixels;
            int tex_pitch;
            SDL_LockTexture(SDLWindowTexture, nullptr, &tex_pixels, &tex_pitch);
            memcpy(tex_pixels, pixels, surface->Get_Height() * surface->Stride());
            SDL_UnlockTexture(SDLWindowTexture);
#else
            SDL_UpdateTexture(SDLWindowTexture, nullptr, pixels, surface->Stride());
#endif
            surface->Unlock();
        }

        static bool scaled = SDL_Should_Scale();

        /**
         *  Then, copy the texture to the renderer.
         */
        if (!SDL_Should_Scale()) {
            Rect src_rect = surface->Get_Rect();
            SDL_FRect dst_rect = {static_cast<float>(src_rect.X), static_cast<float>(src_rect.Y), static_cast<float>(src_rect.Width), static_cast<float>(src_rect.Height)};
            SDL_RenderTexture(SDLWindowRenderer, SDLWindowTexture, nullptr, &dst_rect);
        } else {
            SDL_RenderTexture(SDLWindowRenderer, SDLWindowTexture, nullptr, nullptr);
        }

        /**
         *  If the scale has changed, recalculate the mouse cursor image.
         */
        if (scaled != SDL_Should_Scale()) {
            scaled = SDL_Should_Scale();
            static_cast<SDLMouseClass*>(MouseCursor)->Recalc_Cursor_Image();
        }
    }

    /**
     *  Present the image to the window.
     */
    SDL_RenderPresent(SDLWindowRenderer);

    return true;
}


/**
 *  Returns if scaling should currently be applied.
 *  We turn off scaling when any windows dialogs are open
 *  because we cannot properly scale their input.
 *
 *  @author: ZivDero
 */
bool SDL_Should_Scale()
{
    return WSDialogCount == 0 && SpecialDialog == SDLG_NONE;
}


/**
 *  Changes the display mode to the given resolution.
 *
 *  @author: ZivDero, tomsons26
 */
bool SDL_Change_Display_Mode(int width, int height)
{
    DEBUG_INFO("About to set video mode\n");

    Hide_Mouse();

    /**
     *  Delete the old surfaces.
     */
    if (VisibleSurface != nullptr) {
        DEBUG_INFO("Deleting VisibleSurface\n");
        delete VisibleSurface;
        VisibleSurface = nullptr;
    }

    if (HiddenSurface != nullptr) {
        DEBUG_INFO("Deleting HiddenSurface\n");
        delete HiddenSurface;
        HiddenSurface = nullptr;
    }

    if (TileSurface != nullptr) {
        DEBUG_INFO("Deleting TileSurface\n");
        delete TileSurface;
        TileSurface = nullptr;
    }

    if (SidebarSurface != nullptr) {
        DEBUG_INFO("Deleting SidebarSurface\n");
        delete SidebarSurface;
        SidebarSurface = nullptr;
    }

    if (CompositeSurface != nullptr) {
        DEBUG_INFO("Deleting CompositeSurface\n");
        delete CompositeSurface;
        CompositeSurface = nullptr;
    }

    /**
     *  Set the new surface resultion.
     */
    VisibleRect = Rect(0, 0, width, height);
    VideoWidth = width;
    VideoHeight = height;
    DEBUG_INFO("VisibleRect: %dx%d\n", width, height);

    /**
     *  If the window size isn't set manually, resize the window to refect the new resolution.
     */
    if (WindowedMode) {
        int window_width = width;
        int window_height = height;

        /**
         *  If the window size isn't set manually, resize the window to refect the new resolution.
         */
        if (OptionsExtension->WindowWidth > 0 && OptionsExtension->WindowHeight > 0) {
            window_width = OptionsExtension->WindowWidth;
            window_height = OptionsExtension->WindowHeight;
        }

        /**
         *  Get the current window size and position.
         */
        int old_x, old_y, old_w, old_h;
        SDL_GetWindowPosition(SDLWindow, &old_x, &old_y);
        SDL_GetWindowSize(SDLWindow, &old_w, &old_h);

        /**
         *  Compute the current center point.
         */
        int center_x = old_x + old_w / 2;
        int center_y = old_y + old_h / 2;

        /**
         *  Compute new top-left corner so that the center stays the same.
         */
        int new_x = center_x - window_width / 2;
        int new_y = center_y - window_height / 2;

        /**
         *  Apply and save the new position and size.
         */
        SDL_SetWindowPosition(SDLWindow, new_x, new_y);
        SDL_SetWindowSize(SDLWindow, window_width, window_height);

        SDLWindowWidth = window_width;
        SDLWindowHeight = window_height;
        DEBUG_INFO("SDLWindow size: %d X %d.\n", SDLWindowWidth, SDLWindowHeight);
    }

    /**
     *  Recreate all the SDL intermediates (texture, renderer).
     */
    Set_Video_Mode(MainWindow, width, height, 16);

    /**
     *  Re-allocate all the game surfaces.
     */
    VisibleSurface = SDLSurface::Create_Primary();

    Rect temp = VisibleRect;
    temp.X = Options.SidebarSide || Debug_Map ? 0 : 168;
    temp.Y = 16;
    temp.Width -= 168;
    temp.Height -= 16;

    Allocate_Surfaces(VisibleRect, Rect(0, 0, temp.Width, VisibleRect.Height), Rect(0, 0, temp.Width, VisibleRect.Height), Rect(0, 0, 168, VisibleRect.Height));
    LogicalSurface = HiddenSurface;

    /**
     *  Reset the mouse cursor, since it's scaled.
     */
    Hide_Mouse();
    static_cast<SDLMouseClass*>(MouseCursor)->Recalc_Cursor_Image();
    Show_Mouse();

    /**
     *  Resize the game UI.
     */
    Map.Set_View_Dimensions(temp);
    Map.Init_IO();
    Map.Activate(1);
    Map.Set_Dimensions();
    Map.Flag_To_Redraw(GS_REDRAW_ALL);
    Show_Mouse();

    DEBUG_INFO("Mode change complete.\n");

    return true;
}
