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
#include "extension.h"
#include "voxellib.h"
#include "motionlib.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "miscutil.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class ObjectTypeClassExt final : public ObjectTypeClass
{
    public:
        void _Assign_Theater_Name(char *buffer, TheaterType theater);
        const ShapeFileStruct * _Get_Image_Data() const;
        void _Fetch_Voxel_Image();
        static void _Clear_Voxel_Indexes();
};


/**
 *  Reimplementation of ObjectTypeClass::Assign_Theater_Name to support new theater types.
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
    if (What_Am_I() == RTTI_BUILDINGTYPE && (!std::strncmp(fname, "CITY", 4) || !std::strncmp(fname, "ABAN", 4) || !std::strncmp(fname, "BBOARD", 5))) {
        DEV_DEBUG_WARNING("Skipping new theater filename remap of %s!\n", fname);
        return;
    }

    /**
     *  Same as above, but for the deployed mobile war factory, cabal obelisk, and their
     *  respective animations.
     */
    if (What_Am_I() == RTTI_BUILDINGTYPE && (std::strstr(fname, "MWAR") || std::strstr(fname, "OBL1"))) {
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
 *  This patch replaces an inlined instance of ObjectTypeClass::Assign_Theater_Name
 *  with a direct call.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_ObjectTypeClass_Load_Theater_Art_Assign_Theater_Name_Theater_Patch)
{
    GET_REGISTER_STATIC(ObjectTypeClass *, this_ptr, edi);
    LEA_STACK_STATIC(char *, fullname, esp, 0x0C);
    LEA_STACK_STATIC(char *, destbuffer, esp, 0x08);

    this_ptr->Assign_Theater_Name(fullname, Scen->Theater);

    JMP(0x005889E2);
}


/**
 *  Reimplementation of ObjectTypeClass::Get_Image_Data with added assertion.
 * 
 *  @author: CCHyper
 */
const ShapeFileStruct * ObjectTypeClassExt::_Get_Image_Data() const
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
    bool success = true;

    success &= Load_Voxel(Voxel, Graphic_Name(), true);

    if (What_Am_I() != RTTI_UNITTYPE || reinterpret_cast<UnitTypeClass*>(this)->IsTurretEquipped)
    {
        std::snprintf(buffer, sizeof(buffer), "%sTUR", Graphic_Name());
        success &= Load_Voxel(AuxVoxel, buffer);

        std::snprintf(buffer, sizeof(buffer), "%sBARL", Graphic_Name());
        success &= Load_Voxel(AuxVoxel2, buffer);
    }

    if (success)
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

        VoxelIndex.Clear();
        AuxVoxelIndex.Clear();
        ShadowVoxelIndex.Clear();
        AuxVoxel2Index.Clear();
    }
    else
    {
        delete Voxel.VoxelLibrary;
        delete Voxel.MotionLibrary;
        Voxel.VoxelLibrary = nullptr;
        Voxel.MotionLibrary = nullptr;

        delete AuxVoxel.VoxelLibrary;
        delete AuxVoxel.MotionLibrary;
        AuxVoxel.VoxelLibrary = nullptr;
        AuxVoxel.MotionLibrary = nullptr;

        delete AuxVoxel2.VoxelLibrary;
        delete AuxVoxel2.MotionLibrary;
        AuxVoxel2.VoxelLibrary = nullptr;
        AuxVoxel2.MotionLibrary = nullptr;
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

        const auto otype_ext = Extension::Fetch<ObjectTypeClassExt>(otype);
        otype_ext->AuxVoxel2Index.Clear();
    }

    StaticBuffer.CurrentBufferPtr = StaticBuffer.BufferPtr;
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
}
