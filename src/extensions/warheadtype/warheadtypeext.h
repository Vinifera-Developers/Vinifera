/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended WarheadTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"
#include "warheadtype.h"


class DECLSPEC_UUID(UUID_WARHEADTYPE_EXTENSION)
WarheadTypeClassExtension final : public AbstractTypeClassExtension
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
        WarheadTypeClassExtension(const WarheadTypeClass *this_ptr = nullptr);
        WarheadTypeClassExtension(const NoInitClass &noinit);
        virtual ~WarheadTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual WarheadTypeClass *This() const override { return reinterpret_cast<WarheadTypeClass *>(AbstractTypeClassExtension::This()); }
        virtual const WarheadTypeClass *This_Const() const override { return reinterpret_cast<const WarheadTypeClass *>(AbstractTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_WARHEADTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

        float Fetch_Type_Modifier(RTTIType type) const;

    public:
        /**
         *  Does this warhead instantly destroy walls regardless of the warhead damage value?
         */
        bool IsWallAbsoluteDestroyer;

        /**
         *  Can this warhead damage friendly units?
         */
        bool IsAffectsAllies;

        /**
         *  This is used to override the size of the combat light flash at the point of impact.
         */
        double CombatLightSize;

        /**
         *  These values are used to shake the screen when the projectile impacts.
         */
        unsigned int ShakePixelYHi;
        unsigned int ShakePixelYLo;
        unsigned int ShakePixelXHi;
        unsigned int ShakePixelXLo;

        /**
         *  The minimum damage something using this warhead can deal. Negative means to use Rule->MinDamage.
         */
        int MinDamage;

        /**
         *  The maximum range, in cells, at which a weapon using this warhead will damage objects.
         */
        float CellSpread;

        /**
         *  The fraction of the damage that is applied at this weapon's max range.
         */
        float PercentAtMax;

        /**
         *  The chance that a cell affected by this warhead will spawn a random scorch.
         */
        float ScorchChance;
        float ScorchPercentAtMax;

        /**
         *  The chance that a cell affected by this warhead will spawn a random crater.
         */
        float CraterChance;
        float CraterPercentAtMax;

        /**
         *  The chance that a cell affected by this warhead will spawn a random anim from the list.
         */
        float CellAnimChance;
        float CellAnimPercentAtMax;

        /**
         *  The list of anims to pick from when CellAnimChance is triggered.
         */
        TypeList<AnimTypeClass*> CellAnim;

        /**
         *  Damage multipliers against various object types.
         */
        float InfantryModifier;
        float VehicleModifier;
        float AircraftModifier;
        float BuildingModifier;
        float TerrainModifier;

        /**
         *  Should this warhead always damage things in air, regardless of the explosion height?
         */
        bool IsVolumetric;

        /**
         *  Should explosions using this warhead always take place at the center of the cell?
         */
        bool IsSnapToCellCenter;
};
