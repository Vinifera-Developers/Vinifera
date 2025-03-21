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

        /**
         *  This function replicates part of AnimClass::AI and returns whether the next time
         *  AnimClass::AI is executed, the animation will be deleted.
         *
         *  @note This function doesn't take into account the possibility of the animation destroying
         *  the object it's attached to and thus deleting itself that way.
         *
         *  @author: ZivDero
         */
        static bool Is_About_To_End(AnimClass* this_ptr)
        {
            /**
             *  Check if the anim is about to change its frame.
             */
            if (!this_ptr->IsDisabled && this_ptr->About_To_Change())
            {
                const AnimTypeClass * const animtype = this_ptr->Class;

                int stage = this_ptr->Fetch_Stage();
                int loops = this_ptr->Loops;

                /**
                 *  Simulate the anim changing its frame.
                 */
                stage += this_ptr->Fetch_Step();

                /**
                 *  If the anim is ping-ponging, it's not ensing right now.
                 */
                if (animtype->IsPingPong) {
                    if ((loops <= 1 && (stage >= animtype->Stages || stage == 0)) || (loops > 1 && (stage >= animtype->LoopEnd - animtype->Start || stage == animtype->Start))) {
                        return false;
                    }
                }

                /**
                 *  Check to see if the last frame has been displayed. If so, then the
                 *  animation either ends or loops.
                 */
                if ((loops <= 1 && stage >= animtype->Stages) || (loops > 1 && stage >= animtype->LoopEnd - animtype->Start) || (animtype->IsReverse && stage <= animtype->Start)) {
                    if (loops && loops != UCHAR_MAX) loops--;

                    /**
                     *  if the anim doesn't loop anymore, and has nothing to chain to, return that it's about to end.
                     */
                    if (loops == 0 && animtype->ChainTo == nullptr) {
                        return true;
                    }
                }
            }

            return false;
        }

    private:
        bool Spawn_Animations(const Coordinate &coord, const TypeList<AnimTypeClass *> &animlist, const TypeList<int> &countlist, const TypeList<int> &minlist, const TypeList<int> &maxlist, const TypeList<int>& delaylist);

    public:
};
