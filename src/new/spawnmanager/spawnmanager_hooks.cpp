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

#include "always.h"

#include "spawnmanager_hooks.h"

#include "extension.h"
#include "hooker.h"
#include "kamikazetracker.h"
#include "spawnmanager.h"
#include "syringe.h"
#include "techno.h"
#include "technoext.h"
#include "veinholemonster.h"
#include "vinifera_globals.h"


/**
 *  Patch to make the spawn manager abandon its target when ordered to idle.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00494AB5, _EventClass_Execute_IDLE_Spawn_Manager_Patch, 0)
{
    GET(TechnoClass*, techno, ESI);

    auto extension = Extension::Fetch(techno);
    if (extension->SpawnManager) {
        extension->SpawnManager->Abandon_Target();
    }

    if (techno->RTTI == RTTI_UNIT) {
        return 0x00494AC5;
    } else {
        return 0x00495110;
    }

}


/**
 *  Patch to block vehicles from moving if they're currently preparing to spawn.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0047FE2F, _DriveLocomotionClass_Start_Of_Move_Spawn_Manager_Patch, 0)
{
    GET(TechnoClass*, linked_to, EAX);

    auto extension = Extension::Fetch(linked_to);
    if (linked_to->EMPFramesRemaining > 0 || extension->SpawnManager && extension->SpawnManager->Preparing_Count()) {
        // return 1;
        return 0x0047FE39;
    }

    // Continue execution
    return 0x0047FE45;
}


/**
 *  Patch to perform kamikaze AI.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00507000, _LogicClass_AI_Kamikaze_AI_Patch, 0)
{
    // Stolen instruction
    VeinholeMonsterClass::Update_All();

    KamikazeTracker->AI();

    return 0x00507005;
}


/**
 *  Main function for patching the hooks.
 */
void SpawnManager_Hooks()
{

}
