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
#include "unittype.h"
#include "rules.h"
#include "rulesext.h"
#include "unit.h"
#include "session.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "buildingext.h"
#include "extension_globals.h"
#include "sidebarext.h"
#include "rules.h"
#include "session.h"
#include "ccini.h"
#include "fetchres.h"
#include "sideext.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "houseext.h"
#include "language.h"
#include "scenarioext.h"
#include "spawner.h"
#include "tibsun_functions.h"
#include "vox.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or deconstructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class HouseClassExt final : public HouseClass
{
public:
    ProdFailType _Begin_Production(RTTIType type, int id, bool resume);
    ProdFailType _Abandon_Production(RTTIType type, int id);
    bool _Can_Make_Money();
    UrgencyType _Check_Raise_Money();
    int _AI_Building();
    void _MPlayer_Defeated();
    DiffType _Assign_Handicap(DiffType handicap);
};


/**
 *  Reimplements the entire HouseClass::Begin_Production function.
 *
 *  @author: ZivDero
 */
ProdFailType HouseClassExt::_Begin_Production(RTTIType type, int id, bool resume)
{
    bool has_suspended = false;
    bool suspend = false;
    FactoryClass* fptr;
    TechnoTypeClass const* tech = Fetch_Techno_Type(type, id);

    if (!tech->Who_Can_Build_Me(false, true, true, this))
    {
        if (!resume || !tech->Who_Can_Build_Me(true, false, true, this))
        {
            DEBUG_INFO("Request to Begin_Production of '%s' was rejected. No-one can build.\n", tech->FullName);
            return PROD_CANT;
        }
        suspend = true;
    }

    fptr = Fetch_Factory(type);

    /*
    **	If no factory exists, create one.
    */
    if (fptr == nullptr)
    {
        fptr = new FactoryClass();
        if (!fptr)
        {
            DEBUG_INFO("Request to Begin_Production of '%s' was rejected. Unable to create factory\n", tech->FullName);
            return PROD_CANT;
        }
        Set_Factory(type, fptr);
    }

    /*
    **	If the house is already busy producing a building, then
    **	return with this failure code.
    */
    if (fptr->Is_Building() && type == RTTI_BUILDINGTYPE)
    {
        DEBUG_INFO("Request to Begin_Production of '%s' was rejected. Cannot queue buildings.\n", tech->FullName);
        return PROD_CANT;
    }

    /*
    **	Check if we have an object of this type currently suspended in production.
    */
    if (fptr->IsSuspended)
    {
        ObjectClass* obj = fptr->Get_Object();
        if (obj != nullptr && obj->Techno_Type_Class() == tech)
        {
            has_suspended = true;
        }
    }

    if (has_suspended || fptr->Set(*tech, *this, resume))
    {
        if (has_suspended || resume || fptr->Queued_Object_Count() == 0)
        {
            fptr->Start(suspend);

            /*
            **	Link this factory to the sidebar so that proper graphic feedback
            **	can take place.
            */
            if (PlayerPtr == this)
                Map.Factory_Link(fptr, type, id);

            return PROD_OK;
        }
        else
        {
            SidebarExtension->Flag_Strip_To_Redraw(type);
            return PROD_OK;
        }
    }

    DEBUG_INFO("Request to Begin_Production of '%s' was rejected. Factory was unable to create the requested object\n", tech->Full_Name());

    /*
    **	If the factory has queued objects or is currently
    **  building an object, reject production.
    */
    if (fptr->Queued_Object_Count() > 0 || fptr->Object)
        return PROD_CANT;


    /*
    **	Output debug information if production failed.
    */
    DEBUG_INFO("type=%d\n", type);
    DEBUG_INFO("Frame == %d\n", Frame);
    DEBUG_INFO("fptr->QueuedObjects.Count() == %d\n", fptr->QueuedObjects.Count());
    if (fptr->Get_Object())
    {
        DEBUG_INFO("Object->RTTI == %d\n", fptr->Object->Kind_Of());
        DEBUG_INFO("Object->HeapID == %d\n", fptr->Object->Get_Heap_ID());
    }
    DEBUG_INFO("IsSuspended\t= %d\n", fptr->IsSuspended);

    delete fptr;
    Set_Factory(type, nullptr);

    return PROD_CANT;
}


/**
 *  Reimplements the entire HouseClass::Abandon_Production function.
 *
 *  @author: ZivDero
 */
ProdFailType HouseClassExt::_Abandon_Production(RTTIType type, int id)
{
    FactoryClass* fptr = Fetch_Factory(type);

    /*
    **	If there is no factory to abandon, then return with a failure code.
    */
    if (fptr == nullptr)
        return PROD_CANT;

    /*
    **	If we're just dequeuing a unit, redraw the strip.
    */
    if (fptr->Queued_Object_Count() > 0 && id >= 0)
    {
        const TechnoTypeClass* technotype = Fetch_Techno_Type(type, id);
        if (fptr->Remove_From_Queue(*technotype))
        {
            SidebarExtension->Flag_Strip_To_Redraw(type);
            return PROD_OK;
        }
    }

    if (id != -1)
    {
        ObjectClass* obj = fptr->Get_Object();
        if (obj == nullptr)
            return PROD_OK;

        ObjectTypeClass* cls = obj->Class_Of();
        if (id != cls->Get_Heap_ID())
            return PROD_OK;
    }

    /*
    **	Tell the sidebar that it needs to be redrawn because of this.
    */
    if (PlayerPtr == this)
    {
        Map.Abandon_Production(type, fptr);
        if (type == RTTI_BUILDINGTYPE || type == RTTI_BUILDING)
        {
            Map.PendingObjectPtr = nullptr;
            Map.PendingObject = nullptr;
            Map.PendingHouse = HOUSE_NONE;
            Map.Set_Cursor_Shape(nullptr);
        }
    }

    /*
    **	Abandon production of the object.
    */
    fptr->Abandon();
    if (fptr->Queued_Object_Count() > 0)
    {
        fptr->Resume_Queue();
        return PROD_OK;
    }

    Set_Factory(type, nullptr);
    delete fptr;

    return PROD_OK;
}


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

    if (BuildStructure != BUILDING_NONE) return TICKS_PER_SECOND;

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
    if (node->Type == BASE_DEFENSE || BuildingTypes[node->Type] == Rule->WallTower && node->Where.X != 0 && node->Where.Y != 0) {

        const int nodeid = Base.Nodes.ID(node);
        if (!AI_Build_Defense(nodeid, Base.field_38.Count() > 0 ? &Base.field_38 : nullptr)) {

            /**
             *  If it's a wall tower, delete it twice?
             *  Perhaps it's assumed that the wall tower is followed by its upgrade?
             */
            if (node->Type == Rule->WallTower->Get_Heap_ID()) {
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
    if (Session.Type == GAME_NORMAL || ScenExtension->IsUseMPAIBaseNodes || b->Drain + Drain <= Power - PowerSurplus || Rule->BuildConst.Is_Present(b) || b->Drain <= 0) {

        /**
         *  Check if this is a building upgrade if we can actually place the upgrade where it's scheduled to be placed.
         */
        if (b->PowersUpToLevel == -1 && !node->Where) {
            if (b->PowersUpBuilding[0]) {

                BuildingClass* existing_b = Map[node->Where].Cell_Building();
                BuildingTypeClass* desired_b = BuildingTypes[BuildingTypeClass::From_Name(b->PowersUpBuilding)];

                if (!existing_b || existing_b->Class != desired_b
                    || existing_b->Class->PowersUpToLevel == -1 && existing_b->UpgradeLevel >= existing_b->Class->Upgrades
                    || existing_b->Class->PowersUpToLevel == 0 && existing_b->UpgradeLevel > 0) {

                    node->Where = Cell();
                }
            }
        }

        BuildStructure = node->Type;
        return TICKS_PER_SECOND;
    }

    /**
     *  In skirmish, try to build a power plant if there is insufficient power.
     */
    const BuildingTypeClass* choice = nullptr;
    const auto side_ext = Extension::Fetch<SideClassExtension>(Sides[Class->Side]);

    /**
     *  First let's see if we can upgrade a power plant with a turbine (like GDI).
     */
    if (side_ext->PowerTurbine) {

        bool can_build_turbine = false;
        for (int i = 0; i < Buildings.Count(); i++) {

            BuildingClass* owned_b = Buildings[i];
            if (owned_b->Owning_House() == this) {
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
            if (b2->Owning_House() == this) {
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
    Base.Nodes.Insert(Base.Nodes.ID(node), BaseNodeClass(choice->Type, Cell()));

    return 1;
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
        if (BuildUnit != harvester->Type)
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
                    && Rule->HarvesterUnit.Is_Present(static_cast<UnitTypeClass*>(obj->Techno_Type_Class())))
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
        if (BuildStructure != refinery->Type)
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
                    && Rule->BuildRefinery.Is_Present(static_cast<BuildingTypeClass*>(obj->Techno_Type_Class())))
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
 */
void HouseClassExt::_MPlayer_Defeated()
{
    char txt[80];
    int i, j;
    unsigned char id;
    HouseClass* hptr;
    HouseClass* hptr2;
    int num_alive;
    int num_humans;
    bool all_allies;

    /**
     *  Set the defeat flag for this house
     */
    IsDefeated = true;

    /**
     *  If this is a computer controlled house, then all computer controlled
     *  houses become paranoid.
     */
    if (IQ == Rule->MaxIQ && !(IsHuman || Session.Type == GAME_NORMAL && IsPlayerControl) && Rule->IsComputerParanoid) {
        Computer_Paranoid();
    }

    /**
     *  Remove this house's flag & flag home cell
     */
    if (Special.IsCaptureTheFlag) {
        if (FlagLocation) {
            Flag_Remove(FlagLocation, true);
        }
        else {
            if (FlagHome != 0) {
                Flag_Remove(&Map[FlagHome], true);
            }
        }
    }

    /**
     *  If harvester truce is on, remove all of this player's harvesters.
     */
    if (Session.Type != GAME_NORMAL && Scen->SpecialFlags.IsHarvesterImmune) {
        for (int i = 0; i < Units.Count(); i++) {
            if (Units[i]->Owning_House() == this && Units[i]->IsActive) {
                Units[i]->Remove_This();
            }
        }
    }

    /**
     *  If this is me:
     *  - Set MPlayerObiWan, so I can only send messages to all players, and
     *    not just one (so I can't be obnoxiously omnipotent)
     *  - Reveal the map
     *  - Add my defeat message
     */
    if (PlayerPtr == this) {
        Session.ObiWan = true;
        Map.Reveal_The_Map();
        HiddenSurface->Fill(0);

        if (Vinifera_ObserverPtr != this) {
            /**
             *  Pop up a message showing that I was defeated
             */
            std::snprintf(txt, std::size(txt), Fetch_String(TXT_PLAYER_DEFEATED), IniName);
            Session.Messages.Add_Message(nullptr, 0, txt, static_cast<ColorSchemeType>(Session.ColorIdx), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, Rule->MessageDelay * TICKS_PER_MINUTE);
            Speak(VOX_YOU_HAVE_LOST);
        }

        Map.Flag_To_Redraw(0);
        DEBUG_INFO("MPlayer_Defeated() - Player %s has been defeated (OBIWAN MODE)\n", IniName);

    }
    else {

        /**
         *  If it wasn't me, find out who was defeated
         */
        if (!Class->IsMultiplayPassive) {
            if (Vinifera_ObserverPtr != this) {
                std::snprintf(txt, std::size(txt), Fetch_String(TXT_PLAYER_DEFEATED), IniName);
                Session.Messages.Add_Message(nullptr, 0, txt, RemapColor, TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, Rule->MessageDelay * TICKS_PER_MINUTE);
                Speak(VOX_PLAYER_DEFEATED);
                
            }

            Map.Flag_To_Redraw(0);
            DEBUG_INFO("MPlayer_Defeated() - Opponent %s has been defeated\n", IniName);
        }
    }

    /**
     *  Find out how many players are left alive.
     */
    num_alive = 0;
    num_humans = 0;
    for (i = 0; i < Houses.Count(); i++) {
        hptr = Houses[i];
        if (hptr && !hptr->IsDefeated && !hptr->Class->IsMultiplayPassive) {
            if (hptr->IsHuman || (Session.Type == GAME_NORMAL && hptr->IsPlayerControl)) {
                num_humans++;
            }
            num_alive++;
        }
    }
    DEBUG_INFO("MPlayer_Defeated() - Alive = %d, Humans = %d\n", num_alive, num_humans);

    /**
     *  If all the houses left alive are allied with each other, then in reality
     *  there's only one player left:
     */
    all_allies = true;
    for (i = 0; i < Houses.Count(); i++) {

        /**
         *  Get a pointer to this house
         */
        hptr = Houses[i];
        if (!hptr || hptr->IsDefeated || hptr->Class->IsMultiplayPassive)
            continue;

        /**
         *  Loop through all houses; if there's one left alive that this house
         *  isn't allied with, then all_allies will be false
         */
        for (j = 0; j < Houses.Count(); j++) {
            hptr2 = Houses[j];
            if (!hptr2) {
                continue;
            }

            if (!hptr2->IsDefeated && !hptr2->Class->IsMultiplayPassive && hptr != hptr2 && !hptr->Is_Ally(hptr2)) {
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
     *  If there's only one human player left or no humans left, the game is over:
     *  - Determine whether this player wins or loses, based on the state of the
     *    player's IsDefeated flag
     */
    HouseClassExtension* houseext = Extension::Fetch<HouseClassExtension>(this);
    if (!houseext->IsObserver) {
        if (num_alive == 1 || (num_humans == 0 && !(Vinifera_SpawnerActive && Vinifera_SpawnerConfig->ContinueWithoutHumans))) {
            IsToDie = false;

            if (PlayerPtr->IsDefeated) {
                DEBUG_INFO("MPlayer_Defeated() - Flag_To_Lose\n");
                Flag_To_Lose(false);
            }
            else {
                DEBUG_INFO("MPlayer_Defeated() - Flag_To_Win\n");
                Flag_To_Win(false);
            }
        }
    }

    
}


/**
 *  Assigns the specified handicap rating to the house.
 *
 *  @author: 07/09/1996 JLB - Created
 *           29/10/2024 ZivDero - Adjustments for Tiberian Sun
 */
DiffType HouseClassExt::_Assign_Handicap(DiffType handicap)
{
    DiffType old = Difficulty;
    Difficulty = handicap;

    const DifficultyClass* diff = &RuleExtension->Diff[handicap];

    if (Is_Human_Control() && handicap == DIFF_NORMAL && Vinifera_HumanNormalDifficulty) {
        diff = &RuleExtension->DiffHuman;
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
    }
    else {
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

    TeamTime = 175 * ID + Rule->TeamDelays[handicap];

    return old;
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
DECLARE_PATCH(_HouseClass_AI_Short_Game_BaseUnit_Patch)
{
    GET_REGISTER_STATIC(HouseClass *, this_ptr, esi);
    static UnitTypeClass *unittype;
    static UnitType unit;
    static int count;

    /**
     *  Count all MCVs we own to see if the player should explode.
     */
    count = this_ptr->Count_Owned(RuleExtension->BaseUnit);

    if (count) {
        goto continue_function;
    }

    goto blowup_house;

    /**
     *  
     */
continue_function:
    JMP_REG(eax, 0x004BCF6E);

    /**
     *  Blows up the house, marking the house as defeated.
     */
blowup_house:
    JMP_REG(ecx, 0x004BCF60);
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
                && !this_ptr->Is_Human_Control() && this_ptr != PlayerPtr) {

                special->Forced_Charge(is_player);

            /**
             *  If InstantBuild is toggled on, make sure the local player is a human house.
             */
            } else if (Vinifera_Developer_InstantSuperRecharge
                && this_ptr->Is_Human_Control() && this_ptr == PlayerPtr) {
                
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
            if (((1 << this_ptr->Class->ID) & objecttype->Get_Ownable()) != 0) {
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
    if (factoryobject->Techno_Type_Class() != technotype) {
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
    if (technotype->What_Am_I() == RTTI_UNITTYPE) {
        UnitTypeClass* unittype = reinterpret_cast<UnitTypeClass*>(technotype);
        UnitTypeClassExtension* unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);

        if (unittype->DeploysInto == nullptr && unittypeext->TransformsInto != nullptr) {
            count += factory->House->UQuantity.Count_Of((UnitType)(unittypeext->TransformsInto->Get_Heap_ID()));
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

    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);

    /**
     *  Stolen bytes / code.
     */
    objectcount = house->UQuantity.Count_Of((UnitType)unittype->Get_Heap_ID());

    /**
     *  Check whether this unit can deploy into a building.
     *  If it can, increment the object count by the number of buildings.
     */
    if (unittype->DeploysInto != nullptr) {
        objectcount += house->BQuantity.Count_Of((BuildingType)unittype->DeploysInto->Get_Heap_ID());
    }
    else if (unittypeext->TransformsInto != nullptr) {

        /**
         *  This unit can transform into another unit, increment the object count
         *  by the number of transformed units.
         */
        objectcount += house->UQuantity.Count_Of((UnitType)(unittypeext->TransformsInto->Get_Heap_ID()));
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
 *  #issue-177
 *
 *  Patches the check for if a house owns a Construction Yard to check the entire BuildConst list.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_BuildConst_Patch)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, esi);

    if (this_ptr->Count_Owned(Rule->BuildConst) > 0)
    {
        JMP(0x004BCD85);
    }

    JMP(0x004BCE0B);
}


/**
 *  #issue-177
 *
 *  Patches the check for if a house owns a harvester to check the entire HarvesterUnit list.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Count_HarvesterUnit_Patch)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, esi);
    static int harv_count;

    harv_count = this_ptr->Count_Owned(Rule->HarvesterUnit);

    _asm mov eax, harv_count
    JMP_REG(ecx, 0x004BCF5A);
}


/**
 *  #issue-177
 *
 *  Patches the check for if a house is building a harvester to check the entire HarvesterUnit list.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Is_Building_Harvester_Unit_Patch)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, esi);

    if (this_ptr->BuildUnit != -1 && Rule->HarvesterUnit.Is_Present(UnitTypes[this_ptr->BuildUnit]))
    {
        JMP(0x004BD0E5);
    }

    JMP(0x004BD0D7);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all refineries, weapons factories and harvesters.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Raise_Money_HarvRef1)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, esi);

    static bool build_harv;
    static int object_cost;

    /**
     *  If we have a refinery and a weapons factory, build a harvester, otherwise - a refinery.
     */
    if (this_ptr->Count_Owned(Rule->BuildRefinery) > 0
        && this_ptr->Count_Owned(Rule->BuildWeapons) > 0)
    {
        build_harv = true;
        object_cost = this_ptr->Get_First_Ownable(Rule->HarvesterUnit)->Cost_Of(this_ptr);
    }
    else
    {
        build_harv = false;
        object_cost = this_ptr->Get_First_Ownable(Rule->BuildRefinery)->Cost_Of(this_ptr);
    }

    _asm mov al, build_harv
    _asm mov [esp+0x13], al
    _asm mov eax, object_cost

    JMP_REG(ebx, 0x004C0D94);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly construct its own faction's harvester.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Raise_Money_HarvRef2)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, esi);

    UnitType harv;
    harv = this_ptr->Get_First_Ownable(Rule->HarvesterUnit)->Type;

    _asm mov eax, harv
    JMP_REG(ecx, 0x004C0F72);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly construct its own faction's refinery.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Raise_Money_HarvRef3)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, esi);

    BuildingTypeClass* refinery_ptr;
    BuildingTypeClass** refinery_ptr_ptr;
    refinery_ptr = this_ptr->Get_First_Ownable(Rule->BuildRefinery);
    refinery_ptr_ptr = &refinery_ptr;

    // The instructions here are messy, so we hijack when the game
    // is accessing the vector and substitute our pointer
    _asm mov edx, refinery_ptr_ptr
    JMP(0x004C0FBB);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly construct its own faction's refinery.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Raise_Money_HarvRef4)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, esi);
    BuildingTypeClass* refinery;

    refinery = this_ptr->Get_First_Ownable(Rule->BuildRefinery);

    _asm mov eax, refinery
    JMP_REG(ecx, 0x004C105E);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly count all harvesters and refineries.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Unit_HarvRef1)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, ebp);
    static int harv_count, ref_count;

    harv_count = this_ptr->Count_Owned(Rule->HarvesterUnit);
    ref_count = this_ptr->Count_Owned(Rule->BuildRefinery);

    _asm mov esi, harv_count
    _asm mov eax, ref_count
    JMP_REG(ecx, 0x004C16AE);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly building its own faction's harvester.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Unit_HarvRef2)
{
    GET_REGISTER_STATIC(HouseClass*, this_ptr, ebp);
    static UnitTypeClass* harvester;

    harvester = this_ptr->Get_First_Ownable(Rule->HarvesterUnit);

    _asm mov eax, harvester
    JMP_REG(edx, 0x004C1718);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list in prerequisite checks.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Has_Prerequisites_BuildConst)
{
    GET_REGISTER_STATIC(BuildingTypeClass*, building, ecx);
    _asm pushad

    if (!Rule->BuildConst.Is_Present(building))
    {
        _asm popad
        JMP(0x004C5985);
    }

    _asm popad
    JMP(0x004C5B62);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_GenerateAIBuildList_4C5BB0_BuildConst)
{
    GET_STACK_STATIC(HouseClass*, this_ptr, esp, 0x14);
    BuildingTypeClass* conyard;

    conyard = this_ptr->Get_First_Ownable(Rule->BuildConst);

    _asm mov esi, conyard;
    JMP(0x004C5E28);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list as targets for the Ion Cannon.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Use_Super_Ion_Cannon_BuildConst)
{
    GET_REGISTER_STATIC(UnitTypeClass*, unittype, ecx);
    _asm push eax

    if (Rule->BuildConst.Is_Present(unittype->DeploysInto))
    {
        _asm pop eax
        JMP_REG(ecx, 0x004CA232);
    }

    _asm pop eax
    JMP_REG(edx, 0x004CA240);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list when the AI takes over a player's house.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Takeover_BuildConst)
{
    GET_REGISTER_STATIC(BuildingTypeClass*, buildingtype, ecx);
    _asm push eax

    if (Rule->BuildConst.Is_Present(buildingtype))
    {
        _asm pop eax
        JMP_REG(edi, 0x004CA9A9);
    }

    _asm pop eax
    JMP_REG(edi, 0x004CA9B7)
}


/**
 *  #issue-177
 *
 *  Fix a vanilla bug where vehicles thieves were able to target harvesters even when HarvesterTruce was on.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_InfantryClass_What_Action_Harvester_Thief)
{
    GET_REGISTER_STATIC(UnitClass*, target, esi);

    if (target->What_Am_I() == RTTI_UNIT && Rule->HarvesterUnit.Is_Present(target->Class))
    {
        // return ACTION_SELECT;
        JMP(0x004D7258);
    }

    // return ACTION_CAPTURE;
    JMP(0x004D72A8);
}


/**
 *  Patch to enable base nodes for the AI when UseMPAIBaseNodes=yes is set in the scenario.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_AI_Building_MP_AI_BaseNodes_Patch)
{
    _asm pushad

    /**
     *  Use base nodes in Campaign.
     */
    if (Session.Type == GAME_NORMAL)
    {
        _asm popad
        JMP_REG(ecx, 0x004C1554);
    }

    /**
     *  Also use base nodes if it was requested by the client.
     */
    if (Vinifera_SpawnerActive && ScenExtension->IsUseMPAIBaseNodes)
    {
        _asm popad
        JMP_REG(ecx, 0x004C1554);
    }

    /**
     *  Continue checks.
     */
    _asm popad
    JMP_REG(ecx, 0x004C129D);
}


/**
 *  Patch to enable base nodes for the AI when UseMPAIBaseNodes=yes is set in the scenario.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Can_Build_Here_MP_AI_BaseNodes_Patch)
{
    // Stolen instructions
    _asm push edi
    _asm mov edi, ecx

    /**
     *  Ignore AIBaseSpacing in Campaign.
     */
    if (Session.Type == GAME_NORMAL)
    {
        // return 1;
        JMP(0x004CB9D2);
    }

    /**
     *  Also ignore AIBaseSpacing if it was requested by the client.
     */
    if (ScenExtension->IsUseMPAIBaseNodes)
    {
        // return 1;
        JMP(0x004CB9D2);
    }

    /**
     *  Continue with AIBaseSpacing.
     */
    JMP_REG(ecx, 0x004CB9DE);
}


/**
 *  Patch to enable base nodes for the AI when UseMPAIBaseNodes=yes is set in the scenario.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_HouseClass_Expert_AI_MP_AI_BaseNodes_Patch)
{
    _asm push eax

    if (Session.Type == GAME_NORMAL || ScenExtension->IsUseMPAIBaseNodes)
    {
        /**
         *  Skip trying to raise money.
         */
        _asm pop eax
        JMP_REG(ecx, 0x004C09AF);


    }

    /**
     *  Potentially try to raise money
     */
    _asm pop eax
    JMP_REG(ecx, 0x004C08D1);
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
    Patch_Jump(0x004BBD26, &_HouseClass_Can_Build_BuildCheat_Patch);

    Patch_Jump(0x004BCD5D, &_HouseClass_AI_BuildConst_Patch);
    Patch_Jump(0x004BCEE7, &_HouseClass_AI_Short_Game_BaseUnit_Patch);
    Patch_Jump(0x004BCF3A, &_HouseClass_AI_Count_HarvesterUnit_Patch);
    Patch_Jump(0x004BD0BC, &_HouseClass_AI_Is_Building_Harvester_Unit_Patch);
    Patch_Jump(0x004C0D0C, &_HouseClass_AI_Raise_Money_HarvRef1);
    Patch_Jump(0x004C0F5F, &_HouseClass_AI_Raise_Money_HarvRef2);
    Patch_Jump(0x004C0FAB, &_HouseClass_AI_Raise_Money_HarvRef3);
    Patch_Jump(0x004C1051, &_HouseClass_AI_Raise_Money_HarvRef4);
    Patch_Jump(0x004C166D, &_HouseClass_AI_Unit_HarvRef1);
    Patch_Jump(0x004C1710, &_HouseClass_AI_Unit_HarvRef2);
    Patch_Jump(0x004C5977, &_HouseClass_Has_Prerequisites_BuildConst);
    Patch_Jump(0x004C5E20, &_HouseClass_GenerateAIBuildList_4C5BB0_BuildConst);
    Patch_Jump(0x004CA222, &_HouseClass_AI_Use_Super_Ion_Cannon_BuildConst);
    Patch_Jump(0x004CA9A1, &_HouseClass_AI_Takeover_BuildConst);
    Patch_Jump(0x004D7284, &_InfantryClass_What_Action_Harvester_Thief);

    Patch_Jump(0x004BE200, &HouseClassExt::_Begin_Production);
    Patch_Jump(0x004BE6A0, &HouseClassExt::_Abandon_Production);
    Patch_Jump(0x004BAED0, &HouseClassExt::_Can_Make_Money);
    Patch_Jump(0x004C0A40, &HouseClassExt::_Check_Raise_Money);
    Patch_Jump(0x004C10E0, &HouseClassExt::_AI_Building);
    Patch_Jump(0x004BF4C0, &HouseClassExt::_MPlayer_Defeated);
    Patch_Jump(0x004BB460, &HouseClassExt::_Assign_Handicap);

    Patch_Jump(0x004CB777, &_HouseClass_ShouldDisableCameo_BuildLimit_Fix);
    Patch_Jump(0x004BC187, &_HouseClass_Can_Build_BuildLimit_Handle_Vehicle_Transform);

    Patch_Jump(0x004CB6C1, &_HouseClass_Enable_SWs_Check_For_Building_Power);

    Patch_Jump(0x004BAC2C, 0x004BAC39); // Patch a jump in the constructor to always allocate unit trackers
    Patch_Jump(0x004BC077, 0x004BC082); // HouseClass::Can_Build, always check for ConYard of required Owner

    /**
     *  Patch away a few checks for GAME_INTERNET to enable statistics collection.
     */
    Patch_Jump(0x004C220B, 0x004C2218); // HouseClass::Add_Tracking
    Patch_Jump(0x004C2255, 0x004C2262); // HouseClass::Add_Tracking
    Patch_Jump(0x004C229F, 0x004C22A8); // HouseClass::Add_Tracking
    Patch_Jump(0x004C22E5, 0x004C22EE); // HouseClass::Add_Tracking

    Patch_Jump(0x004C128F, &_HouseClass_AI_Building_MP_AI_BaseNodes_Patch);
    Patch_Jump(0x004CB9CD, &_HouseClass_Can_Build_Here_MP_AI_BaseNodes_Patch);
    Patch_Jump(0x004C08C5, &_HouseClass_Expert_AI_MP_AI_BaseNodes_Patch);
}
