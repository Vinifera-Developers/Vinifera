/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TECHNOEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TechnoClass.
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
#include "spawnmanager.h"
#include "technoext_hooks.h"

#include <vector>

#include "unit.h"
#include "unittype.h"
#include "bullettype.h"
#include "bullettypeext.h"
#include "technoext.h"
#include "techno.h"
#include "technotype.h"
#include "technotypeext.h"
#include "teamtype.h"
#include "team.h"
#include "tibsun_inline.h"
#include "weapontype.h"
#include "weapontypeext.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "house.h"
#include "housetype.h"
#include "rules.h"
#include "rulesext.h"
#include "tiberium.h"
#include "uicontrol.h"
#include "infantry.h"
#include "infantrytype.h"
#include "infantrytypeext.h"
#include "voc.h"
#include "tactical.h"
#include "clipline.h"
#include "mouse.h"
#include "vinifera_util.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "buildingext.h"
#include "debughandler.h"
#include "drawshape.h"
#include "cell.h"
#include "fetchres.h"
#include "wwkeyboard.h"
#include "options.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "language.h"
#include "ionstorm.h"
#include "storageext.h"
#include "textprint.h"
#include "tiberiumext.h"
#include "unittype.h"
#include "unittypeext.h"
#include "verses.h"
#include "session.h"
#include "mouse.h"
#include "sideext.h"
#include "tag.h"
#include "tibsun_functions.h"
#include "utracker.h"
#include "aircraft.h"
#include "spawner.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class TechnoClassExt : public TechnoClass
{
public:
    void _Draw_Pips(Point2D& bottomleft, Point2D& bottomright, Rect& rect) const;
    WeaponSlotType _What_Weapon_Should_I_Use(TARGET target) const;
    bool _Is_Allowed_To_Retaliate(TechnoClass* source, WarheadTypeClass const* warhead) const;
    double _Target_Threat(TechnoClass* target, Coordinate& firing_coord) const;
    int _Anti_Infantry() const;
    ActionType _What_Action(ObjectClass* object, bool disallow_force);
    void _Drop_Tiberium();
    int _Cell_Distance_Squared(const AbstractClass* object) const;
    void _Draw_Target_Laser() const;
    void _Draw_Text_Overlay(Point2D& point1, Point2D& point2, Rect& rect) const;
    const InfantryTypeClass* _Crew_Type() const;
    int _How_Many_Survivors() const;
    bool _Spawner_Fire_At(TARGET target, WeaponTypeClass* weapon);
    bool _Target_Something_Nearby(Coordinate& coord, ThreatType threat);
    void _Stun();
    void _Mission_AI();
    FireErrorType _Can_Fire(TARGET target, WeaponSlotType which = WEAPON_SLOT_PRIMARY);
    bool _Can_Player_Move() const;
    Coordinate _Fire_Coord(WeaponSlotType which) const;
    void _Record_The_Kill(TechnoClass* source);
};


/**
 *  Draw the pips of this Techno.
 *
 *  @author: 08/08/1995 JLB - Created.                       
 *           10/06/1995 JLB - Displays the team group number.
 *           09/10/1996 JLB - Medic hack for red pip.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
void TechnoClassExt::_Draw_Pips(Point2D& bottomleft, Point2D& center, Rect& rect) const
{
    int drawx = bottomleft.X + 6;
    int drawy = bottomleft.Y - 1;
    int dx = 4;
    int dy = 2;

    const ShapeFileStruct* pip_shapes = Class_Of()->PipShapes;
    const ShapeFileStruct* pips1 = Class_Of()->PipShapes;
    const ShapeFileStruct* pips2 = Class_Of()->Pip2Shapes;

    const auto ttype = Techno_Type_Class();
    const auto ttype_ext = Extension::Fetch<TechnoTypeClassExtension>(ttype);
    const auto ext = Extension::Fetch<TechnoClassExtension>(this);

    if (What_Am_I() != RTTI_BUILDING)
    {
        drawx = bottomleft.X - 5;
        drawy = bottomleft.Y;
        dx = 4;
        dy = 0;
        pip_shapes = pips2;
    }

    /**
     *  Because of ts-patches we have to check if the object is selected,
     *  or we'll draw pips for unselected objects
     */
    if (IsSelected)
    {
        /**
         *  Transporter type objects have a different graphic representation for the pips. The
         *  pip color represents the type of occupant.
         */
        const bool carrying_passengers = Techno_Type_Class()->Max_Passengers() > 0;
        if (carrying_passengers)
        {
            ObjectClass const* object = Cargo.Attached_Object();
            for (int index = 0; index < Class_Of()->Max_Pips(); index++)
            {
                int pip = 0;

                if (object != nullptr)
                {
                    pip = 1;
                    if (object->What_Am_I() == RTTI_INFANTRY)
                    {
                        pip = ((InfantryClass*)object)->Class->Pip;
                    }
                    object = object->Next;
                }
                CC_Draw_Shape(LogicSurface, NormalDrawer, pip_shapes, pip, &Point2D(drawx + dx * index, drawy + dy * index), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
            }

        }
        else
        {
            /**
             *  Display number of how many attached objects there are. This is also used
             *  to display the fullness rating for a harvester.
             */
            int pips = Pip_Count();

            /**
             *  Check if it contains Tiberium to show the right type of pips for the
             *  various minerals it could have stored.
             */
            if ((What_Am_I() == RTTI_UNIT || What_Am_I() == RTTI_BUILDING) && Techno_Type_Class()->PipScale == PIP_TIBERIUM)
            {
                std::vector<int> pips_to_draw;
                pips_to_draw.reserve(Class_Of()->Max_Pips());

                /**
                 *  Weeders/Waste Facilities draw all their contents with the Weed pip.
                 */
                if ((ttype->What_Am_I() == RTTI_UNITTYPE && ((UnitTypeClass*)ttype)->IsToVeinHarvest) ||
                    (ttype->What_Am_I() == RTTI_BUILDINGTYPE && ((BuildingTypeClass*)ttype)->IsWeeder))
                {
                    /**
                     *  Add the pips to draw to a vector.
                     */
                    for (int i = 0; i < pips; i++)
                        pips_to_draw.emplace_back(RuleExtension->WeedPipIndex);
                }
                else
                {
                    /**
                     *  The first element is the sorting order, second element is the Tiberium ID.
                     */
                    std::vector<std::tuple<int, int>> tibtypes;

                    /**
                     *  Add all the Tiberiums and sort.
                     */
                    for (int i = 0; i < Tiberiums.Count(); i++)
                        tibtypes.emplace_back(Extension::Fetch<TiberiumClassExtension>(Tiberiums[i])->PipDrawOrder, i);

                    std::stable_sort(tibtypes.begin(), tibtypes.end());

                    /**
                     *  Add all the pips to draw to a vector.
                     */
                    for (auto& tibtuple : tibtypes)
                    {
                        const double amount = Storage.Get_Amount((TiberiumType)std::get<1>(tibtuple));
                        const double fraction = amount / ttype->Storage;
                        const int pip_count = ttype->Max_Pips() * fraction + 0.5;

                        int piptype = Extension::Fetch<TiberiumClassExtension>(Tiberiums[std::get<1>(tibtuple)])->PipIndex;
                        for (int i = 0; i < pip_count; i++)
                            pips_to_draw.emplace_back(piptype);
                    }
                }

                for (int index = 0; index < Class_Of()->Max_Pips(); index++)
                {
                    int shape = 0;
                    if (index < pips_to_draw.size())
                    {
                        shape = pips_to_draw[index];
                    }
                    CC_Draw_Shape(LogicSurface, NormalDrawer, pip_shapes, shape, &Point2D(drawx + dx * index, drawy + dy * index), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
                }
            }
            else if (ext->SpawnManager && ext->SpawnManager->SpawnCount > 0)
            {
                for (int index = 0; index < ext->SpawnManager->SpawnCount; index++)
                {
                    const int pip = index < ext->SpawnManager->Docked_Count() ? 1 : 0;
                    CC_Draw_Shape(LogicSurface, NormalDrawer, pip_shapes, pip, &Point2D(drawx + dx * index, drawy + dy * index), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
                }
            }
            else if (Techno_Type_Class()->PipScale == PIP_AMMO)
            {
                if (ttype_ext->PipWrap > 0)
                {
                    enum { PIP_AMMO_WRAP_FIRST = 7 };

                    const int wrap_count = Ammo / ttype_ext->PipWrap;
                    const int leftover = Ammo % ttype_ext->PipWrap;

                    for (int index = 0; index < ttype_ext->PipWrap; index++)
                    {
                        CC_Draw_Shape(LogicSurface, NormalDrawer, pips2, PIP_AMMO_WRAP_FIRST + wrap_count + (index < leftover), &Point2D(drawx + dx * index, drawy + dy * index - 3), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
                    }
                }
                else
                {
                    for (int index = 0; index < Class_Of()->Max_Pips() && pips > 0; index++, pips--)
                    {
                        CC_Draw_Shape(LogicSurface, NormalDrawer, pips2, 6, &Point2D(drawx + dx * index, drawy + dy * index - 3), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
                    }
                }
                
            }
            else if (Techno_Type_Class()->PipScale == PIP_CHARGE)
            {
                for (int index = 0; index < Class_Of()->Max_Pips(); index++)
                {
                    CC_Draw_Shape(LogicSurface, NormalDrawer, pip_shapes, index < pips ? 1 : 0, &Point2D(drawx + dx * index, drawy + dy * index), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
                }
            }
        }

        /**
         *  Display what group this unit belongs to. This corresponds to the team
         *  number assigned with the <CTRL> key.
         */
        if (Group < 10)
        {
            char buffer[12];

            Point2D drawpoint = bottomleft;
            drawpoint += UIControls->Get_Group_Number_Offset((RTTIType)What_Am_I(), Class_Of()->Max_Pips() > 0);

            int group = Group + 1;

            if (group == 10)
                group = 0;

            std::snprintf(buffer, std::size(buffer), "%d", group >= 10 ? 0 : group);
            const ColorSchemeType colorschemetype = Extension::Fetch<SideClassExtension>(Sides[PlayerPtr->Class->Side])->UIColor;
            Plain_Text_Print(buffer, LogicSurface, &rect, &drawpoint, COLOR_WHITE, COLOR_TBLACK, TPF_FULLSHADOW | TPF_EFNT, colorschemetype, 1);
        }
    }

    /**
     *  Because of ts-patches we have to check if the object is discovered by the player,
     *  or we'll draw pips for shrouded objects
     */
    if (IsDiscoveredByPlayer)
    {
        /**
         *  Special hack to display a red pip on the medic,
         *  or a custom pip.
         */
        const int specialpip = Extension::Fetch<TechnoTypeClassExtension>(Techno_Type_Class())->SpecialPipIndex;
        if (specialpip >= 0)
        {
            CC_Draw_Shape(LogicSurface, NormalDrawer, pips1, specialpip, &(Point2D(drawx, drawy) + UIControls->Get_Special_Pip_Offset((RTTIType)What_Am_I())), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
        }
        else if (What_Am_I() == RTTI_INFANTRY && Combat_Damage() < 0)
        {
            CC_Draw_Shape(LogicSurface, NormalDrawer, pips1, 6, &(Point2D(drawx, drawy) + UIControls->Get_Special_Pip_Offset((RTTIType)What_Am_I())), &rect, SHAPE_WIN_REL | SHAPE_CENTER);
        }

        /**
         *  Display whether this unit is a leader unit or not.
         */
        if (What_Am_I() != RTTI_BUILDING)
            Draw_Text_Overlay(Point2D(bottomleft.X - 10, bottomleft.Y + 10), bottomleft, rect);

        /**
         *  Display a veterancy pip is the unit is promoted.
         */
        int veterancy_shape = -1;
        if (Veterancy.Is_Veteran())
            veterancy_shape = 7;

        if (Veterancy.Is_Elite())
            veterancy_shape = 8;

        if (Veterancy.Is_Dumbass())
            veterancy_shape = 12;

        if (veterancy_shape != -1)
        {
            Point2D drawpoint = center;
            drawpoint += UIControls->Get_Veterancy_Pip_Offset((RTTIType)What_Am_I());
            CC_Draw_Shape(LogicSurface, NormalDrawer, pips1, veterancy_shape, &drawpoint, &rect, SHAPE_WIN_REL | SHAPE_CENTER);
        }
    }
}


/**
 *  This routine will compare the weapons this object is equipped with verses the
 *  candidate target object. The best weapon to use against the target will be returned.
 *  Special emphasis is given to weapons that can fire on the target without requiring
 *  this object to move within range.
 *
 *  @author: 08/12/1996 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
WeaponSlotType TechnoClassExt::_What_Weapon_Should_I_Use(TARGET target) const
{
    if (!Target_Legal(target))
        return WEAPON_SLOT_PRIMARY;

    bool webby_primary = false;
    bool webby_secondary = false;

    /**
     *  Fetch the armor of the candidate target object. Presume that if the target
     *  is not an object, then its armor is equivalent to none. Who knows why?
     */
    ArmorType armor = ARMOR_NONE;
    if (Is_Target_Object(target)) {
        armor = static_cast<ObjectClass*>(target)->Class_Of()->Armor;
    }

    /**
     *  Get the value of the primary weapon verses the candidate target. Increase the
     *  value of the weapon if it happens to be in range.
     */
    int w1 = 0;
    WeaponTypeClass const* wptr = Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon;
    if (wptr && wptr->WarheadPtr) {
        webby_primary = wptr->WarheadPtr->IsWebby;
        w1 = Verses::Get_Modifier(armor, wptr->WarheadPtr) * 1000;
    }
    if (In_Range_Of(target, WEAPON_SLOT_PRIMARY)) w1 *= 2;
    FireErrorType ok1 = Can_Fire(target, WEAPON_SLOT_PRIMARY);
    if (ok1 == FIRE_CANT || ok1 == FIRE_ILLEGAL || ok1 == FIRE_REARM) w1 = 0;

    /**
     *  Calculate a similar value for the secondary weapon.
     */
    int w2 = 0;
    wptr = Get_Weapon(WEAPON_SLOT_SECONDARY)->Weapon;
    if (wptr && wptr->WarheadPtr) {
        webby_secondary = wptr->WarheadPtr->IsWebby;
        w2 = Verses::Get_Modifier(armor, wptr->WarheadPtr) * 1000;
    }
    if (In_Range_Of(target, WEAPON_SLOT_SECONDARY)) w2 *= 2;
    FireErrorType ok2 = Can_Fire(target, WEAPON_SLOT_SECONDARY);
    if (ok2 == FIRE_CANT || ok2 == FIRE_ILLEGAL || ok2 == FIRE_REARM) w2 = 0;

    /**
     *  Return with the weapon identifier that should be used to fire upon the
     *  candidate target.
     */
    if (!webby_primary && !webby_secondary) {

        /**
         *  If neither weapon can shoot, but one can't because it's rearming, use the one that's rearming.
         */
        if (w1 == w2)  {
            if (ok1 == FIRE_REARM && (ok2 == FIRE_CANT || ok2 == FIRE_ILLEGAL)) {
                return WEAPON_SLOT_PRIMARY;
            }
            else if (ok2 == FIRE_REARM && (ok1 == FIRE_CANT || ok1 == FIRE_ILLEGAL)) {
                return WEAPON_SLOT_SECONDARY;
            }
        }

        return w2 > w1 ? WEAPON_SLOT_SECONDARY : WEAPON_SLOT_PRIMARY;
    }

    /**
     *  Determine if the target can be immobilized by the web weapon.
     *  Valid targets are infantry and the ground, except for destroyable cliffs and bridges.
     */
    enum
    {
        OVERLAY_LOBRDG01 = 0x4A,
        OVERLAY_LOBRDG26 = 0x63
    };

    bool immobilize = false;
    if (Is_Target_Object(target)) {
        ObjectClass* obj = static_cast<ObjectClass*>(target);
        if (obj->Is_Infantry()) {
            InfantryClass* inf = static_cast<InfantryClass*>(target);
            if (!inf->Is_Immobilized() && !inf->Class->IsWebImmune) {
                immobilize = true;
            }
        } else if (!obj->Is_Foot()) {
            immobilize = obj->What_Am_I() == RTTI_ISOTILE;
        }
    } else if (target->What_Am_I() == RTTI_CELL) {
        CellClass* cell = static_cast<CellClass*>(target);
        IsometricTileType tile = cell->Tile;
        if (tile != DestroyableCliff && tile != BlackTile && !cell->Bit2_16) {
            if (cell->Overlay < OVERLAY_LOBRDG01 || cell->Overlay > OVERLAY_LOBRDG26) { 
                immobilize = true;
            }
        }
    } else {
        immobilize = false;
    }

    return immobilize == webby_secondary ? WEAPON_SLOT_SECONDARY : WEAPON_SLOT_PRIMARY;
}


/**
 *  Patch in TechnoClass::Fire_At to order the spawner to fire at the target,
 *  as well as reveal it if the weapon is RevealOnFire.
 *
 *  @author: ZivDero
 */
bool TechnoClassExt::_Spawner_Fire_At(TARGET target, WeaponTypeClass* weapon)
{
    auto weapon_ext = Extension::Fetch<WeaponTypeClassExtension>(weapon);

    if (weapon_ext->IsSpawner)
    {
        auto techno_ext = Extension::Fetch<TechnoClassExtension>(this);
        techno_ext->SpawnManager->Queue_Target(target);
        if (IsOwnedByPlayer || IsDiscoveredByPlayer)
        {
            if (!Map.Is_Shrouded(Center_Coord()) && !Map.Is_Fogged(Center_Coord()))
                return true;

            if (What_Am_I() == RTTI_AIRCRAFT && IsOwnedByPlayer)
                return true;
        }

        HouseClass* target_house = target->Owning_House();
        if (target_house && target_house->Is_Player_Control())
        {
            if (weapon_ext->IsRevealOnFire)
            {
                Map.Sight_From(Center_Coord(), 3, target_house);
            }
        }

        return true;
    }

    return false;
}


/**
 *  Reimplementation of TechnoClass::Target_Something_Nearby with adjustments
 *  for the spawner.
 *
 *  @author: ZivDero
 */
bool TechnoClassExt::_Target_Something_Nearby(Coordinate& coord, ThreatType threat)
{
    /**
     *  Determine that if there is an existing target it is still legal
     *  and within range.
     */
    if (Target_Legal(TarCom))
    {
        // This bit is roughly ported from YR but is missing `ShouldLoseTargetNow`
        if (threat & THREAT_RANGE)
        {
            WeaponSlotType primary = What_Weapon_Should_I_Use(TarCom);
            FireErrorType fire = Can_Fire(TarCom, primary);

            if (fire == FIRE_CANT)
            {
                SpawnManagerClass* spawn_manager = Extension::Fetch<TechnoClassExtension>(this)->SpawnManager;
                if (spawn_manager)
                    spawn_manager->Abandon_Target();

                Assign_Target(nullptr);
            }
            else if (fire == FIRE_ILLEGAL || fire == FIRE_RANGE)
            {
                Assign_Target(nullptr);
            }
        }
    }

    /**
     *  If there is no target, then try to find one and assign it as
     *  the target for this unit.
     */
    if (!Target_Legal(TarCom)) {
        Assign_Target(Greatest_Threat(threat & (THREAT_RANGE | THREAT_AREA), coord));
    }

    /**
     *  Return with answer to question: Does this unit now have a target?
     */
    return Target_Legal(TarCom);
}


/**
 *  Reimplementation of TechnoClass::Stun with adjustments for the spawner.
 *
 *  @author: ZivDero
 */
void TechnoClassExt::_Stun()
{
    Assign_Target(nullptr);
    Assign_Destination(nullptr);
    Transmit_Message(RADIO_OVER_OUT);

    const auto extension = Extension::Fetch<TechnoClassExtension>(this);
    if (extension->SpawnManager)
    {
        extension->SpawnManager->Detach_Spawns();
        extension->SpawnManager->Abandon_Target();
    }

    Detach_All(true);
    Unselect();
}


/**
 *  Wrapper function to patch the call in TechnoClass::AI to call
 *  SpawnManagerClass::AI.
 *
 *  @author: ZivDero
 */
void TechnoClassExt::_Mission_AI()
{
    MissionClass::AI();

    const auto extension = Extension::Fetch<TechnoClassExtension>(this);

    if (extension->SpawnManager)
        extension->SpawnManager->AI();
}


/**
 *  Determines if this techno object can fire.
 *
 *  @author: 12/23/1994 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 *           Rampastring - Added support for torpedoes.
 */
FireErrorType TechnoClassExt::_Can_Fire(TARGET target, WeaponSlotType which)
{
    /**
     *  Don't allow firing if the target is illegal.
     */
    if (!Target_Legal(target))
        return FIRE_ILLEGAL;

    const auto ext = Extension::Fetch<TechnoClassExtension>(this);

    /**
     *  If this unit is a spawner, don't let it fire if it's currently in the process of spawning.
     */
    if (ext->SpawnManager && ext->SpawnManager->Preparing_Count())
        return FIRE_BUSY;

    ObjectClass* object = Target_As_Techno(target);

    /**
     *  If the object is completely cloaked, then you can't fire on it.
     */
    if (object && object->Visual_Character(true, House) == VISUAL_HIDDEN
        && !Map[target->Center_Coord()].Sensed_By(static_cast<HousesType>(House->Get_Heap_ID()))
        && object->Owning_House() != House
        && (Combat_Damage() > 0 || !object->Owning_House()->Is_Ally(House)))
    {
        return FIRE_CANT;
    }

    /**
     *  A falling object is too busy falling to fire.
     */
    if (IsFalling)
        return FIRE_CANT;

    /**
     *  An immobilized object can't fire, unless it's a visceroid.
     */
    if (Is_Immobilized())
    {
        if (What_Am_I() != RTTI_UNIT
            || !(reinterpret_cast<UnitClass*>(this)->Class->IsLargeVisceroid
            || reinterpret_cast<UnitClass*>(this)->Class->IsSmallVisceroid))
        {
            return FIRE_CANT;
        }
    }

    /**
     *  If there is no weapon, then firing is not allowed.
     */
    WeaponTypeClass const* weapon = Get_Weapon(which)->Weapon;
    if (!weapon)
        return FIRE_CANT;

    /**
     *  If the weapon is ion sensitive and there's an active Ion Storm,
     *  then firing is not allowed.
     */
    if (weapon->IsIonSensitive && IonStorm_Is_Active())
        return FIRE_CANT;

    /**
     *  #issue-444
     *
     *  If the weapon fires torpedoes and the target is on land,
     *  then firing is not allowed.
     */
    const auto bullettypeext = Extension::Fetch<BulletTypeClassExtension>(weapon->Bullet);
    if (bullettypeext->IsTorpedo) {
        if (Map[target->Center_Coord()].Land_Type() != LAND_WATER) {
            return FIRE_CANT;
        }
    }

    /**
     *  If the weapon is a spawner, it needs to have an object ready to spawn.
     */
    if (weapon && Extension::Fetch<WeaponTypeClassExtension>(weapon)->IsSpawner)
    {
        const auto techno_ext = Extension::Fetch<TechnoClassExtension>(this);

        if (techno_ext->SpawnManager == nullptr)
        {
            Vinifera_DeveloperMode_Warning_WWMessageBox("[FATAL] You have a weapon that has Spawner=yes but you didn't define Spawns= in the techno.");
            Fatal("[FATAL] You have a weapon that has Spawner=yes but you didn't define Spawns= in the techno.");
        }

        if (techno_ext->SpawnManager->Active_Count() == 0)
            return FIRE_REARM;
    }

    /**
     *  If we're firing our primary particle-based/wave/railgun weapon, then
     *  we can't fire our secondary weapon of the same kind.
     */
    WeaponTypeClass const* other_weapon = Get_Weapon(which == WEAPON_SLOT_PRIMARY ? WEAPON_SLOT_SECONDARY : WEAPON_SLOT_PRIMARY)->Weapon;
    
    if (other_weapon)
    {
        if ((other_weapon->IsSonic && Wave)
            || (other_weapon->IsRailgun && ParticleSystems[4])
            || (other_weapon->IsUseFireParticles && ParticleSystems[0])
            || (other_weapon->IsUseSparkParticles && ParticleSystems[1]))
        {
            return FIRE_CANT;
        }
    }

    /**
     *  Can only fire anti-aircraft weapons against aircraft unless the aircraft is
     *  sitting on the ground. If the object is on the ground,
     *  then don't allow firing if it can't fire upon ground objects.
     */
    if (target->In_Air() && !weapon->Bullet->IsAntiAircraft ||
        target->On_Ground() && !weapon->Bullet->IsAntiGround)
    {
        return FIRE_CANT;
    }

    /**
     *  Check if the unit has synchronized shooting.
     */
    bool check_rearm = true;
    if (which != WEAPON_SLOT_SECONDARY && What_Am_I() == RTTI_UNIT)
    {
        const auto unit = reinterpret_cast<UnitClass*>(this);
        const int burst = CurrentBurstIndex % weapon->Burst;
        if (burst < 2)
        {
            if (unit->Class->FiringSyncFrame[burst] != -1
                && unit->FiringSyncDelay != -1)
            {
                if (unit->Class->FiringSyncFrame[burst] != unit->FiringSyncDelay)
                    return FIRE_REARM;
                
                check_rearm = false;
            }
        }
    }

    /**
     *  Don't allow firing if still rearming.
     */
    if (check_rearm && Arm != 0)
        return FIRE_REARM;

    /**
     *  The target must be within range in order to allow firing.
     */
    if (!In_Range_Of(target, which))
        return FIRE_RANGE;

    /**
     *  If the object has an armor type that this unit's warhead is forbidden to fire at, bail.
     */
    if (object && !Verses::Get_ForceFire(object->Techno_Type_Class()->Armor, weapon->WarheadPtr))
    {
        return FIRE_ILLEGAL;
    }

    /**
     *  If there is no ammo left, then it can't fire.
     */
    if (!Ammo)
        return FIRE_AMMO;

    /**
     *  If cloaked, then firing is disabled.
     */
    if (Cloak != UNCLOAKED && (What_Am_I() != RTTI_AIRCRAFT || Cloak == CLOAKED))
        return FIRE_CLOAKED;

    /**
     *  Hunter seekers can't fire since they need to kamikaze into the target.
     */
    if (Techno_Type_Class()->IsHunterSeeker)
        return FIRE_RANGE;

    return FIRE_OK;
}


/**
 *  Determines if the object can move be moved by player.
 *
 *  @author: 01/19/1995 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
bool TechnoClassExt::_Can_Player_Move() const
{
    if (!House->Is_Player_Control())
        return false;

    if (Is_Immobilized())
        return false;

    const auto ext = Extension::Fetch<TechnoClassExtension>(this);
    if (ext->SpawnManager)
    {
        const auto typeext = Extension::Fetch<TechnoTypeClassExtension>(Techno_Type_Class());
        if (ext->SpawnManager->Preparing_Count() > 0 && ext->SpawnManager->Preparing_Count() < typeext->SpawnsNumber)
            return false;
    }

    return true;
}


/**
 *  Wrapper for TechnoClassExtension::Fire_Coord.
 */
Coordinate TechnoClassExt::_Fire_Coord(WeaponSlotType which) const
{
    return Extension::Fetch<TechnoClassExtension>(this)->Fire_Coord(which, TPoint3D<int>());
}


/**
 *  Checks object to see if it can retaliate.
 *  
 *  This routine is called when this object has suffered some damage and it needs to know
 *  if it should fight back. The object that caused the damage is specified as a parameter.
 *
 *  @author: 10/19/1996 JLB - Created. 
 *           ZivDero - Adjustments for Tiberian Sun.
 */
bool TechnoClassExt::_Is_Allowed_To_Retaliate(TechnoClass* source, WarheadTypeClass const* warhead) const
{
    /**
     *  #issue-594
     *
     *  Implements IsCanRetaliate for TechnoTypes.
     *
     *  @author: CCHyper
     */

    const TechnoTypeClass* ttype = Techno_Type_Class();

    /**
     *  If this unit is flagged as not being allowed to retaliate to attacks, return false.
     */
    if (!Extension::Fetch<TechnoTypeClassExtension>(ttype)->IsCanRetaliate)
        return false;

    /**
     *  Human-controlled units that have a target don't retaliate.
     */
    if (House->Is_Human_Control() && Target_Legal(TarCom))
        return false;

    /**
     *  If the source of the damage is a Veinhole, retaliate, unless this is a player-controlled
     *  ground unit and it's moving somewhere.
     */
    if (warhead != nullptr && warhead->IsVeinhole)
    {
        const bool is_foot_with_nav = Is_Foot() && reinterpret_cast<FootClass const*>(this)->NavCom;
        if (!is_foot_with_nav || !House->Is_Human_Control())
            return true;
    }

    /**
     *  If there is no source of the damage, then retaliation cannot occur.
     */
    if (source == nullptr)
        return false;

    /**
     *  If the current mission doesn't allow retaliation, return false;
     */
    if (!Get_Current_Mission_Control().IsRetaliate)
        return false;

    /**
     *  If the source of the damage is an ally, then retaliation shouldn't
     *  occur either.
     */
    if (House->Is_Ally(source))
        return false;

    /**
     *  Only objects that have a damaging weapon are allowed to retaliate.
     */
    if (Combat_Damage() <= 0 || !Is_Weapon_Equipped())
        return false;

    /**
     *  If this is not equipped with a weapon that can attack the molester, then
     *  don't allow retaliation.
     */
    const WeaponInfoStruct* weapon_info = Get_Weapon(What_Weapon_Should_I_Use(source));
    if (weapon_info->Weapon->WarheadPtr &&
        Verses::Get_Modifier(source->Techno_Type_Class()->Armor, weapon_info->Weapon->WarheadPtr) == 0.0)
    {
        return false;
    }

    /**
     *  Don't allow retaliation if it isn't equipped with a weapon that can deal with the threat.
     */
    if (source->What_Am_I() == RTTI_AIRCRAFT && !weapon_info->Weapon->Bullet->IsAntiAircraft) return false;

    /**
     *  Units with C4 are not allowed to retaliate against buildings in the normal sense while in guard mode. That
     *  is, unless it is owned by the computer. Normally, units with C4 can't do anything substantial to a building
     *  except to blow it up.
     */
    if (House->Is_Human_Control() && source->What_Am_I() == RTTI_BUILDING)
    {
        if (What_Am_I() == RTTI_INFANTRY && static_cast<InfantryTypeClass const*>(ttype)->IsBomber)
            return false;

        if (Veterancy.Is_Veteran() && ttype->VeteranAbilities[ABILITY_C4])
            return false;
        
        if (Veterancy.Is_Elite() && (ttype->VeteranAbilities[ABILITY_C4] || ttype->EliteAbilities[ABILITY_C4]))
            return false;
    }

    /**
     *  Artillery that need to deploy to fire don't retaliate.
     */
    if (House->Is_Human_Control() && What_Am_I() == RTTI_UNIT)
    {
        const BuildingTypeClass* deploys_into = reinterpret_cast<UnitClass const*>(this)->Class->DeploysInto;
        if (deploys_into && deploys_into->IsArtillary)
            return false;
    }

    /**
     *  If a human house is not allowed to retaliate automatically, then don't
     */
    if (House->Is_Human_Control() && !Rule->IsSmartDefense && What_Am_I() != RTTI_BUILDING)
    {
        if (Mission != MISSION_GUARD_AREA && Mission != MISSION_GUARD && Mission != MISSION_PATROL)
            return false;
    }

    /**
     *  If this object is part of a team that prevents retaliation then don't allow retaliation.
     */
    if (Is_Foot() && reinterpret_cast<FootClass const*>(this)->Team && reinterpret_cast<FootClass const*>(this)->Team->Class->IsSuicide)
        return false;

    /**
     *  Compare potential threat of the current target and the potential new target. Don't retaliate
     *  if it is currently attacking the greater threat.
     */
    if (!House->Is_Human_Control() && Target_Legal(TarCom) && Is_Target_Object(TarCom))
    {
        const float current_val = Target_Threat(static_cast<TechnoClass*>(TarCom), Coordinate());
        const float source_val = Target_Threat(source, Coordinate());

        if (source_val < current_val)
            return false;
    }

    /**
     *  The warhead may forbid the unit from retaliating against targets with some armor types.
     */
    if (weapon_info->Weapon->WarheadPtr != nullptr &&
        !Verses::Get_Retaliate(source->Techno_Type_Class()->Armor, weapon_info->Weapon->WarheadPtr))
    {
        return false;
    }

    /**
     *  All checks passed, so return that retaliation is allowed.
     */
    return true;
}


/**
 *  Calculates the threat the target object poses.
 *
 *  @author: ZivDero
 */
double TechnoClassExt::_Target_Threat(TechnoClass* target, Coordinate& firing_coord) const
{
    double target_effectiveness_coefficient;
    double target_special_threat_coefficient;
    double my_effectiveness_coefficient;
    double target_strength_coefficient;
    double target_distance_coefficient;

    const TechnoTypeClass* ttype = Techno_Type_Class();

    /**
     *  Nothing is not a threat.
     */
    if (!target->Class_Of())
        return 0.0;
    
    if (House->IsThreatRatingNodeActive)
    {
        my_effectiveness_coefficient = ttype->MyEffectivenessCoefficient;
        target_effectiveness_coefficient = ttype->TargetEffectivenessCoefficient;
        target_special_threat_coefficient = ttype->TargetSpecialThreatCoefficient;
        target_strength_coefficient = ttype->TargetStrengthCoefficient;
        target_distance_coefficient = ttype->TargetDistanceCoefficient;
    }
    else
    {
        my_effectiveness_coefficient = Rule->DumbMyEffectivenessCoefficient;
        target_effectiveness_coefficient = Rule->DumbTargetEffectivenessCoefficient;
        target_special_threat_coefficient = Rule->DumbTargetSpecialThreatCoefficient;
        target_strength_coefficient = Rule->DumbTargetStrengthCoefficient;
        target_distance_coefficient = Rule->DumbTargetDistanceCoefficient;
    }

    double threat = 0.0;

    if (Target_As_Techno(target, false))
    {
        /**
         *  Determine how good is the target at shooting at us.
         */
        const WeaponTypeClass* target_weapon = target->Get_Weapon(target->What_Weapon_Should_I_Use((TARGET)this))->Weapon;
        if (target_weapon && target_weapon->WarheadPtr)
        {
            if (target->TarCom == this)
                threat = -(target_effectiveness_coefficient * Verses::Get_Modifier(ttype->Armor, target_weapon->WarheadPtr));
            else
                threat = target_effectiveness_coefficient * Verses::Get_Modifier(ttype->Armor, target_weapon->WarheadPtr);
        }

        /**
         *  Add the special threat value.
         */
        threat += target_special_threat_coefficient * target->Techno_Type_Class()->SpecialThreatValue;

        /**
         *  The enemy house extra gets priority.
         */
        if (House->Enemy != HOUSE_NONE && House->Enemy == target->House->Get_Heap_ID())
            threat += Rule->EnemyHouseThreatBonus;
    }

    /**
     *  Determine how effective our shooting at the target would be.
     */
    const WeaponTypeClass* weapon = Get_Weapon(What_Weapon_Should_I_Use(target))->Weapon;
    if (weapon && weapon->WarheadPtr)
        threat += my_effectiveness_coefficient * Verses::Get_Modifier(target->Class_Of()->Armor, weapon->WarheadPtr);

    /**
     *  Adjust threat based on how healthy the target is.
     */
    threat += target->Health_Ratio() * target_strength_coefficient;

    /**
     *  Adjust threat if the target is outside our threat range.
     */
    int dist;
    if (firing_coord == Coordinate())
        dist = (Center_Coord() - target->Center_Coord()).Length() / 256;
    else
        dist = (firing_coord - target->Center_Coord()).Length();

    const int threat_range = (weapon ? weapon->Range : Techno_Type_Class()->ThreatRange) / 256;
    threat += std::max(0, dist - threat_range) * target_distance_coefficient;

    return threat + 100000.0;
}


/**
 *  Calculates the anti-infantry strength of this object.
 *
 *  This routine is used to determine the anti-infantry strength of this object.
 *  The typical user of this routine is the expert system base defense AI.              
 *
 *  @author: 10/02/1995 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 */
int TechnoClassExt::_Anti_Infantry() const
{
    if (Is_Weapon_Equipped())
    {
        if (Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon->Bullet->IsAntiGround)
            return 0;

        constexpr int minrange = 0x0400;
        WeaponTypeClass const* weapon = Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon;
        BulletTypeClass const* bullet = weapon->Bullet;
        const int mrange = std::min(static_cast<int>(weapon->Range), minrange);

        int value = ((weapon->Attack * Verses::Get_Modifier(ARMOR_NONE, weapon->WarheadPtr)) * mrange * weapon->WarheadPtr->SpreadFactor) / weapon->ROF;
        if (Techno_Type_Class()->Is_Two_Shooter())
            value *= 2;
        
        if (bullet->IsInaccurate)
            value /= 2;
        
        return value / 50;
    }

    return 0;
}


/**
 *  Wrapper to post-process the result of TechnoClass::What_Action
 *
 *  @author: ZivDero
 */
ActionType TechnoClassExt::_What_Action(ObjectClass* object, bool disallow_force)
{
    ActionType action = TechnoClass::What_Action(object, disallow_force);

    if (action == ACTION_ATTACK)
    {
        const bool ctrldown = WWKeyboard->Down(Options.KeyForceAttack1) || WWKeyboard->Down(Options.KeyForceAttack2);
        const FireErrorType error = Can_Fire(object, What_Weapon_Should_I_Use(object));

        if (error == FIRE_ILLEGAL && !ctrldown)
            return ACTION_NONE;
    }

    return action;
}


/**
 *  Reimplements part of TechnoClass::Take_Damage that is responsible for dropping Tiberium
 *  from a unit's storage to drop the Tiberium that the unit contains, and not always Riparius.
 *
 *  @author: ZivDero
 */
void TechnoClassExt::_Drop_Tiberium()
{
    /**
     *  Vanilla drops Tiberium in a specific order of directions, we replicate that.
     */
    static FacingType drop_facings[9] = { FACING_NONE, FACING_E, FACING_NW, FACING_NE, FACING_S, FACING_SE, FACING_N, FACING_SW, FACING_W };

    if (Storage.Get_Total_Amount() > 0 && What_Am_I() != RTTI_BUILDING && !Scen->SpecialFlags.IsHarvesterImmune)
    {
        TiberiumType droplist[9] = { TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE };
        int dropcount = 0;

        /**
         *  Calculate how many cells of Tiberium we want to drop per type.
         */
        for (int i = 0; i < Tiberiums.Count(); i++)
        {
            double amount = Storage.Get_Amount(static_cast<TiberiumType>(i));
            amount = amount / Techno_Type_Class()->Storage * std::size(drop_facings);
            for (int j = 0; j < amount && dropcount < std::size(droplist); j++)
                droplist[dropcount++] = static_cast<TiberiumType>(i);
        }

        const Cell center_cell = Coord_Cell(Center_Coord());

        /**
         *  Drop Tiberium around the harvester.
         */
        for (int i = 0; i < std::size(drop_facings) && i < dropcount; i++)
        {
            Cell adjacent_cell = Adjacent_Cell(center_cell, drop_facings[i]);
            CellClass& cell = Map[adjacent_cell];

            const int tib_frame = Random_Pick(0, 2);
            cell.Place_Tiberium(droplist[i], tib_frame);
        }
    }
}


/**
 *  Records the death of this object.
 *
 *  @author:  07/08/1995 JLB - Created.
 *            ZivDero - Adjustments for Tiberian Sun.
 */
void TechnoClassExt::_Record_The_Kill(TechnoClass* source)
{
    int total_recorded = 0;
    const int points = Techno_Type_Class()->Cost_Of(House);

    const auto typeext = Extension::Fetch<TechnoTypeClassExtension>(Techno_Type_Class());

    /**
     *  Handle any trigger event associated with this object.
     */
    if (IsActive && Tag && source) Tag->Spring(TEVENT_ATTACKED, this);

    if (IsActive && Tag && source) Tag->Spring(TEVENT_DISCOVERED, this);

    if (IsActive && What_Am_I() != RTTI_UNIT && Tag) Tag->Spring(TEVENT_DESTROYED, this);

    if (IsActive && What_Am_I() != RTTI_UNIT && Tag) Tag->Spring(TEVENT_DESTROYED_BY_ANYTHING, this);

    if (IsActive && What_Am_I() != RTTI_UNIT && Tag) Tag->Spring(TEVENT_FAKES_DESTROYED, this);

    if (source && !typeext->IsDontScore && !House->Is_Ally(source) && !source->House->Is_Ally(this)) {

        const auto source_ext = Extension::Fetch<TechnoClassExtension>(source);
        const auto source_typeext = Extension::Fetch<TechnoTypeClassExtension>(source->Techno_Type_Class());

        if (source->Techno_Type_Class()->IsTrainable) {
            source->Veterancy.Gain_Experience(source->Techno_Type_Class()->Cost_Of(House), points);

        } else if (source_typeext->IsMissileSpawn) {

            if (source_ext->SpawnOwner && source_ext->SpawnOwner->Techno_Type_Class()->IsTrainable) {
                source_ext->SpawnOwner->Veterancy.Gain_Experience(source->Techno_Type_Class()->Cost_Of(House), points);
            }
        }

        House->WhoLastHurtMe = source->Owner();

        /**
         *  Add up the score for killing this unit
         */
        source->House->PointTotal += points;
    }

    switch (What_Am_I()) {
    case RTTI_BUILDING:
    {
        if (!Techno_Type_Class()->IsInsignificant) {

            if (reinterpret_cast<BuildingClass*>(this)->WhoLastHurtMe != HOUSE_NONE) {
                House->BuildingsLost++;
            }
        }

        if (source) {
            if (!typeext->IsDontScore) {
                source->House->DestroyedBuildings->Increment_Unit_Total(reinterpret_cast<BuildingClass*>(this)->Class->Type);
            }
            source->House->BuildingsKilled[Owner()]++;
        }

        /**
         *  If the map is displaying the multiplayer player names & their
         *  # of kills, tell it to redraw.
         */
        if (Map.Is_Player_Names()) {
            Map.Player_Names(false);
        }
    }
    break;

    case RTTI_AIRCRAFT:
        if (source && !typeext->IsDontScore) {
            source->House->DestroyedAircraft->Increment_Unit_Total(reinterpret_cast<AircraftClass*>(this)->Class->Type);
            total_recorded++;
        }
        // Fall through.....
    case RTTI_INFANTRY:
        if (source && !total_recorded && !typeext->IsDontScore) {
            source->House->DestroyedInfantry->Increment_Unit_Total(reinterpret_cast<InfantryClass*>(this)->Class->Type);
            total_recorded++;
        }
        // Fall through.....
    case RTTI_UNIT:
        if (source && !total_recorded && !typeext->IsDontScore) {
            source->House->DestroyedUnits->Increment_Unit_Total(reinterpret_cast<UnitClass*>(this)->Class->Type);
        }

        House->UnitsLost++;
        if (source && !typeext->IsDontScore) source->House->UnitsKilled[Owner()]++;

        /**
         *  If the map is displaying the multiplayer player names & their
         *  # of kills, tell it to redraw.
         */
        if (Map.Is_Player_Names()) {
            Map.Player_Names(false);
        }
        break;

    default:
        break;
    }
}



/**
 *  #issue-1087
 *
 *  Calculates the cell-based distance between this object and another object.
 *  Cell-based distance does not take leptons into account, only cell coordinates.
 *
 *  The original game's distance functions, such as Distance_Squared, also take leptons into
 *  account, which can lead into overflows that have bad consequences.
 *  For example, a harvester looking for a refinery on a big 256x256 sized map can believe
 *  that a refinery on the other side of the map would be next to it.
 *
 *  @author: Rampastring
 */
int TechnoClassExt::_Cell_Distance_Squared(const AbstractClass* object) const
{
    if (!object)
        return 0;

    Coordinate our_coord = Center_Coord();
    Coordinate their_coord = object->Center_Coord();

    int our_cell_x = our_coord.X / CELL_LEPTON_W;
    int their_cell_x = their_coord.X / CELL_LEPTON_W;
    int our_cell_y = our_coord.Y / CELL_LEPTON_H;
    int their_cell_y = their_coord.Y / CELL_LEPTON_H;

    int x_distance = our_cell_x - their_cell_x;
    int y_distance = our_cell_y - their_cell_y;
    return x_distance * x_distance + y_distance * y_distance;
}


/**
 *  Reimplementation of TechnoClass::Draw_Target_Laser().
 * 
 *  @author: CCHyper
 */
void TechnoClassExt::_Draw_Target_Laser() const
{
    if (!TarCom) {
        return;
    }

    /**
     *  Fetch the line properties.
     */
    const bool is_dashed = TarCom ? UIControls->IsTargetLaserDashed : UIControls->IsMovementLineDashed;
    const bool is_thick = TarCom ? UIControls->IsTargetLaserThick : UIControls->IsMovementLineThick;
    const bool is_dropshadow = TarCom ? UIControls->IsTargetLaserDropShadow : UIControls->IsMovementLineDropShadow;

    const unsigned tarcom_color = DSurface::RGB_To_Pixel(
        UIControls->TargetLaserColor.R,
        UIControls->TargetLaserColor.G,
        UIControls->TargetLaserColor.B);

    const unsigned tarcom_drop_color = DSurface::RGB_To_Pixel(
        UIControls->TargetLaserDropShadowColor.R,
        UIControls->TargetLaserDropShadowColor.G,
        UIControls->TargetLaserDropShadowColor.B);

    const unsigned navcom_color = DSurface::RGB_To_Pixel(
        UIControls->MovementLineColor.R,
        UIControls->MovementLineColor.G,
        UIControls->MovementLineColor.B);

    const unsigned navcom_drop_color = DSurface::RGB_To_Pixel(
        UIControls->MovementLineDropShadowColor.R,
        UIControls->MovementLineDropShadowColor.G,
        UIControls->MovementLineDropShadowColor.B);

    const unsigned line_color = TarCom ? tarcom_color : navcom_color;
    const unsigned drop_color = TarCom ? tarcom_drop_color : navcom_drop_color;

    int point_size = 2;
    Point2D point_offset(-1, -1);

    if (is_thick) {
        point_size = 4;
        point_offset = Point2D(-2, -2);
    }

    /**
     *  Fetch the target laser line start and end coord.
     */
    Coordinate start_coord = entry_28C();
    Coordinate end_coord = func_638AF0();

    /**
     *  Convert the world coord to screen pixel.
     */
    Point2D start_point;
    Point2D end_point;
    TacticalMap->Coord_To_Pixel(start_coord, start_point);
    TacticalMap->Coord_To_Pixel(end_coord, end_point);

    /**
     *  Offset pixel position relative to tactical viewport.
     */
    start_point += Point2D(TacticalRect.X, TacticalRect.Y);
    end_point += Point2D(TacticalRect.X, TacticalRect.Y);

    /**
     *  Draw the target laser line.
     */
    if (Clip_Line(start_point, end_point, TacticalRect)) {

        Point2D drop_start_point = start_point;
        Point2D drop_end_point = end_point;

        drop_start_point.Y += 1;
        drop_end_point.Y += 1;

        if (is_dashed) {

            /**
             *  1 pixel on, 1 off, 1 on, 1 off...
             */
            static bool _pattern[] = { true, false, true, false, true, false, true, false, true, false, true, false, true, false, true, false };

            /**
             *  Adjust the offset of the line pattern.
             */
            int offset = 7 * Frame % 16;

            /**
             *  Draw the drop shadow line.
             */
            if (is_dropshadow) {

                if (is_thick) {
                    drop_start_point.Y += 1;
                    drop_end_point.Y += 1;
                }

                CompositeSurface->Draw_Dashed_Line(drop_start_point, drop_end_point, drop_color, _pattern, offset);

                if (is_thick) {
                    drop_start_point.Y += 1;
                    drop_end_point.Y += 1;
                    CompositeSurface->Draw_Dashed_Line(drop_start_point, drop_end_point, drop_color, _pattern, offset);
                }

            }

            /**
             *  Draw the dashed target laser line.
             */
            CompositeSurface->Draw_Dashed_Line(start_point, end_point, line_color, _pattern, offset);

            if (is_thick) {
                start_point.Y += 1;
                end_point.Y += 1;
                CompositeSurface->Draw_Dashed_Line(start_point, end_point, line_color, _pattern, offset);
            }

        }
        else {

            /**
             *  Draw the drop shadow line.
             */
            if (is_dropshadow) {

                if (is_thick) {
                    drop_start_point.Y += 1;
                    drop_end_point.Y += 1;
                }

                CompositeSurface->Draw_Line(drop_start_point, drop_end_point, drop_color);

                if (is_thick) {
                    drop_start_point.Y += 1;
                    drop_end_point.Y += 1;
                    CompositeSurface->Draw_Line(drop_start_point, drop_end_point, drop_color);
                }

            }

            /**
             *  Draw the target laser line.
             */
            CompositeSurface->Draw_Line(start_point, end_point, line_color);

            if (is_thick) {
                start_point.Y += 1;
                end_point.Y += 1;
                CompositeSurface->Draw_Line(start_point, end_point, line_color);
            }

        }

    }

    /**
     *  Draw the target laser line start and end squares.
     */
    if (is_dropshadow) {

        const int drop_point_size = is_thick ? (point_size + 3) : (point_size + 2);
        const Point2D drop_point_offset = is_thick ? (point_offset + Point2D(-2, -2)) : (point_offset + Point2D(-1, -1));

        if (is_thick) {
            point_size -= 1;
        }

        Rect drop_start_point_rect = TacticalRect.Intersect_With(Rect(start_point + drop_point_offset, drop_point_size, drop_point_size));
        CompositeSurface->Fill_Rect(drop_start_point_rect, drop_color);

        Rect drop_end_point_rect = TacticalRect.Intersect_With(Rect(end_point + drop_point_offset, drop_point_size, drop_point_size));
        CompositeSurface->Fill_Rect(drop_end_point_rect, drop_color);
    }

    Rect start_point_rect = TacticalRect.Intersect_With(Rect(start_point + point_offset, point_size, point_size));
    CompositeSurface->Fill_Rect(start_point_rect, line_color);

    Rect end_point_rect = TacticalRect.Intersect_With(Rect(end_point + point_offset, point_size, point_size));
    CompositeSurface->Fill_Rect(end_point_rect, line_color);
}


/**
 *  Reimplements TechnoClass::Draw_Text_Overlay
 *
 *  @author: ZivDero
 */
void TechnoClassExt::_Draw_Text_Overlay(Point2D& point1, Point2D& point2, Rect& rect) const
{
    static char buffer[128];
    const ColorSchemeType colorschemetype = Extension::Fetch<SideClassExtension>(Sides[PlayerPtr->Class->Side])->UIColor;

    /**
     *  Print the Power/Drain text on power plants.
     */
    if (What_Am_I() == RTTI_BUILDING && reinterpret_cast<const BuildingClass*>(this)->Class->Power > 0)
    {
        const auto owner = Owning_House();
        std::sprintf(buffer, Fetch_String(TXT_POWER_DRAIN), owner->Power_Output(), owner->Power_Drain());
        Plain_Text_Print(buffer, LogicSurface, &rect, &point2, COLOR_WHITE, COLOR_TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_EFNT, colorschemetype, 1);
    }

    /**
     *  Print the "Primary" text.
     */
    if (IsLeader)
    {
        const int text = What_Am_I() == RTTI_BUILDING && reinterpret_cast<const BuildingClass*>(this)->Class->Width() == 1 ? TXT_PRI : TXT_PRIMARY;
        Plain_Text_Print(text, LogicSurface, &rect, &point2, COLOR_WHITE, COLOR_TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_EFNT, colorschemetype, 1);
    }
}


/**
 *  Fetches the kind of crew this object contains.
 *
 *  @author:  07/29/1995 JLB - Created
 *            ZivDero - Adjustments for Tiberian Sun
 */
const InfantryTypeClass* TechnoClassExt::_Crew_Type() const
{
    /**
     *  If this object contains no crew, then there can be no
     *  crew inside, duh... return this news.
     */
    if (!Techno_Type_Class()->IsCrew) {
        return nullptr;
    }

    /**
     *  If we don't know what side this belongs to, have a technician exit it.
     */
    if (House->Class->Side == SIDE_NONE) {
        return Rule->Technician;
    }

    /**
     *  If it's armed, it could also have a technician exit it.
     */
    if (Is_Weapon_Equipped() && Percent_Chance(15)) {
        return SideClassExtension::Get_Technician(House);
    }

    /**
     *  The normal infantry survivor is this side's standard issue infantry.
     */
    return SideClassExtension::Get_Crew(House);
}


/**
 *  Determine the number of survivors to escape.
 *
 *  @author: 08/04/1996 JLB - Created
 *           ZivDero - Adjustments for Tiberian Sun
 */
int TechnoClassExt::_How_Many_Survivors() const
{
    if (Techno_Type_Class()->IsCrew) {
        return Extension::Fetch<TechnoTypeClassExtension>(Techno_Type_Class())->CrewCount;
    }

    return 0;
}


/**
 *  #issue-977
 *
 *  Adds check for IsLegalTargetComputer when evaluating a target for attacking.
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Evaluate_Object_Is_Legal_Target_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, edi);
    GET_REGISTER_STATIC(const TechnoClass *, object, esi); // The target object being evaluated.
    static TechnoClassExtension *this_technoext;
    static TechnoClassExtension *object_technoext;
    static const TechnoTypeClass *object_tclass;
    static const TechnoTypeClassExtension *object_tclassext;

    //this_technoext = Extension::Fetch<TechnoClassExtension>(this_ptr);
    //object_technoext = Extension::Fetch<TechnoClassExtension>(object);

    object_tclass = object->Techno_Type_Class();
    object_tclassext = Extension::Fetch<TechnoTypeClassExtension>(object_tclass);

    /**
     *  Determine if the target is theoretically allowed to be a target.
     */
    if (!object_tclass->IsLegalTarget) {
        goto return_false;
    }

    /**
     *  Now, determine if "we" are owned by a non-human house and the target is not theoretically allowed to be a target.
     */
    if (!this_ptr->House->Is_Human_Control() && !object_tclassext->IsLegalTargetComputer) {
        goto return_false;
    }

    /**
     *  Target object passed the "theoretical" check, for now...
     */
continue_eval:
    JMP_REG(edx, 0x0062D4D8);

    /**
     *  Target object is not a "theoretically legal" target.
     */
return_false:
    JMP_REG(edx, 0x0062D8C0);
}


/**
 *  Adds check for if the warhead forbids passively acquiring this unit.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_TechnoClass_Evaluate_Object_PassiveAcquire_Armor_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, this_ptr, edi);
    GET_REGISTER_STATIC(TechnoClass*, object, esi);

    static WeaponTypeClass* weapon;

    /**
     *  Determine if the target object has an armor type that this warhead is not allowed to passive acquire.
     */
    weapon = const_cast<WeaponTypeClass*>(this_ptr->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon);
    if (weapon && weapon->WarheadPtr && !Verses::Get_PassiveAcquire(object->Techno_Type_Class()->Armor, weapon->WarheadPtr))
        goto return_false;

    /**
     *  An object with no health shouldn't be targeted.
     */
    if (!object->Strength)
        goto return_false;

continue_checks:
    JMP(0x0062D129);

return_false:
    JMP(0x0062D8C0);
}


/**
 *  Wrapper for the patch below because doing this messes with the stack.
 */
static bool Can_Attack_Neutrals(TechnoClass* target)
{
    bool attack_neutrals = Vinifera_SpawnerActive && Vinifera_SpawnerConfig->AttackNeutralUnits;
    bool unarmed_building = target->What_Am_I() == RTTI_BUILDING && (!target->Is_Weapon_Equipped() || target->Get_Weapon()->Weapon->Range == 0);

    return attack_neutrals && !unarmed_building;
};


/**
 *  Patch to allow units to target neutral units if the spawner requests it.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_TechnoClass_Evaluate_Object_AttackNeutralUnits_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, target, esi);

    if (Session.Type != GAME_NORMAL && target->Owning_House()->Class->IsMultiplayPassive)
    {
        /**
         *  Allow attacking neutrals, but if it's a building, it must be armed.
         */
        if (!Can_Attack_Neutrals(target))
        {
            // return false;
            JMP(0x0062D8C0);
        }
    }

    // Continue normally.
    JMP(0x0062D4BA);
}


/**
 *  Replaces Verses (Modifier) of the Warhead with the one from the extension.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_TechnoClass_Base_Is_Attacked_Armor1_Patch)
{
    GET_STACK_STATIC(TechnoClass*, enemy, esp, 0x84);
    GET_REGISTER_STATIC(UnitClass*, unit, esi);

    if (Verses::Get_Modifier(enemy->Techno_Type_Class()->Armor, unit->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon->WarheadPtr))
    {
        _asm mov esi, unit
        JMP(0x00636C36);
    }

    _asm mov esi, unit
    JMP(0x00636D60);
}


/**
 *  Replaces Verses (Modifier) of the Warhead with the one from the extension.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_TechnoClass_Base_Is_Attacked_Armor2_Patch)
{
    GET_STACK_STATIC(TechnoClass*, enemy, esp, 0x84);
    GET_REGISTER_STATIC(InfantryClass*, unit, esi);

    if (Verses::Get_Modifier(enemy->Techno_Type_Class()->Armor, unit->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon->WarheadPtr) != 0)
    {
        _asm mov esi, unit
        JMP(0x006369E8);
    }

    _asm mov esi, unit
    JMP(0x00636B14);
}


/**
 *  #issue-357
 * 
 *  Creates an instance of the electric bolt from the firing techno to the target.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Fire_At_Electric_Bolt_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(WeaponTypeClass const *, weapon, ebx);
    GET_STACK_STATIC(TARGET, target, ebp, 0x8);
    static WeaponTypeClassExtension *weapontypeext;
    static TechnoClassExtension *technoext;

    /**
     *  Spawn the electric bolt.
     */
    weapontypeext = Extension::Fetch<WeaponTypeClassExtension>(weapon);
    if (weapontypeext->IsElectricBolt) {

        technoext = Extension::Fetch<TechnoClassExtension>(this_ptr);
        technoext->Electric_Bolt(target);

    /**
     *  Spawn the laser.
     */
    } else if (weapon->IsLaser) {
        goto is_laser;
    }

    JMP(0x006312CD);

is_laser:
    JMP_REG(edi, 0x00631231);
}


/**
 *  #issue-579
 * 
 *  Implements the Suicide (death on firing) logic for technos.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Fire_At_Suicide_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(WeaponTypeClass *, weap, ebx);
    GET_REGISTER_STATIC(BulletTypeClass *, bullet, edx);
    GET_STACK_STATIC(TARGET, target, ebp, 0x8);
    static WeaponTypeClassExtension *weapontypeext;
    static int damage;

    /**
     *  Stolen bytes/code.
     */
    if (!target) {
        goto return_null;
    }

    /**
     *  Fetch the extension instance for the firing weapon.
     */
    weapontypeext = Extension::Fetch<WeaponTypeClassExtension>(weap);

    /**
     *  Firing unit must be active in the game world when performing suicide.
     */
    if (this_ptr->IsActive && !this_ptr->IsInLimbo) {

        /**
         *  Explicitly delete the unit from the game world at this very moment.
         *  This is legacy behavior similar to that of Red Alert.
         */
        if (weapontypeext->IsSuicide && weapontypeext->IsDeleteOnSuicide) {
            DEV_DEBUG_INFO("Deleted: %s\n", this_ptr->Name());
            this_ptr->Remove_This();

        /**
         *  Deal full damage to the firing unit. The removal of the unit will
         *  go though the normal damage handling code.
         */
        } else if (weapontypeext->IsSuicide) {

            /**
             *  #TODO:
             *  We have to skip aircraft as they crash the game because
             *  they do not get removed correctly after taking full damage.
             *  
             *  This same crash happens in Red Alert 2 also, possible engine bug.
             */
            if (this_ptr->What_Am_I() == RTTI_AIRCRAFT) {
                goto limpet_check;
            }

            damage = this_ptr->Techno_Type_Class()->MaxStrength;
            this_ptr->Take_Damage(damage, 0, Rule->C4Warhead, nullptr, true, false);
        }

    }

    /**
     *  Continue checks.
     */
limpet_check:
    _asm { mov edi, target }    // Restore EDI to expected pointer.
    JMP(0x0063039B);

    /**
     *  Return null (didn't fire, no bullet returned).
     */
return_null:
    _asm { mov edi, target }
    JMP(0x006304D2);
}


/**
 *  Handle the player assigned mission and play the respective voice response.
 * 
 *  @author: CCHyper
 */
static void Techno_Player_Assign_Mission_Response_Switch(TechnoClass *this_ptr, MissionType mission)
{
    if (!this_ptr) {
        return;
    }

    if (!AllowVoice) {
        return;
    }

    TechnoClassExtension *technoext = Extension::Fetch<TechnoClassExtension>(this_ptr);

    switch (mission) {

        default:
        case MISSION_MOVE:
            this_ptr->Response_Move();
            break;

        case MISSION_ATTACK:
            this_ptr->Response_Attack();
            break;

        /**
         *  #issue-574
         * 
         *  Implements VoiceCapture, VoiceEnter, VoiceDeploy and VoiceHarvest.
         */
        case MISSION_CAPTURE:
            technoext->Response_Capture();
            break;

        case MISSION_ENTER:
            technoext->Response_Enter();
            break;

        case MISSION_UNLOAD:
            technoext->Response_Deploy();
            break;

        case MISSION_HARVEST:
            technoext->Response_Harvest();
            break;
    }
}


/**
 *  #issue-574
 * 
 *  Patch to allow additional voice responses.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Player_Assign_Mission_Response_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(MissionType, mission, edi);

    Techno_Player_Assign_Mission_Response_Switch(this_ptr, mission);

    JMP(0x0063167E);
}


/**
 *  #issue-434
 * 
 *  Implements Soylent value (refund amount override) for technos.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Refund_Amount_Soylent_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    static TechnoTypeClassExtension *technotypext;
    static TechnoTypeClass *technotype;
    static int cost;

    /**
     *  Stolen bytes/code.
     */
    technotype = this_ptr->Techno_Type_Class();

    /**
     *  Fetch the extension instance.
     */
    technotypext = Extension::Fetch<TechnoTypeClassExtension>(technotype);

    /**
     *  If the object has a soylent value defined, return this.
     */
    if (technotypext->SoylentValue > 0) {
        cost = technotypext->SoylentValue;
        goto return_amount;
    }

continue_function:
    _asm { mov eax, technotype }    // restore EAX pointer.
    JMP_REG(ecx, 0x0063809D);

return_amount:
    _asm { mov edi, [cost] }
    JMP(0x006380DC);
}


/**
 *  #issue-226
 * 
 *  Ensures infantry with IsMechanic only auto-target units and aircraft.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Greatest_Threat_Infantry_Mechanic_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(InfantryClass *, infantry_this_ptr, esi);
    GET_REGISTER_STATIC(ThreatType, method, ebx);
    static InfantryTypeClassExtension *infantrytypeext;

    /**
     *  #NOTE: This case is already within a infantry check.
     */

    method = (method & (THREAT_RANGE|THREAT_AREA));
    
    /**
     *  The following;
     *  - If this is a dual healer: Then infantry, vehicles and aircraft are valid targets.
     *  - If this is a mechanic: Then only consider vehicles and aircraft as valid targets.
     *  - Otherwise, we assume this is a medic and they can only consider other infantry to be a threat.
     * 
     *  #NOTE: Removed THREAT_AIR for IsMechanic and IsOmniHealer infantry and it causes
     *         them to chase down damaged friendly aircraft in the air.
     */
    infantrytypeext = Extension::Fetch<InfantryTypeClassExtension>(infantry_this_ptr->Class);
    if (infantrytypeext->IsOmniHealer) {
        method = method|(THREAT_INFANTRY|THREAT_VEHICLES/*|THREAT_AIR*/|THREAT_4000);
    } else if (infantrytypeext->IsMechanic) {
        method = method|(THREAT_VEHICLES/*|THREAT_AIR*/|THREAT_4000);
    } else {
        method = method|(THREAT_INFANTRY|THREAT_4000);
    }

    _asm { mov ebx, method }
    JMP(0x0062DDB1);
}


/**
 *  #issue-541
 * 
 *  Allow customisation of the infantry health bar draw position.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Draw_Health_Bars_Infantry_Draw_Pos_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, ebx);
    static int x_pos;
    static int y_pos;

    x_pos = UIControls->InfantryHealthBarDrawPos.X;
    y_pos = UIControls->InfantryHealthBarDrawPos.Y;

    _asm { mov ecx, [x_pos] }
    _asm { mov eax, [y_pos] }

    JMP_REG(esi, 0x0062C565);
}


/**
 *  #issue-541
 * 
 *  Allow customisation of the unit health bar draw position.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Draw_Health_Bars_Unit_Draw_Pos_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, ebx);
    static int x_pos;
    static int y_pos;

    x_pos = UIControls->UnitHealthBarDrawPos.X;
    y_pos = UIControls->UnitHealthBarDrawPos.Y;

    _asm { mov ecx, [x_pos] }
    _asm { mov eax, [y_pos] }

    JMP_REG(esi, 0x0062C5DF);
}


/**
 *  #issue-411
 * 
 *  Implements IsAffectsAllies for WarheadTypes.
 * 
 *  @note: This patch does not replace "stolen" code as per our implementation
 *         rules, this is because the call to ObjectClass::Take_Damage that follows
 *         is too much of a risk to not have correctly implemented.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Take_Damage_IsAffectsAllies_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_STACK_STATIC(int *, damage, esp, 0xEC);
    GET_STACK_STATIC(int, distance, esp, 0xF0);
    GET_STACK_STATIC(const WarheadTypeClass *, warhead, esp, 0xF4);
    GET_STACK_STATIC(TechnoClass *, source, esp, 0xF8);
    GET_STACK_STATIC8(bool, forced, esp, 0xFC);
    GET_STACK_STATIC(int, a6, esp, 0x100);
    static WarheadTypeClassExtension *warheadtypeext;
    static ResultType result;

    if (warhead) {

        /**
         *  Is the warhead that hit us one that affects units allied with its firing owner?
         */
        warheadtypeext = Extension::Fetch<WarheadTypeClassExtension>(warhead);
        if (!warheadtypeext->IsAffectsAllies) {

            /**
             *  If the source of the damage is an ally of ours, then reset
             *  the damage amount and return that we took no damage.
             */
            if (source && source->House->Is_Ally(this_ptr->House)) {
                *damage = 0;
                goto return_RESULT_NONE;
            }

        }

    }

    /**
     *  Stolen bytes/code.
     */
    _asm { mov ecx, a6 }

    /**
     *   Restore a few registers to be safe.
     */
    _asm { mov ebx, source }
    //_asm { mov edi, damage }
    JMP_REG(edx, 0x006328E5);

    /**
     *  Function returns RESULT_NONE.
     */
return_RESULT_NONE:
    JMP_REG(edi, 0x00632882);
}


/**
 *  #issue-404
 * 
 *  A object with "CloakStop" set has no effect on the cloaking behavior.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Is_Ready_To_Uncloak_Cloak_Stop_BugFix_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(bool, cloaked_by_house, al);

    /**
     *  Is this object unable to recloak or is it disabled by an EMP?
     */
    //if (!this_ptr->Is_Allowed_To_Recloak() && !this_ptr->IsCloakable || this_ptr->Is_Immobilized()) { // Original code.
    if (!this_ptr->Is_Allowed_To_Recloak() || !this_ptr->IsCloakable || this_ptr->Is_Immobilized()) {
        goto continue_check;
    }

    /**
     *  Object is not allowed to un-cloak at this time.
     */
return_false:
    JMP_REG(ecx, 0x0062F746);

    /**
     *  Continue checks.
     */
continue_check:
    _asm { mov bl, cloaked_by_house }
    JMP_REG(ecx, 0x0062F6DD);
}


/**
 *  #issue-391
 * 
 *  Extends the firing animation effect to support more facings.
 * 
 *  @author: CCHyper
 */
static AnimTypeClass *Techno_Get_Firing_Anim(TechnoClass *this_ptr, WeaponTypeClass *weapon)
{
    AnimTypeClass *anim = nullptr;

    int index = 0;
    int anim_count = weapon->Anim.Count();
    DirStruct dir = this_ptr->Fire_Direction();

    if (anim_count == 8) {

        index = dir.Get_Facing<8>() + 8 / 8;
        anim = weapon->Anim[index % FACING_COUNT];

    } else if (anim_count == 16) {

        index = dir.Get_Facing<16>() + 16 / 8;
        anim = weapon->Anim[index % 16];

    } else if (anim_count == 32) {

        index = dir.Get_Facing<32>() + 32 / 8;
        anim = weapon->Anim[index % 32];

    } else if (anim_count == 64) {

        index = dir.Get_Facing<64>() + 64 / 8;
        anim = weapon->Anim[index % 64];

    } else if (anim_count > 0) {

        index = 0;
        anim = weapon->Anim.Fetch_Head();

    } else {

        index = 0;
        anim = nullptr;
    }

    return anim;
}

DECLARE_PATCH(_TechnoClass_Fire_At_Weapon_Anim_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    GET_REGISTER_STATIC(WeaponTypeClass *, weapon, ebx);
    static AnimTypeClass *anim;

    anim = Techno_Get_Firing_Anim(this_ptr, weapon);

    _asm { mov edi, anim }
    JMP(0x006310A6);
}


/**
 *  #issue-356
 * 
 *  Custom cloaking sound for TechnoTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Do_Cloak_Cloak_Sound_Patch)
{
    GET_REGISTER_STATIC(Coordinate *, coord, eax);
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    static TechnoTypeClass *technotype;
    static TechnoTypeClassExtension *technotypeext;
    static VocType voc;

    technotype = this_ptr->Techno_Type_Class();

    /**
     *  Fetch the default cloaking sound.
     */
    voc = Rule->CloakSound;

    /**
     *  Fetch the extension instance.
     */
    technotypeext = Extension::Fetch<TechnoTypeClassExtension>(technotype);

    /**
     *  Does this object have a custom cloaking sound? If so, use it.
     */
    if (technotypeext->CloakSound != VOC_NONE) {
        voc = technotypeext->CloakSound;
    }

    /**
     *  Play the sound effect at the objects location.
     */
    Sound_Effect(voc, *coord);

    JMP(0x00633C8B);
}


/**
 *  #issue-356
 * 
 *  Custom uncloaking sound for TechnoTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Do_Uncloak_Uncloak_Sound_Patch)
{
    GET_REGISTER_STATIC(Coordinate *, coord, eax);
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, esi);
    static TechnoTypeClass *technotype;
    static TechnoTypeClassExtension *technotypeext;
    static VocType voc;

    technotype = this_ptr->Techno_Type_Class();

    /**
     *  Fetch the default cloaking sound.
     */
    voc = Rule->CloakSound;

    /**
     *  Fetch the extension instance.
     */
    technotypeext = Extension::Fetch<TechnoTypeClassExtension>(technotype);

    /**
     *  Does this object have a custom decloaking sound? If so, use it.
     */
    if (technotypeext->UncloakSound != VOC_NONE) {
        voc = technotypeext->UncloakSound;
    }

    /**
     *  Play the sound effect at the objects location.
     */
    Sound_Effect(voc, *coord);

    JMP(0x00633BE7);
}


/**
 *  A patch that adds debug logging on null house pointers in TechnoClass::Owner().
 * 
 *  This is a common crash observed by mod developers and map creators, and
 *  aims to assist tracking down the offending object.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_TechnoClass_Null_House_Warning_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, this_ptr, ecx);
    static HouseClass *house;
    static int id;
    
    house = this_ptr->House;
    if (!house) {
        DEBUG_WARNING("Techno \"%s\" has an invalid house!", this_ptr->Name());
        Vinifera_DeveloperMode_Warning_WWMessageBox("Techno \"%s\" has an invalid house!", this_ptr->Name());
        Fatal("Null house pointer in TechnoClass::Owner!\n");

    } else {
        id = house->ID;
    }
    
    _asm { mov eax, id }
    _asm { ret }
}


/**
 *  A patch that makes Technos abandon their current target if they can't fire at it.
 *
 *  @note: This is inside `if (TarCom != nullptr)`.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_TechnoClass_AI_Abandon_Invalid_Target_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, this_ptr, esi);

    static FireErrorType fire;
    static WeaponSlotType which;
    static WeaponTypeClass* weapon;

    /**
     *  Vanilla code.
     *  Don't let medics heal non-friendlies.
     */
    if (Session.Type != GAME_NORMAL)
    {
        if (this_ptr->House->IsHuman && this_ptr->Combat_Damage(WEAPON_NONE) < 0 && !this_ptr->House->Is_Ally(this_ptr->TarCom))
        {
            this_ptr->Assign_Target(nullptr);
        }
    }

    if (Frame % 16 == 0)
    {
        if (this_ptr->Mission != MISSION_CAPTURE && this_ptr->Mission != MISSION_SABOTAGE)
        {
            which = this_ptr->What_Weapon_Should_I_Use(this_ptr->TarCom);
            weapon = const_cast<WeaponTypeClass*>(this_ptr->Get_Weapon(which)->Weapon);

            fire = this_ptr->Can_Fire(this_ptr->TarCom, which);
            if (fire == FIRE_ILLEGAL || fire == FIRE_CANT)
            {
                this_ptr->Assign_Target(nullptr);
            }
        }
    }

    JMP(0x0062EB6F);
}


/**
 *  A patch that replaces the Tiberium dropping logic on harvester death.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_TechnoClass_Take_Damage_Drop_Tiberium_Type_Patch)
{
    GET_REGISTER_STATIC(TechnoClassExt*, this_ptr, esi);

    this_ptr->_Drop_Tiberium();

    // Return from the function
    JMP(0x00633073);
}


/**
 *  Patch to update the spawn manager when its owner is captured.
 *
 *  @author: ZivDero
 */
static void _Tag_Spring_Entered(TechnoClass* this_ptr) { this_ptr->Tag->Spring(TEVENT_PLAYER_ENTERED, this_ptr); }
DECLARE_PATCH(_TechnoClass_Captured_Spawn_Manager_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, this_ptr, esi);
    static TechnoClassExtension* extension;

    extension = Extension::Fetch<TechnoClassExtension>(this_ptr);

    if (extension->SpawnManager)
        extension->SpawnManager->Detach_Spawns();

    // Stolen instructions
    if (this_ptr->Tag)
        _Tag_Spring_Entered(this_ptr);

    JMP(0x00632518);
}


/**
 *  Patch to assign the target to the spawner.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_TechnoClass_Assign_Target_Spawn_Manager_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, this_ptr, esi);
    static TechnoClassExtension* extension;

    extension = Extension::Fetch<TechnoClassExtension>(this_ptr);

    if (extension->SpawnManager)
        extension->SpawnManager->Queue_Target(nullptr);

    // Stolen instructions
    this_ptr->CurrentBurstIndex = 0;

    JMP(0x0062FDE8);
}


/**
 *  Patch to pass the fire command to the spawner.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_TechnoClass_Fire_At_Spawn_Manager_Patch)
{
    GET_REGISTER_STATIC(TechnoClassExt*, this_ptr, esi);
    GET_REGISTER_STATIC(WeaponTypeClass*, weapon, ebx);
    GET_REGISTER_STATIC(TARGET, target, edi);

    // Stolen instructions
    if (((weapon->IsSonic && this_ptr->Wave)
      || (weapon->IsRailgun && this_ptr->ParticleSystems[4])
      || (weapon->IsUseFireParticles && this_ptr->ParticleSystems[0])
      || (weapon->IsUseSparkParticles && this_ptr->ParticleSystems[1])))
    {
        // return FIRE_OK;
        JMP(0x006304D2);
    }

    if (this_ptr->_Spawner_Fire_At(target, weapon))
    {
        // return FIRE_OK;
        JMP(0x006304D2);
    }

    // Continue checks
    JMP(0x0063052D);
}


/**
 *  #issue-715
 *
 *  Enables the deploy keyboard command to work for units that
 *  transform into a different unit on deploy.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_TechnoClass_2A0_Is_Allowed_To_Deploy_Unit_Transform_Patch)
{
    GET_REGISTER_STATIC(UnitTypeClass*, unittype, eax);
    static UnitTypeClassExtension* unittypeext;

    /**
     *  Stolen bytes/code.
     */
    if (unittype->DeploysInto != nullptr) {
        goto has_deploy_ability;

    }
    else if (unittype->MaxPassengers > 0) {
        goto has_deploy_ability;

    }
    else if (unittype->IsMobileEMP) {
        goto has_deploy_ability;
    }

    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);

    if (unittypeext->TransformsInto != nullptr) {
        goto has_deploy_ability;
    }

    /**
     *  The unit has no ability that allows it to deploy / unload.
     *  Mark that and continue function after the check.
     */
has_no_deploy_ability:
    _asm { mov eax, unittype }
    JMP_REG(ecx, 0x006320E0);

    /**
     *  The unit has some kind of an ability that allows it to deploy / unload.
     *  Continue function after the check.
     */
has_deploy_ability:
    _asm { mov eax, unittype }
    JMP_REG(ecx, 0x006320E5);
}


/**
 *  #issue-1033
 *
 *  Fixes a bug where AmbientDamage does not take FirepowerBias into account.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_TechnoClass_Railgun_Damage_Apply_Damage_Modifier_Patch)
{
    GET_REGISTER_STATIC(int, damage, ecx);
    GET_STACK_STATIC(TechnoClass*, this_ptr, esp, 0x74);

    damage = damage * (this_ptr->FirepowerBias * this_ptr->House->FirepowerBias);
    _asm { mov  ecx, dword ptr ds : damage }
    _asm { mov[esp + 0x4C], ecx }

    // Restore code that we destroyed by jumping to our code and continue damage applying logic
    _asm { mov  edx, [esp + 0x14] }
    JMP(0x006396D9);
}


/**
 *  Patch that sets a custom duration for the target laser.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_TechnoClass_Fire_At_TargetLaserTimer_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, this_ptr, esi);

    this_ptr->TargetingLaserTimer = UIControls->TargetLaserTime;

    JMP(0x00631223);
}


/**
 *  Main function for patching the hooks.
 */
void TechnoClassExtension_Hooks()
{
    Patch_Jump(0x00633C78, &_TechnoClass_Do_Cloak_Cloak_Sound_Patch);
    Patch_Jump(0x00633BD4, &_TechnoClass_Do_Uncloak_Uncloak_Sound_Patch);
    Patch_Jump(0x0063105C, &_TechnoClass_Fire_At_Weapon_Anim_Patch);
    Patch_Jump(0x0062F6B7, &_TechnoClass_Is_Ready_To_Uncloak_Cloak_Stop_BugFix_Patch);
    Patch_Jump(0x0062E6F0, &_TechnoClass_Null_House_Warning_Patch);
    Patch_Jump(0x006328DE, &_TechnoClass_Take_Damage_IsAffectsAllies_Patch);
    Patch_Jump(0x0062C5D5, &_TechnoClass_Draw_Health_Bars_Unit_Draw_Pos_Patch);
    Patch_Jump(0x0062C55B, &_TechnoClass_Draw_Health_Bars_Infantry_Draw_Pos_Patch);
    Patch_Jump(0x0062DD70, &_TechnoClass_Greatest_Threat_Infantry_Mechanic_Patch);
    Patch_Jump(0x00638095, &_TechnoClass_Refund_Amount_Soylent_Patch);
    Patch_Jump(0x00631661, &_TechnoClass_Player_Assign_Mission_Response_Patch);
    Patch_Jump(0x00630390, &_TechnoClass_Fire_At_Suicide_Patch);
    Patch_Jump(0x00631223, &_TechnoClass_Fire_At_Electric_Bolt_Patch);
    Patch_Jump(0x0062D4CA, &_TechnoClass_Evaluate_Object_Is_Legal_Target_Patch);
    Patch_Jump(0x00637540, &TechnoClassExt::_Draw_Pips);
    Patch_Jump(0x0062A0D0, &TechnoClassExt::_What_Weapon_Should_I_Use);
    Patch_Jump(0x00636F00, &TechnoClassExt::_Is_Allowed_To_Retaliate);
    Patch_Jump(0x00639810, &TechnoClassExt::_Target_Threat);
    Patch_Jump(0x00638240, &TechnoClassExt::_Anti_Infantry);
    Patch_Jump(0x00636BFE, &_TechnoClass_Base_Is_Attacked_Armor1_Patch);
    Patch_Jump(0x006369B0, &_TechnoClass_Base_Is_Attacked_Armor2_Patch);
    Patch_Jump(0x0062D11E, &_TechnoClass_Evaluate_Object_PassiveAcquire_Armor_Patch);
    Patch_Call(0x0042EC25, &TechnoClassExt::_What_Action);
    Patch_Call(0x004A8532, &TechnoClassExt::_What_Action);
    Patch_Jump(0x0062EB27, &_TechnoClass_AI_Abandon_Invalid_Target_Patch);
    Patch_Jump(0x00632F4C, &_TechnoClass_Take_Damage_Drop_Tiberium_Type_Patch);
    Patch_Jump(0x006320C2, &_TechnoClass_2A0_Is_Allowed_To_Deploy_Unit_Transform_Patch);
    Patch_Call(0x00637FF5, &TechnoClassExt::_Cell_Distance_Squared); // Patch Find_Docking_Bay to call our own distance function that avoids overflows
    Patch_Jump(0x006396D1, &_TechnoClass_Railgun_Damage_Apply_Damage_Modifier_Patch);
    Patch_Jump(0x006313D0, &TechnoClassExt::_Draw_Target_Laser);
    Patch_Jump(0x00631207, &_TechnoClass_Fire_At_TargetLaserTimer_Patch);
    Patch_Jump(0x00637D60, &TechnoClassExt::_Draw_Text_Overlay);
    Patch_Jump(0x006364A0, &TechnoClassExt::_Crew_Type);
    Patch_Jump(0x0062A300, &TechnoClassExt::_How_Many_Survivors);
    Patch_Jump(0x006324FF, &_TechnoClass_Captured_Spawn_Manager_Patch);
    Patch_Jump(0x0062FDE2, &_TechnoClass_Assign_Target_Spawn_Manager_Patch);
    Patch_Jump(0x006304DD, &_TechnoClass_Fire_At_Spawn_Manager_Patch);
    Patch_Jump(0x00637450, &TechnoClassExt::_Target_Something_Nearby);
    Patch_Jump(0x0062FD20, &TechnoClassExt::_Stun);
    Patch_Call(0x0062E9D1, &TechnoClassExt::_Mission_AI);
    Patch_Jump(0x0062F980, &TechnoClassExt::_Can_Fire);
    Patch_Jump(0x00631FF0, &TechnoClassExt::_Can_Player_Move);
    Patch_Jump(0x006336F0, &TechnoClassExt::_Record_The_Kill);
    //Patch_Jump(0x0062A3D0, &TechnoClassExt::_Fire_Coord); // Disabled because it's functionally identical to the vanilla function when there's no secondary coordinate
    Patch_Jump(0x0062D49A, &_TechnoClass_Evaluate_Object_AttackNeutralUnits_Patch);
}
