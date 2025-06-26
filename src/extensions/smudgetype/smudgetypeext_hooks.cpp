/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          SMDUGETYPEEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended SmudgeTypeClass.
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
#include "smudgetypeext_hooks.h"
#include "smudgetypeext_init.h"
#include "smudgetypeext.h"
#include "smudgetype.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"
#include "extension.h"
#include "hooker.h"
#include "isotiletype.h"
#include "isotiletypeext.h"
#include "mouse.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(SmudgeTypeClass)
{
public:
    bool _Can_Place_Here(Cell const& origin, bool underbuildings) const;
};


bool SmudgeTypeClassExt::_Can_Place_Here(Cell const& origin, bool underbuildings) const
{
    for (int h = 0; h < Height; h++) {
        for (int w = 0; w < Width; w++) {
            Cell trycell = origin + Cell(w, h);
            CellClass* cell = &Map[trycell];
            if (!Map.In_Radar(trycell)) {
                return false;
            }
            if (cell->Ramp != 0) {
                return false;
            }
            if (cell->Smudge != SMUDGE_NONE) {
                return false;
            }
            if (cell->Overlay != OVERLAY_NONE) {
                return false;
            }
            if (!underbuildings && cell->Cell_Building() != NULL) {
                return false;
            }
            IsometricTileType ittype = cell->ITType;
            if (cell->ITType < ISOTILE_FIRST || cell->ITType >= IsoTileTypes.Count()) {
                ittype = ISOTILE_FIRST;
            }
            if (!IsoTileTypes[ittype]->IsMorphable) {
                return false;
            }
            auto isotype_ext = Extension::Fetch(IsoTileTypes[ittype]);
            if (isotype_ext->AllowedSmudges.Count() > 0 && !isotype_ext->AllowedSmudges.Is_Present(const_cast<SmudgeTypeClassExt*>(this))) {
                return false;
            }
        }
    }
    return true;
}



/**
 *  Main function for patching the hooks.
 */
void SmudgeTypeClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    SmudgeTypeClassExtension_Init();

    Patch_Jump(0x005FBE30, &SmudgeTypeClassExt::_Can_Place_Here);
}
