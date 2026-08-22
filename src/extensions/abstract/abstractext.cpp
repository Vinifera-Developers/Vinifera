/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Base extension class for all game world objects.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "abstractext.h"

#include "debughandler.h"
#include "extension.h"
#include "tibsun_globals.h"
#include "vinifera_saveload.h"


/**
 *  Class constructor
 *
 *  @author: CCHyper
 */
AbstractClassExtension::AbstractClassExtension(const AbstractClass *this_ptr) :
    ThisPtr(this_ptr)
{
    //ASSERT(ThisPtr != nullptr);      // NULL ThisPtr is valid when performing a Load state operation.
}


/**
 *  Class no-init constructor.
 * 
 *  @author: CCHyper
 */
AbstractClassExtension::AbstractClassExtension(const NoInitClass &noinit)
{
}


/**
 *  Class destructor
 * 
 *  @author: CCHyper
 */
AbstractClassExtension::~AbstractClassExtension()
{
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
    return 1;
}


/**
 *  Decrements the reference count for an interface on a COM object.
 * 
 *  @author: CCHyper
 */
ULONG AbstractClassExtension::Release()
{
    return 1;
}


/**
 *  Determines whether an object has changed since it was last saved to its stream.
 * 
 *  @author: CCHyper
 */
HRESULT AbstractClassExtension::IsDirty()
{
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

    std::string this_name = Extension::Utility::Get_TypeID_Name(this) + "::" + "ThisPtr";

    /**
     *  Register this instance to be available for remapping references to.
     */
    VINIFERA_SWIZZLE_REGISTER_POINTER(id, this, this_name.c_str());

    /**
     *  Read this class's binary blob data directly into this instance.
     */
    hr = pStm->Read(this, Get_Object_Size(), nullptr);
    if (FAILED(hr)) {
        return hr;
    }
    
    VINIFERA_SWIZZLE_REQUEST_POINTER_REMAP(ThisPtr, this_name.c_str());

    return hr;
}


/**
 *  Saves the object to the stream.
 * 
 *  @author: CCHyper, tomsons26
 */
HRESULT AbstractClassExtension::Internal_Save(IStream *pStm, BOOL fClearDirty)
{
    if (!pStm) {
        return E_POINTER;
    }

    std::string this_name = Extension::Utility::Get_TypeID_Name(this) + "::" + "ThisPtr";

    /**
     *  Fetch the save id for this instance.
     */
    LONG id;
    VINIFERA_SWIZZLE_FETCH_SWIZZLE_ID(this, id, this_name.c_str());

    //DEV_DEBUG_INFO("Writing id = 0x{:08X}.\n", id);

    HRESULT hr = pStm->Write(&id, sizeof(id), nullptr);
    if (FAILED(hr)) {
        return hr;
    }
    
    /**
     *  Write this class instance as a binary blob.
     */
    hr = pStm->Write(this, Get_Object_Size(), nullptr);
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
    if (!pcbSize) {
        return E_POINTER;
    }

    pcbSize->LowPart = Get_Object_Size() + sizeof(uint32_t); // Add size of swizzle "id".
    pcbSize->HighPart = 0;

    return S_OK;
}
