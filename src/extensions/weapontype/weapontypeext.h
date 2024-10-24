/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WEAPONTYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended WeaponTypeClass class.
 *
 *  @license       Vinifera is free software: you can redistribute it and/or
 *                 modify it under the terms of the GNU General Public License
 *                 as published by the Free Software Foundation, either version
 *                 3 of the License, or (at your option) any later version.
 *
 *                 Vinifera is distributed in the hope that it will be
 *                 useful, but WITHOUT ANY WARRANTY; without even the implied
 *                 warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *                 PURPOSE. See the GNU General Public License for more details.
 *
 *                 You should have received a copy of the GNU General Public
 *                 License along with this program.
 *                 If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/
#pragma once

#include "abstracttypeext.h"
#include "weapontype.h"


class DECLSPEC_UUID(UUID_WEAPONTYPE_EXTENSION)
WeaponTypeClassExtension final : public AbstractTypeClassExtension
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
        WeaponTypeClassExtension(const WeaponTypeClass *this_ptr = nullptr);
        WeaponTypeClassExtension(const NoInitClass &noinit);
        virtual ~WeaponTypeClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual WeaponTypeClass *This() const override { return reinterpret_cast<WeaponTypeClass *>(AbstractTypeClassExtension::This()); }
        virtual const WeaponTypeClass *This_Const() const override { return reinterpret_cast<const WeaponTypeClass *>(AbstractTypeClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_WEAPONTYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
        /**
         *  Does the firing unit destroy itself when this weapon is fired?
         */
        bool IsSuicide;

        /**
         *  Does the firing unit get explictly deleted when this weapon is fired?
         */
        bool IsDeleteOnSuicide;

        /**
         *  Is this a electric bolt weapon (Uses custom drawing)?
         */
        bool IsElectricBolt;

        /**
         *  Electric bolts have 3 lines per section, these controls their colors.
         */
        RGBStruct ElectricBoltColor1;
        RGBStruct ElectricBoltColor2;
        RGBStruct ElectricBoltColor3;

        /**
         *  How many segment blocks should the electric bolt be made up from.
         */
        int ElectricBoltSegmentCount;

        /**
         *  The lifetime [in game frames] of the bolt graphic.
         */
        int ElectricBoltLifetime;

        /**
         *  How many draw iterations should the system perform?
         */
        int ElectricBoltIterationCount;

        /**
         *  The maximum deviation from a straight line the electric bolts can be.
         */
        float ElectricBoltDeviation;

        /**
         *  Does this weapon spawn aircraft when fired?
         */
        bool IsSpawner;

        /**
         *  Should the firer of this weapon be revealed when firing?
         */
        bool IsRevealOnFire;

        /**
         *  The action type used for this weapon's attack cursor.
         */
        ActionType AttackCursor;

        /**
         *  The action type used for this weapon's attack cursor when hovering over shroud.
         */
        ActionType StayAttackCursor;

        /**
         *  Particle systems to display.
         */
        //ParticleSystemClass *ElectricBoltSourceBoltParticleSys;
        //ParticleSystemClass *ElectricBoltTargetBoltParticleSys;
};
