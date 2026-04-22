/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended AnimClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "anim.h"
#include "animtype.h"
#include "objectext.h"
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
        bool Spawn_Animations(const Coord &coord, const TypeList<AnimTypeClass *> &animlist, const TypeList<int> &countlist, const TypeList<int> &minlist, const TypeList<int> &maxlist, const TypeList<int>& delaylist);

    public:
        /**
         *  Separate StageClass instance for damage dealing, to separate it from visual stages.
         */
        StageClass DamageStage;
};
