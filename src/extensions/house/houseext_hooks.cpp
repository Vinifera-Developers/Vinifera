/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended HouseClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "houseext_hooks.h"

#include "asserthandler.h"
#include "building.h"
#include "buildingtypeext.h"
#include "ccini.h"
#include "debughandler.h"
#include "extension_globals.h"
#include "factory.h"
#include "fatal.h"
#include "hooker.h"
#include "house.h"
#include "houseext.h"
#include "houseext_init.h"
#include "housetype.h"
#include "mouse.h"
#include "msgbox.h"
#include "prerequisitegroup.h"
#include "rules.h"
#include "rulesext.h"
#include "session.h"
#include "sideext.h"
#include "super.h"
#include "syringe.h"
#include "techno.h"
#include "technoext.h"
#include "technotype.h"
#include "tiberium.h"
#include "tibsun_globals.h"
#include "unittype.h"
#include "unittypeext.h"
#include "vinifera_globals.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(HouseClass)
{
public:
    int _AI_Building();
    int _AI_Unit();
    int _Expert_AI();
    bool _Can_Build_Required_Forbidden_Houses(const TechnoTypeClass* techno_type);
    void _Active_Remove(TechnoClass const* techno);
    void _Active_Add(TechnoClass const* techno);
    Cell _Find_Build_Location(BuildingTypeClass* btype, int(__fastcall* callback)(int, Cell&, int, int), int a3 = -1);
    void _Production_Check();
    bool _AI_Has_Prerequisites(const TechnoTypeClass* type, DynamicVectorClass<const BuildingTypeClass*>& owned, int ownedcount) const;
    void _Harvested(int tiberium, TiberiumType slot);

    // stubs
    FactoryClass* _Fetch_Factory(RTTIType rtti);
    void _Set_Factory(RTTIType rtti, FactoryClass* factory);
    int* _Factory_Counter(RTTIType rtti);
    int _Factory_Count(RTTIType rtti) const;
    ProdFailType _Suspend_Production(RTTIType type);
    ProdFailType _Begin_Production(RTTIType type, int id, bool resume);
    ProdFailType _Abandon_Production(RTTIType type, int id);
    bool _Place_Object(RTTIType type, Cell const& cell);
    void _Update_Factories(RTTIType rtti);
    TechnoTypeClass const* _Suggest_New_Object(RTTIType objecttype, bool kennel) const;
};


/**
 *  Determines what building to build.
 *
 *  @author: 09/29/1995 JLB - Created.
 *           ZivDero - Adjustments for Tiberian Sun
 */
int HouseClassExt::_AI_Building()
{
    enum {
        BASE_WALL = -3,
        BASE_UNKNOWN = -2,
        BASE_DEFENSE = -1
    };

    /**
     *  Unfortunately, ts-patches spawner has a hack here.
     *  Until we reimplement the spawner in Vinifera, this will have to do.
     */
    static bool spawner_hack_init = false;
    static bool spawner_hack_mpnodes = false;

    if (!spawner_hack_init)
    {
        RawFileClass file("SPAWN.INI");
        CCINIClass spawn_ini;

        if (file.Is_Available()) {

            spawn_ini.Load(file, false);
            spawner_hack_mpnodes = spawn_ini.Get_Bool("Settings", "UseMPAIBaseNodes", spawner_hack_mpnodes);
        }

        spawner_hack_init = true;
    }


    if (BuildStructure != STRUCT_NONE) return TICKS_PER_SECOND;

    if (ConstructionYards.Count() == 0) return TICKS_PER_SECOND;

    BaseNodeClass* node = Base.Next_Buildable();

    if (!node) return TICKS_PER_SECOND;

    /**
     *  Build some walls.
     */
    if (node->Type == BASE_WALL) {
        Base.Nodes.Delete(*node);
        AI_Build_Wall();
        return 1;
    }

    /**
     *  Build some defenses.
     */
    if (node->Type == BASE_DEFENSE || BuildingTypes[node->Type] == Rule->WallTower && node->CellID == Cell(0, 0)) {

        const int nodeid = Base.Nodes.ID(node);
        if (!AI_Build_Defense(nodeid, Base.field_38.Count() > 0 ? &Base.field_38 : nullptr)) {

            /**
             *  If it's a wall tower, delete it twice?
             *  Perhaps it's assumed that the wall tower is followed by its upgrade?
             */
            if (node->Type == Rule->WallTower->HeapID) {
                Base.Nodes.Delete(nodeid);
            }

            /**
             *  Remove the node from the list.
             */
            Base.Nodes.Delete(nodeid);
            return 1;
        }

        node = Base.Next_Buildable();
    }

    if (!node || node->Type == BASE_UNKNOWN) return TICKS_PER_SECOND;

    /**
     *  In campaigns, or if we have enough power, or if we're trying to building a construction yard,
     *  just proceed with building the base node.
     */
    BuildingTypeClass* b = BuildingTypes[node->Type];

    if (Session.Type != GAME_NORMAL && !spawner_hack_mpnodes && b->Drain + Drain > Power - PowerSurplus && b != Rule->BuildConst[0] && b->Drain > 0) {

        /**
         *  In skirmish, try to build a power plant if there is insufficient power.
         */
        const BuildingTypeClass* choice = nullptr;
        const auto side_ext = Extension::Fetch(Sides[Class->Side]);

        /**
         *  First let's see if we can upgrade a power plant with a turbine (like GDI).
         */
        if (side_ext->PowerTurbine) {

            bool can_build_turbine = false;
            for (int i = 0; i < Buildings.Count(); i++) {

                BuildingClass* owned_b = Buildings[i];
                if (owned_b->Owner_HouseClass() == this) {
                    if (owned_b->Class == side_ext->RegularPowerPlant && owned_b->UpgradeLevel < owned_b->Class->Upgrades) {
                        can_build_turbine = true;
                        break;
                    }
                }
            }

            if (can_build_turbine && Probability_Of2(Rule->AIUseTurbineUpgradeProbability)) {
                choice = side_ext->PowerTurbine;
            }
        }

        /**
         *  If we can't build a turbine, try to build an advanced power plant (like Nod).
         */
        if (!choice && side_ext->AdvancedPowerPlant) {
            DynamicVectorClass<BuildingTypeClass*> owned_buildings;

            for (int i = 0; i < Buildings.Count(); i++) {
                BuildingClass* b2 = Buildings[i];
                if (b2->Owner_HouseClass() == this) {
                    owned_buildings.Add(b2->Class);
                }
            }

            if (Has_Prerequisites(side_ext->AdvancedPowerPlant, owned_buildings, owned_buildings.Count())) {
                choice = side_ext->AdvancedPowerPlant;
            }
        }

        /**
         *  If neither worked out, just build a normal power plant.
         */
        if (!choice) {
            choice = side_ext->RegularPowerPlant;
        }

        /**
         *  Build our chosen power structure before building whatever else we're trying to build.
         */
        const int id = Base.Nodes.ID(node);
        Base.Nodes.Insert(id, BaseNodeClass(choice->HeapID, Cell(0, 0)));

        return 1;

    }

    /**
     *  Check if this is a building upgrade if we can actually place the upgrade where it's scheduled to be placed.
     */
    if (b->PowersUpToLevel == -1 && node->CellID != Cell(0, 0) && !b->PowersUpBuilding.empty()) {

        BuildingClass* existing_building = Map[node->CellID].Cell_Building();
        BuildingTypeClass* node_building = BuildingTypes[BuildingTypeClass::From_Name(b->PowersUpBuilding.c_str())];

        if (existing_building == nullptr) {
            node->CellID = Cell(0, 0);
        }
        else if (existing_building->Class != node_building) {
            node->CellID = Cell(0, 0);
        }
        else if (existing_building->Class->PowersUpToLevel == -1 && existing_building->UpgradeLevel >= existing_building->Class->Upgrades || existing_building->Class->PowersUpToLevel > 0 && existing_building->UpgradeLevel > 0) {
            node->CellID = Cell(0, 0);
        }
    }

    BuildStructure = node->Type;
    return TICKS_PER_SECOND;
}


int HouseClassExt::_AI_Unit()
{
    auto extension = Extension::Fetch(this);
    int delay1 = extension->AI_Unit();
    int delay2 = extension->AI_Naval_Unit();
    return std::min(delay1, delay2);
}


/**
 *  Handles expert AI processing.
 *
 *  @author: 09/29/1995 JLB - Created.
 *           10/11/2024 ZivDero - Adjustments for Tiberian Sun
 */
int HouseClassExt::_Expert_AI()
{
    /**
     *  Unfortunately, ts-patches spawner has a hack here.
     *  Until we reimplement the spawner in Vinifera, this will have to do.
     */
    static bool spawner_hack_init = false;
    static bool spawner_hack_mpnodes = false;

    if (!spawner_hack_init)
    {
        RawFileClass file("SPAWN.INI");
        CCINIClass spawn_ini;

        if (file.Is_Available()) {

            spawn_ini.Load(file, false);
            spawner_hack_mpnodes = spawn_ini.Get_Bool("Settings", "UseMPAIBaseNodes", spawner_hack_mpnodes);
        }

        spawner_hack_init = true;
    }

    /**
     *  If there is no enemy assigned to this house, then assign one now. The
     *  enemy that is closest is picked. However, don't pick an enemy if the
     *  base has not been established yet.
     */
    if (ExpertAITimer.Expired()) {
        if (Enemy == HOUSE_NONE && Session.Type != GAME_NORMAL && !Class->IsMultiplayPassive && Center != COORD_NONE) {
            int close = INT_MAX;
            HouseClass* enemy = nullptr;

            for (int i = 0; i < Houses.Count(); i++) {
                HouseClass* house = Houses[i];

                if (house != this && !house->Class->IsMultiplayPassive && !house->IsDefeated && !Is_Ally(house)) {

                    /**
                     *  Determine a priority value based on distance to the center of the
                     *  candidate base. The higher the value, the better the candidate house
                     *  is to becoming the preferred enemy for this house.
                     */
                    const int value = Distance(Center, house->Center);

                    /**
                     *  Compare the calculated value for this candidate house and if it is
                     *  greater than the previously recorded maximum, record this house as
                     *  the prime candidate for enemy.
                     */
                    if (value < close) {
                        close = value;
                        enemy = house;
                    }
                }
            }

            /**
             *  Record this closest enemy base as the first enemy to attack.
             */
            if (enemy) {
                Add_Anger(1, enemy);
            }
        }
    }

    /**
     *  If the current enemy no longer has a base or is defeated, then don't consider
     *  that house a threat anymore. Clear out the enemy record and then try
     *  to find a new enemy.
     */
    if (Enemy != HOUSE_NONE) {
        HouseClass* h = Houses[Enemy];

        if (h->IsDefeated || Is_Ally(h)) {
            Clear_Anger(h);
            Enemy = HOUSE_NONE;
        }
    }

    /**
     *  Use any ready super weapons.
     */
    if (Session.Type != GAME_NORMAL || IQ >= Rule->IQSuperWeapons) {
        AI_Super_Weapons();
    }

    /**
     *  House state transition check occurs here. Transitions that occur here are ones
     *  that relate to general base condition rather than specific combat events.
     *  Typically, this is limited to transitions between normal buildup mode and
     *  broke mode.
     */
    if (State == STATE_ENDGAME) {
        Fire_Sale();
        All_To_Hunt();
    }
    else {
        if (State == STATE_BUILDUP) {
            if (Available_Money() < 25) {
                State = STATE_BROKE;
            }
        }
        if (State == STATE_BROKE) {
            if (Available_Money() >= 25) {
                State = STATE_BUILDUP;
            }
        }
        if (State == STATE_ATTACKED && LATime + TICKS_PER_MINUTE < Frame) {
            State = STATE_BUILDUP;
        }
        if (State != STATE_ATTACKED && LATime + TICKS_PER_MINUTE > Frame) {
            State = STATE_ATTACKED;
        }
    }

    if (Session.Type != GAME_NORMAL && !spawner_hack_mpnodes) {

        /**
         *  Records the urgency of all actions possible.
         */
        UrgencyType urgency[STRATEGY_COUNT];
        StrategyType strat;
        for (strat = STRATEGY_FIRST; strat < STRATEGY_COUNT; strat++) {
            urgency[strat] = URGENCY_NONE;

            switch (strat) {
            case STRATEGY_FIRE_SALE:
                urgency[strat] = Check_Fire_Sale();
                break;

            case STRATEGY_RAISE_MONEY:
                urgency[strat] = Check_Raise_Money();
                break;

            default:
                urgency[strat] = URGENCY_NONE;
                break;
            }
        }

        /**
         *  Performs the action required for each of the strategies that share
         *  the most urgent category. Stop processing if any strategy at the
         *  highest urgency performed any action. This is because higher urgency
         *  actions tend to greatly affect the lower urgency actions.
         */
        for (UrgencyType u = URGENCY_CRITICAL; u >= URGENCY_LOW; u--) {
            bool acted = false;

            for (strat = STRATEGY_FIRST; strat < STRATEGY_COUNT; strat++) {
                if (urgency[strat] == u) {
                    switch (strat) {
                    case STRATEGY_FIRE_SALE:
                        acted |= AI_Fire_Sale(u);
                        break;

                    case STRATEGY_RAISE_MONEY:
                        acted |= AI_Raise_Money(u);
                        break;

                    default:
                        break;
                    }
                }
            }
        }
    }

    return TICKS_PER_SECOND * 7 + Random_Pick(1, TICKS_PER_SECOND / 2);
}


/**
 *  Patch to optionally disable Tiberium storage.
 *
 *  @author: ZivDero, tomsons26, Rampastring
 */
void HouseClassExt::_Harvested(int tiberium, TiberiumType slot)
{
    PointTotal += tiberium * 5;

    if ((Session.Type != GAME_NORMAL && !IsHuman) || !RuleExtension->IsTiberiumStorage) {
        Credits += tiberium * Tiberiums[slot]->CreditValue;
    }
    else {
        long oldcap = Capacity;
        long oldtib = Tiberium.Get_Total_Amount();

        if (tiberium + Tiberium.Get_Total_Amount() > Capacity) {
            tiberium = Capacity - Tiberium.Get_Total_Amount();
        }

        for (int index = 0; index < Buildings.Count(); index++) {
            BuildingClass* b = Buildings[index];
            if (b && b->IsDown && b->House == this) {
                if (b->Class->Storage > 0) {
                    while (tiberium > 0 && b->Class->Storage > b->Storage.Get_Total_Amount()) {
                        b->Storage.Increase_Amount(1, slot);
                        Tiberium.Increase_Amount(1, slot);
                        tiberium--;
                    }
                }
            }
        }

        Silo_Redraw_Check(oldtib, oldcap);
    }
}


/**
 *  Patch for InstantSuperRechargeCommandClass
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004BD30B, _HouseClass_Super_Weapon_Handler_InstantRecharge_Patch, 0)
{
    GET(HouseClass *, this_ptr, EDI);
    GET(SuperClass *, special, ESI);

    bool is_player = false;
    if (this_ptr == PlayerPtr) {
        is_player = true;
    }

    if (Vinifera_DeveloperMode) {
        if (!special->IsReady) {

            /**
             *  If AIInstantBuild is toggled on, make sure this is a non-human AI house.
             */
            if (Vinifera_Developer_AIInstantSuperRecharge && !this_ptr->Is_Human_Player() && this_ptr != PlayerPtr) {
                special->Forced_Charge(is_player);

            /**
             *  If InstantBuild is toggled on, make sure the local player is a human house.
             */
            } else if (Vinifera_Developer_InstantSuperRecharge && this_ptr->Is_Human_Player() && this_ptr == PlayerPtr) {
                special->Forced_Charge(is_player);

            /**
             *  If the AI has taken control of the player house, it needs a special
             *  case to handle the "player" instant recharge mode.
             */
            } else if (Vinifera_Developer_InstantSuperRecharge) {
                if (Vinifera_Developer_AIControl && this_ptr == PlayerPtr) {
                    
                    special->Forced_Charge(is_player);
                }
            }

        }

    }

    /**
     *  Stolen bytes/code.
     */
    if (!special->AI(is_player)) {
        return 0x004BD332;
    }

    return 0x004BD320;
}


/**
 *  Patch for BuildCheatCommandClass
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004BBD26, _HouseClass_Can_Build_BuildCheat_Patch, 8)
{
    GET(HouseClass *, this_ptr, EBP);
    GET_STACK(TechnoTypeClass *, objecttype, 0x30);

    if (Vinifera_DeveloperMode && Vinifera_Developer_BuildCheat) {

        /**
         *  AI houses have access to everything, so we can just
         *  filter to the human houses only.
         */
        if (this_ptr->IsHuman && this_ptr->IsPlayerControl) {

            /**
             *  Check that the object has this house set as one of its owners.
             *  if true, force this 
             */
            if ((1 << this_ptr->Class->HeapID & objecttype->Get_Ownable()) != 0) {
                //DEBUG_INFO("Forcing \"{}\" available.\n", objecttype->IniName);
                return 0x004BBD17;
            }
        }
    }

    return 0;
}


/**
 *  #issue-611, #issue-715
 *
 *  Gets the number of queued objects when determining whether a cameo
 *  should be disabled.
 *
 *  Author: Rampastring
 */
int _HouseClass_ShouldDisableCameo_Get_Queued_Count(FactoryClass* factory, TechnoTypeClass* technotype)
{
    int count = factory->Total_Queued(*technotype);
    TechnoClass* factoryobject = factory->Get_Object();

    if (factoryobject == nullptr || count == 0) {
        return 0;
    }

    /**
     *  Check that the factory is trying to create the object that the player is trying to queue
     *  If not, we don't need to mess with the count
     */
    if (factoryobject->TClass != technotype) {
        return count;
    }

    /**
     *  #issue-611
     *
     *  If the object has a build limit, then reduce count by 1.
     *  In this state, the object is taken into account twice: in the object trackers
     *  and in the factory, resulting in the player being able to queue one object less
     *  than BuildLimit allows.
     */
    if (technotype->BuildLimit > 0) {
        count--;
    }

    /**
    *  #issue-715
    *
    *  If the object can transform into another object through our special logic,
    *  then check that doing so doesn't allow circumventing build limits
    */
    if (technotype->RTTI == RTTI_UNITTYPE) {
        UnitTypeClass* unittype = reinterpret_cast<UnitTypeClass*>(technotype);
        UnitTypeClassExtension* unittypeext = Extension::Fetch(unittype);

        if (unittype->DeploysInto == nullptr && unittypeext->TransformsInto != nullptr) {
            count += factory->House->UQuantity.Value((UnitType)unittypeext->TransformsInto->Fetch_Heap_ID());
        }
    }

    return count;
}


/**
 *  #issue-611 #issue-715
 *
 *  Fixes the game allowing the player to queue one unit too few
 *  when a unit has BuildLimit > 1.
 *
 *  Also updates the build limit logic with unit queuing to
 *  take our unit transformation logic into account.
 */
DEFINE_HOOK(0x004CB777, _HouseClass_ShouldDisableCameo_BuildLimit_Fix, 0)
{
    GET(FactoryClass*, factory, ECX);
    GET(TechnoTypeClass*, technotype, ESI);

    int queuedcount = _HouseClass_ShouldDisableCameo_Get_Queued_Count(factory, technotype);
    R->EAX(queuedcount);

    return 0x004CB77D;
}


/**
 *  #issue-715
 *
 *  Take vehicles that can transform into other vehicles into acccount when
 *  determining whether a build limit has been met/exceeded.
 *  Otherwise these kinds of units could be used to bypass build limits
 *  (build a limited vehicle, transform it, now you can build another vehicle).
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x004BC187, _HouseClass_Can_Build_BuildLimit_Handle_Vehicle_Transform, 0)
{
    GET(UnitTypeClass*, unittype, EDI);
    GET(HouseClass*, house, EBP);

    UnitTypeClassExtension* unittypeext = Extension::Fetch(unittype);

    /**
     *  Stolen bytes / code.
     */
    int objectcount = house->UQuantity.Value((UnitType)unittype->Fetch_Heap_ID());

    /**
     *  Check whether this unit can deploy into a building.
     *  If it can, increment the object count by the number of buildings.
     */
    if (unittype->DeploysInto != nullptr) {
        objectcount += house->BQuantity.Value((StructType)unittype->DeploysInto->Fetch_Heap_ID());
    }
    else if (unittypeext->TransformsInto != nullptr) {

        /**
         *  This unit can transform into another unit, increment the object count
         *  by the number of transformed units.
         */
        objectcount += house->UQuantity.Value((UnitType)unittypeext->TransformsInto->Fetch_Heap_ID());
    }

    R->ESI(objectcount);

    return 0x004BC1B9;
}


/**
 *  #issue-994
 *
 *  Fixes a bug where a superweapon was enabled in non-suspended mode
 *  when the scenario was started with a pre-placed powered-down superweapon
 *  building on the map.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x004CB6C1, _HouseClass_Enable_SWs_Check_For_Building_Power, 6)
{
    GET(int, quiet, EAX);
    GET(BuildingClass*, building, ESI);

    if (!building->IsOn)
    {
        /**
         *  Enable the superweapon in suspended mode.
         */
        R->EAX(true);
    }
    else
    {
        /**
         *  Enable the superweapon in non-suspended mode.
         */
        R->EAX(false);
    }

    /**
     *  Continue the SW enablement process.
     */
    return 0;
}


/**
 *  Checks if the TechnoType can be built by this house based on RequiredHouses and ForbiddenHouses, if set.
 *
 *  Author: ZivDero, Rampastring
 */
bool HouseClassExt::_Can_Build_Required_Forbidden_Houses(const TechnoTypeClass* techno_type)
{
    const auto technotypeext = Extension::Fetch(techno_type);

    if (technotypeext->RequiredHouses != -1 &&
        (technotypeext->RequiredHouses & 1 << ActLike) == 0)
    {
        return false;
    }

    if (technotypeext->ForbiddenHouses != -1 &&
        (technotypeext->ForbiddenHouses & 1 << ActLike) != 0)
    {
        return false;
    }

    return true;
}


/**
 *  Reimplementation of HouseClass::Active_Remove.
 *
 *  @author: ZivDero
 */
void HouseClassExt::_Active_Remove(TechnoClass const* techno)
{
    if (techno->RTTI == RTTI_BUILDING) {
        int* fptr = Extension::Fetch(this)->Factory_Counter(((BuildingClass*)techno)->Class->ToBuild,
            Extension::Fetch(((BuildingClass*)techno)->Class)->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);
        if (fptr != nullptr) {
            *fptr = *fptr - 1;
        }
    }
}


/**
 *  Reimplementation of HouseClass::Active_Add.
 *
 *  @author: ZivDero
 */
void HouseClassExt::_Active_Add(TechnoClass const* techno)
{
    if (techno->RTTI == RTTI_BUILDING) {
        int* fptr = Extension::Fetch(this)->Factory_Counter(((BuildingClass*)techno)->Class->ToBuild,
            Extension::Fetch(((BuildingClass*)techno)->Class)->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);
        if (fptr != nullptr) {
            *fptr = *fptr + 1;
        }
    }
}


/**
 *  #issue-531
 *
 *  Interception of Find_Build_Location. This allows us to find a suitable building
 *  location for the specific buildings, such as the Naval Yard.
 *
 *  @author: CCHyper
 */
Cell HouseClassExt::_Find_Build_Location(BuildingTypeClass* btype, int(__fastcall* callback)(int, Cell&, int, int), int a3)
{
    /**
     *  Fix an edge case crash where this function is called with a null btype.
     *  @author: Rampastring
     */
    if (btype == nullptr) {
        return Cell(0, 0);
    }

    /**
     *  Find the type class extension instance.
     */
    BuildingTypeClassExtension* buildingtypeext = Extension::Fetch(btype);
    if (buildingtypeext && buildingtypeext->IsNaval) {

        DEV_DEBUG_INFO("Find_Build_Location({}): Searching for Naval Yard \"{}\" build location...\n", IniName, btype->Name());

        Cell cell(0, 0);

        /**
         *  Get the cell footprint for the Naval Yard, then add a safety margin of 2.
         */
        int area_w = btype->Width() + 2;
        int area_h = btype->Height() + 2;

        /**
         *  find a nearby location from the center of the base that fits our naval yard.
         */
        Cell found_cell = Map.Nearby_Location(Center.As_Cell(), SPEED_FLOAT, -1, MZONE_NORMAL, false, Point2D(area_w, area_h));
        if (found_cell != CELL_NONE) {

            DEV_DEBUG_INFO("Find_Build_Location({}): Found possible Naval Yard location at {},{}...\n", IniName, found_cell.X, found_cell.Y);

            /**
             *  Iterate over all owned construction yards and find the first that is closest to our cell.
             */
            for (int i = 0; i < ConstructionYards.Count(); ++i) {
                BuildingClass* conyard = ConstructionYards[i];
                if (conyard) {

                    Coord conyard_coord = conyard->Center_Coord();
                    Coord found_coord = Map[found_cell].Center_Coord();

                    /**
                     *  Is this location close enough to the construction yard for us to use?
                     */
                    if (Distance(conyard_coord, found_coord) <= Cell_To_Lepton(RuleExtension->AINavalYardAdjacency)) {
                        DEV_DEBUG_INFO("Find_Build_Location({}): Using location {},{} for Naval Yard.\n", IniName, found_cell.X, found_cell.Y);
                        cell = found_cell;
                        break;
                    }
                }
            }
        }

        if (cell == CELL_NONE) {
            DEV_DEBUG_WARNING("Find_Build_Location({}): Failed to find suitable location for \"{}\"!\n", IniName, btype->Name());
        }

        return cell;

    }

    /**
     *  Call the original function to find a location for land buildings.
     */
    return HouseClass::Find_Build_Location(btype, callback, a3);
}


/**
 *  Adds a check to Can_Build to check for RequiredHouses and ForbiddenHouses
 *
 *  Author: ZivDero
 */
DEFINE_HOOK(0x004BBC74, _Can_Build_Required_Forbidden_Houses_Patch, 9)
{
    GET(TechnoTypeClass*, techno_type, EDI);
    GET(HouseClassExt*, this_ptr, EBP);

    bool can_build = this_ptr->_Can_Build_Required_Forbidden_Houses(techno_type);
    if (!can_build) {
        // return false;
        return 0x004BBC9A;
    }

    // Continue Can_Build
    return 0;
}


/**
 *  Allow to skip the check for the MCV's ActLike.
 *
 *  Author: ZivDero
 */
DEFINE_HOOK(0x004BC0B7, _HouseClass_Can_Build_Multi_MCV_Patch, 6)
{
    if (RuleExtension->IsMultiMCV) {
        return 0x004BC102;
    }

    return 0;
}


/**
 *  Handy macro for the functions below.
 */
#define WARN_AND_EXIT(funcname) { \
    DEBUG_FATAL("The legacy version of " STRINGIZE(funcname) " has been called! If you see this, please notify the developers. The game will now exit.\n"); \
    DEBUG_FATAL("Return address: {}\n", _ReturnAddress()); \
    WWMessageBox().Process("The legacy version of " STRINGIZE(funcname) " has been called! If you see this, please notify the developers. The game will now exit.", 0, TXT_OK); \
    Emergency_Exit(0); } \


/**
 *  The below are dummies for the functions that have been completely supplanted by our extension functions.
 *  These ought not to be used.
 */
FactoryClass* HouseClassExt::_Fetch_Factory(RTTIType rtti)
{
    WARN_AND_EXIT(HouseClass::Fetch_Factory);
    return nullptr;
}

void HouseClassExt::_Set_Factory(RTTIType rtti, FactoryClass* factory)
{
    WARN_AND_EXIT(HouseClass::Set_Factory);
}

int* HouseClassExt::_Factory_Counter(RTTIType rtti)
{
    WARN_AND_EXIT(HouseClass::Factory_Counter);
    return nullptr;
}

int HouseClassExt::_Factory_Count(RTTIType rtti) const
{
    WARN_AND_EXIT(HouseClass::Factory_Count);
    return 0;
}

ProdFailType HouseClassExt::_Suspend_Production(RTTIType type)
{
    WARN_AND_EXIT(HouseClass::Suspend_Production);
    return ProdFailType();
}

ProdFailType HouseClassExt::_Begin_Production(RTTIType type, int id, bool resume)
{
    WARN_AND_EXIT(HouseClass::Begin_Production);
    return ProdFailType();
}

ProdFailType HouseClassExt::_Abandon_Production(RTTIType type, int id)
{
    WARN_AND_EXIT(HouseClass::Abandon_Production);
    return ProdFailType();
}

bool HouseClassExt::_Place_Object(RTTIType type, Cell const& cell)
{
    WARN_AND_EXIT(HouseClass::Place_Object);
    return false;
}

void HouseClassExt::_Update_Factories(RTTIType type)
{
    WARN_AND_EXIT(HouseClass::Update_Factories);
}

TechnoTypeClass const* HouseClassExt::_Suggest_New_Object(RTTIType objecttype, bool kennel) const
{
    WARN_AND_EXIT(HouseClass::Suggest_New_Object);
    return nullptr;
}


/**
 *  The patches below replace calls to various HouseClass functions that we've re-implemented
 *  with calls to our extended implementations.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004CB73D, _HouseClass_Exhausted_Build_Limit_Fetch_Factory_Patch, 0)
{
    GET(HouseClass*, this_ptr, EBX);
    GET(TechnoTypeClass const*, ttype, ESI);

    FactoryClass* factory = Extension::Fetch(this_ptr)->Fetch_Factory(ttype->RTTI, TechnoTypeClassExtension::Get_Production_Flags(ttype));
    R->ECX(factory);

    return 0x004CB773;
}


static void Update_Factories_Helper(BuildingClass* building)
{
    if (building->Class->ToBuild != RTTI_NONE) {
        BuildingTypeClassExtension* type_ext = Extension::Fetch(building->Class);
        HouseClassExtension* house_ext = Extension::Fetch(building->House);
        house_ext->Update_Factories(building->Class->ToBuild, type_ext->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);
    }
}


DEFINE_HOOK(0x0042AACF, _BuildingClass_Unlimbo_Update_Factories_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    Update_Factories_Helper(this_ptr);
    return 0x0042AAEB;
}


DEFINE_HOOK(0x0042DFBE, _BuildingClass_Limbo_Update_Factories_Patch, 0)
{
    GET(BuildingClass*, this_ptr, EDI);
    Update_Factories_Helper(this_ptr);
    return 0x0042DFDA;
}


DEFINE_HOOK(0x0042FCF8, _BuildingClass_Captured_Update_Factories_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    GET_STACK(HouseClass*, oldowner, 0x60);

    if (this_ptr->Class->ToBuild != RTTI_NONE) {
        BuildingTypeClassExtension* type_ext = Extension::Fetch(this_ptr->Class);

        HouseClassExtension* old_house_ext = Extension::Fetch(oldowner);
        old_house_ext->Update_Factories(this_ptr->Class->ToBuild, type_ext->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);

        HouseClassExtension* new_house_ext = Extension::Fetch(oldowner);
        new_house_ext->Update_Factories(this_ptr->Class->ToBuild, type_ext->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);
    }

    return 0x0042FD28;
}


DEFINE_HOOK(0x00434C78, _BuildingClass_Read_INI_Update_Factories_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    Update_Factories_Helper(this_ptr);
    return 0x00434C94;
}


DEFINE_HOOK(0x00436855, _BuildingClass_Turn_On_Update_Factories_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    Update_Factories_Helper(this_ptr);
    return 0x0043686B;
}


DEFINE_HOOK(0x00436911, _BuildingClass_Turn_Off_Update_Factories_Patch, 0)
{
    GET(BuildingClass*, this_ptr, ESI);
    Update_Factories_Helper(this_ptr);
    return 0x0043692D;
}


/**
 *  This patch is part of adding an extra naval queue for the AI.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004C0F40, _HouseClass_Raise_Money_BuildNavalUnit_Patch, 0)
{
    GET(HouseClass*, this_ptr, ESI);
    GET(bool, needs_harvester, ECX);

    HouseClassExtension* house_ext = Extension::Fetch(this_ptr);

    // Stolen instructions
    this_ptr->BuildUnit = UNIT_NONE;
    this_ptr->BuildInfantry = INFANTRY_NONE;
    this_ptr->BuildAircraft = AIRCRAFT_NONE;
    this_ptr->BuildStructure = STRUCT_NONE;

    // Clear naval production target
    house_ext->BuildNavalUnit = UNIT_NONE;

    if (needs_harvester) {
        return 0x004C0F5F;
    } else {
        return 0x004C0F87;
    }
}


/**
 *  Reimplementation of part of HouseClass::AI related to production,
 *  patched for naval queues.
 *
 *  @author: ZivDero
 */
void HouseClassExt::_Production_Check()
{
    auto house_ext = Extension::Fetch(this);

    bool b = BuildUnit == UNIT_NONE && BuildInfantry == INFANTRY_NONE && BuildAircraft == AIRCRAFT_NONE && house_ext->BuildNavalUnit == UNIT_NONE;

    if (BuildUnit != UNIT_NONE && !UnitTypes[BuildUnit]->Who_Can_Build_Me(true, true, true, this)) {
        b = true;
    }
    if (BuildInfantry != INFANTRY_NONE && !InfantryTypes[BuildInfantry]->Who_Can_Build_Me(true, true, true, this)) {
        b = true;
    }
    if (BuildAircraft != AIRCRAFT_NONE && !AircraftTypes[BuildAircraft]->Who_Can_Build_Me(true, true, true, this)) {
        b = true;
    }
    if (house_ext->BuildNavalUnit != UNIT_NONE && !UnitTypes[house_ext->BuildNavalUnit]->Who_Can_Build_Me(true, true, true, this)) {
        b = true;
    }

    if (b) {
        AI_Building();
    }
}

DEFINE_HOOK(0x004BD0E5, _HouseClass_AI_BuildNavalUnit_Patch, 0)
{
    GET(HouseClassExt*, this_ptr, ESI);
    this_ptr->_Production_Check();
    return 0x004BD1A1;
}


/**
 *  Reimplementation of of HouseClass::AI_Has_Prerequisites
 *
 *  @author: ZivDero
 */
bool HouseClassExt::_AI_Has_Prerequisites(const TechnoTypeClass* type, DynamicVectorClass<const BuildingTypeClass*>& owned, int ownedcount) const
{
    for (int i = 0; i < type->Prerequisite.Count(); i++) {

        if (type->Prerequisite[i] >= STRUCT_FIRST) {

            BuildingTypeClass* btype = BuildingTypes[type->Prerequisite[i]];
            if (!Rule->BuildConst.Is_Present(btype)) {

                bool found = false;
                for (int j = 0; j < ownedcount; j++) {
                    if (owned[j] == btype) {
                        found = true;
                        break;
                    }
                }

                if (!found) {
                    return false;
                }
            }

        } else {

            if (type->Prerequisite[i] == -1) {
                continue;
            }

            PrerequisiteGroupType grouptype = PrerequisiteGroupClass::Decode(type->Prerequisite[i]);
            if (grouptype == PREREQ_GROUP_NONE) {
                return false;
            }

            PrerequisiteGroupClass* group = PrerequisiteGroups[grouptype];

            bool found = false;
            for (int j = 0; j < ownedcount; j++) {
                if (group->Prerequisites.Is_Present(owned[j]->HeapID)) {
                    found = true;
                    break;
                }
            }

            if (!found) {
                return false;
            }
        }
    }

    return true;
}


/**
 *  Fixes a bug where the local player could be considered "lost" (Do_Lose was called)
 *  when a multiplayer match ended with the last enemy getting defeated.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x004BF8BD, _HouseClass_MPlayer_Defeated_Flag_Win_Or_Lose, 0)
{
    // The match has ended due to player defeat because there is only one team left.
    // Consider the player as having won if they are not defeated, OR in case of multiplayer,
    // if they have any human allies left alive.
    // This allows the player to be considered a winner if their team wins in a team game,
    // even if the player itself is defeated.

    bool localplayerwon = !PlayerPtr->IsDefeated;

    if (!localplayerwon && Session.Type != GAME_SKIRMISH) {

        DEBUG_INFO("MPlayer_Defeated: Local player is defeated, looking for allies.\n");

        for (int i = 0; i < Houses.Count(); i++) {

            /*
            **	Get a pointer to this house
            */
            HouseClass* hptr = Houses[i];
            if (!hptr || hptr->IsDefeated || !hptr->IsHuman || hptr->Class->IsMultiplayPassive) continue;

            if (PlayerPtr->Is_Ally(hptr)) {
                localplayerwon = true;
                break;
            }
        }
    }

    if (localplayerwon) {
        DEBUG_INFO("MPlayer_Defeated: Flagging local player as victorious.\n");
        PlayerPtr->Flag_To_Win(false);
    } else {
        DEBUG_INFO("MPlayer_Defeated: Flagging local player as lost.\n");
        PlayerPtr->Flag_To_Lose(false);
    }

    return 0x004BF8E3;
}


/**
 *  Fixes an edge case bug where HouseClass::AI_Raise_Money can corrupt
 *  the house's Base Node vector by writing to the vector at index -1.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x004C0F87, _HouseClass_AI_Raise_Money_Fix_Memory_Corruption, 0)
{
    GET(HouseClass*, this_ptr, ESI);
    GET(StructType, buildingtype, EAX);
    int buildable_index;

    buildable_index = this_ptr->Base.Next_Buildable_Index(buildingtype);

    // Stolen bytes / code. Do not insert element to Base Nodes vector
    // if buildable index is 0.
    if (buildable_index == 0) {
        return 0x004C10BC;
    }

    // Bugfix: also do not insert element if buildable index is -1. (or below 0)
    if (buildable_index < 0) {
        return 0x004C10BC;
    }

    // Apply node index variable and also save it in eax,
    // original game code expects this
    R->Stack(0x1C, buildable_index);
    R->EAX(buildable_index);
    return 0x004C0F9F;
}

/**
 *  Patches HouseClass::Recalc_Radar_Availability to allow Free Radar to still function when the player is in low power.
 *  This requires 'FreeRadarOnLowPower=yes' to be set under [General].
 *  Note that Ion Storms still turns off the radar regardless of this flag.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004C958D, _HouseClass_Recalc_Radar_Availability_Free_Radar_Low_Power_Patch, 6)
{
    if (Scen->IsFreeRadar && RuleExtension->IsFreeRadarOnLowPower) {
        return 0x004C966A;
    }

    return 0;
}

/**
 *  Patches HouseClass::Make_Ally to take sight range bonuses into account when revealing the area around technos we that were allied with 
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004BDD4C, HouseClass_Make_Ally_Sight_Range_Patch, 0)
{
    GET(TechnoClass*, techno, ESI);
    GET(HouseClass*, this_ptr, EDI);

    auto techno_ext = Extension::Fetch(techno);

    int sight_range = techno_ext->Get_Sight_Range();
    Coord coord = techno->Center_Coord();

    Map.Sight_From(coord, sight_range, this_ptr);

    return 0x004BDD83;
}

/**
 *  Patches HouseClass::Updated_Spied_By to take sight range bonuses into account when revealing the area
 *  around technos that were revealed by spying a radar
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004C9937, HouseClass_Updated_Spied_By_Sight_Range_Patch, 0)
{
    GET(TechnoClass*, techno, ESI);
    GET(HouseClass*, this_ptr, EDI);

    auto techno_ext = Extension::Fetch(techno);

    int sight_range = techno_ext->Get_Sight_Range();
    Coord coord = techno->Center_Coord();

    Map.Sight_From(coord, sight_range, this_ptr);

    return 0x004C996B;
}

/**
 *  Main function for patching the hooks.
 */
void HouseClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    HouseClassExtension_Init();

    Patch_Jump(0x004C10E0, &HouseClassExt::_AI_Building);
    Patch_Jump(0x004C1650, &HouseClassExt::_AI_Unit);
    Patch_Jump(0x004C0630, &HouseClassExt::_Expert_AI);

    Patch_Jump(0x004BAC2C, 0x004BAC39); // Patch a jump in the constructor to always allocate unit trackers

    Patch_Jump(0x004C23B0, &HouseClassExt::_Active_Remove);
    Patch_Jump(0x004C2450, &HouseClassExt::_Active_Add);

    Patch_Call(0x0042D460, &HouseClassExt::_Find_Build_Location);
    Patch_Call(0x0042D53C, &HouseClassExt::_Find_Build_Location);
    Patch_Call(0x004C8104, &HouseClassExt::_Find_Build_Location);

    Patch_Jump(0x004C5920, &HouseClassExt::_AI_Has_Prerequisites);

    Patch_Jump(0x004C2CA0, &HouseClassExt::_Fetch_Factory);
    Patch_Jump(0x004C2D20, &HouseClassExt::_Set_Factory);
    Patch_Jump(0x004C2330, &HouseClassExt::_Factory_Counter);
    Patch_Jump(0x004C2DB0, &HouseClassExt::_Factory_Count);
    Patch_Jump(0x004BE5D0, &HouseClassExt::_Suspend_Production);
    Patch_Jump(0x004BE200, &HouseClassExt::_Begin_Production);
    Patch_Jump(0x004BE6A0, &HouseClassExt::_Abandon_Production);
    Patch_Jump(0x004BEA10, &HouseClassExt::_Place_Object);
    Patch_Jump(0x004BF180, &HouseClassExt::_Suggest_New_Object);
    Patch_Jump(0x004BD590, &HouseClassExt::_Harvested);
}
