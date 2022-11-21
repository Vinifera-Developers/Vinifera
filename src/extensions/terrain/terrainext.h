/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TERRAINEXT.H
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
#pragma once

#include "objectext.h"
#include "terrain.h"


class LightSourceClass;


class DECLSPEC_UUID(UUID_TERRAIN_EXTENSION)
TerrainClassExtension final : public ObjectClassExtension
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
        TerrainClassExtension(const TerrainClass *this_ptr = nullptr);
        TerrainClassExtension(const NoInitClass &noinit);
        virtual ~TerrainClassExtension();

        virtual int Size_Of() const override;
        virtual void Detach(TARGET target, bool all = true) override;
        virtual void Compute_CRC(WWCRCEngine &crc) const override;

        virtual TerrainClass *This() const override { return reinterpret_cast<TerrainClass *>(ObjectClassExtension::This()); }
        virtual const TerrainClass *This_Const() const override { return reinterpret_cast<const TerrainClass *>(ObjectClassExtension::This_Const()); }
        virtual RTTIType What_Am_I() const override { return RTTI_TERRAIN; }

    public:
        /**
         *  The light source instance for this terrain object.
         */
        LightSourceClass *LightSource;
};
