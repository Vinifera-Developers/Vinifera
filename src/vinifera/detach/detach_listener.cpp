/*******************************************************************************
/*                 O P E N  S O U R C E  --  V I N I F E R A                  **
/*******************************************************************************
 *  @brief  RTTI-based dispatcher for the type-indexed detach registry.
 *
 *  SPDX-License-Identifier: GPL-3.0-or-later
 *  Copyright (c) 2020-2026 Vinifera contributors
 ******************************************************************************/
#include "detach_listener.h"

#include "abstract.h"
#include "aircraft.h"
#include "anim.h"
#include "building.h"
#include "factory.h"
#include "infantry.h"
#include "techno.h"
#include "tibsun_defines.h"
#include "unit.h"


namespace Vinifera::Detach {


/**
 *  When a new Listener<T> is introduced for an AbstractClass-derived T, this
 *  switch must dispatch to Registry<T>::Notify from every RTTI case whose
 *  runtime type IS-A T. Otherwise the listener will compile and register
 *  but never fire.
 *
 *  Today's listeners (and the cases that must reach them):
 *      Listener<BuildingClass>  -> RTTI_BUILDING
 *      Listener<AircraftClass>  -> RTTI_AIRCRAFT
 *      Listener<TechnoClass>    -> RTTI_BUILDING, RTTI_UNIT, RTTI_AIRCRAFT, RTTI_INFANTRY
 *      Listener<AnimClass>      -> RTTI_ANIM
 *      Listener<FactoryClass>   -> RTTI_FACTORY
 *      Listener<AbstractClass>  -> all (fan-out below the switch)
 */
void Notify_Abstract(AbstractClass* t, bool all)
{
    if (t == nullptr) {
        return;
    }

    switch (t->RTTI) {
    case RTTI_BUILDING:
        Registry<BuildingClass>::Notify(static_cast<BuildingClass*>(t), all);
        Registry<TechnoClass>::Notify(static_cast<TechnoClass*>(t), all);
        break;

    case RTTI_UNIT:
        Registry<TechnoClass>::Notify(static_cast<TechnoClass*>(t), all);
        break;

    case RTTI_AIRCRAFT:
        Registry<AircraftClass>::Notify(static_cast<AircraftClass*>(t), all);
        Registry<TechnoClass>::Notify(static_cast<TechnoClass*>(t), all);
        break;

    case RTTI_INFANTRY:
        Registry<TechnoClass>::Notify(static_cast<TechnoClass*>(t), all);
        break;

    case RTTI_ANIM:
        Registry<AnimClass>::Notify(static_cast<AnimClass*>(t), all);
        break;

    case RTTI_FACTORY:
        Registry<FactoryClass>::Notify(static_cast<FactoryClass*>(t), all);
        break;

    default:
        break;
    }

    Registry<AbstractClass>::Notify(t, all);
}


} // namespace Vinifera::Detach
