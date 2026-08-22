/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks that record game state changes for desync
 *          debugging.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "syncrecorder.h"

#include "abstract.h"
#include "animtype.h"
#include "foot.h"
#include "hooker.h"
#include "infantry.h"
#include "random.h"
#include "syringe.h"
#include "tibsun_defines.h"


/**
 *  All hooks in this file sit at function entry, where the caller's return
 *  address is the topmost value on the stack.
 */


/**
 *  Records unranged random number draws.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005BE030, _Random2Class_Unranged_Record_RNG_Call_Patch, 5)
{
    GET(Random2Class*, this_ptr, ECX);
    GET_STACK(unsigned, caller, 0x0);

    SyncRecorder::Record_RNG_Call(this_ptr, caller);

    return 0;
}


/**
 *  Records ranged random number draws.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x005BE080, _Random2Class_Ranged_Record_RNG_Call_Patch, 6)
{
    GET(Random2Class*, this_ptr, ECX);
    GET_STACK(unsigned, caller, 0x0);
    GET_STACK(int, minval, 0x4);
    GET_STACK(int, maxval, 0x8);

    SyncRecorder::Record_RNG_Call(this_ptr, caller, minval, maxval);

    return 0;
}


/**
 *  Records facing changes.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00496670, _FacingClass_Set_Record_Facing_Change_Patch, 7)
{
    GET_STACK(unsigned, caller, 0x0);
    GET_STACK(const DirType*, facing, 0x4);

    SyncRecorder::Record_Facing_Change(facing->Raw, caller);

    return 0;
}


/**
 *  Records infantry target assignments. Other target assignments are recorded
 *  in TechnoClassExt::_Assign_Target, which InfantryClass::Assign_Target calls
 *  into (so it must skip infantry to avoid recording them twice).
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004D4770, _InfantryClass_Assign_Target_Record_TarCom_Change_Patch, 8)
{
    GET(InfantryClass*, this_ptr, ECX);
    GET_STACK(unsigned, caller, 0x0);
    GET_STACK(const AbstractClass*, target, 0x4);

    SyncRecorder::Record_TarCom_Change(this_ptr, target, caller);

    return 0;
}


/**
 *  Records mission overrides.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x004A44F0, _FootClass_Override_Mission_Record_Patch, 6)
{
    GET(FootClass*, this_ptr, ECX);
    GET_STACK(unsigned, caller, 0x0);

    SyncRecorder::Record_Override_Mission(this_ptr, caller);

    return 0;
}


/**
 *  Records animation creations.
 *
 *  @author: ZivDero
 */
DEFINE_HOOK(0x00413AE0, _AnimClass_Constructor_Record_Patch, 6)
{
    GET_STACK(unsigned, caller, 0x0);
    GET_STACK(const AnimTypeClass*, animtype, 0x4);
    GET_STACK(const Coord*, coord, 0x8);

    SyncRecorder::Record_Anim_Construction(animtype, *coord, caller);

    return 0;
}
