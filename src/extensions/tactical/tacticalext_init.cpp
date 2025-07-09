/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTICALEXT_INIT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for initialising the extended Tactical class.
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
#include "tacticalext_hooks.h"
#include "tacticalext.h"
#include "tactical.h"
#include "tibsun_globals.h"
#include "vinifera_util.h"
#include "vinifera_globals.h"
#include "extension.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  Patch for including the extended class members in the creation process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Tactical_Constructor_Patch)
{
    GET_REGISTER_STATIC(Tactical *, this_ptr, esi); // "this" pointer.

    /**
     *  Create the extended class instance.
     */
    TacticalMapExtension = Extension::Singleton::Make<Tactical, TacticalExtension>(this_ptr);

    /**
     *  Stolen bytes here.
     */
original_code:
    _asm { mov eax, this_ptr }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebx }
    _asm { ret }
}


/**
 *  Patch for including the extended class members in the destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Tactical_Destructor_Patch)
{
    GET_REGISTER_STATIC(Tactical *, this_ptr, esi);

    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<Tactical, TacticalExtension>(TacticalMapExtension);

    /**
     *  Stolen bytes here.
     */
original_code:
    this_ptr->AbstractClass::~AbstractClass();
    _asm { ret }
}


/**
 *  Patch for including the extended class members in the virtual destruction process.
 * 
 *  @warning: Do not touch this unless you know what you are doing!
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Tactical_Scalar_Destructor_Patch)
{
    GET_REGISTER_STATIC(Tactical *, this_ptr, esi);

    /**
     *  Remove the extended class instance.
     */
    Extension::Singleton::Destroy<Tactical, TacticalExtension>(TacticalMapExtension);

    /**
     *  Stolen bytes here.
     */
original_code:
    this_ptr->AbstractClass::~AbstractClass();
    JMP(0x0061802F);
}


/**
 *  Main function for patching the hooks.
 */
void TacticalExtension_Init()
{
    Patch_Jump(0x0060F08A, &_Tactical_Constructor_Patch);
    Patch_Jump(0x0060F0E7, &_Tactical_Destructor_Patch);
    Patch_Jump(0x0061802A, &_Tactical_Scalar_Destructor_Patch);
}
