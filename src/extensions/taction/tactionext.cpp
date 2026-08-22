/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Extended TActionClass class.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "tactionext.h"

#include "audio_static_sound.h"
#include "audio_voc.h"
#include "audio_vox.h"
#include "building.h"
#include "debughandler.h"
#include "house.h"
#include "houseext.h"
#include "housetype.h"
#include "mouse.h"
#include "object.h"
#include "reinf.h"
#include "rules.h"
#include "scenario.h"
#include "scenarioext.h"
#include "session.h"
#include "sessionext.h"
#include "tacticalext.h"
#include "taction.h"
#include "tag.h"
#include "tagtype.h"
#include "teamtype.h"
#include "techno.h"
#include "technoext.h"
#include "terrain.h"
#include "tibsun_inline.h"
#include "trigger.h"
#include "triggertype.h"
#include "vinifera_defines.h"
#include "vinifera_globals.h"
#include "voc.h"
#include "mouse.h";


TActionClass::ActionDescriptionStruct TActionClassExtension::ExtActionDescriptions[EXT_TACTION_COUNT - EXT_TACTION_FIRST] = {
    { "Give Credits", "Gives or removes credits from the specified house. A positive amount gives money, a negative amount subtracts it." },
    { "Enable Short Game", "Enables Short Game. Players will lose if all buildings are destroyed." },
    { "Disable Short Game", "Disables Short Game. Players can continue playing even after all buildings are destroyed." },
    { "Create Building At", "Places a building at given waypoint position." },
    { "Destroy all of...", "Kills everything of the specified house and marks them as defeated." },
    { "Make Elite", "All technos attached to this trigger will be promoted to elite status." },
    { "Enable Ally Reveal", "Enables Ally Reveal, allowing allied players to see each other's explored areas." },
    { "Disable Ally Reveal", "Disables Ally Reveal, stopping allied players from seeing each other's explored areas." },
    { "Create Autosave", "Schedules an autosave to be created on the next game frame." },
    { "Delete Attached Objects", "Deletes all units and structures on the map that are linked to this trigger silently." },
    { "All Assign Mission", "Forces all units owned by the trigger's house to begin the specified mission (e.g., hunt, move)." },
    { "Make Ally (One-Way)", "Cause this trigger's house to make a one-sided alliance with the specified house." },
    { "Make Enemy (One-Way)", "Cause this trigger's house to unilaterally declare war on the specified house." },
    { "Modify global (constant)", "Modifies a global variable using a constant as the second operand." },
    { "Modify global (global)", "Modifies a global variable using another global variable as the second operand." },
    { "Modify global (local)", "Modifies a global variable using a local variable as the second operand." },
    { "Increment global", "Increases a global variable by 1." },
    { "Decrement global", "Decreases a global variable by 1." },
    { "Modify local (constant)", "Modifies a local variable using a constant as the second operand." },
    { "Modify local (global)", "Modifies a local variable using a global variable as the second operand." },
    { "Modify local (local)", "Modifies a local variable using another local variable as the second operand." },
    { "Increment local", "Increases a local variable by 1." },
    { "Decrement local", "Decreases a local variable by 1." },
    { "Random number (global)", "Generates a random number and stores it in a global variable." },
    { "Random number (local)", "Generates a random number and stores it in a local variable." },
    { "Print global", "Prints the value of a global variable." },
    { "Print local", "Prints the value of a local variable." },
    { "Enable templated text", "Displays a line of text on the screen with variable substitution. The text may include placeholders like {{g_variableName}} or {{l_variableName}}, which are replaced with the corresponding global or local variable values. Color `-1` uses the color of the player's house." },
    { "Disable templated text", "Removes the currently active templated text from the screen." },
    { "Adjust House Modifier", "Adjusts a house modifier by given percentage points." },
    { "Apply Iron Curtain", "Applies Iron Curtain to attached objects. Can optionally bypass legality checks." },
    { "Stop Sounds At", "Stops sounds at the waypoint that were started by Play Sound At, and detaches any ambient previously attached to a building or terrain there."},
    { "Attach sound", "Attaches an ambient sound to all objects associated with the trigger. The VocType should have Control=LOOP for a continuous attachment; non-looping vocs play once and then go silent." },
    { "Detach sound", "Detaches any ambient sound from all objects associated with the trigger." },
};


/**
 *  Class constructor.
 *
 *  @author: ZivDero
 */
TActionClassExtension::TActionClassExtension(const TActionClass* this_ptr) :
    AbstractClassExtension(this_ptr),
    Text {""}
{
    TActionExtensions.Add(this);
}


/**
 *  Class no-init constructor.
 *
 *  @author: ZivDero
 */
TActionClassExtension::TActionClassExtension(const NoInitClass& noinit) :
    AbstractClassExtension(noinit),
    Text(noinit)
{
}


/**
 *  Class destructor.
 *
 *  @author: ZivDero
 */
TActionClassExtension::~TActionClassExtension()
{
    TActionExtensions.Delete(this);
}


/**
 *  Retrieves the class identifier (CLSID) of the object.
 *
 *  @author: ZivDero
 */
HRESULT TActionClassExtension::GetClassID(CLSID* lpClassID)
{
    if (lpClassID == nullptr) {
        return E_POINTER;
    }

    *lpClassID = __uuidof(this);

    return S_OK;
}


/**
 *  Initializes an object from the stream where it was saved previously.
 *
 *  @author: ZivDero
 */
HRESULT TActionClassExtension::Load(IStream* pStm)
{
    HRESULT hr = AbstractClassExtension::Internal_Load(pStm);
    if (FAILED(hr)) {
        return E_FAIL;
    }

    new (this) TActionClassExtension(NoInitClass());

    return hr;
}


/**
 *  Saves an object to the specified stream.
 *
 *  @author: ZivDero
 */
HRESULT TActionClassExtension::Save(IStream* pStm, BOOL fClearDirty)
{
    HRESULT hr = AbstractClassExtension::Internal_Save(pStm, fClearDirty);
    if (FAILED(hr)) {
        return hr;
    }

    return hr;
}


/**
 *  Return the raw size of class data for save/load purposes.
 *
 *  @author: ZivDero
 */
int TActionClassExtension::Get_Object_Size() const
{
    return sizeof(*this);
}




/**
 *  Compute a unique crc value for this instance.
 *
 *  @author: ZivDero
 */
void TActionClassExtension::Object_CRC(CRCEngine& crc) const
{
}


/**
 *  Returns the name of the TActionType.
 *
 *  @author: CCHyper, ZivDero
 */
const char* TActionClassExtension::Action_Name(int action)
{
    if (action < TACTION_COUNT) {
        return TActionClass::Action_Name(static_cast<TActionType>(action));
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
        return TActionClass::Action_Description(static_cast<TActionType>(action));
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
bool TActionClassExtension::Execute(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    bool success = false;

    /**
     *  Ensure that the specified object is not actually dead. A dead object could
     *  be passed to this routine in the case of a multiple event trigger that
     *  had the first event kill the object.
     */
    if (object != nullptr && !object->IsActive) {
        object = nullptr;
    }

    #define DISPATCH(a) case TACTION_ ## a: success = Do_ ## a (house, object, trig, cell); break;
    #define EXT_DISPATCH(a) case EXT_TACTION_ ## a: success = Do_ ## a (house, object, trig, cell); break;

    // warning C4063: case '#' is not a valid value for switch of enum 'TActionType'
    #pragma warning(push)
    #pragma warning(disable : 4063)

    switch (This()->Action) {

        /**
         *  Intercepted vanilla TActions.
         */
        DISPATCH(PLAY_SPEECH)
        DISPATCH(WIN);
        DISPATCH(LOSE);
        DISPATCH(BEGIN_PRODUCTION);
        DISPATCH(ALL_HUNT);
        DISPATCH(REINFORCEMENTS);
        DISPATCH(FIRE_SALE);
        DISPATCH(TEXT_TRIGGER);
        DISPATCH(DESTROY_TRIGGER);
        DISPATCH(AUTOCREATE);
        DISPATCH(CHANGE_HOUSE);
        DISPATCH(ALL_CHANGE_HOUSE);
        DISPATCH(MAKE_ALLY);
        DISPATCH(MAKE_ENEMY);
        DISPATCH(ENABLE_TRIGGER);
        DISPATCH(DESTROY_TAG);
        DISPATCH(BEGIN_AI_TRIGGERS);
        DISPATCH(STOP_AI_TRIGGERS);
        DISPATCH(PLAY_SOUND_RANDOM);
        DISPATCH(CENTER_VIEWPOINT);
        DISPATCH(REVEAL_SOME);
        DISPATCH(PLAY_SOUND_AT);
        DISPATCH(REINFORCEMENTS_SPECIAL);

        /**
         *  New Vinifera TActions.
         */
        EXT_DISPATCH(GIVE_CREDITS);
        EXT_DISPATCH(ENABLE_SHORT_GAME);
        EXT_DISPATCH(DISABLE_SHORT_GAME);
        EXT_DISPATCH(CREATE_BUILDING_AT);
        EXT_DISPATCH(HOUSE_DESTROY_ALL);
        EXT_DISPATCH(MAKE_ELITE);
        EXT_DISPATCH(ENABLE_ALLYREVEAL);
        EXT_DISPATCH(DISABLE_ALLYREVEAL);
        EXT_DISPATCH(CREATE_AUTOSAVE);
        EXT_DISPATCH(DELETE_OBJECT);
        EXT_DISPATCH(ALL_ASSIGN_MISSION);
        EXT_DISPATCH(MAKE_ALLY_ONE_WAY);
        EXT_DISPATCH(MAKE_ENEMY_ONE_WAY);
        EXT_DISPATCH(MODIFY_GLOBAL_CONSTANT);
        EXT_DISPATCH(MODIFY_GLOBAL_GLOBAL);
        EXT_DISPATCH(MODIFY_GLOBAL_LOCAL);
        EXT_DISPATCH(INCREMENT_GLOBAL);
        EXT_DISPATCH(DECREMENT_GLOBAL);
        EXT_DISPATCH(MODIFY_LOCAL_CONSTANT);
        EXT_DISPATCH(MODIFY_LOCAL_GLOBAL);
        EXT_DISPATCH(MODIFY_LOCAL_LOCAL);
        EXT_DISPATCH(INCREMENT_LOCAL);
        EXT_DISPATCH(DECREMENT_LOCAL);
        EXT_DISPATCH(RANDOM_NUMBER_GLOBAL);
        EXT_DISPATCH(RANDOM_NUMBER_LOCAL);
        EXT_DISPATCH(PRINT_GLOBAL);
        EXT_DISPATCH(PRINT_LOCAL);
        EXT_DISPATCH(ENABLE_TEMPLATED_TEXT);
        EXT_DISPATCH(DISABLE_TEMPLATED_TEXT);
        EXT_DISPATCH(ADJUST_HOUSE_MODIFIER);
        EXT_DISPATCH(APPLY_IRON_CURTAIN);
        EXT_DISPATCH(STOP_SOUNDS_AT);
        EXT_DISPATCH(ATTACH_SOUND);
        EXT_DISPATCH(DETACH_SOUND);

        /**
         *  Unexpected TActionType.
         */
    default:
        DEV_DEBUG_WARNING("Invalid action type ({})!\n", (int)This()->Action);
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
    case TACTION_BEGIN_PRODUCTION:
    case TACTION_ALL_HUNT:
    case TACTION_REINFORCEMENTS:
    case TACTION_FIRE_SALE:
    case TACTION_TEXT_TRIGGER:
    case TACTION_DESTROY_TRIGGER:
    case TACTION_AUTOCREATE:
    case TACTION_CHANGE_HOUSE:
    case TACTION_ALL_CHANGE_HOUSE:
    case TACTION_MAKE_ALLY:
    case TACTION_MAKE_ENEMY:
    case TACTION_ENABLE_TRIGGER:
    case TACTION_DESTROY_TAG:
    case TACTION_BEGIN_AI_TRIGGERS:
    case TACTION_STOP_AI_TRIGGERS:
    case TACTION_PLAY_SOUND_RANDOM:
    case TACTION_CENTER_VIEWPOINT:
    case TACTION_REVEAL_SOME:
    case TACTION_REINFORCEMENTS_SPECIAL:
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
 *  Replaces Do_PLAY_SPEECH because we need to call the new AudioVoxClass::Speak handler.
 *  The old Speak() function is proxied to query the speech by a hardcoded name.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_PLAY_SPEECH(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell const& cell)
{
    AudioVoxClass::Speak(This()->Data.Speech);
    return true;
}


/**
 *  #issue-965
 *
 *  Makes the "Winner is" trigger action set the IsDefeated flag on losing
 *  houses in multiplayer.
 *
 *  @author: Rampastring
 */
bool TActionClassExtension::Do_WIN(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Flag the player as won or lost, like in the original code.
     */
    if (This()->Data.House == PlayerPtr->Class->House) {
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

            if (hptr->Class->House != This()->Data.House) {
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
bool TActionClassExtension::Do_LOSE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Flag the player as won or lost, like in the original code.
     */
    if (This()->Data.House != PlayerPtr->Class->House) {
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

            if (hptr->Class->House == This()->Data.House) {
                hptr->IsDefeated = true;
            }
        }
    }

    return true;
}


/**
 *  Replacement of Do_BEGIN_PRODUCTION to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_BEGIN_PRODUCTION(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Data.House != HOUSE_NONE) {
        HouseClass* hptr = House_From_HousesType(This()->Data.House);
        if (hptr != nullptr) {
            hptr->Begin_Production();
        }
    }
    return true;
}


/**
 *  Replacement of Do_ALL_HUNT to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_ALL_HUNT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Data.House != HOUSE_NONE) {
        HouseClass* hptr = House_From_HousesType(This()->Data.House);
        if (hptr != nullptr) {
            hptr->All_To_Hunt();
        }
    }
    return true;
}


/**
 *  Replacement of Do_REINFORCEMENTS to handle the case when the team has no owner.
 *
 *  @author: Rampastring
 */
bool TActionClassExtension::Do_REINFORCEMENTS(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Team != nullptr && This()->Team->House != nullptr) {
        return Do_Reinforcements(This()->Team);
    }
    return false;
}


/**
 *  Replacement of Do_REINFORCEMENTS_SPECIAL to handle the case when the team has no owner.
 *
 *  @author: Rampastring
 */
bool TActionClassExtension::Do_REINFORCEMENTS_SPECIAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Team != nullptr && This()->Team->House != nullptr && This()->EffectLocation != WAYPOINT_NONE) {
        return Do_Reinforcements(This()->Team, This()->EffectLocation);
    }
    return false;
}


/**
 *  Replacement of Do_FIRE_SALE to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_FIRE_SALE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Data.House != HOUSE_NONE) {
        HouseClass* hptr = House_From_HousesType(This()->Data.House);
        if (hptr != nullptr) {
            hptr->State = STATE_ENDGAME;
        }
    }
    return true;
}


/**
 *  An enhanced text trigger action.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_TEXT_TRIGGER(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (!Vinifera_TutorialText.contains(Text.c_str())) {
        return false;
    }

    /**
     *  Substitute the placeholders in the tutorial string.
     */
    std::string text = ScenarioClassExtension::Substitute_Variable_Placeholders(Vinifera_TutorialText[Text.c_str()]);

    /**
     *  Fetch the requested duration. If it's <= 0, fall back to vanilla.
     */
    int duration = This()->TriggerRect.Y;
    duration = std::max(0, duration);
    if (duration == 0) {
        duration = Rule->MessageDelay * TICKS_PER_MINUTE;
    } else {
        duration *= TIMER_SECOND;
    }

    ColorSchemeType color = static_cast<ColorSchemeType>(This()->TriggerRect.X) * 2;
    if (color < COLORSCHEME_FIRST || color >= ColorSchemes.Count()) {
        color = PlayerPtr->Scheme;
    }

    /**
     *  Display a text message overlayed onto the tactical map.
     */
    Session.Messages.Add_Message(nullptr, 0, text.c_str(), color, TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, duration);
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
bool TActionClassExtension::Do_DESTROY_TRIGGER(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Trigger != nullptr) {
        int count = Triggers.Count();

        for (int index = count - 1; index >= 0; index--) {
            if (Triggers[index]->Class == This()->Trigger) {
                Triggers[index]->Mark_To_Die();
            }
        }
    }
    return true;
}


/**
 *  Replacement of Do_AUTOCREATE to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_AUTOCREATE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Data.House != HOUSE_NONE) {
        HouseClass* hptr = House_From_HousesType(This()->Data.House);
        if (hptr != nullptr) {
            hptr->IsAlerted = true;
        }
    }
    return true;
}


/**
 *  Replacement of Do_CHANGE_HOUSE to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_CHANGE_HOUSE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    bool success = false;
    HouseClass* hptr = House_From_HousesType(This()->Data.House);

    if (hptr != nullptr) {
        for (int index = 0; index < Technos.Count(); index++) {
            TechnoClass* techno = Technos[index];
            if (techno->IsActive && techno->IsDown && !techno->IsInLimbo && techno->Tag != nullptr && techno->Tag->Is_Trigger_Attached(trig)) {

                techno->Captured(hptr);
                success = true;
            }
        }
    }
    return success;
}


/**
 *  Replacement of Do_ALL_CHANGE_HOUSE to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_ALL_CHANGE_HOUSE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    bool retval = false;

    HouseClass* hptr = House_From_HousesType(This()->Data.House);

    if (hptr != nullptr) {
        for (int index = 0; index < Technos.Count(); index++) {
            if (Technos[index]->House == house) {
                Technos[index]->Captured(hptr);
                retval = true;
            }
        }
    }
    return retval;
}


/**
 *  Replacement of Do_MAKE_ALLY to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_MAKE_ALLY(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (house != nullptr) {
        if (This()->Data.House != HOUSE_NONE) {
            HouseClass* house2 = House_From_HousesType(This()->Data.House);
            if (house2 != nullptr) {
                house->Make_Ally(house2);
                house2->Make_Ally(house);
            }
        }
    }
    return true;
}


/**
 *  Replacement of Do_MAKE_ENEMY to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_MAKE_ENEMY(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (house != nullptr) {
        if (This()->Data.House != HOUSE_NONE) {
            HouseClass* house2 = House_From_HousesType(This()->Data.House);
            if (house2 != nullptr) {
                house->Make_Enemy(house2);
                house2->Make_Enemy(house);
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
bool TActionClassExtension::Do_ENABLE_TRIGGER(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  This is direct port of the code from Red Alert 2, which looks to fix this issue.
     */
    if (This()->Trigger != nullptr) {
        for (int index = 0; index < Triggers.Count(); index++) {
            if (Triggers[index]->Class == This()->Trigger) {

                /**
                 *  #issue-1608
                 *
                 *  Bugfix: if the trigger is already enabled, there's nothing to do here.
                 *
                 *  @author: Rampastring
                 */
                if (Triggers[index]->Is_Enabled()) {
                    continue;
                }

                bool really_enable = true;

                /**
                 *  Set this trigger to be disabled if it is marked as disabled
                 *  for this current mission difficulty.
                 */
                if (Session.Type == GAME_NORMAL) {
                    if (Scen->CDifficulty == DIFF_HARD && !Triggers[index]->Class->IsEnabledEasy) {
                        really_enable = false;
                    } else if (Scen->CDifficulty == DIFF_NORMAL && !Triggers[index]->Class->IsEnabledMedium) {
                        really_enable = false;
                    } else if (Scen->CDifficulty == DIFF_EASY && !Triggers[index]->Class->IsEnabledHard) {
                        really_enable = false;
                    }
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
 *  Fixes a bug where caching Tags.Count() before the loop
 *  could cause out-of-bounds access after deletions
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_DESTROY_TAG(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Tag != nullptr) {
        for (int index = 0; index < Tags.Count(); index++) {
            if (Tags[index]->Class == This()->Tag) {
                delete Tags[index];
                index--;
            }
        }
    }
    return true;
}


/**
 *  Replacement of Do_BEGIN_AI_TRIGGERS to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_BEGIN_AI_TRIGGERS(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Data.House != HOUSE_NONE) {
        HouseClass* hptr = House_From_HousesType(This()->Data.House);
        if (hptr != nullptr) {
            hptr->IsAITriggersOn = true;
        }
    }
    return true;
}


/**
 *  Replacement of Do_STOP_AI_TRIGGERS to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_STOP_AI_TRIGGERS(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Data.House != HOUSE_NONE) {
        HouseClass* hptr = House_From_HousesType(This()->Data.House);
        if (hptr != nullptr) {
            hptr->IsAITriggersOn = false;
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
bool TActionClassExtension::Do_PLAY_SOUND_RANDOM(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
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
    Static_Sound(This()->Data.Sound, list[Random_Pick(0u, std::size(list) - 1)].As_Coord());
    return true;
}


/**
 *  Gives credits to the house specified as the argument.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_GIVE_CREDITS(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    HouseClass* hptr = HouseClassExtension::House_From_HousesType(This()->Data.House);

    /**
     *  Give credits to the house.
     */
    if (hptr != nullptr) {

        const int amount = This()->TriggerRect.X;
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
bool TActionClassExtension::Do_ENABLE_SHORT_GAME(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Session.Options.ShortGame = true;

    return true;
}


/**
 *  Disables short game.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_DISABLE_SHORT_GAME(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Session.Options.ShortGame = false;

    return true;
}


/**
 *  Places a building at given waypoint position.
 *
 *  @author: Rampastring
 */
bool TActionClassExtension::Do_CREATE_BUILDING_AT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Cell wpcell = ScenExtension->Waypoint_Cell(This()->EffectLocation);

    if (wpcell != CELL_NONE) {
        HouseClass* hptr = HouseClassExtension::House_From_HousesType(This()->Data.House);

        int buildingtypeid = This()->TriggerRect.X;
        bool forced = This()->TriggerRect.Y > 0;
        BuildingTypeClass* btc = BuildingTypes[buildingtypeid];

        bool success = false;

        if (forced) {
            ScenarioInit++;
            success = btc->Create_And_Place(wpcell, hptr);
            ScenarioInit--;
        } else {
            // Create_And_Place does not play buildup anim
            BuildingClass* building = new BuildingClass(btc, hptr);

            if (building != nullptr) {
                building->Assign_Mission(MISSION_CONSTRUCTION);
                success = building->Unlimbo(wpcell.As_Coord());

                if (!success) {
                    delete building;
                } else {
                    building->Revealed(hptr);
                    building->IsReadyToCommence = true;
                }
            }
        }

        return success;
    }

    return false;
}


/**
 *  Blows up the specified house.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_HOUSE_DESTROY_ALL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    HouseClass* hptr = HouseClassExtension::House_From_HousesType(This()->Data.House);

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
bool TActionClassExtension::Do_MAKE_ELITE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Iterate all technos, and if their tag is attached to this trigger, make them elite.
     */
    for (int i = 0; i < Technos.Count(); i++) {
        TechnoClass* techno = Technos[i];

        if (techno->IsActive && techno->IsDown && !techno->IsInLimbo) {
            if (techno->Tag && techno->Tag->Is_Trigger_Attached(trig)) {
                techno->Crew.Set_Elite(true);
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
bool TActionClassExtension::Do_ENABLE_ALLYREVEAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Rule->IsAllyReveal = true;

    return true;
}


/**
 *  Disables ally reveal.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_DISABLE_ALLYREVEAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    Rule->IsAllyReveal = false;

    return true;
}


/**
 *  Schedules the creation of an autosave the next frame.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExtension::Do_CREATE_AUTOSAVE(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Schedule a save.
     */
    SessionExtension->Flag_To_Save(false);

    return true;
}


/**
 *  Silently deletes all objects attached to this trigger from the map.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_DELETE_OBJECT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
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
bool TActionClassExtension::Do_ALL_ASSIGN_MISSION(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Iterate all units, and if they are owned by the trigger owner, assign the mission.
     */
    for (int i = 0; i < Technos.Count(); i++) {
        TechnoClass* techno = Technos[i];

        if (techno->IsActive && techno->IsDown && !techno->IsInLimbo) {
            if (techno->House == house) {
                techno->Assign_Mission(static_cast<MissionType>(This()->Data.Value));
            }
        }
    }

    return true;
}


/**
 *  Cause this trigger's house to make a one-sided alliance with the specified house.
 *
 *  @author: ZivDero, based on ts-patches implementation by Rampastring
 */
bool TActionClassExtension::Do_MAKE_ALLY_ONE_WAY(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Data.House != HOUSE_NONE) {
        HouseClass* house2 = HouseClassExtension::House_From_HousesType(This()->Data.House);

        /**
         *  We need to increment ScenarioInit to allow houses to ally even if
         *  they would be the "last enemies" to each other in multiplayer
         */
        ScenarioInit++;
        house->Make_Ally(house2);
        ScenarioInit--;
    }
    return true;
}


/**
 *  Cause this trigger's house to unilaterally declare war on the specified house.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_MAKE_ENEMY_ONE_WAY(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    if (This()->Data.House != HOUSE_NONE) {
        HouseClass* house2 = HouseClassExtension::House_From_HousesType(This()->Data.House);

        /**
         *  We need to increment ScenarioInit to allow houses to ally even if
         *  they would be the "last enemies" to each other in multiplayer
         */
        ScenarioInit++;
        house->Make_Enemy(house2);
        ScenarioInit--;
    }
    return true;
}


/**
 *  An enum for the operations that the actions can perform.
 */
enum VariableOperation
{
    OP_ASSIGN,
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE,
    OP_MODULO,
    OP_NEGATE,
    OP_LSHIFT,
    OP_RSHIFT,
    OP_NOT,
    OP_XOR,
    OP_OR,
    OP_AND,
    OP_MAX,
    OP_MIN
};


/**
 *  Performs an operation.
 */
static int Operate(int lhs, int rhs, VariableOperation operation)
{
    switch (operation) {
    case OP_ASSIGN:
        lhs = rhs;
        break;
    case OP_ADD:
        lhs += rhs;
        break;
    case OP_SUBTRACT:
        lhs -= rhs;
        break;
    case OP_MULTIPLY:
        lhs *= rhs;
        break;
    case OP_DIVIDE:
        lhs /= rhs;
        break;
    case OP_MODULO:
        lhs %= rhs;
        break;
    case OP_NEGATE:
        lhs = -lhs;
        break;
    case OP_LSHIFT:
        lhs <<= rhs;
        break;
    case OP_RSHIFT:
        lhs >>= rhs;
        break;
    case OP_NOT:
        lhs = ~lhs;
        break;
    case OP_XOR:
        lhs ^= rhs;
        break;
    case OP_OR:
        lhs |= rhs;
        break;
    case OP_AND:
        lhs &= rhs;
        break;
    case OP_MAX:
        lhs = std::max(lhs, rhs);
        break;
    case OP_MIN:
        lhs = std::min(lhs, rhs);
        break;
    }
    return lhs;
}


/**
 *  Edits a global with a constant as the second operand.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_MODIFY_GLOBAL_CONSTANT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int left_index = This()->Data.Value;
    VariableOperation operation = static_cast<VariableOperation>(This()->TriggerRect.X);
    int right = This()->TriggerRect.Y;

    /**
     *  Fetch the current value of the variable.
     */
    int left;
    if (!ScenExtension->Get_Global_Value(left_index, left)) {
        return false;
    }

    /**
     *  Perform the requested operation.
     */
    left = Operate(left, right, operation);

    /**
     *  Save the result.
     */
    ScenExtension->Set_Global_To(left_index, left);

    return true;
}


/**
 *  Edits a global with another global as the second operand.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_MODIFY_GLOBAL_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int left_index = This()->Data.Value;
    VariableOperation operation = static_cast<VariableOperation>(This()->TriggerRect.X);
    int right_index = This()->TriggerRect.Y;

    /**
     *  Fetch the current value of the variable.
     */
    int left;
    if (!ScenExtension->Get_Global_Value(left_index, left)) {
        return false;
    }

    /**
     *  Fetch the current value of the second variable.
     */
    int right;
    if (!ScenExtension->Get_Global_Value(right_index, right)) {
        return false;
    }

    /**
     *  Perform the requested operation.
     */
    left = Operate(left, right, operation);

    /**
     *  Save the result.
     */
    ScenExtension->Set_Global_To(left_index, left);

    return true;
}


/**
 *  Edits a global with a local as the second operand.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_MODIFY_GLOBAL_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int left_index = This()->Data.Value;
    VariableOperation operation = static_cast<VariableOperation>(This()->TriggerRect.X);
    int right_index = This()->TriggerRect.Y;

    /**
     *  Fetch the current value of the variable.
     */
    int left;
    if (!ScenExtension->Get_Global_Value(left_index, left)) {
        return false;
    }

    /**
     *  Fetch the current value of the second variable.
     */
    int right;
    if (!ScenExtension->Get_Local_Value(right_index, right)) {
        return false;
    }

    /**
     *  Perform the requested operation.
     */
    left = Operate(left, right, operation);

    /**
     *  Save the result.
     */
    ScenExtension->Set_Global_To(left_index, left);

    return true;
}


/**
 *  Increments a global.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_INCREMENT_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int index = This()->Data.Value;

    /**
     *  Fetch the current value of the variable.
     */
    int value;
    if (!ScenExtension->Get_Global_Value(index, value)) {
        return false;
    }

    /**
     *  Perform the requested operation.
     */
    value++;

    /**
     *  Save the result.
     */
    ScenExtension->Set_Global_To(index, value);

    return true;
}


/**
 *  Decrements a global.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_DECREMENT_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int index = This()->Data.Value;

    /**
     *  Fetch the current value of the variable.
     */
    int value;
    if (!ScenExtension->Get_Global_Value(index, value)) {
        return false;
    }

    /**
     *  Perform the requested operation.
     */
    value--;

    /**
     *  Save the result.
     */
    ScenExtension->Set_Global_To(index, value);

    return true;
}


/**
 *  Edits a local with a constant as the second operand.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_MODIFY_LOCAL_CONSTANT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int left_index = This()->Data.Value;
    VariableOperation operation = static_cast<VariableOperation>(This()->TriggerRect.X);
    int right = This()->TriggerRect.Y;

    /**
     *  Fetch the current value of the variable.
     */
    int left;
    if (!ScenExtension->Get_Local_Value(left_index, left)) {
        return false;
    }

    /**
     *  Perform the requested operation.
     */
    left = Operate(left, right, operation);

    /**
     *  Save the result.
     */
    ScenExtension->Set_Local_To(left_index, left);

    return true;
}


/**
 *  Edits a local with a global as the second operand.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_MODIFY_LOCAL_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int left_index = This()->Data.Value;
    VariableOperation operation = static_cast<VariableOperation>(This()->TriggerRect.X);
    int right_index = This()->TriggerRect.Y;

    /**
     *  Fetch the current value of the variable.
     */
    int left;
    if (!ScenExtension->Get_Local_Value(left_index, left)) {
        return false;
    }

    /**
     *  Fetch the current value of the second variable.
     */
    int right;
    if (!ScenExtension->Get_Global_Value(right_index, right)) {
        return false;
    }

    /**
     *  Perform the requested operation.
     */
    left = Operate(left, right, operation);

    /**
     *  Save the result.
     */
    ScenExtension->Set_Local_To(left_index, left);

    return true;
}


/**
 *  Edits a local with another local as the second operand.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_MODIFY_LOCAL_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int left_index = This()->Data.Value;
    VariableOperation operation = static_cast<VariableOperation>(This()->TriggerRect.X);
    int right_index = This()->TriggerRect.Y;

    /**
     *  Fetch the current value of the variable.
     */
    int left;
    if (!ScenExtension->Get_Local_Value(left_index, left)) {
        return false;
    }

    /**
     *  Fetch the current value of the second variable.
     */
    int right;
    if (!ScenExtension->Get_Local_Value(right_index, right)) {
        return false;
    }

    /**
     *  Perform the requested operation.
     */
    left = Operate(left, right, operation);

    /**
     *  Save the result.
     */
    ScenExtension->Set_Local_To(left_index, left);

    return true;
}


/**
 *  Increments a local.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_INCREMENT_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int index = This()->Data.Value;

    /**
     *  Fetch the current value of the variable.
     */
    int value;
    if (!ScenExtension->Get_Local_Value(index, value)) {
        return false;
    }

    /**
     *  Perform the requested operation.
     */
    value++;

    /**
     *  Save the result.
     */
    ScenExtension->Set_Local_To(index, value);

    return true;
}


/**
 *  Decrements a local.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_DECREMENT_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int index = This()->Data.Value;

    /**
     *  Fetch the current value of the variable.
     */
    int value;
    if (!ScenExtension->Get_Local_Value(index, value)) {
        return false;
    }

    /**
     *  Perform the requested operation.
     */
    value--;

    /**
     *  Save the result.
     */
    ScenExtension->Set_Local_To(index, value);

    return true;
}


/**
 *  Generates a random number and stores it in a global.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_RANDOM_NUMBER_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int index = This()->Data.Value;
    int min = This()->TriggerRect.X;
    int max = This()->TriggerRect.Y;

    /**
     *  Generate the number.
     */
    int number = Random_Pick(min, max);

    /**
     *  Save the result.
     */
    if (!ScenExtension->Set_Global_To(index, number)) {
        return false;
    }

    return true;
}


/**
 *  Generates a random number and stores it in a local.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_RANDOM_NUMBER_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int index = This()->Data.Value;
    int min = This()->TriggerRect.X;
    int max = This()->TriggerRect.Y;

    /**
     *  Generate the number.
     */
    int number = Random_Pick(min, max);

    /**
     *  Save the result.
     */
    if (!ScenExtension->Set_Local_To(index, number)) {
        return false;
    }

    return true;
}


/**
 *  Prints the global as a message.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_PRINT_GLOBAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int index = This()->Data.Value;

    /**
     *  Fetch the current value of the variable.
     */
    int value;
    if (!ScenExtension->Get_Global_Value(index, value)) {
        return false;
    }

    /**
     *  Format the value into a string.
     */
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%d", value);

    /**
     *  Display a text message overlaid onto the tactical map.
     */
    Session.Messages.Add_Message(nullptr, 0, buffer, COLORSCHEME_FIRST, TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, Rule->MessageDelay * TICKS_PER_MINUTE);
    return true;
}


/**
 *  Prints the local as a message.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_PRINT_LOCAL(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     *  Save the parameters for convenience.
     */
    int index = This()->Data.Value;

    /**
     *  Fetch the current value of the variable.
     */
    int value;
    if (!ScenExtension->Get_Local_Value(index, value)) {
        return false;
    }

    /**
     *  Format the value into a string.
     */
    char buffer[32];
    std::snprintf(buffer, sizeof(buffer), "%d", value);

    /**
     *  Display a text message overlaid onto the tactical map.
     */
    Session.Messages.Add_Message(nullptr, 0, buffer, COLORSCHEME_FIRST, TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, Rule->MessageDelay * TICKS_PER_MINUTE);
    return true;
}


/**
 *  Enables the variable counter for a global variable.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_ENABLE_TEMPLATED_TEXT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    TacticalMapExtension->Enable_Templated_Text(Text, static_cast<ColorSchemeType>(This()->TriggerRect.X * 2));
    return true;
}


/**
 *  Disables the variable counter.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_DISABLE_TEMPLATED_TEXT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    TacticalMapExtension->Disable_Templated_Text();
    return true;
}


/**
 *  Adjusts a house modifier.
 *
 *  @author: Rampastring
 */
bool TActionClassExtension::Do_ADJUST_HOUSE_MODIFIER(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    int amount = This()->TriggerRect.X;

    switch (This()->Data.Value)
    {
    case 0:
        house->FirepowerBias += (double)amount / 100.0;
        break;
    case 1:
        house->ArmorBias += (double)amount / 100.0;
        break;
    case 2:
        house->GroundspeedBias += (double)amount / 100.0;
        break;
    case 3:
        house->AirspeedBias += (double)amount / 100.0;
        break;
    case 4:
        house->ROFBias += (double)amount / 100.0;
        break;
    case 5:
        house->CostBias += (double)amount / 100.0;
        break;
    case 6:
        house->BuildSpeedBias += (double)amount / 100.0;
        break;
    }

    return true;
}


/**
 *  Applies the Iron Curtain to attached objects.
 *
 *  @author: Rampastring
 */
bool TActionClassExtension::Do_APPLY_IRON_CURTAIN(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    HouseClassExtension* houseext = Extension::Fetch(house);

    // Check for legality, unless this is forced.
    bool forced = This()->Data.Bool;
    if (!forced) {
        if (!houseext->Can_Use_Iron_Curtain()) {
            // If the application is not forced and the house is unable to use the Iron Curtain, skip.
            return true;
        }
    }

    /**
     *  Iterate all technos, and if their tag is attached to this trigger, apply Iron Curtain on them.
     */
    for (int i = 0; i < Technos.Count(); i++) {
        TechnoClass* techno = Technos[i];

        if (techno->IsActive && techno->IsDown && !techno->IsInLimbo) {
            if (techno->Tag && techno->Tag->Is_Trigger_Attached(trig)) {
                TechnoClassExtension* technoext = Extension::Fetch(techno);
                technoext->Iron_Curtain_Me(true);
            }
        }
    }

    houseext->Expend_Iron_Curtain();
    return true;
}

/**
 *  Reimplementation of the trigger action for centering the camera at the desired waypoint.
 *  Enhanced to allow using a negative speed value for an instant snap of the camera to the waypoint, rather than a slow scroll to it.
 *
 *  @author: JoyfulShush
 */
bool TActionClassExtension::Do_CENTER_VIEWPOINT(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    /**
     * Represents valid speeds that are supported by the game
     * -1 = Instant (also supports any negative value)
     * 0 = Very Slow
     * 1 = Slow
     * 2 = Medium
     * 3 = Fast
     * 4 = Very Fast
     */
    constexpr int SCROLL_SPEED_COUNT = 5;

    /**
     * Disallow invalid speeds that could result in reading OOB addresses
     * Speed can be any negative value, or up to 4
     */
    if (This()->Data.Speed > SCROLL_SPEED_COUNT - 1) {
        return false;
    }

    Cell waypt = Scen->Waypoint_Cell(This()->EffectLocation);
    Coord coord = Coord(waypt);

    coord.Z = Map.Get_Height_GL(coord);

    if (Map[waypt].IsUnderBridge || Map[waypt].WasUnderBridge) {
        coord.Z += BRIDGE_LEPTON_HEIGHT;
    }

    if (This()->Data.Speed <= -1) {
        TacticalMap->Set_Tactical_Position(coord);
        return true;
    }

    TacticalMap->Setup_Trigger_Scroll(coord, This()->Data.Speed);
    return true;
}

/**
 *  Reimplements Reveal Around Waypoint trigger action.
 *  This reimplementation allows accepting a second optional value which defines the radius of the reveal.
 *  If this value is 0 or negative, falls back to the original radius through `RevealTriggerRadius` from Rules.
 *
 *  @author: JoyfulShush
 */
bool TActionClassExtension::Do_REVEAL_SOME(HouseClass*, ObjectClass*, TriggerClass*, Cell const&)
{
    if (!PlayerPtr->IsVisionary) {
        Cell waypoint_cell = Scen->Waypoint_Cell(This()->Data.Value);
        
        const auto& cell = Map[waypoint_cell];
        int height = cell.Height;

        if (cell.IsUnderBridge || cell.WasUnderBridge) {
            height += BRIDGE_CELL_HEIGHT;
        }

        /* Allows for a custom reveal radius for the trigger */
        int radius = This()->TriggerRect.X; // P3 value
        if (radius <= 0) {
            radius = Rule->RevealTriggerRadius;
        }

        /*
        *  Requires 'RevealByHeight=yes' (default) to consider elevation, even when set to true, since it is checked in 'Map.Sight_From'
        *  The value is default 0 for true in order to be backwards compatible
        */        
        int consider_elevation = This()->TriggerRect.Y == 0 ? true : false; // P4 value

        Map.Sight_From(Coord(waypoint_cell - Cell(height / 2, height / 2)) + Coord(0, 0, height * LEVEL_LEPTON_H), radius, PlayerPtr, false, false, false, consider_elevation);
    }
    return true;
}



/**
 *  Fetches the object to attach a sound to at a coordinate.
 *
 *  @author: ZivDero
 */
static ObjectClass* Get_Audio_Object(const Coord& coord)
{
    Cell cell = coord.As_Cell();
    ObjectClass* object = nullptr;

    if (cell != CELL_NONE) {
        CellClass* cellptr = &Map[cell];
        object = cellptr->Cell_Building();
        if (!object) {
            object = cellptr->Cell_Terrain();
        }
    }

    return object;
}


/**
 *  Plays a sound at the coordinate, or attaches it to an object there.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_PLAY_SOUND_AT(HouseClass*, ObjectClass*, TriggerClass*, Cell const&)
{
    VocType sound = This()->Data.Sound;
    if (sound < VOC_FIRST || sound >= AudioVocs.Count()) {
        return false;
    }

    Coord coord = Scen->Waypoint_Coord(This()->EffectLocation);
    ObjectClass* object = Get_Audio_Object(coord);

    if (object) {
        Extension::Fetch(object)->Attach_Ambient(sound);
    } else {
        Play_Tracked_Static_Sound(sound, coord);
    }
    return true;
}


/**
 *  Plays playing the sound at the coordinate, or detaches it from the object there.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_STOP_SOUNDS_AT(HouseClass*, ObjectClass*, TriggerClass*, Cell const&)
{
    Coord coord = Scen->Waypoint_Coord(This()->EffectLocation);
    ObjectClass* object;

    object = Get_Audio_Object(coord);

    if (object) {
        Extension::Fetch(object)->Attach_Ambient(VOC_NONE);
    } else {
        Stop_Tracked_Static_Sounds_At(coord);
    }
    return true;
}


/**
 *  Attaches an ambient sound to all objects associated with the trigger.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_ATTACH_SOUND(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    VocType sound = This()->Data.Sound;
    if (sound < VOC_FIRST || sound >= AudioVocs.Count()) {
        return false;
    }

    bool success = false;

    for (int index = 0; index < Objects.Count(); index++) {
        ObjectClass* obj = Objects[index];
        if (obj->IsActive && obj->IsDown && !obj->IsInLimbo && obj->Tag != nullptr && obj->Tag->Is_Trigger_Attached(trig)) {
            auto extension = Extension::Fetch(obj);
            if (extension != nullptr) {
                extension->Attach_Ambient(sound);
                success = true;
            }
        }
    }
    return success;
}


/**
 *  Attaches any ambient sounds from all objects associated with the trigger.
 *
 *  @author: ZivDero
 */
bool TActionClassExtension::Do_DETACH_SOUND(HouseClass* house, ObjectClass* object, TriggerClass* trig, const Cell& cell)
{
    bool success = false;

    for (int index = 0; index < Objects.Count(); index++) {
        ObjectClass* obj = Objects[index];
        if (obj->IsActive && obj->IsDown && !obj->IsInLimbo && obj->Tag != nullptr && obj->Tag->Is_Trigger_Attached(trig)) {
            auto extension = Extension::Fetch(obj);
            if (extension != nullptr) {
                extension->Attach_Ambient(VOC_NONE);
                success = true;
            }
        }
    }
    return success;
}
