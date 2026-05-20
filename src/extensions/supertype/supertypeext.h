/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended SuperWeaponTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"
#include "supertype.h"


class BSurface;


class DECLSPEC_UUID(UUID_SUPERWEAPONTYPE_EXTENSION)
SuperWeaponTypeClassExtension final : public AbstractTypeClassExtension
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
        SuperWeaponTypeClassExtension(const SuperWeaponTypeClass *this_ptr = nullptr);
        SuperWeaponTypeClassExtension(const NoInitClass &noinit);
        virtual ~SuperWeaponTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual bool Read_INI(CCINIClass &ini) override;

        virtual SuperWeaponTypeClass *This() const override { return reinterpret_cast<SuperWeaponTypeClass *>(AbstractTypeClassExtension::This()); }
        virtual const SuperWeaponTypeClass *This_Const() const override { return reinterpret_cast<const SuperWeaponTypeClass *>(AbstractTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_SUPERWEAPONTYPE; }

    protected:
        /**
         *  These are only to be accessed for save and load operations!
         */
        FixedString<24> SidebarImage;

    public:
        /**
         *  When this super weapon is active, does its recharge timer display
         *  on the tactical view?
         */
        bool IsShowTimer;

        /**
         *  Pointer to the cameo image surface.
         */
        BSurface *CameoImageSurface;

        /**
         *  Action type used for the cursor when the SW is out of range to fire.
         */
        ActionType ActionOutOfRange;

        /**
         *  Vox to speak when a missile SW is launched.
         */
        VoxType VoxMissileLaunched;

        /**
         *  Description for the extended sidebar tooltip.
         */
        char Description[200];
};
