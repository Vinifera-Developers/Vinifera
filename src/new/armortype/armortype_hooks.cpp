/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          THEATERTYPE_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for TheaterTypeClass.
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
#include "armortype_hooks.h"
#include "armortype.h"
#include "vinifera_globals.h"
#include "tibsun_defines.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "weapontype.h"
#include "techno.h"
#include "technotype.h"
#include "wwmath.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


//#define GET_VERSES(reg_wh, reg_armor) \
//    GET(WarheadTypeClass *, WH, reg_wh); \
//    GET(int, Armor, reg_armor); \
//    WarheadTypeExt::ExtData *pData = WarheadTypeExt::ExtMap.Find(WH); \
//    WarheadTypeExt::VersesData *vsData = &pData->Verses[Armor];

//#define FLD_VERSES(reg_wh, reg_armor) \
//    GET_VERSES(reg_wh, reg_armor) \
//    double VS = vsData->Verses; \
//    __asm{ fld VS }; \
//    return R->Origin() + 7;

//#define FMUL_VERSES(reg_wh, reg_armor) \
//    GET_VERSES(reg_wh, reg_armor) \
//    double VS = vsData->Verses; \
//    __asm{ fmul VS }; \
//    return R->Origin() + 7;




/**
 *  x
 *
 *  @author: CCHyper
 */
//DECLARE_PATCH(_TechnoClass_Is_Allowed_To_Retaliate_Ext_Verses_Patch)
//{
//    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
//    GET_REGISTER_STATIC(WeaponTypeClass *, weapon, edi);
//    static ArmorTypeClass *atype;
//
//    /**
//     *  
//     */
//    //atype = Get_ArmorType(this_ptr, weapon->WarheadPtr);
//    if (!atype->Is_Allowed_To_Retaliate()) {
//        goto not_allowed;
//    }
//
//    JMP(0x00637019);
//
//not_allowed:
//    JMP(0x006371E7);
//}
//DEFINE_HOOK(708AF7, Verses_fld_2, 0) // TechnoClass::Is_Allowed_To_Retaliate TS 00636FFC
//{
//    GET_VERSES(ECX, EAX);
//    return vsData->Retaliate
//        ? 0x708B0B
//        : 0x708B17
//        ;
//}


//DECLARE_PATCH()
//{
//}
//DEFINE_HOOK(6FCB6A, Verses_fld_3, 0) // TechnoClass__Can_Fire
//{
//    GET_VERSES(EDI, EAX);
//    return vsData->ForceFire
//        ? 0x6FCB8D
//        : 0x6FCB7E
//        ;
//}


//DECLARE_PATCH()
//{
//}
//DEFINE_HOOK(6F7D3D, Verses_fld_4, 0) // TechnoClass::Evaluate_Object
//{
//    GET_VERSES(ECX, EAX);
//    return vsData->PassiveAcquire
//        ? 0x6F7D55
//        : 0x6F894F
//        ;
//}


//DECLARE_PATCH()
//{
//}
//DEFINE_HOOK(70CEB2, Verses_fmul_0, 7) // TechnoClass__Coefficient_stuff_70CD10
//{
//    FMUL_VERSES(EAX, ECX);
//}


//DECLARE_PATCH()
//{
//}
//DEFINE_HOOK(70CEC7, Verses_fmul_1, 7) // TechnoClass__Coefficient_stuff_70CD10
//{
//    FMUL_VERSES(EAX, EDX);
//}


//DECLARE_PATCH()
//{
//}
//DEFINE_HOOK(70CF49, Verses_fmul_2, 7) // TechnoClass__Coefficient_stuff_70CD10
//{
//    FMUL_VERSES(ECX, EAX);
//}



//DEFINE_HOOK(48923D, Verses_fmul_3, 7) // Modify_Damage end TS 0045EBA7
//{
//    FMUL_VERSES(EDI, EDX);
//}





/**
 *  Main function for patching the hooks.
 */
void ArmorTypeClassExtension_Hooks()
{
    //Patch_Jump(0x00636FF1, &_TechnoClass_Is_Allowed_To_Retaliate_Ext_Verses_Patch);
}
