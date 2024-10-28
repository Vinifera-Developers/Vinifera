/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          TACTIONEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended TriggerClass.
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
#include "tactionext_hooks.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "trigger.h"
#include "triggertype.h"
#include "taction.h"
#include "scenario.h"
#include "scenarioext.h"
#include "voc.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "house.h"
#include "housetype.h"
#include "session.h"
#include "object.h"
#include "tagtype.h"
#include "rules.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "options.h"
#include "tag.h"
#include "techno.h"
#include "vinifera_globals.h"


/**
 *  Helper info for writing new actions.
 * 
 *  TActionClass::Data                  = First Param (PARAM1)
 *  TActionClass::Bounds.X              = Second Param (PARAM2)
 *  TActionClass::Bounds.Y              = Third Param (PARAM3)
 *  TActionClass::Bounds.W              = Fourth Param (PARAM4)
 *  TActionClass::Bounds.H              = Fifth Param (PARAM5)
 * 
 *  (PARAM6) (OPTIONAL)
 *  if TActionFormatType == 4
 *    TActionClass::Data (overwrites PARAM1)
 *  else
 *    TActionClass::Location
 * 
 *  
 *  Example action line from a scenario file;
 * 
 *  [Actions]
 *  NAME = [Action Count], [TActionType], [TActionFormatType], [PARAM1], [PARAM2], [PARAM3], [PARAM4], [PARAM5], [PARAM6:OPTIONAL]
 *  
 *  To allow the use of TActionClass::Data (PARAM1), you must have the TActionFormatType set
 *  to "0", otherwise this param is ignored!
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
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
class TActionClassExt final : public TActionClass
{
public:
    enum {   
        TACTION_GIVE_CREDITS = TACTION_COUNT,
        TACTION_ENABLE_SHORT_GAME,
        TACTION_DISABLE_SHORT_GAME,
        TACTION_PRINT_DIFFICULTY,
        TACTION_BLOWUP_HOUSE,
        TACTION_MAKE_ELITE,
        TACTION_ENABLE_ALLYREVEAL,
        TACTION_DISABLE_ALLYREVEAL,
        TACTION_CREATE_AUTOSAVE,
        TACTION_DELETE_OBJECT,
        TACTION_ALL_ASSIGN_MISSION,

        TACTION_NEW_COUNT
    };

public:
    bool _Function_Call_Operator(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);

    bool _TAction_Play_Sound_At_Random_Waypoint(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Destroy_Trigger(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Enable_Trigger(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Win(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Lose(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Change_House(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_All_Change_House(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Make_Ally(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Make_Enemy(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Begin_Production(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Fire_Sale(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Begin_Autocreate(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_All_Hunt(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Set_AI_Triggers_Begin(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Set_AI_Triggers_End(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);

    bool _TAction_Give_Credits(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Enable_Short_Game(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Disable_Short_Game(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Print_Difficulty(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Blowup_House(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Make_Elite(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Enable_AllyReveal(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Disable_AllyReveal(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Create_AutoSave(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_Delete_Object(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
    bool _TAction_All_Assign_Mission(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell);
};


bool TActionClassExt::_Function_Call_Operator(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    /**
     *  Take an appropriate action.
     */
    bool success = true;

    /**
     *  Ensure that the specified object is not actually dead. A dead object could
     *  be passed to this routine in the case of a multiple event trigger that
     *  had the first event kill the object.
     */
    if (object && !object->IsActive) {
        object = nullptr;
    }

    switch (Action) {

        /**
         *  Flag the house specified as the winner. Really the house value
         *  is only used to determine if it is the player or the computer.
         */
    case TACTION_WIN:
        success = TAction_Win(house, object, trig, cell);
        break;

        /**
         *  Flag the house specified as the loser. The house parameter is only
         *  used to determine if it refers to the player or the computer.
         */
    case TACTION_LOSE:
        success = TAction_Lose(house, object, trig, cell);
        break;

        /**
         *  This will enable production to begin for the house specified.
         */
    case TACTION_BEGIN_PRODUCTION:
        success = TAction_Begin_Production(house, object, trig, cell);
        break;

        /**
         *  Manually create the team specified.
         */
    case TACTION_CREATE_TEAM:
        success = TAction_Create_Team(house, object, trig, cell);
        break;

        /**
         *  Destroy all teams of the type specified.
         */
    case TACTION_DESTROY_TEAM:
        success = TAction_Destroy_Team(house, object, trig, cell);
        break;

        /**
         *  Force all units of the house specified to go into
         *  hunt mode.
         */
    case TACTION_ALL_HUNT:
        success = TAction_All_Hunt(house, object, trig, cell);
        break;

        /**
         *  Create a reinforcement of the team specified.
         */
    case TACTION_REINFORCEMENTS:
        success = TAction_Reinforcements(house, object, trig, cell);
        break;

        /**
         *  Place a smoke marker at the waypoint specified.
         */
    case TACTION_DZ:
        success = TAction_Drop_Zone_Flare(house, object, trig, cell);
        break;

        /**
         *  Cause all buildings to be sold and all units to go into
         *  hunt mode.
         */
    case TACTION_FIRE_SALE:
        success = TAction_Fire_Sale(house, object, trig, cell);
        break;

        /**
         *  Play a movie immediately. The game is temporarily
         *  suspended while the movie plays.
         */
    case TACTION_PLAY_MOVIE:
        success = TAction_Play_Movie(house, object, trig, cell);
        break;

        /**
         *  Display a text message overlayed onto the tactical map.
         */
    case TACTION_TEXT_TRIGGER:
        success = TAction_Text_Trigger(house, object, trig, cell);
        break;

        /**
         *  Destroying a trigger means that all triggers of that type will be destroyed.
         */
    case TACTION_DESTROY_TRIGGER:
        success = TAction_Destroy_Trigger(house, object, trig, cell);
        break;

        /**
         *  Begin the team autocreate logic for the house specified.
         */
    case TACTION_AUTOCREATE:
        success = TAction_Begin_Autocreate(house, object, trig, cell);;
        break;

        /**
         *  Change the house of the attached object.
         */
    case TACTION_CHANGE_HOUSE:
        success = TAction_Change_House(house, object, trig, cell);
        break;

        /**
         *  Reveal the entire map.
         */
    case TACTION_REVEAL_ALL:
        success = TAction_Reveal_Map(house, object, trig, cell);
        break;

        /**
         *  Reveal the map around the area specified.
         */
    case TACTION_REVEAL_SOME:
        success = TAction_Reveal_Area(house, object, trig, cell);
        break;

        /**
         *  Reveal all cells of the zone that the specified waypoint is located
         *  in. This can be used to reveal whole islands or bodies of water
         */
    case TACTION_REVEAL_ZONE:
        success = TAction_Reveal_Zone(house, object, trig, cell);
        break;

        /**
         *  Play a sound effect.
         */
    case TACTION_PLAY_SOUND:
        success = TAction_Play_Sound(house, object, trig, cell);
        break;

        /**
         *  Play a musical theme.
         */
    case TACTION_PLAY_MUSIC:
        success = TAction_Play_Music(house, object, trig, cell);
        break;

        /**
         *  Play the speech data specified.
         */
    case TACTION_PLAY_SPEECH:
        success = TAction_Play_Speech(house, object, trig, cell);
        break;

        /**
         *  A forced trigger will force an existing trigger of that type or
         *  will create a trigger of that type and then force it to be sprung.
         */
    case TACTION_FORCE_TRIGGER:
        success = TAction_Force_Trigger(house, object, trig, cell);
        break;

        /**
         *  Star the mission timer.
         */
    case TACTION_START_TIMER:
        success = TAction_Start_Timer(house, object, trig, cell);
        break;

        /**
         *  Stop the mission timer. This will really just
         *  suspend the timer.
         */
    case TACTION_STOP_TIMER:
        success = TAction_Stop_Timer(house, object, trig, cell);
        break;

        /**
         *  Add time to the mission timer.
         */
    case TACTION_ADD_TIMER:
        success = TAction_Add_Timer(house, object, trig, cell);
        break;

        /**
         *  Remove time from the mission timer.
         */
    case TACTION_SUB_TIMER:
        success = TAction_Sub_Timer(house, object, trig, cell);
        break;

        /**
         *  Set the mission timer to the value specified.
         */
    case TACTION_SET_TIMER:
        success = TAction_Set_Timer(house, object, trig, cell);
        break;

        /**
         *  Set a scenario global.
         */
    case TACTION_SET_GLOBAL:
        success = TAction_Global_Set(house, object, trig, cell);
        break;

        /**
         *  Clear a scenario global.
         */
    case TACTION_GLOBAL_CLEAR:
        success = TAction_Global_Clear(house, object, trig, cell);
        break;

        /**
         *  Initiate (or disable) the computer AI. When active, the computer will
         *  build bases and units.
         */
    case TACTION_BASE_BUILDING:
        success = TAction_Base_Building(house, object, trig, cell);
        break;

        /**
         *  Cause the shadow to creep back one step.
         */
    case TACTION_CREEP_SHADOW:
        success = TAction_Creep_Shadow(house, object, trig, cell);
        break;

        /**
          *  This will destroy all objects that this trigger is
          *  attached to.
          */
    case TACTION_DESTROY_OBJECT:
        success = TAction_Destroy_Object(house, object, trig, cell);
        break;

        /**
         *  Give a one-time special weapon to the house.
         */
    case TACTION_1_SPECIAL:
        success = TAction_One_Time_Special(house, object, trig, cell);
        break;

        /**
         *  Give the special weapon to the house.
         */
    case TACTION_FULL_SPECIAL:
        success = TAction_Full_Special(house, object, trig, cell);
        break;

        /**
         *  Set the preferred target for the house.
         */
    case TACTION_PREFERRED_TARGET:
        success = TAction_Preferred_Target(house, object, trig, cell);
        break;

    case TACTION_ALL_CHANGE_HOUSE:
        success = TAction_All_Change_House(house, object, trig, cell);
        break;


    case TACTION_MAKE_ALLY:
        success = TAction_Make_Ally(house, object, trig, cell);
        break;


    case TACTION_MAKE_ENEMY:
        success = TAction_Make_Enemy(house, object, trig, cell);
        break;


    case TACTION_CHANGE_ZOOM_LEVEL:
        success = TAction_Change_Zoom_Level(house, object, trig, cell);
        break;


    case TACTION_RESIZE_VIEW:
        success = TAction_Resize_Player_View(house, object, trig, cell);
        break;


    case TACTION_PLAY_ANIM:
        success = TAction_Play_Anim_At(house, object, trig, cell);
        break;


    case TACTION_EXPLOSION:
        success = TAction_Do_Explosion_At(house, object, trig, cell);
        break;


    case TACTION_METEOR_IMPACT:
        success = TAction_Meteor_Impact_At(house, object, trig, cell);
        break;


    case TACTION_ION_STORM_START:
        success = TAction_Ion_Storm_Start(house, object, trig, cell);
        break;


    case TACTION_ION_STORM_STOP:
        success = TAction_Ion_Storm_End(house, object, trig, cell);
        break;


    case TACTION_LOCK_INPUT:
        success = TAction_Lock_Input(house, object, trig, cell);
        break;


    case TACTION_UNLOCK_INPUT:
        success = TAction_Unlock_Input(house, object, trig, cell);
        break;


    case TACTION_CENTER_CAMERA:
        success = TAction_Center_Camera_At(house, object, trig, cell);
        break;


    case TACTION_ZOOM_IN:
        success = TAction_Zoom_In(house, object, trig, cell);
        break;


    case TACTION_ZOOM_OUT:
        success = TAction_Zoom_Out(house, object, trig, cell);
        break;


    case TACTION_RESHROUD_MAP:
        success = TAction_Reshroud_Map(house, object, trig, cell);
        break;


    case TACTION_CHANGE_SPOTLIGHT_BEHAVIOR:
        success = TAction_Change_Spotlight_Behavior(house, object, trig, cell);
        break;


    case TACTION_ENABLE_TRIGGER:
        success = TAction_Enable_Trigger(house, object, trig, cell);
        break;


    case TACTION_DISABLE_TRIGGER:
        success = TAction_Disable_Trigger(house, object, trig, cell);
        break;


    case TACTION_CREATE_RADAR_EVENT:
        success = TAction_Create_Radar_Event(house, object, trig, cell);
        break;


    case TACTION_LOCAL_SET:
        success = TAction_Local_Set(house, object, trig, cell);
        break;


    case TACTION_LOCAL_CLEAR:
        success = TAction_Local_Clear(house, object, trig, cell);
        break;


    case TACTION_METEOR_SHOWER:
        success = TAction_Meteor_Shower_At(house, object, trig, cell);
        break;


    case TACTION_REDUCE_TIBERIUM:
        success = TAction_Reduce_Tiberium_At(house, object, trig, cell);
        break;


    case TACTION_SELL_BUILDING:
        success = TAction_Sell_Building(house, object, trig, cell);
        break;


    case TACTION_TURN_OFF_BUILDING:
        success = TAction_Turn_Off_Building(house, object, trig, cell);
        break;


    case TACTION_TURN_ON_BUILDING:
        success = TAction_Turn_On_Building(house, object, trig, cell);
        break;


    case TACTION_APPLY_100_DAMAGE:
        success = TAction_Apply_100_Damage(house, object, trig, cell);
        break;


    case TACTION_LIGHT_FLASH_SMALL:
        success = TAction_Small_Light_Flash_At(house, object, trig, cell);
        break;


    case TACTION_LIGHT_FLASH_MEDIUM:
        success = TAction_Medium_Light_Flash_At(house, object, trig, cell);
        break;


    case TACTION_LIGHT_FLASH_LARGE:
        success = TAction_Large_Light_Flash_At(house, object, trig, cell);
        break;


    case TACTION_ANNOUNCE_WIN:
        success = TAction_Annouce_Win(house, object, trig, cell);
        break;


    case TACTION_ANNOUNCE_LOSE:
        success = TAction_Annouce_Lose(house, object, trig, cell);
        break;


    case TACTION_FORCE_END:
        success = TAction_Force_End(house, object, trig, cell);
        break;


    case TACTION_DESTROY_TAG:
        success = TAction_Destroy_Tag(house, object, trig, cell);
        break;


    case TACTION_SET_AMBIENT_STEP:
        success = TAction_Set_Ambient_Step(house, object, trig, cell);
        break;


    case TACTION_SET_AMBIENT_RATE:
        success = TAction_Set_Ambient_Rate(house, object, trig, cell);
        break;


    case TACTION_SET_AMBIENT_LIGHT:
        success = TAction_Set_Ambient_Light(house, object, trig, cell);
        break;


    case TACTION_AI_TRIGGERS_BEGIN:
        success = TAction_Set_AI_Triggers_Begin(house, object, trig, cell);
        break;


    case TACTION_AI_TRIGGERS_END:
        success = TAction_Set_AI_Triggers_End(house, object, trig, cell);
        break;


    case TACTION_RATIO_AI_TRIGGER_TEAMS:
        success = TAction_Set_Ratio_Of_AI_Trigger_Teams(house, object, trig, cell);
        break;


    case TACTION_SET_TEAM_AIRCRAFT_RATIO:
        success = TAction_Set_Ratio_Of_Team_Aircraft(house, object, trig, cell);
        break;


    case TACTION_SET_TEAM_INFANTRY_RATIO:
        success = TAction_Set_Ratio_Of_Team_Infantry(house, object, trig, cell);
        break;


    case TACTION_SET_TEAM_UNIT_RATIO:
        success = TAction_Set_Ratio_Of_Team_Units(house, object, trig, cell);
        break;


    case TACTION_REINFORCEMENTS_AT:
        success = TAction_Reinforcement_At(house, object, trig, cell);
        break;


    case TACTION_WAKEUP_SELF:
        success = TAction_Wakeup_Self(house, object, trig, cell);
        break;


    case TACTION_WAKEUP_ALL_SLEEPERS:
        success = TAction_Wakeup_Sleepers(house, object, trig, cell);
        break;


    case TACTION_WAKEUP_ALL_HARMLESS:
        success = TAction_Wakeup_Harmless(house, object, trig, cell);
        break;


    case TACTION_WAKEUP_GROUP:
        success = TAction_Wakeup_Group(house, object, trig, cell);
        break;


    case TACTION_VEIN_GROWTH:
        success = TAction_Vein_Growth(house, object, trig, cell);
        break;


    case TACTION_TIBERIUM_GROWTH:
        success = TAction_Tiberium_Growth(house, object, trig, cell);
        break;


    case TACTION_ICE_GROWTH:
        success = TAction_Ice_Growth(house, object, trig, cell);
        break;


    case TACTION_PARTICLE_ANIM_AT:
        success = TAction_Particle_Anim_At(house, object, trig, cell);
        break;


    case TACTION_REMOVE_PARTICLE_AT:
        success = TAction_Remove_Particle_Anim_At(house, object, trig, cell);
        break;


    case TACTION_LIGHTENING_STRIKE:
        success = TAction_Lightning_Strike_At(house, object, trig, cell);
        break;


    case TACTION_GO_BERZERK:
        success = TAction_Go_Bezerk(house, object, trig, cell);
        break;


    case TACTION_ACTIVATE_FIRESTORM:
        success = TAction_Activate_Firestorm_Defense(house, object, trig, cell);
        break;


    case TACTION_DEACTIVATE_FIRESTORM:
        success = TAction_Deactivate_Firestorm_Defense(house, object, trig, cell);
        break;


    case TACTION_ION_CANNON_STRIKE:
        success = TAction_Ion_Cannon_Strike(house, object, trig, cell);
        break;


    case TACTION_NUKE_STRIKE:
        success = TAction_Nuke_Strike(house, object, trig, cell);
        break;


    case TACTION_CHEM_MISSILE_STRIKE:
        success = TAction_Chemical_Missile_Strike(house, object, trig, cell);
        break;


    case TACTION_TOGGLE_TRAIN_CARGO:
        success = TAction_Toggle_Train_Cargo(house, object, trig, cell);
        break;


    case TACTION_PLAY_RANDOM_SOUND_EFFECT:
        success = TAction_Play_Sound_At_Random_Waypoint(house, object, trig, cell);
        break;


    case TACTION_PLAY_SOUND_EFFECT_AT:
        success = TAction_Play_Sound_At(house, object, trig, cell);
        break;


    case TACTION_PLAY_INGAME_MOVIE:
        success = TAction_Play_Ingame_Movie(house, object, trig, cell);
        break;


    case TACTION_FLASH_TEAM:
        success = TAction_Flash_Team(house, object, trig, cell);
        break;


    case TACTION_DISABLE_SPEECH:
        success = TAction_Disable_Speech(house, object, trig, cell);
        break;


    case TACTION_ENABLE_SPEECH:
        success = TAction_Enable_Speech(house, object, trig, cell);
        break;


    case TACTION_SET_GROUP_ID:
        success = TAction_Set_Group_ID(house, object, trig, cell);
        break;


    case TACTION_TALK_BUBBLE:
        success = TAction_Talk_Bubble(house, object, trig, cell);
        break;

        /**
         *
         *  New Vinifera actions.
         *
         */

    case TACTION_GIVE_CREDITS:
        success = _TAction_Give_Credits(house, object, trig, cell);
        break;


    case TACTION_ENABLE_SHORT_GAME:
        success = _TAction_Enable_Short_Game(house, object, trig, cell);
        break;

    case TACTION_DISABLE_SHORT_GAME:
        success = _TAction_Disable_Short_Game(house, object, trig, cell);
        break;

    case TACTION_PRINT_DIFFICULTY:
        success = _TAction_Print_Difficulty(house, object, trig, cell);
        break;

    case TACTION_BLOWUP_HOUSE:
        success = _TAction_Blowup_House(house, object, trig, cell);
        break;

    case TACTION_MAKE_ELITE:
        success = _TAction_Make_Elite(house, object, trig, cell);
        break;

    case TACTION_ENABLE_ALLYREVEAL:
        success = _TAction_Enable_AllyReveal(house, object, trig, cell);
        break;

    case TACTION_DISABLE_ALLYREVEAL:
        success = _TAction_Disable_AllyReveal(house, object, trig, cell);
        break;

    case TACTION_CREATE_AUTOSAVE:
        success = _TAction_Create_AutoSave(house, object, trig, cell);
        break;

    case TACTION_DELETE_OBJECT:
        success = _TAction_Delete_Object(house, object, trig, cell);
        break;

    case TACTION_ALL_ASSIGN_MISSION:
        success = _TAction_All_Assign_Mission(house, object, trig, cell);
        break;

        /**
         *  Do no action at all.
         */
    case TACTION_NONE:
    default:
        break;
    }

    return success;
}


/**
 *  #issue-71
 *
 *  Reimplement Play_Sound_At_Random_Waypoint to support the new waypoint limit.
 *
 *  @author: CCHyper
 */
bool TActionClassExt::_TAction_Play_Sound_At_Random_Waypoint(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    Cell cell_list[NEW_WAYPOINT_COUNT];
    int cell_list_count = 0;

    /**
     *  Make a list of all the valid waypoints in this scenario.
     */
    for (WaypointType wp = WAYPOINT_FIRST; wp < NEW_WAYPOINT_COUNT; ++wp) {
        if (ScenExtension->Is_Valid_Waypoint(wp)) {
            cell_list[cell_list_count++] = ScenExtension->Get_Waypoint_Cell(wp);
            if (cell_list_count >= std::size(cell_list)) {
                break;
            }
        }
    }

    /**
     *  Pick a random cell from the valid waypoint list and play the desired sound.
     */
    Cell rnd_cell = cell_list[Random_Pick<unsigned int>(0, std::size(cell_list) - 1)];

    Sound_Effect(Data.Sound, Cell_Coord(rnd_cell, true));

    return true;
}


/**
 *  Fixes a crash when a trigger deleted itself.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_Destroy_Trigger(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    if (Trigger) {
        for (int i = 0; i < Triggers.Count(); i++) {
            auto trigger = Triggers[i];

            /**
             *  Don't allow deleting itself.
             */
            if (trigger && trigger != trig && trigger->Class == Trigger) {

                delete trigger;
                i--;
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
 *  @see: TriggerClass and TriggerTypeClass for the other parts of this fix.
 *
 *  @author: CCHyper
 */
bool TActionClassExt::_TAction_Enable_Trigger(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    if (Trigger) {
        for (int i = 0; i < Triggers.Count(); i++) {
            auto trigger = Triggers[i];

            if (trigger->Class == Trigger) {

                /**
                 *  Only enable the trigger if it's marked as enabled for this difficulty.
                 */
                if (Scen->Difficulty == DIFF_EASY && trigger->Class->Easy
                    || Scen->Difficulty == DIFF_NORMAL && trigger->Class->Normal
                    || Scen->Difficulty == DIFF_HARD && trigger->Class->Hard) {

                    trigger->Enable();
                }
            }
        }
    }

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
bool TActionClassExt::_TAction_Win(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    /**
     *  Flag the house specified as the loser. The house parameter is only
     *  used to determine if it refers to the player or the computer.
     */
    if (Data.House == PlayerPtr->Class->House) {
        PlayerPtr->Flag_To_Lose(false);
    }
    else {
        PlayerPtr->Flag_To_Win(false);
    }

    /**
     *  Outside of campaign, mark all losers as defeated.
     */
    if (Session.Type != GAME_NORMAL) {
        for (int i = 0; i < Houses.Count(); i++) {
            HouseClass* house = Houses[i];

            if (house->Class->House == Data.House) {
                house->IsDefeated = true;
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
bool TActionClassExt::_TAction_Lose(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    /**
     *  Flag the house specified as the winner. Really the house value
     *  is only used to determine if it is the player or the computer.
     */
    if (Data.House == PlayerPtr->Class->House) {
        PlayerPtr->Flag_To_Win(false);
    }
    else {
        PlayerPtr->Flag_To_Lose(false);
    }

    /**
     *  Outside of campaign, mark all houses other than the winner as defeated.
     */
    if (Session.Type != GAME_NORMAL) {
        for (int i = 0; i < Houses.Count(); i++) {
            HouseClass* house = Houses[i];

            if (house->Class->House != Data.House) {
                house->IsDefeated = true;
            }
        }
    }

    return true;
}


/**
 *  Replacement of TAction_Change_House to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_Change_House(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    bool success = false;

    HouseClass* newhouse = HouseClass::As_Pointer(Data.House);

    /**
     *  Fix: check if the house exists, since a spawn house might not.
     */
    if (newhouse) {
        for (int i = 0; i < Technos.Count(); i++) {
            TechnoClass* techno = Technos[i];

            if (techno->IsActive && techno->IsDown && !techno->IsInLimbo) {
                if (techno->Tag && techno->Tag->Is_Trigger_Attached(trig)) {

                    techno->Captured(newhouse);
                    success = true;
                }
            }
        }
    }

    return success;
}


/**
 *  Replacement of TAction_All_Change_House to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_All_Change_House(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    bool success = false;

    HouseClass* newhouse = HouseClass::As_Pointer(Data.House);

    /**
     *  Fix: check if the house exists, since a spawn house might not.
     */
    if (newhouse) {
        for (int i = 0; i < Technos.Count(); i++) {
            TechnoClass* techno = Technos[i];

            if (techno->Owning_House() == house) {
                techno->Captured(newhouse);
                success = true;
            }
        }
    }

    return success;
}


/**
 *  Replacement of TAction_Make_Ally to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_Make_Ally(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    HouseClass* other = HouseClass::As_Pointer(Data.House);

    /**
     *  Fix: check if the house exists, since a spawn house might not.
     */
    if (other) {
        house->Make_Ally(other);
        other->Make_Ally(house);
    }

    return true;
}


/**
 *  Replacement of TAction_Make_Enemy to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_Make_Enemy(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    HouseClass* other = HouseClass::As_Pointer(Data.House);

    /**
     *  Fix: check if the house exists, since a spawn house might not.
     */
    if (other) {
        house->Make_Enemy(other);
        other->Make_Enemy(house);
    }

    return true;
}


/**
 *  Replacement of TAction_Begin_Production to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_Begin_Production(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    HouseClass* other = HouseClass::As_Pointer(Data.House);

    /**
     *  Fix: check if the house exists, since a spawn house might not.
     */
    if (other) {
        other->IsStarted = true;
    }

    return true;
}


/**
 *  Replacement of TAction_Fire_Sale to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_Fire_Sale(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    HouseClass* other = HouseClass::As_Pointer(Data.House);

    /**
     *  Fix: check if the house exists, since a spawn house might not.
     */
    if (other) {
        other->State = STATE_ENDGAME;
    }

    return true;
}


/**
 *  Replacement of TAction_Begin_Autocreate to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_Begin_Autocreate(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    HouseClass* other = HouseClass::As_Pointer(Data.House);

    /**
     *  Fix: check if the house exists, since a spawn house might not.
     */
    if (other) {
        other->IsAlerted = true;
    }

    return true;
}


/**
 *  Replacement of TAction_All_Hunt to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_All_Hunt(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    HouseClass* other = HouseClass::As_Pointer(Data.House);

    /**
     *  Fix: check if the house exists, since a spawn house might not.
     */
    if (other) {
        other->All_To_Hunt();
    }

    return true;
}


/**
 *  Replacement of TAction_Set_AI_Triggers_Begin to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_Set_AI_Triggers_Begin(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    HouseClass* other = HouseClass::As_Pointer(Data.House);

    /**
     *  Fix: check if the house exists, since a spawn house might not.
     */
    if (other) {
        other->IsAITriggersOn = true;
    }

    return true;
}


/**
 *  Replacement of TAction_Set_AI_Triggers_End to handle the case when the target house does not exist.
 *
 *  @author: ZivDero
 */
bool TActionClassExt::_TAction_Set_AI_Triggers_End(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    HouseClass* other = HouseClass::As_Pointer(Data.House);

    /**
     *  Fix: check if the house exists, since a spawn house might not.
     */
    if (other) {
        other->IsAITriggersOn = false;
    }

    return true;
}


/**
 *  Gives credits to the house specified as the argument.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_Give_Credits(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    HouseClass* other = HouseClass::As_Pointer(Data.House);

    /**
     *  Give credits to the house.
     */
    if (other) {

        const int amount = Bounds.X;
        if (amount >= 0) {
            other->Refund_Money(amount);
        }
        else {
            other->Spend_Money(-amount);
        }
    }

    return true;
}


/**
 *  Enables short game.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_Enable_Short_Game(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{    
    Session.Options.ShortGame = true;

    return true;
}


/**
 *  Disables short game.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_Disable_Short_Game(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{    
    Session.Options.ShortGame = false;

    return true;
}


/**
 *  Prints a message with the current difficulty level.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_Print_Difficulty(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    /**
     *  Calculate the message delay.
     */
    const int message_delay = Rule->MessageDelay * TICKS_PER_MINUTE;

    constexpr char difficulty_names[3][20] = {
        "Difficulty: Easy",
        "Difficulty: Medium",
        "Difficulty: Hard",
    };

    /**
     *  Send the message.
     */
    Session.Messages.Add_Message(nullptr, 0, difficulty_names[Options.Difficulty], static_cast<ColorSchemeType>(4), TPF_6PT_GRAD | TPF_USE_GRAD_PAL | TPF_FULLSHADOW, message_delay);

    return true;
}


/**
 *  Blows up the specified house.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_Blowup_House(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    HouseClass* other = HouseClass::As_Pointer(Data.House);

    /**
     *  Blow the house up and mark the player as defeated.
     */
    if (other) {
        other->Blowup_All();
        other->MPlayer_Defeated();
    }

    return true;
}


/**
 *  Makes all objects attached to the trigger elite.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_Make_Elite(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
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
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_Enable_AllyReveal(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    Rule->IsAllyReveal = true;

    return true;
}


/**
 *  Disables ally reveal.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_Disable_AllyReveal(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{    
    Rule->IsAllyReveal = false;

    return true;
}


/**
 *  Schedules the creation of an autosave the next frame.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_Create_AutoSave(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    /**
     *  Schedule a save.
     */
    Vinifera_DoSave = true;

    return true;
}


/**
 *  Silently deletes all objects attached to this trigger from the map.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_Delete_Object(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    /**
     *  Iterate all technos, and if their tag is attached to this trigger, flag them for deletion.
     */
    for (int i = 0; i < Technos.Count(); i++) {
        TechnoClass* techno = Technos[i];

        if (techno->IsActive && techno->IsDown && !techno->IsInLimbo) {
            if (techno->Tag && techno->Tag->Is_Trigger_Attached(trig)) {
                techno->Remove_This();
            }
        }
    }

    return true;
}


/**
 *  Assigns a mission to all units owned by the trigger owner.
 *
 *  @author: ZivDero, Rampastring
 */
bool TActionClassExt::_TAction_All_Assign_Mission(HouseClass* house, ObjectClass* object, TriggerClass* trig, Cell& cell)
{
    /**
     *  Iterate all units, and if they are owned by the trigger owner, assign the mission.
     */
    for (int i = 0; i < Technos.Count(); i++) {
        TechnoClass* techno = Technos[i];

        if (techno->IsActive && techno->IsDown && !techno->IsInLimbo) {
            if (techno->Owning_House() == house) {
                techno->Assign_Mission(static_cast<MissionType>(Data.Value));
            }
        }
    }

    return true;
}


/**
 *  Main function for patching the hooks.
 */
void TActionClassExtension_Hooks()
{
    /**
     *  Replacement of TActionClass::operator().
     */
    Patch_Jump(0x00619110, &TActionClassExt::_Function_Call_Operator);

    /**
     *  #issue-674
     *
     *  Fixes a bug where the game would crash when TACTION_WAKEUP_GROUP was
     *  executed but the game was not able to match the Group to the triggers
     *  group. This was because the game was searching the Foots vector with
     *  the count of the Technos vector, and in cases where the Group did
     *  not match, the game would crash trying to search out of bounds.
     *
     *  @author: CCHyper
     */
    Patch_Dword(0x0061AFB0 + 2, 0x007E4820 + 4); // Foot vector to Technos vector.

    /**
     *  Replacement of various vanilla actions.
     */
    Patch_Jump(0x0061BF50, &TActionClassExt::_TAction_Play_Sound_At_Random_Waypoint);
    Patch_Jump(0x0061CDA0, &TActionClassExt::_TAction_Enable_Trigger);
    Patch_Jump(0x0061CCB0, &TActionClassExt::_TAction_Destroy_Trigger);
    Patch_Jump(0x0061C200, &TActionClassExt::_TAction_Win);
    Patch_Jump(0x0061C230, &TActionClassExt::_TAction_Lose);
    Patch_Jump(0x0061B630, &TActionClassExt::_TAction_Change_House);
    Patch_Jump(0x0061B6E0, &TActionClassExt::_TAction_All_Change_House);
    Patch_Jump(0x0061B820, &TActionClassExt::_TAction_Make_Ally);
    Patch_Jump(0x0061B860, &TActionClassExt::_TAction_Make_Enemy);
    Patch_Jump(0x0061C260, &TActionClassExt::_TAction_Begin_Production);
    Patch_Jump(0x0061C280, &TActionClassExt::_TAction_Fire_Sale);
    Patch_Jump(0x0061C2A0, &TActionClassExt::_TAction_Begin_Autocreate);
    Patch_Jump(0x0061C3D0, &TActionClassExt::_TAction_All_Hunt);
    Patch_Jump(0x0061D0E0, &TActionClassExt::_TAction_Set_AI_Triggers_Begin);
    Patch_Jump(0x0061D100, &TActionClassExt::_TAction_Set_AI_Triggers_End);
}
