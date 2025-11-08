/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          OVERLAYTYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended OverlayTypeClass.
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
#include "overlaytypeext_hooks.h"
#include "overlaytypeext_init.h"
#include "overlaytypeext.h"
#include "overlaytype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "syringe.h"


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
static void OverlayTypeClass_Free_Image(OverlayTypeClass *this_ptr)
{
    if (this_ptr->IsDemandLoad && this_ptr->Image) {
        DEV_DEBUG_WARNING("Overlay: Freeing loaded image for %s\n", this_ptr->Name());

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
EXPORT_FUNC(_OverlayTypeClass_DTOR_Free_Image_Patch)
{
    GET(OverlayTypeClass*, this_ptr, ESI);
    OverlayTypeClass_Free_Image(this_ptr);
    return 0x0058D192;
}

EXPORT_FUNC(_OverlayTypeClass_SDDTOR_Free_Image_Patch)
{
    GET(OverlayTypeClass*, this_ptr, ESI);
    OverlayTypeClass_Free_Image(this_ptr);
    return 0x0058DC82;
}


/**
 *  Main function for patching the hooks.
 */
void OverlayTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    OverlayTypeClassExtension_Init();
}

declhook(0x0058D17B, _OverlayTypeClass_DTOR_Free_Image_Patch, 0);
declhook(0x0058DC6B, _OverlayTypeClass_SDDTOR_Free_Image_Patch, 0);
