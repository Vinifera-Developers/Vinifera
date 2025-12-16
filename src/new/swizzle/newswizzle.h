/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NEWSWIZZLE.H
 *
 *  @author        CCHyper
 *
 *  @contributors  tomsons26, ZivDero
 *
 *  @brief         Replacement pointer swizzling interface for debugging save load issues.
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
#include "iswizzle.h"
#include "tibsun_defines.h"
#include "vector.h"
#include "vinifera_defines.h"
#include <cstdio>
#include <string>
#include <unordered_map>


/**
 *  Replacement of SwizzleManagerClass.
 */
class ViniferaSwizzleManagerClass : public ISwizzle
{
private:
    struct SwizzlePointerStruct {
        SwizzlePointerStruct() : ID(-1), Pointer(nullptr), Line(-1) {}

        SwizzlePointerStruct(LONG id, void* pointer, const char* file = nullptr, const int line = -1, const char* func = nullptr, const char* var = nullptr) : ID(id), Pointer(pointer), Line(line)
        {
            if (file != nullptr) {
                File = file;
            }

            if (func != nullptr) {
                Function = func;
            }

            if (var != nullptr) {
                Variable = var;
            }
        }

        /**
         *  Enable move semantics.
         */
        SwizzlePointerStruct(SwizzlePointerStruct&&) noexcept = default;
        SwizzlePointerStruct& operator=(SwizzlePointerStruct&&) noexcept = default;

        /**
         *  The id of the pointer to remap.
         */
        LONG ID;

        /**
         *  The pointer to fixup.
         */
        void* Pointer;

        /**
         *  Debugging information.
         */
        std::string File;
        int Line;
        std::string Function;
        std::string Variable;
    };

public:
    /**
     *  IUnknown
     */
    STDMETHOD(QueryInterface)(REFIID riid, LPVOID* ppvObj) override;
    STDMETHOD_(ULONG, AddRef)() override;
    STDMETHOD_(ULONG, Release)() override;

    /**
     *  ISwizzle
     */
    STDMETHOD_(LONG, Reset)() override;
    STDMETHOD_(LONG, Swizzle)(void** pointer) override;
    STDMETHOD_(LONG, Fetch_Swizzle_ID)(void* pointer, LONG* id) override;
    STDMETHOD_(LONG, Here_I_Am)(LONG id, void* pointer) override;
    STDMETHOD(Save_Interface)(IStream* stream, IUnknown* pointer) override;
    STDMETHOD(Load_Interface)(IStream* stream, CLSID* riid, void** pointer) override;
    STDMETHOD_(LONG, Get_Save_Size)(LONG* size) override;

    /**
     *  New debug routines.
     */
    STDMETHOD_(LONG, Swizzle_Dbg)(void** pointer, const char* file, const int line, const char* func = nullptr, const char* var = nullptr);
    STDMETHOD_(LONG, Fetch_Swizzle_ID_Dbg)(void* pointer, LONG* id, const char* file, const int line, const char* func = nullptr, const char* var = nullptr);
    STDMETHOD_(LONG, Here_I_Am_Dbg)(LONG id, void* pointer, const char* file, const int line, const char* func = nullptr, const char* var = nullptr);

public:
    ViniferaSwizzleManagerClass();
    ~ViniferaSwizzleManagerClass();

private:
    void Process_Tables();

private:
    /**
     *  List of all the pointers that need remapping.
     */
    std::vector<SwizzlePointerStruct> RequestTable;

    /**
     *  List of all the new pointers.
     */
    std::unordered_map<LONG, SwizzlePointerStruct> PointerTable;
};

extern ViniferaSwizzleManagerClass ViniferaSwizzleManager;
