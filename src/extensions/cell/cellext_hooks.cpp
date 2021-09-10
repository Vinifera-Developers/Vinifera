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
#include "rulesext.h"
#include "foot.h"
#include "unit.h"
#include "unittype.h"
#include "techno.h"
#include "technotype.h"
#include "house.h"
#include "session.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "vinifera_globals.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-177
 * 
 *  
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CellClass_Goodie_Check_BaseUnit_Quantity_Patch)
{
    GET_REGISTER_STATIC(FootClass *, object, ebx);
    static UnitTypeClass *unittype;
    static HouseClass *objhouse;
    static UnitType unit;
    static int count;

    objhouse = object->House;

    /**
     *  Fetch the extended rules class instance and make sure value entries
     *  have been loaded.
     */
    if (RuleExtension && RuleExtension->BaseUnit.Count() > 0) {

        /**
         *  Fetch the first buildable base unit from the new base unit entry
         *  and get the current count of that unit that this house owns.
         */
        unittype = objhouse->Get_First_Ownable(RuleExtension->BaseUnit);
        if (unittype) {
            unit = (UnitType)unittype->Get_Heap_ID();
            count = objhouse->UQuantity.Count_Of(unit);
        }

    /**
     *  Fallback to the original code.
     */
    } else {

        /**
         *  Get the current count of the base unit that this house owns.
         */
        unit = (UnitType)Rule->BaseUnit->Get_Heap_ID();
        count = objhouse->UQuantity.Count_Of(unit);
    }

    /**
     *  If no ownable base units were found, continue the force mcv check.
     */
    if (!count) {
        goto continue_check;
    }

    /**
     *  Skip the check.
     */
skip_check:
    JMP_REG(eax, 0x00457DCF);

    /**
     *  Continue check for setting "force mcv".
     */
continue_check:
    JMP_REG(edi, 0x00457DB8);
}


/**
 *  #issue-177
 * 
 *  
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CellClass_Goodie_Check_CRATE_UNIT_BaseUnit_Patch)
{
    GET_REGISTER_STATIC(FootClass *, object, ebx);
    static UnitTypeClass *unittype;
    static HouseClass *objhouse;
    static UnitType unit;

    objhouse = object->House;

    /**
     *  Fetch the extended rules class instance and make sure value entries
     *  have been loaded.
     */
    if (RuleExtension && RuleExtension->BaseUnit.Count() > 0) {

        /**
         *  Fetch the first buildable base unit from the new base unit entry.
         */
        unittype = objhouse->Get_First_Ownable(RuleExtension->BaseUnit);

    /**
     *  Fallback to the original code.
     */
    } else {
        unittype = Rule->BaseUnit;
    }

label_1:
    _asm { mov eax, [0x0074C488] } // Rule
    JMP_REG(edx, 0x00458148);

label_2:
    _asm { mov eax, [0x0074C488] } // Rule
    _asm { mov edi, unittype }
    JMP(0x004581A5);
}


/**
 *  #issue-177
 * 
 *  
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_CellClass_Goodie_Check_No_Buildings_Force_MCV_BaseUnit_Patch)
{
    GET_REGISTER_STATIC(UnitTypeClass *, unittype, edi);
    static int i;

    /**
     *  Fetch the extended rules class instance and make sure value entries
     *  have been loaded.
     */
    if (RuleExtension && RuleExtension->BaseUnit.Count() > 0) {

        /**
         *  Iterate over all defined BaseUnits, if any match, then this
		 *  will take priority over the goodie crate unit chosen prior to
		 *  this case.
         */
		for (i = 0; i < RuleExtension->BaseUnit.Count(); ++i) {
			if (RuleExtension->BaseUnit[i] == unittype) {
				goto found_unit;
			}
		}

    /**
     *  Fallback to the original code.
     */
    } else {
        unittype = Rule->BaseUnit;
    }

found_unit:
    _asm { mov edi, unittype }
    JMP(0x0045821B);

continue_loop:
    _asm { mov edi, unittype }
	JMP(0x004581BA);
}


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
	static const ShapeFileStruct *_shroud_shape;
	static const ShapeFileStruct *_fog_shape;

	/**
	 *  Stolen bytes/code.
	 */
	_asm { sub esp, 0x34 }

	/**
	 *  Perform a one-time load of the shroud and fog shape data.
	 */
	if (!_shroud_one_time) {
		_shroud_shape = (const ShapeFileStruct *)MFCC::Retrieve("SHROUD.SHP");
		_fog_shape = (const ShapeFileStruct *)MFCC::Retrieve("FOG.SHP");
		_shroud_one_time = true;
	}

	/**
	 *  If we are playing a multiplayer game, use the hardcoded shape data.
	 */
	if (!Session.Singleplayer_Game()) {
		Cell_ShroudShape = (const ShapeFileStruct *)&ShroudShapeBinary;
		Cell_FogShape = (const ShapeFileStruct *)&FogShapeBinary;

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
	static const ShapeFileStruct *_fog_shape;
	
	/**
	 *  Stolen bytes/code.
	 */
	_asm { sub esp, 0x2C }
	
	/**
	 *  Perform a one-time load of the fog shape data.
	 */
	if (!_fog_one_time) {
		_fog_shape = (const ShapeFileStruct *)MFCC::Retrieve("FOG.SHP");
		_fog_one_time = true;
	}

	/**
	 *  If we are playing a multiplayer game, use the hardcoded shape data.
	 */
	if (!Session.Singleplayer_Game()) {
		Cell_FixupFogShape = (const ShapeFileStruct *)&FogShapeBinary;

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
	if (techno->Techno_Type_Class()->IsTrainable) {
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
    Patch_Jump(0x00457D90, &_CellClass_Goodie_Check_BaseUnit_Quantity_Patch);
    Patch_Jump(0x0045813E, &_CellClass_Goodie_Check_CRATE_UNIT_BaseUnit_Patch);
    Patch_Jump(0x0045820E, &_CellClass_Goodie_Check_No_Buildings_Force_MCV_BaseUnit_Patch);
}
