/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended BulletClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "bulletext_hooks.h"

#include "anim.h"
#include "asserthandler.h"
#include "building.h"
#include "bullet.h"
#include "bullettype.h"
#include "bullettypeext.h"
#include "extension.h"
#include "hooker.h"
#include "house.h"
#include "infantry.h"
#include "iomap.h"
#include "overlaytype.h"
#include "syringe.h"
#include "techno.h"
#include "warheadtype.h"
#include "warheadtypeext.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(BulletClass)
{
public:
    bool _Is_Forced_To_Explode(Coord& coord);
};


/**
 *  #issue-444
 *
 *  Full replacement of BulletClass::Is_Forced_To_Explode.
 *
 *  @author: 10/10/1996 JLB : Created.
 *           22/10/2024 Rampastring : Adjustments for Tiberian Sun.
 */
bool BulletClassExt::_Is_Forced_To_Explode(Coord& coord)
{
    coord = Position;
    CellClass* cellptr = &Map[PositionCoord];
    int height = HeightAGL;

    /*
    **  Check for impact on a wall or other high obstacle.
    */
    if (!Class->IsHigh && cellptr->Overlay != OVERLAY_NONE && OverlayTypes[cellptr->Overlay]->IsHigh && height < 100) {
        return true;
    }

    /*
    **  Check for impact on the ground.
    */
    if (height < 0) {
        return true;
    }

    /*
    **  Check to make sure that underwater projectiles (torpedoes) will not
    **  travel in anything but water.
    */
    const auto bullettypeext = Extension::Fetch(Class);
    if (bullettypeext->IsTorpedo) {
        int distance = ::Distance(Coord_Fraction(coord), Coord(CELL_LEPTON_W / 2, CELL_LEPTON_W / 2));

        if (cellptr->Land_Type() != LAND_WATER ||
            (distance < CELL_LEPTON_W / 3 && cellptr->Cell_Techno() != nullptr &&
            (Payback == nullptr || !Payback->House->Is_Ally(cellptr->Cell_Techno())))) {

            /*
            **  If the torpedo was blocked by a bridge, then force the
            **  torpedo to explode on top of that bridge cell.
            */
            if (cellptr->Is_Bridge_Here()) {
                coord = Coord_Snap(coord);
            }

            return true;
        }

        /*
        **  Torpedoes can be blocked by enemy objects on their path.
        */
        TechnoClass* celltechno = cellptr->Cell_Techno();

        if (celltechno != nullptr)
        {
            int snapdistance = CELL_LEPTON_W * 2;

            if (celltechno == TarCom || 
                (Distance(celltechno) < snapdistance && (Payback == nullptr || !Payback->House->Is_Ally(celltechno))))
            {
                /*
                **  If the techno is not a building, force
                **  explosion to be at center of techno object.
                **  Otherwise, explode in the center of the cell.
                */
                if (celltechno->Fetch_RTTI() != RTTI_BUILDING) {
                    coord = cellptr->Cell_Techno()->Target_Coord();
                }
                else {
                    coord = cellptr->Center_Coord();
                }

                return true;
            }
        }
    }

    /*
    **  Bullets are generally more effective when they are fired at flying objects.
    */
    if (Class->IsAntiAircraft && TarCom != nullptr && TarCom->In_Air() && Distance(TarCom) < 0x0080) {
        return true;
    }

    return false;
}


/**
 *  #issue-563
 * 
 *  Implements SpawnDelay for BulletTypes.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004447BF, _BulletClass_AI_SpawnDelay_Patch, 0)
{
    GET(BulletClass *, this_ptr, EBP);

    /**
     *  Fetch the extension instance.
     */
    BulletTypeClassExtension* bullettypeext = Extension::Fetch(this_ptr->Class);

    /**
     *  If this bullet has a custom spawn delay (defaults to the original delay of 3), perform that check first.
     */
    if (Frame % bullettypeext->SpawnDelay == 0) {
        goto create_trailer_anim;
    }

skip_anim:
    return 0x00444801;

create_trailer_anim:
    return 0x004447D0;
}


/**
 *  #issue-415
 * 
 *  Implements screen shake values for WarheadTypes.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x00446652, _BulletClass_Logic_ShakeScreen_Patch, 0)
{
    GET(WarheadTypeClass *, warhead, EAX);
    GET_STACK(Coord*, coord, 0x0A8);

    R->EDI(coord);

    /**
     *  Fetch the extension instance.
     */
    auto warheadext = Extension::Fetch(warhead);

    /**
     *  If this warhead has screen shake values defined, then set the blitter
     *  offset values. GScreenClass::Blit will handle the rest for us.
     */
    if (warheadext->ShakePixelXLo > 0 || warheadext->ShakePixelXHi > 0) {
        Map.ScreenX = Sim_Random_Pick(warheadext->ShakePixelXLo, warheadext->ShakePixelXHi);
    }
    if (warheadext->ShakePixelYLo > 0 || warheadext->ShakePixelYHi > 0) {
        Map.ScreenY = Sim_Random_Pick(warheadext->ShakePixelYLo, warheadext->ShakePixelYHi);
    }

    /**
     *  Jumps back to IsEMEffect check.
     */
    return 0x00446659;
}


/**
 *  Main function for patching the hooks.
 */
void BulletClassExtension_Hooks()
{
    Patch_Jump(0x004462C0, &BulletClassExt::_Is_Forced_To_Explode);
}
