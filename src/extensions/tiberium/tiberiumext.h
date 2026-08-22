/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TiberiumClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#pragma once

#include "abstracttypeext.h"
#include "tiberium.h"

#include <queue>


class DECLSPEC_UUID(UUID_TIBERIUM_EXTENSION) TiberiumClassExtension final : public AbstractTypeClassExtension
{
public:
    /**
     *  IPersist
     */
    IFACEMETHOD(GetClassID)(CLSID* pClassID);

    /**
     *  IPersistStream
     */
    IFACEMETHOD(Load)(IStream* pStm);
    IFACEMETHOD(Save)(IStream* pStm, BOOL fClearDirty);

public:
    TiberiumClassExtension(const TiberiumClass* this_ptr = nullptr);
    TiberiumClassExtension(const NoInitClass& noinit);
    virtual ~TiberiumClassExtension();

    virtual int Get_Object_Size() const override;
    virtual void Object_CRC(CRCEngine& crc) const override;

    virtual TiberiumClass* This() const override { return reinterpret_cast<TiberiumClass*>(AbstractTypeClassExtension::This()); }
    virtual const TiberiumClass* This_Const() const override { return reinterpret_cast<const TiberiumClass*>(AbstractTypeClassExtension::This_Const()); }
    virtual RTTIType Fetch_RTTI() const override { return RTTI_TIBERIUM; }

    virtual bool Read_INI(CCINIClass& ini) override;

    void Spread_AI(void);
    void Init_Spread(void);
    void Recalc_Spread(void);
    void Clear_Spread(void);
    void Queue_Spread(Cell const& cell);

    void Growth_AI(void);
    void Init_Growth(void);
    void Recalc_Growth(void);
    void Clear_Growth(void);
    void Queue_Growth(Cell const& cell);

    static void Clear_Spread_State(Cell const& cell);

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

    /**
     *  The minimum stage at which this Tiberium can spread.
     */
    int MinSpreadStage;

    /**
     *  The stage at which newly spread overlays spawn.
     */
    int SpreadSpawnStage;

private:
    using QueueItem = std::pair<float, Cell>;
    struct CompareQueueItem {
        bool operator()(const QueueItem& a, const QueueItem& b) const
        {
            return a.first > b.first; // min-heap by float
        }
    };

public:

    /**
     *  Replacement queues for spread and growth mechanics.
     *  The vectors represent whether the cell at that index is scheduled to
     *  grow or spread.
     */
    std::priority_queue<QueueItem, std::vector<QueueItem>, CompareQueueItem> SpreadQueue;
    std::vector<bool> SpreadState;
    std::priority_queue<QueueItem, std::vector<QueueItem>, CompareQueueItem> GrowthQueue;
    std::vector<bool> GrowthState;
};

int Map_Cell_Index(Cell const& cell);
int Map_Cell_Count();
