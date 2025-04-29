/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUILDINGEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended AnimClass class.
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

#include "objectext.h"
#include "anim.h"
#include "animtype.h"
#include "ttimer.h"
#include "ftimer.h"
#include "typelist.h"


class AnimClass;
class HouseClass;


class DECLSPEC_UUID(UUID_ANIM_EXTENSION)
AnimClassExtension final : public ObjectClassExtension
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
        AnimClassExtension(const AnimClass *this_ptr = nullptr);
        AnimClassExtension(const NoInitClass &noinit);
        virtual ~AnimClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual AnimClass *This() const override { return reinterpret_cast<AnimClass *>(ObjectClassExtension::This()); }
        virtual const AnimClass *This_Const() const override { return reinterpret_cast<const AnimClass *>(ObjectClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_ANIM; }

        bool Start();
        bool Middle();
        bool End();

    private:
        bool Spawn_Animations(const Coordinate &coord, const TypeList<AnimTypeClass *> &animlist, const TypeList<int> &countlist, const TypeList<int> &minlist, const TypeList<int> &maxlist, const TypeList<int>& delaylist);

    public:
        /**
         *  Separate StageClass instance for damage dealing, to separate it from visual stages.
         */
        StageClass DamageStage;
};
