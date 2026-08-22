/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Main-window Dear ImGui integration.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include <SDL3/SDL_render.h>
#include <windows.h>

namespace ViniferaImGui
{
    /**
     *  Initializes the main-window ImGui context and backends.
     */
    bool Initialize(HWND hwnd, SDL_Renderer* renderer);

    /**
     *  Shuts down the main-window ImGui context and backends.
     */
    void Shutdown();

    /**
     *  Forwards a Win32 message to ImGui and returns whether ImGui consumed it.
     */
    bool Process_Window_Message(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

    /**
     *  Renders the main-window ImGui frame through the active SDL renderer.
     */
    void Render();

    /**
     *  Renders only overlays intended for fullscreen modern movie playback.
     */
    void Render_Movie_Overlay();

    /**
     *  Returns whether the main-window ImGui context is initialized.
     */
    bool Is_Initialized();
}
