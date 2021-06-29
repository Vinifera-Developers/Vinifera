/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_GLOBALS.CPP
 *
 *  @authors       CCHyper
 *
 *  @brief         Vinifera global values.
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

#include "vinifera_globals.h"


bool Vinifera_DeveloperMode = false;

char Vinifera_DebugDirectory[PATH_MAX] = { "Debug" };

bool Vinifera_Developer_InstantBuild = false;
bool Vinifera_Developer_AIInstantBuild = false;
bool Vinifera_Developer_BuildCheat = false;
bool Vinifera_Developer_Unshroud = false;
bool Vinifera_Developer_ShowCursorPosition = false;
bool Vinifera_Developer_FrameStep = false;
int Vinifera_Developer_FrameStepCount = 0;
bool Vinifera_Developer_AIControl = false;

bool Vinifera_SkipWWLogoMovie = false;
bool Vinifera_SkipStartupMovies = false;
