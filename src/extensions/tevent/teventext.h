/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TEVENTEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TEventClass class.
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
#include "tevent.h"
#include "tibsun_defines.h"


class TEventClass;
class HouseClass;
class ObjectClass;
struct TDEventClass;
class TechnoClass;


class DECLSPEC_UUID(UUID_EVENT_EXTENSION)
TEventClassExtension final : public AbstractClassExtension
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
        TEventClassExtension(const TEventClass *this_ptr = nullptr);
        TEventClassExtension(const NoInitClass &noinit);
        virtual ~TEventClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual const char* Name() const override { return reinterpret_cast<const AbstractClassExtension *>(This())->Name(); }
        virtual const char* Full_Name() const override { return reinterpret_cast<const AbstractClassExtension *>(This())->Full_Name(); }

        virtual TEventClass *This() const override { return reinterpret_cast<TEventClass *>(AbstractClassExtension::This()); }
        virtual const TEventClass *This_Const() const override { return reinterpret_cast<const TEventClass *>(AbstractClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_EVENT; }

    public:
        static bool Satisfied(TEventClass *this_ptr, TEventType event, HouseClass *house, const ObjectClass *object, TDEventClass &td, bool &tripped, TechnoClass *source = nullptr);

        static const char *Event_Name(int event);
        static const char *Event_Description(int event);

    protected:
        
};
