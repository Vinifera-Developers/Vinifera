/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RESOURCE.H
 *
 *  @author        CCHyper
 *
 *  @brief         Windows resources include file.
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

#ifndef _WINUSER_
#include <winres.h>
#endif


/**
 *  Icon with lowest ID value placed first to ensure application icon
 *  remains consistent on all systems.
 */
#define IDI_VINIFERA     100


/**
 *  Use these defines in code to allow easy updating in all areas used.
 */
#define MAINICON    IDI_VINIFERA
#define MAINCURSOR  IDC_ARROW

