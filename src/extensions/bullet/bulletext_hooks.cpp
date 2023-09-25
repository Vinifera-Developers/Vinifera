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
#include "building.h"
#include "techno.h"
#include "warheadtype.h"
#include "warheadtypeext.h"
#include "house.h"
#include "rules.h"
#include "fastmath.h"
#include "iomap.h"
#include "extension.h"
#include "fatal.h"
#include "asserthandler.h"
#include "debughandler.h"

#include "hooker.h"
#include "hooker_macros.h"


/**
 *  #issue-563
 *
 *  Implements SpawnDelay for BulletTypes.
 *
 *  @author: CCHyper
 */
DECLARE_PATCH(_BulletClass_AI_SpawnDelay_Patch)
{
    GET_REGISTER_STATIC(BulletClass*, this_ptr, ebp);
    static BulletTypeClassExtension* bullettypeext;

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
    GET_REGISTER_STATIC(BulletClass*, this_ptr, ebx);
    GET_REGISTER_STATIC(WarheadTypeClass*, warhead, eax);
    GET_STACK_STATIC(Coordinate*, coord, esp, 0x0A8);
    static WarheadTypeClassExtension* warheadext;

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
 *  #issue-19
 *
 *  Reimplements the part of BulletClass::AI that deals with homing projectiles
 *  (projectiles that have ROT > 0).
 *
 *  @author: Rampastring
 */
void BulletClass_AI_Homing_Reimplementation(BulletClass* this_ptr)
{
    /*double fly_class_length = this_ptr->Fly.Length_3D();

    if (this_ptr->MaxSpeed >= 40 || (double)this_ptr->MaxSpeed <= fly_class_length + 0.5)
    {
        this_ptr->field_A45 = false; // CourseLocked?
    }

    int acceleration = this_ptr->Class->Acceleration;
    if (this_ptr->field_A45)
    {
        if (Frame % 2 == 0)
        {
            acceleration = 1;
        }
        else
        {
            acceleration = 0;
        }
    }
    int acceleration2 = acceleration;

    int dirrr;
    double max_speed_as_double = (double)this_ptr->MaxSpeed;
    bool process_flyclass = false;

    if (fly_class_length < max_speed_as_double)
    {
        process_flyclass = true;

        fly_class_length = (double)acceleration2 + fly_class_length;
        if (fly_class_length >= max_speed_as_double)
        {
            fly_class_length = max_speed_as_double;
        }
    }
    else if (fly_class_length > max_speed_as_double)
    {
        process_flyclass = true;

        dirrr = acceleration / 2;
        fly_class_length = fly_class_length - (double)(acceleration / 2);
        if (fly_class_length <= 0.0)
        {
            fly_class_length = 0.0;
        }
    }

    if (process_flyclass)
    {
        this_ptr->Fly.If_XYZ_0_Set_X_100();

        double new_fly_class_length = this_ptr->Fly.Length_3D();
        double scalar = fly_class_length / new_fly_class_length;
        this_ptr->Fly.field_88 = scalar * this_ptr->Fly.field_88;
        this_ptr->Fly.field_90 = scalar * this_ptr->Fly.field_90;
        this_ptr->Fly.field_98 = scalar * this_ptr->Fly.field_98;
    }*/

    Coordinate target_coord;

    if (this_ptr->TarCom)
    {
        // target_coord_v26 = this->TarCom->r.m.o.a.vt->t.r.m.o.a.__some_coords__As_Coord(this->TarCom);
        // I can't see a sensible alternative to Center_Coord, so let's just go with it for now
        target_coord = this_ptr->TarCom->Center_Coord();
    }
    else
    {
        // original TS code
        // fetch default invalid coords
        // target_coord = Coordinate(0, 0, 0);

        /**
         *  #issue-19
         *
         *  Let's just continue flying straight instead of assigning invalid coords.
         */
        target_coord = this_ptr->Center_Coord() +
            Coordinate(this_ptr->Fly.field_88, this_ptr->Fly.field_90, this_ptr->Fly.field_98);
    }

    ObjectClass* target_as_object = dynamic_cast<ObjectClass*>(this_ptr->TarCom);
    if (target_as_object)
    {
        target_coord = target_as_object->Target_Coord(); // hopefully copy constructor here too
    }

    int dirrr = (Frame + this_ptr->Fetch_ID()) % 15; // ????
    double pi = 3.141592653; // surely I could find this from somewhere...
    double v31 = (FastMath::Sin((double)dirrr * (1.0 / 15.0) * pi * 2.0) * Rule->MissileROTVar + Rule->MissileROTVar + 1.0) * (double)this_ptr->Class->ROT;
    // long long v32 = (long long)v31;
    // !!! despite (signed __int64) claim here for v32 by IDA, it seems like only a byte of it is ever used
    v31 *= 100.0;
    int v32 = (int)v31;
    dirrr = v32;

    // Calculate vector from target to us
    Coordinate distance_coord = this_ptr->Center_Coord() - target_coord;

    /**
     *  #issue-19
     *
     *  Don't adjust this if we don't have a target.
     */
    if (distance_coord.Length() < 256 && this_ptr->TarCom) // was CoordStruct::Distance(distance_coord)
    {
        // LOBYTE(v32) = (signed __int64)((double)dirrr * 1.5); ?????
        v32 = (int)((double)dirrr * 1.5);
    }

    bool is_targeting_aircraft = false;
    if (this_ptr->TarCom && this_ptr->TarCom->What_Am_I() == RTTI_AIRCRAFT)
    {
        is_targeting_aircraft = true;
    }

    fixed v39 = 0;
    v39.Data.Composite.Fraction = 0;
    v39.Data.Composite.Whole = this_ptr->field_A45 == false ? v32 : 0;

    // LOWORD(dirrr) = v39;
    // This is probably a bad way to hopefully achieve the same.
    dirrr = v39;

    FlyClass v149 = this_ptr->Fly; // should invoke copy constructor

    // Back-up our coordinate prior to calling Projectile_Motion
    Coordinate backup_coord = this_ptr->Coord;

    Coordinate our_coord = this_ptr->Coord;
    dirrr = Projectile_Motion(our_coord, v149, target_coord,
        (DirStruct)dirrr,
        is_targeting_aircraft,
        this_ptr->Class->IsAirburst,
        this_ptr->Class->IsVeryHigh);

    CellClass& cell = Map[our_coord]; // this is actually used only in the bridge check, 
                                      // but original compiled code had it here already
    this_ptr->Fly = v149;

    bool is_forced_to_explode = false;
    bool b_height_v139 = false;

    /**
     *  #issue-19
     *
     *  Don't force projectile to blow up here if we don't have a target.
     *  Otherwise the projectile would explode where it was when the target
     *  disappeared. Often it'd explode on our face, which we don't want to happen.
     */
    if ((this_ptr->TarCom && this_ptr->Fly.Length_3D() * 0.5 >= (double)dirrr) || this_ptr->Get_Height() <= 0)
    {
        is_forced_to_explode = true;
        b_height_v139 = true;

        if (this_ptr->Get_Height() > 0 && !this_ptr->Class->IsAirburst)
        {
            our_coord = target_coord;
        }
    }

    // Calculate what our distance to the target was prior to calling Projectile_Motion?
    distance_coord = backup_coord - target_coord;
    int old_distance = distance_coord.Length();

    distance_coord = our_coord - target_coord;
    // Calculate how much distance we closed to the target
    int distance_closed = old_distance - distance_coord.Length();

    bool check_bridge = false;

    if (this_ptr->field_A45)
    {
        check_bridge = true;
    }
    else if (this_ptr->field_B0 < 60)
    {
        this_ptr->field_B0++;
        this_ptr->field_B8 = (double)distance_closed + this_ptr->field_B8;
        check_bridge = true;
    }
    else
    {
        this_ptr->field_B8 = this_ptr->field_B8 * 0.9833333333333333 + (double)distance_closed;

        // v53 is never assigned to?
        /*if (v53)
        {
            goto LABEL_57;
        }*/

        if (this_ptr->field_B8 >= 60.0)
        {
            check_bridge = true;
        }
        else if (this_ptr->Class->IsAirburst || this_ptr->Class->IsVeryHigh)
        {
            check_bridge = true;
        }
    }

    if (check_bridge)
    {
        // Check if the projectile is going to hit a high bridge?
        // I probably don't have enough knowledge on this stuff to reimplement it
        /*
        if (!v139
            && (*(_DWORD *)(v144[0] + 164) & 0x1000 || MapClass::operator[](&Map.sc.t.s.p.r.d.m, &a2)->Bits & 0x1000))
        {
            if ((v55 = BridgeCellHeight_7605C4 + MapClass_GetCoordFloorHeight(&Map.sc.t.s.p.r.d.m, &coord),
                coord.Z > v55)
                && a2.Z < v55
                || coord.Z < v55 && a2.Z > v55)
            {
                v139 = 1;
                coord.Z = v55;
                forced = 1;
            }
        }*/
    }
    else
    {
        is_forced_to_explode = true;
        b_height_v139 = true;
    }

    this_ptr->Mark();

    /**
     *  This code path would be relevant for a total reimplementation of the BulletClass::AI function,
     *  but it's never hit for ROT > 0 projectiles (homing projectiles)
     *
     *  And yes, it means that b_height_v139 is not actually a bool originally code (or it was multiple
     *  variables with memory space re-used by the compiler)
     */
     /*
     if (b_height_v139 == 2)
     {
         delete this_ptr;
     }
     else
     */

     /*
     **	See if the projectile ran out of fuel (reached its maximum range).
     */
    if (this_ptr->Class->IsFueled)
    {
        Coordinate difference = our_coord - this_ptr->Get_Coord();
        this_ptr->Range = this_ptr->Range - (int)difference.Length();

        if (this_ptr->Range <= 0)
            is_forced_to_explode = true;
    }

    this_ptr->Set_Coord(our_coord);
    CellClass& cell_at_new_coord = Map[our_coord];
    BuildingClass* cell_building = cell_at_new_coord.Cell_Building();

    /*
    **	Check if the bullet hit a firestorm wall on its (potentially new) cell.
    */
    if (cell_building &&
        cell_building->Class->IsFirestormWall &&
        cell_building->House->IsFirestormActive &&
        (this_ptr->Payback == nullptr || cell_building->House != this_ptr->Payback->House) &&
        this_ptr->Class_Of() &&
        !this_ptr->Class_Of()->IsIgnoresFirestorm)
    {
        // BuildingClass_Damage_via_FSWall(this, 0); not in TS++ yet?
        delete this_ptr;
    }
    else
    {
        /*
        **	See if the bullet should be forced to explode now in spite of what
        **	the fuse would otherwise indicate. Maybe the bullet hit a wall?
        */
        if (!is_forced_to_explode)
        {
            Coordinate newcoord = this_ptr->Get_Coord();
            is_forced_to_explode = this_ptr->Is_Forced_To_Explode(newcoord);
            this_ptr->Set_Coord(newcoord); // why? :/
        }

        bool fuse_check = false;
        if (this_ptr->Class->ROT > 0)
        {
            fuse_check = this_ptr->Fuse.Fuse_Checkup(our_coord);
        }

        /**
         *  #issue-19
         *
         *  With our patch, sometimes a projectile might have no valid target.
         *  This happens if a projectile's target dies before the projectile has hit it.
         *  In this case, the projectile will continue flying straight until it runs out of fuel,
         *  hits the ground or hits an enemy object.
         *
         *  Check if there's an enemy object on our cell. If one is found, then force the projectile
         *  to explode.
         */
        if (this_ptr->TarCom == nullptr)
        {
            HouseClass* our_house = nullptr;
            if (this_ptr->Payback)
                our_house = this_ptr->Payback->House;

            ObjectClass* cell_occupier = cell_at_new_coord.Cell_Occupier();
            while (!is_forced_to_explode && cell_occupier != nullptr)
            {
                if (cell_occupier->Owning_House() != our_house)
                    is_forced_to_explode = true;
                else
                    cell_occupier = cell_occupier->Next;
            }
        }

        /*
        **	If the bullet is not to explode, then perform normal flight
        **	maintenance (usually nothing). Otherwise, explode and then
        **	delete the bullet.
        */
        if (!is_forced_to_explode && (this_ptr->Class->IsDropping || !fuse_check))
        {
            /*
            **	Certain projectiles lose strength when they travel.
            */
            /*
             * Vanilla TS code
            if (this_ptr->Class->IsDegenerate && this_ptr->Strength > 5) {
                this_ptr->Strength--;
            }*/
            /*
            **  #issue-234
            **  DTA adjustment from CnCNet ts-patches
            */
            if (this_ptr->Class->IsDegenerate && this_ptr->Strength > 10) {
                this_ptr->Strength = this_ptr->Strength - 2;
            }
        }
        else
        {
            if (this_ptr->TarCom && (fuse_check == true /*|| v134 */) && !this_ptr->Class->IsAirburst)
            {
                // v125 = this->TarCom->r.m.o.a.vt->t.r.m.o.a.__some_coords__As_Coord(this->TarCom);
                Coordinate newtargetcoord = this_ptr->TarCom->Center_Coord();

                Coordinate some_coord = Coordinate(our_coord.X - newtargetcoord.X,
                    our_coord.Y - newtargetcoord.Y,
                    ((newtargetcoord.Z + our_coord.Z) / 2) - newtargetcoord.Z);
                int some_coord_length = some_coord.Length();

                /*
                 * This code path is only hit by ROF <= 0 projectiles
                if (v134)
                {
                    some_coord_length = some_coord_length / 3;
                }
                */

                double limit = 128.0;
                double lengthtwice = this_ptr->Fly.Length_3D() * 2.0;
                if (lengthtwice >= limit)
                {
                    limit = lengthtwice;
                }

                if (fuse_check || some_coord_length <= lengthtwice)
                {
                    this_ptr->Set_Coord(this_ptr->TarCom->Center_Coord());
                }
            }

            this_ptr->Bullet_Explodes(is_forced_to_explode);
            delete this_ptr;
        }
    }
}


DECLARE_PATCH(_BulletClass_AI_Jump_To_Custom_Function_If_ROT_Over_Zero)
{
    GET_REGISTER_STATIC(BulletClass*, this_ptr, ebp);
    if (!this_ptr->Class->IsVeryHigh && !this_ptr->Class->IsSplits && this_ptr->Class->ROT < 100 && this_ptr->Class->Arming != 1000) {
        BulletClass_AI_Homing_Reimplementation(this_ptr);
    }
    else {
        // original code
        _asm { mov ecx, [ebp + 0xA8] }
        JMP(0x00444A44);
    }

    // Return from function
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { mov esp, ebp }
    _asm { pop ebp }
    _asm { retn }
}


/**
 *  Main function for patching the hooks.
 */
void BulletClassExtension_Hooks()
{
    Patch_Jump(0x00446652, &_BulletClass_Logic_ShakeScreen_Patch);
    Patch_Jump(0x004447BF, &_BulletClass_AI_SpawnDelay_Patch);
    Patch_Jump(0x00444A3E, &_BulletClass_AI_Jump_To_Custom_Function_If_ROT_Over_Zero);
}