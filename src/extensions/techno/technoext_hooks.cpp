/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended TechnoClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "technoext_hooks.h"

#include "aircraft.h"
#include "asserthandler.h"
#include "audio_vox.h"
#include "buildingext.h"
#include "bullettype.h"
#include "bullettypeext.h"
#include "cell.h"
#include "clipline.h"
#include "debughandler.h"
#include "drawshape.h"
#include "extension.h"
#include "fatal.h"
#include "fetchres.h"
#include "hooker.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "infantry.h"
#include "infantrytype.h"
#include "infantrytypeext.h"
#include "ionstorm.h"
#include "language.h"
#include "mouse.h"
#include "options.h"
#include "rules.h"
#include "rulesext.h"
#include "session.h"
#include "sideext.h"
#include "spawnmanager.h"
#include "syringe.h"
#include "tactical.h"
#include "tag.h"
#include "team.h"
#include "teamtype.h"
#include "techno.h"
#include "technoext.h"
#include "technotype.h"
#include "technotypeext.h"
#include "textprint.h"
#include "tiberium.h"
#include "tiberiumext.h"
#include "tibsun_inline.h"
#include "uicontrol.h"
#include "unit.h"
#include "unittype.h"
#include "unittypeext.h"
#include "utracker.h"
#include "verses.h"
#include "vinifera_util.h"
#include "voc.h"
#include "vox.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "weapontype.h"
#include "weapontypeext.h"
#include "wwkeyboard.h"

#include <vector>


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(TechnoClass)
{
public:
    void _Draw_Pips(Point2D& bottomleft, Point2D& bottomright, Rect& rect) const;
    WeaponSlotType _What_Weapon_Should_I_Use(AbstractClass * target) const;
    bool _Is_Allowed_To_Retaliate(TechnoClass* source, WarheadTypeClass const* warhead) const;
    double _Target_Threat(TechnoClass* target, Coord& firing_coord) const;
    int _Anti_Infantry() const;
    ActionType _What_Action(ObjectClass* object, bool disallow_force);
    void _Drop_Tiberium();
    int _Cell_Distance_Squared(const AbstractClass* object) const;
    void _Draw_Target_Laser() const;
    void _Draw_Text_Overlay(Point2D& point1, Point2D& point2, Rect& rect) const;
    const InfantryTypeClass* _Crew_Type() const;
    int _How_Many_Survivors() const;
    bool _Spawner_Fire_At(AbstractClass * target, WeaponTypeClass* weapon);
    bool _Target_Something_Nearby(Coord& coord, ThreatType threat);
    void _Stun();
    void _Mission_AI();
    FireErrorType _Can_Fire(AbstractClass * target, WeaponSlotType which = WEAPON_SLOT_PRIMARY);
    bool _Can_Player_Move() const;
    Coord _Fire_Coord(WeaponSlotType which) const;
    void _Record_The_Kill(TechnoClass* source);
    int _Time_To_Build() const;
    void _Assign_Target(AbstractClass * target);
    void _AI_Abandon_Detour();
    bool _Can_Deploy_Now() const;
    int _Refund_Amount() const;
    bool _Evaluate_Object(ThreatType method, int mask, int range, TechnoClass const* object, int& value, int zone, Coord const& coord) const;
    bool _Should_Self_Heal_Now() const;
    int _Apparent_Brightness(int brightness) const;
    void _Flashing_AI();
    int _Anti_Air() const;
    void _Look(bool incremental, bool dontmap);
    bool _Is_Allowed_To_Recloak();
};


/**
 * A unit that has death frames will trigger its death counter upon the first death, and will live until the counter reaches its MaxDeathFrames
 * During this time, the unit is considered "dying" - in the sense that it was already killed, but still lives for the purposes of playing its death animation.
 * This has gameplay ramifications, so this helper function helps identify when the unit is in dying state to align its behavior.
 * Note: currently only RTTI_UNIT technos can have death frames.
 * 
 * @author: JoyfulShush
 */
static bool Is_Unit_Dying(TechnoClassExt* this_ptr)
{
    if (this_ptr->RTTI == RTTI_UNIT) {
        const auto unit = reinterpret_cast<UnitClass*>(this_ptr);
        if (unit->DeathCounter >= 0) {
            return true;
        }
    }

    return false;
}


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

    const ShapeSet* pip_shapes = Class_Of()->PipShapes;
    const ShapeSet* pips1 = Class_Of()->PipShapes;
    const ShapeSet* pips2 = Class_Of()->Pip2Shapes;

    const auto ttype = TClass;
    const auto ttype_ext = Extension::Fetch(ttype);
    const auto ext = Extension::Fetch(this);

    if (RTTI != RTTI_BUILDING)
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
        const bool carrying_passengers = TClass->Max_Passengers() > 0;
        if (carrying_passengers)
        {
            ObjectClass const* object = Cargo.Attached_Object();
            for (int index = 0; index < Class_Of()->Max_Pips(); index++)
            {
                int pip = 0;

                if (object != nullptr)
                {
                    pip = 1;
                    if (object->RTTI == RTTI_INFANTRY)
                    {
                        pip = ((InfantryClass*)object)->Class->Pip;
                    }
                    object = object->Next;
                }
                Draw_Shape(*LogicalSurface, *NormalDrawer, pip_shapes, pip, Point2D(drawx + dx * index, drawy + dy * index), rect, SHAPE_WIN_REL | SHAPE_CENTER);
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
            if ((RTTI == RTTI_UNIT || RTTI == RTTI_BUILDING) && TClass->PipScale == PIPSCALE_TIBERIUM)
            {
                std::vector<int> pips_to_draw;
                pips_to_draw.reserve(Class_Of()->Max_Pips());

                /**
                 *  Weeders/Waste Facilities draw all their contents with the Weed pip.
                 */
                if ((ttype->RTTI == RTTI_UNITTYPE && ((UnitTypeClass*)ttype)->IsToVeinHarvest) ||
                    (ttype->RTTI == RTTI_BUILDINGTYPE && ((BuildingTypeClass*)ttype)->IsWeeder))
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
                        tibtypes.emplace_back(Extension::Fetch(Tiberiums[i])->PipDrawOrder, i);

                    std::stable_sort(tibtypes.begin(), tibtypes.end());

                    /**
                     *  Add all the pips to draw to a vector.
                     */
                    for (auto& tibtuple : tibtypes)
                    {
                        const double amount = Storage.Get_Amount((TiberiumType)std::get<1>(tibtuple));
                        const double fraction = amount / ttype->Storage;
                        const int pip_count = ttype->Max_Pips() * fraction + 0.5;

                        int piptype = Extension::Fetch(Tiberiums[std::get<1>(tibtuple)])->PipIndex;
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
                    Draw_Shape(*LogicalSurface, *NormalDrawer, pip_shapes, shape, Point2D(drawx + dx * index, drawy + dy * index), rect, SHAPE_WIN_REL | SHAPE_CENTER);
                }
            }
            else if (ext->SpawnManager && ext->SpawnManager->SpawnCount > 0)
            {
                for (int index = 0; index < ext->SpawnManager->SpawnCount; index++)
                {
                    const int pip = index < ext->SpawnManager->Docked_Count() ? 1 : 0;
                    Draw_Shape(*LogicalSurface,* NormalDrawer, pip_shapes, pip, Point2D(drawx + dx * index, drawy + dy * index), rect, SHAPE_WIN_REL | SHAPE_CENTER);
                }
            }
            else if (TClass->PipScale == PIPSCALE_AMMO)
            {
                if (ttype_ext->PipWrap > 0)
                {
                    enum { PIPSCALE_AMMO_WRAP_FIRST = 7 };

                    const int wrap_count = Ammo / ttype_ext->PipWrap;
                    const int leftover = Ammo % ttype_ext->PipWrap;

                    for (int index = 0; index < ttype_ext->PipWrap; index++)
                    {
                        Draw_Shape(*LogicalSurface, *NormalDrawer, pips2, PIPSCALE_AMMO_WRAP_FIRST + wrap_count + (index < leftover), Point2D(drawx + dx * index, drawy + dy * index - 3), rect, SHAPE_WIN_REL | SHAPE_CENTER);
                    }
                }
                else
                {
                    for (int index = 0; index < Class_Of()->Max_Pips() && pips > 0; index++, pips--)
                    {
                        Draw_Shape(*LogicalSurface,* NormalDrawer, pips2, 6, Point2D(drawx + dx * index, drawy + dy * index - 3), rect, SHAPE_WIN_REL | SHAPE_CENTER);
                    }
                }
                
            }
            else if (TClass->PipScale == PIPSCALE_CHARGE)
            {
                for (int index = 0; index < Class_Of()->Max_Pips(); index++)
                {
                    Draw_Shape(*LogicalSurface,* NormalDrawer, pip_shapes, index < pips ? 1 : 0, Point2D(drawx + dx * index, drawy + dy * index), rect, SHAPE_WIN_REL | SHAPE_CENTER);
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
            drawpoint += UIControls->Get_Group_Number_Offset(RTTI, Class_Of()->Max_Pips() > 0);

            int group = Group + 1;

            if (group == 10)
                group = 0;

            std::snprintf(buffer, std::size(buffer), "%d", group >= 10 ? 0 : group);
            const ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->UIColor;
            Plain_Text_Print(buffer, *LogicalSurface, rect, drawpoint, COLOR_WHITE, COLOR_TBLACK, TPF_FULLSHADOW | TPF_EFNT, colorschemetype, 1);
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
        const int specialpip = Extension::Fetch(TClass)->SpecialPipIndex;
        if (specialpip >= 0)
        {
            Draw_Shape(*LogicalSurface, *NormalDrawer, pips1, specialpip, Point2D(drawx, drawy) + UIControls->Get_Special_Pip_Offset(RTTI), rect, SHAPE_WIN_REL | SHAPE_CENTER);
        }
        else if (RTTI == RTTI_INFANTRY && Combat_Damage() < 0)
        {
            Draw_Shape(*LogicalSurface,* NormalDrawer, pips1, 6, Point2D(drawx, drawy) + UIControls->Get_Special_Pip_Offset(RTTI), rect, SHAPE_WIN_REL | SHAPE_CENTER);
        }

        /**
         *  Display whether this unit is a leader unit or not.
         */
        if (RTTI != RTTI_BUILDING)
            Draw_Text_Overlay(Point2D(bottomleft.X - 10, bottomleft.Y + 10), bottomleft, rect);

        /**
         *  Display a veterancy pip is the unit is promoted.
         */
        int veterancy_shape = -1;
        if (Crew.IsVeteran)
            veterancy_shape = 7;

        if (Crew.IsElite)
            veterancy_shape = 8;

        if (Crew.Is_Dumbass())
            veterancy_shape = 12;

        if (veterancy_shape != -1)
        {
            Point2D drawpoint = center;
            drawpoint += UIControls->Get_Veterancy_Pip_Offset(RTTI);
            Draw_Shape(*LogicalSurface, *NormalDrawer, pips1, veterancy_shape, drawpoint, rect, SHAPE_WIN_REL | SHAPE_CENTER);
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
WeaponSlotType TechnoClassExt::_What_Weapon_Should_I_Use(AbstractClass * target) const
{
    if (target == nullptr) {
        return WEAPON_SLOT_PRIMARY;
    }

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
        w1 = Verses::Get_Modifier(armor, wptr->WarheadPtr) * Extension::Fetch(wptr->WarheadPtr)->Fetch_Type_Modifier(target->RTTI) * 1000;
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
        w2 = Verses::Get_Modifier(armor, wptr->WarheadPtr) * Extension::Fetch(wptr->WarheadPtr)->Fetch_Type_Modifier(target->RTTI) * 1000;
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
            immobilize = obj->RTTI == RTTI_ISOTILE;
        }
    } else if (target->RTTI == RTTI_CELL) {
        CellClass* cell = static_cast<CellClass*>(target);
        IsometricTileType tile = cell->ITType;
        if (tile != DestroyableCliff && tile != BlackTile && !cell->IsUnderBridge) {
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
bool TechnoClassExt::_Spawner_Fire_At(AbstractClass * target, WeaponTypeClass* weapon)
{
    auto weapon_ext = Extension::Fetch(weapon);

    if (weapon_ext->IsSpawner)
    {
        auto techno_ext = Extension::Fetch(this);
        techno_ext->SpawnManager->Queue_Target(target);
        if (IsOwnedByPlayer || IsDiscoveredByPlayer)
        {
            if (!Map.Is_Shrouded(Center_Coord()) && !Map.Is_Fogged(Center_Coord()))
                return true;

            if (RTTI == RTTI_AIRCRAFT && IsOwnedByPlayer)
                return true;
        }

        HouseClass* target_house = target->Owner_HouseClass();
        if (target_house && target_house->Is_Player_Control())
        {
            if (weapon_ext->IsRevealOnFire)
            {
                Map.Sight_From(Center_Coord(), 2, target_house);
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
bool TechnoClassExt::_Target_Something_Nearby(Coord& coord, ThreatType threat)
{
    threat &= THREAT_RANGE | THREAT_AREA;

    auto extension = Extension::Fetch(this);
    extension->LastTargetFrame = Frame;

    /**
     *  Determine that if there is an existing target it is still legal
     *  and within range.
     */
    if (TarCom != nullptr && (extension->HasOpportunityFireTarget || threat & THREAT_RANGE)) {
        WeaponSlotType primary = What_Weapon_Should_I_Use(TarCom);
        FireErrorType fire = Can_Fire(TarCom, primary);

        if (fire == FIRE_CANT) {
            if (extension->SpawnManager) {
                extension->SpawnManager->Abandon_Target();
            }
            Assign_Target(nullptr);

        } else if (fire == FIRE_ILLEGAL || fire == FIRE_RANGE) {
            Assign_Target(nullptr);
        }
    }

    /**
     *  If there is no target, then try to find one and assign it as
     *  the target for this unit.
     */
    if (TarCom == nullptr) {
        Assign_Target(Greatest_Threat(threat, coord));
    }

    /**
     *  Return with answer to question: Does this unit now have a target?
     */
    return TarCom != nullptr;
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

    const auto extension = Extension::Fetch(this);
    if (extension->SpawnManager)
    {
        extension->SpawnManager->Detach_Spawns();
        extension->SpawnManager->Abandon_Target();
    }

    Detach_All(true);
    Unselect();
}


/**
 *  Wrapper function around MissionClass::AI call in TechnoClass::AI
 *  to conveniently insert code.
 *
 *  @author: ZivDero
 */
void TechnoClassExt::_Mission_AI()
{
    const auto extension = Extension::Fetch(this);

    /**
     *  If the techno was opportunity fire but its mission no longer allows that, stop it.
     */
    if (TarCom != nullptr && extension->HasOpportunityFireTarget) {
        if (CurrentMission == MISSION_SLEEP ||
            CurrentMission == MISSION_ENTER ||
            CurrentMission == MISSION_STOP ||
            CurrentMission == MISSION_AMBUSH ||
            CurrentMission == MISSION_UNLOAD ||
            CurrentMission == MISSION_CONSTRUCTION ||
            CurrentMission == MISSION_DECONSTRUCTION ||
            CurrentMission == MISSION_REPAIR ||
            CurrentMission == MISSION_MISSILE ||
            CurrentMission == MISSION_HARMLESS ||
            CurrentMission == MISSION_OPEN) {

            Assign_Target(nullptr);
        }
    }

    /**
     *  If the techno has abandoned its target, and ROF time has passed, abandon 
     */
    if (extension->IsToResetBurst && extension->BurstResetTimer == 0) {
        extension->IsToResetBurst = false;
        BurstIndex = 0;
    }

    MissionClass::AI();

    if (!IsActive) {
        return;
    }

    /**
     *  Certain missions allow the unit to pick up targets on the move.
     */
    if (CurrentMission == MISSION_MOVE || CurrentMission == MISSION_HARVEST || CurrentMission == MISSION_GUARD) {
        extension->Opportunity_Fire();
    }

    if (extension->SpawnManager) {
        extension->SpawnManager->AI();
    }


    /**
     *  Check if the unit has been promoted.
     */
    if (extension->LastVeterancy != Crew.Get_Rank()) {
        if (extension->LastVeterancy != RANK_NONE) {
            if (Crew.IsElite) {

                /**
                 *  Play the promotion sound and voice line.
                 */
                if (House->Is_Player_Control()) {
                    Static_Sound(RuleExtension->UpgradeEliteSound, PositionCoord);
                    AudioVoxClass::Speak(RuleExtension->VoxUnitPromoted);
                }

                /**
                 *  Elite units also flash for a while.
                 */
                FlashCount = RuleExtension->EliteFlashTimer;
            } else if (Crew.IsVeteran) {

                /**
                 *  Play the promotion sound and voice line.
                 */
                if (House->Is_Player_Control()) {
                    Static_Sound(RuleExtension->UpgradeVeteranSound, PositionCoord);
                    AudioVoxClass::Speak(RuleExtension->VoxUnitPromoted);
                }
            }

            /**
             *  Force the unit to look in case its range has been upgraded.
             */
            Look();
        }

        extension->LastVeterancy = Crew.Get_Rank();
    }
}


/**
 *  Determines if this techno object can fire.
 *
 *  @author: 12/23/1994 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun.
 *           Rampastring - Added support for torpedoes.
 */
FireErrorType TechnoClassExt::_Can_Fire(AbstractClass * target, WeaponSlotType which)
{
    /**
     *  Don't allow firing if the target is illegal.
     */
    if (target == nullptr)
        return FIRE_ILLEGAL;

    const auto ext = Extension::Fetch(this);
    const auto typeext = Extension::Fetch(TClass);

    /**
     *  If this unit is a spawner, don't let it fire if it's currently in the process of spawning.
     */
    if (ext->SpawnManager && ext->SpawnManager->Preparing_Count())
        return FIRE_BUSY;

    TechnoClass* techno = Target_As_Techno(target);
    CellClass* cptr = &Map[target->Center_Coord()];

    /**
     *  If the object is completely cloaked, then you can't fire on it.
     */
    if (techno != nullptr && techno->Visual_Character(true, House) == VISUAL_HIDDEN
        && !cptr->Sensed_By(House->HeapID) && techno->House != House
        && (Combat_Damage() > 0 || !techno->House->Is_Ally(House)))
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
        if (RTTI != RTTI_UNIT)
            return FIRE_CANT;
        if (!reinterpret_cast<UnitClass*>(this)->Class->IsLargeVisceroid && !reinterpret_cast<UnitClass*>(this)->Class->IsSmallVisceroid)
            return FIRE_CANT;
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
    const auto bullettypeext = Extension::Fetch(weapon->Bullet);
    if (bullettypeext->IsTorpedo) {
        if (Map[target->Center_Coord()].Land_Type() != LAND_WATER) {
            return FIRE_CANT;
        }
    }

    /**
     *  If the weapon is a spawner, it needs to have an object ready to spawn.
     */
    if (weapon && Extension::Fetch(weapon)->IsSpawner)
    {
        if (ext->SpawnManager == nullptr)
        {
            Vinifera_DeveloperMode_Warning_WWMessageBox("[FATAL] You have a weapon that has Spawner=yes but you didn't define Spawns= in the techno.");
            Fatal("[FATAL] You have a weapon that has Spawner=yes but you didn't define Spawns= in the techno.");
        }

        if (ext->SpawnManager->Active_Count() == 0)
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
    if (which != WEAPON_SLOT_SECONDARY && RTTI == RTTI_UNIT)
    {
        const auto unit = reinterpret_cast<UnitClass*>(this);
        const int burst = BurstIndex % weapon->Burst;
        if (burst < 2)
        {
            if (unit->Class->FiringSyncFrame[burst] != -1
                && unit->FiringSyncDelay != -1)
            {
                if (unit->FiringSyncDelay != unit->Class->FiringSyncFrame[burst])
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
     *  An object can only have one instance of a particle/wave active at a time.
     */
    if (weapon->IsUseFireParticles && ParticleSystems[ATTACHED_PARTICLE_FIRE]) return FIRE_BUSY;
    if (weapon->IsRailgun && ParticleSystems[ATTACHED_PARTICLE_RAILGUN]) return FIRE_BUSY;
    if (weapon->IsUseSparkParticles && ParticleSystems[ATTACHED_PARTICLE_SPARK]) return FIRE_BUSY;
    if (weapon->IsSonic && Wave) return FIRE_BUSY;

    /**
     *  If the object has an armor type that this unit's warhead is forbidden to fire at, bail.
     */
    if (techno && !Verses::Get_ForceFire(techno->TClass->Armor, weapon->WarheadPtr)) return FIRE_ILLEGAL;

    /**
     *  The target must be within range in order to allow firing.
     */
    if (!In_Range_Of(target, which))
        return FIRE_RANGE;

    /**
     *  If there is no ammo left, then it can't fire.
     */
    if (!Ammo)
        return FIRE_AMMO;

    /**
     *  If cloaked, then firing is disabled.
     */
    if (typeext->IsDecloakToFire && Cloak != UNCLOAKED && (RTTI != RTTI_AIRCRAFT || Cloak == CLOAKED))
        return FIRE_CLOAKED;

    /**
     *  Hunter seekers can't fire since they need to kamikaze into the target.
     */
    if (TClass->IsHunterSeeker)
        return FIRE_RANGE;

    return FIRE_OK;
}


/**
 *  Determines if the object can be moved by the player.
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

     /**
      * Fixes an issue where a unit that has entered death animation and is being held alive by it
      * can be issued additional move commands to keep walking until it dies.
      * This makes it impossible for a player to issue move commands to dying units
      */
    if (Is_Unit_Dying(const_cast<TechnoClassExt*>(this))) {
        return false;
    }

    const auto ext = Extension::Fetch(this);
    if (ext->SpawnManager)
    {
        const auto typeext = Extension::Fetch(TClass);
        if (ext->SpawnManager->Preparing_Count() > 0 && ext->SpawnManager->Preparing_Count() < typeext->SpawnsNumber)
            return false;
    }

    return true;
}


/**
 *  Wrapper for TechnoClassExtension::Fire_Coord.
 */
Coord TechnoClassExt::_Fire_Coord(WeaponSlotType which) const
{
    return Extension::Fetch(this)->Fire_Coord(which, TPoint3D<int>());
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

    const TechnoTypeClass* ttype = TClass;

    /**
     *  If this unit is flagged as not being allowed to retaliate to attacks, return false.
     */
    if (!Extension::Fetch(ttype)->IsCanRetaliate)
        return false;

    /**
     *  Human-controlled units that have a target don't retaliate.
     */
    if (House->Is_Human_Player() && TarCom != nullptr)
        return false;

    /**
     *  If the source of the damage is a Veinhole, retaliate, unless this is a player-controlled
     *  ground unit and it's moving somewhere.
     */
    if (warhead != nullptr && warhead->IsVeinhole)
    {
        const bool is_foot_with_nav = Is_Foot() && reinterpret_cast<FootClass const*>(this)->NavCom;
        if (!is_foot_with_nav || !House->Is_Human_Player())
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
    if (!Current_Mission_Control().IsRetaliate)
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
        Verses::Get_Modifier(source->TClass->Armor, weapon_info->Weapon->WarheadPtr) == 0.0)
    {
        return false;
    }

    /**
     *  Don't allow retaliation if it isn't equipped with a weapon that can deal with the threat.
     */
    if (source->RTTI == RTTI_AIRCRAFT && !weapon_info->Weapon->Bullet->IsAntiAircraft) return false;

    /**
     *  Units with C4 are not allowed to retaliate against buildings in the normal sense while in guard mode. That
     *  is, unless it is owned by the computer. Normally, units with C4 can't do anything substantial to a building
     *  except to blow it up.
     */
    if (House->Is_Human_Player() && source->RTTI == RTTI_BUILDING)
    {
        if (RTTI == RTTI_INFANTRY && static_cast<InfantryTypeClass const*>(ttype)->IsBomber)
            return false;

        if (Crew.IsVeteran && ttype->VeteranAbilities[ABILITY_C4])
            return false;
        
        if (Crew.IsElite && (ttype->VeteranAbilities[ABILITY_C4] || ttype->EliteAbilities[ABILITY_C4]))
            return false;
    }

    /**
     *  Artillery that need to deploy to fire don't retaliate.
     */
    if (House->Is_Human_Player() && RTTI == RTTI_UNIT)
    {
        const BuildingTypeClass* deploys_into = reinterpret_cast<UnitClass const*>(this)->Class->DeploysInto;
        if (deploys_into && deploys_into->IsArtillary)
            return false;
    }

    /**
     *  If a human house is not allowed to retaliate automatically, then don't
     */
    if (House->Is_Human_Player() && !Rule->IsSmartDefense && RTTI != RTTI_BUILDING)
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
    if (!House->Is_Human_Player() && TarCom != nullptr && Is_Target_Object(TarCom))
    {
        const float current_val = Target_Threat(static_cast<TechnoClass*>(TarCom), Coord(0, 0, 0));
        const float source_val = Target_Threat(source, Coord(0, 0, 0));

        if (source_val < current_val)
            return false;
    }

    /**
     *  The warhead may forbid the unit from retaliating against targets with some armor types.
     */
    if (weapon_info->Weapon->WarheadPtr != nullptr &&
        !Verses::Get_Retaliate(source->TClass->Armor, weapon_info->Weapon->WarheadPtr))
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
double TechnoClassExt::_Target_Threat(TechnoClass* target, Coord& firing_coord) const
{
    double target_effectiveness_coefficient;
    double target_special_threat_coefficient;
    double my_effectiveness_coefficient;
    double target_strength_coefficient;
    double target_distance_coefficient;

    const TechnoTypeClass* ttype = TClass;

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
        const WeaponTypeClass* target_weapon = target->Get_Weapon(target->What_Weapon_Should_I_Use((AbstractClass *)this))->Weapon;
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
        threat += target_special_threat_coefficient * target->TClass->SpecialThreatValue;

        /**
         *  The enemy house extra gets priority.
         */
        if (House->Enemy != HOUSE_NONE && House->Enemy == target->House->Fetch_Heap_ID())
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
    threat += target->HealthRatio * target_strength_coefficient;

    /**
     *  Adjust threat if the target is outside our threat range.
     */
    int dist;
    if (firing_coord == Coord(0, 0, 0))
        dist = (Center_Coord() - target->Center_Coord()).Length() / 256;
    else
        dist = (firing_coord - target->Center_Coord()).Length();

    const int threat_range = (weapon ? weapon->Range : TClass->ThreatRange) / 256;
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

        int value = weapon->Attack * Verses::Get_Modifier(ARMOR_NONE, weapon->WarheadPtr) * mrange * weapon->WarheadPtr->SpreadFactor / weapon->ROF;
        if (TClass->Is_Two_Shooter())
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
        const bool ctrldown = Keyboard->Down(Options.KeyForceAttack1) || Keyboard->Down(Options.KeyForceAttack2);

        // Manually call the TechnoClass override so our TechnoClassExt override gets executed
        // Otherwise, moving infantry would not hit our override due to vanilla game code
        // returning early in InfantryClass::Can_Fire
        const FireErrorType error = TechnoClass::Can_Fire(object, What_Weapon_Should_I_Use(object));

        if (error == FIRE_ILLEGAL && !ctrldown)
            return ACTION_NONE;
    }

    return action;
}


#if 0
/**
 *  Disallow alt-moving with a carryall onto a non-totable unit
 *  as the carry-all treats that as toting.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x006318ED, _TechnoClass_What_Action_Totable_Patch, 8)
{
    GET(TechnoClass*, this_ptr, ESI);
    GET(ObjectClass*, object, EBP);

    if (this_ptr->RTTI == RTTI_AIRCRAFT && static_cast<AircraftClass*>(this_ptr)->Class->IsCarryall && object->RTTI == RTTI_UNIT && !Extension::Fetch(static_cast<UnitClass*>(object)->Class)->IsTotable) {
        return 0x006318F9; // ignore alt-move
    }

    return 0;
}
#endif


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

    if (Storage.Get_Total_Amount() > 0 && RTTI != RTTI_BUILDING && !Scen->Special.IsHarvesterImmune)
    {
        TiberiumType droplist[9] = { TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE, TIBERIUM_NONE };
        int dropcount = 0;

        /**
         *  Calculate how many cells of Tiberium we want to drop per type.
         */
        for (int i = 0; i < Tiberiums.Count(); i++)
        {
            double amount = Storage.Get_Amount(static_cast<TiberiumType>(i));
            amount = amount / TClass->Storage * std::size(drop_facings);
            for (int j = 0; j < amount && dropcount < std::size(droplist); j++)
                droplist[dropcount++] = static_cast<TiberiumType>(i);
        }

        const Cell center_cell = Center_Coord().As_Cell();

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

    const int points = TClass->Cost_Of(House);

    const auto typeext = Extension::Fetch(TClass);

    /**
     *  Handle any trigger event associated with this object.
     */
    if (IsActive && Tag && source) Tag->Spring(TEVENT_ATTACKED, this);

    if (IsActive && Tag && source) Tag->Spring(TEVENT_DISCOVERED, this);

    if (IsActive && RTTI != RTTI_UNIT) {

        if (IsActive && Tag && source) Tag->Spring(TEVENT_DESTROYED, this);

        if (IsActive && Tag) Tag->Spring(TEVENT_DESTROYED_ANY, this);

        if (IsActive && Tag) Tag->Spring(TEVENT_DESTROYED_ANY_X, this);
    }

    if (source && !typeext->IsDontScore && !House->Is_Ally(source) && !source->House->Is_Ally(this)) {

        const auto source_ext = Extension::Fetch(source);
        const auto source_typeext = Extension::Fetch(source->TClass);

        if (source->TClass->IsTrainable) {
            source->Crew.Made_A_Kill(source->TClass->Cost_Of(House), points);

        } else if (source_typeext->IsMissileSpawn) {

            if (source_ext->SpawnOwner && source_ext->SpawnOwner->TClass->IsTrainable) {
                source_ext->SpawnOwner->Crew.Made_A_Kill(source_ext->SpawnOwner->TClass->Cost_Of(House), points);
            }
        }

        House->WhoLastHurtMe = source->Owner();

        /**
         *  Add up the score for killing this unit
         */
        source->House->PointTotal += points;
    }

    switch (RTTI) {
    case RTTI_BUILDING:
    {
        if (!TClass->IsInsignificant) {
            if (reinterpret_cast<BuildingClass*>(this)->WhoLastHurtMe != HOUSE_NONE) {
                House->BuildingsLost++;
            }

            if (source != NULL) {
                if ((Session.Type == GAME_INTERNET || Session.Type == GAME_IPX) && !typeext->IsDontScore) {
                    source->House->DestroyedBuildings->Increment_Unit_Total(reinterpret_cast<BuildingClass*>(this)->Class->HeapID);
                }
                source->House->BuildingsKilled[Owner()]++;
            }

            /**
             *  If the map is displaying the multiplayer player names & their
             *  # of kills, tell it to redraw.
             */
            if (Map.Is_Player_Names()) {
                Map.Redraw_Radar(false);
            }

        }
    }
    break;

    case RTTI_AIRCRAFT:
        if (source && (Session.Type == GAME_INTERNET || Session.Type == GAME_IPX) && !typeext->IsDontScore) {
            source->House->DestroyedAircraft->Increment_Unit_Total(reinterpret_cast<AircraftClass*>(this)->Class->HeapID);
            total_recorded++;
        }
        // Fall through.....
    case RTTI_INFANTRY:
        if (source && !total_recorded && (Session.Type == GAME_INTERNET || Session.Type == GAME_IPX) && !typeext->IsDontScore) {
            source->House->DestroyedInfantry->Increment_Unit_Total(reinterpret_cast<InfantryClass*>(this)->Class->HeapID);
            total_recorded++;
        }
        // Fall through.....
    case RTTI_UNIT:
        if (source && !total_recorded && (Session.Type == GAME_INTERNET || Session.Type == GAME_IPX) && !typeext->IsDontScore) {
            source->House->DestroyedUnits->Increment_Unit_Total(reinterpret_cast<UnitClass*>(this)->Class->HeapID);
        }

        House->UnitsLost++;
        if (source && !typeext->IsDontScore) source->House->UnitsKilled[Owner()]++;

        /**
         *  If the map is displaying the multiplayer player names & their
         *  # of kills, tell it to redraw.
         */
        if (Map.Is_Player_Names()) {
            Map.Redraw_Radar(false);
        }
        break;

    default:
        break;
    }
}


/**
 *  Reimplementation of TechnoClass::Time_To_Build.
 *
 *  @author: CCHyper
 */
int TechnoClassExt::_Time_To_Build() const
{
    return Extension::Fetch(this)->Time_To_Build();
}


/**
 *  Reimplementation of TechnoClass::Assign_Target.
 *
 *  @author: ZivDero
 */
void TechnoClassExt::_Assign_Target(AbstractClass* target)
{
    auto extension = Extension::Fetch(this);

    /*
    **  In case the unit was doing opportunity fire, record that it's not the case anymore.
    */
    extension->HasOpportunityFireTarget = false;

    /*
    **  Save our previous target.
    */
    AbstractClass* old_target = TarCom;

    if (target == TarCom) return;

    if (target == nullptr) {
        target = nullptr;
    } else {

        /*
        **  Prevent targeting of self.
        */
        if (target == this) {
            target = &Map[PositionCoord];
        } else {

            /*
            **  Make sure that the target is not already dead.
            */
            ObjectClass* object = dynamic_cast<ObjectClass*>(target);
            if (object != nullptr && (object->IsActive == false || object->Strength == 0)) {
                target = nullptr;
            }
        }

        /*
        **  We have a target now, don't try to reset burst anymore.
        */
        extension->IsToResetBurst = false;
    }

    /*
    **  Set the unit's targeting computer.
    */
    TarCom = target;

    if (target == nullptr) {

        /*
        **  Abandon the spawn manager's target if there is one.
        */
        if (extension->SpawnManager) {
            extension->SpawnManager->Queue_Target(nullptr);
        }

        /*
        **  If we've got no target and didn't have one to begin with, reset burst now.
        */
        if (old_target == nullptr && !extension->IsToResetBurst) {
            BurstIndex = 0;
        }

        /*
        **  However, if we were firing at something, to prevent exploiting this to reset burst start a countdown instead.
        */
        else {

            WeaponSlotType which = What_Weapon_Should_I_Use(old_target);
            const WeaponTypeClass* weapon = Get_Weapon(which)->Weapon;
            if (weapon != nullptr && weapon->Burst > 1) {

                /*
                **  Set BurstIndex to a large value. This is a hack to make it so that Rearm_Delay returns the actual rearm time, not interburst time.
                */
                int old_burst = BurstIndex;
                BurstIndex = INT_MAX;

                extension->IsToResetBurst = true;
                extension->BurstResetTimer = Rearm_Delay(which);

                BurstIndex = old_burst;
            }
        }
    }
}



/**
 *  #issue-1087
 *
 *  Calculates the cell-based distance between this object and another object.
 *  Cell-based distance does not take leptons into account, only cell coordinates.
 *
 *  The original game's distance functions, such as Relative_Distance, also take leptons into
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

    Coord our_coord = Center_Coord();
    Coord their_coord = object->Center_Coord();

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

    const unsigned tarcom_color = DSurface::Build_Hicolor_Pixel(
        UIControls->TargetLaserColor.R,
        UIControls->TargetLaserColor.G,
        UIControls->TargetLaserColor.B);

    const unsigned tarcom_drop_color = DSurface::Build_Hicolor_Pixel(
        UIControls->TargetLaserDropShadowColor.R,
        UIControls->TargetLaserDropShadowColor.G,
        UIControls->TargetLaserDropShadowColor.B);

    const unsigned navcom_color = DSurface::Build_Hicolor_Pixel(
        UIControls->MovementLineColor.R,
        UIControls->MovementLineColor.G,
        UIControls->MovementLineColor.B);

    const unsigned navcom_drop_color = DSurface::Build_Hicolor_Pixel(
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
    Coord start_coord = entry_28C();
    Coord end_coord = func_638AF0();

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
     *  Save the start and end points before we clip them to the viewport,
     *  so that when we draw start and end rectangles they don't show up
     *  on screen edges if they're off-screen.
     */
    Point2D start_point_unclipped = start_point;
    Point2D end_point_unclipped = end_point;

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

                CompositeSurface->Draw_Dashed_Line(TacticalRect, drop_start_point, drop_end_point, drop_color, _pattern, offset);

                if (is_thick) {
                    drop_start_point.Y += 1;
                    drop_end_point.Y += 1;
                    CompositeSurface->Draw_Dashed_Line(TacticalRect, drop_start_point, drop_end_point, drop_color, _pattern, offset);
                }

            }

            /**
             *  Draw the dashed target laser line.
             */
            CompositeSurface->Draw_Dashed_Line(TacticalRect, start_point, end_point, line_color, _pattern, offset);

            if (is_thick) {
                start_point.Y += 1;
                end_point.Y += 1;
                CompositeSurface->Draw_Dashed_Line(TacticalRect, start_point, end_point, line_color, _pattern, offset);
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

        const int drop_point_size = is_thick ? point_size + 3 : point_size + 2;
        const Point2D drop_point_offset = is_thick ? point_offset + Point2D(-2, -2) : point_offset + Point2D(-1, -1);

        if (is_thick) {
            point_size -= 1;
        }

        Rect drop_start_point_rect = Intersect(TacticalRect, Rect(start_point_unclipped + drop_point_offset, drop_point_size, drop_point_size));
        CompositeSurface->Fill_Rect(drop_start_point_rect, drop_color);

        Rect drop_end_point_rect = Intersect(TacticalRect, Rect(end_point_unclipped + drop_point_offset, drop_point_size, drop_point_size));
        CompositeSurface->Fill_Rect(drop_end_point_rect, drop_color);
    }

    Rect start_point_rect = Intersect(TacticalRect, Rect(start_point_unclipped + point_offset, point_size, point_size));
    CompositeSurface->Fill_Rect(start_point_rect, line_color);

    Rect end_point_rect = Intersect(TacticalRect, Rect(end_point_unclipped + point_offset, point_size, point_size));
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
    const ColorSchemeType colorschemetype = Extension::Fetch(Sides[PlayerPtr->Class->Side])->UIColor;

    /**
     *  Print the Power/Drain text on power plants.
     */
    if (RTTI == RTTI_BUILDING && reinterpret_cast<const BuildingClass*>(this)->Class->Power > 0)
    {
        const auto owner = Owner_HouseClass();
        std::sprintf(buffer, Fetch_String(TXT_POWER_DRAIN), owner->Power_Output(), owner->Power_Drain());
        Plain_Text_Print(buffer, *LogicalSurface, rect, point2, COLOR_WHITE, COLOR_TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_EFNT, colorschemetype, 1);
    }

    /**
     *  Print the "Primary" text.
     */
    if (IsLeader)
    {
        const int text = RTTI == RTTI_BUILDING && reinterpret_cast<const BuildingClass*>(this)->Class->Width() == 1 ? TXT_PRI : TXT_PRIMARY;
        Plain_Text_Print(text, *LogicalSurface, rect, point2, COLOR_WHITE, COLOR_TBLACK, TPF_CENTER | TPF_FULLSHADOW | TPF_EFNT, colorschemetype, 1);
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
    if (!TClass->IsCrew) {
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
    if (TClass->IsCrew) {
        return Extension::Fetch(TClass)->CrewCount;
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
DEFINE_HOOK(0x0062D4CA, _TechnoClass_Evaluate_Object_Is_Legal_Target_Patch, 0)
{
    GET(TechnoClass *, this_ptr, EDI);
    GET(const TechnoClass *, object, ESI); // The target object being evaluated.

    auto object_tclass = object->TClass;
    auto object_tclassext = Extension::Fetch(object_tclass);

    /**
     *  Determine if the target is theoretically allowed to be a target.
     */
    if (!object_tclass->IsLegalTarget) {
        goto return_false;
    }

    /**
     *  Now, determine if "we" are owned by a non-human house and the target is not theoretically allowed to be a target.
     */
    if (!this_ptr->House->Is_Human_Player() && !object_tclassext->IsLegalTargetComputer) {
        goto return_false;
    }

    /**
     *  Target object passed the "theoretical" check, for now...
     */
continue_eval:
    return 0x0062D4D8;

    /**
     *  Target object is not a "theoretically legal" target.
     */
return_false:
    return 0x0062D8C0;
}


/**
 *  Adds check for if the warhead forbids passively acquiring this unit.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0062D11E, _TechnoClass_Evaluate_Object_PassiveAcquire_Armor_Patch, 0)
{
    GET(TechnoClass*, this_ptr, EDI);
    GET(TechnoClass*, object, ESI);

    /**
     *  Determine if the target object has an armor type that this warhead is not allowed to passive acquire.
     */
    auto weapon = this_ptr->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon;
    if (weapon && weapon->WarheadPtr && !Verses::Get_PassiveAcquire(object->TClass->Armor, weapon->WarheadPtr)) {
        goto return_false;
    }

    /**
     *  An object with no health shouldn't be targeted.
     */
    if (!object->Strength) {
        goto return_false;
    }

continue_checks:
    return 0x0062D129;

return_false:
    return 0x0062D8C0;
}


/**
 *  Replaces Verses (Modifier) of the Warhead with the one from the extension.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00636BFE, _TechnoClass_Base_Is_Attacked_Armor1_Patch, 0)
{
    GET_STACK(TechnoClass*, enemy, 0x84);
    GET(UnitClass*, unit, ESI);

    if (Verses::Get_Modifier(enemy->TClass->Armor, unit->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon->WarheadPtr)) {
        return 0x00636C36;
    }

    return 0x00636D60;
}


/**
 *  Replaces Verses (Modifier) of the Warhead with the one from the extension.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x006369B0, _TechnoClass_Base_Is_Attacked_Armor2_Patch, 0)
{
    GET_STACK(TechnoClass*, enemy, 0x84);
    GET(InfantryClass*, unit, ESI);

    if (Verses::Get_Modifier(enemy->TClass->Armor, unit->Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon->WarheadPtr) != 0) {
        return 0x006369E8;
    }

    return 0x00636B14;
}


/**
 *  #issue-357
 * 
 *  Creates an instance of the electric bolt from the firing techno to the target.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00631223, _TechnoClass_Fire_At_Electric_Bolt_Patch, 6)
{
    GET(TechnoClass *, this_ptr, ESI);
    GET(WeaponTypeClass const *, weapon, EBX);
    GET_BASE(AbstractClass*, target, 0x8);

    /**
     *  Spawn the electric bolt.
     */
    auto weapontypeext = Extension::Fetch(weapon);
    if (weapontypeext->IsElectricBolt) {

        auto technoext = Extension::Fetch(this_ptr);
        technoext->Electric_Bolt(target);

        /**
         *  Proceed to check for ammo.
         */
        return 0x0063126F;
    }

    return 0;
}


/**
 *  #issue-579
 * 
 *  Implements the Suicide (death on firing) logic for technos.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0063039B, _TechnoClass_Fire_At_Suicide_Patch, 5)
{
    GET(TechnoClass *, this_ptr, ESI);
    GET(WeaponTypeClass *, weap, EBX);
    GET_BASE(AbstractClass *, target, 0x8);

    /**
     *  Stolen bytes/code.
     */
    if (!target) {
        goto return_null;
    }

    /**
     *  Fetch the extension instance for the firing weapon.
     */
    auto weapontypeext = Extension::Fetch(weap);

    /**
     *  Firing unit must be active in the game world when performing suicide.
     */
    if (this_ptr->IsActive && !this_ptr->IsInLimbo) {

        /**
         *  Explicitly delete the unit from the game world at this very moment.
         *  This is legacy behavior similar to that of Red Alert.
         */
        if (weapontypeext->IsSuicide && weapontypeext->IsDeleteOnSuicide) {
            DEV_DEBUG_INFO("Deleted: {}\n", this_ptr->Name());
            this_ptr->Delete_Me();

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
            if (this_ptr->RTTI == RTTI_AIRCRAFT) {
                goto limpet_check;
            }

            int damage = this_ptr->TClass->MaxStrength;
            this_ptr->Take_Damage(damage, 0, Rule->C4Warhead, nullptr, true, false);
        }

    }

    /**
     *  Continue checks.
     */
limpet_check:
    return 0;

    /**
     *  Return null (didn't fire, no bullet returned).
     */
return_null:
    return 0x006304D2;
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

    TechnoClassExtension *technoext = Extension::Fetch(this_ptr);

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
DEFINE_HOOK(0x00631663, _TechnoClass_Player_Assign_Mission_Response_Patch, 0)
{
    GET(TechnoClass *, this_ptr, ESI);
    GET(MissionType, mission, EDI);

    Techno_Player_Assign_Mission_Response_Switch(this_ptr, mission);

    return 0x0063167E;
}


/**
 *  #issue-434
 *
 *  Implements Soylent value (refund amount override) for technos.
 *
 *  @author: CCHyper, tomsons26, ZivDero
 */
int TechnoClassExt::_Refund_Amount() const
{
    TechnoTypeClassExtension* technotypext = Extension::Fetch(TClass);

    /**
     *  If the object has a soylent value defined, return it.
     */
    if (technotypext->SoylentValue >= 0) {
        return technotypext->SoylentValue;
    }

    int cost = TClass->Cost_Of(House);

    if (House->Is_Human_Player()) {
        cost *= Rule->RefundPercent;
    }
    return cost;
}


/**
 *  #issue-226
 * 
 *  Ensures infantry with IsMechanic only auto-target units and aircraft.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0062DD70, _TechnoClass_Greatest_Threat_Infantry_Mechanic_Patch, 0)
{
    GET(TechnoClass *, this_ptr, ESI);
    GET(InfantryClass *, infantry_this_ptr, ESI);
    GET(ThreatType, method, EBX);

    /**
     *  #NOTE: This case is already within a infantry check.
     */
    method = method & (THREAT_RANGE|THREAT_AREA);
    
    /**
     *  The following;
     *  - If this is a dual healer: Then infantry, vehicles and aircraft are valid targets.
     *  - If this is a mechanic: Then only consider vehicles and aircraft as valid targets.
     *  - Otherwise, we assume this is a medic and they can only consider other infantry to be a threat.
     * 
     *  #NOTE: Removed THREAT_AIR for IsMechanic and IsOmniHealer infantry and it causes
     *         them to chase down damaged friendly aircraft in the air.
     */
    auto infantrytypeext = Extension::Fetch(infantry_this_ptr->Class);
    if (infantrytypeext->IsOmniHealer) {
        method = method|(THREAT_INFANTRY|THREAT_VEHICLES/*|THREAT_AIR*/|THREAT_ALLIES);
    } else if (infantrytypeext->IsMechanic) {
        method = method|(THREAT_VEHICLES/*|THREAT_AIR*/|THREAT_ALLIES);
    } else {
        method = method|(THREAT_INFANTRY|THREAT_ALLIES);
    }

    R->EBX(method);
    return 0x0062DDB1;
}


/**
 *  #issue-541
 * 
 *  Allow customisation of the infantry health bar draw position.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0062C55B, _TechnoClass_Draw_Health_Bars_Infantry_Draw_Pos_Patch, 0)
{
    R->ECX(UIControls->InfantryHealthBarDrawPos.X);
    R->EAX(UIControls->InfantryHealthBarDrawPos.Y);

    return 0x0062C565;
}


/**
 *  #issue-541
 * 
 *  Allow customisation of the unit health bar draw position.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0062C5D5, _TechnoClass_Draw_Health_Bars_Unit_Draw_Pos_Patch, 0)
{
    R->ECX(UIControls->UnitHealthBarDrawPos.X);
    R->EAX(UIControls->UnitHealthBarDrawPos.Y);

    return 0x0062C5DF;
}


static bool Should_Take_Damage(TechnoClass* this_ptr, TechnoClass* source, const WarheadTypeClass* warhead, int damage)
{
    /**
     * Fixes an issue where a unit that has entered death animation and is being held alive by it
     * can allow its killers to record its death over and over again, granting them an insane amount of veterancy.
     * It also has other side effects, such as counting the unit as killed for unit death count total, etc.
     * Units typically have a DeathCounter value of -1; it is set to 0 after it is first killed only if it has death frames.
     */
    if (Is_Unit_Dying(reinterpret_cast<TechnoClassExt*>(this_ptr))) {
        return false;
    }

    if (warhead) {

        /**
         *  #issue-411
         * 
         *  Is the warhead that hit us one that affects units allied with its firing owner?
         *
         *  @author: CCHyper
         */
        WarheadTypeClassExtension* warheadtypeext = Extension::Fetch(warhead);
        if (!warheadtypeext->IsAffectsAllies) {

            /**
             *  If the source of the damage is an ally of ours, then reset
             *  the damage amount and return that we took no damage.
             */
            if (source && source->House->Is_Ally(this_ptr->House)) {
                return false;
            }
        }
    }

    /**
     *  #issue-425
     *
     *  If this unit is shielded by the Iron Curtain, no damage should be dealt.
     *
     *  Author: Rampastring
     */
    TechnoClassExtension* technoext = Extension::Fetch(this_ptr);
    if (technoext->IronCurtainTimer > 0) {
        return false;
    }

    /**
     *  If this unit is a priority for shielding with the Iron Curtain and it would take major damage, shield it.
     */
    TechnoTypeClassExtension* technotypeext = Extension::Fetch(this_ptr->TClass);
    if (technotypeext->IronCurtainPriorityTarget && this_ptr->Strength - damage <= Rule->ConditionYellow * this_ptr->TClass->MaxStrength) {
        if (technoext->Iron_Curtain_Me(false)) {
            Extension::Fetch(this_ptr->House)->Expend_Iron_Curtain();
            return false;
        }
    }

    return true;
}

/**
 *  Implements IsAffectsAllies for WarheadTypes as well as the Iron Curtain effect for Technos.
 */
DEFINE_HOOK(0x006328DE, _TechnoClass_Take_Damage_Intercept_Patch, 7)
{
    GET(TechnoClass*, this_ptr, ESI);
    GET_STACK(int*, damage, 0xEC);
    GET_STACK(const WarheadTypeClass*, warhead, 0xF4);
    GET_STACK(TechnoClass*, source, 0xF8);

    if (!Should_Take_Damage(this_ptr, source, warhead, *damage)) {
        *damage = 0;
        goto return_RESULT_NONE;
    }

    return 0;

    /**
     *  Function returns RESULT_NONE.
     */
return_RESULT_NONE:
    return 0x00632882;
}


/**
 *  #issue-404
 * 
 *  A object with "CloakStop" set has no effect on the cloaking behavior.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0062F6B7, _TechnoClass_Is_Ready_To_Uncloak_Cloak_Stop_BugFix_Patch, 0)
{
    GET(TechnoClass *, this_ptr, ESI);
    GET(bool, cloaked_by_house, EAX);

    /**
     *  Is this object unable to recloak or is it disabled by an EMP?
     */
    if (!this_ptr->Is_Allowed_To_Recloak() || !this_ptr->IsCloakable || this_ptr->Is_Immobilized()) {
        goto continue_check;
    }

    /**
     *  Object is not allowed to un-cloak at this time.
     */
return_false:
    return 0x0062F746;

    /**
     *  Continue checks.
     */
continue_check:
    R->BL(cloaked_by_house);
    return 0x0062F6DD;
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
    DirType dir = this_ptr->Fire_Direction();

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

DEFINE_HOOK(0x0063105C, _TechnoClass_Fire_At_Weapon_Anim_Patch, 0)
{
    GET(TechnoClass *, this_ptr, ESI);
    GET(WeaponTypeClass *, weapon, EBX);

    AnimTypeClass* anim = Techno_Get_Firing_Anim(this_ptr, weapon);

    R->EDI(anim);
    return 0x006310A6;
}


/**
 *  #issue-356
 * 
 *  Custom cloaking sound for TechnoTypes.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00633C78, _TechnoClass_Do_Cloak_Cloak_Sound_Patch, 0)
{
    GET(Coord *, coord, EAX);
    GET(TechnoClass *, this_ptr, ESI);

    const TechnoTypeClass* technotype = this_ptr->TClass;

    /**
     *  Fetch the default cloaking sound.
     */
    VocType voc = Rule->CloakSound;

    /**
     *  Fetch the extension instance.
     */
    TechnoTypeClassExtension* technotypeext = Extension::Fetch(technotype);

    /**
     *  Does this object have a custom cloaking sound? If so, use it.
     */
    if (technotypeext->CloakSound != VOC_NONE) {
        voc = technotypeext->CloakSound;
    }

    /**
     *  Play the sound effect at the objects location.
     */
    Static_Sound(voc, *coord);

    return 0x00633C8B;
}


/**
 *  #issue-356
 * 
 *  Custom uncloaking sound for TechnoTypes.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00633BD4, _TechnoClass_Do_Uncloak_Uncloak_Sound_Patch, 0)
{
    GET(Coord *, coord, EAX);
    GET(TechnoClass *, this_ptr, ESI);

    const TechnoTypeClass* technotype = this_ptr->TClass;

    /**
     *  Fetch the default cloaking sound.
     */
    VocType voc = Rule->CloakSound;

    /**
     *  Fetch the extension instance.
     */
    TechnoTypeClassExtension* technotypeext = Extension::Fetch(technotype);

    /**
     *  Does this object have a custom decloaking sound? If so, use it.
     */
    if (technotypeext->UncloakSound != VOC_NONE) {
        voc = technotypeext->UncloakSound;
    }

    /**
     *  Play the sound effect at the objects location.
     */
    Static_Sound(voc, *coord);

    return 0x00633BE7;
}

#if 0 // Disabled because it seems to trigger false alarms when the game crashes for other reasons.
/**
 *  A patch that adds debug logging on null house pointers in TechnoClass::Owner().
 * 
 *  This is a common crash observed by mod developers and map creators, and
 *  aims to assist tracking down the offending object.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x0062E6F0, _TechnoClass_Null_House_Warning_Patch, 6)
{
    GET(TechnoClass *, this_ptr, ECX);
    
    HouseClass* house = this_ptr->House;
    if (!house) {
        DEBUG_WARNING("Techno \"{}\" has an invalid house!", this_ptr->Name());
        Vinifera_DeveloperMode_Warning_WWMessageBox("Techno \"%s\" has an invalid house!", this_ptr->Name());
        Fatal("Null house pointer in TechnoClass::Owner!\n");
    }

    return 0;
}
#endif

/**
 *  A patch that makes Technos abandon their current target if they can't fire at it.
 *
 *  @note: This is inside `if (TarCom != nullptr)`.
 *
 *  @author: ZivDero
 */
void TechnoClassExt::_AI_Abandon_Detour()
{
    /**
     *  Vanilla code.
     *  Don't let medics heal non-friendlies.
     */
    if (Session.Type != GAME_NORMAL) {
        if (House->IsHuman && Combat_Damage(WEAPON_NONE) < 0 && !House->Is_Ally(TarCom)) {
            Assign_Target(nullptr);
        }
    }

    if (Frame % 16 == 0) {
        RadioClass* radio = Contact_With_Whom();
        if (!radio || radio->RTTI != RTTI_BUILDING || radio->Mission != MISSION_UNLOAD) {
            if (Mission != MISSION_CAPTURE && Mission != MISSION_SABOTAGE) {
                WeaponSlotType which = What_Weapon_Should_I_Use(TarCom);
                FireErrorType fire = Can_Fire(TarCom, which);

                if (fire == FIRE_ILLEGAL || fire == FIRE_CANT) {
                    WeaponTypeClass* weapon = const_cast<WeaponTypeClass*>(Get_Weapon(which)->Weapon);
                    bool is_firing_particles = weapon && ((weapon->IsUseFireParticles && ParticleSystems[ATTACHED_PARTICLE_FIRE]) || (weapon->IsRailgun && ParticleSystems[ATTACHED_PARTICLE_RAILGUN]) || (weapon->IsUseSparkParticles && ParticleSystems[ATTACHED_PARTICLE_SPARK]) || (weapon->IsSonic && Wave));

                    if (!is_firing_particles || fire == FIRE_ILLEGAL) {
                        Assign_Target(nullptr);
                    }
                }
            }
        }
    }
}


/**
 *  Checks whether the object is allowed to deploy using the "Deploy" keyboard command.
 *
 *  @author: tomsons26, ZivDero, Rampastring
 */
bool TechnoClassExt::_Can_Deploy_Now() const
{
    bool blocked = false;

    if (RTTI == RTTI_UNIT) {
        UnitClass const * unit = reinterpret_cast<UnitClass const *>(this);
        blocked = unit->Is_Immobilized() || unit->CurrentTube >= TUBE_FIRST;

        /**
         *  #issue-715
         *
         *  Enables the deploy keyboard command to work for units that
         *  transform into a different unit on deploy.
         */
        UnitTypeClassExtension* unittypeext = Extension::Fetch(unit->Class);

        if (unit->Class->DeploysInto == nullptr &&
            unit->Class->Max_Passengers() == 0
            && !unit->Class->IsMobileEMP &&
            unittypeext->TransformsInto == nullptr) {

            blocked = true;
        }

        // Hijacked units that are build-limited cannot be deployed because that would allow circumventing the build limit.
        if (unit->Class->DeploysInto != nullptr && unit->Class->BuildLimit < INT_MAX && EnteredByInfType != INFANTRY_NONE) {
            blocked = true;
        }

        if (unit->Class->Max_Passengers() > 0) {
            if (unit->IsOnBridge) {
                blocked = true;
            }
            if (unit->Class->Max_Passengers() > 0) {
                Cell cell = PositionCell;
                CellClass* cellptr = &Map[cell];
                if (cellptr != nullptr) {
                    CellClass* cellptr_s = &Map[Adjacent_Cell(cell, FACING_S)];
                    CellClass* cellptr_n = &Map[Adjacent_Cell(cell, FACING_N)];
                    CellClass* cellptr_e = &Map[Adjacent_Cell(cell, FACING_E)];
                    CellClass* cellptr_w = &Map[Adjacent_Cell(cell, FACING_W)];

                    if (cellptr->IsUnderBridge ||
                        cellptr_s != nullptr && cellptr_s->IsUnderBridge ||
                        cellptr_w != nullptr && cellptr_w->IsUnderBridge ||
                        cellptr_e != nullptr && cellptr_e->IsUnderBridge ||
                        cellptr_n != nullptr && cellptr_n->IsUnderBridge) {

                        blocked = true;
                    }
                }
            }
        }
    } else {
        blocked = Is_Immobilized();
        if (TClass->Max_Passengers() == 0) {
            blocked = true;
        }
    }

    if (RTTI == RTTI_BUILDING) {
        BuildingClass const* building = reinterpret_cast<BuildingClass const*>(this);
        if (building->Class->IsLimpetMine || building->Class->IsMobileWar) {
            blocked = false;
        }
    }

    Cell cell = PositionCell;
    CellClass* cellptr = &Map[cell];
    if ((cellptr != nullptr && cellptr->Is_Near_Tunnel_ES()) || blocked) {
        return false;
    }
    return true;
}


DEFINE_HOOK(0x0062EB27, _TechnoClass_AI_Abandon_Invalid_Target_Patch, 0)
{
    GET(TechnoClassExt*, this_ptr, ESI);

    this_ptr->_AI_Abandon_Detour();

    return 0x0062EB6F;
}


/**
 *  A patch that replaces the Tiberium dropping logic on harvester death.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00632F4C, _TechnoClass_Take_Damage_Drop_Tiberium_Type_Patch, 0)
{
    GET(TechnoClassExt*, this_ptr, ESI);

    this_ptr->_Drop_Tiberium();

    // Return from the function
    return 0x00633073;
}


/**
 *  Patch to update the spawn manager when its owner is captured.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x006324FF, _TechnoClass_Captured_Spawn_Manager_Patch, 0)
{
    GET(TechnoClass*, this_ptr, ESI);

    TechnoClassExtension* extension = Extension::Fetch(this_ptr);

    if (extension->SpawnManager) {
        extension->SpawnManager->Detach_Spawns();
    }

    // Stolen instructions
    if (this_ptr->Tag) {
        this_ptr->Tag->Spring(TEVENT_PLAYER_ENTERED, this_ptr);
    }

    return 0x00632518;
}


/**
 *  Patch to pass the fire command to the spawner.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x006304DD, _TechnoClass_Fire_At_Spawn_Manager_Patch, 0)
{
    GET(TechnoClassExt*, this_ptr, ESI);
    GET(WeaponTypeClass*, weapon, EBX);
    GET(AbstractClass *, target, EDI);

    // Stolen instructions
    if ((weapon->IsSonic && this_ptr->Wave)
        || (weapon->IsRailgun && this_ptr->ParticleSystems[4])
        || (weapon->IsUseFireParticles && this_ptr->ParticleSystems[0])
        || (weapon->IsUseSparkParticles && this_ptr->ParticleSystems[1]))
    {
        // return FIRE_OK;
        return 0x006304D2;
    }

    if (this_ptr->_Spawner_Fire_At(target, weapon))
    {
        // return FIRE_OK;
        return 0x006304D2;
    }

    // Continue checks
    return 0x0063052D;
}


/**
 *  #issue-1033
 *
 *  Fixes a bug where AmbientDamage does not take FirepowerBias into account.
 *
 *  @author: Rampastring
 */
DEFINE_HOOK(0x006396D1, _TechnoClass_Railgun_Damage_Apply_Damage_Modifier_Patch, 8)
{
    GET(int, damage, ECX);
    GET_STACK(TechnoClass*, this_ptr, 0x74);

    damage *= this_ptr->FirepowerBias * this_ptr->House->FirepowerBias;
    R->ECX(damage);

    return 0;
}


/**
 *  Patch that sets a custom duration for the target laser.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00631207, _TechnoClass_Fire_At_TargetLaserTimer_Patch, 0)
{
    GET(TechnoClass*, this_ptr, ESI);

    this_ptr->TargetingLaserTimer = UIControls->TargetLaserTime;

    return 0x00631223;
}


/**
 *  #issue-1161
 *
 *  Helper function. Checks whether a target is valid considering zone evaluation.
 *
 *  @author: Rampastring
 */
static bool Is_Valid_Target_Zone(const TechnoClass* techno, AbstractClass * target, int ourzone, int targetzone)
{
    auto technotype = techno->TClass;
    auto technotypeext = Extension::Fetch(technotype);

    if (technotypeext->TargetZoneScan == TZST_SAME) {
        // Only allow targeting objects in the same zone.
        return ourzone == targetzone;
    }

    if (technotypeext->TargetZoneScan == TZST_ANY) {
        // Any zone is allowed.
        return true;
    }

    if (technotypeext->TargetZoneScan == TZST_INRANGE) {
        // If the zone is different, only allow targeting if we can reach the target from our zone.

        if (ourzone == targetzone) {
            return true;
        }

        Cell nearbycell = Map.Nearby_Location(target->Center_Coord().As_Cell(),
            technotype->Speed,
            /*Phobos has -1 here*/ ourzone,
            technotype->MZone,
            false, Point2D(1, 1), true, false, false, technotype->Speed != SPEED_FLOAT);

        if (nearbycell == CELL_NONE) {
            // We couldn't find a valid cell to reach the target from
            return false;
        }

        int distance = ::Distance(nearbycell, target->Center_Coord().As_Cell());

        WeaponSlotType weaponslot = techno->What_Weapon_Should_I_Use(target);
        auto weaponinfo = techno->Get_Weapon(weaponslot);
        if (weaponinfo->Weapon == nullptr) {
            return false;
        }

        return distance * CELL_LEPTON_W < weaponinfo->Weapon->Range;
    }

    // For some reason the target zone scan type was invalid. Something is wrong.
    Fatal("Invalid TargetZoneScanType for techno type %s", technotype->IniName.c_str());
    return false;
}


/**
 *  #discussions-1450 
 *  Patch that allows customizing repair cap and repair rate across technos,
 *  rather than relying on values that have other side effects.
 *  Technos can have individual repair cap and rates specified for them.
 *  If they don't have those keys, then those values fall back to the game-wide customizable keys.
 *  If those also aren't specified either, then the game falls back to the keys it used in vanilla.
 *
 *  @author: JoyfulShush
 */
bool TechnoClassExt::_Should_Self_Heal_Now() const
{    
    auto technotypeext = Extension::Fetch(TClass);

    /**
     *  Prevents overhealing - allowing Strength to get above 100%.
     */
    if (Get_Health_Ratio() >= 1) {
        return false;
    }

    /*
    * Prevents units that have died from self-healing. 
    * This resolves an issue where self-healing aircraft would heal while tumbling down, 
    * becoming indestructible and being stuck in an looping tumbling animation until they land.
    */
    if (Get_Health_Ratio() <= 0) {
        return false;
    }

    if (!TClass->IsSelfHealing) {
        if (!Has_Ability(ABILITY_SELF_HEAL)) {
            return false;
        }
    }

    /**
     *  Determine the repair rate of this techno. 
     *  If the techno has its own repair rate, it is used. 
     *  Otherwise, the game-wide repair rate is used.
     *  If both are not specified, then the game falls back to the original RepairRate key used by this function in vanilla.     
     */
    double repair_rate;

    if (technotypeext->SelfHealingRate != -1) {
        repair_rate = technotypeext->SelfHealingRate;
    } else if (RuleExtension->SelfHealingRate != -1) {
        repair_rate = RuleExtension->SelfHealingRate;
    } else {
        repair_rate = Rule->RepairRate;
    }

    int repair_interval = std::max(1, (int)(repair_rate * TICKS_PER_MINUTE));

    if ((Frame % repair_interval) != 0) {
        return false;
    }

    /**
     *  Determines the repair cap (in percentage) of this techno. A techno cannot repair itself beyond its repair cap.
     *  If the techno has its own repair cap, it is used.
     *  Otherwise, the game-wide repair cap is used.
     *  If both are not specified, then the game falls back to the original ConditionYellow key used by this function in vanilla.
     */
    float repair_cap;
    
    if (technotypeext->SelfHealingCap != -1) {
        repair_cap = technotypeext->SelfHealingCap;
    } else if (RuleExtension->SelfHealingCap != -1) {
        repair_cap = RuleExtension->SelfHealingCap;
    } else {
        repair_cap = Rule->ConditionYellow;
    }
    
    return repair_cap >= HealthRatio;
}


/**
 *  Custom Evaluate_Object implementation to allow player-owned defenses to target
 *  enemy defenses without ignoring weaponless objects.
 *
 *  @author: tomsons26, ZivDero, Rampastring
 */
bool TechnoClassExt::_Evaluate_Object(ThreatType method, int mask, int range, TechnoClass const* object, int& value, int zone, Coord const& coord) const
{
    bool engineer = Is_Renovator();

    /*
    **  An object in limbo can never be a valid target.
    */
    if (object == nullptr || object->IsInLimbo || object->Strength == 0) {
        return false;
    }

    /*
    **  If the object is cloaked, then it isn't a legal target.
    */
    if (object->Cloak == CLOAKED) {
        if (!Map[object->Center_Coord()].Sensed_By(House->HeapID) && House != object->House) {
            return false;
        }
    }

    if (!object->IsLocked) {
        return false;
    }

    /*
    **  If the object is in a "harmless" state, then don't bother to consider it
    **  a threat.
    */
    if (object->Current_Mission_Control().IsNoThreat) {
        return false;
    }

    if (object->HeightAGL < -20) {
        return false;
    }

    /*
     *  #issue-1161
     *
     *  Makes Technos consider their TargetZoneScan when potential targets are in a
     *  different movement zone. Makes the AI smarter when targeting objects on different
     *  movement zones (for example, ships targeting ground targets).
     *  Implementation inspired by respective feature for the "Phobos" Yuri's Revenge engine extension.
     *
     *  @author: Rampastring
     */
    int targetzone = Map.Get_Cell_Zone(object->Center_Coord().As_Cell(), TClass->MZone, object->IsOnBridge);
    if (zone != -1 && targetzone != zone) {
        if (!Is_Valid_Target_Zone(this, const_cast<TechnoClass*>(object), zone, targetzone)) {
            return false;
        }
    }

    /*
    **  Friendly units are never considered a good target. Bail if this
    **  object is a friend.  Unless we're a medic, of course.  But then,
    **  only consider it a target if it's injured.
    */
    if ((RTTI != RTTI_INFANTRY || !reinterpret_cast<InfantryClass const*>(this)->IsBerzerk) && House->Is_Ally(object)) {
        if (Combat_Damage() < 0 || engineer) {
            if (object->HealthRatio == Rule->ConditionGreen) {
                return false;
            }
            if (object->RTTI == RTTI_AIRCRAFT && RTTI == RTTI_UNIT) {
                if (object->HeightAGL > 0) {
                    return false;
                }
                if (Map[object->Center_Coord()].Cell_Building()) {
                    return false;
                }
            } else if (RTTI == RTTI_UNIT && !object->Considered_Vehicle()) {
                return false;
            }
        } else {
            return false;
        }
    }

    if (Scen->Special.IsHarvesterImmune) {
        if (Rule->HarvesterUnit.Is_Present((UnitTypeClass*)object->Class_Of())) {
            return false;
        }
    }

    /*
    **  If the object is further away than allowed, bail.
    */
    int dist = Distance_To(object);
    if (range > 0 && dist > range) {
        return false;
    }

    if (range == 0) {
        if (!Is_Weapon_Equipped()) {
            if (dist > TClass->ThreatRange) {
                return false;
            }
        } else {
            WeaponSlotType primary = What_Weapon_Should_I_Use(const_cast<TechnoClass*>(object));
            if (!In_Range_Of(const_cast<TechnoClass*>(object), primary)) {
                return false;
            }
        }
    }

    /*
    **  If the object is not visible, then bail. Human controlled units
    **  are always considered to be visible.
    */
    if (House->Is_Player_Control() && !object->IsOwnedByPlayer && !object->IsDiscoveredByPlayer && Session.Type == GAME_NORMAL && object->RTTI != RTTI_AIRCRAFT) {
        return false;
    }

    if (object->RTTI == RTTI_BUILDING) {

        const BuildingClass* bldg = reinterpret_cast<const BuildingClass*>(object);

        if (bldg->Class->IsInvisibleInGame) return false;

        // Port over ts-patches hack that makes buildings not targeted by default if they have a weapon with a range of 0.
        const WeaponTypeClass* bldgweapon = bldg->Class->Fetch_Weapon_Info(WEAPON_SLOT_PRIMARY).Weapon;
        if (bldgweapon != nullptr && bldgweapon->Range == 0) {
            return false;
        }
    }

    /*
    **  Quickly eliminate all unit types that are not allowed according to the mask
    **  value.
    */
    RTTIType otype = object->RTTI;
    if (!(1 << otype & mask) && (!(mask & 2) || !object->Considered_Vehicle())) {
        return false; // Mask failure.
    }

    if (Session.Type != GAME_NORMAL && object->House->Class->IsMultiplayPassive && object->PrimaryWeapon == nullptr) {
        return false;
    }

    /*
    **  Determine if the target is theoretically allowed to be a target. If
    **  not, then bail.
    */
    TechnoTypeClass const* tclass = object->TClass;
    if (!tclass->IsLegalTarget) {
        return false; // Legality failure.
    }

    if (tclass->IsTrain && RTTI == RTTI_INFANTRY && reinterpret_cast<InfantryClass const*>(this)->Class->IsVehicleThief) {
        return false;
    }

    /*
    **  Never consider a spy to be a valid target, unless you're a dog
    */
    if ((House->Is_Human_Player() || !RuleExtension->IsAIDetectDisguise) && otype == RTTI_INFANTRY && ((InfantryTypeClass const*)tclass)->IsDisguised && !Extension::Fetch(TClass)->IsDetectDisguise) {
        return (false);
    }

    /*
    **  Special case so that SAM site doesn't fire on aircraft that are landed.
    */
    if (PrimaryWeapon != nullptr && !PrimaryWeapon->Bullet->IsAntiGround) {
        if (object->Height == 0) {
            return false;
        }
    }

    /*
    **  If only allowed to attack civilians, then eliminate all other types.
    */
    if (method & THREAT_CIVILIANS) {
        return false;
    }

    /*
    **  If the scan is limited to capturable buildings only, then bail if the examined
    **  object isn't a capturable building.
    */
    if (method & THREAT_CAPTURE && (otype != RTTI_BUILDING || !static_cast<BuildingTypeClass const*>(tclass)->IsCaptureable)) {
        return false;
    }

    /*
    **  SPECIAL CASE: Friendly units won't automatically fire on buildings
    **  if the building is not aggressive. That is, unless it is part of a team. A team
    **  is allowed to pick any target it so chooses.
    */
    if ((!Is_Foot() || reinterpret_cast<FootClass const*>(this)->Team == nullptr) && House->Is_Human_Player() && !object->Considered_Vehicle() && otype == RTTI_BUILDING && object->PrimaryWeapon == nullptr) {
        if (!engineer) {
            return false;
        }
    }

    if (engineer) {
        if (object->RTTI != RTTI_BUILDING || (House->Is_Ally(object->House) && (object->HealthRatio > Rule->ConditionRed || !static_cast<BuildingClass const*>(object)->Class->Cost_Of(House)))) {
            return false;
        }
    }

    /*
    **  If the search is restricted to Tiberium processing objects, then
    **  perform the special qualification check now.
    */
    if (method & THREAT_TIBERIUM) {
        if (tclass->Storage == 0) {
            return false;
        }
    }

    /*
    **  If the search is restricted to harvesters, then
    **  perform the special qualification check now.
    */
    if (method & EXT_THREAT_HARVESTERS) {
        if (object->RTTI != RTTI_UNIT || !reinterpret_cast<const UnitClass*>(object)->Class->IsToHarvest) {
            return false;
        }
    }

    CellClass* cell_this = &Map[Get_Coord()];
    CellClass* cell_that = &Map[object->Get_Coord()];
    if (cell_this->IsUnderBridge && cell_that->IsUnderBridge && IsOnBridge != object->IsOnBridge) {
        return false;
    }

    bool webbysecondary = false;
    WarheadTypeClass const* wh = nullptr;
    WeaponTypeClass const* secondary = SecondaryWeapon;
    if (secondary != nullptr) {
        wh = secondary->WarheadPtr;
        if (wh != nullptr && wh->IsWebby) {
            webbysecondary = true;
        }
    }

    if (!webbysecondary) {
        WeaponTypeClass const* primary = PrimaryWeapon;
        if (primary != nullptr) {
            wh = primary->WarheadPtr;
        } else {
            wh = nullptr;
        }
    }

    if (wh && wh->IsWebby) {
        if (object->RTTI == RTTI_INFANTRY) {
            InfantryClass const* infantry = dynamic_cast<InfantryClass const*>(object);
            if (infantry != nullptr && infantry->ProneStruggleTimer > wh->WebDuration / 4) {
                return false;
            }
        }
    }

    WeaponSlotType which = _What_Weapon_Should_I_Use(const_cast<TechnoClass*>(object));

    /**
     *  #issue-444
     *
     *  If the weapon fires torpedoes and the target is on land,
     *  then firing is not allowed.
     *
     *  UNLESS we're part of a team. Other team members might still be able to fire at the target,
     *  and teams prefer to synchronize their target. Cancelling targeting in this case
     *  could confuse other units of the team.
     *
     *  @author: Rampastring
     */
    if (which != WEAPON_SLOT_NONE && (!Is_Foot() || reinterpret_cast<const FootClass*>(this)->Team == nullptr)) {
        const WeaponTypeClass* weapon = Get_Weapon(which)->Weapon;
        if (weapon != nullptr) {
            const auto bullettypeext = Extension::Fetch(weapon->Bullet);
            if (bullettypeext->IsTorpedo) {
                if (Map[object->Center_Coord()].Land_Type() != LAND_WATER) {
                    return false;
                }
            }
        }
    }

    /*
    **  If this target value is better than the previously recorded best
    **  target value then record this target for possible return as the
    **  best.
    */
    value = Target_Threat(const_cast<TechnoClass*>(object), coord);

    /*
    **  If the candidate object is owned by the designated enemy of this house, then
    **  give it a higher value. This will tend to gravitate attacks toward the main
    **  antagonist of this house.
    */
    if (House->field_11C && House->Enemy != HOUSE_NONE && object->House != Houses[House->Enemy]) {
        value = 1;
    }

    /*
    **  If power plants are to be considered a greater threat, then increase
    **  their value here. Buildings that produce no power are not considered
    **  a threat.
    */
    if (method & THREAT_POWER && otype == RTTI_BUILDING) {
        if (static_cast<BuildingTypeClass const*>(tclass)->Power > 0) {
            value += static_cast<BuildingTypeClass const*>(tclass)->Power * 1000;
        } else {
            value = 0;
        }
    }

    /*
    **  If factories are to be considered a greater threat, then don't
    **  consider any non-factory building.
    */
    if (method & THREAT_FACTORIES && otype == RTTI_BUILDING) {
        if (static_cast<BuildingTypeClass const*>(tclass)->ToBuild == RTTI_NONE) {
            value = 0;
        }
    }

    /*
    **  Rampastring:
    **  If we cannot passively acquire the target, then ignore it.
    */
    if (object->RTTI != RTTI_BUILDING || RTTI != RTTI_INFANTRY || !reinterpret_cast<const InfantryClass*>(this)->Class->IsBomber) {
        if (which != WEAPON_SLOT_NONE) {
            const WeaponTypeClass* weapon = TClass->Fetch_Weapon_Info(which).Weapon;
            if (weapon != nullptr) {
                WarheadTypeClass* warhead = weapon->WarheadPtr;
                if (warhead != nullptr) {
                    if (!Verses::Get_PassiveAcquire(object->TClass->Armor, warhead)) value = 0;
                }
            }
        }
    }

    /*
    **  If base defensive structures are to be considered a greater threat, then
    **  don't consider an unarmed building to be a threat.
    **
    **  Rampastring: Only ignore unarmed buildings, not all unarmed objects.
    */
    if (method & THREAT_BASE_DEFENSE) {
        if (object->PrimaryWeapon == nullptr) {
            value = object->RTTI == RTTI_BUILDING ? 0 : 1;
        }
    }

    /*
    **  Possibly cause a reduction of the target's value if it is nearby friendly
    **  structures and the primary weapon of this object is flagged for
    **  friendly fire supression special check logic.
    */
    double areamod = Area_Modify(object->Center_Coord().As_Cell());
    if (areamod != 1) {
        value = areamod * value;
    }

    /*
    **  Lessen threat as a factor of distance.
    */
    if (value) {
        value = std::max(1, value);
        return true;
    }
    value = 0;
    return false;
}


/**
 *  Fixes a bug where placed buildings are not revealed for allies.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x0062AB5E, _TechnoClass_Revealed_Look_For_Allies_Patch, 0)
{
    GET(TechnoClass*, this_ptr, ESI);
    if (this_ptr->IsOwnedByPlayer || (Session.Type != GAME_NORMAL && Rule->IsAllyReveal && this_ptr->House->Is_Ally(PlayerPtr))) {
        // Tell the object to "look", revealing shroud around the object based on its sight range
        return 0x0062AB9F;
    }

    // Player discovered a previously hidden object that wasn't owned by them or revealed by alliance,
    // trigger TEVENT_DISCOVERED
    return 0x0062AB68;
}


/**
 *  Implements flashing for the Iron Curtain.
 *
 *  Author: tomsons26/ZivDero for original function code, Rampastring for Iron Curtain effect.
 */
int TechnoClassExt::_Apparent_Brightness(int brightness) const
{
    TechnoClassExtension* technoext = Extension::Fetch(this);

    if (technoext->IronCurtainTimer > 0) {
        int index = (technoext->IronCurtainTimer.Value() / RuleExtension->IronCurtainFlashRate) % RuleExtension->IronCurtainPulseTable.Count();
        int extra = RuleExtension->IronCurtainPulseTable[index] * RuleExtension->IronCurtainFlashIntensityMultiplier;

        float scale = Map[Center_Coord()].TileBrightness / 1000.0f;
        int extrascaled = (int)(extra * scale);
        int total = brightness + extrascaled;
        if (total < 0) total = 0;

        return total;
    }

    if (((FlashCount / 2) % 2) == 1) {
        return (brightness > 1500 ? 500 : 2000);
    } else {
        return (brightness);
    }
}


void TechnoClassExt::_Flashing_AI()
{
    int flash = FlashCount;
    unsigned b = ((FlashCount / 2) % 2) == 1;
    TechnoClassExtension* technoext = Extension::Fetch(this);

    if (FlasherClass::Process() || technoext->IronCurtainTimer > 0) {
        Mark(MARK_CHANGE);
    }

    if (RTTI == RTTI_BUILDING && (technoext->IronCurtainTimer > 0 || (flash && flash != FlashCount))) {
        unsigned b2 = ((FlashCount / 2) % 2) == 1;
        if (b != (unsigned char)b2) {
            TacticalMap->Register_Dirty_Area(entry_118(), false);
            ((BuildingClass*)this)->Update_Anim_Appearance();
        }
    }
}


/**
 *  Makes the game redraw an object while it is flashing with the Iron Curtain effect.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x0062ECE3, _TechnoClass_AI_Iron_Curtain_Flash_Redraw_Patch, 0)
{
    GET(TechnoClassExt*, this_ptr, ESI);

    this_ptr->_Flashing_AI();

    return 0x0062ED7A;
}


/**
 *  Fixes a bug where a unit believes it is not AA-capable when its primary weapon has no AA capability,
 *  even when its secondary weapon is AA-capable.
 *
 *  Author: Rampastring
 */
int TechnoClassExt::_Anti_Air(void) const
{
    assert(IsActive);

    if (Is_Weapon_Equipped()) {

        WeaponTypeClass const* weapon = PrimaryWeapon;
        BulletTypeClass const* bullet = weapon->Bullet;
        WarheadTypeClass const* warhead = weapon->WarheadPtr;

        if (bullet->IsAntiAircraft) {
            int value = ((weapon->Attack * warhead->Modifier[ARMOR_ALUMINUM]) * weapon->Range) / weapon->ROF;

            if (TClass->Is_Two_Shooter()) {
                value *= 2;
            }
            return value / 50;
        }

        // If the primary weapon is not AA-capable, check if we have a secondary weapon that is AA-capable.
        weapon = SecondaryWeapon;
        if (weapon) {
            bullet = weapon->Bullet;
            warhead = weapon->WarheadPtr;

            if (bullet->IsAntiAircraft) {
                int value = ((weapon->Attack * warhead->Modifier[ARMOR_ALUMINUM]) * weapon->Range) / weapon->ROF;

                return value / 50;
            }
        }
    }
    return (0);
}


/*
 *  Reimplements TechnoClass::Is_Allowed_To_Recloak.
 *  Adds a check for nearby enemy technos around it preventing it from cloaking by having sensors capabilities.
 *  Uses the same logic as when cloaked units move to new cells, which has the same check forcing them to uncloak.
 */
bool TechnoClassExt::_Is_Allowed_To_Recloak()
{
    if (!IsCloakable) {
        return false;
    }

    Cell cell = Center_Coord().As_Cell();    

    // Get all adjacent cells and check if there are units with Sensors or sensing ability.
    for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
        Cell adjacent_cell = Adjacent_Cell(cell, dir);
        CellClass* adjacent_cellptr = &Map[adjacent_cell];

        if (!Map.In_Local_Radar(adjacent_cell)) {
            continue;
        }

        TechnoClass* cell_techno = adjacent_cellptr->Cell_Techno();
        if (cell_techno != nullptr) {
            if (!cell_techno->House->Is_Ally(this) && (cell_techno->TClass->IsScanner || cell_techno->Has_Ability(ABILITY_SENSORS))) {
                return false;
            }
        }
    }

    return true;
}


/*
 *  Reimplements TechnoClass::Look based on the RE project code
 *  Additionally, adds the Veteran and Elite Sight ranges that are used when the techno is either Veteran or Elite, respectively.
 * 
 *  @author: JoyfulShush
 */
void TechnoClassExt::_Look(bool incremental, bool dontmap)
{    
    assert(!IsInLimbo);

    if (IsLocked && (!House->Class->IsMultiplayPassive || Session.Type == GAME_NORMAL)) {
        int sight_increase = 10 * (Get_Coord().Z / Rule->LeptonsPerSightIncrease);
        if (sight_increase > SightIncrease) {
            incremental = false;
        }
        SightIncrease = sight_increase;

        int sight_range = Extension::Fetch(this)->Get_Sight_Range();

        if (sight_range) {
            HouseClass* house = House;
            if (((1 << PlayerPtr->HeapID) & LimpetType) != 0) {
                house = PlayerPtr;
            }
            Map.Sight_From(PositionCoord, sight_range, house, incremental, dontmap);
        }
    }
}


/**
 *  Patches Find_Docking_Bay to make aircraft search for an un-occupied dock to land on.
 *  This is achieved by changing the "only_unocciped" parameter (third argument, bool) into true when it's aircraft.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x00637F0B, _TechnoClass_Find_Docking_Bay_Unoccupied_Aircraft_Patch, 6)
{
    GET(TechnoClass*, this_ptr, ESI);
    REF_STACK(bool, only_unoccupied_ptr, 0x48);

    if (this_ptr->Fetch_RTTI() == RTTI_AIRCRAFT) {
        only_unoccupied_ptr = true;
    }

    return 0;
}


/**
 *  Reimplements TechnoClass::AI where the game tests for AI units that are firing upon a target that is allied.
 *  Fixes an issue where it would cause medics (and by extension: mechanics and omnihealers) to constantly lose their targeting,
 *  causing them to never finish their "attack" animation and never actually heal the target unless they were prone.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0062E799, _TechnoClass_AI_Medics_Lose_Targets_AI_Houses, 0)
{
    GET(TechnoClass*, this_ptr, ESI);

    if (this_ptr->House->Is_Ally(this_ptr->TarCom) && this_ptr->Combat_Damage() >= 0) {
        if (this_ptr->RTTI != RTTI_AIRCRAFT && (this_ptr->RTTI != RTTI_INFANTRY || !((InfantryClass*)this_ptr)->Class->IsEngineer)) {
            this_ptr->Assign_Target(NULL);
        }
    }

    return 0x0062E7DD;
}


/**
 *  Patches TechnoClass::AI to allow customizing the amount of health regenerated whenever a unit self-heals.
 *  Can be configured on both the techno level and globally.
 *  Defaults to 1 when the key is omitted.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x0062E9EF, TechnoClass_AI_Self_Heal_Repair_Step, 0)
{
    GET(TechnoClass*, this_ptr, ESI);

    auto this_ptr_ext = Extension::Fetch(this_ptr->TClass);

    int strength_to_recover = this_ptr_ext->SelfHealingStep > 0 
        ? this_ptr_ext->SelfHealingStep
        : RuleExtension->SelfHealingStep;
    
    int max_strength = this_ptr->TClass->MaxStrength;

    // Don't allow to self-heal 0 health or negative values
    if (strength_to_recover > 0) {
        // Don't allow unit to recover strength beyond its max strength
        this_ptr->Strength = std::min(this_ptr->Strength + strength_to_recover, max_strength);
    }

    // Stolen bytes
    R->EAX(this_ptr->Strength);
    R->ECX(this_ptr);

    return 0x0062E9F8;
}


/**
 *  Main function for patching the hooks.
 */
void TechnoClassExtension_Hooks()
{
    Patch_Jump(0x00637540, &TechnoClassExt::_Draw_Pips);
    Patch_Jump(0x0062A0D0, &TechnoClassExt::_What_Weapon_Should_I_Use);
    Patch_Jump(0x00636F00, &TechnoClassExt::_Is_Allowed_To_Retaliate);
    Patch_Jump(0x00639810, &TechnoClassExt::_Target_Threat);
    Patch_Jump(0x00638240, &TechnoClassExt::_Anti_Infantry);
    Patch_Call(0x0042EC25, &TechnoClassExt::_What_Action);
    Patch_Call(0x004A8532, &TechnoClassExt::_What_Action);
    Patch_Jump(0x00632070, &TechnoClassExt::_Can_Deploy_Now);
    Patch_Call(0x00637FF5, &TechnoClassExt::_Cell_Distance_Squared); // Patch Find_Docking_Bay to call our own distance function that avoids overflows
    Patch_Jump(0x006313D0, &TechnoClassExt::_Draw_Target_Laser);
    Patch_Jump(0x00637D60, &TechnoClassExt::_Draw_Text_Overlay);
    Patch_Jump(0x006364A0, &TechnoClassExt::_Crew_Type);
    Patch_Jump(0x0062A300, &TechnoClassExt::_How_Many_Survivors);
    Patch_Jump(0x00637450, &TechnoClassExt::_Target_Something_Nearby);
    Patch_Jump(0x0062FD20, &TechnoClassExt::_Stun);
    Patch_Call(0x0062E9D1, &TechnoClassExt::_Mission_AI);
    Patch_Jump(0x0062F980, &TechnoClassExt::_Can_Fire);
    Patch_Jump(0x00631FF0, &TechnoClassExt::_Can_Player_Move);
    Patch_Jump(0x006336F0, &TechnoClassExt::_Record_The_Kill);
    //Patch_Jump(0x0062A3D0, &TechnoClassExt::_Fire_Coord); // Disabled because it's functionally identical to the vanilla function when there's no secondary coordinate
    Patch_Jump(0x00633745, 0x00633762); // Do not trigger "Discovered by Player" when an object is destroyed
    Patch_Jump(0x0062A970, &TechnoClassExt::_Time_To_Build);
    Patch_Jump(0x0062FD70, &TechnoClassExt::_Assign_Target);
    Patch_Jump(0x00638090, &TechnoClassExt::_Refund_Amount);
    Patch_Jump(0x0062D0F0, &TechnoClassExt::_Evaluate_Object);
    Patch_Jump(0x00638CA0, &TechnoClassExt::_Should_Self_Heal_Now);
    Patch_Jump(0x00639C70, &TechnoClassExt::_Apparent_Brightness);
    Patch_Jump(0x006380F0, &TechnoClassExt::_Anti_Air);
    Patch_Jump(0x00638310, &TechnoClassExt::_Look);
    Patch_Jump(0x00639120, &TechnoClassExt::_Is_Allowed_To_Recloak);
}
