/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SPAWNMANAGER_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for SpawnManagerClass
 *                 and KamikazeTrackerClass.
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

#include "spawnmanager_hooks.h"

#include "extension.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "spawnmanager.h"
#include "techno.h"
#include "technoext.h"
#include "kamikazetracker.h"
#include "veinholemonster.h"
#include "vinifera_globals.h"


/**
 *  Patch to make the spawn manager abandon its target when ordered to idle.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_EventClass_Execute_IDLE_Spawn_Manager_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, techno, esi);
    static TechnoClassExtension* extension;
    static RTTIType rtti;

    extension = Extension::Fetch(techno);
    if (extension->SpawnManager)
        extension->SpawnManager->Abandon_Target();

    rtti = techno->RTTI;
    if (rtti == RTTI_UNIT)
    {
        JMP(0x00494AC5);
    }
    else
    {
        JMP(0x00495110);
    }

}


/**
 *  Patch to block vehicles from moving if they're currently preparing to spawn.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_DriveLocomotionClass_Start_Of_Move_Spawn_Manager_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, linked_to, eax);
    static TechnoClassExtension* extension;

    _asm pushad

    extension = Extension::Fetch(linked_to);
    if (linked_to->EMPFramesRemaining > 0 || extension->SpawnManager && extension->SpawnManager->Preparing_Count())
    {
        _asm popad
        // return 1;
        JMP(0x0047FE39);
    }

    // Continue execution
    _asm popad
    JMP_REG(edx, 0x0047FE45);
}


/**
 *  Patch to perform kamikaze AI.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_LogicClass_AI_Kamikaze_AI_Patch)
{
    // Stolen instruction
    VeinholeMonsterClass::Update_All();

    KamikazeTracker->AI();

    JMP(0x00507005);
}


/**
 *  Main function for patching the hooks.
 */
void SpawnManager_Hooks()
{
    Patch_Jump(0x00494AB5, &_EventClass_Execute_IDLE_Spawn_Manager_Patch);
    Patch_Jump(0x0047FE2F, &_DriveLocomotionClass_Start_Of_Move_Spawn_Manager_Patch);
    Patch_Jump(0x00507000, &_LogicClass_AI_Kamikaze_AI_Patch);
}
