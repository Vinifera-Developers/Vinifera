/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TerrainClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "objectext.h"
#include "terrain.h"


class LightSourceClass;


class DECLSPEC_UUID(UUID_TERRAIN_EXTENSION)
TerrainClassExtension final : public ObjectClassExtension
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
        TerrainClassExtension(const TerrainClass *this_ptr = nullptr);
        TerrainClassExtension(const NoInitClass &noinit);
        virtual ~TerrainClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual TerrainClass *This() const override { return reinterpret_cast<TerrainClass *>(ObjectClassExtension::This()); }
        virtual const TerrainClass *This_Const() const override { return reinterpret_cast<const TerrainClass *>(ObjectClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_TERRAIN; }

        void Spread_Tiberium() const;

    public:
        /**
         *  The light source instance for this terrain object.
         */
        LightSourceClass *LightSource;
};
