/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TIBERIUMEXT.H
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TiberiumClass class.
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
#include "tiberium.h"
#include <queue>


class DECLSPEC_UUID(UUID_TIBERIUM_EXTENSION)
TiberiumClassExtension final : public AbstractTypeClassExtension
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
        TiberiumClassExtension(const TiberiumClass *this_ptr = nullptr);
        TiberiumClassExtension(const NoInitClass &noinit);
        virtual ~TiberiumClassExtension();

        virtual int Get_Object_Size() const override;
        virtual void Detach(AbstractClass * target, bool all = true) override;
        virtual void Object_CRC(CRCEngine &crc) const override;

        virtual TiberiumClass *This() const override { return reinterpret_cast<TiberiumClass *>(AbstractTypeClassExtension::This()); }
        virtual const TiberiumClass *This_Const() const override { return reinterpret_cast<const TiberiumClass *>(AbstractTypeClassExtension::This_Const()); }
        virtual RTTIType Fetch_RTTI() const override { return RTTI_TIBERIUM; }

        virtual bool Read_INI(CCINIClass &ini) override;

        void Spread_AI(void);
        void Initialize_Spread(void);
        void Recalc_Spread(void);
        void Clear_Spread(void);
        void Queue_Spread(Cell const& cell);

        void Growth_AI(void);
        void Initialize_Growth(void);
        void Recalc_Growth(void);
        void Clear_Growth(void);
        void Queue_Growth(Cell const& cell);

        static void Clear_Tiberium_Spread_State(Cell const& cell);

    public:
        /**
         *  The index of the pip shape to be drawn for this Tiberium.
         */
        int PipIndex;

        /**
         *  The order in which this Tiberium appears when pips are drawn.
         */
        int PipDrawOrder;

        /**
         *  The damage this Tiberium does to infantry.
         */
        int DamageToInfantry;

        using QueueItem = std::pair<float, Cell>;

        struct CompareQueueItem {
            bool operator()(const QueueItem& a, const QueueItem& b) const
            {
                return a.first > b.first; // min-heap by float
            }
        };

        std::priority_queue<QueueItem, std::vector<QueueItem>, CompareQueueItem> SpreadQueue;
        std::vector<bool> SpreadState;
        std::priority_queue<QueueItem, std::vector<QueueItem>, CompareQueueItem> GrowthQueue;
        std::vector<bool> GrowthState;
};
