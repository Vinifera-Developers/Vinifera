/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SuperClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"
#include "super.h"
#include "supertype.h"


class HouseClass;


class DECLSPEC_UUID(UUID_SUPERWEAPON_EXTENSION)
SuperClassExtension final : public AbstractClassExtension
{
    public:
        /**
         *  IPersist
         */
        IFACEMETHOD(GetClassID)(CLSID *pClassID);

        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        SuperClassExtension(const SuperClass *this_ptr = nullptr);
        SuperClassExtension(const NoInitClass &noinit);
        virtual ~SuperClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual const char *Name() const override { return reinterpret_cast<const SuperClass *>(This())->Class->Name(); }
        virtual const char *Full_Name() const override { return reinterpret_cast<const SuperClass *>(This())->Class->Full_Name(); }

        virtual SuperClass *This() const override { return reinterpret_cast<SuperClass *>(AbstractClassExtension::This()); }
        virtual const SuperClass *This_Const() const override { return reinterpret_cast<const SuperClass *>(AbstractClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_SUPERWEAPON; }

    public:
        /**
         *  The time at which the flash mode should return to normal.
         */
        unsigned long FlashTimeEnd;

        /**
         *  The current flash state of the timer printed on the tactical view.
         */
        bool TimerFlashState;
};
