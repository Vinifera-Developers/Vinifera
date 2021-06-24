/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BULLETEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended BulletClass.
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
#include "bulletext_hooks.h"
#include "bullet.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "iomap.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-415
 * 
 *  Implements screen shake values for WarheadTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BulletClass_Logic_ShakeScreen_Patch)
{
    GET_REGISTER_STATIC(BulletClass *, this_ptr, ebx);
    GET_REGISTER_STATIC(WarheadTypeClass *, warhead, eax);
    GET_STACK_STATIC(Coordinate *, coord, esp, 0x0A8);
    static WarheadTypeClassExtension *warheadext;

    /**
     *  Fetch the extended warhead type instance if it exists.
     */
    warheadext = WarheadTypeClassExtensions.find(warhead);
    if (warheadext) {

        /**
         *  If this warhead has screen shake values defined, then set the blitter
         *  offset values. GScreenClass::Blit will handle the rest for us.
         */
        if (warheadext->ShakePixelXLo > 0 || warheadext->ShakePixelXHi > 0) {
            Map.ScreenX = Sim_Random_Pick(warheadext->ShakePixelXLo, warheadext->ShakePixelXHi);
        }
        if (warheadext->ShakePixelYLo > 0 || warheadext->ShakePixelYHi > 0) {
            Map.ScreenY = Sim_Random_Pick(warheadext->ShakePixelYLo, warheadext->ShakePixelYHi);
        }
    }

    /**
     *  Restore some registers.
     */
    _asm { mov eax, warhead }
    _asm { mov edi, coord /*[esp+0x0A8]*/ } // coord

    /**
     *  Jumps back to IsEMEffect check.
     */
    JMP_REG(edx, 0x00446659);
}


/**
 *  Main function for patching the hooks.
 */
void BulletClassExtension_Hooks()
{
    Patch_Jump(0x00446652, &_BulletClass_Logic_ShakeScreen_Patch);
}
