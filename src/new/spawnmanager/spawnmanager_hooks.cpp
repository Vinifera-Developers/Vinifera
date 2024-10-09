/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SPAWNMANAGER_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for SpawnManagerClass.
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


DECLARE_PATCH(_EventClass_Execute_Spawn_Manager_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, techno, esi);
    static TechnoClassExtension* extension;

    extension = Extension::Fetch<TechnoClassExtension>(techno);
    if (extension && extension->SpawnManager)
        extension->SpawnManager->Abandon_Target();

    static RTTIType rtti = techno->Kind_Of();
    if (rtti == RTTI_UNIT)
    {
        JMP(0x00494AC5);
    }
    else
    {
        JMP(0x00495110);
    }

}


DECLARE_PATCH(_DriveLocomotionClass_Start_Of_Move_Spawn_Manager_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, linked_to, eax);
    static TechnoClassExtension* extension;

    _asm pushad

    if (linked_to->EMPFramesRemaining > 0)
    {
        _asm popad
        // return 1;
        JMP(0x0047FE39);
    }

    extension = Extension::Fetch<TechnoClassExtension>(linked_to);
    if (extension->SpawnManager && extension->SpawnManager->Preparing_Count())
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
 *  Main function for patching the hooks.
 */
void SpawnManager_Hooks()
{
    Patch_Jump(0x00494AB5, &_EventClass_Execute_Spawn_Manager_Patch);
    Patch_Jump(0x0047FE2F, &_DriveLocomotionClass_Start_Of_Move_Spawn_Manager_Patch);
}
