/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended ThemeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "extension.h"
#include "theme.h"


class CCINIClass;


class ThemeControlExtension final : public GlobalExtensionClass<ThemeClass::ThemeControl>
{
    public:
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        ThemeControlExtension(const ThemeClass::ThemeControl *this_ptr);
        ThemeControlExtension(const NoInitClass &noinit);
        virtual ~ThemeControlExtension();

        /**
         *  ThemeControl extension does not require these to be used, but we
         *  implement them for completeness.
         */
        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual const char *Name() const override { return "ThemeControl"; }
        virtual const char *Full_Name() const override { return "ThemeControl"; }

        bool Read_INI(CCINIClass &ini);

    public:
        /**
         *  The addon required to be active for this theme to be available.
         */
        AddonType RequiredAddon;
};
