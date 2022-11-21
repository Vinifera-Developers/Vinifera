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
#include "bullettype.h"
#include "bullettypeext.h"
#include "bullet.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "iomap.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-563
 * 
 *  Implements SpawnDelay for BulletTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BulletClass_AI_SpawnDelay_Patch)
{
    GET_REGISTER_STATIC(BulletClass *, this_ptr, ebp);
    static BulletTypeClassExtension *bullettypeext;

    /**
     *  Fetch the extension instance.
     */
    bullettypeext = Extension::Fetch<BulletTypeClassExtension>(this_ptr->Class);

    /**
     *  If this bullet has a custom spawn delay (defaults to the original delay of 3), perform that check first.
     */
    if ((Frame % bullettypeext->SpawnDelay) == 0) {
        goto create_trailer_anim;
    }

skip_anim:
    JMP(0x00444801);

create_trailer_anim:
    JMP(0x004447D0);
}


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
     *  Fetch the extension instance.
     */
    warheadext = Extension::Fetch<WarheadTypeClassExtension>(warhead);

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
    Patch_Jump(0x004447BF, &_BulletClass_AI_SpawnDelay_Patch);
}
