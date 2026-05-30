/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended XSurface.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "xsurfaceext_hooks.h"

#include "hooker.h"
#include "xsurface.h"


typedef int OutCode;

/*
**	Build bits that indicate which end points lie outside the clipping rectangle.
**	Quick checks against these flag bits will speed the clipping process.
*/
const int CODE_INSIDE = 0;	// 0000
const int CODE_LEFT = 1;	// 0001
const int CODE_RIGHT = 2;	// 0010
const int CODE_BOTTOM = 4;	// 0100
const int CODE_TOP = 8;		// 1000


/***********************************************************************************************
 * Compute_Out_Code                                                                            *
 *                                                                                             *
 *    Compute the bit code for a point (x, y) using the clip rectangle.                        *
 *    Bounded diagonally by (xmin, ymin), and (xmax, ymax).                                    *
 *                                                                                             *
 * INPUT:   x     --                                                                           *
 *                                                                                             *
 *          y     --                                                                           *
 *                                                                                             *
 *          rect  --                                                                           *
 *                                                                                             *
 * OUTPUT:                                                                                     *
 *                                                                                             *
 * WARNINGS:   none                                                                            *
 *                                                                                             *
 * HISTORY:                                                                                    *
 *                                                                                             *
 *=============================================================================================*/
OutCode __forceinline Compute_Out_Code(double x, double y, Rect const& rect)
{
	OutCode code = CODE_INSIDE;

	int xx = rect.X + rect.Width;
	if (x >= xx) {
		code |= CODE_RIGHT;
	}
	else if (x < rect.X) {
		code |= CODE_LEFT;
	}

	int yy = rect.Y + rect.Height;
	if (y >= yy) {
		code |= CODE_BOTTOM;
	}
	else if (y < rect.Y) {
		code |= CODE_TOP;
	}

	return(code);
}

//
// Modified version of:
// https://en.wikipedia.org/wiki/Cohen%E2%80%93Sutherland_algorithm
//
// From the book "Computer Graphics. Principles and Practice in C".
//
// Reverse-engineered by tomsons26 (most likely, if not, then ZivDero or CCHyper).
// Afterwards, rewritten by ChatGPT.
//
bool Vinifera_Clip_Line(Point2D& point1, Point2D& point2, Rect const& rect)
{
    int outcode0 = Compute_Out_Code(point1.X, point1.Y, rect);
    int outcode1 = Compute_Out_Code(point2.X, point2.Y, rect);

    double x0 = point1.X;
    double y0 = point1.Y;
    double x1 = point2.X;
    double y1 = point2.Y;

    while (true)
    {
        // Trivial accept
        if (outcode0 == CODE_INSIDE && outcode1 == CODE_INSIDE)
        {
            point1.X = x0; point1.Y = y0;
            point2.X = x1; point2.Y = y1;
            return true;
        }

        // Trivial reject
        if (outcode0 & outcode1)
            return false;

        // Choose endpoint outside rect
        int outcodeOut = (outcode0 != CODE_INSIDE) ? outcode0 : outcode1;

        double x = 0.0;
        double y = 0.0;

        // Find intersection
        if (outcodeOut & CODE_TOP)           // above clip window
        {
            double dy = y1 - y0;
            if (dy == 0.0) return false; // horizontal line outside
            double slope_y = (x1 - x0) / dy;
            y = rect.Y;
            x = x0 + (y - y0) * slope_y;
        }
        else if (outcodeOut & CODE_BOTTOM)   // below clip window
        {
            double dy = y1 - y0;
            if (dy == 0.0) return false;
            double slope_y = (x1 - x0) / dy;
            y = rect.Y + rect.Height - 1;
            x = x0 + (y - y0) * slope_y;
        }
        else if (outcodeOut & CODE_RIGHT)    // to the right of clip window
        {
            double dx = x1 - x0;
            if (dx == 0.0) return false; // vertical line outside
            double slope_x = (y1 - y0) / dx;
            x = rect.X + rect.Width - 1;
            y = y0 + (x - x0) * slope_x;
        }
        else if (outcodeOut & CODE_LEFT)     // to the left of clip window
        {
            double dx = x1 - x0;
            if (dx == 0.0) return false;
            double slope_x = (y1 - y0) / dx;
            x = rect.X;
            y = y0 + (x - x0) * slope_x;
        }
        else
        {
            // Safety net: outcodeOut has no directional bits? -> break
            return false;
        }

        // Move the outside point to intersection and recalc code
        if (outcodeOut == outcode0)
        {
            x0 = x; y0 = y;
            outcode0 = Compute_Out_Code(x0, y0, rect);
        }
        else
        {
            x1 = x; y1 = y;
            outcode1 = Compute_Out_Code(x1, y1, rect);
        }
    }
}


void XSurfaceExtension_Hooks()
{
	Patch_Jump(0x006A8870, &Vinifera_Clip_Line);
}
