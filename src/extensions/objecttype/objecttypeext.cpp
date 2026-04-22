/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended ObjectTypeClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "objecttypeext.h"

#include "asserthandler.h"
#include "building.h"
#include "buildingtypeext.h"
#include "ccini.h"
#include "extension_globals.h"
#include "house.h"
#include "miscutil.h"
#include "motionlib.h"
#include "objecttype.h"
#include "rules.h"
#include "rulesext.h"
#include "technotypeext.h"
#include "unittypeext.h"
#include "voxellib.h"
#include "vinifera_globals.h"

#include "audio_sample.h"
#include "voc.h"

/**
 *  Class constructor.
 *
 *  @author: CCHyper
 */
ObjectTypeClassExtension::ObjectTypeClassExtension(const ObjectTypeClass *this_ptr) :
    AbstractTypeClassExtension(this_ptr),
    GraphicName(),
    AlphaGraphicName(),
    NoSpawnAlt(false),
    NoSpawnVoxel(),
    NoSpawnVoxelIndex(),
    WaterAlt(false),
    WaterVoxel(),
    WaterVoxelIndex(),
    AmbientSound(VOC_NONE)
{
    //if (this_ptr) EXT_DEBUG_TRACE("ObjectTypeClassExtension::ObjectTypeClassExtension - Name: %s (0x%08X)\n", Name(), (uintptr_t)(This()));
}


/**
 *  Class no-init constructor.
 *  
 *  @author: CCHyper
 */
ObjectTypeClassExtension::ObjectTypeClassExtension(const NoInitClass &noinit) :
    AbstractTypeClassExtension(noinit),
    GraphicName(noinit),
    AlphaGraphicName(noinit),
    NoSpawnVoxel(noinit),
    WaterVoxel(noinit)
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

    NoSpawnVoxel.MotionLibrary = nullptr;
    NoSpawnVoxel.VoxelLibrary = nullptr;

    WaterVoxel.MotionLibrary = nullptr;
    WaterVoxel.VoxelLibrary = nullptr;

    Fetch_Voxel_Image(GraphicName.c_str());
    
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
     *  Store the graphic name strings, these are used by the load operation.
     */
    GraphicName = Graphic_Name();
    AlphaGraphicName = Alpha_Graphic_Name();

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
        WaterAlt = strcmpi(This()->IniName.c_str(), "APC") == 0;
    }

    if (!AbstractTypeClassExtension::Read_INI(ini)) {
        return false;
    }

    if (!ini.Is_Present(ini_name)) {
        return false;
    }

    NoSpawnAlt = ini.Get_Bool(ini_name, "NoSpawnAlt", NoSpawnAlt);
    WaterAlt = ini.Get_Bool(ini_name, "WaterAlt", WaterAlt);

    if (This()->IsVoxel) {
        Fetch_Voxel_Image(Graphic_Name());
    }
    
    AmbientSound = ini.Get_VocType(ini_name, "AmbientSound", AmbientSound);

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

    if (NoSpawnAlt) {
        std::snprintf(buffer, sizeof(buffer), "%sWO", graphic_name);
        NoSpawnVoxel.Load(NoSpawnVoxelIndex, buffer);
    }

    if (WaterAlt) {
        std::snprintf(buffer, sizeof(buffer), "%sW", graphic_name);
        WaterVoxel.Load(WaterVoxelIndex, buffer);
    }
}


/**
 *  Reimplementation of ObjectTypeClass::Who_Can_Build_Me.
 *
 *  @author: ZivDero
 */
BuildingClass* ObjectTypeClassExtension::Who_Can_Build_Me(bool intheory, bool needsnopower, bool legal, HouseClass* house, bool to_exit) const
{
    BuildingClass* freebuilding = nullptr;
    BuildingClass* anybuilding = nullptr;
    int ownable = This()->Get_Ownable();

    for (int index = 0; index < Buildings.Count(); index++) {
        BuildingClass* building = Buildings[index];

        if (!building->IsInLimbo &&
            building->House == house &&
            building->Class->ToBuild == This()->RTTI &&
            (!needsnopower || building->IsOn) &&
            building->Mission != MISSION_DECONSTRUCTION && building->MissionQueue != MISSION_DECONSTRUCTION &&
            (!legal || building->House->Can_Build(This(), true, true) > 0) &&
            building->Class->Get_Ownable() & ownable &&

            /*
            **  Construction yards can only produce objects according to their ActLike, but not if MultiMCV is enabled.
            */
            (!Rule->BuildConst.Is_Present(building->Class) || RuleExtension->IsMultiMCV || 1L << building->ActLike & ownable)) {

            if (This()->RTTI == RTTI_UNITTYPE || This()->RTTI == RTTI_INFANTRYTYPE || This()->RTTI == RTTI_BUILDINGTYPE || This()->RTTI == RTTI_AIRCRAFTTYPE) {
                const TechnoTypeClassExtension* type_ext = static_cast<const TechnoTypeClassExtension*>(this);
                BuildingTypeClassExtension* btype_ext = Extension::Fetch(building->Class);

                /*
                ** There may be limitations on whether this specific factory can build this object.
                */
                if (!type_ext->BuiltAt.Is_Present(building->Class)) {

                    /*
                    **  This object doesn't allow this factory to produce it.
                    */
                    if (type_ext->BuiltAt.Count() != 0) continue;

                    /*
                    **  This factory can't produce this kind of object.
                    */
                    if (btype_ext->IsExclusiveFactory) continue;
                }
            }

            /*
            **  If we're looking for a place to exit then don't consider weapons factories doing MISSION_UNLOAD (because they are currently exiting something).
            */
            if (to_exit && building->Class->IsWeaponsFactory && building->Mission == MISSION_UNLOAD) continue;

            if (intheory || !building->In_Radio_Contact() || This()->RTTI != RTTI_AIRCRAFTTYPE) {
                if (This()->RTTI == RTTI_UNITTYPE) {
                    const UnitTypeClassExtension* type_ext = static_cast<const UnitTypeClassExtension*>(this);
                    BuildingTypeClassExtension* btype_ext = Extension::Fetch(building->Class);
                    if (btype_ext->IsNaval != type_ext->IsNaval) continue;
                }
                if (building->IsLeader) return building;
                freebuilding = building;
            } else {
                if (This()->RTTI == RTTI_AIRCRAFTTYPE) {
                    anybuilding = building;
                }
            }
        }
    }

    if (freebuilding != nullptr) {
        return freebuilding;
    }

    return anybuilding;
}
