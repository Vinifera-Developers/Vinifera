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
#include "buildingext.h"
#include "buildingtypeext.h"
#include "ccini.h"
#include "debughandler.h"
#include "extension_globals.h"
#include "factory.h"
#include "fatal.h"
#include "fetchres.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "house.h"
#include "houseext.h"
#include "houseext_init.h"
#include "housetype.h"
#include "language.h"
#include "logic.h"
#include "mouse.h"
#include "msgbox.h"
#include "prerequisitegroup.h"
#include "rules.h"
#include "rulesext.h"
#include "scenarioext.h"
#include "session.h"
#include "sessionext.h"
#include "sideext.h"
#include "spawner.h"
#include "super.h"
#include "syringe.h"
#include "techno.h"
#include "technoext.h"
#include "technotype.h"
#include "tiberium.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "unit.h"
#include "unittype.h"
#include "unittypeext.h"
#include "vinifera_globals.h"
#include "vox.h"


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
    void _Active_Remove(TechnoClass const* techno);
    void _Active_Add(TechnoClass const* techno);
    Cell _Find_Build_Location(BuildingTypeClass* btype, int(__fastcall* callback)(int, Cell&, int, int), int a3 = -1);
    void _Production_Check();
    bool _AI_Has_Prerequisites(const TechnoTypeClass* type, DynamicVectorClass<const BuildingTypeClass*>& owned, int ownedcount) const;
    void _Harvested(int tiberium, TiberiumType slot);
    bool _Can_Make_Money();
    UrgencyType _Check_Raise_Money();
    void _MPlayer_Defeated();
    void _Make_Ally(HouseClass* house);

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
    ExtDiffType _Assign_Handicap(ExtDiffType handicap);
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

    if (Session.Type != GAME_NORMAL && !ScenExtension->IsUseMPAIBaseNodes && b->Drain + Drain > Power - PowerSurplus && b != Rule->BuildConst[0] && b->Drain > 0) {

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
     *  If there is no enemy assigned to this house, then assign one now. The
     *  enemy that is closest is picked. However, don't pick an enemy if the
     *  base has not been established yet.
     */
    if (ExpertAITimer == 0) {
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
    } else {
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

    if (Session.Type != GAME_NORMAL && !ScenExtension->IsUseMPAIBaseNodes) {

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
 *  #issue-177
 *
 *  Checks if the AI house has the capability to make money. Adjusted to
 *  use the entire Build* and HarvesterUnit lists.
 *
 *  @author: ZivDero
 */
bool HouseClassExt::_Can_Make_Money()
{
    const int credits = Available_Money();
    const int ref_cost = Get_First_Ownable(Rule->BuildRefinery)->Cost_Of(this);
    const int harv_cost = Get_First_Ownable(Rule->HarvesterUnit)->Cost_Of(this);

    const int ref_count = Count_Owned(Rule->BuildRefinery);
    const int harv_count = Count_Owned(Rule->HarvesterUnit);

    /**
     *  If we don't have any refineries, building one is a priority.
     */
    if (ref_count == 0)
        return credits > ref_cost;

    /**
     *  If we have a refinery and a harvester, all's well.
     */
    if (harv_count)
        return true;

    const bool has_factory = Count_Owned(Rule->BuildWeapons) > 0;
    const int factory_cost = Get_First_Ownable(Rule->BuildWeapons)->Cost_Of(this);

    /**
     *  If we have a refinery, but not a harvester, see if
     *  we can build one if we have a factory.
     */
    if (has_factory && credits >= harv_cost)
        return true;

    /**
     *  And if we don't have a factory, see if we can build one.
     */
    if (credits >= harv_cost + factory_cost)
        return true;

    /**
     *  Worst case, see if we can build a new refinery to get a free harvester.
     */
    if (credits >= ref_cost)
        return true;

    return false;
}


/**
 *  #issue-177
 *
 *  Checks if the AI needs to urgently raise more money.
 *  Adjusted to use the entire Build* and HarvesterUnit lists.
 *
 *  @author: ZivDero
 */
UrgencyType HouseClassExt::_Check_Raise_Money()
{
    UrgencyType urgency = URGENCY_NONE;

    /**
     *  Human players don't need AI to raise money for them.
     */
    const bool human = Session.Type == GAME_NORMAL ? Is_Player_Control() : IsHuman;
    if (human)
        return urgency;

    /**
     *  If we can afford to have a harvester and a refinery, all is well.
     */
    if (Can_Make_Money())
        return urgency;

    /**
     *  See if we have a refinery.
     */
    if (Count_Owned(Rule->BuildRefinery))
    {
        /**
         *  Iterate all the buildings and check if we have a refinery under construction.
         *  If so, we don't need raise money, since we'll get a free harvester.
         */
        for (int i = 0; i < Buildings.Count(); i++)
        {
            BuildingClass* building = Buildings[i];
            if (building->House == this)
            {
                if (Rule->BuildRefinery.Is_Present(building->Class) && building->Get_Mission() == MISSION_CONSTRUCTION)
                    return urgency;

                urgency = URGENCY_NONE;
            }
        }

        /**
         *  Check if what we're currently building is a harvester.
         *  If it's not and we don't have enough money to build one,
         *  we've got minor issues.
         */
        const UnitTypeClass* harvester = Get_First_Ownable(Rule->HarvesterUnit);
        if (BuildUnit != harvester->HeapID)
        {
            if (Available_Money() < harvester->Cost_Of(this))
                urgency++;

            return urgency;
        }

        /**
         *  Check all the factories and find which is building our harvester.
         *  If we haven't got enough money to complete contruction, we've got issues.
         */
        for (int i = 0; i < Factories.Count(); i++)
        {
            const FactoryClass* factory = Factories[i];
            if (factory && factory->House == this)
            {
                ObjectClass* obj = factory->Get_Object();
                if (obj && obj->What_Am_I() == RTTI_UNIT
                    && Rule->HarvesterUnit.Is_Present((UnitTypeClass*)(obj->TClass)))
                {
                    if (Available_Money() < factory->Balance)
                        urgency++;

                    return urgency;

                }
            }
        }
    }
    else
    {
        /**
         *  Check if what we're currently building is a refinery.
         *  If it's not and we don't have enough money to build one,
         *  we've got minor issues.
         */
        const BuildingTypeClass* refinery = Get_First_Ownable(Rule->BuildRefinery);
        if (BuildStructure != refinery->HeapID)
        {
            if (Available_Money() < refinery->Cost_Of(this))
                urgency++;

            return urgency;
        }

        /**
         *  Check all the factories and find which is building our refinery.
         *  If we haven't got enough money to complete contruction, we've got issues.
         */
        for (int i = 0; i < Factories.Count(); i++)
        {
            const FactoryClass* factory = Factories[i];
            if (factory && factory->House == this)
            {
                ObjectClass* obj = factory->Get_Object();
                if (obj && obj->What_Am_I() == RTTI_BUILDING
                    && Rule->BuildRefinery.Is_Present((BuildingTypeClass*)(obj->TClass)))
                {
                    if (Available_Money() < factory->Balance)
                        urgency++;

                    return urgency;
                }
            }
        }
    }

    /**
     *  Something weird has happened, it's surely not good.
     */
    urgency++;
    return urgency;
}


/**
 *  A house is defeated in multiplayer.
 *
 *  @author: 05/25/1995 BRR - Created
 *           29/10/2024 ZivDero - Adjustments for Tiberian Sun
 *           19/07/2026 Rampastring - Correct local/team win and loss flagging
 */
void HouseClassExt::_MPlayer_Defeated()
{
    char txt[80];

    /**
     *  Set the defeat flag for this house
     */
    IsDefeated = true;

    /**
     *  If this is a computer controlled house, then all computer controlled
     *  houses become paranoid.
     */
    if (IQ == Rule->MaxIQ && !Is_Human_Player() && Rule->IsComputerParanoid) {
        Computer_Paranoid();
    }

    /**
     *  Remove this house's flag & flag home cell
     */
    if (Special.IsCaptureTheFlag) {
        if (FlagLocation) {
            Flag_Remove(FlagLocation, true);
        } else {
            if (FlagHome != CELL_NONE) {
                Flag_Remove(&Map[FlagHome], true);
            }
        }
    }

    /**
     *  If harvester truce is on, remove all of this player's harvesters.
     */
    if (Session.Type != GAME_NORMAL && Scen->Special.IsHarvesterImmune) {
        for (int i = 0; i < Units.Count(); i++) {
            if (Units[i]->House == this && Units[i]->IsActive) {
                Units[i]->Delete_Me();
            }
        }
    }

    /**
     *  If this is me:
     *  - Add my defeat message
     */
    if (PlayerPtr == this) {
        if (!Extension::Fetch(PlayerPtr)->IsObserver) {

            /**
             *  Pop up a message showing that I was defeated
             */
            std::snprintf(txt, std::size(txt), Fetch_String(TXT_PLAYER_DEFEATED), IniName.c_str());
            Session.Messages.Add_Message(nullptr, 0, txt, static_cast<ColorSchemeType>(Session.ColorIdx), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, Rule->MessageDelay * TICKS_PER_MINUTE);
            Speak(VOX_YOU_HAVE_LOST);
        }

        Map.Flag_To_Redraw();
        DEBUG_INFO("MPlayer_Defeated() - Player {} has been defeated\n", IniName);

    } else {

        /**
         *  If it wasn't me, find out who was defeated
         */
        if (!Class->IsMultiplayPassive) {
            if (!Extension::Fetch(PlayerPtr)->IsObserver) {
                std::snprintf(txt, std::size(txt), Fetch_String(TXT_PLAYER_DEFEATED), IniName.c_str());
                Session.Messages.Add_Message(nullptr, 0, txt, Scheme, TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, Rule->MessageDelay * TICKS_PER_MINUTE);
                Speak(VOX_PLAYER_DEFEATED);
            }

            Map.Flag_To_Redraw();
            DEBUG_INFO("MPlayer_Defeated() - Opponent {} has been defeated\n", IniName);
        }
    }

    /**
     *  If the local player is been defeated, check if they should be given OBIWAN mode.
     */
    if (PlayerPtr->IsDefeated && !Extension::Fetch(PlayerPtr)->IsObserver && !Session.ObiWan) {

        /**
         *  With the spawner active, if Coach mode is enabled, players don't get vision.
         */
        bool obiwan = true;
        if (SessionExtension->ExtOptions.IsCoachMode) {
            obiwan = false;
        }

        /**
         *  Now check if the player has any player allies remaining.
         */
        for (int i = 0; i < Houses.Count(); i++) {
            HouseClass* hptr = Houses[i];
            if (!hptr->IsDefeated && !hptr->Class->IsMultiplayPassive && (hptr->Is_Ally(PlayerPtr) || PlayerPtr->Is_Ally(hptr))) {
                obiwan = false;
                break;
            }
        }

        /**
         *  - Set MPlayerObiWan, so I can only send messages to all players, and
         *    not just one (so I can't be obnoxiously omnipotent)
         *  - Reveal the map
         */
        if (obiwan) {
            Session.ObiWan = true;
            Map.Reveal_The_Map();
            PlayerPtr->RecalcRadar = true;
            HiddenSurface->Fill(0);
            Map.Flag_To_Redraw();
            DEBUG_INFO("MPlayer_Defeated() - Player {} has no allies left (OBIWAN MODE)\n", IniName);
        }
    }

    /**
     *  Find out how many players are left alive.
     */
    int num_alive = 0;
    int num_humans = 0;
    for (int i = 0; i < Houses.Count(); i++) {
        HouseClass* hptr = Houses[i];
        if (hptr && !hptr->IsDefeated && !hptr->Class->IsMultiplayPassive) {
            if (hptr->Is_Human_Player()) {
                num_humans++;
            }
            num_alive++;
        }
    }
    DEBUG_INFO("MPlayer_Defeated() - Alive = {}, Humans = {}\n", num_alive, num_humans);

    /**
     *  If all the houses left alive are allied with each other, then in reality
     *  there's only one player left:
     */
    bool all_allies = true;
    for (int i = 0; i < Houses.Count(); i++) {

        /**
         *  Get a pointer to this house
         */
        HouseClass* hptr = Houses[i];
        if (!hptr || hptr->IsDefeated || hptr->Class->IsMultiplayPassive) continue;

        /**
         *  Loop through all houses; if there's one left alive that this house
         *  isn't allied with, then all_allies will be false
         */
        for (int j = 0; j < Houses.Count(); j++) {
            HouseClass* hptr2 = Houses[j];
            if (!hptr2) {
                continue;
            }

            if (!hptr2->IsDefeated && !hptr2->Class->IsMultiplayPassive && (!hptr->Is_Ally(hptr2) || !hptr2->Is_Ally(hptr))) {
                all_allies = false;
                break;
            }
        }
        if (!all_allies) {
            break;
        }
    }

    /**
     *  If all houses left are allies, set 'num_alive' to 1; game over.
     */
    if (all_allies) {
        Session.SawCompletion = true;
        DEBUG_INFO("Saw game completion due to player defeat\n");
        DEBUG_INFO("MPlayer_Defeated() - All remaining players are allied\n");
        num_alive = 1;
    }

    /**
     *  If there's only one human player left or no humans left, the game is over.
     */
    if (!Extension::Fetch(this)->IsObserver) {
        if (num_alive == 1 || (num_humans == 0 && !SessionExtension->ExtOptions.IsContinueWithoutHumans && (!Session.Singleplayer_Game() || !Extension::Fetch(PlayerPtr)->IsObserver))) {
            IsToDie = false;

            /**
             *  Consider the local player victorious if they are still alive, or if
             *  they have a surviving human ally in a multiplayer team game.
             */
            bool localplayerwon = !PlayerPtr->IsDefeated;

            if (!localplayerwon && Session.Type != GAME_SKIRMISH) {
                DEBUG_INFO("MPlayer_Defeated: Local player is defeated, looking for allies.\n");

                for (int i = 0; i < Houses.Count(); i++) {
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
        }
    }
}


/**
 *  Make the specified house an ally.
 *
 *  @author: 05/08/1995 JLB - Created
 *           29/10/2024 ZivDero - Adjustments for Tiberian Sun
 *           11/07/2026 Rampastring - Use Shush's extended sight range logic when revealing allied objects,
 *                                    don't reveal MultiplayPassive house objects in multiplayer
 */
void HouseClassExt::_Make_Ally(HouseClass* house)
{
    if (Is_Allowed_To_Ally(house)) {

        Allies |= (1L << house->HeapID);

        /**
         *  Don't consider the newfound ally to be an enemy -- of course.
         */
        Recalc_Threat_Regions();
        Clear_Anger(house);

        if (Enemy == house->HeapID) {
            Enemy = HOUSE_NONE;
        }

        if (ScenarioInit) {
            Control.Allies |= (1L << house->HeapID);
        }

        if (Session.Type != GAME_NORMAL || !ScenarioInit) {

            if (!ScenarioInit) {

                /**
                 *  An alliance with another human player will cause the computer
                 *  players (if present) to become paranoid.
                 */
                if (Is_Human_Player() && Rule->IsComputerParanoid && !house->Class->IsMultiplayPassive) {
                    Computer_Paranoid();
                }

                /**
                 *  Sweep through all techno objects and perform a cheeseball tarcom clear to ensure
                 *  that fighting will most likely stop when the cease fire begins.
                 */
                for (int index = 0; index < Logic.Count(); index++) {
                    ObjectClass* object = Logic[index];

                    if (object != nullptr && object->Is_Techno() && !object->IsInLimbo && object->Owner() == HeapID) {
                        TargetClass target = static_cast<TechnoClass*>(object)->TarCom;
                        if (target.Is_Valid() && target.As_Techno() != nullptr) {
                            if (Is_Ally(target.As_Techno())) {
                                static_cast<TechnoClass*>(object)->Assign_Target(nullptr);
                            }
                        }
                    }
                }

                if (Is_Human_Player() && Session.Type != GAME_NORMAL && !house->Class->IsMultiplayPassive) {

                    char buffer[80];
                    std::snprintf(buffer, std::size(buffer), Fetch_String(TXT_HAS_ALLIED), IniName, house->IniName);
                    Session.Messages.Add_Message(nullptr, 0, buffer, Scheme, TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, TICKS_PER_MINUTE * Rule->MessageDelay);

                    if (Is_Player_Control()) {
                        Speak(VOX_ALLIANCE_FORMED);
                    }
                }
            }

            /**
             *  Cause all technos to be revealed to the house that has been
             *  allied with.
             */
            if (Rule->IsAllyReveal && house == PlayerPtr) {
                for (int index = 0; index < Technos.Count(); index++) {
                    TechnoClass const* t = Technos[index];

                    /**
                     *  If in multiplayer, don't reveal objects owned by MultiplayPassive houses.
                     *  This matches the behaviour of TechnoClass::Look.
                     */
                    if (!t->IsInLimbo && t->House == this && (!Class->IsMultiplayPassive || Session.Type == GAME_NORMAL)) {
                        int sight_range = Extension::Fetch(t)->Get_Sight_Range();

                        Map.Sight_From(t->Center_Coord(), sight_range, PlayerPtr);
                    }
                }
            }

            Map.Flag_To_Redraw();
        }
    }
}


/**
 *  #issue-177
 * 
 *  Allow the game to check BaseUnit for all pertinent entries for "Short Game".
 * 
 *  #NOTE: The code before this patch already checks if the house has
 *         any buildings first.
 * 
 *  @author: CCHyper, ZivDero
 */
DEFINE_HOOK(0x004BCEE7, _HouseClass_AI_Short_Game_BaseUnit_Patch, 0)
{
    GET(HouseClass *, this_ptr, ESI);

    /**
     *  Count all MCVs we own to see if the player should explode.
     */
    const int count = this_ptr->Count_Owned(RuleExtension->BaseUnit);

    if (count) {
        return 0x004BCF6E;
    }

    /**
     *  Blows up the house, marking the house as defeated.
     */
    return 0x004BCF60;
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

    bool can_build = Extension::Fetch(this_ptr)->Required_Forbidden_Houses_Check(techno_type);

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
 *  #issue-177
 *
 *  Patches the check for if a house owns a Construction Yard to check the entire BuildConst list.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004BCD5D, _HouseClass_AI_BuildConst_Patch, 0)
{
    GET(HouseClass*, this_ptr, ESI);

    if (this_ptr->Count_Owned(Rule->BuildConst) > 0) {
        return 0x004BCD85;
    }

    return 0x004BCE0B;
}


/**
 *  #issue-177
 *
 *  Patches the check for if a house owns a harvester to check the entire HarvesterUnit list.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004BCF3A, _HouseClass_AI_Count_HarvesterUnit_Patch, 0)
{
    GET(HouseClass*, this_ptr, ESI);
    const int harv_count = this_ptr->Count_Owned(Rule->HarvesterUnit);

    R->EAX(harv_count);
    return 0x004BCF5A;
}


/**
 *  #issue-177
 *
 *  Patches the check for if a house is building a harvester to check the entire HarvesterUnit list.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004BD0BC, _HouseClass_AI_Is_Building_Harvester_Unit_Patch, 0)
{
    GET(HouseClass*, this_ptr, ESI);

    if (this_ptr->BuildUnit != UNIT_NONE && Rule->HarvesterUnit.Is_Present(UnitTypes[this_ptr->BuildUnit])) {
        return 0x004BD0E5;
    }

    return 0x004BD0D7;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all refineries, weapons factories and harvesters.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004C0D0C, _HouseClass_AI_Raise_Money_HarvRef1, 0)
{
    GET(HouseClass*, this_ptr, ESI);

    bool build_harv;
    int object_cost;

    /**
     *  If we have a refinery and a weapons factory, build a harvester, otherwise - a refinery.
     */
    if (this_ptr->Count_Owned(Rule->BuildRefinery) > 0 && this_ptr->Count_Owned(Rule->BuildWeapons) > 0) {
        build_harv = true;
        object_cost = this_ptr->Get_First_Ownable(Rule->HarvesterUnit)->Cost_Of(this_ptr);
    } else {
        build_harv = false;
        object_cost = this_ptr->Get_First_Ownable(Rule->BuildRefinery)->Cost_Of(this_ptr);
    }

    R->Stack8(0x13, build_harv);
    R->EAX(object_cost);
    return 0x004C0D94;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly construct its own faction's harvester.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004C0F5F, _HouseClass_AI_Raise_Money_HarvRef2, 0)
{
    GET(HouseClass*, this_ptr, ESI);

    UnitType harv = this_ptr->Get_First_Ownable(Rule->HarvesterUnit)->HeapID;

    R->EAX(harv);
    return 0x004C0F72;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly construct its own faction's refinery.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004C0FAB, _HouseClass_AI_Raise_Money_HarvRef3, 0)
{
    GET(HouseClass*, this_ptr, ESI);

    static BuildingTypeClass* refinery_ptr;
    refinery_ptr = this_ptr->Get_First_Ownable(Rule->BuildRefinery);

    // The instructions here are messy, so we hijack when the game
    // is accessing the vector and substitute our pointer
    R->EDX(&refinery_ptr);
    return 0x004C0FBB;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly construct its own faction's refinery.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004C1051, _HouseClass_AI_Raise_Money_HarvRef4, 0)
{
    GET(HouseClass*, this_ptr, ESI);

    BuildingTypeClass* refinery = this_ptr->Get_First_Ownable(Rule->BuildRefinery);

    R->EAX(refinery);
    return 0x004C105E;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly count all harvesters and refineries.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004C166D, _HouseClass_AI_Unit_HarvRef1, 0)
{
    GET(HouseClass*, this_ptr, EBP);
    const int harv_count = this_ptr->Count_Owned(Rule->HarvesterUnit);
    const int ref_count = this_ptr->Count_Owned(Rule->BuildRefinery);

    R->ESI(harv_count);
    R->EAX(ref_count);
    return 0x004C16AE;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly building its own faction's harvester.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004C1710, _HouseClass_AI_Unit_HarvRef2, 0)
{
    GET(HouseClass*, this_ptr, EBP);
    UnitTypeClass* harvester = this_ptr->Get_First_Ownable(Rule->HarvesterUnit);

    R->EAX(harvester);
    return 0x004C1718;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list in prerequisite checks.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004C5977, _HouseClass_Has_Prerequisites_BuildConst, 0)
{
    GET(BuildingTypeClass*, building, ECX);

    if (!Rule->BuildConst.Is_Present(building)) {
        return 0x004C5985;
    }

    return 0x004C5B62;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004C5E20, _HouseClass_GenerateAIBuildList_4C5BB0_BuildConst, 0)
{
    GET_STACK(HouseClass*, this_ptr, 0x14);
    BuildingTypeClass* conyard = this_ptr->Get_First_Ownable(Rule->BuildConst);

    R->ESI(conyard);
    return 0x004C5E28;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list as targets for the Ion Cannon.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004CA222, _HouseClass_AI_Use_Super_Ion_Cannon_BuildConst, 0)
{
    GET(UnitTypeClass*, unittype, ECX);

    if (Rule->BuildConst.Is_Present(unittype->DeploysInto)) {
        return 0x004CA232;
    }

    return 0x004CA240;
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list when the AI takes over a player's house.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004CA9A1, _HouseClass_AI_Takeover_BuildConst, 0)
{
    GET(BuildingTypeClass*, buildingtype, ECX);

    if (Rule->BuildConst.Is_Present(buildingtype)) {
        return 0x004CA9A9;
    }

    return 0x004CA9B7;
}


/**
 *  #issue-177
 *
 *  Fix a vanilla bug where vehicles thieves were able to target harvesters even when HarvesterTruce was on.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004D7284, _InfantryClass_What_Action_Harvester_Thief, 0)
{
    GET(UnitClass*, target, ESI);

    if (target->RTTI == RTTI_UNIT && Rule->HarvesterUnit.Is_Present(target->Class)) {
        return 0x004D7258;
    }

    return 0x004D72A8;
}


/**
 *  Patch to enable base nodes for the AI when UseMPAIBaseNodes=yes is set in the scenario.
 *
 *  @author: ZivDero, Rampastring
 */
DEFINE_HOOK(0x004CB9DE, _HouseClass_Can_Build_Here_MP_AI_BaseNodes_Patch, 5)
{
    /**
     *  Also ignore AIBaseSpacing if it was requested by the client.
     */
    if (ScenExtension->IsUseMPAIBaseNodes) {
        return 0x004CB9D2;
    }

    /**
     *  Continue with AIBaseSpacing.
     */
    return 0;
}


/**
 *  Replacement to Assign_Handicap to read from our new difficulty settings.
 *
 *  @author: Rampastring
 */
ExtDiffType HouseClassExt::_Assign_Handicap(ExtDiffType handicap)
{
    ExtDiffType old = (ExtDiffType)Difficulty;

    /**
     *  We have not fully replaced the original difficulty logic yet, so
     *  we'll have to limit the "actual house difficulty" to vanilla
     *  levels or it'll read out of bounds.
     */
    Difficulty = (DiffType)(handicap >= DIFF_COUNT ? 0 : handicap);

    DEBUG_INFO("Assigning handicap {} to house {}\n", (int)handicap, (int)HeapID);

    if (handicap >= EXT_DIFF_COUNT) {
        DEBUG_ERROR("Invalid value supplied to HouseClassExt::_Assign_Handicap! {}", (int)handicap);
        Emergency_Exit(0);
        return old;
    }

    DifficultyClass* diff = &RuleExtension->Diff[handicap];
    if (handicap == DIFF_NORMAL && Is_Human_Player() && RuleExtension->IsHasPlayerNormal) {
        diff = &RuleExtension->PlayerNormal;
    }

    if (Session.Type != GAME_NORMAL) {
        HouseTypeClass const* hptr = Class;
        FirepowerBias = hptr->FirepowerBias * diff->FirepowerBias;
        GroundspeedBias = hptr->GroundspeedBias * diff->GroundspeedBias * Rule->GameSpeedBias;
        AirspeedBias = hptr->AirspeedBias * diff->AirspeedBias * Rule->GameSpeedBias;
        ArmorBias = hptr->ArmorBias * diff->ArmorBias;
        ROFBias = hptr->ROFBias * diff->ROFBias;
        CostBias = hptr->CostBias * diff->CostBias;
        RepairDelay = diff->RepairDelay;
        BuildDelay = diff->BuildDelay;
        BuildSpeedBias = hptr->BuildSpeedBias * diff->BuildSpeedBias * Rule->GameSpeedBias;
    } else {
        FirepowerBias = diff->FirepowerBias;
        GroundspeedBias = diff->GroundspeedBias * Rule->GameSpeedBias;
        AirspeedBias = diff->AirspeedBias * Rule->GameSpeedBias;
        ArmorBias = diff->ArmorBias;
        ROFBias = diff->ROFBias;
        CostBias = diff->CostBias;
        RepairDelay = diff->RepairDelay;
        BuildDelay = diff->BuildDelay;
        BuildSpeedBias = diff->BuildSpeedBias * Rule->GameSpeedBias;
    }

    TeamTime = 30 * HeapID + Rule->TeamDelays[Difficulty];

    return old;
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

    Patch_Jump(0x004BAED0, &HouseClassExt::_Can_Make_Money);
    Patch_Jump(0x004C0A40, &HouseClassExt::_Check_Raise_Money);
    Patch_Jump(0x004BDB50, &HouseClassExt::_Make_Ally);

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

    Patch_Jump(0x004BC077, 0x004BC082); // HouseClass::Can_Build, always check for ConYard of required Owner

    Patch_Jump(0x004BF4C0, &HouseClassExt::_MPlayer_Defeated);
    Patch_Jump(0x004C4730, &HouseClassExtension::House_From_HousesType);

    /**
     *  Patch away a few checks for GAME_INTERNET to enable statistics collection.
     */
    Patch_Jump(0x004C220B, 0x004C2218); // HouseClass::Add_Tracking
    Patch_Jump(0x004C2255, 0x004C2262); // HouseClass::Add_Tracking
    Patch_Jump(0x004C229F, 0x004C22A8); // HouseClass::Add_Tracking
    Patch_Jump(0x004C22E5, 0x004C22EE); // HouseClass::Add_Tracking

    Patch_Jump(0x004BB460, &HouseClassExt::_Assign_Handicap);
}
