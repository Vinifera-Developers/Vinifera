/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          RADAREXT_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the extended RadarClass.
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
#include "radarext_hooks.h"

#include "bsurface.h"
#include "extension_globals.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "optionsext.h"
#include "radar.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class RadarClassExt : public RadarClass
{
public:
    void _One_Time();
    void _Make_Surface(Rect const& rect, int& width, int& height);
    void _Center_Radar_Rect();
};

void RadarClassExt::_One_Time()
{
    DEBUG_INFO("RadarClass::One_Time()\n");
    RadX = 0;
    RadY = OptionsExtension->SidebarControls.TabHeight + OptionsExtension->SidebarControls.RadarTopHeight;
    RadWidth = SidebarSurface->Get_Width();
    RadHeight = OptionsExtension->SidebarControls.RadarHeight;
    RadOffX = OptionsExtension->SidebarControls.RadarMapRect.X;
    RadOffY = OptionsExtension->SidebarControls.RadarMapRect.Y;
    RadPWidth = OptionsExtension->SidebarControls.RadarMapRect.Width;
    RadPHeight = OptionsExtension->SidebarControls.RadarMapRect.Height;
    RadIWidth = OptionsExtension->SidebarControls.RadarMapRect.Width;
    RadIHeight = OptionsExtension->SidebarControls.RadarMapRect.Height;

    DisplayClass::One_Time();

    RadarButton.X = RadX + SidebarRect.X;
    RadarButton.Y = RadY;
    RadarButton.Width = RadWidth;
    RadarButton.Height = RadHeight;
    RadarButton.Set_Flags(GadgetClass::FlagEnum::LEFTPRESS | GadgetClass::FlagEnum::LEFTHELD | GadgetClass::FlagEnum::LEFTRELEASE | GadgetClass::FlagEnum::LEFTUP | GadgetClass::FlagEnum::RIGHTPRESS | GadgetClass::FlagEnum::RIGHTRELEASE | GadgetClass::FlagEnum::RIGHTUP);
}

void RadarClassExt::_Make_Surface(Rect const& rect, int& width, int& height)
{
    if (field_1228 == nullptr) {
        float scale = static_cast<float>(RadPWidth) / rect.Width;
        float scaled_height = rect.Height * scale;
        if (scaled_height < RadPHeight) {
            width = RadPWidth;
            height = scaled_height;
        } else {
            scale = static_cast<float>(RadPHeight) / rect.Height;
            float scaled_width = rect.Width * scale;
            width = scaled_width;
            height = RadPHeight;
        }
        ZoomFactor = scale;
        field_1228 = new BSurface(width, height, 2);
        field_1228->Clear();
    } else {
        width = field_1228->Get_Width();
        height = field_1228->Get_Height();
    }
}

void RadarClassExt::_Center_Radar_Rect()
{
    RadarRect.X = RadX + RadOffX;
    if (RadarRect.Width < RadPWidth) {
        RadarRect.X += (RadPWidth - RadarRect.Width) / 2;
    }

    RadarRect.Y = RadY + RadOffY;
    if (RadarRect.Height < RadPHeight) {
        RadarRect.Y += (RadPHeight - RadarRect.Height) / 2;
    }
}


DECLARE_PATCH(_RadarClass_func_5B9D10_Surface_Size_Patch)
{
    GET_REGISTER_STATIC(Rect*, rect, ebp);
    LEA_STACK_STATIC(int*, width, esp, 0x84);
    LEA_STACK_STATIC(int*, height, esp, 0x10);
    GET_REGISTER_STATIC(RadarClassExt*, this_ptr, ebx);

    this_ptr->_Make_Surface(*rect, *width, *height);
    JMP(0x005B9F08);
}


DECLARE_PATCH(_RadarClass_Compute_Radar_Image_Center_Patch)
{
    GET_REGISTER_STATIC(RadarClassExt*, this_ptr, esi);

    this_ptr->_Center_Radar_Rect();
    JMP(0x005B9C99);
}


/**
 *  Main function for patching the hooks.
 */
void RadarClassExtension_Hooks()
{
    Patch_Jump(0x005B8B90, &RadarClassExt::_One_Time);
    Patch_Jump(0x005B9DE1, &_RadarClass_func_5B9D10_Surface_Size_Patch);
    Patch_Jump(0x005B9C48, &_RadarClass_Compute_Radar_Image_Center_Patch);
}