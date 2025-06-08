/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          COMMANDEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended command class.
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
#include "commandext.h"
#include "vinifera_globals.h"
#include "tibsun_functions.h"
#include "tibsun_globals.h"
#include "session.h"
#include "ccfile.h"
#include "ccini.h"
#include "object.h"
#include "unit.h"
#include "unittype.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  This function reimplements the chunk of code that populates the keyboard
 *  command list box. This allows us to enforce limits on what commands
 *  to show depending on the current game mode.
 * 
 *  @author: CCHyper
 */
static void Populate_Command_Categories(HWND hWnd, const char *category)
{
    for (int i = 0; i < Commands.Count(); ++i) {
        CommandClass *cmd = Commands[i];
        if (!cmd) {
            continue;
        }

        /**
         *  Any Vinifera commands are subject to game mode checks. We only need
         *  to check these if are actually "in game".
         */
        if (ScenarioStarted || TacticalViewActive) {

            ViniferaCommandClass *vcmd = dynamic_cast<ViniferaCommandClass *>(cmd);
            if (vcmd) {
                if (vcmd->Multiplayer_Only() && (Session.Type == GAME_NORMAL || Session.Type == GAME_SKIRMISH || Session.Type == GAME_WDT)) {
                    continue;
                }
                if (vcmd->Developer_Only() && !Vinifera_DeveloperMode) {
                    continue;
                }
            }

        }

        if (strcmpi(cmd->Get_Category(), category) != 0) {
            continue;
        }

        /**
         *  Add the string and data pair to the list box.
         */
        LRESULT result = SendMessage(hWnd, LB_ADDSTRING, 0, (LPARAM)cmd->Get_UI_Name());
        if (result != LB_ERR && result != LB_ERRSPACE) {
            UINT index = (UINT)result;
            SendMessage(hWnd, LB_SETITEMDATA, index, (LPARAM)cmd);
        }

    }
}


/**
 *  Patch to intercept the populating of the keyboard command list box.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_OptionsClass_Keyboard_Options_Dialog_Populate_Intercept_Patch)
{
    GET_REGISTER_STATIC(HWND, hWnd, ebp);
    LEA_STACK_STATIC(const char *, category, esp, 0x0A4);

    Populate_Command_Categories(hWnd, category);

    JMP(0x0058A79C);
}


/**
 *  #issue-#53
 * 
 *  Allow harvesters to be considered when executing the "Guard" command.
 * 
 *  The original GuardCommandClass code checks if the unit has a weapon
 *  before assigning MISSION_GUARD_AREA, so harvesters never pass this
 *  check as they are without a weapon. Harvesters actually have a special
 *  case in Mission_Guard_Area() to tell them to find the nearest tiberium
 *  patch if GUARD_AREA is assigned.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_GuardCommandClass_Process_Harvesters_Set_Mission_Patch)
{
    GET_REGISTER_STATIC(TechnoClass *, techno, esi);
    static UnitClass *unit;
    static UnitTypeClass *unittype;

    /**
     *  Original code:
     *  
     *  Can the player control this object?
     */
    if (techno->Can_Player_Move()) {

        /**
         *  Is the selected object currently being processing a unit? We perform
         *  this check 'before' Can_Player_Fire() so we can handle any possible
         *  case where harvesters might have a weapon.
         */
        if (techno->RTTI == RTTI_UNIT) {

            /**
             *  Make sure this object is in fact a harvester of some type.
             */
            unit = reinterpret_cast<UnitClass *>(techno);
            unittype = unit->Class;
            if (unittype->IsToHarvest || unittype->IsToVeinHarvest) {

                /**
                 *  If the harvester is currently busy unloading, skip it.
                 */
                if (unit->Get_Mission() != MISSION_UNLOAD && !unit->IsDumping) {

                    /**
                     *  For tiberium harvesters, GUARD_AREA actually tells the
                     *  harvester to start scanning for a tiberium patch. But for
                     *  weed eaters, we need to use HARVEST. So just use HARVEST
                     *  for handling both units.
                     */
                    unit->Player_Assign_Mission(MISSION_HARVEST, techno->Get_Target_Cell_Ptr());

                    AllowVoice = false;

                    /**
                     *  Mission assigned, iterate to the next selected object.
                     */
                    goto continue_loop;
                }
            }
        }

        /**
         *  Original code:
         *  
         *  Any unit that has a weapon, assign the enter guard area mission.
         */
        if (techno->Can_Player_Fire()) {
            techno->Player_Assign_Mission(MISSION_GUARD_AREA, techno->Get_Target_Cell_Ptr());
            AllowVoice = false;
        }

    }

    /**
     *  Continue the loop over CurrentObjects.
     */
continue_loop:
    JMP(0x004E95FC);
}


/**
 *  Initialises the new hotkey commands.
 * 
 *  @author: CCHyper, ZivDero
 */
void Init_Vinifera_Commands()
{
    DEBUG_INFO("Init_Vinifera_Commands(enter).\n");

    /**
     *  Initialize new commands.
     */
    DEBUG_INFO("Initializing new commands.\n");

    Commands.Add(new ManualPlaceCommandClass);
    Commands.Add(new RepeatLastBuildingCommandClass);
    Commands.Add(new RepeatLastInfantryCommandClass);
    Commands.Add(new RepeatLastUnitCommandClass);
    Commands.Add(new RepeatLastAircraftCommandClass);
    Commands.Add(new PrevThemeCommandClass);
    Commands.Add(new NextThemeCommandClass);
    Commands.Add(new ScrollNECommandClass);
    Commands.Add(new ScrollSECommandClass);
    Commands.Add(new ScrollSWCommandClass);
    Commands.Add(new ScrollNWCommandClass);
    Commands.Add(new JumpCameraWestCommandClass);
    Commands.Add(new JumpCameraEastCommandClass);
    Commands.Add(new JumpCameraNorthCommandClass);
    Commands.Add(new JumpCameraSouthCommandClass);
    Commands.Add(new ToggleSuperTimersCommandClass);

    /**
     *  Initialize hotkeys for the sidebar tabs, if sidebar tabs are enabled.
     */
    if (Vinifera_NewSidebar) {
        DEBUG_INFO("Initializing sidebar tab commands.\n");

        Commands.Add(new SetStructureTabCommandClass);
        Commands.Add(new SetInfantryTabCommandClass);
        Commands.Add(new SetUnitTabCommandClass);
        Commands.Add(new SetSpecialTabCommandClass);
    }

    /**
     *  Next, initialize any developer mode commands if developer mode is enabled.
     */
    if (Vinifera_DeveloperMode) {
        DEBUG_INFO("Initializing developer commands.\n");

        Commands.Add(new MemoryDumpCommandClass);
        Commands.Add(new DumpHeapCRCCommandClass);
        Commands.Add(new DumpTriggersCommandClass);
        Commands.Add(new InstantBuildCommandClass);
        Commands.Add(new AIInstantBuildCommandClass);
        Commands.Add(new ForceWinCommandClass);
        Commands.Add(new ForceLoseCommandClass);
        Commands.Add(new ForceDieCommandClass);
        Commands.Add(new CaptureObjectCommandClass);
        Commands.Add(new SpecialWeaponsCommandClass);
        Commands.Add(new FreeMoneyCommandClass);
        Commands.Add(new LightningBoltCommandClass);
        Commands.Add(new IonBlastCommandClass);
        Commands.Add(new ExplosionCommandClass);
        Commands.Add(new SuperExplosionCommandClass);
        Commands.Add(new BailOutCommandClass);
        Commands.Add(new IonStormCommandClass);
        Commands.Add(new MapSnapshotCommandClass);
        Commands.Add(new DeleteObjectCommandClass);
        Commands.Add(new SpawnAllCommandClass);
        Commands.Add(new DamageCommandClass);
        Commands.Add(new ToggleEliteCommandClass);
        Commands.Add(new BuildCheatCommandClass);
        Commands.Add(new ToggleShroudCommandClass);
        Commands.Add(new HealCommandClass);
        Commands.Add(new ToggleInertCommandClass);
        Commands.Add(new DumpAIBaseNodesCommandClass);
        Commands.Add(new ToggleBerzerkCommandClass);
        Commands.Add(new EncroachShadowCommandClass);
        Commands.Add(new EncroachFogCommandClass);
        Commands.Add(new ToggleAllianceCommandClass);
        Commands.Add(new AddPowerCommandClass);
        Commands.Add(new PlaceCrateCommandClass);
        Commands.Add(new CursorPositionCommandClass);
        Commands.Add(new ToggleFrameStepCommandClass);
        Commands.Add(new Step1FrameCommandClass);
        Commands.Add(new Step5FramesCommandClass);
        Commands.Add(new Step10FramesCommandClass);
        Commands.Add(new ToggleAIControlCommandClass);
        Commands.Add(new StartingWaypointsCommandClass);
        Commands.Add(new PlaceInfantryCommandClass);
        Commands.Add(new PlaceUnitCommandClass);
        Commands.Add(new PlaceTiberiumCommandClass);
        Commands.Add(new ReduceTiberiumCommandClass);
        Commands.Add(new PlaceFullTiberiumCommandClass);
        Commands.Add(new RemoveTiberiumCommandClass);
        Commands.Add(new InstantSuperRechargeCommandClass);
        Commands.Add(new AIInstantSuperRechargeCommandClass);
        Commands.Add(new DumpNetworkCRCCommandClass);
        Commands.Add(new DumpHeapsCommandClass);
        Commands.Add(new ReloadRulesCommandClass);
    }

    /**
     *  Create any supporting directories.
     */
    CreateDirectory(Vinifera_ScreenshotDirectory, nullptr);
    
    DEBUG_INFO("Init_Vinifera_Commands(exit).\n");
}


/**
 *  Set the default key assignments.
 * 
 *  @author: ZivDero
 */
static void Process_Vinifera_Hotkey_Defaults()
{
    for (int i = 0; i < Commands.Count(); i++)
    {
        auto vcmd = dynamic_cast<ViniferaCommandClass*>(Commands[i]);
        if (vcmd) {
            KeyNumType key = vcmd->Default_Key();
            if (key != KN_NONE && !HotkeyIndex.Is_Present(key) && HotkeyIndex.Fetch_ID_By_Data(vcmd) == -1) {
                HotkeyIndex.Add_Index(key, vcmd);
            }
        }
    }
}


/**
 *  Patch for initializing the new hotkey commands.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Commands_Patch)
{
    Init_Vinifera_Commands();

    /**
     *  Stolen bytes/code here.
     */
    Load_Keyboard_Hotkeys();

    Process_Vinifera_Hotkey_Defaults();

    JMP(0x004E6FAE);
}


/**
 *  Main function for patching the hooks.
 */
void CommandExtension_Hooks()
{
    Patch_Jump(0x004E6FA9, &_Init_Commands_Patch);

    Patch_Dword(0x0058A917+1, 0x006FEFEC); // "Keyboard.ini" to "KEYBOARD.INI"

    /**
     *  Replace ScreenCaptureCommandClass with PNGScreenCaptureCommandClass.
     */
    Hook_Virtual(0x004EAAC0, PNGScreenCaptureCommandClass::Get_Name);
    Hook_Virtual(0x004EAAE0, PNGScreenCaptureCommandClass::Get_UI_Name);
    Hook_Virtual(0x004EAAD0, PNGScreenCaptureCommandClass::Get_Category);
    Hook_Virtual(0x004EAAF0, PNGScreenCaptureCommandClass::Get_Description);
    Hook_Virtual(0x004EAB00, PNGScreenCaptureCommandClass::Process);

    Patch_Jump(0x004E95C2, &_GuardCommandClass_Process_Harvesters_Set_Mission_Patch);

    /**
     *  This can not be in client compatabile builds currently as the additional
     *  commands added do not have runtime type information.
     */
#if !defined(TS_CLIENT)
    Patch_Jump(0x0058A72E, &_OptionsClass_Keyboard_Options_Dialog_Populate_Intercept_Patch);
#endif
}
