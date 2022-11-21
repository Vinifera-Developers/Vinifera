/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ABSTRACTEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Base extension class for all game world objects.
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

#include "abstractext.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "swizzle.h"
#include "vinifera_saveload.h"
#include "extension.h"
#include "debughandler.h"
#include "asserthandler.h"


/**
 *  Class constructor
 * 
 *  @author: CCHyper
 */
AbstractClassExtension::AbstractClassExtension(const AbstractClass *this_ptr) :
    ThisPtr(this_ptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("AbstractClassExtension::AbstractClassExtension - 0x%08X\n", (uintptr_t)(ThisPtr));
    //ASSERT(ThisPtr != nullptr);      // NULL ThisPtr is valid when performing a Load state operation.
}


/**
 *  Class no-init constructor.
 * 
 *  @author: CCHyper
 */
AbstractClassExtension::AbstractClassExtension(const NoInitClass &noinit)
{
    //EXT_DEBUG_TRACE("AbstractClassExtension::AbstractClassExtension(NoInitClass) - 0x%08X\n", (uintptr_t)(ThisPtr));
}


/**
 *  Class destructor
 * 
 *  @author: CCHyper
 */
AbstractClassExtension::~AbstractClassExtension()
{
    //EXT_DEBUG_TRACE("AbstractClassExtension::~AbstractClassExtension - 0x%08X\n", (uintptr_t)(ThisPtr));

    ThisPtr = nullptr;
}


/**
 *  Retrieves pointers to the supported interfaces on an object.
 *  
 *  @author: CCHyper, tomsons26
 */
LONG AbstractClassExtension::QueryInterface(REFIID riid, LPVOID *ppv)
{
    /**
     *  Always set out parameter to NULL, validating it first.
     */
    if (ppv == nullptr) {
        return E_POINTER;
    }
    *ppv = nullptr;

    if (riid == __uuidof(IUnknown)) {
        *ppv = reinterpret_cast<IUnknown *>(this);
    } 

    if (riid == __uuidof(IStream)) {
        *ppv = reinterpret_cast<IStream *>(this);
    } 

    if (riid == __uuidof(IPersistStream)) {
        *ppv = static_cast<IPersistStream *>(this);
    }

    if (*ppv == nullptr) {
        return E_NOINTERFACE;
    }

    /**
     *  Increment the reference count and return the pointer.
     */
    reinterpret_cast<IUnknown *>(*ppv)->AddRef();

    return S_OK;
}


/**
 *  Increments the reference count for an interface pointer to a COM object.
 * 
 *  @author: CCHyper
 */
ULONG AbstractClassExtension::AddRef()
{
    //EXT_DEBUG_TRACE("AbstractClassExtension::AddRef - 0x%08X\n", (uintptr_t)(ThisPtr));

    return 1;
}


/**
 *  Decrements the reference count for an interface on a COM object.
 * 
 *  @author: CCHyper
 */
ULONG AbstractClassExtension::Release()
{
    //EXT_DEBUG_TRACE("AbstractClassExtension::Release - 0x%08X\n", (uintptr_t)(ThisPtr));

    return 1;
}


/**
 *  Determines whether an object has changed since it was last saved to its stream.
 * 
 *  @author: CCHyper
 */
HRESULT AbstractClassExtension::IsDirty()
{
    //EXT_DEBUG_TRACE("AbstractClassExtension::IsDirty - 0x%08X\n", (uintptr_t)(ThisPtr));

    return S_OK;
}


/**
 *  Loads the object from the stream and requests a new pointer to
 *  the class we extended post-load.
 * 
 *  @author: CCHyper, tomsons26
 */
HRESULT AbstractClassExtension::Internal_Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("AbstractClassExtension::Internal_Load - 0x%08X\n", (uintptr_t)(ThisPtr));

    if (!pStm) {
        return E_POINTER;
    }

    /**
     *  Load the unique id for this class.
     */
    LONG id = 0;
    HRESULT hr = pStm->Read(&id, sizeof(LONG), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    Wstring this_name = Wstring(Extension::Utility::Get_TypeID_Name(this).c_str()) + ":" + Wstring("ThisPtr");

    /**
     *  Register this instance to be available for remapping references to.
     */
    VINIFERA_SWIZZLE_REGISTER_POINTER(id, this, this_name.Peek_Buffer());

    /**
     *  Read this classes binary blob data directly into this instance.
     */
    hr = pStm->Read(this, Size_Of(), nullptr);
    if (FAILED(hr)) {
        return hr;
    }
    
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(ThisPtr, this_name.Peek_Buffer());

    return hr;
}


/**
 *  Saves the object to the stream.
 * 
 *  @author: CCHyper, tomsons26
 */
HRESULT AbstractClassExtension::Internal_Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("AbstractClassExtension::Internal_Save - 0x%08X\n", (uintptr_t)(ThisPtr));

    if (!pStm) {
        return E_POINTER;
    }

    Wstring this_name = Wstring(Extension::Utility::Get_TypeID_Name(this).c_str()) + ":" + Wstring("ThisPtr");

    /**
     *  Fetch the save id for this instance.
     */
    LONG id;
    VINIFERA_SWIZZLE_FETCH_SWIZZLE_ID(this, id, this_name.Peek_Buffer());

    //DEV_DEBUG_INFO("Writing id = 0x%08X.\n", id);

    HRESULT hr = pStm->Write(&id, sizeof(id), nullptr);
    if (FAILED(hr)) {
        return hr;
    }
    
    /**
     *  Write this class instance as a binary blob.
     */
    hr = pStm->Write(this, Size_Of(), nullptr);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Retrieves the size of the stream needed to save the object.
 * 
 *  @author: CCHyper, tomsons26
 */
LONG AbstractClassExtension::GetSizeMax(ULARGE_INTEGER *pcbSize)
{
    //EXT_DEBUG_TRACE("AbstractClassExtension::GetSizeMax - 0x%08X\n", (uintptr_t)(ThisPtr));

    if (!pcbSize) {
        return E_POINTER;
    }

    pcbSize->LowPart = Size_Of() + sizeof(uint32_t); // Add size of swizzle "id".
    pcbSize->HighPart = 0;

    return S_OK;
}
