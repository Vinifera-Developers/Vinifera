/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TIBERIUMEXT.CPP
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
#include "tiberiumext.h"
#include "tiberium.h"
#include "ccini.h"
#include "overlaytype.h"
#include "extension.h"
#include "asserthandler.h"
#include "cellext.h"
#include "debughandler.h"
#include "findmake.h"
#include "mouse.h"
#include "tibsun_globals.h"
#include "vinifera_saveload.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TiberiumClassExtension::TiberiumClassExtension(const TiberiumClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    MinSpreadStage(0),
    SpawnSpreadStage(5)
{
    //if (this_ptr) EXT_DEBUG_TRACE("TiberiumClassExtension::TiberiumClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("TiberiumClassExtension::TiberiumClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TiberiumClassExtension::~TiberiumClassExtension()
{
    //EXT_DEBUG_TRACE("TiberiumClassExtension::~TiberiumClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TiberiumExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT TiberiumClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("TiberiumClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("TiberiumClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("TiberiumClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
    //EXT_DEBUG_TRACE("TiberiumClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TiberiumClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("TiberiumClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TiberiumClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TiberiumClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool TiberiumClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("TiberiumClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    const char* ini_name = Name();

    if (!IsInitialized) {
        This()->FrameCount = 12;
        This()->Variety = 12;
        DamageToInfantry = std::max(1, This()->Power / 10);
        MinSpreadStage = This()->HeapID / 2 + 1; // default from CellClass::Can_Tiberium_Spread
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
    SpawnSpreadStage = ini.Get_Int(ini_name, "SpawnSpreadStage", SpawnSpreadStage);

    IsInitialized = true;
    
    return true;
}


int Map_Cell_Index(Cell const& cell)
{
    return ((cell.X - cell.Y + Map.PlayRect.Width - 1) >> 1) + Map.PlayRect.Width * (cell.X - Map.PlayRect.Width + cell.Y - 1);
}


int Map_Cell_Count(void)
{
    return (2 * Map.PlayRect.Width) * (Map.PlayRect.Height + 4);
}


void TiberiumClassExtension::Spread_AI()
{
    if (!SpreadQueue.empty() && This()->SpreadPercentage > 0.00001) {
        int count = std::clamp(static_cast<int>(SpreadQueue.size() * This()->SpreadPercentage), 5, 300);
        count = Random_Pick(1, count);

        int index = 0;
        while (index < count && !SpreadQueue.empty()) {
            auto node = SpreadQueue.top();
            SpreadQueue.pop();

            Cell cell = node.second;
            CellClass& cellptr = Map[cell];

            if (!cellptr.Can_Tiberium_Spread()) {
                SpreadState[Map_Cell_Index(cellptr.CellID)] = false;
                continue;
            }

            int numallowed = 0;

            for (FacingType facing = FACING_N; facing < FACING_COUNT; facing++) {
                if (cellptr.Adjacent_Cell(facing).Can_Tiberium_Germinate(nullptr)) {
                    numallowed++;
                }
            }

            if (numallowed != 0) {
                CellClassExtension::Spread_Tiberium(&cellptr, false, SpawnSpreadStage);
                index++;

                if (numallowed > 1) {
                    SpreadQueue.emplace(Frame + Random_Pick(0, 49), cell);
                    SpreadState[Map_Cell_Index(cellptr.CellID)] = true;
                }
            } else {
                SpreadState[Map_Cell_Index(cellptr.CellID)] = false;
            }
        }
    }
}


void TiberiumClassExtension::Initialize_Spread()
{
    Recalc_Spread();
}


void TiberiumClassExtension::Recalc_Spread()
{
    Clear_Spread();

    Map.Reset_Iterator();
    CellClass* iter = Map.Iterate();

    while (iter != nullptr) {
        if (iter->Tiberium_Type_Here() == This()->HeapID && iter->Can_Tiberium_Spread()) {
            SpreadQueue.emplace(0.0, iter->CellID);
            SpreadState[Map_Cell_Index(iter->CellID)] = true;
        }
        iter = Map.Iterate();
    }
}


void TiberiumClassExtension::Clear_Spread()
{
    SpreadQueue = decltype(SpreadQueue)();
    SpreadState.clear();
    SpreadState.resize(Map_Cell_Count());
}


void TiberiumClassExtension::Queue_Spread(Cell const& cell)
{
    if (Map[cell].Can_Tiberium_Spread() && !SpreadState[Map_Cell_Index(cell)]) {
        SpreadQueue.emplace(Frame + Random_Pick(0, 49), cell);
        SpreadState[Map_Cell_Index(cell)] = true;
    }
}


void TiberiumClassExtension::Growth_AI()
{
    if (!GrowthQueue.empty() && This()->GrowthPercentage > 0.00001) {
        int count = std::clamp(static_cast<int>(GrowthQueue.size() * This()->GrowthPercentage), 5, 300);
        count = Random_Pick(1, count);

        int index = 0;
        while (index < count && !GrowthQueue.empty()) {
            auto node = GrowthQueue.top();
            GrowthQueue.pop();

            Cell cell = node.second;
            CellClass& cellptr = Map[cell];

            if (!cellptr.Can_Tiberium_Grow()) {
                GrowthState[Map_Cell_Index(cell)] = false;
                continue;
            }

            if (cellptr.Tiberium_Type_Here() == This()->HeapID) {
                cellptr.Grow_Tiberium();

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


void TiberiumClassExtension::Initialize_Growth()
{
    Recalc_Growth();
}


void TiberiumClassExtension::Recalc_Growth()
{
    Clear_Growth();

    Map.Reset_Iterator();
    CellClass* iter = Map.Iterate();

    while (iter != nullptr) {
        if (iter->Tiberium_Type_Here() == This()->HeapID && iter->Can_Tiberium_Grow()) {
            GrowthQueue.emplace(0.0, iter->CellID);
            GrowthState[Map_Cell_Index(iter->CellID)] = true;
        }
        iter = Map.Iterate();
    }
}


void TiberiumClassExtension::Clear_Growth()
{
    GrowthQueue = decltype(GrowthQueue)();
    GrowthState.clear();
    GrowthState.resize(Map_Cell_Count());
}


void TiberiumClassExtension::Queue_Growth(Cell const& cell)
{
    if (Map[cell].OverlayData < This()->FrameCount - 1) {
        GrowthQueue.emplace(Frame + Random_Pick(0, 49), cell);
        GrowthState[Map_Cell_Index(cell)] = true;
    }
}


void TiberiumClassExtension::Clear_Tiberium_Spread_State(Cell const& cell)
{
    int cellindex = Map_Cell_Index(cell);
    for (int i = 0; i < Tiberiums.Count(); i++) {
        TiberiumExtensions[i]->SpreadState[cellindex] = false;
    }
}
