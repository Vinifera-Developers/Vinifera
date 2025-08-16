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
#pragma once
#include "dsurface.h"
#include "hooker.h"
#include "sdlsurface.h"


class DSurfaceExt : public DSurface
{
public:
    DSurface* CTOR_Proxy(int width, int height, bool system_memory);
};


DSurface* DSurfaceExt::CTOR_Proxy(int width, int height, bool system_memory)
{
    return reinterpret_cast<DSurface*>(new (this) SDLSurface(width, height));
}


/**
 *  Main function for patching the hooks.
 */
void SDLSurface_Hooks()
{
    Patch_Jump(0x0048AD60, &SDLSurface::Create_Primary);
    Patch_Jump(0x0048B510, &SDLSurface::Restore_Check);
    Patch_Jump(0x0048B2E0, &SDLSurface::GetDC);
    Patch_Jump(0x0048B320, &SDLSurface::ReleaseDC);

    Patch_Byte(0x0056848A, sizeof(SDLSurface)); // MultiScore::Init
    Patch_Byte(0x005AC325, sizeof(SDLSurface)); // PreviewClass::Create_Preview
    Patch_Byte(0x005ACA6B, sizeof(SDLSurface)); // PreviewClass::Read_INI
    Patch_Byte(0x005ACD43, sizeof(SDLSurface)); // PreviewClass::Read_PCX_Preview
    Patch_Byte(0x005AD4C8, sizeof(SDLSurface)); // PreviewClass::Create_Preview_Surface
    Patch_Byte(0x005ACA6B, sizeof(SDLSurface)); // RadarClass::Compute_Radar_Image
    Patch_Byte(0x005E304E, sizeof(SDLSurface)); // ScoreClass::Presentation

    Patch_Jump(0x0048ABB0, &DSurfaceExt::CTOR_Proxy);
}