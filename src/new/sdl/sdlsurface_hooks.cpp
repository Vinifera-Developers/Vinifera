/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SDLSURFACE_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the SDLSurface class.
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
#include "dsurface.h"
#include "hooker.h"
#include "sdlsurface.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class DSurfaceExt : public DSurface
{
public:
    DSurface* CTOR_Proxy(int width, int height, bool system_memory);
};


/**
 *  A function imitating a constructor because we can't take the address of a constructor.
 *
 *  @author: ZivDero
 */
DSurface* DSurfaceExt::CTOR_Proxy(int width, int height, bool system_memory)
{
    return new (this) SDLSurface(width, height);
}


/**
 *  Main function for patching the hooks.
 */
void SDLSurface_Hooks()
{
    Patch_Call(0x0060141E, &SDLSurface::Create_Primary);
    Patch_Call(0x0059C506, &SDLSurface::GetDC);
    Patch_Call(0x0059E063, &SDLSurface::GetDC);
    Patch_Call(0x0059F227, &SDLSurface::GetDC);
    Patch_Call(0x0059C5B8, &SDLSurface::ReleaseDC);
    Patch_Call(0x0059E0C7, &SDLSurface::ReleaseDC);
    Patch_Call(0x0059F30E, &SDLSurface::ReleaseDC);

    Patch_Jump(0x00685A73, 0x00685B67); // Skip Restore_Check calls in Focus_Restore

    Patch_Byte(0x00491587, sizeof(SDLSurface)); // Show_Who_Was_Responsible
    Patch_Byte(0x0056848A, sizeof(SDLSurface)); // MultiScore::Init
    Patch_Byte(0x005AC325, sizeof(SDLSurface)); // MapPreviewClass::Create_Preview
    Patch_Byte(0x005ACA6B, sizeof(SDLSurface)); // MapPreviewClass::Read_INI
    Patch_Byte(0x005ACD43, sizeof(SDLSurface)); // MapPreviewClass::Read_PCX_Preview
    Patch_Byte(0x005AD4C8, sizeof(SDLSurface)); // MapPreviewClass::Create_Preview_Surface
    Patch_Byte(0x005B9CB0, sizeof(SDLSurface)); // RadarClass::Compute_Radar_Image
    Patch_Byte(0x005E304E, sizeof(SDLSurface)); // ScoreClass::Presentation

    Patch_Call(0x004915A5, &DSurfaceExt::CTOR_Proxy); // Show_Who_Was_Responsible
    Patch_Call(0x005684A5, &DSurfaceExt::CTOR_Proxy); // MultiScore::Init
    Patch_Call(0x005AC33C, &DSurfaceExt::CTOR_Proxy); // MapPreviewClass::Create_Preview
    Patch_Call(0x005ACA97, &DSurfaceExt::CTOR_Proxy); // MapPreviewClass::Read_INI
    Patch_Call(0x005ACD66, &DSurfaceExt::CTOR_Proxy); // MapPreviewClass::Read_PCX_Preview
    Patch_Call(0x005AD4DF, &DSurfaceExt::CTOR_Proxy); // MapPreviewClass::Create_Preview_Surface
    Patch_Call(0x005B9CDF, &DSurfaceExt::CTOR_Proxy); // RadarClass::Compute_Radar_Image
    Patch_Call(0x005E307D, &DSurfaceExt::CTOR_Proxy); // ScoreClass::Presentation
}
