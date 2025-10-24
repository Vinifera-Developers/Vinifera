#include <algorithm>

#include "sdl_init.h"

#include "cctooltip.h"
#include "cdctrl.h"
#include "command.h"
#include "convert.h"
#include "debughandler.h"
#include "filepng.h"
#include "mouse.h"
#include "optionsext.h"
#include "playmovie.h"
#include "rect.h"
#include "sdlsurface.h"
#include "tactical.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "windialog.h"
#include "wsproto.h"
#include "wwmouse.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_version.h"
#include "SDL3/SDL_video.h"

#include <windowsx.h>

bool SDL_Allocate_Surfaces(const Rect& hidden_rect, const Rect& composite_rect, const Rect& tile_rect, const Rect& sidebar_rect, bool hidden_first)
{
    bool success = true;

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

    return (success);
}


/***********************************************************************************************
 * Set_Video_Mode -- Initializes Direct Draw and sets the required Video Mode                  *
 *                                                                                             *
 * INPUT:  		int width   			- the width of the video mode in pixels						  *
 *					int height           - the height of the video mode in pixels                   *
 *					int bits_per_pixel	- the number of bits per pixel the video mode supports     *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/26/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
bool SDL_Set_Video_Mode(HWND, int w, int h, int bits_per_pixel)
{
    if (SDLWindow == nullptr) {
        DEBUG_ERROR("SDLWindow is null!\n");
        return false;
    }

    SDL_Reset_Video_Mode();

    SDL_PixelFormat pixel_format = SDL_GetWindowPixelFormat(SDLWindow);
    if (pixel_format == SDL_PIXELFORMAT_UNKNOWN || SDL_BITSPERPIXEL(pixel_format) < 16) {
        DEBUG_ERROR("SDL3 window pixel format unsupported: %s (%d bpp)\n", SDL_GetPixelFormatName(pixel_format), SDL_BITSPERPIXEL(pixel_format));
        return false;
    }

    DEBUG_INFO("Pixel format: %s (%d bpp)\n", SDL_GetPixelFormatName(pixel_format), SDL_BITSPERPIXEL(pixel_format));

    /**
     *  Create renderer for window.
     */
    SDLWindowRenderer = SDL_CreateRenderer(SDLWindow, nullptr);
    if (SDLWindowRenderer == nullptr) {
        DEBUG_ERROR("SDLWindowRenderer could not be created! SDL Error: %s\n", SDL_GetError());
        return false;
    }
    DEBUG_INFO("SDLWindowRenderer created.\n");

    if (OptionsExtension->ScaleMode != SDL_SCALEMODE_INVALID) {
        SDL_SetDefaultTextureScaleMode(SDLWindowRenderer, OptionsExtension->ScaleMode);
    }

    /**
     *  Create window texture.
     */
    SDLWindowTexture = SDL_CreateTexture(SDLWindowRenderer, SDL_PIXELFORMAT_RGB565, SDL_TEXTUREACCESS_STREAMING, w, h);
    if (SDLWindowTexture == nullptr) {
        DEBUG_ERROR("SDLWindowTexture could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    DEBUG_INFO("SDLWindowTexture created.\n");

    /**
     *  Explicitly set input focus to the window.
     */
    SDL_RaiseWindow(SDLWindow);
    GameInFocus = true; // The SDL window needs this initially otherwise we need to alt-tab to gain focus.

    VideoWidth = w;
    VideoHeight = h;
    VideoBitsPerPixel = bits_per_pixel;

    return true;
}

/***********************************************************************************************
 * Reset_Video_Mode -- Resets video mode and deletes Direct Draw Object                        *
 *                                                                                             *
 * INPUT:		none                                                                            *
 *                                                                                             *
 * OUTPUT:     none                                                                            *
 *                                                                                             *
 * WARNINGS:                                                                                   *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *   09/26/1995 PWG : Created.                                                                 *
 *=============================================================================================*/
void SDL_Reset_Video_Mode()
{
    /**
     *  Destroy renderer.
     */
    SDL_DestroyRenderer(SDLWindowRenderer);
    SDLWindowRenderer = nullptr;

    /**
     *  Deallocate texture.
     */
    SDL_DestroyTexture(SDLWindowTexture);
    SDLWindowTexture = nullptr;

    VideoWidth = 0;
    VideoHeight = 0;
    VideoBitsPerPixel = 0;
}

static WNDPROC SDL_Proc = nullptr;

LRESULT CALLBACK SDL_Windows_Procedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    if (SDL_Should_Scale()) {
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
        case WM_XBUTTONUP: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            x = static_cast<int>(x * SDL_XScale());
            y = static_cast<int>(y * SDL_YScale());

            lParam = MAKELPARAM(x, y);
            break;
        }
        }
    }

    /*
    **  Pass on any messages intended for the winsock message handler.
    */
    if (PacketTransport) {
        if (message == (UINT)PacketTransport->Protocol_Event_Message()) {
            if (PacketTransport->Message_Handler(hwnd, message, wParam, lParam)) {
                //return CallWindowProc(SDL_Proc, hwnd, message, wParam, lParam);
            } else {
                //return 0;
            }
        }
    }

    Map.Message_Handler(hwnd, message, wParam, lParam);

    if (MainWindow) {
        GetMenu(MainWindow);
    }

    switch (message) {

    case WM_SHOWWINDOW:
        break; // return 0;

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
        break;

    case WM_ERASEBKGND:
        break; // return 1;

    case WM_CLOSE:
        CDControl.Unlock_All_CD_Trays();
        break;

    case WM_CREATE:
        ToolTips = new CCToolTip(hwnd);
        if (ToolTips) {
            ToolTips->Set_Timer_Delay(500);
        }
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
        break; // return 0;

    case WM_ACTIVATEAPP:
        if (hwnd == MainWindow && GameInFocus != (wParam != 0)) {
            GameInFocus = wParam != 0;
            if (!GameInFocus) {
                Focus_Loss();
                //DEBUG_INFO("Focus lost\n");
            } else {
                Focus_Restore();
                //DEBUG_INFO("Focus gained\n");
            }
            SurfacesRestored = true;
        }
        break; // return 0;

    case WM_RBUTTONUP:
        Map.field_1D0C = false;
        break;

    case WM_MOVING:
        On_WM_MOVING(hwnd, wParam, lParam);
        return CallWindowProc(SDL_Proc, hwnd, message, wParam, lParam);

    case WM_MOUSEWHEEL:
        if (!_MouseWheel) {
            _MouseWheel = true;
            if (GET_WHEEL_DELTA_WPARAM(wParam) < 0) {
                Do_Command("SidebarDown");
            } else {
                Do_Command("SidebarUp");
            }
            _MouseWheel = false;
        }
        break;

    case WM_SYSCOMMAND:
        switch (wParam) {

        case SC_CLOSE:
            CDControl.Unlock_All_CD_Trays();
            /*
            **  Windows sent us a close message. Probably in response to Alt-F4. Ignore it by
            **  pretending to handle the message and returning true;
            */
            break; // return 0;

        case SC_SCREENSAVE:
            /*
            **  Windoze is about to start the screen saver. If we just return without passing
            **  this message to DefWindowProc then the screen saver will not be allowed to start.
            */
            return 0;
        }
        break;
    }

    /*
    **  Pass this message through to the keyboard handler. If the message
    **  was processed and requires no further action, then return with
    **  this information.
    */
    if (Keyboard->Message_Handler(hwnd, message, wParam, lParam)) {
        //return 0;
    }

    return CallWindowProc(SDL_Proc, hwnd, message, wParam, lParam);
}


LRESULT CALLBACK Combined_Windows_Procedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // force controls to repaint when we activate the window - so that when we alt-tab back in we don't
    // end up with invisible menus.
    if (msg == WM_ACTIVATEAPP) {
        GameInFocus = (wParam != 0);
        if (GameInFocus) {
            // Force all child controls to redraw when regaining focus.
            EnumChildWindows(
                hwnd,
                [](HWND child, LPARAM) -> BOOL {
                    RedrawWindow(child, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
                    return TRUE;
                },
                0);

            // Optionally redraw the main window, too
            RedrawWindow(hwnd, nullptr, nullptr, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
        }
    }

    if (SDL_Should_Scale()) {
        switch (msg) {
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
        case WM_XBUTTONUP: {
            int x = GET_X_LPARAM(lParam);
            int y = GET_Y_LPARAM(lParam);

            x = static_cast<int>(x * SDL_XScale());
            y = static_cast<int>(y * SDL_YScale());

            lParam = MAKELPARAM(x, y);
            break;
        }
        }
    }

    switch (msg) {
    case WM_ERASEBKGND:
        return 1; // skip default background erase

        // case WM_SETFOCUS:
        //     EnumChildWindows(
        //         MainWindow,
        //         [](HWND hwnd, LPARAM) -> BOOL {
        //             InvalidateRect(hwnd, NULL, TRUE);
        //             return TRUE;
        //         },
        //         0);
        //     break;
    }

    // 2. Feed other messages to the game's original handler
    LRESULT game_result = Windows_Procedure(hwnd, msg, wParam, lParam);

    // 3. Optionally let SDL see everything else too, if you want SDL to handle unknowns
    LRESULT sdl_result = CallWindowProc(SDL_Proc, hwnd, msg, wParam, lParam);

    // 4. Decide which result to return
    // Normally, return the game's result unless SDL needs to override (rare)
    return game_result ? game_result : sdl_result;
}


/**
 *
 *
 *  @author: CCHyper
 */
bool SDL_Create_Main_Window(HINSTANCE hInstance, int width, int height)
{
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        DEBUG_ERROR("SDL_Init failed! SDL_Error: %s\n", SDL_GetError());
        return false;
    }

    SDL_PropertiesID props = SDL_CreateProperties();

    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_HIDDEN_BOOLEAN, false);

    if (SDLClipMouseToWindow) {
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_MOUSE_GRABBED_BOOLEAN, true);
        SDL_SetWindowMouseGrab(SDLWindow, true);
    }

    if (!WindowedMode) {
        DEBUG_INFO("Creating fullscreen desktop window.\n");
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);

    } else {
        if (SDLBorderless) {
            DEBUG_INFO("Creating borderless window.\n");
            SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true);
        }

        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
    }

    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, "Tiberian Sun");

    /**
     *  Create the window.
     */
    SDLWindow = SDL_CreateWindowWithProperties(props);
    if (SDLWindow == nullptr) {
        DEBUG_ERROR("SDLWindow could not be created! SDL_Error: %s\n", SDL_GetError());
        return false;
    }
    props = SDL_GetWindowProperties(SDLWindow);
    DEBUG_INFO("SDLWindow created.\n");

    SDL_GetWindowSize(SDLWindow, &SDLWindowWidth, &SDLWindowHeight);
    DEBUG_INFO("SDLWindow size: %d X %d.\n", SDLWindowWidth, SDLWindowHeight);

    /**
     *  Do various stuff to make the SDL window intersect with the game correctly.
     */
    //SDL_SetWindowRelativeMouseMode(SDLWindow, true);

    MainWindow = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

    LONG_PTR style = GetWindowLongPtr(MainWindow, GWL_STYLE);
    style &= ~(WS_CLIPSIBLINGS | WS_CLIPCHILDREN);
    SetWindowLongPtr(MainWindow, GWL_STYLE, style);

    /**
     *  Set the games windows proc function to the window.
     */
    SDL_Proc = (WNDPROC)SetWindowLongPtr(MainWindow, GWLP_WNDPROC, (LONG_PTR)Combined_Windows_Procedure);

    GameInFocus = true;

    return true;
}


/**
 *
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
 *  @author: CCHyper, tomsons26
 */
bool SDL_Update_Screen(Surface* surface)
{
    // DEBUG_INFO("SDL_Update_Screen\n");

    SDL_RenderClear(SDLWindowRenderer);

    /**
     *  Blit games surface to SDL's window surface.
     */
    if (surface) {

        /**
         *  Update the window texture.
         */
        if (void* pixels = surface->Lock()) {
            SDL_UpdateTexture(SDLWindowTexture, nullptr, pixels, surface->Stride());
            surface->Unlock();
        }

        /**
         *  Copy the texture to the renderer.
         */
        if (!SDL_Should_Scale()) {
            Rect src_rect = surface->Get_Rect();
            SDL_FRect dst_rect = {static_cast<float>(src_rect.X), static_cast<float>(src_rect.Y), static_cast<float>(src_rect.Width), static_cast<float>(src_rect.Height)};
            SDL_RenderTexture(SDLWindowRenderer, SDLWindowTexture, nullptr, &dst_rect);
        } else {
            SDL_RenderTexture(SDLWindowRenderer, SDLWindowTexture, nullptr, nullptr);
        }
        
    }

    /**
     *  Update the renderer to the window.
     */
    SDL_RenderPresent(SDLWindowRenderer);

    return true;
}


bool SDL_Should_Scale()
{
    return WSDialogCount == 0;
}

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

        if (OptionsExtension->WindowWidth > 0 && OptionsExtension->WindowHeight > 0) {
            window_width = OptionsExtension->WindowWidth;
            window_height = OptionsExtension->WindowHeight;
        }

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
    temp.X = ((Options.SidebarSide || Debug_Map) ? 0 : 168);
    temp.Y = 16;
    temp.Width -= 168;
    temp.Height -= 16;

    Allocate_Surfaces(VisibleRect, Rect(0, 0, temp.Width, VisibleRect.Height), Rect(0, 0, temp.Width, VisibleRect.Height), Rect(0, 0, 168, VisibleRect.Height));
    LogicalSurface = HiddenSurface;

    /**
     *  Reset the mouse cursor, since it's scaled.
     */
    void const* mouseshp = MFCD::Retrieve("MOUSE.SHP");
    if (mouseshp != nullptr) {
        Point2D hotspot = Point2D(0, 0);
        Set_Mouse_Cursor(hotspot, static_cast<ShapeSet const*>(mouseshp), 0);
    }

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
