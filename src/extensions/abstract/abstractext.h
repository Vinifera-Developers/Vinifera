/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Base extension class for all game world objects.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "tibsun_defines.h"
#include "vinifera_defines.h"


class AbstractClass;
class CRCEngine;


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
        virtual int Get_Object_Size() const = 0;

        /**
         *  Compute a unique crc value for this instance.
         *  
         *  @note: This must be overridden by the extended class!
         */
        virtual void Object_CRC(CRCEngine &crc) const = 0;

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
        virtual RTTIType Fetch_RTTI() const { return RTTI_ABSTRACT; }

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
