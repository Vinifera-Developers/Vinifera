/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TerrainTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "objecttypeext.h"
#include "terraintype.h"


class DECLSPEC_UUID(UUID_TERRAINTYPE_EXTENSION)
TerrainTypeClassExtension final : public ObjectTypeClassExtension
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
        TerrainTypeClassExtension(const TerrainTypeClass *this_ptr = nullptr);
        TerrainTypeClassExtension(const NoInitClass &noinit);
        virtual ~TerrainTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual TerrainTypeClass *This() const override { return reinterpret_cast<TerrainTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const TerrainTypeClass *This_Const() const override { return reinterpret_cast<const TerrainTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_TERRAINTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
        /**
         *  Does this terrain object emit light?
         */
        bool IsLightEnabled;

        /**
         *  This terrain object radiates this amount of light.
         */
        int LightVisibility;

        /**
         *  The distance (in leptons) that this light is visible from.
         */
        int LightIntensity;

        /**
         *  The red tint of this terrain objects light.
         */
        int LightRedTint;

        /**
         *  The green tint of this terrain objects light.
         */
        int LightGreenTint;

        /**
         *  The blue tint of this terrain objects light.
         */
        int LightBlueTint;

        /**
         *  If SpawnsTiberium=yes, the max range in which Tiberium will be spawned.
         */
        int TiberiumSpawnRange;

        /**
         *  If SpawnsTiberium=yes, the growth stage at which the Tiberium will be spawned (min, max).
         */
        Point2D TiberiumSpawnStage;

        /**
         *  If SpawnsTiberium=yes, amount by which the growth stage will decrease for ever ring after 1.
         */
        float TiberiumSpawnStageFalloff;

        /**
         *  If SpawnsTiberium=yes, the number of Tiberium overlays that will be spawned (min, max).
         */
        Point2D TiberiumSpawnCount;

        /**
         *  If SpawnsTiberium=yes, should TIberium be spawned randomly in the range instead of spreading from the center.
         */
        bool IsTiberiumScatterSpawn;
};
