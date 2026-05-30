/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  Contains the hooks for the extended ObjectClass.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/

#include "always.h"

#include "objectext_hooks.h"

#include "anim.h"
#include "animtype.h"
#include "asserthandler.h"
#include "audio_voc_handle.h"
#include "cell.h"
#include "colorscheme.h"
#include "extension.h"
#include "hooker.h"
#include "house.h"
#include "mouse.h"
#include "object.h"
#include "objectext.h"
#include "objecttype.h"
#include "objecttypeext.h"
#include "rules.h"
#include "syringe.h"
#include "techno.h"
#include "tibsun_globals.h"
#include "tibsun_inline.h"
#include "vinifera_globals.h"
#include "tag.h"
#include "syringe.h"
#include "vinifera_defines.h"


/**
 *  A fake class for implementing new member functions which allow
 *  access to the "this" pointer of the intended class.
 *
 *  @note: This must not contain a constructor or destructor!
 *  @note: All functions must be prefixed with "_" to prevent accidental virtualization.
 */
static DECLARE_EXTENDING_CLASS_AND_PAIR(ObjectClass)
{
public:
    bool _Paradrop(Coord const& coord);
};


/**
 *  Replacement of ObjectClass::Paradrop.
 *
 *  @author: ZivDero
 */
bool ObjectClassExt::_Paradrop(Coord const& coord)
{
    if (Map.In_Local_Radar(coord)) {
        IsFalling = true;

        Coord ucoord = coord;

        /**
         *  We don't really want units to be off-center so snap their coordinate to the
         *  center of the cell.
         */
        if (RTTI == RTTI_UNIT) {
            ucoord = Coord_Snap(ucoord);
        }

        CellClass* cellptr = &Map[ucoord];

        /**
         *  Set the unit to be on the bridge if there's a bridge on the cell.
         */
        IsOnBridge = cellptr->IsUnderBridge;

        /**
         *  Ported from YR, unknown condition. Is bridge not intact, maybe?
         */
        if (cellptr->IsUnderBridge && !cellptr->Bit2_32) {
            return false;
        }

        /**
         *  Can't move here, don't try paradropping here.
         */
        if (Is_Techno() && !cellptr->Is_Clear_To_Move(TClass->Speed, false, false, Map.Get_Cell_Zone(ucoord.As_Cell(), TClass->MZone, IsOnBridge), TClass->MZone)) {
            return false;
        }

        if (Unlimbo(ucoord, DIR_S)) {

            /**
             *  Make sure that if this is a techno that was on a carryall,
             *  it doesn't stay "on the carryall".
             */
            if (Is_Techno()) {
                reinterpret_cast<TechnoClass*>(this)->IsOnCarryall = false;
            }

            PositionCoord = ucoord;

            AnimClass* anim = nullptr;
            if (RTTI == RTTI_BULLET) {
                anim = new AnimClass(Rule->BombParachute, ucoord);
            } else {
                anim = new AnimClass(Rule->Parachute, ucoord);
            }

            /**
             *  If the animation was created, then attach it to this object.
             */
            if (anim != nullptr) {
                anim->Attach_To(this);

                /**
                 *  For parachuted technos, we can try to remap the parachute with the color of the
                 *  techno's owner.
                 */
                if (RTTI != RTTI_BULLET && Is_Techno() && anim->Class->IsAltPalette) {
                    anim->AlternativeDrawer = ColorSchemes[reinterpret_cast<TechnoClass*>(this)->House->Scheme]->Converter;
                    anim->AlternativeBrightness = Get_Cell_Ptr()->Brightness;
                }
            }
            return true;
        }
    }


    return false;
}

/*
* Triggers when any object is destroyed via Take Damage.
* This springs a custom trigger event for units that were specifically destroyed by taking damage if they had an attached tag.
* Changing houses, capturing it, and selling it doesn't count for this event.
* Destruction trigger event persists even when captured - very useful for "Capture and Keep Safe" missions.
* Works well with Entered By by linking triggers together - if you want to react to the structure being captured.
* Also works well with the Building Exists trigger.
*/
DEFINE_HOOK(0x0058634A, _Take_Damage_Unit_Destroyed_Trigger_Event_Patch, 6) {    
    GET(ObjectClass*, obj, ESI);

    if (obj->Tag) {        
        obj->Tag->Spring(static_cast<TEventType>(EXT_TEVENT_DESTROYED_ONLY), obj);
    }

    return 0;
}


DEFINE_HOOK(0x00584C18, _ObjectClass_AI_AmbientSound_Patch, 5)
{
    GET(ObjectClass*, this_ptr, ESI);

    auto ext = Extension::Fetch(this_ptr);
    if (ext != nullptr) {
        Extension::Fetch(this_ptr)->Ambient_AI();
    }

    return 0;
}


DEFINE_HOOK(0x00585AAD, _ObjectClass_Limbo_AmbientSound_Patch, 5)
{
    GET(ObjectClass*, this_ptr, ESI);

    auto ext = Extension::Fetch(this_ptr);
    if (ext != nullptr) {
        Extension::Fetch(this_ptr)->Stop_Ambient();
    }

    return 0;
}


/**
 *  Main function for patching the hooks.
 */
void ObjectClassExtension_Hooks()
{
    Patch_Jump(0x005864C0, &ObjectClassExt::_Paradrop);
}
