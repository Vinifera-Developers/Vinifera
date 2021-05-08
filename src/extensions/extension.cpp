/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          EXTENSION.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Base class for declaring extended class instances.
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
#include "extension.h"
#include "tibsun_globals.h"
#include "swizzle.h"


/**
 *  Base class implementation of Load.
 * 
 *  @author: CCHyper
 */
HRESULT ExtensionBase::Load(IStream *pStm)
{
    if (!pStm) {
        return E_POINTER;
    }

    return S_OK;
}


/**
 *  Base class implementation of Save.
 * 
 *  @author: CCHyper
 */
HRESULT ExtensionBase::Save(IStream *pStm, BOOL fClearDirty)
{
    if (!pStm) {
        return E_POINTER;
    }

    if (fClearDirty) {
        IsDirty = false;
    }

    return S_OK;
}
