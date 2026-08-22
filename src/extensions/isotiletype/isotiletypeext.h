/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended IsometricTileTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "isotiletype.h"
#include "objecttypeext.h"
#include "typelist.h"


class DECLSPEC_UUID(UUID_ISOTILE_EXTENSION)
IsometricTileTypeClassExtension final : public ObjectTypeClassExtension
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
        IsometricTileTypeClassExtension(const IsometricTileTypeClass *this_ptr = nullptr);
        IsometricTileTypeClassExtension(const NoInitClass &noinit);
        virtual ~IsometricTileTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual IsometricTileTypeClass *This() const override { return reinterpret_cast<IsometricTileTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const IsometricTileTypeClass *This_Const() const override { return reinterpret_cast<const IsometricTileTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_ISOTILETYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

        /**
         *  Initialises theater control file globals.
         */
        static bool Init(CCINIClass &ini);

    public:
        /**
         *  What set is this tile type part of?
         */
        char TileSetName[64];

        /**
         *  The list of Tiberiums that can grow on this tile type.
         */
        TypeList<TiberiumType> AllowedTiberiums;

        /**
         *  The list of Smudges that can appear on this tile type.
         */
        TypeList<SmudgeType> AllowedSmudges;

        /**
         *  Can veins grow on this tile type?
         */
        bool IsAllowVeins;

        /**
         *  Is this a water tunnel entrance?
         */
        bool IsWaterTunnel;
};
