/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTIONEXT.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Extended TActionClass class.
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
#include "tactionext.h"
#include "taction.h"
#include "house.h"
#include "object.h"
#include "vinifera_defines.h"
#include "wwcrc.h"
#include "asserthandler.h"
#include "debughandler.h"
#include "houseext.h"
#include "housetype.h"
#include "rules.h"
#include "scenario.h"
#include "scenarioext.h"
#include "session.h"
#include "tag.h"
#include "techno.h"
#include "tibsun_inline.h"
#include "trigger.h"
#include "triggertype.h"
#include "voc.h"


TActionClass::ActionDescriptionStruct TActionClassExtension::ExtActionDescriptions[EXT_TACTION_COUNT - EXT_TACTION_FIRST] = {
    { "Give Credits", "Gives or removes credits from the specified house. A positive amount gives money, a negative amount subtracts it." },
    { "Enable Short Game", "Enables the Short Game mode. Players will lose if all buildings are destroyed." },
    { "Disable Short Game", "Disables the Short Game mode. Players can continue playing even after all buildings are destroyed." },
    { "Unused Action", "This action does nothing. Originally used to display the difficulty in ts-patches." },
    { "Blow Up House", "Instantly destroys all buildings and units of the specified house and marks them as defeated." },
    { "Make Elite", "All units and buildings attached to this trigger will be promoted to elite status." },
    { "Enable Ally Reveal", "Enables the Ally Reveal feature, allowing allied players to see each other's explored areas." },
    { "Disable Ally Reveal", "Disables the Ally Reveal feature, hiding the fog of war even between allies." },
    { "Create Autosave", "Schedules an autosave to be created on the next game frame." },
    { "Delete Attached Objects", "Deletes all units and structures on the map that are linked to this trigger silently." },
    { "All Assign Mission", "Forces all units owned by the trigger's house to begin the specified mission (e.g., hunt, move)." },
};


/**
  *  Returns the name of the TActionType.
  *
  *  @author: CCHyper, ZivDero
  */
const char* TActionClassExtension::Action_Name(int action)
{
    if (action < TACTION_COUNT) {
        return TActionClass::Action_Name(TActionType(action));
    }

    if (action < EXT_TACTION_COUNT) {
        return ExtActionDescriptions[action - EXT_TACTION_FIRST].Name;
    }

    return "<invalid>";
}


/**
 *  Returns the description of the TActionType.
 *
 *  @author: CCHyper, ZivDero
 */
const char* TActionClassExtension::Action_Description(int action)
{
    if (action < TACTION_COUNT) {
        return TActionClass::Action_Description(TActionType(action));
    }

    if (action < EXT_TACTION_COUNT) {
        return ExtActionDescriptions[action - EXT_TACTION_FIRST].Description;
    }

    return "<invalid>";
}


/**
 *  Executes the new trigger action.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Execute(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    bool success = false;

    #define DISPATCH(a) case TACTION_ ## a: success = Do_ ## a (taction, house, object, trig, cell); break;
    #define EXT_DISPATCH(a) case EXT_TACTION_ ## a: success = Do_ ## a (taction, house, object, trig, cell); break;

    // warning C4063: case '#' is not a valid value for switch of enum 'TActionType'
    #pragma warning(push)
    #pragma warning(disable : 4063)

    switch (taction.Action) {

        /**
         *  Intercepted vanilla TActions.
         */
        DISPATCH(WIN);
        DISPATCH(LOSE);
        DISPATCH(DESTROY_TRIGGER);
        DISPATCH(ENABLE_TRIGGER);
        DISPATCH(PLAY_SOUND_RANDOM);

        /**
         *  New Vinifera TActions.
         */
        EXT_DISPATCH(GIVE_CREDITS);
        EXT_DISPATCH(ENABLE_SHORT_GAME);
        EXT_DISPATCH(DISABLE_SHORT_GAME);
        EXT_DISPATCH(BLOWUP_HOUSE);
        EXT_DISPATCH(MAKE_ELITE);
        EXT_DISPATCH(ENABLE_ALLYREVEAL);
        EXT_DISPATCH(DISABLE_ALLYREVEAL);
        EXT_DISPATCH(CREATE_AUTOSAVE);
        EXT_DISPATCH(DELETE_OBJECT);
        EXT_DISPATCH(ALL_ASSIGN_MISSION);

        /**
         *  Used to print the current difficulty in ts-patches, available to be repurposed.
         */
    case EXT_TACTION_UNUSED1:
        success = true;

        /**
         *  Unexpected TActionType.
         */
    default:
        DEV_DEBUG_WARNING("Invalid action type (%d)!\n", taction.Action);
        break;
    }

    #pragma warning(pop)

    return success;
}


/**
 *  Checks if this is a TAction whose execution we should intercept.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Is_Vinifera_TAction(TActionType type)
{
    /**
     *  Hack: if the action is to create an autosave, pass that to the game
     *  for ts-patches to handle until we implement our own autosaves.
     */
    if (type == EXT_TACTION_CREATE_AUTOSAVE) {
        return false;
    }

    /**
     *  All new TActions are always executed by us.
     */
    if (type >= EXT_TACTION_FIRST && type < EXT_TACTION_COUNT) {
        return true;
    }

    /**
     *  We also intercept some vanilla TActions.
     */
    switch (type) {
    case TACTION_WIN:
    case TACTION_LOSE:
    case TACTION_DESTROY_TRIGGER:
    case TACTION_ENABLE_TRIGGER:
    case TACTION_PLAY_SOUND_RANDOM:
        return true;

    default:
        break;
    }

    /**
     *  Let the game execute the rest.
     */
    return false;
}


/**
 *  Helper info for writing new actions.
 *
 *  TActionClass::Data                  = First Param (PARAM1)
 *  TActionClass::TriggerRect.X         = Second Param (PARAM2)
 *  TActionClass::TriggerRect.Y         = Third Param (PARAM3)
 *  TActionClass::TriggerRect.W         = Fourth Param (PARAM4)
 *  TActionClass::TriggerRect.H         = Fifth Param (PARAM5)
 *
 *  (PARAM6) (OPTIONAL)
 *  if NeedCode == 4
 *    TActionClass::Data (overwrites PARAM1)
 *  else
 *    TActionClass::EffectLocation
 *
 *
 *  Example action line from a scenario file;
 *
 *  [Actions]
 *  NAME = [Action Count], [TActionType], [NeedCode], [PARAM1], [PARAM2], [PARAM3], [PARAM4], [PARAM5], [PARAM6:OPTIONAL]
 *
 *
 *  For producing FinalSun [Action] entries;
 *  NOTE: For available ParamTypes, see the [ParamTypes] section in FSData.INI.
 *  NOTE: "DEF_PARAM1_VALUE" if negative (-ve), PARAM1 will be set to the absolute value of this number (filled in).
 *
 *  [Actions]
 *  TActionType = [Name], [DEF_PARAM1_VALUE], [PARAM1_TYPE], [PARAM2_TYPE], [PARAM3_TYPE], [PARAM4_TYPE], [PARAM5_TYPE], [PARAM6_TYPE], [USE_WP], [USE_TAG], [Description], 1, 0, [TActionType]
 */


/**
 *  #issue-965
 *
 *  Makes the "Winner is" trigger action set the IsDefeated flag on losing
 *  houses in multiplayer.
 *
 *  @author: Rampastring
 */
bool TActionClassExtension::Do_WIN(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Flag the player as won or lost, like in the original code.
     */
    if (taction.Data.House == PlayerPtr->Class->House) {
        PlayerPtr->Flag_To_Win();
    } else {
        PlayerPtr->Flag_To_Lose();
    }

    if (Session.Type != GAME_NORMAL) {

        /**
         *  Mark all other houses than the winner as defeated.
         */
        for (int i = 0; i < Houses.Count(); i++) {
            HouseClass* hptr = Houses[i];

            if (hptr->Class->House != taction.Data.House) {
                hptr->IsDefeated = true;
            }
        }
    }

    return true;
}


/**
 *  #issue-965
 *
 *  Makes the "Loser is" trigger action set the IsDefeated flag on the
 *  losing house in multiplayer.
 *
 *  @author: Rampastring
 */
bool TActionClassExtension::Do_LOSE(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Flag the player as won or lost, like in the original code.
     */
    if (taction.Data.House != PlayerPtr->Class->House) {
        PlayerPtr->Flag_To_Win();
    } else {
        PlayerPtr->Flag_To_Lose();
    }
    
    if (Session.Type != GAME_NORMAL) {

        /**
         *  Mark all losers as defeated.
         */
        for (int i = 0; i < Houses.Count(); i++) {
            HouseClass* hptr = Houses[i];

            if (hptr->Class->House == taction.Data.House) {
                hptr->IsDefeated = true;
            }
        }
    }

    return true;
}


/**
 *  #issue-33
 *
 *  Fixes the issue where TACTION_DESTROY_TRIGGER would cause the game to
 *  crash if it's used to destroy triggers of its own type.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_DESTROY_TRIGGER(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (taction.Trigger != nullptr) {
        int count = Triggers.Count();

        for (int index = count - 1; index >= 0; index--) {
            if (Triggers[index]->Class == taction.Trigger) {
                Triggers[index]->Mark_To_Die();
            }
        }
    }
    return true;
}


/**
 *  #issue-299
 *
 *  Fixes the issue with the current difficulty not being checked
 *  when enabling triggers.
 *
 *  @author: CCHyper
 */
bool TActionClassExtension::Do_ENABLE_TRIGGER(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  This is direct port of the code from Red Alert 2, which looks to fix this issue.
     */
    if (taction.Trigger != nullptr) {
        for (int index = 0; index < Triggers.Count(); index++) {
            if (Triggers[index]->Class == taction.Trigger) {
                bool really_enable = true;

                /**
                 *  Set this trigger to be disabled if it is marked as disabled
                 *  for this current mission difficulty.
                 */
                if (Scen->Difficulty == DIFF_EASY && !Triggers[index]->Class->IsEnabledEasy) {
                    really_enable = false;
                } else if (Scen->Difficulty == DIFF_NORMAL && !Triggers[index]->Class->IsEnabledMedium) {
                    really_enable = false;
                } else if (Scen->Difficulty == DIFF_HARD && !Triggers[index]->Class->IsEnabledHard) {
                    really_enable = false;
                }

                if (really_enable) {
                    Triggers[index]->Enable();
                }
            }
        }
    }
    return true;
}


/**
 *  #issue-71
 *
 *  Reimplement Do_PLAY_SOUND_RANDOM to support the new waypoint limit.
 *
 *  @author: CCHyper
 */
bool TActionClassExtension::Do_PLAY_SOUND_RANDOM(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Cell list[NEW_WAYPOINT_COUNT];
    int count = 0;

    /**
     *  Make a list of all the valid waypoints in this scenario.
     */
    for (WAYPOINT index = WAYPOINT_FIRST; index < NEW_WAYPOINT_COUNT; ++index) {
        if (ScenExtension->Is_Waypoint_Valid(index)) {
            list[count++] = ScenExtension->Waypoint_Cell(index);
            if (count >= std::size(list)) break;
        }
    }

    /**
     *  Pick a random cell from the valid waypoint list and play the desired sound.
     */
    Static_Sound(taction.Data.Sound, list[Random_Pick(0u, std::size(list) - 1)].As_Coord());
    return true;
}


/**
 *  Gives credits to the house specified as the argument.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_GIVE_CREDITS(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    HouseClass* hptr = HouseClassExtension::House_From_HousesType(taction.Data.House);

    /**
     *  Give credits to the house.
     */
    if (hptr != nullptr) {

        const int amount = taction.TriggerRect.X;
        if (amount >= 0) {
            hptr->Refund_Money(amount);
        } else {
            hptr->Spend_Money(-amount);
        }
    }

    return true;
}


/**
 *  Enables short game.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_ENABLE_SHORT_GAME(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Session.Options.ShortGame = true;

    return true;
}


/**
 *  Disables short game.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_DISABLE_SHORT_GAME(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Session.Options.ShortGame = false;

    return true;
}


/**
 *  Blows up the specified house.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_BLOWUP_HOUSE(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    HouseClass* hptr = HouseClassExtension::House_From_HousesType(taction.Data.House);

    /**
     *  Blow the house up and mark the player as defeated.
     */
    if (hptr != nullptr) {
        hptr->Blowup_All();
        hptr->MPlayer_Defeated();
    }

    return true;
}


/**
 *  Makes all objects attached to the trigger elite.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_MAKE_ELITE(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Iterate all technos, and if their tag is attached to this trigger, make them elite.
     */
    for (int i = 0; i < Technos.Count(); i++) {
        TechnoClass* techno = Technos[i];

        if (techno->IsActive && techno->IsDown && !techno->IsInLimbo) {
            if (techno->Tag && techno->Tag->Is_Trigger_Attached(trig)) {
                techno->Veterancy.Set_Elite(true);
            }
        }
    }

    return true;
}


/**
 *  Enables ally reveal
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_ENABLE_ALLYREVEAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Rule->IsAllyReveal = true;

    return true;
}


/**
 *  Disables ally reveal.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_DISABLE_ALLYREVEAL(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Rule->IsAllyReveal = false;

    return true;
}


/**
 *  Schedules the creation of an autosave the next frame.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_CREATE_AUTOSAVE(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    DEBUG_ERROR("EXT_TACTION_CREATE_AUTOSAVE is not yet implemented, but has been executed!");
    return false;
}


/**
 *  Silently deletes all objects attached to this trigger from the map.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_DELETE_OBJECT(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Iterate all technos, and if their tag is attached to this trigger, flag them for deletion.
     */
    for (int i = 0; i < Technos.Count(); i++) {
        TechnoClass* techno = Technos[i];

        if (techno->IsActive && techno->IsDown && !techno->IsInLimbo) {
            if (techno->Tag && techno->Tag->Is_Trigger_Attached(trig)) {
                techno->Delete_Me();
            }
        }
    }

    return true;
}


/**
 *  Assigns a mission to all units owned by the trigger owner.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_ALL_ASSIGN_MISSION(TActionClass& taction, HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Iterate all units, and if they are owned by the trigger owner, assign the mission.
     */
    for (int i = 0; i < Technos.Count(); i++) {
        TechnoClass* techno = Technos[i];

        if (techno->IsActive && techno->IsDown && !techno->IsInLimbo) {
            if (techno->House == house) {
                techno->Assign_Mission(static_cast<MissionType>(taction.Data.Value));
            }
        }
    }

    return true;
}
