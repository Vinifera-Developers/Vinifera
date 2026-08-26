/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended InfantryClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "infantryext_hooks.h"

#include "animtype.h"
#include "asserthandler.h"
#include "building.h"
#include "buildingtype.h"
#include "debughandler.h"
#include "extension.h"
#include "fetchres.h"
#include "hooker.h"
#include "house.h"
#include "housetypeext.h"
#include "infantry.h"
#include "infantryext.h"
#include "infantryext_init.h"
#include "infantrytype.h"
#include "infantrytypeext.h"
#include "ccini.h"
#include "language.h"
#include "mouse.h"
#include "options.h"
#include "rules.h"
#include "rulesext.h"
#include "sideext.h"
#include "syringe.h"
#include "tag.h"
#include "tagtype.h"
#include "technotype.h"
#include "technotypeext.h"
#include "tiberium.h"
#include "tiberiumext.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "vinifera_globals.h"
#include "vinifera_util.h"
#include "voc.h"
#include "wwkeyboard.h"


/***************************************************************************
**	Relative coordinate offsets from the center of a cell for each
**	of the legal positions that an object in a cell may stop at. Only infantry
**	are allowed to stop at other than the center of the cell.
*/
Coord const StoppingCoordAbs[5] = {
    Coord(128, 128, 0), // center
    Coord(64, 64, 0),   // upper left
    Coord(192, 64, 0),  // upper right
    Coord(64, 192, 0),  // lower left
    Coord(192, 192, 0)  // lower right
};


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(InfantryClass)
{
public:
    const ShapeSet* _Get_Image_Data() const;
    const char* _Full_Name(void) const;
    static void _Read_INI(CCINIClass const & ini);
};


/**
 *  Fetches the image data for this infantry unit.
 *
 *  The image data for the infantry differs from normal if this is a spy. A spy always
 *  appears like a regular infantry to the non-owning players.
 *
 *  Infantry currently in webs also display a different image.
 *
 *  This implementation adds support for side-specific disguises and also fixes
 *  a bug in the original game where friendly Spies were shown as disguised.
 *
 *  @author: ZivDero, Rampastring
 */
const ShapeSet* InfantryClassExt::_Get_Image_Data() const
{
    if (Doing == DO_STRUGGLE && Rule->WebbedInfantry) {
        return Rule->WebbedInfantry->Get_Image_Data();
    }

    if (!House->Is_Ally(PlayerPtr) && Class->IsDisguised) {

        const auto disguise = SideClassExtension::Get_Disguise(House);
        if (disguise) {
            return disguise->Image;
        }
    }

    return ObjectClass::Get_Image_Data();
};


/**
 *  Fetches the full name for this infantry unit.
 *
 *  This implementation adds support for side-specific disguises and also fixes
 *  a bug in the original game where friendly Spies were shown as disguised.
 *
 *  @author: tomsons26/ZivDero, Rampastring
 */
const char* InfantryClassExt::_Full_Name(void) const
{
    assert(IsActive);

    if (IsTechnician) {
        return Fetch_String(TXT_TECHNICIAN);
    }

    if (Class->IsDisguised && !House->Is_Ally(PlayerPtr) && Rule->Disguise != NULL) {
        const auto disguise = SideClassExtension::Get_Disguise(House);
        if (disguise) {
            return disguise->GivenName.c_str();
        }

        return Rule->Disguise->GivenName.c_str();
    }

    return Class->GivenName.c_str();
}


/**
 *  #issue-635
 * 
 *  Fixes a bug where EngineerDamage was not used to calculate the engineer damage.
 * 
 *  @author: CCHyper
 */
static int Get_Engineer_Damage(TechnoClass *tech)
{
    float damage = Rule->EngineerDamage;    // Was "Rule->ConditionRed * 0.5f"
    return std::min(tech->TClass->MaxStrength * damage, (float)(tech->Strength-1));
}


/**
 *  Patch to intercept the engineer capture checks.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D35F9, _InfantryClass_Per_Cell_Process_Engineer_Capture_Damage_Patch, 0)
{
    GET(InfantryClass *, this_ptr, ESI);
    GET(TechnoClass *, tech, EDI);      // From "cellptr->Cell_Building()".

    /**
     *  If the target buildings health is low enough, go ahead and capture it.
     *  Changed to use Rule->EngineerCaptureLevel.
     * 
     *  @author: CCHyper
     */
    if (tech->HealthRatio <= Rule->EngineerCaptureLevel) {
        goto capture;
    }

    /**
     *  Health is still not low enough, go ahead and apply some more damage to it.
     */
    int damage = Get_Engineer_Damage(tech);
    tech->Take_Damage(damage, 0, Rule->C4Warhead, this_ptr, true);

    /**
     *  Spring the DESTROYED_BY_ANYTHING event and remove this infantry.
     */
spring_and_delete:
    return 0x004D378D;

    /**
     *  Processing capturing of the target building.
     */
capture:
    return 0x004D36E1;
}


/**
 *  #issue-264
 * 
 *  Implements EnterTransportSound for infantry when they enter a transport.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D3A7B, _InfantryClass_Per_Cell_Process_Transport_Attach_Sound_Patch, 0)
{
    GET(InfantryClass *, this_ptr, ESI);
    GET(TechnoClass *, techno, EDI);        // Radio contact

    /**
     *  Stolen bytes/code.
     */
    techno->Cargo.Attach(this_ptr);

    /**
     *  If this transport we are entering has a passenger entering sound, play it now.
     */
    auto radio_technotypeext = Extension::Fetch(techno->TClass);
    if (radio_technotypeext->EnterTransportSound != VOC_NONE) {
        Static_Sound(radio_technotypeext->EnterTransportSound, techno->Position);
    }

    return 0x004D3A87;
}


/**
 *  #issue-226
 * 
 *  Implements IsMechanic for infantry when searching for targets.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D87E9, _InfantryClass_Firing_AI_Mechanic_Patch, 0)
{
    GET(InfantryClass *, this_ptr, EBP);
    GET(ObjectClass *, targ, ESI);      // TarCom as ObjectClass.

    auto infantrytypeext = Extension::Fetch(this_ptr->Class);

    /**
     *  Is this infantry a "dual healer" (can it heal both infantry and units)?
     */
    if (infantrytypeext->IsOmniHealer) {

        /**
         *  Is the target being queried a unit, aircraft or infantry? If so, make
         *  sure this infantry is a mechanic before allowing it to heal the unit.
         */
        if (targ->RTTI == RTTI_UNIT || 
            (targ->RTTI == RTTI_AIRCRAFT && !targ->In_Air()) || 
            targ->RTTI == RTTI_INFANTRY || 
            (targ->RTTI == RTTI_BUILDING && targ->TClass->UndeploysInto != nullptr && !reinterpret_cast<BuildingClass*>(targ)->Class->IsConstructionYard)) {
            goto health_ratio_check;
        }

    /**
     *  Is this infantry a mechanic?
     */
    } else if (infantrytypeext->IsMechanic) {

        /**
         *  Is the target being queried a unit or aircraft? If so, make sure this
         *  infantry is a mechanic before allowing it to heal the unit.
         */
        if (targ->RTTI == RTTI_UNIT || 
            (targ->RTTI == RTTI_AIRCRAFT && !targ->In_Air()) || 
            (targ->RTTI == RTTI_BUILDING && targ->TClass->UndeploysInto != nullptr && !reinterpret_cast<BuildingClass*>(targ)->Class->IsConstructionYard)) {
            goto health_ratio_check;
        }

    /**
     *  Original code.
     */
    } else if (targ->RTTI == RTTI_INFANTRY) {
        goto health_ratio_check;
    }

assign_NULL_target:
    return 0x004D8824;

    /**
     *  Check the targets health ratio.
     */
health_ratio_check:
    return 0x004D87F5;
}


/**
 *  #issue-226
 * 
 *  Implements IsMechanic and IsDualHealer for infantry when deciding what action to perform.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D7168, _InfantryClass_What_Action_Mechanic_Patch, 0)
{
    GET(InfantryClass *, this_ptr, EDI);
    GET(/*const */ObjectClass *, object, ESI);  // target

    auto infantrytypeext = Extension::Fetch(this_ptr->Class);

    /**
     *  Is this infantry a "dual healer" (can it heal both infantry and units)?
     */
    if (infantrytypeext->IsOmniHealer) {

        /**
         *  If the mouse is over ourself, show the guard area cursor.
         */
        if (object == this_ptr) {
            goto guard_area;
        }

        /**
         *  Is the target being queried a unit, aircraft or infantry? If so, make
         *  sure this infantry is a mechanic before allowing it to heal the unit.
         */
        if (object->RTTI == RTTI_UNIT ||
            object->RTTI == RTTI_AIRCRAFT || 
            object->RTTI == RTTI_INFANTRY || 
            (object->RTTI == RTTI_BUILDING && object->TClass->UndeploysInto != nullptr && !reinterpret_cast<BuildingClass*>(object)->Class->IsConstructionYard)) {

            /**
             *  If we are force-moving into an Transport, don't try to heal it!
             */
            if (object->TClass->MaxPassengers > 0) {
                if (Keyboard->Down(Options.KeyForceMove1) || Keyboard->Down(Options.KeyForceMove2)) {
                    goto next_check;
                }
            }

            /**
             *  Before return ACTION_HEAL, check the targets health.
             */
            goto health_ratio_check;
        }

    /**
     *  Is this infantry a mechanic?
     */
    } else if (infantrytypeext->IsMechanic) {

        /**
         *  If the mouse is over ourself, show the guard area cursor.
         */
        if (object == this_ptr) {
            goto guard_area;
        }

        /**
         *  Is the target being queried a unit or aircraft? If so, make sure this
         *  infantry is a mechanic before allowing it to heal the unit.
         */
        if (object->RTTI == RTTI_UNIT ||
            object->RTTI == RTTI_AIRCRAFT || (object->RTTI == RTTI_BUILDING && object->TClass->UndeploysInto != nullptr && !reinterpret_cast<BuildingClass*>(object)->Class->IsConstructionYard)) {

            /**
             *  If we are force-moving into an Transport, don't try to heal it!
             */
            if (object->TClass->MaxPassengers > 0) {
                if (Keyboard->Down(Options.KeyForceMove1) || Keyboard->Down(Options.KeyForceMove2)) {
                    goto next_check;
                }
            }

            /**
             *  Before return ACTION_HEAL, check the targets health.
             */
            goto health_ratio_check;
        }

    /**
     *  Original code.
     */
    } else if (object->RTTI == RTTI_INFANTRY) {

        /**
         *  If the mouse is over ourself, show the guard area cursor.
         */
        if (object == this_ptr) {
            goto guard_area;
        }

        /**
         *  Before return ACTION_HEAL, check the targets health.
         */
        goto health_ratio_check;
    }

next_check:
    return 0x004D71B0;

    /**
     *  Show the guard area mouse cursor over us.
     */
guard_area:
    return 0x004D71A1;

    /**
     *  Check the targets health ratio.
     */
health_ratio_check:
    return 0x004D7178;
}


/**
 *  #issue-226
 * 
 *  Allow all foot objects to be valid targets when this infantry deals negative damage.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D5AB4, _InfantryClass_Can_Fire_Target_Check_Patch, 0)
{
    GET_STACK(AbstractClass *, target, 0x10);

    TechnoClass* techno = dynamic_cast<TechnoClass*>(target);
    R->EAX(techno);

    return 0x004D5ACB;
}


/**
 *  #issue-80
 * 
 *  Fixes the bug where the Jumpjet uses the wrong DoType when idle on the
 *  ground. This was because the original code did not check if the infantry
 *  was actually in the air or not and always assumed it was on the ground, as
 *  a result it was always setting DO_STAND_READY.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D8C83, _InfantryClass_Doing_AI_JumpJet_Idle_Patch, 0)
{
    GET(InfantryClass *, this_ptr, ESI);

    auto infantrytype = reinterpret_cast<const InfantryTypeClass *>(this_ptr->Class_Of());

    /**
     *  Stolen code.
     * 
     *  If infantry is prone, set DO_PRONE.
     */
    if (this_ptr->IsProne) {
        return 0x004D8B12;
    }

    if (infantrytype->IsJumpJet) {

        /**
         *  This is a Jumpjet infantry, make sure its in the air before
         *  assigning the hover graphic sequence.
         */
        if (this_ptr->In_Air()) {
            this_ptr->Do_Action(DO_HOVER, true);

        } else {
            this_ptr->Do_Action(DO_STAND_READY, true);
        }

    /**
     *  Handle normal infantry.
     */
    } else {
        this_ptr->Do_Action(DO_STAND_READY, true);
    }

    return 0x004D8CA1;
}


/**
 *  #issue-80
 * 
 *  Fixes the bug where the Jumpjet uses the wrong DoType when on the
 *  ground and in between firing rounds. This was because the original code
 *  did not check if the infantry was actually in the air and assumed it always
 *  is, thus setting DO_STAND_READY.
 * 
 *  @warning: This patch is within a branch that has already checked if the
 *            infantry is firing!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D50C9, _InfantryClass_AI_JumpJet_Idle_Between_Firing_Patch, 0)
{
    GET(InfantryClass *, this_ptr, ESI);

    auto infantrytype = reinterpret_cast<const InfantryTypeClass *>(this_ptr->Class_Of());

    if (infantrytype->IsJumpJet) {

        /**
         *  This is a Jumpjet infantry, make sure its in the air before
         *  assigning the hover graphic sequence.
         */
        if (this_ptr->In_Air()) {
            this_ptr->Do_Action(DO_HOVER);

        } else {
            this_ptr->Do_Action(DO_STAND_READY);
        }

    /**
     *  Handle normal infantry.
     */
    } else {
        this_ptr->Do_Action(DO_STAND_READY);
    }

    /**
     *  Stolen code.
     * 
     *  Clear the firing flag.
     */
    this_ptr->IsFiring = false;

    return 0x004D50E0;
}


/**
 *  #issue-80
 * 
 *  Fixes the bug where the Jumpjet uses the wrong DoType when not moving
 *  but actually in the air.
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D9076, _InfantryClass_Movement_AI_JumpJet_Not_Moving_Patch, 0)
{
    GET(InfantryClass *, this_ptr, EBP);

    auto infantrytype = reinterpret_cast<const InfantryTypeClass *>(this_ptr->Class_Of());

    if (infantrytype->IsJumpJet) {

        /**
         *  This is a Jumpjet infantry, make sure its in the air before
         *  assigning the hover graphic sequence.
         */
        if (this_ptr->In_Air()) {
            this_ptr->Do_Action(DO_HOVER);

        } else {
            this_ptr->Do_Action(DO_STAND_READY);
        }

    /**
     *  Handle normal infantry.
     */
    } else {
        this_ptr->Do_Action(DO_STAND_READY);
    }

    return 0x004D9087;
}


/**
 *  #issue-81
 * 
 *  Fixes the bug where the Jumpjet uses the wrong DoType when firing on
 *  the ground. This was because the original code did not check if the infantry
 *  was actually in the air and assumed it always is, thus setting DO_FIREFLY.
 * 
 *  @warning: This patch is within a branch that has already checked if the
 *            infantry is controlled by the Jumpjet locomotor!
 * 
 *  @author: CCHyper
 */
DEFINE_HOOK(0x004D88FA, _InfantryClass_Firing_AI_JumpJet_In_Air_Patch, 0)
{
    GET(InfantryClass *, this_ptr, EBP);

    /**
     *  Make sure its in the air before assigning the hover firing graphic sequence.
     */
    if (this_ptr->In_Air()) {
        this_ptr->Do_Action(DO_FIREFLY);
    } else {
        this_ptr->Do_Action(DO_FIRE_WEAPON);
    }

    return 0x004D8933;
}


/**
 *  Uses a new extension value as the damage Tiberium deals to infantry.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004D3F5D, _InfantryClass_Per_Cell_Process_Tiberium_Damage_Patch, 0)
{
    GET(int, tib_id, EAX);

    int damage = Extension::Fetch(Tiberiums[tib_id])->DamageToInfantry;

    R->EAX(damage);
    R->Stack(0x10, damage);

    return 0x004D3F8E;
}


/**
 *  #issue-1080
 *
 *  Fixes an out-of-bounds DoControls read when an infantry is "doing nothing".
 *
 *  @author: Rampastring
 */
void _Set_Infantry_Facing_After_Doing_Check_For_Do_Nothing(InfantryClass* this_ptr)
{
    if (this_ptr->Doing == DO_NOTHING) {
        return;
    }

    FacingType facing = this_ptr->Class->DoControls[this_ptr->Doing].Finish;
    if (facing == FACING_NONE) {
        return;
    }

    Dir256 dirtype = Facing_Dir(facing);
    DirType ds = DirType(dirtype);
    this_ptr->PrimaryFacing.Set(ds);
}


DEFINE_HOOK(0x004D8BE4, _InfantryClass_Doing_AI_Fix_Invalid_Facing_Set, 0)
{
    GET(InfantryClass*, inf, ESI);
    _Set_Infantry_Facing_After_Doing_Check_For_Do_Nothing(inf);
    return 0x004D8C14;
}


/**
 *  Fixes an exploit where hijackers are able to hijack vehicles of their allies.
 *
 *  Author: Rampastring
 */
DEFINE_HOOK(0x004D7267, _InfantryClass_What_Action_Prevent_Hijacking_Allied_Vehicles, 0)
{
    GET(TechnoClass*, target, ESI);
    GET(InfantryClass*, this_ptr, EDI);

    if (this_ptr->House->Is_Ally(target)) {
        // The target is allied to the hijacker. Move on.
        return 0x004D72B7;
    }

    // Move to harvester truce check.
    return 0x004D7277;
}


/**
 *  Patches InfantryClass::What_Action when the target object is an armory.
 *  No longer transmits a message to the armory requesting it to establish radio connection (which only allows one unit at a time).
 *  Instead, simply checks for the armory's conditions (unit's veterancy and armory's ammo) to determine if the unit can go in or not.
 *  This also allows additional units to be ordered into the armory even if other unit(s) are already on their way to it.
 * 
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D7355, _InfantryClass_What_Action_Armory_Action_Patch, 0)
{
    GET(BuildingClass*, object, ESI);
    GET(InfantryClass*, this_ptr, EDI);

    ActionType action = ACTION_ENTER;

    if (this_ptr->Crew.Is_Elite()) {
        action = ACTION_NO_ENTER;
    }

    if (object->Ammo <= 0) {
        action = ACTION_NO_ENTER;
    }

    R->EBX(action);

    return 0x004D738E;
}


/**
 *  Patches InfantryClass::What_Action when the target object is a hospital.
 *  No longer transmits a message to the hospital requesting it to establish radio connection (which only allows one unit at a time).
 *  Instead, simply checks for the hospital's conditions (unit's health and hospital's ammo) to determine if the unit can go in or not.
 *  This also allows additional units to be ordered into the hospital even if other unit(s) are already on their way to it.
 * 
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D72F2, _InfantryClass_What_Action_Hospital_Action_Patch, 0)
{
    GET(BuildingClass*, object, ESI);
    GET(InfantryClass*, this_ptr, EDI);

    ActionType action = ACTION_ENTER;

    if (this_ptr->Strength >= this_ptr->Class->MaxStrength) {
        action = ACTION_NO_ENTER;
    }

    if (object->Ammo <= 0) {
        action = ACTION_NO_ENTER;
    }

    R->EBX(action);

    return 0x004D731A;
}


/**
 *  Clears the current cell from bridge damage trackers, if any.
 *  Recursively looks around all orthogonal cells in order to find all directly adjacent bridge cells
 *  That are part of that bridge, which expands until all bridge cells are evaluated.
 *
 *  @author: JoyfulShush
 */
void Scan_And_Clear_Bridge(Cell current_cell, DynamicVectorClass<Cell>& visited_cells)
{
    if (visited_cells.Is_Present(current_cell)) {
        return;
    }

    visited_cells.Add(current_cell);

    if (BridgeHealths.contains(current_cell)) {        
        BridgeHealths.erase(current_cell);
    }

    CellClass* current_cellptr = &Map[current_cell];

    FacingType dirs[4] = {FACING_W, FACING_E, FACING_S, FACING_N};
    for (FacingType dir : dirs) {
        if (current_cellptr->Is_Overlay_Low_Bridge()) {
            /*
             * Handle two low overlay bridges that are very close to each other.
             * We skip one side checking another side that should never be part of the same low bridge.
             */
            if (current_cellptr->OverlayData == 0) {
                if (current_cellptr->Is_Low_Bridge_SE_NW() && dir == FACING_N) {
                    continue;
                }

                if (current_cellptr->Is_Low_Bridge_SW_NE() && dir == FACING_E) {
                    continue;
                }                    
            }

            if (current_cellptr->OverlayData == 2) {
                if (current_cellptr->Is_Low_Bridge_SE_NW() && dir == FACING_S) {
                    continue;
                }

                if (current_cellptr->Is_Low_Bridge_SW_NE() && dir == FACING_W) {
                    continue;
                }
            }
        }

        Cell adjacent_cell = Adjacent_Cell(current_cell, dir);
        CellClass* adjacent_cellptr = &Map[adjacent_cell];

        if (adjacent_cellptr->Is_Bridge_Here() || adjacent_cellptr->Is_Overlay_Low_Bridge() || adjacent_cellptr->WasUnderBridge) {
            Scan_And_Clear_Bridge(adjacent_cell, visited_cells);
        }
    }
}


/**
 *  Scans for a bridge attached to a bridge hut that was just entered by an Engineer.
 *  Checks for all types of bridges: Low Bridges (via overlay), High Bridges and High Train Bridges.
 *  Since in some cases, notably high bridges the overlay is actually far, we need to search around the bridge hut
 *  Up to 3 cells away from it, as only the bridge's center piece in a bridge tile is actually considered a bridge.
 *  Once a bridge cell is located, it begins evaluation of the entire bridge and returns afterwards.
 *
 *  @author: JoyfulShush
 */
void Scan_Around_Bridge_Hut_For_Bridge(Cell const& bridge_hut_cell)
{
    DynamicVectorClass<Cell> visited_cells;

    for (int depth = 1; depth <= 3; ++depth) {        
        for (FacingType dir = FACING_FIRST; dir < FACING_COUNT; dir++) {
            Cell target_cell = Get_Nearby_Cell_At_Depth(bridge_hut_cell, dir, depth);
            CellClass* target_cellptr = &Map[target_cell];            
            if (target_cellptr && (target_cellptr->Is_Bridge_Here() || target_cellptr->Is_Overlay_Low_Bridge() || target_cellptr->WasUnderBridge)) {
                Scan_And_Clear_Bridge(target_cell, visited_cells);
                return;
            }
        }
    }

    return;
}


/**
 *  Patches InfantryClass::Process_Per_Cell at the portion where engineers enter a Bridge Hut
 *  that is adjacent to either a Low Bridge or a High (non-train) Bridge.
 *  Used to find the attached bridge parts and remove all bridge damage registers from it (essentially fully healing all bridge cells)
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D356B, _InfantryClass_Process_Per_Cell_Engineer_Bridge_Patch, 6)
{    
    GET(Cell*, cell_ptr, EAX);

    if (RuleExtension->IsUseBridgeHealth) {
        Scan_Around_Bridge_Hut_For_Bridge(*cell_ptr);
    }

    return 0;
}


/**
 *  Patches InfantryClass::Process_Per_Cell at the portion where engineers enter a Bridge Hut
 *  that is adjacent to a Train High Bridge.
 *  Used to find the attached bridge parts and remove all bridge damage registers from it (essentially fully healing all bridge cells)
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D3551, _InfantryClass_Process_Per_Cell_Engineer_Train_Bridge_Patch, 6)
{    
    GET(Cell*, cell_ptr, EAX);

    if (RuleExtension->IsUseBridgeHealth) {
        Scan_Around_Bridge_Hut_For_Bridge(*cell_ptr);
    }

    return 0;
}


/**
 *  Patches InfantryClass::Assign_Destination at the part that identifies infantry units with mission "MISSION_ENTER",
 *  at a part where units are communicating with a building that has a radio buddy.
 *  For hospitals and armories, this typically means that a unit got the permission to dock into the hospital/armory.
 *  Other units will then go near the hospital/armory (due to being assigned MISSION_MOVE) and will constantly contact the hospital/armory to try to enter it.
 *  Once the hospital/armory finish with the unit inside them, they dismiss contact with them and are available to accept requests from another unit.
 * 
 *  All other buildings preserve the original behavior that they had before.
 * 
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D4251, _Assign_Destination_Hospital_Armory_Queue_Patch, 9)
{
    GET(InfantryClass*, this_ptr, EBP);
    GET(BuildingClass*, object, EBX);

    bool assigned_archive_target = false;
    if (object->Class->IsHospital || object->Class->IsArmory) {
        if (object->Ammo >= 0) {
            this_ptr->Assign_Archive_Target(object);
            this_ptr->field_20C = object; // Seems to be the object that becomes the "entry target" of this unit.
            this_ptr->Assign_Mission(MISSION_MOVE);
            assigned_archive_target = true;
        }
    }

    // original behavior
    if (!assigned_archive_target) {
        this_ptr->Assign_Archive_Target(nullptr);
    }
    
    return 0x004D425A;
}


/**
 *  Patches InfantryClass::What_Action at the part where medics/mechanics (AKA infantry with negative combat damage)
 *  are evaluated whether they should commit to their action or switch to ACTION_SELECT.
 *  
 *  Fixes an issue where ACTION_TOGGLE_SELECT (selecting units while shift is held) was converted into ACTION_SELECT,
 *  causing the game to instead unselect the currently selected units if the medic was the "best object" in the current selection.
 * 
 *  Causes What_Action to return with the ACTION_TOGGLE_SELECT action if this is the current mission.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D71CF, _Infantry_Class_What_Action_Medic_Toggle_Select_Patch, 9)
{
    GET(ActionType, action, EBX);

    if (action == ACTION_TOGGLE_SELECT || action == ACTION_GUARD_AREA || action == ACTION_MOVE) {
        return 0x004D76F9;
    }

    return 0;
}


/**
 *  Patches InfantryClass::AI at the part where Jumpjets are moving to determine if they should use Look
 *  Sight range now takes into account adjustments from veterancy.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D5011, InfantryClass_AI_Sight_Range_Jumpjet_Patch, 0)
{
    GET(InfantryClass*, this_ptr, ESI);

    auto this_ptr_ext = Extension::Fetch(this_ptr);

    if (this_ptr->Locomotion->Is_Moving() && this_ptr->Get_Height_AGL() > 0 && this_ptr->IsOwnedByPlayer && this_ptr_ext->Get_Sight_Range() > 0) {
        return 0x004D504C;
    }

    return 0x004D509D;
}


/**
 *  Patches InfantryClass::Unlimbo at the part where blind infantry are evaluated to check for making infantry not be considered as disocvered
 *  Sight range now takes into account adjustments from veterancy.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D6CBD, InfantryClass_Unlimbo_Sight_Range_Patch, 0)
{
    GET(InfantryClass*, this_ptr, EBX);

    auto this_ptr_ext = Extension::Fetch(this_ptr);

    if (this_ptr_ext->Get_Sight_Range() == 0) {
        return 0x004D6CCD;
    }

    return 0x004D6CD4;
}

/**
 *  Patches InfantryClass::Assign_Destination at the part where jumpjets determine whether to add a Q-Move.
 *  This occurs when jumpjets are assigned a new destination while using the Walk Locomotion to move.
 *  The Q-Move is used to make the jumpjets effectively stop near their position in order to reconsider swapping locomotions to fly
 *  to the newly ordered position. However, in some cases, this behavior is not desired and causes issues, which this patch fixes.
 *
 *  @author: JoyfulShush
 */
DEFINE_HOOK(0x004D4356, InfantryClass_Assign_Destination_Jumpjet_Move_Queue_Patch, 6)
{
    GET(InfantryClass*, this_ptr, EBP);

    enum {
        SKIP_NAVQUEUE_CHAIN = 0x004D43DB
    };

    // If the unit has an archive target, this typically means it was instructed to reach a position
    // as soon as it finishes what it's currently doing, such as when being rallied out of a Barracks, Hospital or Armory.
    // We should not add Q-Move to the destination when this is the case.
    // Fixes a bug where jumpjets would go back to the building they exited from after reaching the building's rally point.
    if (this_ptr->ArchiveTarget != nullptr) {
        return SKIP_NAVQUEUE_CHAIN;
    }

    // If the unit is about to enter a building, it shouldn't have any Q-Moves lines up.
    // Fixes a bug where jumpjets behave very erratically when trying to enter a hospital or an armory.
    if (this_ptr->Get_Mission() == MISSION_ENTER) {
        this_ptr->Clear_Navigation_List();

        return SKIP_NAVQUEUE_CHAIN;
    }

    return 0;
}


TagClass* Find_Or_Make_Tag_Inf(TagTypeClass* type)
{
    for (int index = 0; index < Tags.Count(); index++) {
        TagClass* tag = Tags[index];
        if (tag->Class == type) {
            return (tag);
        }
    }

    return (new TagClass(type));
}


/**
 *  Replacement for InfantryClass::Read_INI for the multiplayer spawner.
 *
 *  @author: Rampastring
 */
void InfantryClassExt::_Read_INI(CCINIClass const & ini)
{
    char buf[128];

    const char* sectionname = "Infantry";

    int len = ini.Entry_Count(sectionname);
    for (int index = 0; index < len; index++) {
        char const* entry = ini.Get_Entry(sectionname, index);

        /*
        **	Get an infantry entry
        */
        ini.Get_String(sectionname, entry, nullptr, buf, sizeof(buf));

        /*
        **	1st token: house name.
        */
        char* housename = strtok(buf, ",");
        HousesType inhouse = HouseTypeClassExtension::House_From_Name(housename);
        HouseClass* inhousep = House_From_HousesType(inhouse);

        if (inhousep == nullptr) {
            if (Session.Type == GAME_NORMAL || inhouse < EXT_HOUSE_SPAWN1) {
                Vinifera_Log_And_Show_WWMessageBox("Unable to find house %s while reading infantry!", housename);
                continue;
            } else {
                DEBUG_INFO("Ignoring unit placed for {} because the house is not present\n", housename);
                continue;
            }
        }

        /*
        **	2nd token: infantry type name.
        */
        char* infantrytypename = strtok(nullptr, ",");
        InfantryType classid = InfantryTypeClass::From_Name(infantrytypename);

        if (classid == INFANTRY_NONE) {
            Vinifera_Log_And_Show_WWMessageBox("Unable to find InfantryType %s while reading units!", infantrytypename);
            continue;
        }

        InfantryClass* infantry = new InfantryClass(InfantryTypes[classid], inhousep);
        if (infantry != nullptr) {

            /*
            **	3rd token: strength.
            */
            int strength = atoi(strtok(nullptr, ","));

            /*
            **	4th token: cell #.
            */
            int x, y;
            if (NewINIFormat >= 4) {
                x = atoi(strtok(nullptr, ","));
                y = atoi(strtok(nullptr, ","));
            } else {
                int cellnum = atoi(strtok(nullptr, ","));
                x = cellnum % 128;
                y = cellnum / 128;
            }
            Cell cell(x, y);
            Coord coord = cell.As_Coord();

            /*
            **	5th token: cell sub-location.
            */
            int sub = atoi(strtok(nullptr, ","));
            coord = Coord_Whole(coord) + StoppingCoordAbs[sub];

            /*
            **	Fetch the mission and facing.
            */
            MissionType mission = MissionClass::Mission_From_Name(strtok(nullptr, ","));
            Dir256 dir;
            char* token = strtok(nullptr, ",");
            if (token) {
                dir = (Dir256)atoi(token);
            } else {
                dir = (Dir256)0;
            }

            TagType tagtype = TagTypeClass::From_Name(strtok(nullptr, ","));
            if (tagtype != TAG_NONE) {
                TagTypeClass* tp = TagTypes[tagtype];
                if (tp != nullptr) {
                    TagClass* tt = Find_Or_Make_Tag_Inf(tp);
                    if (tt != nullptr) {
                        infantry->Attach_Tag(tt);
                    }
                }
            }

            token = strtok(nullptr, ",");
            if (token) {
                infantry->Crew.From_Integer(atoi(token));
            }

            token = strtok(nullptr, ",");
            if (token) {
                infantry->Group = atoi(token);
            }

            token = strtok(nullptr, ",");
            if (token) {
                infantry->IsOnBridge = atoi(token) != 0;
                if (infantry->IsOnBridge) {
                    coord.Z = Map.Get_Height_GL(coord) + BRIDGE_LEPTON_HEIGHT;
                } else {
                    coord.Z = Map.Get_Height_GL(coord);
                }
            }

            token = strtok(nullptr, ",");
            if (token) {
                infantry->field_205 = atoi(token) != 0;
            }

            token = strtok(nullptr, ",");
            if (token) {
                infantry->field_206 = atoi(token) != 0;
            }

            if (&Map[coord] != &BlubCell && infantry->Unlimbo(coord, dir)) {
                infantry->Strength = infantry->Class->MaxStrength * (strength / 256.0);
                if (infantry->Strength > infantry->Class->MaxStrength - 3) infantry->Strength = infantry->Class->MaxStrength;
                if (infantry->Strength < 1) infantry->Strength = 1;
                if (Session.Type == GAME_NORMAL || infantry->House->Is_Human_Player()) {
                    infantry->Assign_Mission(mission);
                    infantry->Commence();
                } else {
                    infantry->Enter_Idle_Mode();
                }
            } else {

                /*
                **	If the infantry could not be unlimboed, then this is a big error.
                **	Delete the infantry.
                */
                delete infantry;
            }
        }
    }
}


/**
 *  Main function for patching the hooks.
 */
void InfantryClassExtension_Hooks()
{
    /**
     *  Initialises the extended class.
     */
    InfantryClassExtension_Init();

    Patch_Jump(0x004D90B0, &InfantryClassExt::_Get_Image_Data);
    Patch_Jump(0x004D77A0, &InfantryClassExt::_Full_Name);
    Patch_Jump(0x004D7B30, &InfantryClassExt::_Read_INI);
}
