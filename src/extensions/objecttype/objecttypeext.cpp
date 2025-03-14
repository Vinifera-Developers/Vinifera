/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OBJECTTYPEEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended ObjectTypeClass class.
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
#include "objecttypeext.h"
#include "objecttype.h"
#include "ccini.h"
#include "asserthandler.h"
#include "ccfile.h"
#include "debughandler.h"
#include "voxellib.h"
#include "motionlib.h"
#include "miscutil.h"


/**
 *  Class constructor.
 *  
 *  @author: CCHyper
 */
ObjectTypeClassExtension::ObjectTypeClassExtension(const ObjectTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    GraphicName(),
    AlphaGraphicName(),
    NoSpawnAlt(false)
{
    //if (this_ptr) EXT_DEBUG_TRACE("ObjectTypeClassExtension::ObjectTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
ObjectTypeClassExtension::ObjectTypeClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit)
{
    //EXT_DEBUG_TRACE("ObjectTypeClassExtension::ObjectTypeClassExtension(NoInitClass) - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class destructor.
 *  
 *  @author: CCHyper
 */
ObjectTypeClassExtension::~ObjectTypeClassExtension()
{
    //EXT_DEBUG_TRACE("ObjectTypeClassExtension::~ObjectTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *  
 *  @author: CCHyper
 */
HRESULT ObjectTypeClassExtension::Load(IStream *pStm)
{
    NoSpawnVoxelIndex.Clear();
    WaterVoxelIndex.Clear();

    NoSpawnVoxel.~VoxelObject();
    WaterVoxel.~VoxelObject();

    //EXT_DEBUG_TRACE("ObjectTypeClassExtension::Load - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    HRESULT hr = AbstractTypeClassExtension::Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (&NoSpawnVoxelIndex) VoxelIndexClass;
    new (&WaterVoxelIndex) VoxelIndexClass;

    NoSpawnVoxel.Clear();
    WaterVoxel.Clear();

    Fetch_Voxel_Image(GraphicName);
    
    return hr;
}


/**
 *  Saves an object to the specified stream.
 *  
 *  @author: CCHyper
 */
HRESULT ObjectTypeClassExtension::Save(IStream *pStm, BOOL fClearDirty)
{
    //EXT_DEBUG_TRACE("ObjectTypeClassExtension::Save - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    /**
     *  Store the graphic name strings as raw data, these are used by the load operation.
     */
    std::strncpy(GraphicName, Graphic_Name(), sizeof(GraphicName));
    std::strncpy(AlphaGraphicName, Alpha_Graphic_Name(), sizeof(AlphaGraphicName));

    HRESULT hr = AbstractTypeClassExtension::Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Removes the specified target from any targeting and reference trackers.
 *  
 *  @author: CCHyper
 */
void ObjectTypeClassExtension::Detach(AbstractClass * target, bool all)
{
    //EXT_DEBUG_TRACE("ObjectTypeClassExtension::Detach - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Compute a unique crc value for this instance.
 *  
 *  @author: CCHyper
 */
void ObjectTypeClassExtension::Object_CRC(CRCEngine &crc) const
{
    //EXT_DEBUG_TRACE("ObjectTypeClassExtension::Object_CRC - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Fetches the extension data from the INI database.  
 *  
 *  @author: CCHyper
 */
bool ObjectTypeClassExtension::Read_INI(CCINIClass &ini)
{
    //EXT_DEBUG_TRACE("ObjectTypeClassExtension::Read_INI - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));

    const char* ini_name = Name();

    if (!IsInitialized) {
        WaterAlt = strcmpi(This()->IniName, "APC") == 0;
    }

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    NoSpawnAlt = ini.Get_Bool(ini_name, "NoSpawnAlt", NoSpawnAlt);
    WaterAlt = ini.Get_Bool(ini_name, "WaterAlt", WaterAlt);

    if (This()->IsVoxel)
    {
        Fetch_Voxel_Image(Graphic_Name());
    }
    
    return true;
}


/**
 *  Fetches new voxel model data from files.
 *
 *  @author: ZivDero
 */
void ObjectTypeClassExtension::Fetch_Voxel_Image(const char* graphic_name)
{
    char buffer[260];

    if (NoSpawnAlt)
    {
        std::snprintf(buffer, sizeof(buffer), "%sWO", graphic_name);
        NoSpawnVoxel.Load(NoSpawnVoxelIndex, buffer);
    }

    if (WaterAlt)
    {
        std::snprintf(buffer, sizeof(buffer), "%sW", graphic_name);
        WaterVoxel.Load(WaterVoxelIndex, buffer);
    }
}

