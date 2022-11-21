/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SETUP_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the main function that sets up all hooks.
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
#include "setup_hooks.h"

/**
 *  Include the hook headers here.
 */
#include "vinifera_newdel.h"
#include "crt_hooks.h"
#include "debug_hooks.h"
#include "vinifera_hooks.h"
#include "newswizzle_hooks.h"
#include "extension_hooks.h"
#include "cncnet4_hooks.h"
#include "cncnet5_hooks.h"


void Setup_Hooks()
{
    Vinifera_Memory_Hooks();

    CRT_Hooks();
    Debug_Hooks();
    Vinifera_Hooks();
    NewSwizzle_Hooks();
    Extension_Hooks();

    CnCNet4_Hooks();
    CnCNet5_Hooks();
}
