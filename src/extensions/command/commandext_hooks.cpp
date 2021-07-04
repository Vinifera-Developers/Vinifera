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
#include "ccfile.h"
#include "ccini.h"
#include "object.h"
#include "unit.h"
#include "unittype.h"
#include "asserthandler.h"
#include "debughandler.h"


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
        if (techno->What_Am_I() == RTTI_UNIT) {

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
 *  @author: CCHyper
 */
void Init_Vinifera_Commands()
{
    CommandClass *cmdptr = nullptr;

    DEBUG_INFO("Init_Vinifera_Commands(enter).\n");

    /**
     *  Initialises any new commands here.
     */
    DEBUG_INFO("Initialising new commands.\n");

    //cmdptr = new PNGScreenCaptureCommandClass;
    //Commands.Add(cmdptr);

    cmdptr = new ManualPlaceCommandClass;
    Commands.Add(cmdptr);

    cmdptr = new PrevThemeCommandClass;
    Commands.Add(cmdptr);

    cmdptr = new NextThemeCommandClass;
    Commands.Add(cmdptr);

    /**
     *  Next, initialised any new commands here if the developer mode is enabled.
     */
    if (Vinifera_DeveloperMode) {
        DEBUG_INFO("Initialising developer commands.\n");

        cmdptr = new MemoryDumpCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new DumpHeapCRCCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new InstantBuildCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new AIInstantBuildCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ForceWinCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ForceLoseCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ForceDieCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new CaptureObjectCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new SpecialWeaponsCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new FreeMoneyCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new LightningBoltCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new IonBlastCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ExplosionCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new SuperExplosionCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new BailOutCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new IonStormCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new MapSnapshotCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new DeleteObjectCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new SpawnAllCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new DamageCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ToggleEliteCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new BuildCheatCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ToggleShroudCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new HealCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ToggleInertCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new DumpAIBaseNodesCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ToggleBerzerkCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new EncroachShadowCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new EncroachFogCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ToggleAllianceCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new AddPowerCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new PlaceCrateCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new CursorPositionCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ToggleFrameStepCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new Step1FrameCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new Step5FramesCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new Step10FramesCommandClass;
        Commands.Add(cmdptr);

        cmdptr = new ToggleAIControlCommandClass;
        Commands.Add(cmdptr);
    }

#ifndef NDEBUG
    /**
     *  Initialises debug commands, these must be last!
     */
    //DEBUG_INFO("Initialising debug commands.\n");

#endif
    
    DEBUG_INFO("Init_Vinifera_Commands(exit).\n");
}

/**
 *  Set the default key assignments.
 * 
 *  @author: CCHyper
 */
static void Process_Vinifera_Hotkeys()
{
    CommandClass *cmdptr = nullptr;
    KeyNumType key;

    CCFileClass file("KEYBOARD.INI");
    CCINIClass ini;

    ini.Load(file, false);

    if (!ini.Is_Present("Hotkey", "ManualPlace")) {
        cmdptr = CommandClass::From_Name("ManualPlace");
        if (cmdptr) {
            key = reinterpret_cast<ViniferaCommandClass *>(cmdptr)->Default_Key();
            HotkeyIndex.Add_Index(key, cmdptr);
        }
    }

    if (!ini.Is_Present("Hotkey", "PrevTheme")) {
        cmdptr = CommandClass::From_Name("PrevTheme");
        if (cmdptr) {
            key = reinterpret_cast<ViniferaCommandClass *>(cmdptr)->Default_Key();
            HotkeyIndex.Add_Index(key, cmdptr);
        }
    }

    if (!ini.Is_Present("Hotkey", "NextTheme")) {
        cmdptr = CommandClass::From_Name("NextTheme");
        if (cmdptr) {
            key = reinterpret_cast<ViniferaCommandClass *>(cmdptr)->Default_Key();
            HotkeyIndex.Add_Index(key, cmdptr);
        }
    }
}

/**
 *  Patch for initialising the new hotkey commands.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_Init_Commands_Patch)
{
    Init_Vinifera_Commands();

    /**
     *  Stolen bytes/code here.
     */
original_code:
    Load_Keyboard_Hotkeys();

    Process_Vinifera_Hotkeys();

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
}
