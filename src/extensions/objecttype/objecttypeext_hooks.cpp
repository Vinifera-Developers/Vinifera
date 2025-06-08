/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OBJECTTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended ObjectTypeClass.
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
#include "objecttypeext_hooks.h"
#include "objecttypeext.h"
#include "objecttype.h"
#include "theatertype.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "house.h"
#include "housetype.h"
#include "scenario.h"
#include "wstring.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "building.h"
#include "buildingtypeext.h"
#include "extension.h"
#include "voxellib.h"
#include "motionlib.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "miscutil.h"
#include "rulesext.h"
#include "unit.h"
#include "unittypeext.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(ObjectTypeClass)
{
public:
    void _Assign_Theater_Name(char *buffer, TheaterType theater);
    const ShapeSet * _Get_Image_Data() const;
    void _Fetch_Voxel_Image();
    static void _Clear_Voxel_Indexes();
    BuildingClass* _Who_Can_Build_Me(bool intheory, bool needsnopower, bool legal, HouseClass* house) const;
};


/**
 *  Reimplementation of ObjectTypeClass::Theater_Naming_Convention to support new theater types.
 * 
 *  @author: CCHyper
 */
void ObjectTypeClassExt::_Assign_Theater_Name(char *fname, TheaterType theater)
{
    /**
     *  Make sure filename is uppercase.
     */
    strupr(fname);

    /**
     *  An edge case we exposed in the original game where it assumed anything that
     *  matched the pattern (e.g. GACNST) would also be marked with "IsNewTheater".
     *  Now that we support custom theaters, this means that might no longer be
     *  the case, so we perform a check before we perform the filename theater remap
     *  so the filename is left unmodified.
     */
    if (!IsNewTheater) return;

    /**
     *  Another edge case we have exposed in the original where some civilian buildings were
     *  marked with "IsNewTheater", but did not follow the theater filename system. These
     *  were most likely early additions into the game development when it was still using
     *  the Red Alert filename format. Unfortunately, the only way we can resolve this is
     *  to hard code checks for this filename prefixes and skip any remap attempt.
     */
    if (RTTI == RTTI_BUILDINGTYPE && (!std::strncmp(fname, "CITY", 4) || !std::strncmp(fname, "ABAN", 4) || !std::strncmp(fname, "BBOARD", 5))) {
        DEV_DEBUG_WARNING("Skipping new theater filename remap of %s!\n", fname);
        return;
    }

    /**
     *  Same as above, but for the deployed mobile war factory, cabal obelisk, and their
     *  respective animations.
     */
    if (RTTI == RTTI_BUILDINGTYPE && (std::strstr(fname, "MWAR") || std::strstr(fname, "OBL1"))) {
        DEV_DEBUG_WARNING("Skipping new theater filename remap of %s!\n", fname);
        return;
    }

    if (theater != THEATER_NONE && theater < TheaterTypes.Count()) {

        char first = fname[0];
        char second = fname[1];

        /**
         *  Remap the second character to the current theater image character. We perform
         *  a simple check to make sure the characters are valid.
         */
        if (std::isalpha(first) && std::isalpha(second)) {
            fname[0] = first;
            fname[1] = TheaterTypeClass::ImageLetter_From(theater);

        } else {
            DEV_DEBUG_WARNING("Failed to remap filename \"%s\" to current theater (%s)!\n", fname, TheaterTypeClass::Name_From(theater));
        }
    }
}


/**
 *  This patch replaces an inlined instance of ObjectTypeClass::Theater_Naming_Convention
 *  with a direct call.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ObjectTypeClass_Load_Theater_Art_Assign_Theater_Name_Theater_Patch)
{
    GET_REGISTER_STATIC(ObjectTypeClass *, this_ptr, edi);
    LEA_STACK_STATIC(char *, fullname, esp, 0x0C);
    LEA_STACK_STATIC(char *, destbuffer, esp, 0x08);

    this_ptr->Theater_Naming_Convention(fullname, Scen->Theater);

    JMP(0x005889E2);
}


/**
 *  Reimplementation of ObjectTypeClass::Get_Image_Data with added assertion.
 * 
 *  @author: CCHyper
 */
const ShapeSet * ObjectTypeClassExt::_Get_Image_Data() const
{
    if (Image == nullptr) {
        DEBUG_WARNING("Object %s has NULL image data!\n", Name());
    }

    return Image;
}


/**
 *  Fetches voxel model data from files.
 *
 *  @author: ZivDero
 */
void ObjectTypeClassExt::_Fetch_Voxel_Image()
{
    char buffer[260];

    if (IsVoxel)
    {
        if (Voxel.Load(VoxelIndex, Graphic_Name()))
        {
            unsigned char max_dimension = Voxel.VoxelLibrary->Get_Layer_Info(0, 0)->XSize;
            for (int i = 0; i < Voxel.VoxelLibrary->Get_Layer_Count(); i++)
            {
                max_dimension = std::max(max_dimension, Voxel.VoxelLibrary->Get_Layer_Info(i, 0)->XSize);
                max_dimension = std::max(max_dimension, Voxel.VoxelLibrary->Get_Layer_Info(i, 0)->YSize);
                max_dimension = std::max(max_dimension, Voxel.VoxelLibrary->Get_Layer_Info(i, 0)->ZSize);
            }

            max_dimension = std::max(max_dimension, static_cast<unsigned char>(8));
            MaxDimension = max_dimension;

            ShadowVoxelIndex.Clear();
        }
    }

    if (RTTI != RTTI_UNITTYPE || reinterpret_cast<UnitTypeClass*>(this)->IsTurretEquipped)
    {
        std::snprintf(buffer, sizeof(buffer), "%sTUR", Graphic_Name());
        AuxVoxel.Load(AuxVoxelIndex, buffer);

        std::snprintf(buffer, sizeof(buffer), "%sBARL", Graphic_Name());
        AuxVoxel2.Load(AuxVoxel2Index, buffer);
    }
}


/**
 *  Clears voxel caches.
 *
 *  @author: ZivDero
 */
void ObjectTypeClassExt::_Clear_Voxel_Indexes()
{
    for (int i = 0; i < ObjectTypes.Count(); i++)
    {
        const auto otype = ObjectTypes[i];
        otype->VoxelIndex.Clear();
        otype->AuxVoxelIndex.Clear();
        otype->ShadowVoxelIndex.Clear();
        otype->AuxVoxel2Index.Clear();

        const auto otype_ext = Extension::Fetch(otype);
        otype_ext->NoSpawnVoxelIndex.Clear();
        otype_ext->WaterVoxelIndex.Clear();
    }

    StaticBuffer.CurrentBufferPtr = StaticBuffer.BufferPtr;
}


/**
 *  Reimplementation of ObjectTypeClass::Who_Can_Build_Me.
 *
 *  @author: ZivDero
 */
BuildingClass* ObjectTypeClassExt::_Who_Can_Build_Me(bool intheory, bool needsnopower, bool legal, HouseClass* house) const
{
    BuildingClass* freebuilding = nullptr;
    BuildingClass* anybuilding = nullptr;
    int ownable = Get_Ownable();

    for (int index = 0; index < Buildings.Count(); index++) {
        BuildingClass* building = Buildings[index];

        if (!building->IsInLimbo &&
            building->House == house &&
            building->Class->ToBuild == RTTI &&
            (!needsnopower || building->IsPowerOn) &&
            building->Mission != MISSION_DECONSTRUCTION && building->MissionQueue != MISSION_DECONSTRUCTION &&
            (!legal || building->House->Can_Build(this, true, true) > 0) &&
            building->Class->Get_Ownable() & ownable &&

            /*
            **	Construction yards can only produce objects according to their ActLike, but not if MultiMCV is enabled.
            */
            (!Rule->BuildConst.Is_Present(building->Class) || RuleExtension->IsMultiMCV || 1L << building->ActLike & ownable)) {

            if (RTTI == RTTI_UNITTYPE || RTTI == RTTI_INFANTRYTYPE || RTTI == RTTI_BUILDINGTYPE || RTTI == RTTI_AIRCRAFTTYPE) {
                TechnoTypeClassExtension* type_ext = Extension::Fetch<TechnoTypeClassExtension>(this);
                BuildingTypeClassExtension* btype_ext = Extension::Fetch<BuildingTypeClassExtension>(building->Class);

                /*
                **	This object doesn't allow this factory to produce it.
                */
                if (type_ext->BuiltAt.Count() != 0 && !type_ext->BuiltAt.Is_Present(building->Class)) continue;

                /*
                **	This factory doesn't produce this kind of object.
                */
                if (btype_ext->IsExclusiveFactory && !type_ext->BuiltAt.Is_Present(building->Class)) continue;
            }

            if (intheory || !building->In_Radio_Contact() || RTTI != RTTI_AIRCRAFTTYPE) {
                if (RTTI == RTTI_UNITTYPE) {
                    UnitTypeClassExtension* type_ext = Extension::Fetch<UnitTypeClassExtension>(this);
                    BuildingTypeClassExtension* btype_ext = Extension::Fetch<BuildingTypeClassExtension>(building->Class);
                    if (btype_ext->IsNaval != type_ext->IsNaval) continue;
                }
                if (building->IsLeader) return building;
                freebuilding = building;
            } else {
                if (RTTI == RTTI_AIRCRAFTTYPE) {
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


/**
 *  Main function for patching the hooks.
 */
void ObjectTypeClassExtension_Hooks()
{
    //Patch_Jump(0x004101A0, &ObjectTypeClassExt::_Get_Image_Data);
    Patch_Jump(0x00588D00, &ObjectTypeClassExt::_Assign_Theater_Name);
    Patch_Jump(0x0058891D, &_ObjectTypeClass_Load_Theater_Art_Assign_Theater_Name_Theater_Patch);
    Patch_Jump(0x00587C80, &ObjectTypeClassExt::_Fetch_Voxel_Image);
    Patch_Jump(0x00589030, &ObjectTypeClassExt::_Clear_Voxel_Indexes);
    Patch_Jump(0x00587B20, &ObjectTypeClassExt::_Who_Can_Build_Me);
}
