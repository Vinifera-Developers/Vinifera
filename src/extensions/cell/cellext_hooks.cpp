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
#include "unit.h"
#include "utracker.h"
#include "voc.h"
#include "vox.h"
#include "warheadtypeext.h"


/**
  *  A fake class for implementing new member functions which allow
  *  access to the "this" pointer of the intended class.
  *
  *  @note: This must not contain a constructor or deconstructor!
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
        bool force_mcv = false;
        int force_money = 0;
        int damage;
        Coordinate coord;

        if (Session.Type == GAME_NORMAL || !object->House->Class->IsMultiplayPassive) {

            if (OverlayTypeClass::As_Reference(Overlay).IsCrateTrigger && object->Tag) {
                DEBUG_INFO("Springing trigger on crate at %d,%d\n", Pos.X, Pos.Y);
                object->Tag->Spring(TEVENT_PICKUP_CRATE, object);

                if (!object->IsActive) {
                    return false;
                }

                Scen->CratePickedUp = true;
            }

            /**
             *  Determine the total number of shares for all the crate powerups. This is used as
             *  the base pool to determine the odds from.
             */
            int total_shares = 0;
            for (int index = CRATE_FIRST; index < CRATE_COUNT; index++) {
                total_shares += CrateShares[index];
            }

            /**
             *  Pick a random crate powerup according to the shares allotted to each powerup.
             *  In solo play, the bonus item is dependant upon the rules control.
             */
            CrateType powerup = CRATE_MONEY;
            if (Session.Type == GAME_NORMAL) {

                /**
                 *  Solo play has money amount determined by rules.ini file.
                 */
                force_money = Rule->SoloCrateMoney;

                if (OverlayTypes[Overlay] == Rule->SteelCrateImage) {
                    powerup = Rule->SilverCrate;
                }

                if (OverlayTypes[Overlay] == Rule->WoodCrateImage) {
                    powerup = Rule->WoodCrate;
                }
            }
            else {
                int pick = Random_Pick(1, total_shares);

                int share_count = 0;
                for (powerup = CRATE_FIRST; powerup < CRATE_COUNT; powerup++) {
                    share_count += CrateShares[powerup];
                    if (pick <= share_count) break;
                }

                /**
                 *  Possibly force it to be an MCV if there is
                 *  sufficient money and no buildings left.
                 */
                if (object->House->CurBuildings == 0 &&
                    object->House->Available_Money() > 1500
                    && object->House->UQuantity.Count_Of(Rule->BaseUnit->Type) == 0
                    && Session.Options.Bases) {
                    powerup = CRATE_UNIT;
                    force_mcv = true;
                }

                /**
                 *  Depending on what was picked, there might be an alternate goodie if the selected
                 *  goodie would have no effect.
                 */
                switch (powerup) {
                case CRATE_UNIT:
                    if (object->House->CurUnits > 50) powerup = CRATE_MONEY;
                    break;

                case CRATE_SQUAD:
                    if (object->House->CurInfantry > 100) powerup = CRATE_MONEY;
                    break;

                case CRATE_ARMOR:
                    if (object->ArmorBias != 1.0) powerup = CRATE_MONEY;
                    break;

                case CRATE_SPEED:
                    if (object->SpeedBias != 1.0 || object->What_Am_I() == RTTI_AIRCRAFT) powerup = CRATE_MONEY;
                    break;

                case CRATE_FIREPOWER:
                    if (object->FirepowerBias != 1.0 || !object->Is_Weapon_Equipped()) powerup = CRATE_MONEY;
                    break;

                case CRATE_CLOAK:
                    if (object->IsCloakable) powerup = CRATE_MONEY;
                    break;

                case CRATE_MONEY:
                default:
                    break;
                }

                /**
                 *  Special override for water crates so that illegal goodies items
                 *  won't appear.
                 */
                //if (Overlay == OVERLAY_WATER_CRATE) {
                //    switch (powerup) {
                //    case CRATE_UNIT:
                //    case CRATE_SQUAD:
                //        powerup = CRATE_MONEY;
                //        break;

                //    default:
                //        break;
                //    }
                //}

                /**
                 * Keep track of the number of each type of crate found
                 */
                object->House->TotalCrates->Increment_Unit_Total(powerup);
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

            if (powerup == CRATE_SQUAD) {
                powerup = CRATE_MONEY;
            }

            /**
             *  Create the effect requested.
             */
            bool tospeak = false;
            switch (powerup) {
                /**
                 *  Give the player money.
                 */
            case CRATE_MONEY: {
            crate_money:
                DEBUG_INFO("Crate at %d,%d contains money\n", Pos.X, Pos.Y);

                int money;
                if (force_money > 0) {
                    money = force_money;
                }
                else {
                    money = Random_Pick(CrateData[powerup], CrateData[powerup] + 900);
                }

                if (object->House->Is_Player_Control() && Session.Type == GAME_NORMAL) {
                    PlayerPtr->Refund_Money(money);
                }
                else {
                    object->House->Refund_Money(money);
                }

                break;
            }

                /**
                 *  Shroud the world in blackness.
                 */
            case CRATE_DARKNESS:
                DEBUG_INFO("Crate at %d,%d contains 'shroud'\n", Pos.X, Pos.Y);
                if (object->House->Is_Player_Control()) {
                    Map.Shroud_The_Map();
                }
                break;

                /**
                 *  Reveal the entire map.
                 */
            case CRATE_REVEAL:
                DEBUG_INFO("Crate at %d,%d contains 'reveal'\n", Pos.X, Pos.Y);
                if (object->House->Is_Human_Control()) {
                    Map.Reveal_The_Map();
                }
                break;

                /**
                 *  Try to create a unit where the crate was.
                 */
            case CRATE_UNIT: {

                DEBUG_INFO("Crate at %d,%d contains a unit\n", this->Pos.X, this->Pos.Y);

                UnitTypeClass const* utp = nullptr;

                /**
                 *  Give the player an MCV if he has no base left but does have more than enough
                 *  money to rebuild a new base. Of course, if he already has an MCV, then don't
                 *  give him another one.
                 */
                if (force_mcv) {
                    utp = Rule->BaseUnit;
                }

                /**
                 *  If the player has a base and a refinery, but no harvester, then give him
                 *  a free one.
                 */
                if (utp == nullptr && (object->House->BQuantity.Count_Of(Rule->BuildRefinery[0]->Type)) && !(object->House->UQuantity.Count_Of(Rule->HarvesterUnit[0]->Type))) {
                    utp = Rule->HarvesterUnit[0];
                }

                /**
                 *  Check for special unit type override value.
                 */
                if (Rule->UnitCrateType != nullptr) {
                    utp = Rule->UnitCrateType;
                }

                /**
                 *  If no unit type has been determined, then pick one at random.
                 */
                while (utp == nullptr) {
                    UnitType utype = Random_Pick(UNIT_FIRST, static_cast<UnitType>(UnitTypes.Count() - 1));
                    if (&UnitTypeClass::As_Reference(utype) != Rule->BaseUnit || Session.Options.Bases) {
                        utp = &UnitTypeClass::As_Reference(utype);
                        if (utp->IsCrateGoodie && (utp->Ownable & 1 << object->Owning_House()->ID)) {
                            break;
                        }
                        utp = nullptr;
                    }
                }

                if (utp != nullptr) {
                    UnitClass* goodie_unit = reinterpret_cast<UnitClass*>(utp->Create_One_Of(object->House));
                    if (goodie_unit != nullptr) {
                        if (goodie_unit->Unlimbo(Cell_Coord())) {
                            return false;
                        }

                        /**
                         *  Try to place the object into a nearby cell if something is preventing
                         *  placement at the crate location.
                         */
                        Cell cell = Map.Nearby_Location(Pos, goodie_unit->Class->Speed);
                        if (goodie_unit->Unlimbo(::Cell_Coord(cell))) {
                            return false;
                        }
                        delete goodie_unit;
                        goto crate_money;
                    }
                }
            }
                break;

                /**
                 *  Create a squad of miscellaneous composition.
                 */
            //case CRATE_SQUAD:
            //    for (int index = 0; index < 5; index++) {
            //        static InfantryType _inf[] = {
            //            INFANTRY_E1,INFANTRY_E1,INFANTRY_E1,INFANTRY_E1,INFANTRY_E1,INFANTRY_E1,
            //            INFANTRY_E2,
            //            INFANTRY_E3,
            //            INFANTRY_RENOVATOR
            //        };
            //        if (!InfantryTypeClass::As_Reference(_inf[Random_Pick(0, ARRAY_SIZE(_inf) - 1)]).Create_And_Place(Cell_Number(), object->Owner())) {
            //            if (index == 0) {
            //                goto crate_money;
            //            }
            //        }
            //    }
            //    return(false);

                /**
                 *  A one para-bomb mission.
                 */
            //case CRATE_PARA_BOMB:
            //    if (object->House->SuperWeapon[SPC_PARA_BOMB].Enable(true)) {
            //        // Changes for client/server multiplayer. ST - 8/2/2019 2:56PM
            //        if (Session.Type == GAME_GLYPHX_MULTIPLAYER) {
            //            if (object->House->IsHuman) {
            //                Sidebar_Glyphx_Add(RTTI_SPECIAL, SPC_PARA_BOMB, object->House);
            //            }
            //        }
            //        else {
            //            if (object->IsOwnedByPlayer) {
            //                Map.Add(RTTI_SPECIAL, SPC_PARA_BOMB);
            //                Map.Column[1].Flag_To_Redraw();
            //            }
            //        }
            //    }
            //    break;

                /**
                 *  A one time sonar pulse
                 */
            //case CRATE_SONAR:
            //    if (object->House->SuperWeapon[SPC_SONAR_PULSE].Enable(true)) {
            //        // Changes for client/server multiplayer. ST - 8/2/2019 2:56PM
            //        if (Session.Type == GAME_GLYPHX_MULTIPLAYER) {
            //            if (object->House->IsHuman) {
            //                Sidebar_Glyphx_Add(RTTI_SPECIAL, SPC_SONAR_PULSE, object->House);
            //            }
            //        }
            //        else {
            //            if (object->IsOwnedByPlayer) {
            //                Map.Add(RTTI_SPECIAL, SPC_SONAR_PULSE);
            //                Map.Column[1].Flag_To_Redraw();
            //            }
            //        }
            //    }
            //    break;

                /**
                 *  A group of explosions are triggered around the crate.
                 */
            case CRATE_EXPLOSION:
                DEBUG_INFO("Crate at %d,%d contains explosives\n", Pos.X, Pos.Y);
                {
                    int d = CrateData[powerup];
                    object->Take_Damage(d, 0, Rule->C4Warhead, nullptr, true);
                }
                for (int index = 0; index < 5; index++) {
                    Coordinate frag_coord = Coord_Scatter(Cell_Coord(), Random_Pick(0, 0x0200));
                    damage = CrateData[powerup];
                    Explosion_Damage(frag_coord, damage, nullptr, Rule->C4Warhead, true);
                    new AnimClass(Combat_Anim(damage, Rule->C4Warhead, LAND_CLEAR, &frag_coord), frag_coord, Get_Explosion_Z(frag_coord.Z, &frag_coord));
                }
                break;

                /**
                 *  A napalm blast is triggered.
                 */
            case CRATE_NAPALM:
                DEBUG_INFO("Crate at %d,%d contains napalm\n", Pos.X, Pos.Y);
                coord = Coord_Mid(Cell_Coord(), object->Center_Coord());
                new AnimClass(AnimTypes[0], coord);
                {
                    int d = CrateData[powerup];
                    object->Take_Damage(d, 0, Rule->FlameDamage, nullptr, true);
                }
                damage = CrateData[powerup];
                Explosion_Damage(coord, damage, nullptr, Rule->FlameDamage, true);
                break;

                /**
                 *  All objects within a certain range will gain the ability to cloak.
                 */
            case CRATE_CLOAK:
                DEBUG_INFO("Crate at %d,%d contains cloaking device\n", Pos.X, Pos.Y);
                for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
                    ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

                    if (obj && obj->Is_Techno() && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius) {
                        static_cast<TechnoClass*>(obj)->IsCloakable = true;
                    }
                }
                break;

                /**
                 *  All of the player's objects heal up.
                 */
            case CRATE_HEAL_BASE: {
                DEBUG_INFO("Crate at %d,%d contains base healing\n", Pos.X, Pos.Y);
                if (object->IsOwnedByPlayer) {
                    Sound_Effect(Rule->HealCrateSound, object->Center_Coord());
                }
                for (int index = 0; index < Logic.Count(); index++) {
                    ObjectClass* obj = Logic[index];

                    if (obj && object->Is_Techno() && object->House == obj->Owning_House()) {
                        int healby = obj->Strength - obj->Class_Of()->MaxStrength;
                        obj->Take_Damage(healby, 0, Rule->C4Warhead, nullptr, true, true);
                    }
                }
                }
                break;


            case CRATE_ICBM: {
                DEBUG_INFO("Crate at %d,%d contains ICBM\n", Pos.X, Pos.Y);
                const SuperWeaponTypeClass* sw = SuperWeaponTypeClass::From_Action(ACTION_NUKE_BOMB);
                for (SpecialWeaponType spc = SPECIAL_FIRST; spc < object->House->SuperWeapon.Count(); spc++) {
                    if (object->House->SuperWeapon[spc]->Class->ActsLike == SPECIAL_MULTI_MISSILE) {
                        if (spc != SPECIAL_NONE && object->House->SuperWeapon[sw->Type]->Enable(true) && object->IsOwnedByPlayer) {
                            Map.Add(RTTI_SPECIAL, sw->Type);
                            SidebarExtension->Flag_Strip_To_Redraw(RTTI_SPECIAL);
                        }
                        break;
                    }
                }
            }
                break;

            case CRATE_ARMOR:
                DEBUG_INFO("Crate at %d,%d contains armor\n", Pos.X, Pos.Y);
                for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
                    ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

                    if (obj != nullptr && obj->Is_Techno() && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius && static_cast<TechnoClass*>(obj)->ArmorBias == 1.0) {
                        static_cast<TechnoClass*>(obj)->ArmorBias *= CrateData[powerup];
                        if (obj->Owning_House()->Is_Player_Control()) tospeak = true;
                    }
                }
                if (tospeak) Speak(VOX_UPGRADE_ARMOR);
                break;

            case CRATE_SPEED:
                DEBUG_INFO("Crate at %d,%d contains speed\n", Pos.X, Pos.Y);
                for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
                    ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

                    if (obj && obj->Is_Foot() && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius && static_cast<FootClass*>(obj)->SpeedBias == 1.0 && obj->What_Am_I() != RTTI_AIRCRAFT) {
                        FootClass* foot = static_cast<FootClass*>(obj);
                        foot->SpeedBias *= CrateData[powerup];
                        if (foot->IsOwnedByPlayer) tospeak = true;
                    }
                }
                if (tospeak) Speak(VOX_UPGRADE_SPEED);
                break;

            case CRATE_FIREPOWER:
                DEBUG_INFO("Crate at %d,%d contains firepower\n", Pos.X, Pos.Y);
                for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
                    ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

                    if (obj && obj->Is_Techno() && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius && static_cast<TechnoClass*>(obj)->FirepowerBias == 1.0) {
                        static_cast<TechnoClass*>(obj)->FirepowerBias *= CrateData[powerup];
                        if (obj->Owning_House()->Is_Player_Control()) tospeak = true;
                    }
                }
                if (tospeak) Speak(VOX_UPGRADE_FIREPOWER);
                break;

            case CRATE_VETERAN:
                DEBUG_INFO("Crate at %d,%d contains veterancy(TM)\n", Pos.X, Pos.Y);
                for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
                    ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

                    /**
                     *  #issue-161
                     *
                     *  Veterancy crate bonus does not check if a object is un-trainable
                     *  before granting it the veterancy bonus.
                     *
                     *  @author: CCHyper (based on research by Iran)
                     */
                    if (obj != nullptr && obj->Is_Techno() && object->IsDown && obj->Techno_Type_Class()->IsTrainable && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius) {

                        for (int i = 0; i < CrateData[powerup]; i++) {
                            VeterancyClass& veterancy = static_cast<TechnoClass*>(obj)->Veterancy;
                            if (veterancy.Is_Veteran()) {
                                veterancy.Set_Elite(true);
                            }
                            if (veterancy.Is_Rookie()) {
                                veterancy.Set_Veteran(true);
                            }
                            if (veterancy.Is_Dumbass()) {
                                veterancy.Set_Rookie(true);
                            }
                        }
                        //if (obj->Owning_House()->Is_Player_Control()) tospeak = true;
                    }
                }
               // if (tospeak) Speak(VOX_UPGRADE_ARMOR); // could add a promotion vox?
                break;

            case CRATE_GAS: {
                DEBUG_INFO("Crate at %d,%d contains poison gas\n", Pos.X, Pos.Y);
                const WarheadTypeClass* wh = WarheadTypeClass::As_Pointer("GAS");
                damage = CrateData[powerup];
                Explosion_Damage(Center_Coord(), damage, nullptr, wh, false);
                for (FacingType facing = FACING_FIRST; facing < FACING_COUNT; facing++) {
                    Explosion_Damage(Adjacent_Cell(facing).Center_Coord(), damage, nullptr, wh, false);
                }
            }
                break;

            case CRATE_TIBERIUM: {
                DEBUG_INFO("Crate at %d,%d contains tiberium\n", Pos.X, Pos.Y);
                TiberiumType tib = static_cast<TiberiumType>(Random_Pick(0, Tiberiums.Count() - 1));
                if (tib == TIBERIUM_CRUENTUS) {
                    tib = TIBERIUM_RIPARIUS;
                }
                int amount = Random_Pick(10, 20);
                while (amount) {
                    Coordinate frag_coord = Coord_Scatter(Center_Coord(), Random_Pick(0, CELL_LEPTON_W * 3), true);
                    Map[frag_coord].Place_Tiberium(tib, 1);
                    amount--;
                }
            }

            //case CRATE_INVULN:
            //    for (int index = 0; index < DisplayClass::Layer[LAYER_GROUND].Count(); index++) {
            //        ObjectClass* obj = DisplayClass::Layer[LAYER_GROUND][index];

            //        if (obj && obj->Is_Techno() && Distance(Cell_Coord(), obj->Center_Coord()) < Rule->CrateRadius) {
            //            ((TechnoClass*)obj)->IronCurtainCountDown = (TICKS_PER_MINUTE * fixed(CrateData[powerup], 256));
            //            obj->Mark(MARK_CHANGE);
            //        }
            //    }
            //    break;

            default:
                break;
            }

            /**
             *  Generate any corresponding animation associated with this crate powerup.
             */
            if (CrateAnims[powerup] != ANIM_NONE) {
                new AnimClass(AnimTypes[CrateAnims[powerup]], Cell_Coord());
            }
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
}
