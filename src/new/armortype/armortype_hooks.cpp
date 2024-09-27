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



//DEFINE_HOOK(48923D, Verses_fmul_3, 7) // Modify_Damage end TS 0045EBA7
//{
//    FMUL_VERSES(EDI, EDX);
//}





/**
 *  Main function for patching the hooks.
 */
void ArmorTypeClassExtension_Hooks()
{

}
