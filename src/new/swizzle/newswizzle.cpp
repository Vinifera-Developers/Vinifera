/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NEWSWIZZLE.CPP
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
#include "newswizzle.h"

#ifdef VINIFERA_USE_NEW_SWIZZLE_MANAGER

#include "tibsun_globals.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "vinifera_util.h"
#include "fatal.h"
#include <cstdlib>  // for std::qsort


extern void Clear_All_Surfaces();


/**
 *  Instance of the new swizzle manager.
 */
//ViniferaSwizzleManagerClass ViniferaSwizzleManager;


/**
 *  This compare function presumes that its parameters are pointing to SwizzlePointerStruct
 *  and that the first "int" in the struct is the pointer ID number to be used for comparison.
 * 
 *  @return     Returns with the comparison value between the two pointer structs.
 * 
 *  @author:    CCHyper
 */
int __cdecl ViniferaSwizzleManagerClass::ptr_compare_func(const void *ptr1, const void *ptr2)
{
    const SwizzlePointerStruct *p1 = static_cast<const SwizzlePointerStruct *>(ptr1);
    const SwizzlePointerStruct *p2 = static_cast<const SwizzlePointerStruct *>(ptr2);

    if (p1->ID == p2->ID) {
        return 0;
    }
    if (p1->ID < p2->ID) {
        return -1;
    }
    return 1;
}


/**
 *  Retrieves pointers to the supported interfaces on an object.
 * 
 *  @author:    tomsons26, CCHyper
 *  
 *  @param      riid    The interface to this object being queried for.
 * 
 *  @param      ppv     Buffer to fill with obtained interface.
 * 
 *  @return     S_OK if interface obtained; E_NOINTERFACE otherwise.
 */
LONG STDMETHODCALLTYPE ViniferaSwizzleManagerClass::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (ppv == nullptr) {
        return E_POINTER;
    }

    *ppv = nullptr;
    if (riid == __uuidof(IUnknown)) {
        if (reinterpret_cast<IUnknown *>(this) != nullptr) {
            *ppv = reinterpret_cast<IUnknown *>(this);
        }
    } else if (riid == __uuidof(ISwizzle)) {
        if (reinterpret_cast<ISwizzle *>(this) != nullptr) {
            *ppv = reinterpret_cast<ISwizzle *>(this);
        }
    }

    if (*ppv != nullptr) {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


/**
 *  Increments the reference count for an interface pointer to a COM object.
 * 
 *  @author: CCHyper
 */
ULONG STDMETHODCALLTYPE ViniferaSwizzleManagerClass::AddRef()
{
    return S_FALSE;
}


/**
 *  Decrements the reference count for an interface on a COM object.
 * 
 *  @author: CCHyper
 */
ULONG STDMETHODCALLTYPE ViniferaSwizzleManagerClass::Release()
{
    return S_FALSE;
}


/**
 *  Reset swizzler in preparation for load.
 * 
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE ViniferaSwizzleManagerClass::Reset()
{
    Process_Tables();

    return S_OK;
}


/**
 *  Swizzle a pointer after load (requests new pointer).
 * 
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE ViniferaSwizzleManagerClass::Swizzle(void **pointer)
{
    if (pointer == nullptr) {
        return E_POINTER;
    }

    uintptr_t id = uintptr_t(*pointer);
    if (!id) {
        return S_OK;
    }

    SwizzlePointerStruct pair(id, pointer);
    bool added = RequestTable.Add(pair);
    ASSERT(added);

    *pointer = nullptr;

    return (added == true ? S_OK : S_FALSE);
}


/**
 *  Convert pointer to ID number.
 * 
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE ViniferaSwizzleManagerClass::Fetch_Swizzle_ID(void *pointer, LONG *id)
{
    if (pointer == nullptr || id == nullptr) {
        return E_POINTER;
    }

    *id = reinterpret_cast<uintptr_t>(pointer);

    //DEV_DEBUG_INFO("SwizzleManager::Fetch_Swizzle_ID() - ID: 0x%08X.\n", *id);

    return S_OK;
}


/**
 *  Inform (register) swizzler of new object location.
 * 
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE ViniferaSwizzleManagerClass::Here_I_Am(LONG id, void *pointer)
{
    SwizzlePointerStruct pair(id, pointer);
    bool added = PointerTable.Add(pair);
    ASSERT(added);

    return (added == true ? S_OK : S_FALSE);
}


/**
 *  Save interface pointer to stream.
 * 
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE ViniferaSwizzleManagerClass::Save_Interface(IStream *stream, IUnknown *pointer)
{
    return E_NOTIMPL;
}


/**
 *  Loads interface pointer from stream.
 * 
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE ViniferaSwizzleManagerClass::Load_Interface(IStream *stream, CLSID *riid, void **pointer)
{
    return E_NOTIMPL;
}


/**
 *  Fetch bytes required to save interface pointer.
 * 
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE ViniferaSwizzleManagerClass::Get_Save_Size(LONG *psize)
{
    if (psize == nullptr) {
        return E_POINTER;
    }

    *psize = sizeof(LONG);

    return S_OK;
}


/**
 *  The default class constructor.
 * 
 *  @author: CCHyper
 */
ViniferaSwizzleManagerClass::ViniferaSwizzleManagerClass() :
    RequestTable(),
    PointerTable()
{
    RequestTable.Set_Growth_Step(1000);
    PointerTable.Set_Growth_Step(1000);
}


/**
 *  The class deconstructor.
 * 
 *  @author: CCHyper
 */
 ViniferaSwizzleManagerClass::~ViniferaSwizzleManagerClass()
 {
     Process_Tables();
 }


/**
 *  Sort the pointer tables.
 * 
 *  @author: CCHyper
 */
void ViniferaSwizzleManagerClass::Sort_Tables()
{
    if (PointerTable.Count() > 0) {
        std::qsort(&PointerTable[0], PointerTable.Count(), sizeof(SwizzlePointerStruct), ptr_compare_func);
    }
    if (RequestTable.Count() > 0) {
        std::qsort(&RequestTable[0], RequestTable.Count(), sizeof(SwizzlePointerStruct), ptr_compare_func);
    }
}


/**
 *  Process and remap pointers in the tables.
 * 
 *  @author: tomsons26, CCHyper
 */
void ViniferaSwizzleManagerClass::Process_Tables()
{
    if (RequestTable.Count() > 0) {

        Sort_Tables();

        int request_index = 0;
        int request_count = RequestTable.Count();

        int pointer_index = 0;
        int pointer_count = PointerTable.Count();

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
        DEV_DEBUG_INFO("SwizzleManager::Process_Tables() - RequestTable.Count %d.\n", request_count);
        DEV_DEBUG_INFO("SwizzleManager::Process_Tables() - PointerTable.Count %d.\n", pointer_count);
#endif

        while (request_count > 0) {

            int pre_search_id = RequestTable[request_index].ID;
            int ptr_id = PointerTable[pointer_index].ID;
            
#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
            DEV_DEBUG_INFO("SwizzleManager::Process_Tables() - Processing request \"%s\" from %s.\n", RequestTable[request_index].Variable, RequestTable[request_index].Function);
#endif

            if (pre_search_id == ptr_id) {

                /**
                 *  The id's match, remap the pointer.
                 */
                uintptr_t *ptr = (uintptr_t *)RequestTable[request_index].Pointer;
                *ptr = (uintptr_t)PointerTable[pointer_index].Pointer;

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
                DEV_DEBUG_INFO("SwizzleManager::Process_Tables() - Remapped \"%s\" (ID: %08X) to 0x%08X.\n",
                                            RequestTable[request_index].Variable, RequestTable[request_index].ID, (uintptr_t)PointerTable[pointer_index].Pointer);
#endif

                ++request_index;
                --request_count;

                continue;

            }

            /**
             *  Perform a quick search.
             */
            while (pre_search_id > ptr_id) {
                ++pointer_index;
                --pointer_count;
                ptr_id = PointerTable[pointer_index].ID;
            }

            void *old_ptr = RequestTable[request_index].Pointer;
            int new_id = PointerTable[pointer_index].ID;

            /**
             *  #NOTE: Original code was divide by zero to force a crash!
             */
            bool failed = (pre_search_id != new_id);

            /**
             *  Non matching id's means we failed to remap!
             */
            if (failed) {

                DEV_DEBUG_ERROR("SwizzleManager::Process_Tables() - Failed to remap a pointer from the save file!\n");

                /**
                 *  If there is additional debug information attached to this
                 *  pointer, then throw an assertion instead.
                 */
                if (RequestTable[request_index].Variable != nullptr) {

                    SwizzlePointerStruct &req = RequestTable[request_index];

                    /**
                     *  If a variable value has been set, then it will be a 
                     *  pointer from the original game code. Use this as we
                     *  have no line information.
                     */
                    static char buffer[1024];

                    DEV_DEBUG_ERROR("SwizzleManager::Process_Tables() - Request info:\n  File: %s\n  Line: %d\n  Function: %s\n  Variable: %s\n",
                                        req.File ? req.File : "<no-filename-info>",
                                        req.Line,
                                        req.Function ? req.Function : "<no-function-info>",
                                        req.Variable ? req.Variable : "<no-variable-info>");

                    std::snprintf(buffer, sizeof(buffer),
                            "SwizzleManager failed to remap a pointer from the save file!\n\n"
                            "Additional debug information:\n"
                            "  File: %s\n"
                            "  Line: %d\n"
                            "  Function: %s\n"
                            "  Variable: %s\n"
#if defined(TS_CLIENT)
                            "\nThe game will now exit.\n",
#else
                            "\nThe game will now return to the main menu.\n",
#endif
                            req.File ? req.File : "<no-filename-info>",
                            req.Line,
                            req.Function ? req.Function : "<no-function-info>",
                            req.Variable ? req.Variable : "<no-variable-info>");

                    MessageBox(MainWindow, buffer, "Vinifera", MB_OK|MB_ICONEXCLAMATION);

                } else {

#if defined(TS_CLIENT)
                    MessageBox(MainWindow, "SwizzleManager failed to remap a pointer from the save file!\n\nThe game will now exit.", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
#else
                    MessageBox(MainWindow, "SwizzleManager failed to remap a pointer from the save file!\n\nThe game will now return to the main menu.", "Vinifera", MB_OK|MB_ICONEXCLAMATION);
#endif

                }

#if defined(TS_CLIENT)
                //Fatal("SwizzleManager failed to remap a pointer from the save file!\n");
                Emergency_Exit(EXIT_FAILURE);
                exit(EXIT_FAILURE);
#else

                /**
                 *  #BUGFIX:
                 *  Clear all surfaces to remove any blitting artifacts.
                 */
                Clear_All_Surfaces();

                //WWMouseClass::System_Hide_Mouse();
                ShowCursor(FALSE);

                /**
                 *  Return to the main menu. This is abusing the exception return
                 *  address information, which points back to the Select_Game
                 *  call in Main_Game.
                 */
                {
                    static CONTEXT _ctx;
                    ZeroMemory(&_ctx, sizeof(_ctx));

                    RtlCaptureContext(&_ctx);

                    DWORD *ebp = &(_ctx.Ebp);
                    DWORD *esp = &(_ctx.Esp);
                    DWORD *eip = &(_ctx.Eip);
                    *ebp = ExceptionReturnBase;
                    *esp = ExceptionReturnStack;
                    *eip = ExceptionReturnAddress;
                }
#endif

                return; // For clean binary analysis.
            }

        }

        /**
         *  We fixed up all pointers, clear the tables.
         */
        RequestTable.Clear();
        PointerTable.Clear();
    }

}


/**
 *  Swizzle a pointer after load (requests new pointer). [Debug version]
 * 
 *  @author: CCHyper
 */
LONG STDAPICALLTYPE ViniferaSwizzleManagerClass::Swizzle_Dbg(void **pointer, const char *file, const int line, const char *func, const char *var)
{
    if (pointer == nullptr) {
        return E_POINTER;
    }

    uintptr_t id = uintptr_t(*pointer);
    if (!id) {
        return S_OK;
    }

    SwizzlePointerStruct pair(id, pointer, file, line, func, var);
    bool added = RequestTable.Add(pair);
    ASSERT(added);

    *pointer = nullptr;

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
    DEV_DEBUG_INFO("SwizzleManager::Swizzle() - Requested remap for \"%s\" (0x%08X) in %s.\n", var, id, func);
#endif

    return (added == true ? S_OK : S_FALSE);
}


/**
 *  Convert pointer to ID number. [Debug version]
 * 
 *  @author: CCHyper
 */
LONG STDAPICALLTYPE ViniferaSwizzleManagerClass::Fetch_Swizzle_ID_Dbg(void *pointer, LONG *id, const char *file, const int line, const char *func, const char *var)
{
    if (pointer == nullptr || id == nullptr) {
        return E_POINTER;
    }

    *id = reinterpret_cast<uintptr_t>(pointer);

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
    DEV_DEBUG_INFO("SwizzleManager::Fetch_Swizzle_ID() - ID: 0x%08X.\n", *id);
    DEV_DEBUG_INFO("SwizzleManager::Fetch_Swizzle_ID() - File: %s.\n", file);
    if (line != -1) {
        DEV_DEBUG_INFO("SwizzleManager::Fetch_Swizzle_ID() - Line: %d.\n", line);
    }
    if (func) {
        DEV_DEBUG_INFO("SwizzleManager::Fetch_Swizzle_ID() - Func: %s.\n", func);
    }
    if (var) {
        DEV_DEBUG_INFO("SwizzleManager::Fetch_Swizzle_ID() - Var: %s.\n", var);
    }
#endif

    return S_OK;
}


/**
 *  Inform (register) swizzler of new object location. [Debug version]
 * 
 *  @author: CCHyper
 */
LONG STDAPICALLTYPE ViniferaSwizzleManagerClass::Here_I_Am_Dbg(LONG id, void *pointer, const char *file, const int line, const char *func, const char *var)
{
    SwizzlePointerStruct pair(id, pointer, file, line, func, var);
    bool added = PointerTable.Add(pair);
    ASSERT(added);

#ifdef VINIFERA_ENABLE_SWIZZLE_DEBUG_PRINTING
    DEV_DEBUG_INFO("SwizzleManager::Here_I_Am() - PointerTable.Count = %d.\n", PointerTable.Count());
    DEV_DEBUG_INFO("SwizzleManager::Here_I_Am() - Informed swizzler of \"%s\" (0x%08X) in %s.\n", var, id, func);
#endif

    return (added == true ? S_OK : S_FALSE);
}

#endif
