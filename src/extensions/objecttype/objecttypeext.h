/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended ObjectTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"
#include "objecttype.h"


class ObjectTypeClassExtension : public AbstractTypeClassExtension
{
    public:
        /**
         *  IPersistStream
         */
        IFACEMETHOD(Load)(IStream *pStm);
        IFACEMETHOD(Save)(IStream *pStm, BOOL fClearDirty);

    public:
        ObjectTypeClassExtension(const ObjectTypeClass *this_ptr);
        ObjectTypeClassExtension(const NoInitClass &noinit);
        virtual ~ObjectTypeClassExtension();

        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual const char *Name() const override { return reinterpret_cast<const ObjectTypeClass *>(This())->Name(); }
        virtual const char *Full_Name() const override { return reinterpret_cast<const ObjectTypeClass *>(This())->Full_Name(); }

        virtual ObjectTypeClass *This() const override { return reinterpret_cast<ObjectTypeClass *>(AbstractTypeClassExtension::This()); }
        virtual const ObjectTypeClass *This_Const() const override { return reinterpret_cast<const ObjectTypeClass *>(AbstractTypeClassExtension::This_Const()); }

        virtual const char *Graphic_Name() const { return reinterpret_cast<const ObjectTypeClass *>(This())->Graphic_Name(); }
        virtual const char *Alpha_Graphic_Name() const { return reinterpret_cast<const ObjectTypeClass *>(This())->Alpha_Graphic_Name(); }

        virtual bool Read_INI(CCINIClass &ini) override;

        void Fetch_Voxel_Image(const char* graphic_name);
        BuildingClass* Who_Can_Build_Me(bool intheory, bool needsnopower, bool legal, HouseClass* house, bool to_exit = false) const;

    protected:
        /**
         *  These are only to be accessed for save and load operations!
         */
        FixedString<24> GraphicName;
        FixedString<24> AlphaGraphicName;

    public:

        /**
         *  Should the object use a different voxel model when it has no spawn?
         */
        bool NoSpawnAlt;

        /**
         *  The voxel model to use when the object has no spawn, and its cache.
         */
        VoxelObject NoSpawnVoxel;
        VoxelIndexClass NoSpawnVoxelIndex;

        /**
         *  Should the object use a different voxel model when it is in water?
         */
        bool WaterAlt;

        /**
         *  The voxel model to use when the object is in water, and its cache.
         */
        VoxelObject WaterVoxel;
        VoxelIndexClass WaterVoxelIndex;

        /**
         *  The ambient sound effect type to play while this object is active.
         */
        VocType AmbientSound;
};
