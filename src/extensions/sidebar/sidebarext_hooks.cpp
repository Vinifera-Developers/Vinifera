/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAREXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended SidebarClass.
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
#include "sidebarext_hooks.h"
#include "tibsun_globals.h"
#include "sidebar.h"
#include "technotype.h"
#include "technotypeext.h"
#include "supertype.h"
#include "supertypeext.h"
#include "spritecollection.h"
#include "bsurface.h"
#include "drawshape.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


static const ObjectTypeClass *_SidebarClass_StripClass_obj = nullptr;
static const SuperWeaponTypeClass *_SidebarClass_StripClass_spc = nullptr;
static BSurface *_SidebarClass_StripClass_CustomImage = nullptr;


/**
 *  #issue-487
 * 
 *  Adds support for PCX/PNG cameo icons.
 * 
 *  The following two patches store the PCX/PNG image for the factory object or special.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_SidebarClass_StripClass_ObjectTypeClass_Custom_Cameo_Image_Patch)
{
    GET_REGISTER_STATIC(const ObjectTypeClass *, obj, ebp);
    static const TechnoTypeClassExtension *technotypeext;
    static const ShapeFileStruct *shapefile;

    shapefile = obj->Get_Cameo_Data();

    _SidebarClass_StripClass_obj = obj;
    _SidebarClass_StripClass_CustomImage = nullptr;

    technotypeext = Extension::Fetch<TechnoTypeClassExtension>(reinterpret_cast<const TechnoTypeClass *>(obj));
    if (technotypeext->CameoImageSurface) {
        _SidebarClass_StripClass_CustomImage = technotypeext->CameoImageSurface;
    }

    _asm { mov eax, shapefile }

    JMP_REG(ebx, 0x005F5193);
}

DECLARE_PATCH(_SidebarClass_StripClass_SuperWeaponType_Custom_Cameo_Image_Patch)
{
    GET_REGISTER_STATIC(const SuperWeaponTypeClass *, supertype, eax);
    static const SuperWeaponTypeClassExtension *supertypeext;
    static const ShapeFileStruct *shapefile;

    shapefile = supertype->SidebarIcon;

    _SidebarClass_StripClass_spc = supertype;
    _SidebarClass_StripClass_CustomImage = nullptr;

    supertypeext = Extension::Fetch<SuperWeaponTypeClassExtension>(supertype);
    if (supertypeext->CameoImageSurface) {
        _SidebarClass_StripClass_CustomImage = supertypeext->CameoImageSurface;
    }

    _asm { mov ebx, shapefile }

    JMP(0x005F5220);
}


/**
 *  #issue-487
 * 
 *  Adds support for PCX/PNG cameo icons.
 * 
 *  @author: CCHyper
 */
static Point2D pointxy;
static Rect pcxrect;
DECLARE_PATCH(_SidebarClass_StripClass_Custom_Cameo_Image_Patch)
{
    GET_STACK_STATIC(SidebarClass::StripClass *, this_ptr, esp, 0x24);
    LEA_STACK_STATIC(Rect *, window_rect, esp, 0x34);
    GET_REGISTER_STATIC(int, pos_x, edi);
    GET_REGISTER_STATIC(int, pos_y, esi);
    GET_REGISTER_STATIC(const ShapeFileStruct *, shapefile, ebx);
    static BSurface *image_surface;

    image_surface = nullptr;

    /**
     *  Was a factory object or special image found?
     */
    if (_SidebarClass_StripClass_CustomImage) {
        image_surface = _SidebarClass_StripClass_CustomImage;
    }

    /**
     *  Draw the cameo pcx image.
     */
    if (image_surface) {
        pcxrect.X = window_rect->X + pos_x;
        pcxrect.Y = window_rect->Y + pos_y;
        pcxrect.Width = image_surface->Get_Width();
        pcxrect.Height = image_surface->Get_Height();

        SpriteCollection.Draw(pcxrect, *SidebarSurface, *image_surface);

    /**
     *  Draw shape cameo image.
     */
    } else if (shapefile) {
        pointxy.X = pos_x;
        pointxy.Y = pos_y;

        CC_Draw_Shape(SidebarSurface, CameoDrawer, shapefile, 0, &pointxy, window_rect, SHAPE_WIN_REL|SHAPE_NORMAL);
    }

    _SidebarClass_StripClass_CustomImage = nullptr;

    /**
     *  Next, draw the clock darken shape.
     */
draw_darken_shape:
    JMP(0x005F52F3);
}


/**
 *  Main function for patching the hooks.
 */
void SidebarClassExtension_Hooks()
{
    Patch_Jump(0x005F5188, &_SidebarClass_StripClass_ObjectTypeClass_Custom_Cameo_Image_Patch);
    Patch_Jump(0x005F5216, &_SidebarClass_StripClass_SuperWeaponType_Custom_Cameo_Image_Patch);
    Patch_Jump(0x005F52AF, &_SidebarClass_StripClass_Custom_Cameo_Image_Patch);
}
