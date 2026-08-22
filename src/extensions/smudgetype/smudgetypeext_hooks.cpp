/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended SmudgeTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "smudgetypeext_hooks.h"

#include "asserthandler.h"
#include "debughandler.h"
#include "extension.h"
#include "hooker.h"
#include "isotiletype.h"
#include "isotiletypeext.h"
#include "mouse.h"
#include "smudgetype.h"
#include "smudgetypeext.h"
#include "smudgetypeext_init.h"


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


/**
 *  Re-implementation of SmudgeTypeClass::Can_Place_Here.
 *
 *  @author: ZivDero
 */
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
            if (isotype_ext->AllowedSmudges.Count() > 0 && !isotype_ext->AllowedSmudges.Is_Present(HeapID)) {
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
