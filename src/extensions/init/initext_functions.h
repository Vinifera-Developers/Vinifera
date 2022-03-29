/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INITEXT_FUNCTIONS.H
 *
 *  @author        CCHyper
 *
 *  @brief         Contains supporting functions for game init process.
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

#include "always.h"
#include "tibsun_defines.h"


void Vinifera_Create_Main_Window(HINSTANCE hInstance, int nCmdShow, int width, int height);
bool Vinifera_Prep_For_Side(SideType side);
bool Vinifera_Init_Secondary_Mixfiles();
bool Vinifera_Init_Expansion_Mixfiles();
bool Vinifera_Init_Bootstrap_Mixfiles();
