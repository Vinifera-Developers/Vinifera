/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_CONST.CPP
 *
 *  @authors       CCHyper
 *
 *  @brief         Constant values and strings.
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
#include "vinifera_const.h"


#ifndef RELEASE
/**
 *  78 characters max for 640 width!
 */
#if defined(NIGHTLY)
const char TXT_VINIFERA_NIGHTLY_BUILD[78] = { "This is a nightly build of Vinifera, please use with caution!" };
#elif defined(PREVIEW)
const char TXT_VINIFERA_PREVIEW_BUILD[78] = { "This is a preview build of Vinifera, please use with caution!" };
#else
const char TXT_VINIFERA_LOCAL_BUILD[78] = { "This is a local unofficial build of Vinifera, please use with caution!" };
const char TXT_VINIFERA_UNOFFICIAL_BUILD[78] = { "This is an unofficial build of Vinifera, please use with caution!" };
#endif
#endif
