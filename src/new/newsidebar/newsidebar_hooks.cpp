/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          NEWSIDEBAR_HOOKS.CPP
 *
 *  @author        ZivDero
 *
 *  @brief         Contains the hooks for the new sidebar.
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

#include "newsidebar_hooks.h"

#include "debughandler.h"
#include "hooker.h"
#include "hooker_macros.h"
#include "newsidebar.h"
#include "tibsun_globals.h"
#include "vinifera_globals.h"


static void Delete_Sidebar()
{
    delete Sidebar;
    Sidebar = nullptr;
}


static void Create_Sidebar()
{
    DEBUG_INFO("Creating New Sidebar\n");
    delete Sidebar;
    Sidebar = new NewSidebarClass;
}


DECLARE_PATCH(_Init_Game_Create_Sidebar_Patch)
{
    // Stolen instruction
    LogicalSurface = HiddenSurface;

    Create_Sidebar();

    DEBUG_INFO("Init Bulk Data\n");
    JMP(0x004E08DE);
}


/**
 *  Main function for patching the hooks.
 */
void NewSidebar_Hooks()
{
    Patch_Jump(0x004E08D3, &_Init_Game_Create_Sidebar_Patch);
}
