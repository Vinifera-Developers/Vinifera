/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TERRAINEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TerrainClass class.
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
#include "terrainext.h"
#include "terrain.h"
#include "lightsource.h"
#include "wwcrc.h"
#include "extension.h"
#include "tiberium.h"
#include "mouse.h"
#include "cellext.h"
#include "terraintypeext.h"
#include "asserthandler.h"
#include "debughandler.h"
#include <random>


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
TerrainClassExtension::TerrainClassExtension(const TerrainClass *this_ptr) :
    ObjectClassExtension(this_ptr),
    LightSource(nullptr)
{
    //if (this_ptr) EXT_DEBUG_TRACE("TerrainClassExtension::TerrainClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    TerrainExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
TerrainClassExtension::TerrainClassExtension(const NoInitClass &noinit) :
    ObjectClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("TerrainClassExtension::TerrainClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TerrainClassExtension::~TerrainClassExtension()
{
    //EXT_DEBUG_TRACE("TerrainClassExtension::~TerrainClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    if (LightSource) {
        LightSource->Disable();
        delete LightSource;
        LightSource = nullptr;
    }

    TerrainExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *  
 *  @author: CCHyper
 */
HRESULT TerrainClassExtension::GetClassID(CLSID *lpClassID)
{
    //EXT_DEBUG_TRACE("TerrainClassExtension::GetClassID - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

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
HRESULT TerrainClassExtension::Load(IStream *pStm)
{
    //EXT_DEBUG_TRACE("TerrainClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TerrainClassExtension(NoInitClass());

    LightSource = nullptr;
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT TerrainClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("TerrainClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = ObjectClassExtension::Save(pStm, fClearDirty);
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
int TerrainClassExtension::Get_Object_Size() const
{
    //EXT_DEBUG_TRACE("TerrainClassExtension::Get_Object_Size - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    return sizeof(*this);
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void TerrainClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("TerrainClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    ObjectClassExtension::Detach(target, all);
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TerrainClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("TerrainClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Spreads Tiberium from this Terrain.
 *
 *  @author: ZivDero
 */
void TerrainClassExtension::Spread_Tiberium() const
{
    TerrainTypeClass* ttype = This()->Class;
    TerrainTypeClassExtension* ttype_ext = Extension::Fetch(ttype);

    Cell origin = This()->PositionCell;
    int count = Scen->RandomNumber(ttype_ext->TiberiumSpawnCount.X, ttype_ext->TiberiumSpawnCount.Y);

    TiberiumClass* tiberium = Tiberiums[ttype->TiberiumToSpawn];

    int spreads = 0;

    if (ttype_ext->IsTiberiumScatterSpawn) {
        static std::vector<Cell> cells(128);
        cells.clear();

        /**
         *  Collect all possible cells to spread to.
         */
        const int r = ttype_ext->TiberiumSpawnRange;
        const int r2 = r * r;

        for (int x = origin.X - r; x <= origin.X + r; ++x) {
            for (int y = origin.Y - r; y <= origin.Y + r; ++y) {
                int dx = x - origin.X;
                int dy = y - origin.Y;
                int dist2 = dx * dx + dy * dy;
                if (dist2 <= r2 && Map[Cell(x, y)].Can_Tiberium_Germinate(Tiberiums[ttype->TiberiumToSpawn])) {
                    cells.emplace_back(x, y);
                }
            }
        }

        /**
         *  Place Tiberium as many times as we want to spread.
         */
        while (spreads < count && !cells.empty()) {
            int index = Random_Pick(0u, cells.size() - 1);
            Cell cell = cells[index];
            cells.erase(cells.begin() + index);
            int stage = Scen->RandomNumber(ttype_ext->TiberiumSpawnStage.X, ttype_ext->TiberiumSpawnStage.Y);
            stage = std::clamp(stage, 0, tiberium->FrameCount - 1);
            if (Map[cell].Place_Tiberium(ttype->TiberiumToSpawn, stage)) {
                spreads++;
            }
        }

    } else {

        /**
         *  First try spreading from under the tree, like in vanilla.
         *  Try up to 8 times, since there are 8 cells bordering the center.
         */
        for (int i = 0; i < FACING_COUNT; i++) {
            int stage = Scen->RandomNumber(ttype_ext->TiberiumSpawnStage.X, ttype_ext->TiberiumSpawnStage.Y);
            stage = std::clamp(stage, 0, tiberium->FrameCount - 1);
            if (CellClassExtension::Spread_Tiberium(&Map[This()->PositionCoord], true, stage)) {
                spreads++;
                if (spreads >= count) break;
            }
        }

        if (spreads >= count) return;

        /**
         *  Then we start spreading in concentric rings.
         */
        for (int r = 1; r <= ttype_ext->TiberiumSpawnRange - 1; ++r) {
            static std::vector<Cell> ring(128);
            ring.clear();

            /**
             *  Collect all cells of the current ring.
             */
            for (int x = origin.X - r; x <= origin.X + r; ++x) {
                for (int y = origin.Y - r; y <= origin.Y + r; ++y) {
                    int dx = x - origin.X;
                    int dy = y - origin.Y;
                    int dist2 = dx * dx + dy * dy;
                    int r_min2 = (r - 0.5) * (r - 0.5);
                    int r_max2 = (r + 0.5) * (r + 0.5);
                    if (dist2 >= r_min2 && dist2 < r_max2) {
                        ring.emplace_back(x, y);
                    }
                }
            }

            /**
             *  Shuffle them so that spread isn't orderly.
             */
            static std::minstd_rand rng(Scen->RandomNumber());
            std::shuffle(ring.begin(), ring.end(), rng);

            while (spreads < count) {

                /**
                 *  Track if we've spread at all in this cycle. If not, then bail.
                 */
                bool has_spread = false;

                /**
                 *  Try to spread from cells in this ring until we've spread enough.
                 */
                for (auto& cell : ring) {
                    if (Map[cell].Tiberium_Type_Here() == ttype->TiberiumToSpawn) {
                        int stage = Scen->RandomNumber(ttype_ext->TiberiumSpawnStage.X, ttype_ext->TiberiumSpawnStage.Y);
                        stage -= ttype_ext->TiberiumSpawnStageFalloff * r;
                        stage = std::clamp(stage, 0, tiberium->FrameCount - 1);

                        if (CellClassExtension::Spread_Tiberium(&Map[cell], true, stage)) {
                            spreads++;
                            has_spread = true;
                            if (spreads >= count) break;
                        }
                    }
                }

                if (!has_spread) break;
            }

            if (spreads >= count) break;
        }
    }
}
