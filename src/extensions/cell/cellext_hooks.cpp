/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          CELLEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended CellClass.
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
#include "cellext_hooks.h"
#include "cellext_const.h"
#include "tibsun_globals.h"
#include "session.h"
#include "rules.h"
#include "iomap.h"
#include "techno.h"
#include "technotype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-381
 * 
 *  Hardcodes shroud and fog to circumvent cheating in multiplayer games.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CellClass_Draw_Shroud_Fog_Patch)
{
	static bool _shroud_one_time = false;
	static const ShapeSet *_shroud_shape;
	static const ShapeSet *_fog_shape;

	/**
	 *  Stolen bytes/code.
	 */
	_asm { sub esp, 0x34 }

	/**
	 *  Perform a one-time load of the shroud and fog shape data.
	 */
	if (!_shroud_one_time) {
		_shroud_shape = (const ShapeSet *)MFCC::Retrieve("SHROUD.SHP");
		_fog_shape = (const ShapeSet *)MFCC::Retrieve("FOG.SHP");
		_shroud_one_time = true;
	}

	/**
	 *  If we are playing a multiplayer game, use the hardcoded shape data.
	 */
	if (!Session.Singleplayer_Game()) {
		Cell_ShroudShape = (const ShapeSet *)&ShroudShapeBinary;
		Cell_FogShape = (const ShapeSet *)&FogShapeBinary;

	} else {
		Cell_ShroudShape = _shroud_shape;
		Cell_FogShape = _fog_shape;
	}

	/**
	 *  Continues function flow.
	 */
continue_function:
	JMP(0x00454E91);
}


/**
 *  #issue-381
 * 
 *  Hardcodes shroud and fog to circumvent cheating in multiplayer games.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CellClass_Draw_Fog_Patch)
{
	static bool _fog_one_time = false;
	static const ShapeSet *_fog_shape;
	
	/**
	 *  Stolen bytes/code.
	 */
	_asm { sub esp, 0x2C }
	
	/**
	 *  Perform a one-time load of the fog shape data.
	 */
	if (!_fog_one_time) {
		_fog_shape = (const ShapeSet *)MFCC::Retrieve("FOG.SHP");
		_fog_one_time = true;
	}

	/**
	 *  If we are playing a multiplayer game, use the hardcoded shape data.
	 */
	if (!Session.Singleplayer_Game()) {
		Cell_FixupFogShape = (const ShapeSet *)&FogShapeBinary;

	} else {
		Cell_FixupFogShape = _fog_shape;
	}

	/**
	 *  Continues function flow.
	 */
continue_function:
	_asm { mov eax, Cell_FixupFogShape }
	_asm { mov eax, [eax] }
	JMP_REG(ecx, 0x00455159);
}


/**
 *  #issue-191
 * 
 *  Fixes a bug where pre-placed crates and crates spawned by a destroyed
 *  truck will trigger a respawn when they are picked up, even when the
 *  Crates game option was disabled.
 * 
 *  @author: CCHyper (based on research by Rampastring)
 */
DECLARE_PATCH(_CellClass_Goodie_Check_Crates_Disabled_Respawn_BugFix_Patch)
{
	/**
	 *  Random crates are only thing in multiplayer.
	 */
	if (Session.Type != GAME_NORMAL) {

		/**
		 *  Check to make sure crates are enabled for this game session.
		 * 
		 *  The original code was missing the Session "Goodies" check.
		 */
		if (Rule->IsMPCrates && Session.Options.Goodies) {
			Map.Place_Random_Crate();
		}
	}

	/**
	 *  Continues function flow.
	 */
continue_function:
	JMP_REG(ecx, 0x00457ECE);
}


/**
 *  #issue-161
 * 
 *  Veterancy crate bonus does not check if a object is un-trainable
 *  before granting it the veterancy bonus.
 * 
 *  @author: CCHyper (based on research by Iran)
 */
DECLARE_PATCH(_CellClass_Goodie_Check_Veterency_Trainable_BugFix_Patch)
{
	GET_REGISTER_STATIC(ObjectClass *, object, ecx);
	static TechnoClass *techno;
	static TechnoTypeClass *technotype;

	/**
	 *  Make sure the ground layer object is a techno.
	 */
	if (!object->Is_Techno()) {
		goto continue_loop;
	}

	/**
	 *  Is this object trainable? If so, grant it the bonus.
	 */
	techno = reinterpret_cast<TechnoClass *>(object);
	if (techno->TClass->IsTrainable) {
		goto passes_check;
	}

	/**
	 *  Continues the loop over the ground layer objects.
	 */
continue_loop:
	JMP(0x0045894E);

	/**
	 *  Continue to grant the veterancy bonus.
	 */
passes_check:
	JMP(0x00458839);
}


/**
 *  Main function for patching the hooks.
 */
void CellClassExtension_Hooks()
{
	Patch_Jump(0x0045882C, &_CellClass_Goodie_Check_Veterency_Trainable_BugFix_Patch);
	Patch_Jump(0x00457EAB, &_CellClass_Goodie_Check_Crates_Disabled_Respawn_BugFix_Patch);
	Patch_Jump(0x00454E60, &_CellClass_Draw_Shroud_Fog_Patch);
	Patch_Jump(0x00455130, &_CellClass_Draw_Fog_Patch);
}
