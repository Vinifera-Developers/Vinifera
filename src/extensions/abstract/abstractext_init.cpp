/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          ABSTRACTEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended AbstractClass.
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

#include "abstractext_init.h"

#include "abstract.h"
#include "asserthandler.h"
#include "extension.h"
#include "hooker.h"
#include "syringe.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(AbstractClass)
{
public:
    IFACEMETHOD_(LONG, IsDirty)();
};


/**
 *  This patch forces AbstractClass::IsDirty() to return true.
 * 
 *  @author: CCHyper
 */
LONG STDMETHODCALLTYPE AbstractClassExt::IsDirty()
{
    return TRUE;
}


/**
 *  This patch clears the DWORD at 0x10 (0x10 is "bool IsDirty") to use the space
 *  for storing a pointer to the extension class instance for this AbstractClass.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00405B61, _AbstractClass_Constructor_Extension, 6)
{
    GET(AbstractClass*, this_ptr, EAX);

    // IsDirty, now reused as an extension pointer, so we need to clear the whole DWORD.
    reinterpret_cast<int&>(this_ptr->Dirty) = 0;

    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void AbstractClassExtension_Init()
{
    Patch_Jump(0x00405E00, &AbstractClassExt::Is_Dirty);

    /**
     *  Removes the branch from AbstractClass::Internal_Save which clears IsDirty.
     */
    Patch_Byte_Range(0x00405CF8, 0x90, 12);
}
