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
#include "dsurface.h"
#include "hooker.h"
#include "sdlmouse.h"
#include "sdlsurface.h"
#include "wwmouse.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class WWMouseClassExt : public WWMouseClass
{
public:
    SDLMouseClass* CTOR_Proxy(Surface*, HWND);
};


/**
 *  A function imitating a constructor because we can't take the address of a constructor.
 *
 *  @author: ZivDero
 */
SDLMouseClass* WWMouseClassExt::CTOR_Proxy(Surface*, HWND)
{
    return new (reinterpret_cast<SDLMouseClass*>(this)) SDLMouseClass;
}


/**
 *  Main function for patching the hooks.
 */
void SDLMouse_Hooks()
{
    Patch_Byte(0x00601857, sizeof(SDLMouseClass));
    Patch_Call(0x00601877, &WWMouseClassExt::CTOR_Proxy);
}
