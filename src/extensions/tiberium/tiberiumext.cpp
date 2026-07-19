/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TiberiumClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "tiberiumext.h"

#include "ccini.h"
#include "cellext.h"
#include "extension.h"
#include "findmake.h"
#include "mouse.h"
#include "overlaytype.h"
#include "tiberium.h"
#include "tibsun_globals.h"
#include "vinifera_saveload.h"


/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
TiberiumClassExtension::TiberiumClassExtension(const TiberiumClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    MinSpreadStage(5),
    SpreadSpawnStage(5)
{
    if (this_ptr)
    {
        /**
         *  By default Tiberium 0 gets green pips, and the rest get blue.
         *  Blue Tiberium is also drawn first
         */
        if (this_ptr->Fetch_Heap_ID() == 0)
        {
            PipIndex = 1;
            PipDrawOrder = 1;
        }
        else
        {
            PipIndex = 5;
            PipDrawOrder = 0;
        }
    }

    TiberiumExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TiberiumClassExtension::TiberiumClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit)
{
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TiberiumClassExtension::~TiberiumClassExtension()
{
    TiberiumExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT TiberiumClassExtension::GetClassID(CLSID *lpClassID)
{
    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT TiberiumClassExtension::Load(IStream *pStm)
{
    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TiberiumClassExtension(NoInitClass());
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TiberiumClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    HRESULT hr = AbstractTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *  
 *  @author: CCHyper
 */
int TiberiumClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TiberiumClassExtension::Object_CRC(CRCEngine &crc) const
{
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool TiberiumClassExtension::Read_INI(CCINIClass &ini)
{
    const char* ini_name = Name();

    if (!IsInitialized) {
        This()->FrameCount = 12;
        This()->Variety = 12;
        DamageToInfantry = std::max(1, This()->Power / 10);
    }

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    This()->Overlay = TGet_Class(ini, ini_name, "Overlay", This()->Overlay);

    const bool useSlopes = ini.Get_Bool(ini_name, "UseSlopes", This()->RampVariety > 0);
    This()->RampVariety = useSlopes ? 8 : 0;

    This()->Variety = ini.Get_Int(ini_name, "Variety", This()->Variety);
    This()->Variety = std::max(1, This()->Variety); // at least one overlay, please

    PipIndex = ini.Get_Int(ini_name, "PipIndex", PipIndex);
    PipDrawOrder = ini.Get_Int(ini_name, "PipDrawOrder", PipDrawOrder);

    DamageToInfantry = ini.Get_Int(ini_name, "DamageToInfantry", DamageToInfantry);

    MinSpreadStage = ini.Get_Int(ini_name, "MinSpreadStage", MinSpreadStage);
    SpreadSpawnStage = ini.Get_Int(ini_name, "SpreadSpawnStage", SpreadSpawnStage);

    IsInitialized = true;
    
    return true;
}


/**
 *  Computes a unique index for this map cell.
 *
 *  @author: ZivDero, tomsons26
 */
int Map_Cell_Index(Cell const& cell)
{
    return ((cell.X - cell.Y + Map.PlayRect.Width - 1) >> 1) + Map.PlayRect.Width * (cell.X - Map.PlayRect.Width + cell.Y - 1);
}


/**
 *  Computes the number of cells on the map.
 *
 *  @author: ZivDero, tomsons26
 */
int Map_Cell_Count()
{
    return (2 * Map.PlayRect.Width) * (Map.PlayRect.Height + 4);
}


/**
 *  Tiberium spread logic main function.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Spread_AI()
{
    if (!SpreadQueue.empty() && This()->SpreadPercentage > 0.00001) {

        /**
         *  The number of spreads depends on the queue size and Tiberium properties,
         *  capped for performance (and a minimum so that it never completely halts).
         */
        int count = std::clamp(static_cast<int>(SpreadQueue.size() * This()->SpreadPercentage), 5, 300);
        count = Random_Pick(1, count);

        int index = 0;
        while (index < count && !SpreadQueue.empty()) {
            auto node = SpreadQueue.top();
            SpreadQueue.pop();

            Cell cell = node.second;
            CellClass& cellptr = Map[cell];

            /**
             *  If we can't spread, skip without increasing the counter. This fixes a bug in vanilla
             *  where sometimes the queue would contain a bunch of entries that all failed,
             *  and it appeared as if Tiberium broke.
             */
            if (!cellptr.Can_Tiberium_Spread()) {
                SpreadState[Map_Cell_Index(cellptr.CellID)] = false;
                continue;
            }

            /**
             *  Count how many neighbors we can potentially spread to.
             */
            int possible_spreads = 0;
            for (FacingType facing = FACING_N; facing < FACING_COUNT; facing++) {
                if (cellptr.Adjacent_Cell(facing).Can_Tiberium_Germinate(nullptr)) {
                    possible_spreads++;
                }
            }

            if (possible_spreads != 0) {
                CellClassExtension::Spread_Tiberium(&cellptr, false, SpreadSpawnStage);
                index++;

                /**
                 *  If there's more than one option, queue to spread again later.
                 */
                if (possible_spreads > 1) {
                    SpreadQueue.emplace(Frame + Random_Pick(0, 49), cell);
                    SpreadState[Map_Cell_Index(cellptr.CellID)] = true;
                }
            } else {
                SpreadState[Map_Cell_Index(cellptr.CellID)] = false;
            }
        }
    }
}


/**
 *  Initializes the spread logic for this Tiberium.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Init_Spread()
{
    Recalc_Spread();
}


/**
 *  Resets and recalculates the spread logic for this Tiberium.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Recalc_Spread()
{
    Clear_Spread();

    Map.Reset_Iterator();
    CellClass* iter = Map.Iterate();

    while (iter != nullptr) {
        if (iter->Tiberium_Type_Here() == This()->HeapID && iter->Can_Tiberium_Spread()) {
            SpreadQueue.emplace((float)Random_Pick(0, 10000), iter->CellID);
            SpreadState[Map_Cell_Index(iter->CellID)] = true;
        }
        iter = Map.Iterate();
    }
}


/**
 *  Clears and recalculates the spread logic for this Tiberium.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Clear_Spread()
{
    SpreadQueue = decltype(SpreadQueue)();
    SpreadState.clear();
    SpreadState.resize(Map_Cell_Count());
}


/**
 *  Queues Tiberium to spread FROM this cell.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Queue_Spread(Cell const& cell)
{
    if (Map[cell].Can_Tiberium_Spread() && !SpreadState[Map_Cell_Index(cell)]) {
        SpreadQueue.emplace(Frame + Random_Pick(0, 49), cell);
        SpreadState[Map_Cell_Index(cell)] = true;
    }
}


/**
 *  Tiberium growth logic main function.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Growth_AI()
{
    if (!GrowthQueue.empty() && This()->GrowthPercentage > 0.00001) {

        /**
         *  The number of growths depends on the queue size and Tiberium properties,
         *  capped for performance (and a minimum so that it never completely halts).
         */
        int count = std::clamp(static_cast<int>(GrowthQueue.size() * This()->GrowthPercentage), 5, 300);
        count = Random_Pick(1, count);

        int index = 0;
        while (index < count && !GrowthQueue.empty()) {
            auto node = GrowthQueue.top();
            GrowthQueue.pop();

            Cell cell = node.second;
            CellClass& cellptr = Map[cell];

            /**
             *  If we can't grow, skip without increasing the counter. This fixes a bug in vanilla
             *  where sometimes the queue would contain a bunch of entries that all failed,
             *  and it appeared as if Tiberium broke.
             */
            if (!cellptr.Can_Tiberium_Grow()) {
                GrowthState[Map_Cell_Index(cell)] = false;
                continue;
            }

            if (cellptr.Tiberium_Type_Here() == This()->HeapID) {
                cellptr.Grow_Tiberium();

                /**
                 *  If the overlay isn't completely grown yet, queue it to grow
                 *  again later.
                 */
                if (cellptr.OverlayData < This()->FrameCount - 1) {
                    GrowthQueue.emplace(Frame + Random_Pick(0, 49), cell);
                    GrowthState[Map_Cell_Index(cell)] = true;
                    Queue_Spread(cell);
                } else {
                    GrowthState[Map_Cell_Index(cell)] = false;
                }
            }

            index++;
        }
    }
}


/**
 *  Initializes the spread logic for this Tiberium.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Init_Growth()
{
    Recalc_Growth();
}


/**
 *  Resets and recalculates the spread logic for this Tiberium.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Recalc_Growth()
{
    Clear_Growth();

    Map.Reset_Iterator();
    CellClass* iter = Map.Iterate();

    while (iter != nullptr) {
        if (iter->Tiberium_Type_Here() == This()->HeapID && iter->Can_Tiberium_Grow()) {
            GrowthQueue.emplace((float)Random_Pick(0, 10000), iter->CellID);
            GrowthState[Map_Cell_Index(iter->CellID)] = true;
        }
        iter = Map.Iterate();
    }
}


/**
 *  Clears and recalculates the spread logic for this Tiberium.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Clear_Growth()
{
    GrowthQueue = decltype(GrowthQueue)();
    GrowthState.clear();
    GrowthState.resize(Map_Cell_Count());
}


/**
 *  Queues Tiberium to spread AT this cell.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Queue_Growth(Cell const& cell)
{
    if (Map[cell].OverlayData < This()->FrameCount - 1) {
        GrowthQueue.emplace(Frame + Random_Pick(0, 49), cell);
        GrowthState[Map_Cell_Index(cell)] = true;
    }
}


/**
 *  Marks this cell as not spreading for all Tiberiums.
 *
 *  @author: ZivDero, tomsons26
 */
void TiberiumClassExtension::Clear_Spread_State(Cell const& cell)
{
    int cellindex = Map_Cell_Index(cell);
    for (int i = 0; i < Tiberiums.Count(); i++) {
        TiberiumExtensions[i]->SpreadState[cellindex] = false;
    }
}
