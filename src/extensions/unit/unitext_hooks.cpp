/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *
 *  @project       Vinifera
 *
 *  @file          UNITEXT_HOOKS.H
 *
 *  @author        CCHyper
 *
 *  @brief         Contains the hooks for the extended UnitClass.
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
#include "unitext_hooks.h"
#include "unitext_init.h"
#include "tibsun_inline.h"
#include "vinifera_globals.h"
#include "tibsun_globals.h"
#include "tibsun_functions.h"
#include "tag.h"
#include "technotype.h"
#include "technotypeext.h"
#include "house.h"
#include "warheadtype.h"
#include "unit.h"
#include "unitext.h"
#include "unittype.h"
#include "unittypeext.h"
#include "tag.h"
#include "target.h"
#include "rules.h"
#include "rulesext.h"
#include "iomap.h"
#include "infantry.h"
#include "voc.h"
#include "extension.h"
#include "unitext.h"
#include "fatal.h"
#include "debughandler.h"
#include "asserthandler.h"

#include "hooker.h"
#include "hooker_macros.h"
#include "spawnmanager.h"
#include "verses.h"
#include "warheadtypeext.h"
#include "weapontype.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor.
 *
 *  @note: All functions must not be virtual and must also be prefixed
 *         with "_" to prevent accidental virtualization.
 */
class UnitClassExt : public UnitClass
{
public:
    void _Firing_AI();
    void _Draw_Voxel(unsigned int frame, int key, Rect& rect, Point2D& point, const Matrix3D& other_matrix, int color, int flags);
    int _Mission_Hunt();

};


/**
 *  Handles firing logic for this unit.
 *
 *  @author: 07/30/1996 JLB - Created
 *           ZivDero - Adjustments for Tiberian Sun
 */
void UnitClassExt::_Firing_AI()
{
    if (Target_Legal(TarCom) && Get_Weapon(WEAPON_SLOT_PRIMARY)->Weapon)
    {
        /**
         *  Determine which weapon can fire. First check for the primary weapon. If that weapon
         *  cannot fire, then check any secondary weapon. If neither weapon can fire, then the
         *  failure code returned is that from the primary weapon.
         */
        WeaponSlotType primary = What_Weapon_Should_I_Use(TarCom);
        FireErrorType ok = Can_Fire(TarCom, primary);
        const WeaponTypeClass* weapon = Get_Weapon(primary)->Weapon;

        if (weapon && weapon->WarheadPtr && weapon->WarheadPtr->IsWebby && TarCom->What_Am_I() == RTTI_INFANTRY)
        {
            InfantryClass* inf = reinterpret_cast<InfantryClass*>(TarCom);
            if (inf->ProneStruggleTimer.Value() > weapon->WarheadPtr->WebDuration / 4)
            {
                Assign_Target(nullptr);
                Assign_Mission(MISSION_GUARD);
                ok = FIRE_CANT;
            }
        }

        if ((ok == FIRE_OK || ok == FIRE_FACING) && Deploy_To_Fire())
        {
            Assign_Mission(MISSION_UNLOAD);
            return;
        }

        static int visceroid_stages[8] = { 0x64, 0x69, 0x6E, 0x73, 0x78, 0x7D, 0x5A, 0x5F };
        UnitClassExtension* ext;

        switch (ok)
        {
        case FIRE_OK:
            if (!Class->IsFireAnim)
                IsFiring = false;

            if (Class->IsLargeVisceroid || Class->IsSmallVisceroid)
            {
                Set_Stage(visceroid_stages[Dir_Facing(Direction(TarCom).Get_Dir())]);
                Set_Rate(5);
            }

            if (primary != WEAPON_SLOT_SECONDARY && weapon)
            {
                const int firing_sync = CurrentBurstIndex % weapon->Burst;
                if (firing_sync < 2 && Class->FiringSyncFrame[firing_sync] != -1)
                {
                    if (FiringSyncDelay == -1)
                        FiringSyncDelay = 2 * Class->FiringFrames - 1;
                    else if (FiringSyncDelay != Class->FiringSyncFrame[firing_sync])
                        return;
                }
            }

            Fire_At(TarCom, primary);
            break;

        case FIRE_FACING:
            if (Class->IsLockTurret || !Class->IsTurretEquipped)
            {
                if (!Target_Legal(NavCom) && !Locomotion->Is_Moving()) {
                    PrimaryFacing.Set_Desired(Direction(TarCom));
                    SecondaryFacing.Set_Desired(PrimaryFacing.Desired());
                }
            }
            else
            {
                SecondaryFacing.Set_Desired(Direction(TarCom));
            }
            break;

        case FIRE_ILLEGAL:
            if (Combat_Damage(primary) < 0)
            {
                if (!Is_Object(TarCom) || TarCom->What_Am_I() != RTTI_UNIT || static_cast<ObjectClass*>(TarCom)->Health_Ratio() >= Rule->ConditionGreen)
                {
                    Assign_Target(nullptr);
                }
            }
            break;

        case FIRE_CANT:
            ext = Extension::Fetch<UnitClassExtension>(this);
            if (ext->SpawnManager)
                ext->SpawnManager->Abandon_Target();
            break;
            

        case FIRE_RANGE:
        case FIRE_MUST_DEPLOY:
            IsFiring = false;
            Approach_Target();
            break;

        case FIRE_CLOAKED:
            IsFiring = false;
            Do_Uncloak();
            break;

        default:
            return;
        }
    }
}


/**
 *  Draws the voxel model for this unit.
 *
 *  @author: ZivDero
 */
void UnitClassExt::_Draw_Voxel(unsigned int frame, int key, Rect& rect, Point2D& point, const Matrix3D& other_matrix, int color, int flags)
{
    Matrix3D matrix;
    Matrix3D::Multiply(Get_Voxel_Draw_Matrix(), other_matrix, &matrix);
    const auto typeext = Extension::Fetch<UnitTypeClassExtension>(Class);
    const auto ext = Extension::Fetch<UnitClassExtension>(this);

    VoxelObject* voxel = nullptr;
    VoxelIndexClass* cache = nullptr;

    if (typeext->WaterAlt
        && Map[Get_Coord()].Land_Type() == LAND_WATER
        && !IsOnBridge
        && Get_Height() < CELL_HEIGHT(1))
    {
        voxel = &typeext->WaterVoxel;
        cache = &typeext->WaterVoxelIndex;
    }
    else if (typeext->NoSpawnAlt && ext->SpawnManager && !ext->SpawnManager->Docked_Count())
    {
        voxel = &typeext->NoSpawnVoxel;
        cache = &typeext->NoSpawnVoxelIndex;
    }
    else
    {
        voxel = &Class->Voxel;
        cache = &Class->VoxelIndex;
    }

    Draw_Voxel(*voxel, frame, key, *cache, rect, point, matrix, color, flags);
}


/**
 *  Patch that replaces the call to draw the voxel model to allow us to
 *  chose which voxel to draw.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_UnitClass_Draw_Voxel_Patch)
{
    GET_STACK_STATIC(unsigned int, frame, esp, 0x58);
    GET_STACK_STATIC(int, key, esp, 0x40);
    LEA_STACK_STATIC(Rect*, rect, esp, 0x68);
    LEA_STACK_STATIC(Point2D*, point, esp, 0x50);
    LEA_STACK_STATIC(Matrix3D*, matrix, esp, 0x90);
    GET_STACK_STATIC(int, color, esp, 0x17C);
    GET_STACK_STATIC(int, flags, esp, 0x4C);
    GET_REGISTER_STATIC(UnitClassExt*, this_ptr, ebp);

    this_ptr->_Draw_Voxel(frame, key, *rect, *point, *matrix, color, flags);

    _asm mov ebx, color

    JMP(0x006528E9);
}


/**
 *  #issue-177
 *
 *  Reaplces UnitClass::MissionHunt to consider the entire BuildConst vector.
 *
 *  @author: ZivDero
 */
int UnitClassExt::_Mission_Hunt()
{
    if (Class->DeploysInto && (Rule->BuildConst.Is_Present(Class->DeploysInto) || TarCom || House->Is_Human_Control()))
    {
        if (Status)
        {
            if (Status == 1 && !IsDeploying)
                Status = 0;
        }
        else if (Goto_Clear_Spot())
        {
            if (Try_To_Deploy())
                Status = 1;
        }

        return Get_Current_Mission_Control().Rate * TICKS_PER_MINUTE + Random_Pick(0, 2);
    }

    return FootClass::Mission_Hunt();
}


#if 0
/**
 *  #issue-510
 * 
 *  This patch fixes a bug where harvesters that are idling on a bridge with
 *  tiberium below it would begin erroneously harvesting the patch below it.
 * 
 *  @author: CCHyper
 */
static CellClass *Unit_Get_Current_Cell(UnitClass *this_ptr) { return &Map[this_ptr->Get_Coord()]; }
DECLARE_PATCH(_UnitClass_Mission_Harvest_Block_Harvesting_On_Bridge_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    static CellClass *cellptr;

    /**
     *  Perform a check to see if the cell we occupy contains a bridge.
     */
    cellptr = Unit_Get_Current_Cell(this_ptr);
    if (cellptr && cellptr->Is_Bridge_Here()) {
        goto function_return;
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    _asm { mov eax, [esi+0x360] } // this->Class
    _asm { mov ecx, [eax+0x268] } // Class->Dock.ActiveCount
    JMP_REG(ebp, 0x00654AB6);

function_return:
    JMP(0x00654AA3);
}
#endif


#if 0
/**
 *  #issue-510
 * 
 *  This patch fixes a bug where harvesters that are idling on a bridge with
 *  tiberium below it would begin erroneously harvesting the patch below it.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Enter_Idle_Mode_Block_Harvesting_On_Bridge_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    static CellClass *cellptr;

    /**
     *  Code before here assumes we are a harvester of some kind.
     */

    /**
     *  Perform a check to see if the cell we occupy contains a bridge.
     */
    cellptr = Unit_Get_Current_Cell(this_ptr);
    if (cellptr && cellptr->Is_Bridge_Here()) {
        goto function_return;
    }

    /**
     *  Stolen bytes/code.
     */
original_code:
    static MissionType mission;
    mission = this_ptr->Get_Mission();
    _asm { mov eax, mission }
    JMP_REG(edi, 0x00650559);

function_return:
    JMP_REG(ecx, 0x0065066A);
}
#endif


/**
 *  #issue-510
 * 
 *  This patch fixes the bug where the user could action a harvester to 
 *  target tiberium that was 'hidden' below a bridge by forcing any orders
 *  on a bridge with tiberium below it to be move orders.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_What_Action_ACTION_HARVEST_Block_On_Bridge_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, edi);
    GET_REGISTER_STATIC(Cell *, cell, esi);
    static CellClass *cellptr;
    static ActionType action;

    /**
     *  Code before here assumes we are a harvester of some kind.
     */

    action = ACTION_HARVEST;

    /**
     *  If the user has the mouse over a cell that contains tiberium, but also
     *  contains a bridge above it, force the MOVE action.
     */
    cellptr = &Map[*cell];
    if (cellptr && cellptr->Is_Bridge_Here()) {
        action = ACTION_MOVE;
    }

    _asm { mov eax, action }
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 0x10 }
    _asm { ret 0x0C }
}


/**
 *  #issue-421
 * 
 *  Implements IdleRate for UnitTypes.
 *
 *  This replaces the WalkFrames branch as we needed to move FiringSyncDelay
 *  before so it has the highest priority over what shape frame to use.
 * 
 *  @author: CCHyper
 */
static bool Locomotion_Is_Moving(UnitClass *this_ptr) { return this_ptr->Locomotion->Is_Moving(); }
DECLARE_PATCH(_UnitClass_Draw_Shape_IdleRate_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    GET_REGISTER_STATIC(int, facing, ebx);
    GET_REGISTER_STATIC(ShapeFileStruct *, shape, edi);
    static UnitTypeClassExtension *unittypeext;
    static const UnitTypeClass *unittype;
    static int frame;

    unittype = reinterpret_cast<const UnitTypeClass *>(this_ptr->Techno_Type_Class());
    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);

    if (!Locomotion_Is_Moving(this_ptr)) {
        if (this_ptr->FiringSyncDelay >= 0) {
            frame = (this_ptr->FiringSyncDelay/2)
                + this_ptr->Class->StartFiringFrame
                + (this_ptr->Class->FiringFrames * facing);

            goto continue_to_draw;
        }
    }

    if (!Locomotion_Is_Moving(this_ptr)) {
        if (this_ptr->DeathCounter >= 0) {

            static int death_frame;

            death_frame = (this_ptr->DeathCounter / unittype->DeathFrameRate);
            if (death_frame >= (unittype->DeathFrames-1)) {
                death_frame = (unittype->DeathFrames-1);
            }

            frame = death_frame + unittype->StartDeathFrame;

            goto continue_to_draw;
        }
    }

    if (Locomotion_Is_Moving(this_ptr)) {
        frame = unittype->StartWalkFrame
            + (this_ptr->TotalFramesWalked % unittype->WalkFrames)
            + (unittype->WalkFrames * facing);

        goto continue_to_draw;
    }

    /**
     *  Unit is not moving, so if the unit has a idle animation rate, use this.
     */
    if (!Locomotion_Is_Moving(this_ptr) && unittypeext->IdleRate > 0) {
        frame = unittypeext->StartIdleFrame
            + (this_ptr->TotalFramesWalked % unittypeext->IdleFrames)
            + (unittypeext->IdleFrames * facing);

        goto continue_to_draw;
    }

    if (this_ptr->field_34D) {
        if (unittype->StandingFrames > 0) {
            frame = unittype->StartStandFrame + (facing * unittype->StandingFrames);

        } else {
            frame = unittype->StartWalkFrame + (facing * unittype->WalkFrames);
        }

        goto continue_to_draw;
    }

    /**
     *  Continue to the shape drawing.
     */
continue_to_draw:
    _asm { mov ebx, frame }
    _asm { mov edi, shape }     // Restore EDI register (shape file pointer).
    JMP(0x006531FB);
}


/**
 *  #issue-264
 * 
 *  Implements LeaveTransportSound for this unit is unloading its passengers.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Mission_Unload_Transport_Detach_Sound_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    static FootClass *passenger;
    static TechnoTypeClassExtension *radio_technotypeext;

    /**
     *  Do we have a sound to play when passengers leave us? If so, play it now.
     */
    radio_technotypeext = Extension::Fetch<TechnoTypeClassExtension>(this_ptr->Techno_Type_Class());
    if (radio_technotypeext->LeaveTransportSound != VOC_NONE) {
        Sound_Effect(radio_technotypeext->LeaveTransportSound, this_ptr->Coord);
    }

    /**
     *  Stolen bytes/code.
     * 
     *  Are we a part of a team? If so, make any passengers we unload part of it too.
     */
    if (this_ptr->Team) {
        goto add_to_team;
    }

    /**
     *  Finished unloading passengers.
     */
finish_up:
    JMP(0x006543BB);

    /**
     *  Add this passenger to my team.
     */
add_to_team:
    _asm { mov ebp, passenger } // Restore EBP pointer.
    JMP(0x006543A3);
}


/**
 *  #issue-188
 * 
 *  Adds support for custom (per-type) unloading class when a harvester is unloading at a refinery.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Draw_It_Unloading_Harvester_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, esi);
    GET_REGISTER_STATIC(UnitTypeClass *, unittype, eax);
    static const UnitTypeClass *unloading_class;
    static const UnitTypeClassExtension *unittypeext;

    /**
     *  The code just before this backs up the current Class, so we
     *  don't need to worry about doing that here.
     */

    /**
     *  Are we currently unloading at a refinery?
     */
    if (this_ptr->IsDumping) {

        /**
         *  Is this unit some type of harvester that is unloading?
         * 
         *  The original code only checked for "IsToHarvest".
         */
        if (unittype->IsToHarvest || unittype->IsToVeinHarvest) {

            unloading_class = nullptr;

            /**
             *  Fetch the default unloading class.
             * 
             *  If this is a weed harvester that is unloading, then they need
             *  a special case to ensure they do not switch unless defined as
             *  they do not have a unloading graphics switch in the original
             *  Tiberian Sun when they enter the facility.
             */
            if (!unittype->IsToVeinHarvest) {
                unloading_class = Rule->UnloadingHarvester;
            }

            /**
             *  Fetch the unloading class from the extended class instance if it exists.
             */
            unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);
            if (unittypeext->UnloadingClass) {
                if (unittypeext->UnloadingClass->Kind_Of() == RTTI_UNITTYPE) {
                    unloading_class = reinterpret_cast<const UnitTypeClass *>(unittypeext->UnloadingClass);
                }
            }

            /**
             *  Only switch the graphic if the unloading class is valid.
             */
            if (unloading_class) {
                this_ptr->Class = const_cast<UnitTypeClass *>(unloading_class);
            }

        }

    }

    JMP(0x00653DA5);
}


/**
 *  Returns the graphic shape number based on the input current facing and desired facing count.
 * 
 *  @author: CCHyper
 */
static int Facing_To_Frame_Number(FacingClass &facing, int facing_count)
{
    int shape_number = 0;

    /**
     *  Fetch the current facing value in the required units.
     */
    switch (facing_count) {

    case 8:
        shape_number = (facing.Current().Get_Facing<8>() + 1) % 8;
        break;

    case 16:
        shape_number = (facing.Current().Get_Facing<16>() + 2) % 16;
        break;

    case 32:
        shape_number = (facing.Current().Get_Facing<32>() + 4) % 32;
        break;

    case 64:
        shape_number = (facing.Current().Get_Facing<64>() + 8) % 64;
        break;

    default:
        shape_number = 0;
        break;
    }

    return shape_number;
}


/**
 *  #issue-#214
 * 
 *  Support for additional facings counts for units with shape graphics.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Draw_Shape_Primary_Facing_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, ebp);
    GET_REGISTER_STATIC(const UnitTypeClass *, unittype, eax);
    //const UnitTypeClass *unittype;
    static int shape_number;

    /**
     *  #NOTE:
     *  Using either of these causes a memory leak for some reason...
     *  So we now just fetch EAX which is a UnitTypeClass instance already.
     */
    //unittype = reinterpret_cast<UnitTypeClass *>(this_ptr->Techno_Type_Class());
    //unittype = this_ptr->Class;

    /**
     *  Fetch the frame index for current turret facing.
     */
    shape_number = Facing_To_Frame_Number(this_ptr->PrimaryFacing, unittype->Facings);

    /**
     *  EBX == desired shape number.
     */
    _asm { mov ebx, shape_number }

    JMP(0x00653114);
}


/**
 *  #issue-#214
 * 
 *  Support for additional facings counts for units with shape graphics.
 * 
 *  This function replaces and reimplements the call to Draw_Object();
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Draw_Shape_Turret_Facing_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, ebp);
    GET_REGISTER_STATIC(const void *, shape, edi);
    static const UnitTypeClass *unittype;
    static const UnitTypeClassExtension *unittypeext;
    static int shape_number;
    static int frame_number;
    static int turret_facings;
    static int start_turret_frame;

    frame_number = 0;

    unittype = reinterpret_cast<UnitTypeClass *>(this_ptr->Techno_Type_Class());
    
    /**
     *  All turrets have 32 facings in Tiberian Sun.
     */
    turret_facings = 32;

    /**
     *  Turret frames start directly after the facing frames.
     */
    start_turret_frame = unittype->Facings * unittype->WalkFrames;

    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);

    /**
     *  #issue-393
     * 
     *  Allow the custom turret facings.
     * 
     *  @author: CCHyper
     */
    turret_facings = unittypeext->TurretFacings;

    /**
     *  Fetch the frame index for current turret facing.
     */
    shape_number = Facing_To_Frame_Number(this_ptr->SecondaryFacing, turret_facings);

    /**
     *  Now adjust the frame index based on the units walk frames.
     */

    /**
     *  #issue-389
     * 
     *  Allow the starting turret frame index to be defined.
     * 
     *  @author: CCHyper
     */
    if (unittypeext && unittypeext->StartTurretFrame != -1) {
        frame_number = unittypeext->StartTurretFrame + (shape_number % turret_facings);
    } else {
        frame_number = start_turret_frame + (shape_number % turret_facings);
    }

    /**
     *  The location we jump back to pushes EAX into the stack for
     *  the call to Draw_Object().
     */
    _asm { mov eax, frame_number }

    /**
     *  Restore some registers to make sure nothing got reused and all is good.
     */
    _asm { mov edi, shape }
    _asm { mov ecx, [this_ptr] }
    _asm { mov ebx, [ecx] }

    JMP_REG(edx, 0x006537AE);
}


/**
 *  #issue-334
 * 
 *  Fixes a division by zero crash when Rule->ShakeScreen is zero
 *  and a unit dies/explodes.
 * 
 *  @author: CCHyper
 */
static void UnitClass_Shake_Screen(UnitClass *unit)
{
    UnitTypeClassExtension *unittypeext;

    /**
     *  Fetch the extension instance.
     */
    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unit->Techno_Type_Class());

    /**
     *  #issue-414
     * 
     *  Can this unit shake the screen when it is destroyed?
     * 
     *  @author: CCHyper
     */
    if (unittypeext->IsShakeScreen) {

        /**
         *  If this unit has screen shake values defined, then set the blitter
         *  offset values. GScreenClass::Blit will handle the rest for us.
         */
        if ((unittypeext->ShakePixelXLo > 0 || unittypeext->ShakePixelXHi > 0)
         || (unittypeext->ShakePixelYLo > 0 || unittypeext->ShakePixelYHi > 0)) {

            if (unittypeext->ShakePixelXLo > 0 || unittypeext->ShakePixelXHi > 0) {
                Map.ScreenX = Sim_Random_Pick(unittypeext->ShakePixelXLo, unittypeext->ShakePixelXHi);
            }
            if (unittypeext->ShakePixelYLo > 0 || unittypeext->ShakePixelYHi > 0) {
                Map.ScreenY = Sim_Random_Pick(unittypeext->ShakePixelYLo, unittypeext->ShakePixelYHi);
            }

        } else {

            /**
             *  Very strong units that have an explosion will also rock the
             *  screen when they are destroyed.
             */
            if (unit->Class->MaxStrength > Rule->ShakeScreen) {

                /**
                 *  Make sure both the screen shake factor and the units strength
                 *  are valid before performing the division.
                 */
                if (Rule->ShakeScreen > 0 && unit->Class->MaxStrength > 0) {

                    int shakes = std::min<int>(unit->Class->MaxStrength / (Rule->ShakeScreen/2), 6);

                    /**
                     *  #issue-414
                     * 
                     *  Restores the vertical screen shake when a strong unit is destroyed.
                     * 
                     *  @author: CCHyper
                     */
                    Map.ScreenY = shakes;

                    //Shake_The_Screen(shakes);
                }

            }

        }

    }
}

DECLARE_PATCH(_UnitClass_Explode_ShakeScreen_Division_BugFix_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, edi);

    /**
     *  Stolen bytes/code.
     */
    _asm { pop ebx }

    UnitClass_Shake_Screen(this_ptr);

    /**
     *  Return from the function.
     */
function_return:
    JMP_REG(ecx, 0x0065B581);
}


/**
 *  #issue-#6
 * 
 *  A "quality of life" patch for harvesters so they auto harvest
 *  when they have just been kicked out of the war factory.
 * 
 *  @author: CCHyper
 */
DECLARE_PATCH(_UnitClass_Per_Cell_Process_AutoHarvest_Assign_Harvest_Mission_Patch)
{
    GET_REGISTER_STATIC(UnitClass *, this_ptr, ebp);
    GET_REGISTER_STATIC(TARGET, target, esi);
    static BuildingClass *building_contact;
    static UnitTypeClass *unittype;

    /**
     *  Is the unit we are processing a harvester?
     */
    unittype = reinterpret_cast<UnitTypeClass *>(this_ptr->Class_Of());
    if (unittype->IsToHarvest || unittype->IsToVeinHarvest) {

        /**
         *  Order the unit to harvest.
         */
        this_ptr->Assign_Mission(MISSION_HARVEST);

        goto continue_check_scatter;
    }

    /**
     *  Stolen bytes/code from here on, continues function flow.
     */

    /**
     *  Find out if the target is a building. (flagged to not use dynamic_cast).
     */
continue_function:
    building_contact = Target_As_Building(target, false);

    /**
     *  This is real ugly, but we replace the dynamic_cast in the original
     *  location and we need to return to just after its stack fixup.
     */
    _asm { mov ebp, this_ptr }
    _asm { mov ecx, [ebp+0x0EC] } // this->House
    _asm { mov eax, building_contact }

    JMP_REG(edx, 0x006517DB);

continue_check_scatter:
    _asm { mov ebp, this_ptr }
    JMP_REG(ecx, 0x0065194E);
}


/**
 *  Replaces Verses (Modifier) of the Warhead with the one from the extension.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_UnitClass_Jellyfish_AI_Armor_Patch)
{
    GET_REGISTER_STATIC(TechnoClass*, target, esi);
    GET_REGISTER_STATIC(UnitClass*, this_ptr, ebp);
    GET_STACK_STATIC(WeaponTypeClass*, weapon, esp, 0x20);
    GET_STACK_STATIC(WarheadTypeClass*, warhead, esp, 0x14);

    static int damage;
    damage = weapon->Attack * Verses::Get_Modifier(target->Techno_Type_Class()->Armor, warhead);
    target->Take_Damage(damage, 0, warhead, this_ptr, false, false);

    JMP(0x0064F2FA);
}


/**
 *  Helper function.
 *  Creates a unit based on an already existing unit.
 *  Returns the new unit if successful, otherwise null.
 *
 *  @author: Rampastring
 */
UnitClass* Create_Transform_Unit(UnitClass* this_ptr) {

    UnitTypeClassExtension* unittypeext = Extension::Fetch<UnitTypeClassExtension>(this_ptr->Class);

    UnitClass* newunit = reinterpret_cast<UnitClass*>(unittypeext->TransformsInto->Create_One_Of(this_ptr->House));
    if (newunit == nullptr) {

        /**
         *  Creating the new unit failed! Re-mark our occupation bits and return false.
         */
        return nullptr;
    }

    // Try_To_Deploy copies the tag this way at 0x0065112C
    if (this_ptr->Tag != nullptr) {
        newunit->Attach_Tag(this_ptr->Tag);
        this_ptr->Tag->AttachCount--;
        this_ptr->Tag = nullptr;
    }

    newunit->ActLike = this_ptr->ActLike;
    newunit->LimpetSpeedFactor = this_ptr->LimpetSpeedFactor;
    newunit->field_214 = this_ptr->field_214; // also copied at 0x00650F4E
    newunit->Veterancy.From_Integer(this_ptr->Veterancy.To_Integer());
    newunit->Group = this_ptr->Group;
    newunit->BarrelFacing.Set(this_ptr->BarrelFacing.Current());
    newunit->BarrelFacing.Set_Desired(this_ptr->BarrelFacing.Desired());
    newunit->PrimaryFacing.Set(this_ptr->PrimaryFacing.Current());
    newunit->PrimaryFacing.Set_Desired(this_ptr->PrimaryFacing.Desired());
    newunit->SecondaryFacing.Set(this_ptr->SecondaryFacing.Current());
    newunit->SecondaryFacing.Set_Desired(this_ptr->SecondaryFacing.Desired());
    newunit->Strength = (int)(this_ptr->Health_Ratio() * (int)newunit->Class->MaxStrength);
    newunit->ArmorBias = this_ptr->ArmorBias;
    newunit->FirepowerBias = this_ptr->FirepowerBias;
    newunit->SpeedBias = this_ptr->SpeedBias;
    newunit->Coord = this_ptr->Coord;
    newunit->EMPFramesRemaining = this_ptr->EMPFramesRemaining;
    newunit->Ammo = this_ptr->Ammo;


    if (newunit->Unlimbo(newunit->Coord, this_ptr->PrimaryFacing.Current().Get_Dir())) {

        /**
         *  Unlimbo successful, select our new unit and return it
         */

        if (PlayerPtr == newunit->Owning_House()) {
            newunit->Select();
        }

        if (this_ptr->TarCom) {
            newunit->Assign_Target(this_ptr->TarCom);
            newunit->Assign_Mission(MISSION_ATTACK);
            newunit->Commence();
        }

        return newunit;
    }

    /**
     *  Unlimboing the new unit failed! Delete the new unit and return false.
     */
    delete newunit;
    return nullptr;
}


enum TransformReturnValue {
    OriginalCode = 0x00650BC2,
    NotEnoughCharge = 0x006511A0,
    TransformSucceeded = 0x0065114C,
    TransformFailed = 0x00651168
};

/**
 *  Work-around function because the compiler likes smashing the stack by using ebp
 *  when calling locomotor functions :)
 *
 *  Returns the address that the calling function should jump to after calling this.
 *
 *  @author: Rampastring
 */
TransformReturnValue _UnitClass_Try_To_Deploy_Transform_To_Vehicle_Patch_Func(UnitClass* this_ptr) {

    /**
     *  Stolen bytes/code.
     */
    if (this_ptr->Class->DeploysInto != nullptr) {

        /**
         *  This unit is deployable rather than transformable, check whether it can deploy.
         */
        return OriginalCode;
    }

    UnitTypeClassExtension* unittypeext = Extension::Fetch<UnitTypeClassExtension>(this_ptr->Class);

    if (unittypeext->TransformsInto != nullptr) {

        /**
         *  Use custom "transform to vehicle" logic if we don't need charge or we have enough of it.
         */

        if (unittypeext->IsTransformRequiresFullCharge && this_ptr->CurrentCharge < this_ptr->Class->MaxCharge) {

            /**
             *  We don't have enough charge, return false
             */
            return NotEnoughCharge;
        }

        this_ptr->Mark(MARK_UP);
        this_ptr->Locomotor_Ptr()->Mark_All_Occupation_Bits(MARK_UP);

        UnitClass* newunit = Create_Transform_Unit(this_ptr);

        if (newunit != nullptr) {

            /**
             *  Creating transformed unit succeeded, erase the original unit and force function to return true
             */
            return TransformSucceeded;
        }
        else {

            /**
             *  Creating transformed unit failed. Re-mark our occupation bits and return false.
             */
            return TransformFailed;
        }
    }

    /**
     *  Continue to deployability check.
     */
    return OriginalCode;
}


/**
 *  #issue-715
 *
 *  Transforms a unit to another unit when a transformable unit deploys.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_UnitClass_Try_To_Deploy_Transform_To_Vehicle_Patch) {
    GET_REGISTER_STATIC(UnitClass*, this_ptr, esi);
    static int address;
    address = (int)(_UnitClass_Try_To_Deploy_Transform_To_Vehicle_Patch_Func(this_ptr));
    JMP(address);
}


/**
 *  #issue-715
 *
 *  Hack to display the the correct cursor for transformable units
 *  upon ACTION_SELF.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_UnitClass_What_Action_Self_Check_For_Vehicle_Transform_Patch) {
    GET_REGISTER_STATIC(UnitClass*, this_ptr, esi);
    static UnitTypeClass* unittype;
    static UnitTypeClassExtension* unittypeext;
    static ActionType action;

    unittype = this_ptr->Class;
    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);

    /**
     *  Stolen bytes/code.
     *  If the unit can deploy into a building, check whether it's currently allowed.
     */
    if (unittype->DeploysInto != nullptr) {
        JMP(0x0065602B);
    }

    /**
     *  Check if this unit is able to transform into another unit.
     *  If not, we don't have anything else to do here.
     */
    if (unittypeext->TransformsInto == nullptr) {
        _asm { mov eax, unittype }
        JMP_REG(ecx, 0x00656344);
    }

    /**
     *  If this unit is able to transform to a different unit, check if it requires charge for it.
     *  If it does, then check whether we have enough charge.
     */
    if (unittypeext->IsTransformRequiresFullCharge && this_ptr->CurrentCharge < this_ptr->Class->MaxCharge) {

        /**
         *  We don't have enough charge!
         */
        action = ACTION_NO_DEPLOY;
    }
    else if (this_ptr->Is_Immobilized()) {

        /**
         *  The unit is dying or under an EMP effect, don't allow it to transform.
         */
        action = ACTION_NO_DEPLOY;
    }
    else {
        action = ACTION_SELF;
    }

    /**
     *  Rebuilt function epilogue
     */
    _asm { pop edi }
    _asm { pop esi }
    _asm { pop ebp }
    _asm { pop ebx }
    _asm { add esp, 10h }
    _asm { mov eax, dword ptr ds:action }
    _asm { retn 8 }    
}


/**
 *  #issue-715
 *
 *  Check whether the unit is able to transform into another unit
 *  when performing the "Unload" mission.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_UnitClass_Mission_Unload_Transform_To_Vehicle_Patch) {
    GET_REGISTER_STATIC(UnitTypeClass*, unittype, eax);
    static UnitTypeClassExtension* unittypeext;

    /**
     *  Stolen bytes/code.
     */
    if (unittype->IsToHarvest || unittype->IsToVeinHarvest) {
    harvester_process:
        JMP(0x006545A5);
    }

    unittypeext = Extension::Fetch<UnitTypeClassExtension>(unittype);

    if (unittype->DeploysInto != nullptr || unittypeext->TransformsInto != nullptr) {
    deployable_process:
        JMP(0x00654403);
    }

mobile_emp_process:
    _asm { mov eax, unittype }
    JMP_REG(edx, 0x006543FD);
}


/**
 *  Finds the nearest docking bay for a specific unit.
 *
 *  @author: Rampastring
 */
void UnitClassExtension_Find_Nearest_Refinery(UnitClass* this_ptr, BuildingClass** building_addr, int* distance_addr, bool include_reserved)
{
    int nearest_refinery_distance = INT_MAX;
    BuildingClass* nearest_refinery = nullptr;

    /**
     *  Find_Docking_Bay looks also through occupied docking bays if ScenarioInit is set
     */
    if (include_reserved) {
        ScenarioInit++;
    }

    for (int i = 0; i < this_ptr->Class->Dock.Count(); i++) {
        BuildingTypeClass* dockbuildingtype = this_ptr->Class->Dock[i];

        BuildingClass* dockbuilding = this_ptr->Find_Docking_Bay(dockbuildingtype, false, false);
        if (dockbuilding == nullptr)
            continue;

        int distance = this_ptr->Distance(dockbuilding);

        if (distance < nearest_refinery_distance) {
            nearest_refinery_distance = distance;
            nearest_refinery = dockbuilding;
        }
    }

    if (include_reserved) {
        ScenarioInit--;
    }

    *building_addr = nearest_refinery;
    *distance_addr = nearest_refinery_distance;
}


/**
 *  #issue-201
 *
 *  A "quality of life" patch for harvesters so they don't discriminate against dock
 *  buildings that are not the first on their Dock= list. Also makes harvesters
 *  smarter by making them prefer queuing for nearby occupied refineries instead
 *  of wandering to distant free refineries.
 *
 *  @author: Rampastring
 */
DECLARE_PATCH(_UnitClass_Mission_Harvest_FINDHOME_Find_Nearest_Refinery_Patch)
{
    /**
     *  Enum for MISSION_HARVEST status constants.
     */
    enum {
        LOOKING,
        HARVESTING,
        FINDHOME,
        HEADINGHOME,
        GOINGTOIDLE,
    };


    GET_REGISTER_STATIC(UnitClass*, harvester, esi);
    static RadioMessageType response;
    static UnitClassExtension* unitext;
    static int free_refinery_distance_bias;
    static BuildingClass* nearest_free_refinery;
    static int nearest_free_refinery_distance;
    static BuildingClass* nearest_possibly_occupied_refinery;
    static int nearest_possibly_occupied_refinery_distance;
    static bool reserve_free_refinery;

    /**
     *  Find the nearest refinery that is not occupied.
     */
    UnitClassExtension_Find_Nearest_Refinery(harvester, &nearest_free_refinery, &nearest_free_refinery_distance, false);

    /**
     *  Find the nearest refinery, regardless of whether it's occupied.
     */
    UnitClassExtension_Find_Nearest_Refinery(harvester, &nearest_possibly_occupied_refinery, &nearest_possibly_occupied_refinery_distance, true);

    reserve_free_refinery = true;

    if (nearest_free_refinery == nullptr) {

        /**
         *  There was no free refinery, check if there was an occupied one.
         */
        if (nearest_possibly_occupied_refinery == nullptr) {

            /**
             *  No refinery existed at all! We have nothing to do here.
             */
            goto set_mission_delay_and_return;
        }

        /**
         *  There was an occupied refinery, queue for it instead.
         */
        reserve_free_refinery = false;
    }
    else if (nearest_free_refinery != nearest_possibly_occupied_refinery) {

        /**
         *  There was a free refinery as well as an occupied one.
         *  Check if the occupied refinery is significantly closer to us than the free refinery.
         */

        free_refinery_distance_bias = RuleExtension->MaxFreeRefineryDistanceBias;

        if (nearest_free_refinery_distance >
            nearest_possibly_occupied_refinery_distance + Cell_To_Lepton(free_refinery_distance_bias)) {

            reserve_free_refinery = false;
        }
    }

    unitext = Extension::Fetch<UnitClassExtension>(harvester);

    if (reserve_free_refinery) {

        /**
         *  We want to contact the free refinery, send a radio message to it.
         */
        response = harvester->Transmit_Message(RADIO_HELLO, nearest_free_refinery);

        /**
         *  Check if the refinery answered as expected. If not, we'll queue for it instead.
         */
        if (response == RADIO_ROGER) {

            /**
             *  The refinery accepted us! Change mission status to HEADINGHOME and jump to original code.
             */
            harvester->Status = HEADINGHOME;

            unitext->LastDockedBuilding = nearest_free_refinery;

            goto set_mission_delay_and_return;
        }
    }


    /**
     *  Re-use the original game's code for queueing to an occupied refinery.
     *  The game expects the occupied refinery pointer to be in edi.
     */
queue_to_occupied:

    unitext->LastDockedBuilding = nearest_possibly_occupied_refinery;

    _asm { mov edi, [nearest_possibly_occupied_refinery] };
    JMP(0x00654FAA);


    /**
     *  Set mission delay and return from function.
     */
set_mission_delay_and_return:
    JMP(0x00655226);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_UnitClass_AI_BuildConst_Patch)
{
    GET_REGISTER_STATIC(UnitTypeClass*, unittype, edx);

    if (Rule->BuildConst.Is_Present(unittype->DeploysInto))
    {
        JMP_REG(ecx, 0x0064E0EC);
    }

    JMP_REG(eax, 0x0064E134);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_UnitClass_What_Action_BuildConst)
{
    GET_REGISTER_STATIC(BuildingTypeClass*, buildingtype, ebp);
    _asm pushad

    if (Rule->BuildConst.Is_Present(buildingtype))
    {
        _asm popad
        JMP_REG(edx, 0x00656084);
    }

    _asm popad
    JMP_REG(edi, 0x006560A3);
}


/**
 *  #issue-177
 *
 *  Patches the AI to correctly consider all Construction Yards from the list.
 *
 *  @author: ZivDero
 */
DECLARE_PATCH(_UnitClass_Mission_Guard_BuildConst)
{
    GET_REGISTER_STATIC(UnitClass*, unit, esi);

    if (Rule->BuildConst.Is_Present(unit->Class->DeploysInto))
    {
        JMP(0x00656770);
    }

    JMP(0x006567FD);
}

/**
 *  Main function for patching the hooks.
 */
void UnitClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    UnitClassExtension_Init();

    Patch_Jump(0x006517BE, &_UnitClass_Per_Cell_Process_AutoHarvest_Assign_Harvest_Mission_Patch);
    Patch_Jump(0x0065B547, &_UnitClass_Explode_ShakeScreen_Division_BugFix_Patch);
    Patch_Jump(0x006530EB, &_UnitClass_Draw_Shape_Primary_Facing_Patch);
    Patch_Jump(0x006537A8, &_UnitClass_Draw_Shape_Turret_Facing_Patch);
    Patch_Jump(0x00653D7F, &_UnitClass_Draw_It_Unloading_Harvester_Patch);
    Patch_Jump(0x00654399, &_UnitClass_Mission_Unload_Transport_Detach_Sound_Patch);
    Patch_Jump(0x00653114, &_UnitClass_Draw_Shape_IdleRate_Patch);
    Patch_Jump(0x00656623, &_UnitClass_What_Action_ACTION_HARVEST_Block_On_Bridge_Patch); // IsToHarvest
    Patch_Jump(0x0065665D, &_UnitClass_What_Action_ACTION_HARVEST_Block_On_Bridge_Patch); // IsToVeinHarvest
    Patch_Jump(0x0064F2BE, &_UnitClass_Jellyfish_AI_Armor_Patch);
    Patch_Jump(0x00650BAE, &_UnitClass_Try_To_Deploy_Transform_To_Vehicle_Patch);
    Patch_Jump(0x00656017, &_UnitClass_What_Action_Self_Check_For_Vehicle_Transform_Patch);
    Patch_Jump(0x006543DB, &_UnitClass_Mission_Unload_Transform_To_Vehicle_Patch);
    Patch_Jump(0x0064E920, &UnitClassExt::_Firing_AI);
    Patch_Jump(0x006527B1, &_UnitClass_Draw_Voxel_Patch);
    Patch_Jump(0x00654EEE, &_UnitClass_Mission_Harvest_FINDHOME_Find_Nearest_Refinery_Patch);
    Patch_Jump(0x0064E0D7, &_UnitClass_AI_BuildConst_Patch);
    Patch_Jump(0x00655270, &UnitClassExt::_Mission_Hunt);
    Patch_Jump(0x00656074, &_UnitClass_What_Action_BuildConst);
    Patch_Jump(0x00656751, &_UnitClass_Mission_Guard_BuildConst);
    //Patch_Jump(0x0065054F, &_UnitClass_Enter_Idle_Mode_Block_Harvesting_On_Bridge_Patch); // Removed, keeping code for reference.
    //Patch_Jump(0x00654AB0, &_UnitClass_Mission_Harvest_Block_Harvesting_On_Bridge_Patch); // Removed, keeping code for reference.
}
