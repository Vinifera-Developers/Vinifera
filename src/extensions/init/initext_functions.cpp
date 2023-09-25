/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          INITEXT_FUNCTIONS.CPP
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
#include "vinifera_const.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "language.h"
#include "addon.h"
#include "cd.h"
#include "session.h"
#include "iomap.h"
#include "theme.h"
#include "dsaudio.h"
#include "vinifera_gitinfo.h"
#include "tspp_gitinfo.h"
#include "resource.h"
#include "debughandler.h"
#include "asserthandler.h"
#include <Windows.h>
#include <commctrl.h>

#include "options.h"


extern HMODULE DLLInstance;


/**
 *  Tiberian Sun resource constants.
 */
#define TS_MAINICON		    93
#define TS_MAINCURSOR		104