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
#include "asserthandler.h"
#include "debughandler.h"


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
    //DEBUG_INFO("Initialising new commands.\n");

    //cmdptr = new PNGScreenCaptureCommandClass;
    //Commands.Add(cmdptr);
    
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

        //cmdptr = new ExplosionCommandClass;
        //Commands.Add(cmdptr);

        //cmdptr = new SuperExplosionCommandClass;
        //Commands.Add(cmdptr);

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

    JMP(0x004E6FAE);
}


/**
 *  Main function for patching the hooks.
 */
void CommandExtension_Hooks()
{
    Patch_Jump(0x004E6FA9, &_Init_Commands_Patch);

    /**
     *  Replace ScreenCaptureCommandClass with PNGScreenCaptureCommandClass.
     */
    Hook_Virtual(0x004EAAC0, PNGScreenCaptureCommandClass::Get_Name);
    Hook_Virtual(0x004EAAE0, PNGScreenCaptureCommandClass::Get_UI_Name);
    Hook_Virtual(0x004EAAD0, PNGScreenCaptureCommandClass::Get_Category);
    Hook_Virtual(0x004EAAF0, PNGScreenCaptureCommandClass::Get_Description);
    Hook_Virtual(0x004EAB00, PNGScreenCaptureCommandClass::Process);
}
