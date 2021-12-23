/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          FETCHRES_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for Fetch_Resource and Fetch_String.
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
#include "fetchres_hooks.h"
#include "fetchres.h"
#include "vinifera_util.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


extern HMODULE DLLInstance;


/**
 *  This intercept allows us to override default dialogs using a new id, while
 *  allowing the new dialogs to be overridden by LANGUAGE.DLL.
 * 
 *  @author: CCHyper
 */
static HGLOBAL Fetch_Resource_Intercept(const char *id, const char *type)
{
    HGLOBAL hGlobal = Fetch_Resource(id, type);
    if (hGlobal) {
        return hGlobal;
    }

    HGLOBAL v_hGlobal = FETCH_RESOURCE(DLLInstance, id, type);
    if (v_hGlobal) {
        return v_hGlobal;
    }

    return nullptr;
}

 
/**
 *  Main function for patching the hooks.
 */
void FetchRes_Hooks()
{
    Patch_Call(0x005A0645, &Fetch_Resource_Intercept);
    Patch_Call(0x005A0C75, &Fetch_Resource_Intercept);
    Patch_Call(0x0068301B, &Fetch_Resource_Intercept);
}
