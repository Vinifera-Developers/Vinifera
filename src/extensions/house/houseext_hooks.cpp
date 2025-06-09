/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          HOUSEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended HouseClass.
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
#include "houseext_hooks.h"
#include "houseext_init.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "building.h"
#include "house.h"
#include "housetype.h"
#include "technotype.h"
#include "super.h"
#include "factory.h"
#include "techno.h"
#include "unittype.h"
#include "unittypeext.h"
#include "mouse.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "buildingtypeext.h"
#include "extension_globals.h"
#include "sidebarext.h"
#include "rules.h"
#include "session.h"
#include "ccini.h"
#include "sideext.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "houseext.h"
#include "msgbox.h"
#include "rulesext.h"
#include "tibsun_functions.h"


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
        Base.Nodes.Delete(Base.Nodes.ID(node));
        AI_Build_Wall();
        return 1;
    }

    /**
     *  Build some defenses.
     */
    if (node->Type == BASE_DEFENSE || BuildingTypes[node->Type] == Rule->WallTower && node->Where == Cell(0, 0)) {

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
    if (b->PowersUpToLevel == -1 && node->Where != Cell(0, 0) && b->PowersUpBuilding[0]) {

        BuildingClass* existing_building = Map[node->Where].Cell_Building();
        BuildingTypeClass* node_building = BuildingTypes[BuildingTypeClass::From_Name(b->PowersUpBuilding)];

        if (existing_building == nullptr) {
            node->Where = Cell(0, 0);
        }
        else if (existing_building->Class != node_building) {
            node->Where = Cell(0, 0);
        }
        else if (existing_building->Class->PowersUpToLevel == -1 && existing_building->UpgradeLevel >= existing_building->Class->Upgrades || existing_building->Class->PowersUpToLevel > 0 && existing_building->UpgradeLevel > 0) {
            node->Where = Cell(0, 0);
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
        AI_Super_Weapon_Handler();
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
 *  Patch for InstantSuperRechargeCommandClass
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_HouseClass_Super_Weapon_Handler_InstantRecharge_Patch)
{
    GET_REGISTER_STATIC(HouseClass *, this_ptr, edi);
    GET_REGISTER_STATIC(SuperClass *, special, esi);
    static bool is_player;

    is_player = false;
    if (this_ptr == PlayerPtr) {
        is_player = true;
    }

    if (Vinifera_DeveloperMode) {

        if (!special->IsReady) {

            /**
             *  If AIInstantBuild is toggled on, make sure this is a non-human AI house.
             */
            if (Vinifera_Developer_AIInstantSuperRecharge
                && !this_ptr->Is_Human_Player() && this_ptr != PlayerPtr) {

                special->Forced_Charge(is_player);

            /**
             *  If InstantBuild is toggled on, make sure the local player is a human house.
             */
            } else if (Vinifera_Developer_InstantSuperRecharge
                && this_ptr->Is_Human_Player() && this_ptr == PlayerPtr) {
                
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
        goto continue_function;
    }

add_to_sidebar:
    JMP(0x004BD320);

continue_function:
    JMP(0x004BD332);
}


/**
 *  Patch for BuildCheatCommandClass
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_HouseClass_Can_Build_BuildCheat_Patch)
{
    GET_REGISTER_STATIC(HouseClass *, this_ptr, ebp);
    GET_REGISTER_STATIC(int, vector_count, ecx);
    GET_STACK_STATIC(TechnoTypeClass *, objecttype, esp, 0x30);

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
            if (((1 << this_ptr->Class->HeapID) & objecttype->Get_Ownable()) != 0) {
                //DEBUG_INFO("Forcing \"%s\" available.\n", objecttype->IniName);
                goto return_true;
            }
        }
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    _asm { xor eax, eax }
    _asm { mov [esp+0x34], eax }

    _asm { mov ecx, vector_count }
    _asm { test ecx, ecx }

    JMP_REG(ecx, 0x004BBD2E); // Need to use ECX as EAX is used later on.

return_true:
    JMP(0x004BBD17);
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
            count += factory->House->UQuantity.Count_Of((UnitType)(unittypeext->TransformsInto->Fetch_Heap_ID()));
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
DECLARE_PATCH(_HouseClass_ShouldDisableCameo_BuildLimit_Fix)
{
    GET_REGISTER_STATIC(FactoryClass*, factory, ecx);
    GET_REGISTER_STATIC(TechnoTypeClass*, technotype, esi);
    static int queuedcount;

    queuedcount = _HouseClass_ShouldDisableCameo_Get_Queued_Count(factory, technotype);

    _asm { mov eax, [queuedcount] }
    JMP_REG(ecx, 0x004CB77D);
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
DECLARE_PATCH(_HouseClass_Can_Build_BuildLimit_Handle_Vehicle_Transform)
{
    GET_REGISTER_STATIC(UnitTypeClass*, unittype, edi);
    GET_REGISTER_STATIC(HouseClass*, house, ebp);
    static UnitTypeClassExtension* unittypeext;
    static int objectcount;

    unittypeext = Extension::Fetch(unittype);

    /**
     *  Stolen bytes / code.
     */
    objectcount = house->UQuantity.Count_Of((UnitType)unittype->Fetch_Heap_ID());

    /**
     *  Check whether this unit can deploy into a building.
     *  If it can, increment the object count by the number of buildings.
     */
    if (unittype->DeploysInto != nullptr) {
        objectcount += house->BQuantity.Count_Of((StructType)unittype->DeploysInto->Fetch_Heap_ID());
    }
    else if (unittypeext->TransformsInto != nullptr) {

        /**
         *  This unit can transform into another unit, increment the object count
         *  by the number of transformed units.
         */
        objectcount += house->UQuantity.Count_Of((UnitType)(unittypeext->TransformsInto->Fetch_Heap_ID()));
    }

    _asm { mov esi, objectcount }

continue_function:
    JMP(0x004BC1B9);
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
DECLARE_PATCH(_HouseClass_Enable_SWs_Check_For_Building_Power)
{
    GET_REGISTER_STATIC(int, quiet, eax);
    GET_REGISTER_STATIC(BuildingClass*, building, esi);

    if (!building->IsPowerOn)
    {
        /**
         *  Enable the superweapon in suspended mode.
         */
        _asm { mov eax, 1 }
    }
    else
    {
        /**
         *  Enable the superweapon in non-suspended mode.
         */
        _asm {xor eax, eax }
    }

    /**
     *  Stolen bytes/code.
     */
    _asm { mov  esi, [PlayerPtr] }

    /**
     *  Continue the SW enablement process.
     */
    JMP_REG(ecx, 0x004CB6C7);
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
     *  Find the type class extension instance.
     */
    BuildingTypeClassExtension* buildingtypeext = Extension::Fetch(btype);
    if (buildingtypeext && buildingtypeext->IsNaval) {

        DEV_DEBUG_INFO("Find_Build_Location(%s): Searching for Naval Yard \"%s\" build location...\n", Name(), btype->Name());

        Cell cell(0, 0);

        /**
         *  Get the cell footprint for the Naval Yard, then add a safety margin of 2.
         */
        int area_w = btype->Width() + 2;
        int area_h = btype->Height() + 2;

        /**
         *  find a nearby location from the center of the base that fits our naval yard.
         */
        Cell found_cell = Map.Nearby_Location(Coord_Cell(Center), SPEED_FLOAT, -1, MZONE_NORMAL, false, Point2D(area_w, area_h));
        if (found_cell != CELL_NONE) {

            DEV_DEBUG_INFO("Find_Build_Location(%s): Found possible Naval Yard location at %d,%d...\n", Name(), found_cell.X, found_cell.Y);

            /**
             *  Iterate over all owned construction yards and find the first that is closest to our cell.
             */
            for (int i = 0; i < ConstructionYards.Count(); ++i) {
                BuildingClass* conyard = ConstructionYards[i];
                if (conyard) {

                    Coordinate conyard_coord = conyard->Center_Coord();
                    Coordinate found_coord = Map[found_cell].Center_Coord();

                    /**
                     *  Is this location close enough to the construction yard for us to use?
                     */
                    if (Distance(conyard_coord, found_coord) <= Cell_To_Lepton(RuleExtension->AINavalYardAdjacency)) {
                        DEV_DEBUG_INFO("Find_Build_Location(%s): Using location %d,%d for Naval Yard.\n", Name(), found_cell.X, found_cell.Y);
                        cell = found_cell;
                        break;
                    }
                }
            }
        }

        if (cell == CELL_NONE) {
            DEV_DEBUG_WARNING("Find_Build_Location(%s): Failed to find suitable location for \"%s\"!\n", Name(), btype->Name());
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
DECLARE_PATCH(_Can_Build_Required_Forbidden_Houses_Patch)
{
    GET_REGISTER_STATIC(TechnoTypeClass*, techno_type, edi);
    GET_REGISTER_STATIC(HouseClassExt*, this_ptr, ebp);
    static bool can_build;

    can_build = this_ptr->_Can_Build_Required_Forbidden_Houses(techno_type);

    if (!can_build)
    {
        //return false;
        JMP(0x004BBC9A);
    }

    // Stolen bytes
    _asm
    {
        mov eax, [esi+0x14]
        mov edx, [edi+0x32C]
    }

    // Continue Can_Build
    JMP_REG(ecx, 0x004BBC7D);
}


/**
 *  Allow to skip the check for the MCV's ActLike.
 *
 *  Author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Can_Build_Multi_MCV_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, building, esi);

    if (RuleExtension->IsMultiMCV) {
        JMP(0x004BC102);
    }

    static HousesType act_like;
    act_like = building->ActLike;

    _asm mov ecx, act_like
    JMP(0x004BC0BD);
}


/**
 *  Handy macro for the functions below.
 */
#define WARN_AND_EXIT(funcname) { \
    DEBUG_FATAL("The legacy version of " STRINGIZE(funcname) " has been called! If you see this, please notify the developers. The game will now exit.\n"); \
    DEBUG_FATAL("Return address: %p\n", _ReturnAddress()); \
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
DECLARE_PATCH(_HouseClass_Exhausted_Build_Limit_Fetch_Factory_Patch)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, ebx);
    GET_REGISTER_STATIC(TechnoTypeClass const*, ttype, esi);

    static FactoryClass* factory;
    factory = Extension::Fetch(this_ptr)->Fetch_Factory(ttype->RTTI, TechnoTypeClassExtension::Get_Production_Flags(ttype));

    _asm mov ecx, factory
    JMP(0x004CB773);
}


void Update_Factories_Helper(BuildingClass* building)
{
    if (building->Class->ToBuild != RTTI_NONE) {
        BuildingTypeClassExtension* type_ext = Extension::Fetch(building->Class);
        HouseClassExtension* house_ext = Extension::Fetch(building->House);
        house_ext->Update_Factories(building->Class->ToBuild, type_ext->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);
    }
}


DECLARE_PATCH(_BuildingClass_Unlimbo_Update_Factories_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    Update_Factories_Helper(this_ptr);
    JMP(0x0042AAEB);
}


DECLARE_PATCH(_BuildingClass_Limbo_Update_Factories_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, edi);
    Update_Factories_Helper(this_ptr);
    JMP(0x0042DFDA);
}


DECLARE_PATCH(_BuildingClass_Captured_Update_Factories_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    GET_STACK_STATIC(HouseClass*, newowner, esp, 0x18);
    GET_STACK_STATIC(HouseClass*, oldowner, esp, 0x60);

    static BuildingTypeClassExtension* type_ext;
    static HouseClassExtension* old_house_ext;
    static HouseClassExtension* new_house_ext;

    if (this_ptr->Class->ToBuild != RTTI_NONE) {
        type_ext = Extension::Fetch(this_ptr->Class);

        old_house_ext = Extension::Fetch(oldowner);
        old_house_ext->Update_Factories(this_ptr->Class->ToBuild, type_ext->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);

        new_house_ext = Extension::Fetch(oldowner);
        new_house_ext->Update_Factories(this_ptr->Class->ToBuild, type_ext->IsNaval ? PRODFLAG_NAVAL : PRODFLAG_NONE);
    }

    JMP(0x0042FD28);
}


DECLARE_PATCH(_BuildingClass_Read_INI_Update_Factories_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    Update_Factories_Helper(this_ptr);
    JMP(0x00434C94);
}


DECLARE_PATCH(_BuildingClass_Turn_On_Update_Factories_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    Update_Factories_Helper(this_ptr);
    JMP(0x0043686B);
}


DECLARE_PATCH(_BuildingClass_Turn_Off_Update_Factories_Patch)
{
    GET_REGISTER_STATIC(BuildingClass*, this_ptr, esi);
    Update_Factories_Helper(this_ptr);
    JMP(0x0043692D);
}


/**
 *  This patch is part of adding an extra naval queue for the AI.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Raise_Money_BuildNavalUnit_Patch)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, esi);
    GET_REGISTER_STATIC(bool, needs_harvester, cl);
    static HouseClassExtension* house_ext;

    house_ext = Extension::Fetch(this_ptr);

    // Stolen instructions
    this_ptr->BuildUnit = UNIT_NONE;
    this_ptr->BuildInfantry = INFANTRY_NONE;
    this_ptr->BuildAircraft = AIRCRAFT_NONE;
    this_ptr->BuildStructure = STRUCT_NONE;

    // Clear naval production target
    house_ext->BuildNavalUnit = UNIT_NONE;

    if (needs_harvester) {
        JMP(0x004C0F5F);
    } else {
        JMP(0x004C0F87);
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

DECLARE_PATCH(_HouseClass_AI_BuildNavalUnit_Patch)
{
    GET_REGISTER_STATIC(HouseClassExt*, this_ptr, esi);
    this_ptr->_Production_Check();
    JMP(0x004BD1A1);
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

    Patch_Jump(0x004BBD26, &_HouseClass_Can_Build_BuildCheat_Patch);
    Patch_Jump(0x004BD30B, &_HouseClass_Super_Weapon_Handler_InstantRecharge_Patch);

    Patch_Jump(0x004CB777, &_HouseClass_ShouldDisableCameo_BuildLimit_Fix);
    Patch_Jump(0x004BC187, &_HouseClass_Can_Build_BuildLimit_Handle_Vehicle_Transform);
    Patch_Jump(0x004CB6C1, &_HouseClass_Enable_SWs_Check_For_Building_Power);

    Patch_Jump(0x004C10E0, &HouseClassExt::_AI_Building);
    Patch_Jump(0x004C1650, &HouseClassExt::_AI_Unit);
    Patch_Jump(0x004C0630, &HouseClassExt::_Expert_AI);
    Patch_Jump(0x004BBC74, &_Can_Build_Required_Forbidden_Houses_Patch);

    Patch_Jump(0x004BAC2C, 0x004BAC39); // Patch a jump in the constructor to always allocate unit trackers
    Patch_Jump(0x004BC0B7, &_HouseClass_Can_Build_Multi_MCV_Patch);

    Patch_Jump(0x004CB73D, &_HouseClass_Exhausted_Build_Limit_Fetch_Factory_Patch);
    Patch_Jump(0x0042AACF, &_BuildingClass_Unlimbo_Update_Factories_Patch);
    Patch_Jump(0x0042DFBE, &_BuildingClass_Limbo_Update_Factories_Patch);
    Patch_Jump(0x0042FCF8, &_BuildingClass_Captured_Update_Factories_Patch);
    Patch_Jump(0x00434C78, &_BuildingClass_Read_INI_Update_Factories_Patch);
    Patch_Jump(0x00436855, &_BuildingClass_Turn_On_Update_Factories_Patch);
    Patch_Jump(0x00436911, &_BuildingClass_Turn_Off_Update_Factories_Patch);
    Patch_Jump(0x004C0F40, &_HouseClass_Raise_Money_BuildNavalUnit_Patch);
    Patch_Jump(0x004BD0E5, &_HouseClass_AI_BuildNavalUnit_Patch);

    Patch_Jump(0x004C23B0, &HouseClassExt::_Active_Remove);
    Patch_Jump(0x004C2450, &HouseClassExt::_Active_Add);

    Patch_Call(0x0042D460, &HouseClassExt::_Find_Build_Location);
    Patch_Call(0x0042D53C, &HouseClassExt::_Find_Build_Location);
    Patch_Call(0x004C8104, &HouseClassExt::_Find_Build_Location);

    Patch_Jump(0x004C2CA0, &HouseClassExt::_Fetch_Factory);
    Patch_Jump(0x004C2D20, &HouseClassExt::_Set_Factory);
    Patch_Jump(0x004C2330, &HouseClassExt::_Factory_Counter);
    Patch_Jump(0x004C2DB0, &HouseClassExt::_Factory_Count);
    Patch_Jump(0x004BE5D0, &HouseClassExt::_Suspend_Production);
    Patch_Jump(0x004BE200, &HouseClassExt::_Begin_Production);
    Patch_Jump(0x004BE6A0, &HouseClassExt::_Abandon_Production);
    Patch_Jump(0x004BEA10, &HouseClassExt::_Place_Object);
    Patch_Jump(0x004BF180, &HouseClassExt::_Suggest_New_Object);
}
