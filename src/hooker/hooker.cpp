/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOOKER.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Provides methods for accessing data and functions in an
 *                 existing binary and replacing functions with new 
 *                 implementations from an injected DLL.
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
#include "hooker.h"
#include <cassert>


extern HMODULE DLLInstance;

static const int TextSegementStart = 0x00401000;  // Start of the .text segment in the binary.
static const int GameBinarySize = VINIFERA_TARGET_SIZE;  // Raw size of binary in bytes.
static DWORD OldProtect = 0;


void StartHooking()
{
	/**
	 *  Change the protection of the .text segment to READ+WRITE in the target binary.
	 *  This allows us to modify code and place hooks in the binary.
	 */
    VirtualProtectEx(GetCurrentProcess(), (LPVOID)&TextSegementStart, GameBinarySize, PAGE_EXECUTE_READWRITE, &OldProtect);
}


void StopHooking()
{
	/**
	 *  Restore the protection of the .text segment back to the original access.
	 */
    DWORD OldProtect2;
    VirtualProtectEx(GetCurrentProcess(), (LPVOID)&TextSegementStart, GameBinarySize, OldProtect, &OldProtect2);
}
