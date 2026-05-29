/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  In-game ImGui debug overlay window.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


namespace DebugOverlay
{
    /**
     *  When true, Draw() emits the debug window on the current ImGui frame.
     *  Toggled by ToggleDebugOverlayCommandClass.
     */
    extern bool IsVisible;

    /**
     *  Draws the debug overlay window. Must be called from inside an active
     *  ImGui frame (between NewFrame/Render), i.e. from ViniferaImGui::Render().
     */
    void Draw();
}
