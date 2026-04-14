/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SIDEBAR_RENDER_UTILS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Shared sidebar rendering utilities implementation.
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

#include "sidebar_render_utils.h"

#include "sidebar_model.h"

#include "bsurface.h"
#include "colorscheme.h"
#include "drawshape.h"
#include "dsurface.h"
#include "extension.h"
#include "house.h"
#include "language.h"
#include "mouse.h"
#include "sidebar.h"
#include "sideext.h"
#include "super.h"
#include "spritecollection.h"
#include "supertype.h"
#include "supertypeext.h"
#include "technotypeext.h"
#include "textprint.h"
#include "tibsun_defines.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"

#include <cstdio>


/**
 *  Draws a cameo icon for a build item. Supports PCX/PNG custom
 *  cameo images as well as standard shape-based cameos. Ported from
 *  the old sidebar hook patches.
 *
 *  @author: ZivDero, CCHyper
 */
void Draw_Cameo(Surface& surface, const Rect& rect, const BuildItem& item, const Point2D& point)
{
    const ShapeSet* shapefile = nullptr;
    BSurface* image_surface = nullptr;

    if (item.Type != RTTI_SPECIAL) {
        const TechnoTypeClass* obj = Fetch_Techno_Type(item.Type, item.ID);
        if (obj != nullptr) {
            shapefile = obj->Get_Cameo_Data();
            auto* technoext = Extension::Fetch(obj);
            if (technoext->CameoImageSurface != nullptr) {
                image_surface = technoext->CameoImageSurface;
            }
        } else {
            shapefile = SidebarClass::StripClass::LogoShape;
        }
    } else {
        SuperWeaponType spc = static_cast<SuperWeaponType>(item.ID);
        shapefile = Map.Column[0].Get_Special_Cameo(spc);
        auto* supertypeext = Extension::Fetch(PlayerPtr->SuperWeapon[spc]->Class);
        if (supertypeext->CameoImageSurface != nullptr) {
            image_surface = supertypeext->CameoImageSurface;
        }

        if (spc == SUPER_NONE) {
            shapefile = SidebarClass::StripClass::LogoShape;
        }
    }

    if (image_surface != nullptr) {
        Rect pcxrect(rect.X + point.X, rect.Y + point.Y, image_surface->Get_Width(), image_surface->Get_Height());
        SpriteCollection.Draw(pcxrect, surface, *image_surface);
    } else if (shapefile != nullptr) {
        Draw_Shape(surface, *CameoDrawer, shapefile, 0, point, rect, SHAPE_WIN_REL);
    }
}


/**
 *  Draws the green production clock overlay at the given stage.
 *
 *  @author: ZivDero
 */
void Draw_Clock_Overlay(Surface& surface, ConvertClass& drawer, const ShapeSet* shape, const Rect& rect, const Point2D& point, int stage)
{
    if (shape != nullptr) {
        Draw_Shape(surface, drawer, shape, stage + 1, point, rect, SHAPE_WIN_REL | SHAPE_TRANS50);
    }
}


/**
 *  Draws the superweapon recharge clock overlay at the given stage.
 *
 *  @author: ZivDero
 */
void Draw_Recharge_Clock(Surface& surface, ConvertClass& drawer, const ShapeSet* shape, const Rect& rect, const Point2D& point, int stage)
{
    if (shape != nullptr) {
        Draw_Shape(surface, drawer, shape, stage + 1, point, rect, SHAPE_WIN_REL | SHAPE_TRANS50);
    }
}


/**
 *  Draws the darkened overlay for unavailable items.
 *
 *  @author: ZivDero
 */
void Draw_Darken_Overlay(Surface& surface, ConvertClass& drawer, const ShapeSet* shape, const Rect& rect, const Point2D& point)
{
    if (shape != nullptr) {
        Draw_Shape(surface, drawer, shape, 0, point, rect, SHAPE_WIN_REL | SHAPE_DARKEN);
    }
}


/**
 *  Draws production state text (e.g. "READY") on a cameo.
 *
 *  @author: ZivDero
 */
void Draw_Ready_Text(Surface& surface, const Rect& rect, const Point2D& point, const char* text, int object_width)
{
    if (text == nullptr) {
        return;
    }

    Point2D drawpoint(point.X + object_width / 2, point.Y);
    Fancy_Text_Print(text, surface, rect, drawpoint, Fetch_Scheme_By_Name("LightBlue", 1), COLOR_TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_8POINT);
}


/**
 *  Draws "HOLD" text on a paused production item.
 *
 *  @author: ZivDero
 */
void Draw_Hold_Text(Surface& surface, const Rect& rect, const Point2D& point, int object_width, bool has_queue_count)
{
    if (has_queue_count) {
        Fancy_Text_Print(TXT_HOLD, surface, rect, point, Fetch_Scheme_By_Name("LightGrey", 1), COLOR_TBLACK, TPF_FULLSHADOW | TPF_8POINT);
    } else {
        Point2D centered(point.X + object_width / 2, point.Y);
        Fancy_Text_Print(TXT_HOLD, surface, rect, centered, Fetch_Scheme_By_Name("LightGrey", 1), COLOR_TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_8POINT);
    }
}


/**
 *  Draws the queue count number on a production item.
 *
 *  @author: ZivDero
 */
void Draw_Queue_Count(Surface& surface, const Rect& rect, const Point2D& point, int count)
{
    Fancy_Text_Print("%d", surface, rect, point, Fetch_Scheme_By_Name("LightGrey", 1), COLOR_TBLACK, TPF_RIGHT | TPF_FULLSHADOW | TPF_8POINT, count);
}


/**
 *  Draws the hover highlight rectangle around a cameo.
 *
 *  @author: ZivDero
 */
void Draw_Hover_Highlight(Surface& surface, const Rect& cameo_rect)
{
    const ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->UIColor;
    surface.Draw_Rect(cameo_rect, DSurface::Build_Hicolor_Pixel(ColorSchemes[colorschemetype]->HSV.operator RGBClass()));
}


/**
 *  Draws the cameo name text below the icon.
 *
 *  @author: ZivDero
 */
void Draw_Cameo_Name(const Rect& rect, const Point2D& point, const char* name, int object_width)
{
    if (name != nullptr) {
        Print_Cameo_Text(name, point, rect, object_width);
    }
}


/**
 *  Formats sidebar tooltip text for a build item. Ported from the
 *  extended sidebar tooltip hook.
 *
 *  @author: ZivDero, Rampastring
 */
const char* Format_Cameo_Tooltip(const BuildItem& item)
{
    static char buffer[512];

    if (item.Type == RTTI_SPECIAL) {
        const SuperWeaponTypeClass* swtype = SuperWeaponTypes[item.ID];
        if (swtype == nullptr) {
            return nullptr;
        }

        const SuperWeaponTypeClassExtension* swtypeext = Extension::Fetch(swtype);
        const char* description = swtypeext->Description;

        if (description[0] == '\0') {
            return swtype->Full_Name();
        }

        std::snprintf(buffer, sizeof(buffer), "%s@@%s", swtype->Full_Name(), description);
        return buffer;
    }

    const TechnoTypeClass* ttype = Fetch_Techno_Type(item.Type, item.ID);
    if (ttype == nullptr) {
        return nullptr;
    }

    const TechnoTypeClassExtension* technotypeext = Extension::Fetch(ttype);
    const char* description = technotypeext->Description;

    if (description[0] == '\0') {
        std::snprintf(buffer, sizeof(buffer), "%s@$%d", ttype->Full_Name(), ttype->Cost_Of(PlayerPtr));
    } else {
        std::snprintf(buffer, sizeof(buffer), "%s@$%d@@%s", ttype->Full_Name(), ttype->Cost_Of(PlayerPtr), description);
    }

    return buffer;
}
