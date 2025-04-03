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
#include "combatext_hooks.h"
#include "tibsun_inline.h"
#include "buildingext_hooks.h"
#include <algorithm>
#include <unordered_set>
#include "aircraft.h"
#include "aircrafttracker.h"
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
#include "smudgetype.h"


template <typename T>
static T Percent_At_Max(T value, int range, int distance, float percent_at_max)
{
    if (range != 0 && percent_at_max != 1.0f) {

        /**
         *  Calculate the damage at the furthest point.
         */
        float at_max = static_cast<float>(value) * percent_at_max;

        /**
         *  Reduce the damage based on the distance and the damage % at max distance.
         */
        value = static_cast<T>((static_cast<float>(value) - at_max) * static_cast<float>(range - distance) / static_cast<float>(range) + at_max);

        /**
         *  Our damage was originally positive, don't allow it to go negative.
         */
        value = std::max(static_cast<T>(0), value);
    }

    return value;
}


/**
 *  Adjusts damage to reflect the nature of the target.
 *
 *  @author: 04/16/1994 JLB - Created.                                                 
 *           04/17/1994 JLB - Always does a minimum of damage.                         
 *           01/01/1995 JLB - Takes into account distance from damage source.          
 *           04/11/1996 JLB - Changed damage fall-off formula for less damage fall-off.
 *           ZivDero : Adjustments for Tiberian Sun
 *
 *  @note: Originally, the function took `ArmorType armor` instead of `ObjectClass * target`.
 *  The single vanilla call site has been patched to take this into account.
 * 
 */
int Vinifera_Modify_Damage(int damage, WarheadTypeClass* warhead, ObjectClass * target, int distance)
{
    /**
     *	If there is no raw damage value to start with, then
     *	there can be no modified damage either.
     */
    if (!damage || Scen->SpecialFlags.IsInert || warhead == nullptr)
        return 0;

    ArmorType armor = target->Class_Of()->Armor;

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

    float type_modifier = 1.0f;
    switch (target->RTTI)
    {
    case RTTI_INFANTRY:
        type_modifier = warhead_ext->InfantryModifier;
        break;
    case RTTI_UNIT:
        type_modifier = warhead_ext->VehicleModifier;
        break;
    case RTTI_AIRCRAFT:
        type_modifier = warhead_ext->AircraftModifier;
        break;
    case RTTI_BUILDING:
        type_modifier = warhead_ext->BuildingModifier;
        break;
    case RTTI_TERRAIN:
        type_modifier = warhead_ext->TerrainModifier;
        break;
    default:
        break;
    }

    /**
     *  Apply TS Spread logic.
     */
    if (warhead_ext->CellSpread < 0.0)
    {
        /**
         *  Apply the warhead's modifier to the damage.
         */
        damage *= Verses::Get_Modifier(armor, warhead);

        /**
         *  Apply an extra modifier based on the object's type.
         */
        if (type_modifier != 1.0f)
            damage *= type_modifier;

        /**
         *  Ensure that the damage is at least MinDamage.
         */
        damage = std::max(min_damage, damage);

        /**
         *  Reduce damage according to the distance from the impact point.
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
        /**
         *  Apply PercentAtMax.
         */
        damage = Percent_At_Max<int>(damage, static_cast<int>(warhead_ext->CellSpread * CELL_LEPTON_W), distance, warhead_ext->PercentAtMax);

        /**
         *  Apply the warhead's modifier to the damage.
         */
        damage *= Verses::Get_Modifier(armor, warhead);

        /**
         *  Apply an extra modifier based on the object's type.
         */
        if (type_modifier != 1.0f)
            damage *= type_modifier;

        /**
         *  Ensure that the damage is at least MinDamage.
         */
        damage = std::max(min_damage, damage);
    }

    damage = std::min(damage, Rule->MaxDamage);
    return damage;
}


static bool Is_On_High_Bridge(const Coordinate& coord)
{
    return Map[coord].IsUnderBridge && coord.Z >= BRIDGE_LEPTON_HEIGHT + Map.Get_Height_GL(coord);
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

    const bool isbridge = cellptr->IsUnderBridge && coord.Z > BRIDGE_LEPTON_HEIGHT / 2 + Map.Get_Height_GL(coord);

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
                    if (object->Fetch_RTTI() != RTTI_UNIT || !Scen->SpecialFlags.IsHarvesterImmune || !Rule->HarvesterUnit.Is_Present(static_cast<UnitTypeClass*>(object->Class_Of()))) {
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
                        //objects.Delete(veinhole); // vanilla doesn't do this, let's not do this either just in case it's intended.
                        objects.Add(veinhole);
                    }
                }
            }
        }
    }
}


/**
 *  Damages the overlay at this cell.
 *
 *  @author: ZivDero
 */
void Damage_Overlay(Cell const & cell, const WarheadTypeClass * warhead, int strength, bool do_chain_reaction)
{
    /**
     *  If there is a wall present at this location, it may be destroyed. Check to
     *  make sure that the warhead is of the kind that can destroy walls.
     */
    CellClass * cellptr = &Map[cell];
    if (cellptr->Overlay != OVERLAY_NONE) {
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
}


/**
 *  Spawns random smudges and fires in the cell.
 *
 *  @author: ZivDero
 */
void Spawn_Flames_And_Smudges(const Cell & cell, int range, int distance, const WarheadTypeClass * warhead)
{
    Coordinate cell_coord = Cell_Coord(cell);
    cell_coord.Z = Map.Get_Height_GL(cell_coord);

    const auto warhead_ext = Extension::Fetch<WarheadTypeClassExtension>(warhead);

    if (Probability_Of(std::clamp(Percent_At_Max(warhead_ext->ScorchChance, range, distance, warhead_ext->ScorchPercentAtMax), 0.0f, 1.0f))) {
        SmudgeTypeClass::Create_Scorch(cell_coord, 100, 100, false);
    }
    else if (Probability_Of(std::clamp(Percent_At_Max(warhead_ext->CraterChance, range, distance, warhead_ext->CraterPercentAtMax), 0.0f, 1.0f))) {
        SmudgeTypeClass::Create_Crater(cell_coord, 100, 100, false);
    }
    if (Probability_Of(std::clamp(Percent_At_Max(warhead_ext->CellAnimChance, range, distance, warhead_ext->CellAnimPercentAtMax), 0.0f, 1.0f))) {
        const TypeList<AnimTypeClass*>* anims = &warhead_ext->CellAnim;
        if (anims->Count() == 0) {
            anims = &Rule->OnFire;
        }
        new AnimClass((*anims)[Random_Pick(0, anims->Count() - 1)], cell_coord);
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
    Cell cell;                                 // Cell number under explosion.
    DynamicVectorClass<ObjectClass*> objects;  // Objects to be damaged.
    int distance;                              // Distance to unit.
    int range;                                 // Damage effect radius.

    if (Special.IsInert || !warhead) return;

    if (!strength && !warhead->IsWebby) return;

    const auto warhead_ext = Extension::Fetch<WarheadTypeClassExtension>(warhead);

    Coordinate explosion_coord = coord;
    if (warhead_ext->IsSnapToCellCenter) {
        explosion_coord = Coord_Snap(explosion_coord);
    }

    bool use_cell_spread = warhead_ext->CellSpread >= 0;

    if (use_cell_spread) {
        range = warhead_ext->CellSpread * CELL_LEPTON_W;
    } else {
        range = CELL_LEPTON_W + (CELL_LEPTON_W >> 1);
    }

    cell = Coord_Cell(explosion_coord);

    CellClass* cellptr = &Map[cell];
    const bool isbridge = cellptr->IsUnderBridge && explosion_coord.Z > BRIDGE_LEPTON_HEIGHT / 2 + Map.Get_Height_GL(explosion_coord);
    ObjectClass* impacto = cellptr->Cell_Occupier(isbridge);

    /**
     *  Fill the list with units that are in flight, because
     *  they are not present in cell data.
     */
    if (warhead_ext->IsVolumetric || Map.Get_Height_GL(Cell_Coord(cell)) < explosion_coord.Z) {
        int air_range = use_cell_spread ? static_cast<int>(warhead_ext->CellSpread + 0.99) : 1;
        AircraftTracker->Fetch_Targets(&Map[cell], air_range);

        FootClass* target = AircraftTracker->Get_Target();
        while (target != nullptr) {
            if (target->IsActive && target->IsDown && target->Strength > 0) {
                if (use_cell_spread || Distance(explosion_coord, target->Get_Coord()) < CELL_LEPTON_W) {
                    objects.Delete(target);
                    objects.Add(target);
                }
            }
            target = AircraftTracker->Get_Target();
        }
    }

    if (use_cell_spread) {
        Get_Explosion_Targets(explosion_coord, source, range, objects);
    } else {
        /**
         *  Collecting targets within a range of 1 cell is the same as sweeping through this cell,
         *  as well as the surrounding cells.
         */
        Get_Explosion_Targets(explosion_coord, source, CELL_LEPTON_W, objects);
    }

    /**
     *  Sweep through the units to be damaged and damage them. When damaging
     *  buildings, consider a hit on any cell the building occupies as if it
     *  were a direct hit on the building's center.
     */
    //for (ObjectClass* object : objects) {
    for (int i = 0; i < objects.Count(); i++) {
        ObjectClass* object = objects[i];

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
                distance = std::abs(explosion_coord.Z - object->AbsoluteHeight);
                if (distance < LEVEL_LEPTON_H * 2) {
                    distance = 0;
                }
            } else if (object->RTTI == RTTI_BUILDING && use_cell_spread) {
                /**
                 *  For buildings, let's consider their closest cell to the explosion.
                 */
                const Cell* list = object->Occupy_List();
                distance = INT_MAX;
                while (*list != REFRESH_EOL) {
                    int trydist = Distance_Level_Snap(explosion_coord, Cell_Coord(object->Get_Cell() + *list++));
                    distance = std::min(trydist, distance);
                }
            } else {
                distance = Distance_Level_Snap(explosion_coord, object->Target_Coord());
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
                ObjectClass* object = isbridge ? Map[Cell(x, y)].Cell_Occupier(true) : Map[Cell(x, y)].Cell_Occupier(false);

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
                            techno->Rock(explosion_coord, rocking_force);
                        }
                    }
                    object = object->Next;
                }
            }
        }
    }

    const bool close_to_ground = std::abs(explosion_coord.Z - Map.Get_Height_GL(explosion_coord)) < LEVEL_LEPTON_H;

    cellptr = &Map[cell];

    if (close_to_ground) {
        if (use_cell_spread) {
            int cell_radius = (range + CELL_LEPTON_W - 1) / CELL_LEPTON_W;
            for (int x = -cell_radius; x <= cell_radius; x++) {
                for (int y = -cell_radius; y <= cell_radius; y++) {
                    Cell newcell = cell + Cell(x, y);
                    if (!Map.In_Radar(newcell)) continue;
                    distance = Distance(Cell_Coord(newcell), Cell_Coord(cell));
                    if (distance <= range) {
                        Damage_Overlay(newcell, warhead, strength, do_chain_reaction);
                        Spawn_Flames_And_Smudges(newcell, range, distance, warhead);
                    }
                }
            }
        } else {
            Damage_Overlay(cell, warhead, strength, do_chain_reaction);
            Spawn_Flames_And_Smudges(cell, 0, 0, warhead);
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
            if (!cellptr->IsUnderBridge || explosion_coord.Z <= BRIDGE_LEPTON_HEIGHT + LEVEL_LEPTON_H * (cellptr->Height + 1) && explosion_coord.Z > BRIDGE_LEPTON_HEIGHT + LEVEL_LEPTON_H * (cellptr->Height - 2)) {
                if (warhead->IsWallDestroyer && (warhead == Rule->IonCannonWarhead || Random_Pick(1, Rule->BridgeStrength) < strength)) {
                    for (int i = 0; i < (ion_cannon ? 4 : 1); i++) {
                        if (Map.Destroy_Bridge_At(cell)) {
                            TechnoClass::Update_Mission_Targets(cellptr);
                            break;
                        }
                    }
                    Point2D point;
                    TacticalMap->Coord_To_Pixel(explosion_coord, point);
                    TacticalMap->Register_Dirty_Area(Rect(point.X - 128, point.Y - 128, 256, 256), false);
                }
            }
        }

        if (bridge_owner_cell && bridge_owner_cell->Is_Overlay_Rail_Bridge()
            || cellptr->Is_Tile_Train_Bridge_Middle()) {
            if (!cellptr->IsUnderBridge || explosion_coord.Z <= BRIDGE_LEPTON_HEIGHT + LEVEL_LEPTON_H * (cellptr->Height + 1) && explosion_coord.Z > BRIDGE_LEPTON_HEIGHT + LEVEL_LEPTON_H * (cellptr->Height - 2)) {
                if (warhead->IsWallDestroyer && (warhead == Rule->IonCannonWarhead || Random_Pick(1, Rule->BridgeStrength) < strength)) {
                    for (int i = 0; i < (ion_cannon ? 4 : 1); i++) {
                        if (Map.Destroy_Bridge_At(cell)) {
                            TechnoClass::Update_Mission_Targets(cellptr);
                            break;
                        }
                    }
                    Point2D point;
                    TacticalMap->Coord_To_Pixel(explosion_coord, point);
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

            new AnimClass(Rule->BarrelExplode, explosion_coord);
            Explosion_Damage(explosion_coord, Rule->AmmoCrateDamage, nullptr, Rule->C4Warhead, true);
            for (int i = 0; i < Rule->BarrelDebris.Count(); i++) {
                if (Percent_Chance(15)) {
                    new VoxelAnimClass(Rule->BarrelDebris[i], explosion_coord);
                    break;
                }
            }
            if (Percent_Chance(25)) {
                ParticleSystemClass* psys = new ParticleSystemClass(Rule->BarrelParticle, explosion_coord);
                psys->Spawn_Held_Particle(explosion_coord, explosion_coord);
            }

            static FacingType _part_facings[] = { FACING_N, FACING_E, FACING_S, FACING_W };

            for (auto facing : _part_facings) {
                Coordinate adjacent = Adjacent_Coord_With_Height(explosion_coord, facing);
                if (Map[adjacent].Overlay != OVERLAY_NONE && OverlayTypes[Map[adjacent].Overlay]->IsExplosive) {
                    new AnimClass(AnimTypes[AnimTypeClass::From_Name("FIRE3")], explosion_coord, Random_Pick(1, 3) + 3);
                }
            }
        }

        if (strength > warhead->DeformThreshhold) {
            if (Percent_Chance(strength * 0.01 * warhead->Deform * 100.0) && !Is_On_High_Bridge(explosion_coord)) {
                Map.Deform(Coord_Cell(explosion_coord), false);
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
        if ((warhead->IsWallDestroyer || warhead->IsFire) && !Is_On_High_Bridge(explosion_coord)
            && (RuleExtension->IceStrength <= 0 || Random_Pick(0, RuleExtension->IceStrength) < strength)) {
            Map.field_DC.Clear();
            if (Map.Crack_Ice(*cellptr, nullptr)) {
                Map.Recalc_Ice();
            }
        }
    }

    if (warhead->Particle) {
        if (warhead->Particle->BehavesLike == 1) { // This is actually Gas behavior, there are 2 slightly different enums
            MasterParticle->Spawn_Held_Particle(explosion_coord, explosion_coord);
        } else {
            ParticleSystemClass* psys = new ParticleSystemClass(warhead->Particle, explosion_coord);
            psys->Spawn_Held_Particle(explosion_coord, explosion_coord);
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
    Patch_Byte(0x0058604A, 0x56); // push eax -> push esi; Modify_Damage originally takes ArmorType as its argument, we instead pass the target object

    Patch_Jump(0x00460477, &_Do_Flash_CombatLightSize_Patch);
    Patch_Jump(0x0045EB60, &Vinifera_Modify_Damage);
    Patch_Jump(0x0045EEB0, &Vinifera_Explosion_Damage);
}
