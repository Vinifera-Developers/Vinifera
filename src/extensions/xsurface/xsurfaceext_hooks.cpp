/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          XSURFACEEXT_HOOKS.CPP
 *
 *  @author        Rampastring
 *
 *  @brief         Contains the hooks for the extended XSurface.
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

#include	"xsurfaceext_hooks.h"
#include	"xsurface.h"
#include	"hooker.h"


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
//
bool Vinifera_Clip_Line(Point2D& point1, Point2D& point2, Rect const& rect)
{
	int outcode0;
	int outcode1;
	int outcodeOut;

	double x = rect.X;
	double y = rect.Y;

	double x0 = point1.X;
	double y0 = point1.Y;
	double x1 = point2.X;
	double y1 = point2.Y;

	double slope_y = (x1 - x0) / (y1 - y0); // slope to use for possibly-vertical lines
	double slope_x = (y1 - y0) / (x1 - x0); // slope to use for possibly-horizontal lines

	/*
	**	Compute outcodes for P0, P1, and whatever point lies outside the clip rectangle.
	*/
	outcode0 = Compute_Out_Code(x0, y0, rect);
	outcode1 = Compute_Out_Code(x1, y1, rect);

AGAIN:
	if (outcode0 == CODE_INSIDE && outcode1 == CODE_INSIDE) {
		/*
		**	Both points inside window; trivially accept and return true.
		*/
		point1.X = x0;
		point1.Y = y0;
		point2.X = x1;
		point2.Y = y1;
		return(true);
	}

	/*
	**	Check to see if the line segment falls outside of the viewing rectangle.
	*/
	if (outcode0 & outcode1) {

		/*
		**	Bitwise AND is not 0: both points share an outside zone (LEFT, RIGHT, TOP,
		**	or BOTTOM), so both must be outside window; exit loop (result is false).
		*/
		return(false);
	}


	/*
	**	Failed both tests, so calculate the line segment to clip
	**	from an outside point to an intersection with clip edge.
	*/


	/*
	**	At least one endpoint is outside the clip rectangle; pick it.
	*/
	outcodeOut = (outcode0 != CODE_INSIDE) ? outcode0 : outcode1;

	/*
	**	Now find the intersection point;
	**	use formulas:
	**	slope = (y1 - y0) / (x1 - x0)
	**	x = x0 + (1 / slope) * (ym - y0), where ym is ymin or ymax
	**	y = y0 + slope * (xm - x0), where xm is xmin or xmax
	**	No need to worry about divide-by-zero because, in each case, the
	**	outcode bit being tested guarantees the denominator is non-zero
	*/
	if (outcodeOut & CODE_TOP) {			// point is above the clip window
		x = (rect.Y - y0) * slope_y + x0;
		y = rect.Y;
	}
	else if (outcodeOut & CODE_BOTTOM) {  // point is below the clip window
		x = ((rect.Y + rect.Height - 1) - y0) * slope_y + x0;
		y = (rect.Height + rect.Y - 1);
	}
	else if (outcodeOut & CODE_RIGHT) {   // point is to the right of clip window
		y = ((rect.X + rect.Width - 1) - x0) * slope_x + y0;
		x = (rect.Width + rect.X - 1);
	}
	else if (outcodeOut & CODE_LEFT) {	// point is to the left of clip window
		y = (rect.X - x0) * slope_x + y0;
		x = rect.X;
	}

	/*
	**	Now we move outside point to intersection point to clip
	**	and get ready for next pass.
	*/
	if (outcodeOut == outcode0) {
		x0 = x;
		y0 = y;
		outcode0 = Compute_Out_Code(x0, y0, rect);
	}
	else {
		x1 = x;
		y1 = y;
		outcode1 = Compute_Out_Code(x1, y1, rect);
	}

	goto AGAIN;

	return(false);
}


void XSurfaceExtension_Hooks()
{
	Patch_Jump(0x006A8870, &Vinifera_Clip_Line);
}