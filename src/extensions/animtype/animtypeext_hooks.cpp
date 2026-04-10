/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ANIMTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended AnimTypeClass.
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

#include "always.h"

#include "animtypeext_hooks.h"

#include "animtype.h"
#include "animtypeext.h"
#include "animtypeext_init.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "extension.h"
#include "hooker.h"
#include "supertype.h"
#include "syringe.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(AnimTypeClass)
{
public:
    void _Free_Image();
    void _Load_Image(TheaterType theater);
};


/**
 *  Reimplementation of AnimTypeClass::Free_Image.
 *
 *  @author: CCHyper
 */
void AnimTypeClassExt::_Free_Image()
{
    if (IsDemandLoad && Image) {

        if (IsFreeAfterPlaying) {
            DEV_DEBUG_WARNING("Anim: Freeing loaded image for %s\n", Name());

            /**
             *  The original function would incorrectly try to free memory
             *  that the game does not actually allocate, and as a result of
             *  this, Vinifera's new memory management triggers an assertion
             *  because this is no longer allowed. The original game silently
             *  failed when doing this.
             * 
             *  We now remove this and just correctly nullify the pointer.
             */
             //delete Image;

            Image = nullptr;
        }
    }
}


/**
 *  Reimplementation of AnimTypeClass::Load_Image.
 *
 *  @author: ZivDero
 */
void AnimTypeClassExt::_Load_Image(TheaterType theater)
{
    if (!IsDemandLoad && Image == nullptr) {
        if (IsTheater) {
            Fetch_Normal_Image();
        } else {
            char fullname[_MAX_FNAME + _MAX_EXT];
            _makepath(fullname, nullptr, nullptr, Graphic_Name(), ".SHP");
            Theater_Naming_Convention(fullname, theater);
            Image = static_cast<ShapeSet const*>(MixFileClass::Retrieve(fullname));
        }
    }

    /**
     *  The game would calculate Stages and LoopEnd now, set them to -1
     *  instead to be calcalated in AnimClass::AI.
     */
    if (Stages == 0) {
        Stages = -1;
    }
    if (LoopEnd == 0) {
        LoopEnd = -1;
    }

    /**
     *  No longer important as we use the MiddleFrames type list now.
     */
    Biggest = -1;
}


/**
 *  Write to the debug log when freeing up pre-loaded buildup images.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004187DB, _AnimTypeClass_DTOR_Free_Image_Patch, 0)
{
    GET(AnimTypeClass*, this_ptr, ESI);
    this_ptr->Free_Image();
    return 0x004187F2;
}

DEFINE_HOOK(0x00419C0B, _AnimTypeClass_SDDTOR_Free_Image_Patch, 0)
{
    GET(AnimTypeClass*, this_ptr, ESI);
    this_ptr->Free_Image();
    return 0x00419C22;
}


/**
 *  Main function for patching the hooks.
 */
void AnimTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    AnimTypeClassExtension_Init();

    Patch_Jump(0x00419B40, &AnimTypeClassExt::_Free_Image);
    Patch_Jump(0x00418A70, &AnimTypeClassExt::_Load_Image);
}
