/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          VINIFERA_NEWDEL.CPP
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
#include "always.h"
#include "debughandler.h"
#include "newdel.h" // TS++ new and delete wrappers.
#include <new>

#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


int Vinifera_New_Count = 0;
int Vinifera_Delete_Count = 0;


/**
 *  Round up input value to nearest multiple of.
 */
static unsigned Round_Up(unsigned number, int a)
{
    return (number + (a - 1)) & (~(a - 1));
}


/**
 *  Implement wrappers for C memory functions.
 */
void * __cdecl vinifera_allocate(unsigned int size)
{
    /**
     *  Round up input size to nearest multiple of 4 for alignment.
     */
    unsigned r_size = Round_Up(size, 4);

    void *block_ptr = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, r_size);
    if (!block_ptr) {
        DEBUG_FATAL("Failed to allocate memory!\n");
    }
    return block_ptr;
}

void * __cdecl vinifera_count_allocate(unsigned int count, unsigned int size)
{
    /**
     *  Round up input size to nearest multiple of 4 for alignment.
     */
    unsigned r_size = Round_Up(size, 4);

    void *block_ptr = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, r_size * count);
    if (!block_ptr) {
        DEBUG_FATAL("Failed to allocate memory!\n");
    }
    return block_ptr;
}

void * __cdecl vinifera_reallocate(void *ptr, unsigned int size)
{
    /**
     *  Round up input size to nearest multiple of 4 for alignment.
     */
    unsigned r_size = Round_Up(size, 4);

    void *block_ptr = HeapReAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, ptr, r_size);
    if (!block_ptr) {
        DEBUG_FATAL("Failed to allocate memory!\n");
    }
    return block_ptr;
}

void __cdecl vinifera_free(void *ptr)
{
    bool freed = HeapFree(GetProcessHeap(), HEAP_ZERO_MEMORY, ptr);
    if (!freed) {
        DEBUG_FATAL("Failed to free memory!\n");
    }
    ASSERT(freed);
}


/**
 *  Overload the New and Delete operators to use our memory functions.
 */
void  __cdecl operator delete(void *ptr)
{
    vinifera_free(ptr);
}

//void  __cdecl operator delete(void *ptr, void *place) noexcept
//{
//    vinifera_free(ptr);
//}

void  __cdecl operator delete(void *ptr, const char *file, int line)
{
#ifndef NDEBUG
    //DEV_DEBUG_INFO("operator delete() called with from file: %s, line: %d.\n", file, line);
#endif

    vinifera_free(ptr);
}

void  __cdecl operator delete(void *ptr, const std::nothrow_t &tag)
{
    vinifera_free(ptr);
}

void  __cdecl operator delete[](void *ptr)
{
    vinifera_free(ptr);
}

//void  __cdecl operator delete[](void *ptr, void *place) noexcept
//{
//    vinifera_free(ptr);
//}

void  __cdecl operator delete[](void *ptr, const char *file, int line)
{
#ifndef NDEBUG
    //DEV_DEBUG_INFO("operator delete[]() called with from file: %s, line: %d.\n", file, line);
#endif

    vinifera_free(ptr);
}

void  __cdecl operator delete[](void *ptr, const std::nothrow_t &tag)
{
    vinifera_free(ptr);
}

void * __cdecl operator new(std::size_t size)
{
    return vinifera_allocate(size);
}

void * __cdecl operator new(std::size_t size, const char *file, int line)
{
#ifndef NDEBUG
    //DEV_DEBUG_INFO("operator new() called with size: %zd, from file: %s, line: %d.\n", size, file, line);
#endif

    return vinifera_allocate(size);
}

void * __cdecl operator new(std::size_t size, const std::nothrow_t &tag)
{
    return vinifera_allocate(size);
}

//void * __cdecl operator new(std::size_t size, void *place) noexcept
//{
//    return vinifera_allocate(size);
//}

void * __cdecl operator new[](std::size_t size)
{
    return vinifera_allocate(size);
}

void * __cdecl operator new[](std::size_t size, const char *file, int line)
{
#ifndef NDEBUG
    //DEV_DEBUG_INFO("operator new[]() called with size: %zd, from file: %s, line: %d.\n", size, file, line);
#endif

    return vinifera_allocate(size);
}

void * __cdecl operator new[](std::size_t size, const std::nothrow_t &tag)
{
    return vinifera_allocate(size);
}


/**
 *  Wrapper to allow _CrtDumpMemoryLeaks() to be parsed into atexit().
 */
static void __cdecl vinifera_dump_memory_leaks()
{
    _CrtDumpMemoryLeaks();
}


/**
 *  Init memory flags.
 */
void vinifera_init_memory()
{
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);

    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_DEBUG);
    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_DEBUG);

    std::atexit(vinifera_dump_memory_leaks);
}


/**
 *  Main function for patching the hooks.
 */
void Vinifera_Memory_Hooks()
{
    vinifera_init_memory();
}
