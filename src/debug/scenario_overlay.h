/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Developer-mode scenario debug window.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once


namespace ScenarioOverlay
{
    /**
     *  When true, Draw() emits the scenario debug window on the current ImGui
     *  frame. Toggled by ToggleScenarioOverlayCommandClass.
     */
    extern bool IsVisible;

    /**
     *  Draws the scenario debug window. Must be called from inside an active
     *  ImGui frame, i.e. from ViniferaImGui::Render().
     */
    void Draw();
}
