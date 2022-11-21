/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ABSTRACTEXT.H
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

#include "always.h"
#include "tibsun_defines.h"
#include "vinifera_defines.h"
#include "noinit.h"

#include <unknwn.h> // for IStream


class AbstractClass;
class WWCRCEngine;


/**
 *  This class is the base class for all game objects we can extend that have an
 *  existence on the battlefield.
 */
class AbstractClassExtension : public IPersistStream
{
    public:
        /**
         *  IUnknown
         */
        IFACEMETHOD(QueryInterface)(REFIID riid, LPVOID *ppvObj);
        IFACEMETHOD_(ULONG, AddRef)();
        IFACEMETHOD_(ULONG, Release)();

        /**
         *  IPersistStream
         */
        IFACEMETHOD(IsDirty)();
        IFACEMETHOD_(LONG, GetSizeMax)(ULARGE_INTEGER *pcbSize);

    protected:
        /**
         *  #NOTE:
         *  These two are used as the real base Load/Save, but are not virtual.
         *  Ensure you call these directly if your class derives from Abstract and
         *  do not call AbstractClassExtension::Save or Load directly as they
         *  are pure virtual and must be overridden by the final class!
         */
        HRESULT STDMETHODCALLTYPE Internal_Load(IStream *pStm);
        HRESULT STDMETHODCALLTYPE Internal_Save(IStream *pStm, BOOL fClearDirty);

    public:
        AbstractClassExtension(const AbstractClass *this_ptr);
        AbstractClassExtension(const NoInitClass &noinit);
        virtual ~AbstractClassExtension();

        /**
         *  Return the raw size of class data for save/load purposes.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual int Size_Of() const = 0;

        /**
         *  Removes the specified target from any targeting and reference trackers.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual void Detach(TARGET target, bool all = true) = 0;

        /**
         *  Compute a unique crc value for this instance.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual void Compute_CRC(WWCRCEngine &crc) const = 0;

        /**
         *  Access to the class instance we extend.
         */
        virtual AbstractClass *This() const { return const_cast<AbstractClass *>(ThisPtr); }
        virtual const AbstractClass *This_Const() const { return ThisPtr; }

        /**
         *  Access to the extended class instance.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual RTTIType What_Am_I() const = 0; // { return RTTI_ABSTRACT; }

        /**
         *  Returns the name of this object type.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual const char *Name() const = 0;

        /**
         *  Returns the full name of this object type.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual const char *Full_Name() const = 0;

    private:
        /**
         *  Pointer to the class we are extending. This provides us with a way of
         *  quickly referencing the base class without doing a look-up each time.
         */
        const AbstractClass *ThisPtr;

    private:
        AbstractClassExtension(const AbstractClassExtension &) = delete;
        void operator = (const AbstractClassExtension &) = delete;

    public:
};
