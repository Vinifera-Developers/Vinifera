/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SMUDGETYPEEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended SmudgeTypeClass class.
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

#include "objecttypeext.h"
#include "smudgetype.h"


class DECLSPEC_UUID(UUID_SMUDGETYPE_EXTENSION)
SmudgeTypeClassExtension final : public ObjectTypeClassExtension
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
        SmudgeTypeClassExtension(const SmudgeTypeClass *this_ptr = nullptr);
        SmudgeTypeClassExtension(const NoInitClass &noinit);
        virtual ~SmudgeTypeClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual SmudgeTypeClass *This() const override { return reinterpret_cast<SmudgeTypeClass *>(ObjectTypeClassExtension::This()); }
        virtual const SmudgeTypeClass *This_Const() const override { return reinterpret_cast<const SmudgeTypeClass *>(ObjectTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_SMUDGETYPE; }

        virtual bool Read_INI(CCINIClass &ini) override;

    public:
};
