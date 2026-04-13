/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_RENDER_UTILS.H
 *
 *  @author        ZivDero
 *
 *  @brief         Shared sidebar rendering utilities. Used by all sidebar
 *                 views for drawing cameos, clock overlays, and tooltips.
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

#include "point.h"
#include "rect.h"

struct BuildItem;
class Surface;
class ConvertClass;
class ShapeSet;


/**
 *  Draws a cameo icon (PCX/PNG surface or shape) for a build item.
 */
void Draw_Cameo(Surface& surface, const Rect& rect, const BuildItem& item, const Point2D& point);

/**
 *  Draws the production clock overlay (green clock).
 */
void Draw_Clock_Overlay(Surface& surface, ConvertClass& drawer, const Rect& rect, const Point2D& point, int stage);

/**
 *  Draws the superweapon recharge clock overlay.
 */
void Draw_Recharge_Clock(Surface& surface, ConvertClass& drawer, const Rect& rect, const Point2D& point, int stage);

/**
 *  Draws the darkened overlay for unavailable items.
 */
void Draw_Darken_Overlay(Surface& surface, ConvertClass& drawer, const Rect& rect, const Point2D& point);

/**
 *  Draws the "READY" text on a completed item.
 */
void Draw_Ready_Text(Surface& surface, const Rect& rect, const Point2D& point, const char* text, int object_width);

/**
 *  Draws the "HOLD" text on a paused item.
 */
void Draw_Hold_Text(Surface& surface, const Rect& rect, const Point2D& point, int object_width, bool has_queue_count);

/**
 *  Draws the queue count number on a production item.
 */
void Draw_Queue_Count(Surface& surface, const Rect& rect, const Point2D& point, int count);

/**
 *  Draws the hover highlight rectangle on a cameo.
 */
void Draw_Hover_Highlight(Surface& surface, const Rect& cameo_rect);

/**
 *  Draws the cameo name text below the icon.
 */
void Draw_Cameo_Name(Surface& surface, const Rect& rect, const Point2D& point, const char* name, int object_width);

/**
 *  Formats the tooltip text for a BuildItem (name, cost, description).
 *  Returns a pointer to a static buffer, or nullptr if invalid.
 */
const char* Format_Cameo_Tooltip(const BuildItem& item);
