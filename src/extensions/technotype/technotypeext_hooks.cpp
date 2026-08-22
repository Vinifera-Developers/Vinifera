/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended TechnoTypeClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "technotypeext_hooks.h"

#include "asserthandler.h"
#include "ccini.h"
#include "extension.h"
#include "extension_globals.h"
#include "hooker.h"
#include "house.h"
#include "rulesext.h"
#include "syringe.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tibsun_defines.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
DECLARE_EXTENDING_CLASS_AND_PAIR(TechnoTypeClass)
{
public:
    int _Max_Pips() const;
    void const* _Get_Cameo_Data();
};


/**
 *  Reimplements TechnoTypeClass::Max_Pips.
 *
 *  @author: ZivDero
 */
int TechnoTypeClassExt::_Max_Pips() const
{
    int max_pips = 0;
    if (PipScale - 1 < RuleExtension->MaxPips.Count()) {
        max_pips = RuleExtension->MaxPips[PipScale - 1];
    }

    // Negative values are not allowed
    if (max_pips < 0) return 0;

    switch (PipScale) {
    case PIPSCALE_AMMO:
        return std::clamp(MaxAmmo, 0, max_pips);

    case PIPSCALE_PASSENGERS:
        return std::clamp(MaxPassengers, 0, max_pips);

    case PIPSCALE_TIBERIUM:
    case PIPSCALE_POWER:
    case PIPSCALE_CHARGE:
    default:
        return max_pips;
    }
}


/**
 *  Replaces TechnoTypeClass::Get_Cameo_Data to fetch cameos in runtime.
 *
 *  @author: ZivDero
 */
void const* TechnoTypeClassExt::_Get_Cameo_Data()
{
    if (CameoData == nullptr) {
        std::string cameo_name = ArtINI.Get_String(GraphicName.c_str(), "Cameo", "");
        if (cameo_name.empty()) {
            std::string cameo_name = ArtINI.Get_String(IniName.c_str(), "Cameo", "");
        }
        if (!cameo_name.empty()) {
            CameoFilename = cameo_name;
            CameoData = static_cast<ShapeSet const*>(MFCD::Retrieve((cameo_name + ".SHP").c_str()));
        }
        if (CameoData == nullptr) {
            CameoData = static_cast<ShapeSet const*>(MFCD::Retrieve("XXICON.SHP"));
        }
    }

    return CameoData;
}


/**
 *  Null out the stale CameoData pointer after load.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x0063DE35, _TechnoTypeClass_Load_CameoName_Patch, 0)
{
    GET(TechnoTypeClass*, this_ptr, EDI);
    this_ptr->CameoData = nullptr;
    return 0x0063DEEB;
}


/**
 *  Main function for patching the hooks.
 */
void TechnoTypeClassExtension_Hooks()
{
    Patch_Jump(0x0063D460, &TechnoTypeClassExt::_Max_Pips);
    Patch_Jump(0x0063B910, &TechnoTypeClassExt::_Get_Cameo_Data);
    Patch_Jump(0x0063D02A, 0x0063D0D7); // Skip fetching the cameo in TechnoTypeClass::Read_INI.
}
