/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BUILDINGTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended BuildingTypeClass.
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
#include "buildingtypeext_hooks.h"

#include "buildingtypeext_init.h"
#include "buildingtypeext.h"
#include "buildingtype.h"
#include "bullettype.h"
#include "warheadtype.h"
#include "weapontype.h"
#include "rules.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "extension.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "tibsun_globals.h"
#include "verses.h"
#include "warheadtypeext.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 * 
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class BuildingTypeClassExt final : public BuildingTypeClass
{
    public:
        void _Free_Buildup_Image();
        void _Set_Base_Defense_Values();
};


/**
 *  Reimplementation of BuildingClass::Free_Buildup_Image.
 *
 *  @author: CCHyper
 */
void BuildingTypeClassExt::_Free_Buildup_Image()
{
    if (IsFreeBuildup && BuildupData) {
        DEV_DEBUG_WARNING("Freeing loaded buildup image for %s\n", Name());

        /**
         *  The original function would incorrectly try to free memory
         *  that the game does not actually allocate, and as a result of
         *  this, Vinifera's new memory management triggers an assertion
         *  because this is no longer allowed. The original game silently
         *  failed when doing this.
         *
         *  We now remove this and just correctly nullify the pointer.
         */
        //delete BuildupData;

        BuildupData = nullptr;
    }
}


/**
 *  Reimplementation of BuildingClass::Set_Base_Defense_Values.
 *
 *  @author: ZivDero
 */
void BuildingTypeClassExt::_Set_Base_Defense_Values()
{
    if (this->IsBaseDefense)
    {
        const WeaponTypeClass* weapon = Fetch_Weapon_Info(WEAPON_SLOT_PRIMARY).Weapon;
        if (weapon != nullptr)
        {
            int damage = weapon->Attack / (weapon->ROF * 0.025);

            if (weapon->Bullet->IsAntiAircraft)
            {
                int anti_air_value = damage * Verses::Get_Modifier(ARMOR_STEEL, weapon->WarheadPtr);
                AntiAirValue = std::min(anti_air_value, Rule->MaximumBaseDefenseValue);
            }

            if (weapon->Bullet->IsAntiGround)
            {
                int anti_armor_value = damage * Verses::Get_Modifier(ARMOR_STEEL, weapon->WarheadPtr);
                AntiArmorValue = std::min(anti_armor_value, Rule->MaximumBaseDefenseValue);

                int anti_ground_value = damage * Verses::Get_Modifier(ARMOR_NONE, weapon->WarheadPtr);
                AntiArmorValue = std::min(anti_ground_value, Rule->MaximumBaseDefenseValue);
            }
        }
    }
}


/**
 *  Write to the debug log when freeing up pre-loaded buildup images.
 * 
 *  #NOTE:
 *  These patches are also done to remove the incorrect freeing
 *  of memory the game does not actually allocate, and as a result
 *  of this, Vinifera's new memory management triggers an assertion
 *  because this is not allowed. The original game silently failed
 *  when doing this.
 * 
 *  @author: CCHyper
 */
static void BuildingTypeClass_Free_Buildup_Image(BuildingTypeClass *this_ptr)
{
    if (this_ptr->IsDemandLoadBuildup && this_ptr->BuildupData) {
        DEV_DEBUG_WARNING("Freeing loaded buildup image for %s\n", this_ptr->Name());

        /**
         *  The original function would incorrectly try to free memory
         *  that the game does not actually allocate, and as a result of
         *  this, Vinifera's new memory management triggers an assertion
         *  because this is no longer allowed. The original game silently
         *  failed when doing this.
         *
         *  We now remove this and just correctly nullify the pointer.
         */
        //delete this_ptr->BuildupData;

        this_ptr->BuildupData = nullptr;
    }
}


/**
 *  Write to the debug log when freeing up pre-loaded buildup images.
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingTypeClass_SDDTOR_Free_Buildup_Image_Patch) { GET_REGISTER_STATIC(BuildingTypeClass *, this_ptr, esi); BuildingTypeClass_Free_Buildup_Image(this_ptr); JMP(0x00444079); }
DECLARE_PATCH(_BuildingTypeClass_Init_Free_Buildup_Image_Patch) { GET_REGISTER_STATIC(BuildingTypeClass *, this_ptr, esi); BuildingTypeClass_Free_Buildup_Image(this_ptr); JMP(0x0043FDBF); }
DECLARE_PATCH(_BuildingTypeClass_DTOR_Free_Buildup_Image_Patch) { GET_REGISTER_STATIC(BuildingTypeClass *, this_ptr, esi); BuildingTypeClass_Free_Buildup_Image(this_ptr); JMP(0x0043F949); }


/**
 *  Write to the debug log when freeing up pre-loaded images.
 * 
 *  #NOTE:
 *  These patches are also done to remove the incorrect freeing
 *  of memory the game does not actually allocate, and as a result
 *  of this, Vinifera's new memory management triggers an assertion
 *  because this is not allowed. The original game silently failed
 *  when doing this.
 * 
 *  @author: CCHyper
 */
static void BuildingTypeClass_Free_Image(BuildingTypeClass *this_ptr)
{
    if (this_ptr->IsDemandLoad && this_ptr->Image) {
        DEV_DEBUG_WARNING("Building: Freeing loaded image for %s\n", this_ptr->Name());

        /**
         *  The original function would incorrectly try to free memory
         *  that the game does not actually allocate, and as a result of
         *  this, Vinifera's new memory management triggers an assertion
         *  because this is no longer allowed. The original game silently
         *  failed when doing this.
         *
         *  We now remove this and just correctly nullify the pointer.
         */
        //delete this_ptr->Image;

        this_ptr->Image = nullptr;
    }
}


/**
 *  Write to the debug log when freeing up pre-loaded buildup images.
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingTypeClass_SDDTOR_Free_Image_Patch) { GET_REGISTER_STATIC(BuildingTypeClass *, this_ptr, esi); BuildingTypeClass_Free_Image(this_ptr); JMP(0x00444052); }
DECLARE_PATCH(_BuildingTypeClass_Init_Free_Image_Patch) { GET_REGISTER_STATIC(BuildingTypeClass *, this_ptr, esi); BuildingTypeClass_Free_Image(this_ptr); JMP(0x0043FD9E); }
DECLARE_PATCH(_BuildingTypeClass_DTOR_Free_Image_Patch) { GET_REGISTER_STATIC(BuildingTypeClass *, this_ptr, esi); BuildingTypeClass_Free_Image(this_ptr); JMP(0x0043F922); }


/**
 *  Patches in an assertion check for image data.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BuildingTypeClass_Get_Image_Data_Assertion_Patch)
{
    GET_REGISTER_STATIC(BuildingTypeClass *, this_ptr, esi);
    GET_REGISTER_STATIC(const ShapeFileStruct *, image, eax);

    if (image == nullptr) {
        DEBUG_WARNING("Building %s has NULL image data!\n", this_ptr->Name());
    }

    _asm { mov eax, image } // restore eax state.
    _asm { pop esi }
    _asm { add esp, 0x64 }
    _asm { ret }
}


/**
 *  Main function for patching the hooks.
 */
void BuildingTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    BuildingTypeClassExtension_Init();

    //Patch_Jump(0x00440365, &_BuildingTypeClass_Get_Image_Data_Assertion_Patch);

    Patch_Jump(0x00443CF0, &BuildingTypeClassExt::_Free_Buildup_Image);
    Patch_Jump(0x00443D20, &BuildingTypeClassExt::_Set_Base_Defense_Values);

    Patch_Jump(0x0044403B, &_BuildingTypeClass_SDDTOR_Free_Image_Patch);
    Patch_Jump(0x0043FD83, &_BuildingTypeClass_Init_Free_Image_Patch);
    Patch_Jump(0x0043F90B, &_BuildingTypeClass_DTOR_Free_Image_Patch);
    Patch_Jump(0x00444052, &_BuildingTypeClass_SDDTOR_Free_Buildup_Image_Patch);
    Patch_Jump(0x0043FDB0, &_BuildingTypeClass_Init_Free_Buildup_Image_Patch);
    Patch_Jump(0x0043F936, &_BuildingTypeClass_DTOR_Free_Buildup_Image_Patch);
}
