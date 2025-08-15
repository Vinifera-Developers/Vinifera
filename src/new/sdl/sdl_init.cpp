

#include <algorithm>

#include "debughandler.h"
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
    DEBUG_INFO("Prep SDL.\n");

    if (SDLWindow != nullptr) {
        return;
    }

    // Initialize SDL Video subsystem
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        DEBUG_WARNING("SDL_Init failed: %s\n", SDL_GetError());
        return;
    }

    SDL_PropertiesID props = SDL_CreateProperties();
    SDL_SetPointerProperty(props, SDL_PROP_WINDOW_CREATE_WIN32_HWND_POINTER, MainWindow); // wrap existing HWND
    if (!WindowedMode) {
        SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_FULLSCREEN_BOOLEAN, true); // fullscreen
    }

    SDLWindow = SDL_CreateWindowWithProperties(props);
    if (!SDLWindow) {
        DEBUG_WARNING("SDL_CreateWindowFrom failed: %s\n", SDL_GetError());
        return;
    }

    SDL_ShowWindow(SDLWindow);

    // Create a renderer for drawing
    SDLWindowRenderer = SDL_CreateRenderer(SDLWindow, nullptr);
    if (!SDLWindowRenderer) {
        DEBUG_WARNING("SDL_CreateRenderer failed: %s\n", SDL_GetError());
        SDL_DestroyWindow(SDLWindow);
        SDLWindow = nullptr;
        return;
    }

    SDL_Cursor* cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_DEFAULT);
    SDL_SetCursor(cursor); 

    DEBUG_INFO("SDL initialized with existing HWND.\n");
}


void Destroy_SDL()
{
    if (SDLWindowRenderer) {
        SDL_DestroyRenderer(SDLWindowRenderer);
        SDLWindowRenderer = nullptr;
    }
    if (SDLWindow) {
        SDL_DestroyWindow(SDLWindow); // Note: may or may not destroy MainWindow depending on SDL
        SDLWindow = nullptr;
    }

    SDL_QuitSubSystem(SDL_INIT_VIDEO);
    DEBUG_INFO("SDL destroyed.\n");
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
    HRESULT result;

    Prep_SDL();
#if 0
    //
    // Set the required display mode with 8 bits per pixel
    //
    DEBUG_INFO("SetDisplayMode: %dx%dx%d\n", w, h, bits_per_pixel);
    // MessageBox(MainWindow, "In Set_Video_Mode. About to call call SetDisplayMode.","Note", MB_ICONEXCLAMATION|MB_OK);
    result = DirectDrawObject->SetDisplayMode(w, h, bits_per_pixel);
    if (result != DD_OK) {
        //		Process_DD_Result(result, false);
        //		DirectDrawObject->Release();
        //		DirectDrawObject = NULL;
        DEBUG_INFO("SetDisplayMode failed\n");
        return (false);
    }
#endif

    VideoWidth = w;
    VideoHeight = h;
    VideoBitsPerPixel = bits_per_pixel;

#if 0
    if (bits_per_pixel == 8) {
        DEBUG_INFO("CreatePalette\n");
        //
        // Create a direct draw palette object
        //
        // MessageBox(MainWindow, "In Set_Video_Mode. About to call CreatePalette.","Note", MB_ICONEXCLAMATION|MB_OK);
        result = DirectDrawObject->CreatePalette(DDPCAPS_8BIT | DDPCAPS_ALLOW256, &PaletteEntries[0], &PalettePtr, nullptr);
        Process_DD_Result(result, false);
        if (result != DD_OK) {
            DEBUG_INFO("CreatePalette failed\n");
            return (false);
        }
    }

    Check_Overlapped_Blit_Capability();
    DEBUG_INFO("Display mode set\n");
#endif

    return (true);
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

    ///**
    // *  Deallocate surface.
    // */
    //SDL_DestroySurface(SDLWindowSurface);
    //SDLWindowSurface = nullptr;

    ///**
    // *  Deallocate texture.
    // */
    //SDL_DestroyTexture(SDLWindowTexture);
    //SDLWindowTexture = nullptr;

    ///**
    // *  Deallocate palette.
    // */
    //SDL_DestroyPalette(SDLPalette);
    //SDLPalette = nullptr;

    VideoWidth = 0;
    VideoHeight = 0;
    VideoBitsPerPixel = 0;
}


void SDL_Update_Visible_Surface(bool flip_mouse, Surface* surface, Rect* rect)
{
    Rect fill_rect;

    if (rect == NULL) {
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

    if (SDLWindow) {
        SDL_Surface* win_surf = SDL_GetWindowSurface(SDLWindow);
        if (win_surf) {
            SDL_Rect dst;
            dst.x = 0;
            dst.y = 0;
            dst.w = VisibleSurface->Get_Width();
            dst.h = VisibleSurface->Get_Height();

            // If VisibleSurface is a wrapper around SDL_Surface, just copy pixels
            SDL_BlitSurface(static_cast<SDLSurface*>(VisibleSurface)->Get_SDL_Surface(), nullptr, win_surf, &dst);

            // Present the new framebuffer
            SDL_UpdateWindowSurface(SDLWindow);
        }
    }

    if (flip_mouse && MouseCursor != nullptr) {
        Sleep(50);
        MouseCursor->Erase_Mouse(surface, false);
    }
}


/**
 *
 *
 *  @author: CCHyper
 */
static LRESULT CALLBACK WndProcWrapper(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
    int low_param = LOWORD(wParam);

    /**
     *  Call the games windows procedure.
     */
    LRESULT res = Main_Window_Procedure(hWnd, Message, wParam, lParam);

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

    const SDL_DisplayMode* dm = SDL_GetCurrentDisplayMode(0);

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
        int wnd_xpos = (dm->w - width) / 2;
        int wnd_ypos = (dm->h - height) / 2;

        if (SDLBorderless) {
            DEBUG_INFO("Creating borderless window.\n");
            SDL_SetBooleanProperty(props, SDL_PROP_WINDOW_CREATE_BORDERLESS_BOOLEAN, true);
        } else {
            wnd_ypos += 38; // Take into account the window title bar.
        }

        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_WIDTH_NUMBER, width);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_HEIGHT_NUMBER, height);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_X_NUMBER, wnd_xpos);
        SDL_SetNumberProperty(props, SDL_PROP_WINDOW_CREATE_Y_NUMBER, wnd_ypos);
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

    MainWindow = static_cast<HWND>(SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr));

    /**
     *  Set the games windows proc function to the window.
     */
    SetWindowLong(MainWindow, GWL_WNDPROC, (LONG)WndProcWrapper);

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
