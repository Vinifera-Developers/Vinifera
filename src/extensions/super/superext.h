/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SUPEREXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended SuperClass class.
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

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual const char *Name() const override { return reinterpret_cast<const SuperClass *>(This())->Class->Name(); }
        virtual const char *Full_Name() const override { return reinterpret_cast<const SuperClass *>(This())->Class->Full_Name(); }

        virtual SuperClass *This() const override { return reinterpret_cast<SuperClass *>(AbstractClassExtension::This()); }
        virtual const SuperClass *This_Const() const override { return reinterpret_cast<const SuperClass *>(AbstractClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_SUPERWEAPON; }

        const char* Ready_String() const;

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
