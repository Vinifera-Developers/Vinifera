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
 *  @contributors  tomsons26
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
#include "vector.h"
#include "tibsun_defines.h"
#include "vinifera_defines.h"
#include <cstdio>


#ifdef VINIFERA_USE_NEW_SWIZZLE_MANAGER

/**
 *  Reimplementation of SwizzleManagerClass.
 * 
 *  #WARNING: Do not add any additional members to the class, we much match
 *            expected class size otherwise everything will break!
 */
class ViniferaSwizzleManagerClass : public ISwizzle
{
    private:
        struct SwizzlePointerStruct
        {
            SwizzlePointerStruct() :
                ID(-1), Pointer(nullptr), File(nullptr), Line(-1), Function(nullptr), Variable(nullptr)
            {}

            SwizzlePointerStruct(LONG id, void *pointer, const char *file = nullptr, const int line = -1, const char *func = nullptr, const char *var = nullptr) :
                ID(id), Pointer(pointer), File(nullptr), Line(line), Function(nullptr), Variable(nullptr)
            {
                if (file != nullptr) {
                    File = new char[strlen(file) + 1];
                    strcpy(File, file);
                }

                if (func != nullptr) {
                    Function = new char[strlen(func) + 1];
                    strcpy(Function, func);
                }

                if (var != nullptr) {
                    Variable = new char[strlen(var) + 1];
                    strcpy(Variable, var);
                }
            }

            ~SwizzlePointerStruct()
            {
                if (File) {
                    delete File;
                    File = nullptr;
                }

                if (Function) {
                    delete Function;
                    Function = nullptr;
                }

                if (Variable) {
                    delete Variable;
                    Variable = nullptr;
                }
            }

            void operator=(const SwizzlePointerStruct &that)
            {
                ID = that.ID;
                Pointer = that.Pointer;

                if (File) {
                    delete File;
                    File = nullptr;
                }

                if (that.File != nullptr) {
                    File = new char[strlen(that.File) + 1];
                    strcpy(File, that.File);
                }

                Line = that.Line;

                if (Function) {
                    delete Function;
                    Function = nullptr;
                }

                if (that.Function != nullptr) {
                    Function = new char[strlen(that.Function) + 1];
                    strcpy(Function, that.Function);
                }

                if (Variable) {
                    delete Variable;
                    Variable = nullptr;
                }

                if (that.Variable != nullptr) {
                    Variable = new char[strlen(that.Variable) + 1];
                    strcpy(Variable, that.Variable);
                }
            }

            bool operator==(const SwizzlePointerStruct &that) const { return ID == that.ID; }
            bool operator!=(const SwizzlePointerStruct &that) const { return ID != that.ID; }
            bool operator<(const SwizzlePointerStruct &that) const { return ID < that.ID; }
            bool operator>(const SwizzlePointerStruct &that) const { return ID > that.ID; }

            /**
             *  The id of the pointer to remap.
             */
            LONG ID;

            /**
             *  The pointer to fixup.
             */
            void *Pointer;
            
            /**
             *  Debugging information.
             */
            char *File;
            /*const*/ int Line;
            char *Function;
            char *Variable;
        };

    public:
        /**
         *  IUnknown
         */
        STDMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj) override;
        STDMETHOD_(ULONG, AddRef)() override;
        STDMETHOD_(ULONG, Release)() override;

        /**
         *  ISwizzle
         */
        STDMETHOD_(LONG, Reset)() override;
        STDMETHOD_(LONG, Swizzle)(void **pointer) override;
        STDMETHOD_(LONG, Fetch_Swizzle_ID)(void *pointer, LONG *id) override;
        STDMETHOD_(LONG, Here_I_Am)(LONG id, void *pointer) override;
        STDMETHOD(Save_Interface)(IStream *stream, IUnknown *pointer) override;
        STDMETHOD(Load_Interface)(IStream *stream, CLSID *riid, void **pointer) override;
        STDMETHOD_(LONG, Get_Save_Size)(LONG *size) override;

        /**
         *  New debug routines.
         */
        STDMETHOD_(LONG, Swizzle_Dbg)(void **pointer, const char *file, const int line, const char *func = nullptr, const char *var = nullptr);
        STDMETHOD_(LONG, Fetch_Swizzle_ID_Dbg)(void *pointer, LONG *id, const char *file, const int line, const char *func = nullptr, const char *var = nullptr);
        STDMETHOD_(LONG, Here_I_Am_Dbg)(LONG id, void *pointer, const char *file, const int line, const char *func = nullptr, const char *var = nullptr);

    public:
        ViniferaSwizzleManagerClass();
        ~ViniferaSwizzleManagerClass();

    private:
        void Sort_Tables();
        void Process_Tables();

    private:
        /**
         *  List of all the pointers that need remapping.
         */
        DynamicVectorClass<SwizzlePointerStruct> RequestTable;

        /**
         *  List of all the new pointers.
         */
        DynamicVectorClass<SwizzlePointerStruct> PointerTable;

    private:
        static int __cdecl ptr_compare_func(const void *ptr1, const void *ptr2);
};

//extern ViniferaSwizzleManagerClass ViniferaSwizzleManager;

#endif
