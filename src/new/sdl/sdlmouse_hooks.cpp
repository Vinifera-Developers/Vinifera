/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SDLMOUSE_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the SDLMouse class.
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
#include "sdlmouse.h"
#include "sdlsurface.h"
#include "wwmouse.h"


class WWMouseClassExt : public WWMouseClass
{
public:
    SDLMouseClass* CTOR_Proxy(Surface*, HWND);
    void _Calc_Confining_Rect() {}
};

SDLMouseClass* WWMouseClassExt::CTOR_Proxy(Surface*, HWND)
{
    return new (reinterpret_cast<SDLMouseClass*>(this)) SDLMouseClass;
}


/**
 *  Main function for patching the hooks.
 */
void SDLMouse_Hooks()
{
    Patch_Call(0x0050B12D, &WWMouseClassExt::_Calc_Confining_Rect);
    Patch_Call(0x00685DB9, &WWMouseClassExt::_Calc_Confining_Rect);

    Patch_Byte(0x0050AD6F, sizeof(SDLMouseClass));
    Patch_Byte(0x0050B0A9, sizeof(SDLMouseClass));
    Patch_Byte(0x00601857, sizeof(SDLMouseClass));

    Patch_Call(0x0050AD8F, &WWMouseClassExt::CTOR_Proxy);
    Patch_Call(0x0050B0C9, &WWMouseClassExt::CTOR_Proxy);
    Patch_Call(0x00601877, &WWMouseClassExt::CTOR_Proxy);
}