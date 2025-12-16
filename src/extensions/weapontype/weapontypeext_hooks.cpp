/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          WEAPONTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended WeaponTypeClass.
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
#include "weapontypeext_hooks.h"
#include "weapontypeext_init.h"
#include "weapontypeext.h"
#include "weapontype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "animtype.h"
#include "findmake.h"
#include "syringe.h"


/**
 *  #issue-391
 *
 *  Expands the buffer size used to read the AnimType list.
 *
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00680F07, _WeaponTypeClass_Read_INI_Get_AnimTypes_Patch, 0)
{
    GET(WeaponTypeClass*, this_ptr, ESI);
    GET(CCINIClass*, ini, EBX);
    GET(const char*, ini_name, EDI);

    /**
     *  Load the AnimType list.
     */
    this_ptr->Anim = TGet_TypeList(*ini, ini_name, "Anim", this_ptr->Anim);

    return 0x00681004;
}


/**
 *  Main function for patching the hooks.
 */
void WeaponTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    WeaponTypeClassExtension_Init();
}
