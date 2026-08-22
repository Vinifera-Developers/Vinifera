/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Base extension class for all type objects.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstractext.h"
#include "stringid.h"


class AbstractTypeClass;
class CCINIClass;


class AbstractTypeClassExtension : public AbstractClassExtension
{
    public:
        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        AbstractTypeClassExtension(const AbstractTypeClass *this_ptr);
        AbstractTypeClassExtension(const NoInitClass &noinit);
        virtual ~AbstractTypeClassExtension();

        virtual const char *Name() const override;
        virtual const char *Full_Name() const override;

        virtual bool Read_INI(CCINIClass &ini);

    protected:
        /**
         *  These are only to be accessed for save and load operations!
         */
        FixedString<24> IniName;
        FixedString<48> GivenName;

        /**
         *  Has this extension already executed Read_INI?
         *  Set this to true at the end of Read_INI of the last extension
         *  in the inheritance hierarchy.
         */
        bool IsInitialized;

    public:

    private:
        AbstractTypeClassExtension(const AbstractTypeClassExtension &) = delete;
        void operator = (const AbstractTypeClassExtension &) = delete;
};
