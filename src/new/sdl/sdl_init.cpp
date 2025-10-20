#include <algorithm>

#include "sdl_init.h"
#include "debughandler.h"
#include "filepng.h"
#include "mouse.h"
#include "rect.h"
#include "sdlsurface.h"
#include "tactical.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"
#include "wwmouse.h"
#include "SDL3/SDL_init.h"
#include "SDL3/SDL_oldnames.h"
#include "SDL3/SDL_version.h"
#include "SDL3/SDL_video.h"

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


void Prep_SDL()
{

}


void Destroy_SDL()
{

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
    if (!SDLWindow) {
        DEBUG_ERROR("SDLWindow is null!\n");
        return false;
    }

    if (SDLWindowRenderer) {
        DEBUG_WARNING("Video mode has already been set!\n");
        return true;
    }

    SDL_PixelFormat pixel_format = SDL_GetWindowPixelFormat(SDLWindow);
    if (pixel_format == SDL_PIXELFORMAT_UNKNOWN || SDL_BITSPERPIXEL(pixel_format) < 16) {
        DEBUG_ERROR("SDL2 window pixel format unsupported: %s (%d bpp)\n", SDL_GetPixelFormatName(pixel_format), SDL_BITSPERPIXEL(pixel_format));
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


void SDL_Update_Visible_Surface(bool flip_mouse, Surface* surface, Rect* rect)
{
    Rect fill_rect;

    if (rect == nullptr) {
        fill_rect = surface->Get_Rect();
        rect = &fill_rect;
    }

    RECT client_rect;

    if (!GetClientRect(MainWindow, &client_rect)) {
        return;
    }

    POINT screen_pt;

    screen_pt.x = client_rect.left;
    screen_pt.y = client_rect.top;
    if (!ClientToScreen(MainWindow, &screen_pt)) {
        return;
    }

    Rect dest_rect(screen_pt.x, screen_pt.y, surface->Get_Width(), surface->Get_Height());

    // Screen shake handling
    if (Map.ScreenX == 0 && Map.ScreenY == 0) {
        // Do nothing
    } else {
        if (Map.ScreenX > 0) {
            dest_rect.X += Map.ScreenX;
            dest_rect.Width -= Map.ScreenX;
        } else if (Map.ScreenX < 0) {
            dest_rect.Width += Map.ScreenX;
        }
        if (Map.ScreenY > 0) {
            dest_rect.Y += Map.ScreenY;
            dest_rect.Height -= Map.ScreenY;
        } else if (Map.ScreenY < 0) {
            dest_rect.Height += Map.ScreenY;
        }
    }

    // Adjust for sidebar position
    if (!Options.SidebarSide && !Debug_Map) {

        dest_rect.X += std::max(std::min(SidebarSurface->Get_Width(), VisibleRect.Width - dest_rect.Width), 0);
    }

    // Copy input rect to source rect
    Rect src_rect = *rect;

    // Apply zoom factor if tactical map is zoomed
    if (TacticalMap && (TacticalMap->ZoomFactor != 1.0)) {

        // Compute zoomed source rect centered
        int zoom_surface_width = surface->Get_Width();
        int zoom_surface_height = surface->Get_Height();

        double zoomed_width = (double)zoom_surface_width / TacticalMap->ZoomFactor;
        double zoomed_height = (double)zoom_surface_height / TacticalMap->ZoomFactor;

        src_rect = Rect((int)(((double)zoom_surface_width - zoomed_width) / 2.0), (int)(((double)zoom_surface_height - zoomed_height) / 2.0), (int)zoomed_width, (int)zoomed_height);

    } else {

        src_rect.Width = std::min(src_rect.Width, dest_rect.Width);
        src_rect.Height = std::min(src_rect.Height, dest_rect.Height);
    }

    if (Map.ScreenX < 0) {
        src_rect.X -= Map.ScreenX;
    }

    if (Map.ScreenY < 0) {
        src_rect.Y -= Map.ScreenY;
    }

    // Draw filler for X offset
    if (Map.ScreenX != 0) {

        fill_rect.Set(fill_rect.X, dest_rect.Y, abs(Map.ScreenX), surface->Get_Height());
        fill_rect.X = Map.ScreenX < 0 ? dest_rect.X + dest_rect.Width : dest_rect.X - Map.ScreenX;

        VisibleSurface->Fill_Rect(VisibleSurface->Get_Rect(), fill_rect, 0);
    }

    // Draw filler for Y offset
    if (Map.ScreenY != 0) {

        fill_rect.Set(dest_rect.X, fill_rect.Y, surface->Get_Width(), abs(Map.ScreenY));
        fill_rect.Y = Map.ScreenY < 0 ? dest_rect.Y + dest_rect.Height : dest_rect.Y - Map.ScreenY;

        VisibleSurface->Fill_Rect(VisibleSurface->Get_Rect(), fill_rect, 0);
    }

    if (flip_mouse && MouseCursor != nullptr) {
        MouseCursor->Draw_Mouse(surface, false);
    }

    // Now blit the source surface to the visible surface
    static int& _dialog_count = *reinterpret_cast<int*>(0x007E492C);
    VisibleSurface->Blit_From(dest_rect, *surface, src_rect, false, _dialog_count == 0);

    if (flip_mouse && MouseCursor != nullptr) {
        Sleep(50);
        MouseCursor->Erase_Mouse(surface, false);
    }
}


static LRESULT CALLBACK GameMessageHook(int code, WPARAM wParam, LPARAM lParam)
{
    if (code == HC_ACTION) {
        MSG* msg = reinterpret_cast<MSG*>(lParam);

        // Dispatch to your game’s existing handler
        // (e.g. OwnerDraw::DefaultDialogProc, etc.)
        Windows_Procedure(msg->hwnd, msg->message, msg->wParam, msg->lParam);
    }

    return CallNextHookEx(nullptr, code, wParam, lParam);
}


/**
 *
 *
 *  @author: CCHyper
 */
static LRESULT CALLBACK Windows_Procedure_Wrapper(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    int low_param = LOWORD(wParam);

    /**
     *  Call the games windows procedure.
     */
    LRESULT res = Windows_Procedure(hWnd, Message, wParam, lParam);

    switch (Message) {
    case WM_MOVE:
        MouseCursor->Calc_Confining_Rect();
        // SDL_WarpMouseInWindow(SDLWindow, WWMouse->Get_Mouse_X(), WWMouse->Get_Mouse_Y());
        break;

    case WM_ACTIVATEAPP:
        SDL_SetWindowMouseGrab(SDLWindow, true);
        break;

    case WM_ACTIVATE:
        if (low_param == WA_INACTIVE) {
            SDL_SetWindowMouseGrab(SDLWindow, false);
        }

    default:
        break;
    };

    return res;
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

    if (SDLBorderlessFullscreen) {
        DEBUG_INFO("Creating fullscreen desktop window.\n");
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true);

    } else {
        //int wnd_xpos;
        //int wnd_ypos;

        //int num_displays;
        //SDL_DisplayID* displays = SDL_GetDisplays(&num_displays);
        //if (num_displays > 0) {
        //    const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(displays[0]);
        //    wnd_xpos = (dm->w - width) / 2;
        //    wnd_ypos = (dm->h - height) / 2;
        //} else {
        //    wnd_xpos = 0;
        //    wnd_ypos = 0;
        //}

        //if (SDLBorderless) {
        //    DEBUG_INFO("Creating borderless window.\n");
        //    SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true);
        //} else {
        //    wnd_ypos += 38; // Take into account the window title bar.
        //}

        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, SDL_WINDOWPOS_CENTERED);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, SDL_WINDOWPOS_CENTERED);
    }

    const char* window_title = "Tiberian Sun";
    char buffer[64];
    std::snprintf(buffer, sizeof(buffer), "%s (SDL)", window_title);

    SDL_SetStringProperty(props, SDL_PROP_WINDOW_CREATE_TITLE_STRING, buffer);

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
     *  Do various stuff to make the SDL window intersect with the game correctly.
     */

    // SDL_ShowCursor(SDL_DISABLE);

    SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    SDL_SetCursor(cursor);

    MainWindow = static_cast<HWND>(SDL_GetPointerProperty(SDL_GetWindowProperties(SDLWindow), SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

    /**
     *  Set the games windows proc function to the window.
     */
    SetWindowsHookEx(WH_GETMESSAGE, GameMessageHook, nullptr, GetCurrentThreadId());
    //SetWindowLong(MainWindow, GWL_WNDPROC, (LONG)Windows_Procedure_Wrapper);

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
bool SDL_Update_Screen(SDLSurface* surface)
{
    // DEBUG_INFO("SDL_Update_Screen\n");

    SDL_RenderClear(SDLWindowRenderer);

    /**
     *  Blit games surface to SDL's window surface.
     */
    if (surface) {

        SDL_Surface* surf = surface->Get_SDL_Surface();

        /**
         *  Convert the 16bit pixel data from the surface to the SDL window 32bit texture.
         */
        //void* pixels;
        //int pitch;
        //SDL_LockTexture(SDLWindowTexture, nullptr, &pixels, &pitch);
        //SDL_ConvertPixels(surf->w, surf->h, surf->format, surf->pixels, surf->pitch, SDL_PIXELFORMAT_RGB565, pixels, pitch);
        //SDL_UnlockTexture(SDLWindowTexture);

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
        SDL_RenderTexture(SDLWindowRenderer, SDLWindowTexture, nullptr, nullptr);
    }

    /**
     *  Update the renderer to the window.
     */
    SDL_RenderPresent(SDLWindowRenderer);

    return true;
}
