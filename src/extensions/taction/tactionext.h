/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTIONEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TActionClass class.
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

#include "abstractext.h"
#include "taction.h"
#include "tibsun_defines.h"


class TActionClass;
class HouseClass;
class ObjectClass;
class TriggerClass;


class DECLSPEC_UUID(UUID_ACTION_EXTENSION)
TActionClassExtension final : public AbstractClassExtension
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
        TActionClassExtension(const TActionClass *this_ptr = nullptr);
        TActionClassExtension(const NoInitClass &noinit);
        virtual ~TActionClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual const char *Name() const override { return reinterpret_cast<const AbstractClassExtension *>(This())->Name(); }
        virtual const char *Full_Name() const override { return reinterpret_cast<const AbstractClassExtension *>(This())->Full_Name(); }

        virtual TActionClass *This() const override { return reinterpret_cast<TActionClass *>(AbstractClassExtension::This()); }
        virtual const TActionClass *This_Const() const override { return reinterpret_cast<const TActionClass *>(AbstractClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_ACTION; }

    public:
        static bool Execute(TActionClass *taction, HouseClass *house, ObjectClass *object, TriggerClass *trigger, Cell *cell);

        static const char *Action_Name(int action);
        static const char *Action_Description(int action);

    protected:
        
};
