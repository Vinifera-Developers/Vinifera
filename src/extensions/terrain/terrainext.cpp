/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TerrainClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "terrainext.h"

#include "cellext.h"
#include "extension.h"
#include "lightsource.h"
#include "mouse.h"
#include "terrain.h"
#include "terraintypeext.h"
#include "tiberium.h"
#include "wwcrc.h"

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
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
TerrainClassExtension::~TerrainClassExtension()
{
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
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void TerrainClassExtension::Object_CRC(CRCEngine &crc) const
{
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
