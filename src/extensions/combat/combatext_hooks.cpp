/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          COMBATEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended combat functions.
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
#include "tibsun_inline.h"
#include "buildingext_hooks.h"
#include "combatext_hooks.h"

#include "aircraft.h"
#include "anim.h"
#include "animtype.h"
#include "vinifera_globals.h"
#include "combat.h"
#include "cell.h"
#include "overlaytype.h"
#include "rulesext.h"
#include "scenarioext.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "armortype.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "building.h"
#include "coord.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "infantry.h"
#include "infantrytype.h"
#include "mouse.h"
#include "particlesys.h"
#include "particlesystype.h"
#include "tactical.h"
#include "team.h"
#include "teamtype.h"
#include "unit.h"
#include "unittype.h"
#include "veinholemonster.h"
#include "verses.h"
#include "voxelanim.h"
#include "jumpjetlocomotion.h"


/**
 *  Adjusts damage to reflect the nature of the target.
 *
 *  @author: 04/16/1994 JLB - Created.                                                 
 *           04/17/1994 JLB - Always does a minimum of damage.                         
 *           01/01/1995 JLB - Takes into account distance from damage source.          
 *           04/11/1996 JLB - Changed damage fall-off formula for less damage fall-off.
 *           ZivDero : Adjustments for Tiberian Sun
 */
int Vinifera_Modify_Damage(int damage, WarheadTypeClass* warhead, ArmorType armor, int distance)
{
    /**
     *	If there is no raw damage value to start with, then
     *	there can be no modified damage either.
     */
    if (!damage || Scen->SpecialFlags.IsInert || warhead == nullptr)
        return 0;

    /**
     *	Negative damage (i.e., heal) is always applied full strength, but only if the heal
     *	effect is close enough.
     */
    if (damage < 0)
    {
        enum { MAX_HEAL_DISTANCE = 8 };
        if (distance < MAX_HEAL_DISTANCE)
            return damage;

        return 0;
    }

    const auto warhead_ext = Extension::Fetch<WarheadTypeClassExtension>(warhead);
    const int min_damage = warhead_ext->MinDamage >= 0 ? warhead_ext->MinDamage : Rule->MinDamage;

    /**
     *  Apply TS Spread logic.
     */
    if (warhead_ext->CellSpread < 0.0)
    {

        /**
         *  Apply the warhead's modifier to the damage and ensure it's at least MinDamage.
         */
        damage *= Verses::Get_Modifier(armor, warhead);
        damage = std::max(min_damage, damage);

        /**
         *	Reduce damage according to the distance from the impact point.
         */
        if (damage)
        {
            if (!warhead->SpreadFactor)
                distance /= PIXEL_LEPTON_W / 2;
            else
                distance /= warhead->SpreadFactor * (PIXEL_LEPTON_W / 2 + 1);

            distance = std::clamp(distance, 0, 16);

            if (distance)
                damage /= distance;

            /**
             *	Allow damage to drop to zero only if the distance would have
             *	reduced damage to less than 1/4 full damage. Otherwise, ensure
             *	that at least one damage point is done.
             */
            if (distance < 4)
                damage = std::max(damage, min_damage);
        }
    }
    /**
     *  Apply RA2 CellSpread logic.
     */
    else
    {
        float fdamage = (float)damage;

        /**
         *  Calculate the damage at the furthest point.
         */
        float at_max = fdamage * warhead_ext->PercentAtMax;

        /**
         *  Calculate our range in leptons.
         */
        int cell_spread = warhead_ext->CellSpread * CELL_LEPTON_W;

        /**
         *  Reduce the damage based on the distance and the damage % at max distance.
         */
        if (at_max != fdamage && cell_spread != 0)
            damage = ((fdamage - at_max) * (cell_spread - distance) / (warhead_ext->CellSpread * CELL_LEPTON_W) + at_max);

        /**
         *  Our damage was originally positive, don't allow it to go negative.
         */
        damage = std::max(0, damage);

        /**
         *  Apply the warhead's modifier to the damage and ensure it's at least MinDamage.
         */
        damage *= Verses::Get_Modifier(armor, warhead);
        damage = std::max(min_damage, damage);
    }

    damage = std::min(damage, Rule->MaxDamage);
    return damage;
}


static bool Is_On_High_Bridge(const Coordinate& coord)
{
    return Map[coord].IsUnderBridge && coord.Z >= BRIDGE_LEPTON_HEIGHT + Map.Get_Cell_Height(coord);
}


/**
 *  Collects the targets to deal damage to in a certain range.
 *
 *  @author: ZivDero
 */
void Get_Explosion_Targets(const Coordinate& coord, TechnoClass* source, int range, DynamicVectorClass<ObjectClass*>& objects)
{
    Cell cell;      // Cell number under explosion.
    ObjectClass* object; // Working object pointer

    cell = Coord_Cell(coord);
    CellClass* cellptr = &Map[cell];

    int cell_radius = (range + CELL_LEPTON_W - 1) / CELL_LEPTON_W;

    const bool isbridge = cellptr->IsUnderBridge && coord.Z > BRIDGE_LEPTON_HEIGHT / 2 + Map.Get_Cell_Height(coord);

    /**
     *  Fill the list of unit IDs that will have damage
     *  assessed upon them. The units can be lifted from
     *  the cell data directly.
     */
    for (int x = -cell_radius; x <= cell_radius; x++) {
        for (int y = -cell_radius; y <= cell_radius; y++) {

            Cell newcell = cell + Cell(x, y);

            /**
             *  Fetch a pointer to the cell to examine.
             */
            cellptr = &Map[newcell];
            

            /**
             *  Add all objects in this cell to the list of objects to possibly apply
             *  damage to.
             */
            object = cellptr->Cell_Occupier(isbridge);
            while (object) {
                if (object != source) {
                    if (object->Kind_Of() != RTTI_UNIT || !Scen->SpecialFlags.IsHarvesterImmune || !Rule->HarvesterUnit.Is_Present(static_cast<UnitTypeClass*>(object->Class_Of()))) {
                        objects.Delete(object);
                        objects.Add(object);
                    }
                }
                object = object->Next;
            }

            /**
             *  If there is a veinhole monster, it may be destroyed.
             */
            if (cellptr->Overlay != OVERLAY_NONE) {
                if (OverlayTypes[cellptr->Overlay]->IsVeinholeMonster) {
                    VeinholeMonsterClass* veinhole = VeinholeMonsterClass::Fetch_At(cell);
                    if (veinhole) {
                        objects.Add(veinhole);
                    }
                }
            }
        }
    }
}


/**
 *  Inflict an explosion damage affect.
 *
 *  @author: 08/16/1991 JLB : Created.
 *           12/14/2024 ZivDero : Adjustments for Tiberian Sun
 *           02/19/2025 Rampastring : Improved handling of explosion height level.
 *           03/04/2025 ZivDero : Implement CellSpread.
 */
void Vinifera_Explosion_Damage(const Coordinate& coord, int strength, TechnoClass* source, const WarheadTypeClass* warhead, bool do_chain_reaction)
{
    Cell cell;      // Cell number under explosion.
    ObjectClass* object;      // Working object pointer.
    DynamicVectorClass<ObjectClass*> objects;  // Maximum number of objects that can be damaged.
    int distance;  // Distance to unit.
    int range;    // Damage effect radius.

    if (Special.IsInert || !warhead) return;

    if (!strength && !warhead->IsWebby) return;

    range = CELL_LEPTON_W + (CELL_LEPTON_W >> 1);
    cell = Coord_Cell(coord);

    CellClass* cellptr = &Map[cell];
    const bool isbridge = cellptr->IsUnderBridge && coord.Z > BRIDGE_LEPTON_HEIGHT / 2 + Map.Get_Cell_Height(coord);
    ObjectClass* impacto = cellptr->Cell_Occupier(isbridge);

    /**
     *  Fill the list with units that are in flight, because
     *  they are not present in cell data.
     */
    if (Map.Get_Cell_Height(Cell_Coord(cell)) < coord.Z) {

        for (int index = 0; index < Aircrafts.Count(); index++) {
            AircraftClass* aircraft = Aircrafts[index];

            if (aircraft->IsActive) {
                if (aircraft->IsDown && aircraft->Strength > 0) {
                    distance = Distance(coord, aircraft->Get_Coord());
                    if (distance < CELL_LEPTON_W) {
                        objects.Delete(aircraft);
                        objects.Add(aircraft);
                    }
                }
            }
        }

        for (int index = 0; index < Infantry.Count(); index++) {
            InfantryClass* infantry = Infantry[index];

            if (infantry->IsActive && infantry->Class->IsJumpJet) {
                if (infantry->IsDown && infantry->Strength > 0) {
                    distance = Distance(coord, infantry->Get_Coord());
                    if (distance < CELL_LEPTON_W) {
                        objects.Delete(infantry);
                        objects.Add(infantry);
                    }
                }
            }
        }

        for (int index = 0; index < Units.Count(); index++) {
            UnitClass* unit = Units[index];

            if (unit->IsActive && (unit->Class->IsJellyfish || unit->Class->Locomotor == __uuidof(JumpjetLocomotionClass))) {
                if (unit->IsDown && unit->Strength > 0) {
                    distance = Distance(coord, unit->Get_Coord());
                    if (distance < CELL_LEPTON_W) {
                        objects.Delete(unit);
                        objects.Add(unit);
                    }
                }
            }
        }
    }

    const auto warhead_ext = Extension::Fetch<WarheadTypeClassExtension>(warhead);

    if (warhead_ext->CellSpread < 0) {
        /**
         *  Collecting targets within a range of 1 cell is the same as sweeping through this cell,
         *  as well as the surrounding cells.
         */
        Get_Explosion_Targets(coord, source, CELL_LEPTON_W, objects);
    } else {
        range = warhead_ext->CellSpread * CELL_LEPTON_W;
        Get_Explosion_Targets(coord, source, range, objects);
    }

    /**
     *  Sweep through the units to be damaged and damage them. When damaging
     *  buildings, consider a hit on any cell the building occupies as if it
     *  were a direct hit on the building's center.
     */
    for (int index = 0; index < objects.Count(); index++) {
        object = objects[index];

        object->IsToDamage = false;
        if (object->IsActive && !(object->RTTI == RTTI_BUILDING && reinterpret_cast<BuildingClass*>(object)->Class->IsInvisibleInGame)) {
            if (object->RTTI == RTTI_BUILDING && impacto == object) {
#if 0
                distance = 0
#endif
                /**
                 *  #issue-1150
                 *
                 *  Take distance to ground level to account when damaging buildings.
                 */
                distance = std::abs(coord.Z - object->Get_Z_Coord());
                if (distance < LEVEL_LEPTON_H * 2) {
                    distance = 0;
                }
            } else {
                distance = Distance_Level_Snap(coord, object->Target_Coord());
                if (object->RTTI == RTTI_AIRCRAFT && object->In_Air()) {
                    distance /= 2;
                }
            }
            if (object->Strength > 0 && object->IsDown && !object->IsInLimbo && distance <= range) {
                int damage = strength;
                if (warhead != Rule->IonStormWarhead || !object->Is_Foot() || static_cast<FootClass*>(object)->Team == nullptr || !static_cast<FootClass*>(object)->Team->Class->IsIonImmune) {
                    object->Take_Damage(damage, distance, warhead, source);
                }
            }
        }
    }

    const double rocking_force = std::min(strength * 0.01, 4.0);
    if (warhead->IsRocker && rocking_force > 0.3) {
        for (int x = cell.X - 3; x <= cell.X + 3; x++) {
            for (int y = cell.Y - 3; y <= cell.Y + 3; y++) {
                object = isbridge ? Map[Cell(x, y)].Cell_Occupier(true) : Map[Cell(x, y)].Cell_Occupier(false);

                while (object) {
                    TechnoClass* techno = object->As_Techno();
                    if (techno != NULL) {
                        if (Cell(x, y) == cell && source) {
                            Coordinate tcoord = techno->Get_Coord();

                            Coordinate rockdir = source->Get_Coord() - tcoord;
                            TPoint3D<float> rockdirf(rockdir.X, rockdir.Y, rockdir.Z);
                            rockdirf = rockdirf.Normalized() * 10.0f;

                            tcoord += Coordinate(rockdirf.X, rockdirf.Y, rockdirf.Z);
                            techno->Rock(tcoord, rocking_force);
                        }
                        else {
                            techno->Rock(coord, rocking_force);
                        }
                    }
                    object = object->Next;
                }
            }
        }
    }

    const bool close_to_ground = std::abs(coord.Z - Map.Get_Cell_Height(coord)) < LEVEL_LEPTON_H * 2;

    /**
     *  If there is a wall present at this location, it may be destroyed. Check to
     *  make sure that the warhead is of the kind that can destroy walls.
     */
    cellptr = &Map[cell];
    if (cellptr->Overlay != OVERLAY_NONE && close_to_ground) {
        OverlayTypeClass const* optr = OverlayTypes[cellptr->Overlay];

        if (optr->IsChainReactive) {
            if (!(optr->IsTiberium && !warhead->IsTiberiumDestroyer) && do_chain_reaction) {
                Chain_Reaction_Damage(cell);
                cellptr->Reduce_Tiberium(strength / 10);
            }
        }
        if (optr->IsWall) {

            /**
             *  #issue-410
             *
             *  Implements IsWallAbsoluteDestroyer for WarheadTypes.
             *
             *  @author: CCHyper
             */
            const auto warheadtypeext = Extension::Fetch<WarheadTypeClassExtension>(warhead);
            if (warheadtypeext->IsWallAbsoluteDestroyer) {
                Map[cell].Reduce_Wall(-1);
            }
            else if (warhead->IsWallDestroyer || (warhead->IsWoodDestroyer && optr->Armor == ARMOR_WOOD)) {
                Map[cell].Reduce_Wall(strength);
            }
        }
        if (cellptr->Overlay == OVERLAY_NONE) {
            TechnoClass::Update_Mission_Targets(cellptr);
        }
    }

    /**
     *  If there is a bridge at this location, then it may be destroyed by the
     *  combat damage.
     */
    bool ion_cannon = warhead == Rule->IonCannonWarhead;
    if (Scen->SpecialFlags.IsDestroyableBridges && warhead->IsWallDestroyer) {

        const CellClass* bridge_owner_cell = cellptr->Get_Bridge_Owner();

        if (bridge_owner_cell && bridge_owner_cell->Is_Overlay_Bridge()
            || cellptr->Is_Tile_Bridge_Middle()) {
            if (!cellptr->IsUnderBridge || coord.Z <= BRIDGE_LEPTON_HEIGHT + LEVEL_LEPTON_H * (cellptr->Height + 1) && coord.Z > BRIDGE_LEPTON_HEIGHT + LEVEL_LEPTON_H * (cellptr->Height - 2)) {
                if (warhead->IsWallDestroyer && (warhead == Rule->IonCannonWarhead || Random_Pick(1, Rule->BridgeStrength) < strength)) {
                    for (int i = 0; i < (ion_cannon ? 4 : 1); i++) {
                        if (Map.Destroy_Bridge_At(cell)) {
                            TechnoClass::Update_Mission_Targets(cellptr);
                            break;
                        }
                    }
                    Point2D point;
                    TacticalMap->Coord_To_Pixel(coord, point);
                    TacticalMap->Register_Dirty_Area(Rect(point.X - 128, point.Y - 128, 256, 256), false);
                }
            }
        }

        if (bridge_owner_cell && bridge_owner_cell->Is_Overlay_Rail_Bridge()
            || cellptr->Is_Tile_Train_Bridge_Middle()) {
            if (!cellptr->IsUnderBridge || coord.Z <= BRIDGE_LEPTON_HEIGHT + LEVEL_LEPTON_H * (cellptr->Height + 1) && coord.Z > BRIDGE_LEPTON_HEIGHT + LEVEL_LEPTON_H * (cellptr->Height - 2)) {
                if (warhead->IsWallDestroyer && (warhead == Rule->IonCannonWarhead || Random_Pick(1, Rule->BridgeStrength) < strength)) {
                    for (int i = 0; i < (ion_cannon ? 4 : 1); i++) {
                        if (Map.Destroy_Bridge_At(cell)) {
                            TechnoClass::Update_Mission_Targets(cellptr);
                            break;
                        }
                    }
                    Point2D point;
                    TacticalMap->Coord_To_Pixel(coord, point);
                    TacticalMap->Register_Dirty_Area(Rect(point.X - 96, point.Y - 96, 192, 192), false);
                }
            }
        }

        if (cellptr->Is_Overlay_Low_Bridge() && close_to_ground) {
            if (warhead == Rule->IonCannonWarhead || Random_Pick(1, Rule->BridgeStrength) < strength) {
                const bool destroyed = Map.Destroy_Low_Bridge_At(cell);
                Map.Destroy_Low_Bridge_At(cell);
                if (destroyed) {
                    TechnoClass::Update_Mission_Targets(cellptr);
                }
            }
        }
    }

    if (close_to_ground) {
        if (cellptr->Overlay != OVERLAY_NONE && OverlayTypes[cellptr->Overlay]->IsExplosive) {
            cellptr->Redraw_Overlay();
            cellptr->Overlay = OVERLAY_NONE;
            cellptr->Recalc_Attributes();
            Map.Update_Cell_Zone(cellptr->Pos);
            Map.Update_Cell_Subzones(cellptr->Pos);
            TechnoClass::Update_Mission_Targets(cellptr);

            new AnimClass(Rule->BarrelExplode, coord);
            Explosion_Damage(coord, Rule->AmmoCrateDamage, nullptr, Rule->C4Warhead, true);
            for (int i = 0; i < Rule->BarrelDebris.Count(); i++) {
                if (Percent_Chance(15)) {
                    new VoxelAnimClass(Rule->BarrelDebris[i], coord);
                    break;
                }
            }
            if (Percent_Chance(25)) {
                ParticleSystemClass* psys = new ParticleSystemClass(Rule->BarrelParticle, coord);
                psys->Spawn_Held_Particle(coord, coord);
            }

            static FacingType _part_facings[] = { FACING_N, FACING_E, FACING_S, FACING_W };

            for (int f = 0; f < 4; f++) {
                Coordinate adjacent = Adjacent_Coord_With_Height(coord, _part_facings[f]);
                if (Map[adjacent].Overlay != OVERLAY_NONE && OverlayTypes[Map[adjacent].Overlay]->IsExplosive) {
                    new AnimClass(AnimTypes[AnimTypeClass::From_Name("FIRE3")], coord, Random_Pick(1, 3) + 3);
                }
            }
        }

        if (strength > warhead->DeformThreshhold) {
            if (Percent_Chance(strength * 0.01 * warhead->Deform * 100.0) && !Is_On_High_Bridge(coord)) {
                Map.Deform(Coord_Cell(coord), false);
            }
        }

        if (cellptr->Is_Tile_Destroyable_Cliff()) {
            if (Percent_Chance(Rule->CollapseChance)) {
                Map.Collapse_Cliff(*cellptr);
            }
        }

        /**
         *  #issue-897
         *
         *  Implements IsIceDestruction scenario option for preventing destruction of ice,
         *  and IceStrength for configuring the chance for ice getting destroyed.
         *
         *  @author: Rampastring
         */
        if ((warhead->IsWallDestroyer || warhead->IsFire) && !Is_On_High_Bridge(coord)
            && (RuleExtension->IceStrength <= 0 || Random_Pick(0, RuleExtension->IceStrength) < strength)) {
            Map.field_DC.Clear();
            if (Map.Crack_Ice(*cellptr, nullptr)) {
                Map.Recalc_Ice();
            }
        }
    }

    if (warhead->Particle) {
        if (warhead->Particle->BehavesLike == 1) { // This is actually Gas behavior, there are 2 slightly different enums
            MasterParticle->Spawn_Held_Particle(coord, coord);
        } else {
            ParticleSystemClass* psys = new ParticleSystemClass(warhead->Particle, coord);
            psys->Spawn_Held_Particle(coord, coord);
        }
    }
}


/**
 *  convert a float to integer with the desired scale.
 * 
 *  @author CCHyper
 */
static int Scale_Float_To_Int(float value, int scale)
{
    value = std::clamp(value, 0.0f, 1.0f);
    return (value * scale);
}


/**
 *  #issue-412
 * 
 *  Implements CombatLightSize for WarheadTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Do_Flash_CombatLightSize_Patch)
{
    GET_REGISTER_STATIC(int, damage, ecx);
    GET_REGISTER_STATIC(const WarheadTypeClass *, warhead, edx);
    static const WarheadTypeClassExtension *warheadtypeext;
    static float light_size;
    static int flash_size;

    /**
     *  Fetch the extension instance.
     */
    warheadtypeext = Extension::Fetch<WarheadTypeClassExtension>(warhead);

    /**
     *  If no custom light size has been set, then just use the default code.
     *  This sets the size of the light based on the damage dealt by the Warhead.
     */
    if (warheadtypeext->CombatLightSize <= 0.0f) {

        /**
         *  Original code.
         */
        flash_size = (damage / 4);
        if (flash_size < 63) {
            if (flash_size <= 21) {
                flash_size = 21;
            }
        } else {
            flash_size = 63;
        }
    }


    /**
     *  Has a custom light size been set on the warhead?
     */
    if (warheadtypeext->CombatLightSize > 0.0f) {

        /**
         *  Clamp the light size and scale to expected size range.
         */
        light_size = warheadtypeext->CombatLightSize;
        if (light_size > 1.0f) {
            light_size = 1.0f;
        }
        flash_size = Scale_Float_To_Int(light_size, 63);
    }

    /**
     *  Set the desired flash size.
     */
    _asm { mov esi, flash_size }

    JMP(0x00460495);
}


/**
 *  Main function for patching the hooks.
 */
void CombatExtension_Hooks()
{
    Patch_Jump(0x00460477, &_Do_Flash_CombatLightSize_Patch);
    Patch_Jump(0x0045EB60, &Vinifera_Modify_Damage);
    Patch_Jump(0x0045EEB0, &Vinifera_Explosion_Damage);
}
