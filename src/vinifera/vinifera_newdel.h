/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_NEWDEL.H
 *
 *  @author        CCHyper
 *
 *  @brief         Overriden new and delete operators.
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


#if 0

/**
 *  Define for debug memory allocation to include __FILE__ and __LINE__ for
 *  every memory allocation. This helps find memory leaks.
 */


/**
 *  Include Microsoft memory leak detection procedures.
 */
#define _CRTDBG_MAP_ALLOC

/**
 *  Exclude standard memory alloc procedures.
 */
//#define _INC_MALLOC

/**
 *  C runtime headers.
 */
//#include	<stdlib.h>
#include	<crtdbg.h>

// _alloca() declared in "malloc.h".
#include    <malloc.h>

/**
 *  Windows OS has native memory debugging capabilities. Re-define to use
 *  CRT debug versions with file and line number.
 */
#define		malloc(s)         _malloc_dbg(s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define		calloc(c, s)      _calloc_dbg(c, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define		realloc(p, s)     _realloc_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define		_expand(p, s)     _expand_dbg(p, s, _NORMAL_BLOCK, __FILE__, __LINE__)
#define		free(p)           _free_dbg(p, _NORMAL_BLOCK)
#define		_msize(p)         _msize_dbg(p, _NORMAL_BLOCK)

#undef _aligned_malloc
#define     _aligned_malloc(s, a)	_malloc_dbg(s, a, __FILE__, __LINE__)

#undef _aligned_free
#define     _aligned_free(s, a)	    _free_dbg(s, a)

// Allow to use _alloca(), _aligned_malloc(), _expand() and _msize() with no underscore.
#define     alloca             _alloca
#define     aligned_malloc     _aligned_malloc
#define     expand             _expand
#define     msize              _msize

/**
 *  Replace _NORMAL_BLOCK with _CLIENT_BLOCK if you want the allocations to
 *  be of _CLIENT_BLOCK type.
 */
#define     debug_new                  new(_NORMAL_BLOCK, __FILE__, __LINE__)
#define     debug_placement_new(...)   new(__VA_ARGS__)
#define     debug_delete               delete

#undef      new
#define     new            debug_new

#undef      delete
#define     delete         debug_delete

#endif


extern int Vinifera_New_Count;
extern int Vinifera_Delete_Count;

void * __cdecl vinifera_allocate(unsigned int size);
void * __cdecl vinifera_count_allocate(unsigned int count, unsigned int size);
void * __cdecl vinifera_reallocate(void *ptr, unsigned int size);
void __cdecl vinifera_free(void *ptr);

void vinifera_init_memory();


void Vinifera_Memory_Hooks();
