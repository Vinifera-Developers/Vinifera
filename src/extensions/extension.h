/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EXTENSION.H
 *
 *  @author        CCHyper
 *
 *  @brief         Base class for declaring extended class instances.
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
#include "tibsun_defines.h"
#include "tibsun_globals.h"
#include "swizzle.h"
#include "debughandler.h"

/**
 *  Included here so all extended classes don't have to.
 */
#include "hooker.h"
#include "hooker_macros.h"

#include <unknwn.h> // for IStream.


class NoInitClass;
class WWCRCEngine;



/**
 *  For printing out extension debug info.
 */
#ifndef NDEBUG
#define EXT_DEBUG_SAY(x, ...) DEV_DEBUG_SAY(x, ##__VA_ARGS__)
#define EXT_DEBUG_INFO(x, ...) DEV_DEBUG_INFO(x, ##__VA_ARGS__)
#define EXT_DEBUG_WARNING(x, ...) DEV_DEBUG_WARNING(x, ##__VA_ARGS__)
#define EXT_DEBUG_ERROR(x, ...) DEV_DEBUG_ERROR(x, ##__VA_ARGS__)
#define EXT_DEBUG_FATAL(x, ...) DEV_DEBUG_FATAL(x, ##__VA_ARGS__)
#define EXT_DEBUG_TRACE(x, ...) DEV_DEBUG_TRACE(x, ##__VA_ARGS__)
#else
#define EXT_DEBUG_SAY(x, ...) ((void)0)
#define EXT_DEBUG_INFO(x, ...) ((void)0)
#define EXT_DEBUG_WARNING(x, ...) ((void)0)
#define EXT_DEBUG_ERROR(x, ...) ((void)0)
#define EXT_DEBUG_FATAL(x, ...) ((void)0)
#define EXT_DEBUG_TRACE(x, ...) ((void)0)
#endif


/**
 *  The base class for the extension class which implements save/load.
 */
class ExtensionBase
{
    public:
        ExtensionBase() :
            IsDirty(false)
        {
        }

        ExtensionBase(const NoInitClass &noinit)
        {
        }

        virtual ~ExtensionBase()
        {
        }

        /**
         *  Initializes an object from the stream where it was saved previously.
         */
        virtual HRESULT Load(IStream *pStm);

        /**
         *  Saves an object to the specified stream.
         */
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty);

        /**
         *  Return the raw size of class data for save/load purposes.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual int Size_Of() const = 0;

    private:
        /**
         *  Has the object changed since the last save?
         */
        bool IsDirty;
};


/**
 *  This is the main base class for all class extensions.
 */
template<class T>
class Extension : public ExtensionBase
{
    public:
        Extension(T *this_ptr) :
            ExtensionBase(),
            IsInitialized(false),
            ThisPtr(this_ptr)
        {
        }

        Extension(const NoInitClass &noinit) :
            ExtensionBase(noinit)
        {
        }

        virtual ~Extension()
        {
            IsInitialized = false;
            ThisPtr = nullptr;
        }

        virtual HRESULT Load(IStream *pStm) override;
        virtual HRESULT Save(IStream *pStm, BOOL fClearDirty) override;

        /**
         *  Removes the specified target from any targeting and reference trackers.
         */
        virtual void Detach(TARGET target, bool all = true) = 0;

        /**
         *  Compute a unique crc value for this instance.
         */
        virtual void Compute_CRC(WWCRCEngine &crc) const = 0;

    protected:
        /**
         *  Is this instance extended and valid?
         */
        bool IsInitialized;

    public:
        /**
         *  Pointer to the class we are extending.
         */
        T *ThisPtr;

    private:
        Extension(const Extension &) = delete;
        void operator = (const Extension &) = delete;
};


/**
 *  Loads the object from the stream and requests a new pointer to
 *  the class we extended post-load.
 * 
 *  @author: CCHyper
 */
template<class T>
HRESULT Extension<T>::Load(IStream *pStm)
{
    HRESULT hr = ExtensionBase::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    LONG id;
    hr = pStm->Read(&id, sizeof(id), nullptr);
    if (FAILED(hr)) {
        return E_FAIL;
    }
    
    ULONG size = Size_Of();
    hr = pStm->Read(this, size, nullptr);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    SwizzleManager.Swizzle((void **)&ThisPtr);

    return S_OK;
}


/**
 *  Saves the object to the stream.
 * 
 *  @author: CCHyper
 */
template<class T>
HRESULT Extension<T>::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = ExtensionBase::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    ULONG id = (ULONG)this;
    hr = pStm->Write(&id, sizeof(id), nullptr);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    ULONG size = Size_Of();
    hr = pStm->Write(this, size, nullptr);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    return S_OK;
}
