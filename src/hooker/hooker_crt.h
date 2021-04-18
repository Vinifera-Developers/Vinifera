/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOOKER_CRT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Hooks for CRT functions to ensure the project calls the same
 *                 versions as the original for functions that maintain state.
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


#include "hooker.h"


/**
 *  Define references to MSVC++ CRT functions in the target binary that maintain
 *  state only important if calls relying on state are made by both the original
 *  binary and the project.
 */
#define crt_strtok  (Make_Global<char *(__cdecl *const)(char *, const char *)>(0x006B602A))
#define crt_sscanf  (Make_Global<int (__cdecl *const)(const char *, const char *, ...)>(0x006B67B0))
#define crt_free    (Make_Global<void (__cdecl *const)(void *)>(0x006B67E4))
#define crt_malloc  (Make_Global<void *(__cdecl *const)(size_t)>(0x006B72CC))
#define crt_exit    (Make_Global<void *(__cdecl *const)(size_t)>(0x006B6EAA))


/**
 *  Redefine the following CRT functions to point to our references to the target binary.
 */
#ifdef strtok
#undef strtok
#endif
#define strtok crt_strtok

#ifdef sscanf
#undef sscanf
#endif
#define sscanf crt_sscanf

#ifdef free
#undef free
#endif
#define free crt_free

#ifdef malloc
#undef malloc
#endif
#define malloc crt_malloc

#ifdef exit
#undef exit
#endif
#define exit crt_exit
