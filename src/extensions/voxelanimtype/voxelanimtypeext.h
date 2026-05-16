/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended VoxelAnimTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "objecttypeext.h"
#include "voxelanimtype.h"


class DECLSPEC_UUID(UUID_VOXELANIMTYPE_EXTENSION)
VoxelAnimTypeClassExtension final : public ObjectTypeClassExtension
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
        VoxelAnimTypeClassExtension(const VoxelAnimTypeClass *this_ptr = nullptr);
        VoxelAnimTypeClassExtension(const NoInitClass &noinit);
        virtual ~VoxelAnimTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual VoxelAnimTypeClass *This() const override { return reinterpret_cast<VoxelAnimTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const VoxelAnimTypeClass *This_Const() const override { return reinterpret_cast<const VoxelAnimTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_VOXELANIMTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
        /**
         *  The sound effect to play when this voxel anim has finished.
         */
        VocType StopSound;
};
