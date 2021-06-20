/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          MAINLOOPEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the intercepting Main_Loop().
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
#include "mainloopext_hooks.h"
#include "tibsun_functions.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


static void Before_Main_Loop()
{
}


static void After_Main_Loop()
{
}


static void Main_Loop_Intercept()
{
	//DEV_DEBUG_INFO("Before Main_Loop()\n");

	Before_Main_Loop();

	Main_Loop();

	After_Main_Loop();

	//DEV_DEBUG_INFO("After Main_Loop()\n");
}

/**
 *  Main function for patching the hooks.
 */
void MainLoop_Hooks()
{
	Patch_Call(0x00462A8E, &Main_Loop_Intercept);
	Patch_Call(0x00462A9C, &Main_Loop_Intercept);
	Patch_Call(0x005A0B85, &Main_Loop_Intercept);
}
