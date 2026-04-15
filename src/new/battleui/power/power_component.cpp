/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Power bar component implementation.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "power_component.h"


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
PowerComponent::PowerComponent()
{
    View.Set_Model(&Model);
}


/**
 *  One-time initialization. Called once at game startup.
 *
 *  @author: ZivDero
 */
void PowerComponent::One_Time()
{
    View.One_Time();
}


/**
 *  Clears all state for a new scenario.
 *
 *  @author: ZivDero
 */
void PowerComponent::Init_Clear()
{
    Model.Init_Clear();
    View.Init_Clear();
}


/**
 *  No-op. The power bar has no interactive gadgets.
 *
 *  @author: ZivDero
 */
void PowerComponent::Init_IO()
{
}


/**
 *  Initializes for the current player house.
 *
 *  @author: ZivDero
 */
void PowerComponent::Init_For_House()
{
    View.Init_For_House();
}


/**
 *  Per-frame update. Updates the model and view.
 *
 *  @author: ZivDero
 */
void PowerComponent::AI(KeyNumType &key, Point2D &mouse)
{
    Model.AI();
    View.AI();
}


/**
 *  Renders the power bar.
 *
 *  @author: ZivDero
 */
void PowerComponent::Draw()
{
    View.Draw();
}


/**
 *  No-op. The power bar is drawn on the sidebar surface and blitted with it.
 *
 *  @author: ZivDero
 */
void PowerComponent::Blit(bool complete)
{
}


/**
 *  Reflows the power bar layout.
 *
 *  @author: ZivDero
 */
void PowerComponent::Shift_Sidebar()
{
    View.Shift_Sidebar();
}


/**
 *  Sets the available pixel height for the power bar.
 *
 *  @author: ZivDero
 */
void PowerComponent::Set_Height(int pixels)
{
    View.Set_Height(pixels);
}


/**
 *  Sets the sidebar view type used for layout config lookups.
 *
 *  @author: ZivDero
 */
void PowerComponent::Set_Sidebar_View_Type(SidebarViewType view_type)
{
    View.Set_Sidebar_View_Type(view_type);
}


/**
 *  Shuts down the power component.
 *
 *  @author: ZivDero
 */
void PowerComponent::Shutdown()
{
}


/**
 *  Returns help text for the power bar tooltip.
 *
 *  @author: ZivDero
 */
const char *PowerComponent::Help_Text(int gadget_id)
{
    return View.Help_Text(gadget_id);
}


/**
 *  Triggers the power bar flash effect.
 *
 *  @author: ZivDero
 */
void PowerComponent::Flash_Power()
{
    View.Flash_Power();
}
