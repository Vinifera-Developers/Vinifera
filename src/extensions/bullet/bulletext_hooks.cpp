/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          BULLETEXT_HOOKS.CPP
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended BulletClass.
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
#include "bulletext_hooks.h"
#include "bullettype.h"
#include "bullettypeext.h"
#include "bullet.h"
#include "anim.h"
#include "animtype.h"
#include "building.h"
#include "house.h"
#include "infantry.h"
#include "overlaytype.h"
#include "techno.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "iomap.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static class BulletClassExt final : public BulletClass
{
public:
    bool _Is_Forced_To_Explode(Coordinate& coord);
};


/**
 *  #issue-444
 *
 *  Full replacement of BulletClass::Is_Forced_To_Explode.
 *
 *  @author: 10/10/1996 JLB : Created.
 *           22/10/2024 Rampastring : Adjustments for Tiberian Sun.
 */
bool BulletClassExt::_Is_Forced_To_Explode(Coordinate& coord)
{
    coord = Coord;
    CellClass* cellptr = &Map[Get_Coord()];

    /*
    **  Check for impact on a wall or other high obstacle.
    */
    if (!Class->IsHigh && cellptr->Overlay != OVERLAY_NONE && OverlayTypeClass::As_Reference(cellptr->Overlay).IsHigh) {

        if (Get_Height() < 100) {
            coord = Cell_Coord(Coord_Cell(coord));
            return true;
        }
    }

    /*
    **  Check for impact on the ground.
    */
    if (Get_Height() < 0) {
        return true;
    }

    /*
    **  Check to make sure that underwater projectiles (torpedoes) will not
    **  travel in anything but water.
    */
    const auto bullettypeext = Extension::Fetch<BulletTypeClassExtension>(Class);
    if (bullettypeext->IsTorpedo)
    {
        int d = ::Distance(Coord_Fraction(coord), XY_Coord(CELL_LEPTON_W / 2, CELL_LEPTON_W / 2));

        if (cellptr->Land_Type() != LAND_WATER ||
            (d < CELL_LEPTON_W / 3 && cellptr->Cell_Techno() != nullptr &&
                (Payback == nullptr || !Payback->House->Is_Ally(cellptr->Cell_Techno()))))
        {
            /*
            **  Force explosion to be at center of techno object if one is present.
            */
            if (cellptr->Cell_Techno() != nullptr) {
                coord = cellptr->Cell_Techno()->Target_Coord();
            }

            /*
            **  However, if the torpedo was blocked by a bridge, then force the
            **  torpedo to explode on top of that bridge cell.
            */
            if (cellptr->Is_Bridge_Here()) {
                coord = Coord_Snap(coord);
            }

            return true;
        }
    }

    /*
    **  Bullets are generally more effective when they are fired at aircraft or flying jumpjets.
    */
    if (Class->IsAntiAircraft && Target_Legal(TarCom) &&
        (TarCom->What_Am_I() == RTTI_AIRCRAFT || (TarCom->What_Am_I() == RTTI_INFANTRY && reinterpret_cast<InfantryClass*>(TarCom)->Is_Flying_JumpJet())) &&
        Distance(TarCom) < 0x0080)
    {
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
DECLARE_PATCH(_BulletClass_AI_SpawnDelay_Patch)
{
    GET_REGISTER_STATIC(BulletClass *, this_ptr, ebp);
    static BulletTypeClassExtension *bullettypeext;

    /**
     *  Fetch the extension instance.
     */
    bullettypeext = Extension::Fetch<BulletTypeClassExtension>(this_ptr->Class);

    /**
     *  If this bullet has a custom spawn delay (defaults to the original delay of 3), perform that check first.
     */
    if ((Frame % bullettypeext->SpawnDelay) == 0) {
        goto create_trailer_anim;
    }

skip_anim:
    JMP(0x00444801);

create_trailer_anim:
    JMP(0x004447D0);
}


/**
 *  #issue-415
 * 
 *  Implements screen shake values for WarheadTypes.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_BulletClass_Logic_ShakeScreen_Patch)
{
    GET_REGISTER_STATIC(BulletClass *, this_ptr, ebx);
    GET_REGISTER_STATIC(WarheadTypeClass *, warhead, eax);
    GET_STACK_STATIC(Coordinate *, coord, esp, 0x0A8);
    static WarheadTypeClassExtension *warheadext;

    /**
     *  Fetch the extension instance.
     */
    warheadext = Extension::Fetch<WarheadTypeClassExtension>(warhead);

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
     *  Restore some registers.
     */
    _asm { mov eax, warhead }
    _asm { mov edi, coord /*[esp+0x0A8]*/ } // coord

    /**
     *  Jumps back to IsEMEffect check.
     */
    JMP_REG(edx, 0x00446659);
}


/**
 *  Main function for patching the hooks.
 */
void BulletClassExtension_Hooks()
{
    Patch_Jump(0x004462C0, &BulletClassExt::_Is_Forced_To_Explode);
    Patch_Jump(0x00446652, &_BulletClass_Logic_ShakeScreen_Patch);
    Patch_Jump(0x004447BF, &_BulletClass_AI_SpawnDelay_Patch);
}
