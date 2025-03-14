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

#include "anim.h"
#include "cellext_const.h"
#include "tibsun_globals.h"
#include "session.h"
#include "rules.h"
#include "iomap.h"
#include "techno.h"
#include "technotype.h"
#include "overlaytype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "combat.h"
#include "coord.h"
#include "extension_globals.h"
#include "foot.h"
#include "tibsun_globals.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "housetype.h"
#include "logic.h"
#include "sidebarext.h"
#include "super.h"
#include "tag.h"
#include "technoext.h"
#include "unit.h"
#include "utracker.h"
#include "voc.h"
#include "vox.h"
#include "warheadtypeext.h"


/**
  *  A fake class for implementing new member functions which allow
  *  access to the "this" pointer of the intended class.
  *
  *  @note: This must not contain a constructor or destructor!
  *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
  */
static class CellClassExt final : public CellClass
{
public:
    bool _Goodie_Check(FootClass* object);
};


bool CellClassExt::_Goodie_Check(FootClass* object)
{
    if (object != nullptr && Overlay != OVERLAY_NONE && OverlayTypeClass::As_Reference(Overlay).IsCrate) {
        const TechnoClassExtension* object_ext = Extension::Fetch<TechnoClassExtension>(object);

        if (Session.Type == GAME_NORMAL || !object->House->Class->IsMultiplayPassive) {
            bool force_mcv = false;

            if (OverlayTypeClass::As_Reference(Overlay).IsCrateTrigger && object->Tag) {
                DEBUG_INFO("Springing trigger on crate at %d,%d\n", Pos.X, Pos.Y);
                object->Tag->Spring(TEVENT_PICKUP_CRATE, object);

                if (!object->IsActive) {
                    return false;
                }

                Scen->CratePickedUp = true;
            }

            /**
             *  Pick a random crate powerup according to the shares allotted to each powerup.
             *  In solo play, the bonus item is dependant upon the rules control.
             */
            CrateTypeClass* powerup = CrateTypes[CrateTypeClass::From_Name("Money")];
            if (Session.Type == GAME_NORMAL) {

                if (OverlayTypes[Overlay] == Rule->SteelCrateImage) {
                    powerup = CrateTypes[Rule->SilverCrate];
                }

                if (OverlayTypes[Overlay] == Rule->WoodCrateImage) {
                    powerup = CrateTypes[Rule->WoodCrate];
                }
            }
            else {

                /**
                 *  Determine the total number of shares for all the crate powerups. This is used as
                 *  the base pool to determine the odds from.
                 */
                int total_shares = 0;
                DynamicVectorClass<CrateTypeClass*> crates;

                for (int index = CRATE_FIRST; index < CrateTypes.Count(); index++) {
                    CrateTypeClass* crate = CrateTypes[index];
                    switch (crate->Effect)
                    {
                    case CRATE_EFFECT_UNIT:
                        if (crate->Data.Unit.MaxUnits > 0 && object->House->CurUnits > crate->Data.Unit.MaxUnits) continue;
                        break;

                    case CRATE_EFFECT_SQUAD:
                        if (crate->Data.Squad.MaxInfantry > 0 && object->House->CurInfantry > crate->Data.Squad.MaxInfantry) continue;
                        break;

                    case CRATE_EFFECT_ARMOR:
                        if (crate->Data.Armor.MaxStacking > 0 && object_ext->ArmorCrates >= crate->Data.Armor.MaxStacking) continue;
                        break;

                    case CRATE_EFFECT_SPEED:
                        if (crate->Data.Speed.MaxStacking > 0 && object_ext->SpeedCrates >= crate->Data.Speed.MaxStacking) continue;
                        break;

                    case CRATE_EFFECT_FIREPOWER:
                        if (crate->Data.Firepower.MaxStacking > 0 && object_ext->FirepowerCrates >= crate->Data.Firepower.MaxStacking) continue;
                        break;

                    case CRATE_EFFECT_CLOAK:
                        if (object->IsCloakable) continue;
                        break;

                    default:
                        break;
                    }

                    /**
                     *  Depending on the cell's movement properties, some crates may be unavailable.
                     */
                    if (!Is_Clear_To_Move(crate->Speed, true, true)) continue;

                    if (crate->Weight > 0) {
                        total_shares += crate->Weight;
                        crates.Add(crate);
                    }
                }

                const int pick = Random_Pick(1, total_shares);

                int share_count = 0;
                for (int i = 0; i < crates.Count(); i++) {
                    share_count += crates[i]->Weight;
                    if (pick <= share_count) {
                        powerup = crates[i];
                        break;
                    }
                }

                /**
                 *  Possibly force it to be an MCV if there is
                 *  sufficient money and no buildings left.
                 */
                if (object->House->CurBuildings == 0 &&
                    object->House->Available_Money() > 1500
                    && object->House->UQuantity.Count_Of(Rule->BaseUnit->Type) == 0
                    && Session.Options.Bases) {
                    powerup = CrateTypes[CrateTypeClass::From_Name("Unit")];
                    force_mcv = true;
                }

                /**
                 * Keep track of the number of each type of crate found
                 */
                object->House->TotalCrates->Increment_Unit_Total(CrateTypes.ID(powerup));
            }

            /**
             *  Remove the crate from the map.
             */
            Map.Remove_Crate(Pos);

            /**
             *  #issue-191
             *
             *  Fixes a bug where pre-placed crates and crates spawned by a destroyed
             *  truck will trigger a respawn when they are picked up, even when the
             *  Crates game option was disabled.
             *
             *  @author: CCHyper (based on research by Rampastring)
             */
            if (Session.Type != GAME_NORMAL && Rule->IsMPCrates && Session.Options.Goodies) {
                Map.Place_Random_Crate();
            }

            /**
             *  Create the effect requested.
             */
            return powerup->Execute(object, *this, force_mcv);
        }
    }

    return true;
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
 *  Main function for patching the hooks.
 */
void CellClassExtension_Hooks()
{
	Patch_Jump(0x00454E60, &_CellClass_Draw_Shroud_Fog_Patch);
	Patch_Jump(0x00455130, &_CellClass_Draw_Fog_Patch);
	Patch_Jump(0x00457C00, &CellClassExt::_Goodie_Check);
}
